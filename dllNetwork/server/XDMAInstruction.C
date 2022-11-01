//IBM_PROLOG_BEGIN_TAG
/* 
 * Copyright 2003,2020 IBM International Business Machines Corp.
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
#ifndef __STDC_FORMAT_MACROS
  #define __STDC_FORMAT_MACROS 1
#endif
#include <XDMAInstruction.H>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <iomanip>
#include <errno.h>
#include <inttypes.h>

#ifdef OTHER_USE
#include <arpa/inet.h>
#include <OutputLite.H>
extern OutputLite out;
#else
#include <CronusData.H>
#include <cronus_endian.h>
#endif

/*****************************************************************************/
/* XDMAInstruction Implementation *********************************************/
/*****************************************************************************/
XDMAInstruction::XDMAInstruction(void) : Instruction(),
address(0),
readLength(0),
msDelay(0),
data(NULL)
{
    version = 0x1;
    type = XDMA;
}

XDMAInstruction::~XDMAInstruction(void)
{
    while (!mydata.empty())
    {
        delete mydata.back();
        mydata.pop_back();
    }
}

uint32_t XDMAInstruction::setup(InstructionCommand i_command, std::string &i_deviceString, uint32_t i_address, uint32_t i_readLength, uint32_t i_msDelay, uint32_t i_flags, const ecmdDataBuffer * i_data)
{
    command = i_command;
    flags = i_flags;
    deviceString = i_deviceString;
    address = i_address;
    readLength = i_readLength;
    msDelay = i_msDelay;
    data = i_data;
    return 0;
}

uint32_t XDMAInstruction::setup(InstructionCommand i_command, std::string &i_deviceString, uint32_t i_address, uint32_t i_msDelay, uint32_t i_flags, const ecmdDataBuffer * i_data)
{
    command = i_command;
    flags = i_flags;
    deviceString = i_deviceString;
    address = i_address;
    readLength = 0;
    msDelay = i_msDelay;
    data = i_data;
    return 0;
}

uint32_t XDMAInstruction::execute(ecmdDataBuffer & o_data, InstructionStatus & o_status, Handle ** io_handle)
{
    int rc = 0;
    char errstr[200];

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
        case XDMA_COMMAND:
            {
                // Open the Handle
                rc = xdma_open(io_handle, o_status);
                if ( rc )
                {
                    o_status.rc = rc;
                    return rc;
                }

                ssize_t bytelen = readLength % 8 ? (readLength / 8) + 1 : readLength / 8;

                // write data to the device
                if ( flags & INSTRUCTION_FLAG_SERVER_DEBUG )
                {
                    snprintf(errstr, 200, "SERVER_DEBUG : dma_command() address = 0x%08X, readLength = %u, flags = 0x%08X\n", address, readLength, flags);
                    o_status.errorMessage.append(errstr);
                }

                errno = 0;
                rc = xdma_command(*io_handle, o_data, o_status);

                if ( flags & INSTRUCTION_FLAG_SERVER_DEBUG )
                {
                    std::string words;
                    genWords(o_data, words);
                    snprintf(errstr, 200, "SERVER_DEBUG : xdma_command() o_data = %s, rc = %u, errno = %d\n", words.c_str(), rc, errno);
                    o_status.errorMessage.append(errstr);
                }

                if ( rc != bytelen )
                {
                    snprintf(errstr, 200, "XDMAInstruction::execute(DMAIN) Write length exp (%zd) actual (%d)\n", bytelen, rc);
                    o_status.errorMessage.append(errstr);
                    rc = o_status.rc = SERVER_DMA_COMMAND_FAIL;
                    xdma_ffdc(io_handle, o_status); 
                    break;
                }
                else
                {
                    rc = o_status.rc = SERVER_COMMAND_COMPLETE;
                }
            }
            break;

        default:
            rc = o_status.rc = SERVER_COMMAND_NOT_SUPPORTED;
            break;
      }

      return rc;
}

uint32_t XDMAInstruction::flatten(uint8_t * o_data, uint32_t i_len) const
{
    uint32_t rc = 0;
    uint32_t * o_ptr = (uint32_t *) o_data;
  
    if (i_len < flattenSize())
    {
        out.error("XDMAInstruction::flatten", "i_len %d bytes is too small to flatten\n", i_len);
        rc = 1;
    }
    else
    {
        // clear memory
        memset(o_data, 0, flattenSize());
        o_ptr[0] = htonl(version);
        o_ptr[1] = htonl(command);
        o_ptr[2] = htonl(flags);
        o_ptr[3] = htonl(address);
        o_ptr[4] = htonl(readLength);
        o_ptr[5] = htonl(msDelay);
        uint32_t deviceStringSize = deviceString.size() + 1;
        if (deviceStringSize % sizeof(uint32_t))
            deviceStringSize += (sizeof(uint32_t) - (deviceStringSize % sizeof(uint32_t)));
        o_ptr[6] = htonl(deviceStringSize);
        uint32_t dataSize = getFlattenSize(data);
        o_ptr[7] = htonl(dataSize);
        if (deviceString.size() > 0)
            strcpy(((char *)(o_ptr + 8)), deviceString.c_str());
        if (data)
            data->flatten((uint8_t *) (o_ptr + 8 + (deviceStringSize / sizeof(uint32_t))), dataSize);
    }

    return rc;
}

uint32_t XDMAInstruction::unflatten(const uint8_t * i_data, uint32_t i_len)
{
    uint32_t rc = 0;
    uint32_t * i_ptr = (uint32_t *) i_data;

    version = ntohl(i_ptr[0]);
    if (version == 0x1)
    {
        command = (InstructionCommand) ntohl(i_ptr[1]);
        flags = ntohl(i_ptr[2]);
        address = ntohl(i_ptr[3]);
        readLength = ntohl(i_ptr[4]);
        msDelay = ntohl(i_ptr[5]);
        uint32_t deviceStringSize = ntohl(i_ptr[6]);
        uint32_t dataSize = ntohl(i_ptr[7]);
        if (deviceStringSize > 0)
            deviceString = ((char *)(i_ptr + 8));
        ecmdDataBuffer * tempdata = new ecmdDataBuffer();
        mydata.push_back(tempdata);
        data = tempdata;
        rc = tempdata->unflatten((uint8_t *) (i_ptr + 8 + (deviceStringSize / sizeof(uint32_t))), dataSize);
        if (rc) { error = rc; }
    }
    else
    {
        error = rc = SERVER_UNKNOWN_INSTRUCTION_VERSION;
    }
    return rc;
}

uint32_t XDMAInstruction::flattenSize(void) const
{
    uint32_t size = 0;
    if (version >= 0x1)
    {
        size = 8 * sizeof(uint32_t); // version, command, flags, address, readLength, msDelay, dataSize, deviceStringSize
        uint32_t deviceStringSize = deviceString.size() + 1;
        if (deviceStringSize % sizeof(uint32_t))
            deviceStringSize += (sizeof(uint32_t) - (deviceStringSize % sizeof(uint32_t))); 
        size += deviceStringSize; // deviceString

        size += getFlattenSize(data); // data
    }
    return size;
}

std::string XDMAInstruction::dumpInstruction(void) const
{
    std::ostringstream oss;
    oss << "XDMAInstruction" << std::endl;
    oss << "version       : " << version << std::endl;
    oss << "command       : " << InstructionCommandToString(command) << std::endl;
    oss << "type          : " << InstructionTypeToString(type) << std::endl;
    oss << "flags         : " << InstructionFlagToString(flags) << std::endl;
    oss << "deviceString  : " << deviceString << std::endl;
    oss << "address       : " << std::hex << std::setw(8) << std::setfill('0') << address << std::dec << std::endl;
    oss << "readLength    : " << std::dec << readLength << std::endl;
    oss << "msDelay       : " << std::dec << msDelay << std::endl;
    if (data)
    {
        oss << "data length   : " << data->getBitLength() << std::endl;
        oss << "data          : ";
        for(uint32_t j = 0; j < data->getWordLength(); j++)
        {
            oss << std::hex << std::setw(8) << std::setfill('0') << data->getWord(j) << " ";
            if (!((j+1) % 5)) oss << "\n\t\t";
        }
        oss << std::dec << std::endl;
    }

    return oss.str();
}

uint64_t XDMAInstruction::getHash(void) const
{
    uint32_t devstrhash = 0x0;
    uint32_t rc = devicestring_genhash(deviceString, devstrhash);
    uint64_t hash64 = 0x0ull;
    hash64 |= ((0x0000000Full & type)     << 60);
    hash64 |= ((0x0FFFFFFFull & address) << 32);
    if (rc == 0)
        hash64 |= ((uint64_t) devstrhash);
    return hash64;
}

uint32_t XDMAInstruction::closeHandle(Handle ** i_handle)
{
    int rc = 0;

    /* Close the device */
    errno = 0;
    switch (command)
    {
        default:
        rc = xdma_close(*i_handle);
        break;
    }
    *i_handle = NULL;
    if (rc) rc = SERVER_DMA_CLOSE_FAIL;

    return rc;
}

std::string XDMAInstruction::getInstructionVars(const InstructionStatus & i_status) const
{
    std::ostringstream oss;

    oss << std::hex << std::setfill('0');
    oss << "rc: " << std::setw(8) << i_status.rc;
    oss << " devstr: " << deviceString;
    oss << " address: " << std::setw(8) << address;
    oss << " readLength: " << std::setw(8) << readLength;

    return oss.str();
}
