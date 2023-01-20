//IBM_PROLOG_BEGIN_TAG
/* 
 * Copyright 2003,2023 IBM International Business Machines Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//IBM_PROLOG_END_TAG


//--------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------
#include <SBEFIFOInstruction.H>
#include <arpa/inet.h>
#include <sstream>
#include <iomanip>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#ifdef OTHER_USE
#include <OutputLite.H>
extern OutputLite out;
#endif

/*****************************************************************************/
/* SBEFIFOInstruction Implementation *****************************************/
/*****************************************************************************/
SBEFIFOInstruction::SBEFIFOInstruction(void) : Instruction(),
deviceString(""),
timeout(0),
replyLength(0)
{
    // version 0x1 is base sbefifo to a proc based solely on deviceString
    // version 0x2 is base sbefifo to a proc + port (not supported on bmc croserver)
    // version 0x3 is sbefifo to another device based on deviceString + port
    version = 0x3;
    type = SBEFIFO;
}

SBEFIFOInstruction::~SBEFIFOInstruction(void)
{
}

uint32_t SBEFIFOInstruction::setup(InstructionCommand i_command, std::string &i_deviceString, uint32_t i_timeout, uint32_t i_replyLength, uint32_t i_flags, ecmdDataBuffer * i_data, uint32_t i_port)
{
    version = 0x1;
    deviceString = i_deviceString;
    command = i_command;
    timeout = i_timeout;
    replyLength = i_replyLength;
    flags = i_flags | INSTRUCTION_FLAG_DEVSTR;
    if(i_data != NULL)
    {
        i_data->shareBuffer(&data);
    }
    if ( i_port != UINT32_MAX )
    {
        port = i_port;
        version = 0x3;
    }
    return 0;
}


uint32_t SBEFIFOInstruction::execute(ecmdDataBuffer & o_data, InstructionStatus & o_status, Handle ** io_handle)
{
    int rc = 0;

    /* set the version of this instruction in the status */
    o_status.instructionVersion = version;

    /* check for any previous errors to report back */
    if (error)
    {
        rc = o_status.rc = error;
        return rc;
    }

    switch(command)
    {
        case SUBMIT:
        {
            char errstr[200];

            /* Open the Handle */
            rc = sbefifo_open(io_handle, o_status);
            if (rc)
            {
                o_status.rc = rc;
                break;
            }

            o_data.setWordLength(replyLength);

            /* Actually write to the device */
            errno = 0;

            if (flags & INSTRUCTION_FLAG_SERVER_DEBUG)
            {
                snprintf(errstr, 200, "SERVER_DEBUG : adal_sbefifo_submit(*io_handle, &l_request, &l_reply, %d)\n", timeout);
                o_status.errorMessage.append(errstr);
            }

            uint32_t l_reply_wordcount = 0;
            rc = sbefifo_submit(*io_handle, o_data, o_status, l_reply_wordcount);

            // resize for wordcount in reply
            uint32_t db_rc = o_data.shrinkBitLength(l_reply_wordcount * 32);
            if (db_rc)
            {
                snprintf(errstr, 200, "SBEFIFOInstruction::execute(SBEFIFO) Problem resizing reply buffer: rc = 0x%08X\n", db_rc);
                o_status.errorMessage.append(errstr);
                if (db_rc == ECMD_DBUF_BUFFER_OVERFLOW)
                {
                    snprintf(errstr, 200, "SBEFIFOInstruction::execute(SBEFIFO) rc = ECMD_DBUF_BUFFER_OVERFLOW, max reply = %d, actual reply = %d\n", replyLength, l_reply_wordcount);
                    o_status.errorMessage.append(errstr);
                }
                snprintf(errstr, 200, "SBEFIFOInstruction::execute(SBEFIFO) adal_sbefifo_submit: rc = %d, errno %d\n", rc, errno);
                o_status.errorMessage.append(errstr);
                rc = o_status.rc = SERVER_SBEFIFO_SUBMIT_FAIL;
                break;
            }

            if (rc < 0)
            {
                snprintf(errstr, 200, "SBEFIFOInstruction::execute(SBEFIFO) Problem submitting to SBEFIFO device : rc = %d, errno %d\n", rc, errno);
                o_status.errorMessage.append(errstr);
                rc = o_status.rc = SERVER_SBEFIFO_SUBMIT_FAIL;
                sbefifo_ffdc_and_reset(io_handle, o_status);
                break;
            }
            else
            {
                rc = o_status.rc = SERVER_COMMAND_COMPLETE;
            }
        }
        break;
        case REQUEST_RESET:
        {
            /* Open the Handle */
            rc = sbefifo_open(io_handle, o_status);
            if (rc)
            {
                o_status.rc = rc;
                break;
            }

            sbefifo_ffdc_and_reset(io_handle, o_status);
            rc = o_status.rc = SERVER_COMMAND_COMPLETE;
        }
        break;
        default:
        break;
    }

    return (uint32_t) rc;
}

uint32_t SBEFIFOInstruction::flatten(uint8_t * o_data, uint32_t i_len) const
{
    uint32_t rc = 0;
    uint32_t * o_ptr = (uint32_t *) o_data;
  
    if (i_len < flattenSize())
    {
        out.error("SBEFIFOInstruction::flatten", "i_len %d bytes is too small to flatten\n", i_len);
        rc = 1;
    }
    else
    {
        // clear memory
        memset(o_data, 0, flattenSize());
        o_ptr[0] = htonl(version);
        o_ptr[1] = htonl(command);
        o_ptr[2] = htonl(flags);
        o_ptr[3] = htonl(timeout);
        o_ptr[4] = htonl(replyLength);
        uint32_t offset = 5;
        if( version == 0x3 )
        {
            o_ptr[offset] = htonl(port);
            offset++;
        }
        uint32_t deviceStringSize = deviceString.size() + 1;
        if (deviceStringSize % sizeof(uint32_t))
        {
            deviceStringSize += (sizeof(uint32_t) - (deviceStringSize % sizeof(uint32_t)));
        }
        o_ptr[offset] = htonl(deviceStringSize);
        uint32_t dataSize = data.flattenSize();
        data.flatten((uint8_t *) (o_ptr + offset + 2 + (deviceStringSize / sizeof(uint32_t))), dataSize);
        o_ptr[offset + 1] = htonl(dataSize);
        if (deviceString.size() > 0)
        {
            strcpy(((char *)(o_ptr + offset + 2)), deviceString.c_str());
        }
    }
    return rc;
}

uint32_t SBEFIFOInstruction::unflatten(const uint8_t * i_data, uint32_t i_len)
{
    uint32_t rc = 0;
    uint32_t * i_ptr = (uint32_t *) i_data;

    version = ntohl(i_ptr[0]);
    if( version == 0x2 )
    {
        error = rc = SERVER_UNKNOWN_INSTRUCTION_VERSION;
    }
    else if( version <= 0x3 )
    {
        command = (InstructionCommand) ntohl(i_ptr[1]);
        flags = ntohl(i_ptr[2]);
        timeout = ntohl(i_ptr[3]);
        replyLength = ntohl(i_ptr[4]);
        uint32_t offset = 5;
        if( version == 0x3 )
        {
            port = ntohl(i_ptr[offset]); offset++;
        }
        uint32_t deviceStringSize = ntohl(i_ptr[offset]);
        uint32_t dataSize = ntohl(i_ptr[offset + 1]);
        rc = data.unflatten((uint8_t *) (i_ptr + offset + 2 + (deviceStringSize / sizeof(uint32_t))), dataSize);
        if( rc ) { error = rc; }
        if( deviceStringSize > 0 )
        {
            deviceString = ((char *)(i_ptr + offset + 2));
        }
    }
    else
    {
        error = rc = SERVER_UNKNOWN_INSTRUCTION_VERSION;
    }
    return rc;
}

uint32_t SBEFIFOInstruction::flattenSize(void) const
{
    // FIXME remove data and timeout and change to different size for REQUEST_RESET
    //
    uint32_t size = 7 * sizeof(uint32_t); // version, command, flags, timeout, replyLength, deviceStringSize, dataSize
    if( version == 0x3 ) size += sizeof(uint32_t); // port
    uint32_t deviceStringSize = deviceString.size() + 1;
    if (deviceStringSize % sizeof(uint32_t)) {
      deviceStringSize += (sizeof(uint32_t) - (deviceStringSize % sizeof(uint32_t)));
    }
    size += deviceStringSize; // deviceString
    size += data.flattenSize(); // data

    return size;
}

std::string SBEFIFOInstruction::dumpInstruction(void) const
{
    std::ostringstream oss;
    oss << "SBEFIFOInstruction" << std::endl;
    oss << "version       : " << version << std::endl;
    oss << "command       : " << InstructionCommandToString(command) << std::endl;
    oss << "type          : " << InstructionTypeToString(type) << std::endl;
    oss << "flags         : " << InstructionFlagToString(flags) << std::endl;
    oss << "deviceString  : " << deviceString << std::endl;
    oss << "timeout       : " << timeout << std::endl;
    oss << "replyLength   : " << replyLength << std::endl;
    oss << "port          : " << port << std::endl;
    oss << "data length   : " << data.getBitLength() << std::endl;
    oss << "data          : ";
    for(uint32_t j = 0; j < data.getWordLength(); j++)
    {
        oss << std::hex << std::setw(8) << std::setfill('0') << data.getWord(j) << " ";
        if (!((j+1) % 5)) oss << "\n\t\t";
    }
    oss << std::dec << std::endl;

    return oss.str();
}

uint64_t SBEFIFOInstruction::getHash(void) const {
    uint32_t devstrhash = 0x0;
    uint32_t rc = devicestring_genhash(deviceString, devstrhash);
    uint64_t hash64 = 0x0ull;
    hash64 |= ((0x0000000Full & type)     << 60);   // 4 bits for type
    if( version == 0x3 )
    {
        hash64 |= ((0x000000FFull & port)     << 52);   // 8 bits for port
    }
    if (rc == 0)
    {
        hash64 |= ((uint64_t) devstrhash);
    }
    return hash64;
}

uint32_t SBEFIFOInstruction::closeHandle(Handle ** i_handle)
{
    uint32_t rc = 0;
    rc = sbefifo_close(*i_handle);
    *i_handle = NULL;
    return rc;
}

std::string SBEFIFOInstruction::getInstructionVars(const InstructionStatus & i_status) const
{
    std::ostringstream oss;

    oss << std::hex << std::setfill('0');
    oss << "rc: " << std::setw(8) << i_status.rc;
    if (i_status.data.getWordLength() > 0) {
        oss << " status: " << std::setw(8) << i_status.data.getWord(0);
    }
    oss << " devstr: " << deviceString;
    if (command != Instruction::REQUEST_RESET)
    {
        oss << " data: " << std::setw(8) << data.getWord(0);
    }

    return oss.str();
}

