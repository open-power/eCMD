//IBM_PROLOG_BEGIN_TAG
/* 
 * Copyright 2003,2017 IBM International Business Machines Corp.
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
#include <Instruction.H>

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <iomanip>
#include <errno.h>
#include <limits.h>
#include <iostream>

#ifdef OTHER_USE
#include <OutputLite.H>
extern OutputLite out;
#else
#include <CronusData.H>
#endif

std::string InstructionTypeToString(Instruction::InstructionType i_type) {
  switch (i_type) {
    case Instruction::NOINSTRUCTION:
      return "NOINSTRUCTION";
    case Instruction::FSI:
      return "FSI";
    case Instruction::JTAG:
      return "JTAG";
    case Instruction::MULTIPLEJTAG:
      return "MULTIPLEJTAG";
    case Instruction::PSI:
      return "PSI";
    case Instruction::GPIO:
      return "GPIO";
    case Instruction::I2C:
      return "I2C";
    case Instruction::VPD:
      return "VPD";
    case Instruction::DMA:
      return "DMA";
    case Instruction::CONTROL:
      return "CONTROL";
    case Instruction::DMAEXER:
      return "DMAEXER";
    case Instruction::POWR:
      return "POWR";
    case Instruction::FSISTREAM:
      return "FSISTREAM";
    case Instruction::SBEFIFO:
      return "SBEFIFO";
    case Instruction::GSD2PIB:
      return "GSD2PIB";
    case Instruction::FSIMEMPROC:
      return "FSIMEMPROC";
    case Instruction::PNOR:
      return "PNOR";
    case Instruction::D2C:
      return "D2C";
  }
  return "";
}

std::string InstructionCommandToString(Instruction::InstructionCommand i_command) {
  switch (i_command) {
    case Instruction::NOCOMMAND:
      return "NOCOMMAND";
    case Instruction::SENDCMD:
      return "SENDCMD";
    case Instruction::READ:
      return "READ";
    case Instruction::WRITE:
      return "WRITE";
    case Instruction::LONGIN:
      return "LONGIN";
    case Instruction::LONGOUT:
      return "LONGOUT";
    case Instruction::READ_WRITE:
      return "READ_WRITE";
    case Instruction::SHIFTOUT:
      return "SHIFTOUT";
    case Instruction::DMAIN:
      return "DMAIN";
    case Instruction::DMAOUT:
      return "DMAOUT";
    case Instruction::MISC:
      return "MISC";
    case Instruction::TOTAP:
      return "TOTAP";
    case Instruction::READVPD:
      return "READVPD";
    case Instruction::READSPMEM:
      return "READSPMEM";
    case Instruction::WRITESPMEM:
      return "WRITESPMEM";
    case Instruction::SCOMIN:
      return "SCOMIN";
    case Instruction::SCOMOUT:
      return "SCOMOUT";
    case Instruction::WRITEVPD:
      return "WRITEVPD";
    case Instruction::PSI_RESET:
      return "PSI_RESET";
    case Instruction::PSI_LINK_CALIB:
      return "PSI_LINK_CALIB";
    case Instruction::PSI_EI_REG_READ:
      return "PSI_EI_REG_READ";
    case Instruction::PSI_EI_REG_WRITE:
      return "PSI_EI_REG_WRITE";
    case Instruction::PSI_VERIFY:
      return "PSI_VERIFY";
    case Instruction::PSI_SCOM_READ:
      return "PSI_SCOM_READ";
    case Instruction::PSI_SCOM_WRITE:
      return "PSI_SCOM_WRITE";
    case Instruction::PSI_READ:
      return "PSI_READ";
    case Instruction::PSI_WRITE:
      return "PSI_WRITE";
    case Instruction::PSI_INIT:
      return "PSI_INIT";
    case Instruction::PSI_LINK_ENABLE:
      return "PSI_LINK_ENABLE";
    case Instruction::PSI_SET_SPEED:
      return "PSI_SET_SPEED";
    case Instruction::PSI_LINK_VERIFY:
      return "PSI_LINK_VERIFY";
    case Instruction::I2CWRITE:
      return "I2CWRITE";
    case Instruction::I2CREAD:
      return "I2CREAD";
    case Instruction::I2CRESETLIGHT:
      return "I2CRESETLIGHT";
    case Instruction::I2CRESETFULL:
      return "I2CRESETFULL";
    case Instruction::GPIO_CONFIGPIN:
      return "GPIO_CONFIGPIN";
    case Instruction::GPIO_READPIN:
      return "GPIO_READPIN";
    case Instruction::GPIO_READPINS:
      return "GPIO_READPINS";
    case Instruction::GPIO_READLATCH:
      return "GPIO_READLATCH";
    case Instruction::GPIO_WRITELATCH:
      return "GPIO_WRITELATCH";
    case Instruction::GPIO_WRITELATCHES:
      return "GPIO_WRITELATCHES";
    case Instruction::GPIO_READCONFIG:
      return "GPIO_READCONFIG";
    case Instruction::GPIO_WRITECONFIG:
      return "GPIO_WRITECONFIG";
    case Instruction::GPIO_WRITECNFGSET:
      return "GPIO_WRITECNFGSET";
    case Instruction::GPIO_WRITECNFGCLR:
      return "GPIO_WRITECNFGCLR";
    case Instruction::DMAEXER_START:
      return "DMAEXER_START";
    case Instruction::DMAEXER_REPORT:
      return "DMAEXER_REPORT";
    case Instruction::DMAEXER_STOP:
      return "DMAEXER_STOP";
    case Instruction::INFO:
      return "INFO";
    case Instruction::RUN_CMD:
      return "RUN_CMD";
    case Instruction::MULTI_ENABLE:
      return "MULTI_ENABLE";
    case Instruction::AUTH:
      return "AUTH";
    case Instruction::ADDAUTH:
      return "ADDAUTH";
    case Instruction::CLEARAUTH:
      return "CLEARAUTH";
    case Instruction::VERSION:
      return "VERSION";
    case Instruction::FLIGHTRECORDER:
      return "FLIGHTRECORDER";
    case Instruction::EXIT:
      return "EXIT";
    case Instruction::SCOMIN_MASK:
      return "SCOMIN_MASK";
    case Instruction::MASK_PERSISTENT:
      return "MASK_PERSISTENT";
    case Instruction::SET_PERSISTENT:
      return "SET_PERSISTENT";
    case Instruction::GET_PERSISTENT:
      return "GET_PERSISTENT";
    case Instruction::READKEYWORD:
      return "READKEYWORD";
    case Instruction::WRITEKEYWORD:
      return "WRITEKEYWORD";
    case Instruction::FRUSTATUS:
      return "FRUSTATUS";
    case Instruction::CHICDOIPL:
      return "CHICDOIPL";
    case Instruction::ENABLE_MEM_VOLTAGES:
      return "ENABLE_MEM_VOLTAGES";
    case Instruction::DISABLE_MEM_VOLTAGES:
      return "DISABLE_MEM_VOLTAGES";
    case Instruction::SNDISTEPMSG:
      return "SNDISTEPMSG";
    case Instruction::MBXTRACEENABLE:
      return "MBXTRACEENABLE";
    case Instruction::MBXTRACEDISABLE:
      return "MBXTRACEDISABLE";
    case Instruction::MBXTRACEREAD:
      return "MBXTRACEREAD";
    case Instruction::PUTMEMPBA:
      return "PUTMEMPBA";
    case Instruction::GETMEMPBA:
      return "GETMEMPBA";
    case Instruction::BULK_SCOMIN:
      return "BULK_SCOMIN";
    case Instruction::BULK_SCOMOUT:
      return "BULK_SCOMOUT";
    case Instruction::STREAM_SETUP:
      return "STREAM_SETUP";
    case Instruction::STREAM_FINISH:
      return "STREAM_FINISH";
    case Instruction::FSPDMAIN:
      return "FSPDMAIN";
    case Instruction::FSPDMAOUT:
      return "FSPDMAOUT";
    case Instruction::SUBMIT:
      return "SUBMIT";
    case Instruction::REQUEST_RESET:
      return "REQUEST_RESET";
    case Instruction::SEND_TAPI_CMD:
      return "SEND_TAPI_CMD";
    case Instruction::PUTMEMPROC:
      return "PUTMEMPROC";
    case Instruction::GETMEMPROC:
      return "GETMEMPROC";
    case Instruction::PNORGETLIST:
      return "PNORGETLIST";
    case Instruction::PNORGET:
      return "PNORGET";
    case Instruction::PNORPUT:
      return "PNORPUT";
    case Instruction::QUERYSP:
      return "QUERYSP";
    case Instruction::ADJUST_PROC_VOLTAGES:
      return "ADJUST_PROC_VOLTAGES";
    case Instruction::PSI_CMU_REG_READ:
      return "PSI_CMU_REG_READ";
    case Instruction::PSI_CMU_REG_WRITE:
      return "PSI_CMU_REG_WRITE";
  }
  return "";
}


/*****************************************************************************/
/* Instruction Implementation ************************************************/
/*****************************************************************************/
Instruction::Instruction(void) : version(0x1), command(NOCOMMAND), type(NOINSTRUCTION), error(0){
}

/* for all of these methods check if we are of a different type */
uint32_t Instruction::execute(ecmdDataBuffer & o_data, InstructionStatus & o_status, Handle ** io_handle) {
  uint32_t rc = 0;

  /* set the version of this instruction in the status */
  o_status.instructionVersion = version;

  o_status.errorMessage.append("Instruction::execute Unknown Instruction specified. New FSP server may be required.");
  o_status.rc = SERVER_UNKNOWN_INSTRUCTION;

  return rc;
}

uint32_t Instruction::decrementVersion(void) {
  if (version > 0x1) {
    version--;
    return 0;
  }
  // return an error if we can not decrement
  return 1;
}

uint32_t Instruction::flatten(uint8_t * o_data, uint32_t i_len) const {
  uint32_t rc = 0;

  uint32_t * o_ptr = (uint32_t *) o_data;
  if (i_len < flattenSize()) {
    out.error("Instruction::flatten", "i_len %d bytes is too small to flatten\n", i_len);
    rc = 1;
  } else {
    // clear memory
    memset(o_data, 0, flattenSize());
    o_ptr[0] = htonl(version);
    o_ptr[1] = htonl(command);
    o_ptr[2] = htonl(flags);
  }

  return rc;
}

uint32_t Instruction::unflatten(const uint8_t * i_data, uint32_t i_len) {
  uint32_t rc = 0;

  uint32_t * i_ptr = (uint32_t *) i_data;
  version = ntohl(i_ptr[0]);
  if(version == 0x1) {
    command = (InstructionCommand) ntohl(i_ptr[1]);
    flags = ntohl(i_ptr[2]);
  } else {
    error = rc = SERVER_UNKNOWN_INSTRUCTION_VERSION;
  }

  return rc;
}

uint32_t Instruction::flattenSize(void) const {
  return (3 * sizeof(uint32_t));
}

std::string Instruction::dumpInstruction(void) const {
  std::ostringstream oss;

  oss << "Instruction" << std::endl;

  return oss.str();
}

uint64_t Instruction::getHash(void) const {
  return ((0x0000000F & type) << 28);
}

uint32_t Instruction::closeHandle(Handle ** i_handle) {
  uint32_t rc = 0;

  *i_handle = NULL;

  return rc;
}

std::string Instruction::getInstructionVars(const InstructionStatus & i_status) const {
  std::ostringstream oss;

  oss << std::hex << std::setfill('0');
  oss << "rc: " << std::setw(8) << i_status.rc;

  return oss.str();
}

Instruction::InstructionType Instruction::getType(void) const { return type; }
Instruction::InstructionCommand Instruction::getCommand(void) const { return command; }
uint32_t Instruction::getFlags(void) const { return flags; }
uint32_t Instruction::getVersion(void) const { return version; }

int Instruction::genWords(const ecmdDataBuffer &data, std::string &words) const {
  // copied from Debug.C
  int rc = 0;

  if (data.getBitLength() == 0) {
    words = "";  // This will kill the genHexLeftStr error if we have no data 
  } else if (data.getBitLength() <= 128) {
    words = data.genHexLeftStr(0, data.getBitLength());
  } else {
    words = data.genHexLeftStr(0, 128);
  }

  return rc;
}

std::ostream& operator<<(std::ostream& io_stream, const Instruction& i_instruction ) {
  io_stream << i_instruction.dumpInstruction() << std::endl;
  return io_stream;
}

uint32_t devicestring_genhash(const std::string & i_string, uint32_t & o_hash)
{
    uint32_t rc = 0;
    int length = i_string.length();
    const char * data = i_string.c_str();
    int found = 0;
    int type = 0;       // 3 bits
    int fsi = 0;        // 6 bits
    int cfam = 0;       // 2 bits
    int engine = 0;     // 5 bits
    int subfsi = 0;     // 3 bits
    int subcfam = 0;    // 2 bits
    int subengine = 0;  // 5 bits
    int subsubfsi = 0;  // 3 bits
    int subsubcfam = 0; // 2 bits
    o_hash = 0x0;
    if (length == 5)
    {
        type = 1;
        found = sscanf(data, "L%02dC%d", &fsi, &cfam);
        if (found != 2)
        {
            rc = 1;
        }
    }
    else if (length == 13)
    {
        type = 2;
        found = sscanf(data, "L%02dC%dE%02d:L%dC%d", &fsi, &cfam, &engine, &subfsi, &subcfam);
        if (found != 5)
        {
            rc = 2;
        }
    }
    else if (length == 21)
    {
        type = 3;
        found = sscanf(data, "L%02dC%dE%02d:L%dC%dE%02d:L%dC%d", &fsi, &cfam, &engine, &subfsi, &subcfam,
                       &subengine, &subsubfsi, &subsubcfam);
        if (found != 8)
        {
            rc = 3;
        }
    }
    else
    {
        // BMC device case
        char * endptr = NULL;
        errno = 0;
        fsi = strtol(data, &endptr, 10);
        if (((errno == ERANGE) && ((fsi == LONG_MAX) || (fsi == LONG_MIN))) ||
            ((errno != 0) && (fsi == 0)))
        {
            rc = 5;
        }
        else if (endptr == data)
        {
            rc = 6;
        }
        else if (*endptr != '\0')
        {
            rc = 4;
        }
    }

    if (rc)
    {
        // error
        std::cout << "ERROR unknown format for device " << i_string << " rc = " << rc << std::endl;
    }
    else
    {
        o_hash |= ((0x07 & type)    << 0);      //0x00000007
        o_hash |= ((0x3F & fsi)     << 3);      //0x000001F8
        o_hash |= ((0x03 & cfam)    << 9);      //0x00000600
        o_hash |= ((0x1F & engine)  << 11);     //0x0000F800
        o_hash |= ((0x07 & subfsi)  << 16);     //0x00070000
        o_hash |= ((0x03 & subcfam)     << 19); //0x00180000
        o_hash |= ((0x1F & subengine)   << 21); //0x03E00000
        o_hash |= ((0x07 & subsubfsi)   << 26); //0x1C000000
        o_hash |= ((0x03 & subsubcfam)  << 29); //0x60000000
    }
    return rc;
}
