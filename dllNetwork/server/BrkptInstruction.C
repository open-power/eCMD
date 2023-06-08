//IBM_PROLOG_BEGIN_TAG
/* 
 * Copyright 2019 IBM International Business Machines Corp.
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
#include <BrkptInstruction.H>
#include <ecmdSharedUtils.H>
#include <arpa/inet.h>
#include <sstream>
#include <iomanip>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#ifdef OTHER_USE
#include <OutputLite.H>
extern OutputLite out;
#else
#include <CronusData.H>
#endif

static void packReturnData(const ecmdDataBuffer & i_virtualAddress,
                           const std::list<cipBrkptTableEntry> & i_brkptTableEntries,
                           ecmdDataBuffer & o_data);
static void packReturnData(const std::list<cipSoftwareEvent_t> & i_events,
                           ecmdDataBuffer & o_data);

/*****************************************************************************/
/* BrkptInstruction Implementation *******************************************/
/*****************************************************************************/
BrkptInstruction::BrkptInstruction(void) : Instruction(),
    deviceString(""),
    timeout(0),
    steps(0)
{
    version = 0x1;
    type = BRKPT;
}

BrkptInstruction::~BrkptInstruction(void)
{
}

uint32_t BrkptInstruction::setup(InstructionCommand i_command, std::string &i_deviceString, const ecmdChipTarget & i_target, const ecmdDataBuffer & i_address, const cipXlateVariables i_xlateVars, uint32_t i_flags)
{
    deviceString = i_deviceString;
    target = i_target;
    address = i_address;
    xlateVars = i_xlateVars;
    command = i_command;
    flags = i_flags | INSTRUCTION_FLAG_DEVSTR;
    return 0;
}

uint32_t BrkptInstruction::setup(InstructionCommand i_command, uint32_t i_timeout, uint32_t i_flags)
{
    timeout = i_timeout;
    command = i_command;
    flags = i_flags;
    return 0;
}

uint32_t BrkptInstruction::setup(InstructionCommand i_command, const ecmdChipTarget & i_target, uint32_t i_steps, uint32_t i_flags)
{
    target = i_target;
    command = i_command;
    steps = i_steps;
    flags = i_flags;
    return 0;
}

uint32_t BrkptInstruction::execute(ecmdDataBuffer & o_data, InstructionStatus & o_status, Handle ** io_handle)
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

    //o_status.errorMessage.append(dumpInstruction());
    switch(command)
    {
        case BRKPT_SET:
        case BRKPT_CLEAR:
        case BRKPT_GET:
        {
            std::list<cipBrkptTableEntry> l_brkptTableEntries;
            ecmdDataBuffer l_virtualAddress;
            rc = brkpt_general(*io_handle, o_status, l_brkptTableEntries, l_virtualAddress);
            if (rc == 0)
            {
                packReturnData(l_virtualAddress, l_brkptTableEntries, o_data);
                rc = o_status.rc = SERVER_COMMAND_COMPLETE;
            }
            else
                o_status.rc = rc;
        }
        break;
        case BRKPT_WAIT:
        {
            std::list<cipSoftwareEvent_t> l_events;
            rc = brkpt_wait(*io_handle, o_status, l_events);
            if (rc == 0)
            {
                packReturnData(l_events, o_data);
                rc = o_status.rc = SERVER_COMMAND_COMPLETE;
            }
            else
                o_status.rc = rc;
        }
        break;
        case BRKPT_INSTR_START:
        case BRKPT_INSTR_STOP:
        case BRKPT_INSTR_STEP:
            rc = brkpt_instr_general(*io_handle, o_status);
            if (rc == 0)
                rc = o_status.rc = SERVER_COMMAND_COMPLETE;
            else
                o_status.rc = rc;
        break;
        default:
            rc = o_status.rc = SERVER_COMMAND_NOT_SUPPORTED;
        break;
    }

    return (uint32_t) rc;
}

uint32_t BrkptInstruction::flatten(uint8_t * o_data, uint32_t i_len) const
{
    uint32_t rc = 0;
    uint32_t * o_ptr = (uint32_t *) o_data;
  
    if (i_len < flattenSize())
    {
        out.error("BrkptInstruction::flatten", "i_len %d bytes is too small to flatten\n", i_len);
        rc = 1;
    }
    else
    {
        // clear memory
        memset(o_data, 0, flattenSize());
        o_ptr[0] = htonl(version);
        o_ptr[1] = htonl(command);
        o_ptr[2] = htonl(flags);
        if (command == BRKPT_WAIT)
        {
            o_ptr[3] = htonl(timeout);
        }
        else if ((command == BRKPT_SET) ||
                 (command == BRKPT_CLEAR) ||
                 (command == BRKPT_GET))
        {
            uint32_t deviceStringSize = deviceString.size() + 1;
            if (deviceStringSize % sizeof(uint32_t))
                deviceStringSize += (sizeof(uint32_t) - (deviceStringSize % sizeof(uint32_t)));
            o_ptr[3] = htonl(deviceStringSize);
            uint32_t targetSize = target.flattenSize();
            o_ptr[4] = htonl(targetSize);
            uint32_t addressSize = address.flattenSize();
            o_ptr[5] = htonl(addressSize);
            uint32_t xlateVarsSize = xlateVars.flattenSize();
            o_ptr[6] = htonl(xlateVarsSize);
            uint32_t offset = 7;
            if (deviceString.size() > 0)
                strcpy(((char *)(o_ptr + offset)), deviceString.c_str());
            offset += deviceStringSize / sizeof(uint32_t);
            target.flatten((uint8_t *) (o_ptr + offset), targetSize);
            offset += targetSize / sizeof(uint32_t);
            address.flatten((uint8_t *) (o_ptr + offset), addressSize);
            offset += addressSize / sizeof(uint32_t);
            xlateVars.flatten((uint8_t *) (o_ptr + offset), xlateVarsSize);
        }
        else if ((command == BRKPT_INSTR_START) ||
                 (command == BRKPT_INSTR_STOP) ||
                 (command == BRKPT_INSTR_STEP))
        {
            o_ptr[3] = htonl(steps);
            uint32_t targetSize = target.flattenSize();
            o_ptr[4] = htonl(targetSize);
            uint32_t offset = 5;
            target.flatten((uint8_t *) (o_ptr + offset), targetSize);
        }
    }
    return rc;
}

uint32_t BrkptInstruction::unflatten(const uint8_t * i_data, uint32_t i_len)
{
    uint32_t rc = 0;
    uint32_t * i_ptr = (uint32_t *) i_data;

    version = ntohl(i_ptr[0]);
    if(version == 0x1)
    {
        command = (InstructionCommand) ntohl(i_ptr[1]);
        flags = ntohl(i_ptr[2]);
        if (command == BRKPT_WAIT)
        {
            timeout = ntohl(i_ptr[3]);
        }
        else if ((command == BRKPT_SET) ||
                 (command == BRKPT_CLEAR) ||
                 (command == BRKPT_GET))
        {
            uint32_t deviceStringSize = ntohl(i_ptr[3]);
            uint32_t targetSize = ntohl(i_ptr[4]);
            uint32_t addressSize = ntohl(i_ptr[5]);
            uint32_t xlateVarsSize = ntohl(i_ptr[6]);
            uint32_t offset = 7;
            if (deviceStringSize > 0)
                deviceString = ((char *)(i_ptr + offset));
            offset += deviceStringSize / sizeof(uint32_t);
            rc = target.unflatten((uint8_t *) (i_ptr + offset), targetSize);
            if (rc) { error = rc; }
            offset += targetSize / sizeof(uint32_t);
            rc = address.unflatten((uint8_t *) (i_ptr + offset), addressSize);
            if (rc) { error = rc; }
            offset += addressSize / sizeof(uint32_t);
            rc = xlateVars.unflatten((uint8_t *) (i_ptr + offset), xlateVarsSize);
            if (rc) { error = rc; }
        }
        else if ((command == BRKPT_INSTR_START) ||
                 (command == BRKPT_INSTR_STOP) ||
                 (command == BRKPT_INSTR_STEP))
        {
            steps = ntohl(i_ptr[3]);
            uint32_t targetSize = ntohl(i_ptr[4]);
            uint32_t offset = 5;
            rc = target.unflatten((uint8_t *) (i_ptr + offset), targetSize);
            if (rc) { error = rc; }
        }
    }
    else
    {
        error = rc = SERVER_UNKNOWN_INSTRUCTION_VERSION;
    }
    return rc;
}

uint32_t BrkptInstruction::flattenSize(void) const
{
    uint32_t size = 3 * sizeof(uint32_t); // version, command, flags
    if (command == BRKPT_WAIT)
    {
        size += sizeof(uint32_t); // timeout
    }
    else if ((command == BRKPT_SET) ||
             (command == BRKPT_CLEAR) ||
             (command == BRKPT_GET))
    {
        size += 4 * sizeof(uint32_t); // sizes of flattened objects
        uint32_t deviceStringSize = deviceString.size() + 1;
        if (deviceStringSize % sizeof(uint32_t))
            deviceStringSize += (sizeof(uint32_t) - (deviceStringSize % sizeof(uint32_t)));
        size += deviceStringSize; // deviceString
        size += target.flattenSize(); // target
        size += address.flattenSize(); // address
        size += xlateVars.flattenSize(); // xlateVars
    }
    else if ((command == BRKPT_INSTR_START) ||
             (command == BRKPT_INSTR_STOP) ||
             (command == BRKPT_INSTR_STEP))
    {
        size += sizeof(uint32_t); // steps
        size += sizeof(uint32_t); // target size
        size += target.flattenSize(); // target
    }

    return size;
}

std::string BrkptInstruction::dumpInstruction(void) const
{
    std::ostringstream oss;
    oss << "BrkptInstruction" << std::endl;
    oss << "version       : " << version << std::endl;
    oss << "command       : " << InstructionCommandToString(command) << std::endl;
    oss << "type          : " << InstructionTypeToString(type) << std::endl;
    oss << "flags         : " << InstructionFlagToString(flags) << std::endl;
    if (command == BRKPT_WAIT)
        oss << "timeout       : " << timeout << std::endl;
    else
        oss << "target        : " << ecmdWriteTarget(target, ECMD_DISPLAY_TARGET_HYBRID) << std::endl;
    if ((command == BRKPT_SET) ||
        (command == BRKPT_CLEAR) ||
        (command == BRKPT_GET))
    {
        oss << "deviceString  : " << deviceString << std::endl;
        oss << "address       : " << ((address.getBitLength() > 0) ? address.genHexLeftStr() : "" ) << std::endl;
        oss << "xlateVars.tagsActive  : " << (xlateVars.tagsActive ? "true" : "false") << std::endl;
        oss << "xlateVars.mode32bit   : " << (xlateVars.mode32bit ? "true" : "false") << std::endl;
        oss << "xlateVars.writeECC    : " << (xlateVars.writeECC ? "true" : "false") << std::endl;
        oss << "xlateVars.manualXlateFlag : " << (xlateVars.manualXlateFlag ? "true" : "false") << std::endl;
        oss << "xlateVars.addrType    : " << xlateVars.addrType << std::endl;
        oss << "xlateVars.partitionId : " << xlateVars.partitionId << std::endl;
    }
    if ((command == BRKPT_INSTR_START) ||
        (command == BRKPT_INSTR_STOP) ||
        (command == BRKPT_INSTR_STEP))
        oss << "steps         : " << steps << std::endl;
    return oss.str();
}

uint64_t BrkptInstruction::getHash(void) const {
    uint32_t devstrhash = 0x0;
    uint32_t rc = 0;
    if (deviceString.size() != 0)
        rc = devicestring_genhash(deviceString, devstrhash);
    uint64_t hash64 = 0x0ull;
    hash64 |= ((0x0000000Full & type)     << 60);
    if (rc == 0) {
        hash64 |= ((uint64_t) devstrhash);
    }
    return hash64;
}

uint32_t BrkptInstruction::closeHandle(Handle ** i_handle)
{
    uint32_t rc = 0;
    *i_handle = NULL;
    return rc;
}

std::string BrkptInstruction::getInstructionVars(const InstructionStatus & i_status) const
{
    std::ostringstream oss;

    oss << std::hex << std::setfill('0');
    oss << "rc: " << std::setw(8) << i_status.rc;
    if (i_status.data.getWordLength() > 0) {
        oss << " status: " << std::setw(8) << i_status.data.getWord(0);
    }
    oss << " devstr: " << deviceString;

    return oss.str();
}

void BrkptInstruction::unpackReturnData(const ecmdDataBuffer & i_data, std::list<cipBrkptTableEntry> & o_brkptTableEntries, ecmdDataBuffer & o_virtualAddress)
{
    uint32_t word_offset = 0;
    uint32_t l_virtualAddress_flat_size = i_data.getWord(word_offset);
    word_offset += 1;

    uint8_t * l_data = new uint8_t[l_virtualAddress_flat_size];
    i_data.extract(l_data, word_offset * 32, l_virtualAddress_flat_size * 8);
    o_virtualAddress.unflatten(l_data, l_virtualAddress_flat_size);
    delete [] l_data;
    word_offset += l_virtualAddress_flat_size / sizeof(uint32_t);

    uint32_t l_list_size = i_data.getWord(word_offset);
    word_offset += 1;

    cipBrkptTableEntry l_empty_entry;
    for (uint32_t l_list_entry = 0; l_list_entry < l_list_size; l_list_entry++)
    {
        uint32_t l_entry_flat_size = i_data.getWord(word_offset);
        word_offset += 1;

        uint8_t * l_data = new uint8_t[l_entry_flat_size];
        i_data.extract(l_data, word_offset * 32, l_entry_flat_size * 8);
        o_brkptTableEntries.push_back(l_empty_entry);
        o_brkptTableEntries.back().unflatten(l_data, l_entry_flat_size);
        delete [] l_data;
        word_offset += l_entry_flat_size / sizeof(uint32_t);
        // backwards compatibility handling here
        // Old way:  word_offset in pack would overwrite end of flattened object
        // New way:  word align everything, if flattened object isn't word aligned, make it so
        if ( o_brkptTableEntries.back().flattenSize() == l_entry_flat_size ) // condition to handle backwards compatibility (is this enough) 
            if ( l_entry_flat_size % 4 ) word_offset++;   // l_entry_flat_size is not word bound, needs to be, bump if we are not word bound.
    }
}

void BrkptInstruction::unpackReturnData(const ecmdDataBuffer & i_data, std::list<cipSoftwareEvent_t> & o_events)
{
    uint32_t word_offset = 0;
    uint32_t l_list_size = i_data.getWord(word_offset);
    word_offset += 1;

    cipSoftwareEvent_t l_empty_entry;
    for (uint32_t l_list_entry = 0; l_list_entry < l_list_size; l_list_entry++)
    {
        uint32_t l_entry_flat_size = i_data.getWord(word_offset);
        word_offset += 1;

        uint8_t * l_data = new uint8_t[l_entry_flat_size];
        i_data.extract(l_data, word_offset * 32, l_entry_flat_size * 8);
        o_events.push_back(l_empty_entry);
        o_events.back().unflatten(l_data, l_entry_flat_size);
        delete [] l_data;
        word_offset += l_entry_flat_size / sizeof(uint32_t);
        // backwards compatibility handling here
        // Old way:  word_offset in pack would overwrite end of flattened object
        // New way:  word align everything, if flattened object isn't word aligned, make it so
        if ( o_events.back().flattenSize() == l_entry_flat_size ) // condition to handle backwards compatibility (is this enough) 
            if ( l_entry_flat_size % 4 ) word_offset++;   // l_entry_flat_size is not word bound, needs to be, bump if we are not word bound.
    }
}

static void packReturnData(const ecmdDataBuffer & i_virtualAddress, const std::list<cipBrkptTableEntry> & i_brkptTableEntries, ecmdDataBuffer & o_data)
{
    // byte size of flattened l_virtualAddress (word)
    uint32_t l_virtualAddress_flat_size = i_virtualAddress.flattenSize();
    // data for l_virtualAddress
    // number of elements in list (word)
    uint32_t l_list_size = i_brkptTableEntries.size();
    uint32_t l_list_flat_size = 0;
    for (std::list<cipBrkptTableEntry>::const_iterator entry = i_brkptTableEntries.begin();
         entry != i_brkptTableEntries.end();
         entry++)
    {
        // byte size of element n (word)
        l_list_flat_size += entry->flattenSize();
        // data for element n
        if (l_list_flat_size % 4)  // 4 bytes in a word, modify word boundary if not 0 here.   0 means we're on a word boundary
        {
            l_list_flat_size += (4 - (l_list_flat_size % 4));  // add number of bytes to make this a word boundary
        }
    }
    uint32_t total_size = (2 + l_list_size) * sizeof(uint32_t);
    total_size += l_virtualAddress_flat_size;
    total_size += l_list_flat_size;

    o_data.setByteLength(total_size);

    uint32_t word_offset = 0;
    o_data.setWord(word_offset, l_virtualAddress_flat_size);
    word_offset += 1;

    uint8_t * l_data = new uint8_t[l_virtualAddress_flat_size];
    i_virtualAddress.flatten(l_data, l_virtualAddress_flat_size);
    o_data.insert(l_data, word_offset * 32, l_virtualAddress_flat_size * 8);
    delete [] l_data;
    word_offset += l_virtualAddress_flat_size / sizeof(uint32_t);

    o_data.setWord(word_offset, l_list_size);
    word_offset += 1;

    for (std::list<cipBrkptTableEntry>::const_iterator entry = i_brkptTableEntries.begin();
         entry != i_brkptTableEntries.end();
         entry++)
    {
        uint32_t l_entry_flat_size = entry->flattenSize();
        o_data.setWord(word_offset, l_entry_flat_size);
        word_offset += 1;

        uint8_t * l_data = new uint8_t[l_entry_flat_size];
        entry->flatten(l_data, l_entry_flat_size);
        o_data.insert(l_data, word_offset * 32, l_entry_flat_size * 8);
        delete [] l_data;
        word_offset += l_entry_flat_size / sizeof(uint32_t);
        if ( l_entry_flat_size % 4 ) word_offset++;   // l_entry_flat_size is not word bound, needs to be, bump if we are not word bound.
    }
}

static void packReturnData(const std::list<cipSoftwareEvent_t> & i_events, ecmdDataBuffer & o_data)
{
    // number of elements in list (word)
    uint32_t l_list_size = i_events.size();
    uint32_t l_list_flat_size = 0;
    for (std::list<cipSoftwareEvent_t>::const_iterator entry = i_events.begin();
         entry != i_events.end();
         entry++)
    {
        // byte size of element n (word)
        l_list_flat_size += entry->flattenSize();
        // data for element n
        if (l_list_flat_size % 4)  // 4 bytes in a word, modify word boundary if not 0 here.   0 means we're on a word boundary
        {
            l_list_flat_size += (4 - (l_list_flat_size % 4));  // add number of bytes to make this a word boundary
        }
    }
    uint32_t total_size = (1 + l_list_size) * sizeof(uint32_t);
    total_size += l_list_flat_size;

    o_data.setByteLength(total_size);

    uint32_t word_offset = 0;
    o_data.setWord(word_offset, l_list_size);
    word_offset += 1;

    for (std::list<cipSoftwareEvent_t>::const_iterator entry = i_events.begin();
         entry != i_events.end();
         entry++)
    {
        uint32_t l_entry_flat_size = entry->flattenSize();
        o_data.setWord(word_offset, l_entry_flat_size);
        word_offset += 1;

        uint8_t * l_data = new uint8_t[l_entry_flat_size];
        entry->flatten(l_data, l_entry_flat_size);
        o_data.insert(l_data, word_offset * 32, l_entry_flat_size * 8);
        delete [] l_data;
        word_offset += l_entry_flat_size / sizeof(uint32_t);
        if ( l_entry_flat_size % 4 ) word_offset++;   // l_entry_flat_size is not word bound, needs to be, bump if we are not word bound.
    }
}
