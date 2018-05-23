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
#include <ControlInstruction.H>
#include <arpa/inet.h>
#include <iomanip>
#include <sstream>
#include <stdio.h>
#include <string.h>

#ifdef OTHER_USE
#include <OutputLite.H>
extern OutputLite out;
#else
#include <CronusData.H>
#endif

#ifndef SERVER_TYPE
  #define SERVER_TYPE SERVER_BMC
#endif

/*****************************************************************************/
/* ControlInstruction Implementation *****************************************/
/*****************************************************************************/
ControlInstruction::ControlInstruction(void) : Instruction(),
controls(NULL)
{
  version = 0x4;
  type = CONTROL;
  majorIstepNum = 0;
  minorIstepNum = 0;
  timeout = 0;
}

ControlInstruction::ControlInstruction(InstructionCommand i_command, uint32_t i_flags, const char * i_commandToRun) : Instruction(),
controls(NULL)
{
  version = 0x1;
  type = CONTROL;
  command = i_command;
  flags = i_flags;
  if(i_commandToRun != NULL) {
    commandToRun = i_commandToRun;
  }
  if (command == QUERYSP) {
    version = 0x4;
  }
}

ControlInstruction::ControlInstruction(ServerControls * i_controls) : Instruction(),
controls(i_controls)
{
  version = 0x4;
  type = CONTROL;
}

ControlInstruction::~ControlInstruction(void) {
}

uint32_t ControlInstruction::setup(InstructionCommand i_command, uint32_t i_flags, const char * i_commandToRun) {
  version = 0x1;
  command = i_command;
  flags = i_flags;
  if(i_commandToRun != NULL) {
    commandToRun = i_commandToRun;
  }
  if (command == QUERYSP) {
    version = 0x4;
  }
  return 0;
}

uint32_t ControlInstruction::setup(InstructionCommand i_command, uint32_t i_key, uint32_t i_flags, const char * i_contactInfo) {
  command = i_command;
  flags = i_flags;
  key = i_key;
  if(i_contactInfo != NULL) {
    contactInfo = i_contactInfo;
  }
  version = 0x1;
  return 0;
}

uint32_t ControlInstruction::setup(InstructionCommand i_command, uint32_t i_arg1 , uint32_t i_arg2, uint32_t i_arg3, uint32_t i_flags) {
  command = i_command;
  flags = i_flags;
  if (i_command == SNDISTEPMSG) {
    majorIstepNum = i_arg1;
    minorIstepNum = i_arg2;
    timeout = i_arg3;
    version = 0x4;
  }
  return 0;
}

uint32_t ControlInstruction::execute(ecmdDataBuffer & o_data, InstructionStatus & o_status, Handle ** io_handle) {
  uint32_t rc = 0;

  /* set the version of this instruction in the status */
  o_status.instructionVersion = version;

  /* check for any previous errors to report back */
  if (error) {
    rc = o_status.rc = error;
    return rc;
  }

  switch(command) {
    case INFO:
      {
        /* They want to know who we are */
        o_data.setWordLength(7);
        o_data.setWord(0, 0x1);
        o_data.setWord(1, SERVER_TYPE);
        o_data.setWord(2, 0x1FFFFFFF);
        o_data.setWord(3, 0x000000FF);
        o_data.setWord(4, 0x000000FF);
        o_data.setWord(5, 0x000000FF);
        uint32_t myFlags = 0x0;
        // FSP1 should support HW CRC properly now, so enable it
        myFlags |= SERVER_INFO_HW_CRC_SUPPORT;
        if ((controls != NULL) && (controls->global_multi_client_pointer != NULL) && (*(controls->global_multi_client_pointer)) == true ) {
          myFlags |= SERVER_INFO_MULTI_CLIENT;
        }
        if ((controls != NULL) && (controls->global_auth_pointer != NULL) && (controls->global_auth_pointer->enabled)) {
          myFlags |= SERVER_INFO_AUTH_NEEDED;
        }
        o_data.setWord(6, myFlags);
        rc = o_status.rc = SERVER_COMMAND_COMPLETE;
      }
      break;

    case RUN_CMD:

      {
        if (commandToRun.size() == 0) {
          rc = o_status.rc = SERVER_INVALID_COMMAND_BLOCK_FIELD_COMMPTR;
          break;
        }
        const int maxbuf = 1024 * 1024;
        char buf[maxbuf];
        memset(buf, 0x0, maxbuf);

        FILE * lFilePtr = popen(commandToRun.c_str(), "r") ;

        char lChar;
        char compareChar = -1;
        int idx = 0;
        // read the output from ls
        while(compareChar != (lChar = getc(lFilePtr)))
        {
          buf[idx++] = lChar;
          if (idx > (maxbuf - 2)) {
            rc = o_status.rc = SERVER_COMMAND_BUFFER_OVERFLOW;
            break;
          }
        }
        uint32_t exit_status = pclose(lFilePtr);
        exit_status = WEXITSTATUS(exit_status);

        o_status.data.setBitLength(32);
        o_status.data.insert(exit_status, 0, 32);
       
        /* Now let's copy this into our return structure */
        if(idx > 0) {
          o_data.setWordLength(((strlen(buf) + 1) / sizeof(uint32_t)) + 1);
          rc = o_status.rc = o_data.memCopyIn((uint8_t *) buf, strlen(buf) + 1);
        }
        if(!rc) {
          rc = o_status.rc = SERVER_COMMAND_COMPLETE;
        }
      }
      break;

    case SNDISTEPMSG:

      // FIXME implement
      rc = o_status.rc = SERVER_COMMAND_COMPLETE;
      break;

    case AUTH:
      {
        if ((controls != NULL) && (controls->global_auth_pointer != NULL) && (controls->global_auth_pointer->enabled)) {
          if (controls->global_auth_pointer->keyMap.count(key) != 0) {
            /* authorization valid */
            *(controls->threadKeyValid_pointer) = true;
            *(controls->threadKey_pointer) = key;
            rc = o_status.rc = SERVER_COMMAND_COMPLETE;
          } else {
            /* authorization invalid */
            rc = o_status.rc = SERVER_AUTHORIZATION_INVALID;
            o_status.errorMessage = std::string("Server is locked. Invalid authorization. Contact : ") + controls->global_auth_pointer->keyMap[controls->global_auth_pointer->firstKey];
          }
        } else {
          /* authorization was not needed */
          rc = o_status.rc = SERVER_COMMAND_COMPLETE;
        }
      }
      break;

    case ADDAUTH:
      {
        if ((controls != NULL) && (controls->global_auth_pointer != NULL)) {
          if (controls->global_auth_pointer->enabled == false) {
            controls->global_auth_pointer->firstKey = key;
          }
          controls->global_auth_pointer->enabled = true;
          controls->global_auth_pointer->keyMap[key] = contactInfo;
          *(controls->threadKeyValid_pointer) = true;
          *(controls->threadKey_pointer) = key;
          rc = o_status.rc = SERVER_COMMAND_COMPLETE;
        }
      }
      break;

    case CLEARAUTH:
      {
        if ((controls != NULL) && (controls->global_auth_pointer != NULL)) {
          controls->global_auth_pointer->enabled = false;
          controls->global_auth_pointer->keyMap.clear();
          controls->global_auth_pointer->firstKey = 0;
          *(controls->threadKeyValid_pointer) = false;
          *(controls->threadKey_pointer) = 0x0;
          rc = o_status.rc = SERVER_COMMAND_COMPLETE;
        }
      }
      break;

    case VERSION:
      {
        std::ostringstream oss;
        oss << "Server Version" << std::endl;
        oss << "Date: " << __DATE__ << " " << __TIME__ << std::endl;
        std::map<std::string, uint32_t>::iterator versionIterator = controls->global_version_map_pointer->begin();
        while (versionIterator != controls->global_version_map_pointer->end()) {
          oss << std::left << std::setw(24) << versionIterator->first << " : " << versionIterator->second << std::endl;
          versionIterator++;
        }
        o_data.setWordLength(((oss.str().length() + 1) / sizeof(uint32_t)) + 1);
        o_data.memCopyIn((uint8_t *) oss.str().c_str(), oss.str().length() + 1);
        rc = o_status.rc = SERVER_COMMAND_COMPLETE;
      }
      break;

    case FLIGHTRECORDER:

      {
        std::ostringstream oss;
        oss << "Flight Recorder" << std::endl;

        std::list<FlightRecorderEntry >::iterator frIterator = controls->global_flight_recorder_pointer->begin();
        while (frIterator != controls->global_flight_recorder_pointer->end()) {
          oss << std::left << std::setw(16) << InstructionTypeToString(frIterator->type) << " : "
              << std::left << std::setw(16) << InstructionCommandToString(frIterator->command) << " : " << frIterator->vars << std::endl;
          frIterator++;
        }

        o_data.setWordLength(((oss.str().length() + 1) / sizeof(uint32_t)) + 1);
        o_data.memCopyIn((uint8_t *) oss.str().c_str(), oss.str().length() + 1);

        rc = o_status.rc = SERVER_COMMAND_COMPLETE;
      }
      break;

    case EXIT:
      if ((controls != NULL) && (controls->global_exit_pointer != NULL)) {
        *(controls->global_exit_pointer) = true;
      }
      rc = o_status.rc = SERVER_COMMAND_COMPLETE;
      break;

    case QUERYSP:
      {
        if (commandToRun.size() == 0) {
          rc = o_status.rc = SERVER_INVALID_COMMAND_BLOCK_FIELD_COMMPTR;
          break;
        }
        if (commandToRun == "LIST") {
          o_data.insertFromAsciiAndResize("LIST SPTYPE");
          rc = o_status.rc = SERVER_COMMAND_COMPLETE;
          break;
        } else if (commandToRun == "SPTYPE") {
          o_data.insertFromAsciiAndResize(serverTypeToString(SERVER_TYPE).c_str());
          rc = o_status.rc = SERVER_COMMAND_COMPLETE;
          break;
        } else {
          rc = o_status.rc = SERVER_CONTROL_UNKNOWN_QUERYSP;
        }
      }
      break;

    default:
      rc = o_status.rc = SERVER_COMMAND_NOT_SUPPORTED;
      break;
  }
  return rc;
}

uint32_t ControlInstruction::flatten(uint8_t * o_data, uint32_t i_len) const {
  uint32_t rc = 0;
  uint32_t * o_ptr = (uint32_t *) o_data;
  
  if (i_len < flattenSize()) {
    out.error("ControlInstruction::flatten", "i_len %d bytes is too small to flatten\n", i_len);
    rc = 1;
  } else {
    // clear memory
    memset(o_data, 0, flattenSize());
    o_ptr[0] = htonl(version);
    o_ptr[1] = htonl(command);
    o_ptr[2] = htonl(flags);
    if (command == RUN_CMD || command == QUERYSP) {
      if (commandToRun.size() > 0) {
        strcpy(((char *)(o_ptr + 3)), commandToRun.c_str());
      } else {
        memset(((char *)(o_ptr + 3)), 0x0, 1);
      }
    } else if (command == AUTH) {
      o_ptr[3] = htonl(key);
    } else if (command == ADDAUTH) {
      o_ptr[3] = htonl(key);
      if (contactInfo.size() > 0) {
        strcpy(((char *)(o_ptr + 4)), contactInfo.c_str());
      } else {
        memset(((char *)(o_ptr + 4)), 0x0, 1);
      }
    } else if (command == SNDISTEPMSG && version >= 0x4) {
      o_ptr[3] = htonl(majorIstepNum);
      o_ptr[4] = htonl(minorIstepNum);
      o_ptr[5] = htonl(timeout);
    }
  }
  return rc;
}

uint32_t ControlInstruction::unflatten(const uint8_t * i_data, uint32_t i_len) {
  uint32_t rc = 0;
  uint32_t * i_ptr = (uint32_t *) i_data;

  version = ntohl(i_ptr[0]);
  if(version == 0x1 || version == 0x2 || version == 0x3 || version == 0x4) {
    command = (InstructionCommand) ntohl(i_ptr[1]);
    flags = ntohl(i_ptr[2]);
    if (command == RUN_CMD || command == QUERYSP) {
      commandToRun = ((char *)(i_ptr + 3));
    } else if (command == AUTH) {
      key = ntohl(i_ptr[3]);
    } else if (command == ADDAUTH) {
      key = ntohl(i_ptr[3]);
      contactInfo = ((char *)(i_ptr + 4));
    } else if (command == SNDISTEPMSG && version >= 0x4) {
      majorIstepNum = ntohl(i_ptr[3]);
      minorIstepNum = ntohl(i_ptr[4]);
      timeout = ntohl(i_ptr[5]);
    }
  } else {
    error = rc = SERVER_UNKNOWN_INSTRUCTION_VERSION;
  }
  return rc;
}

uint32_t ControlInstruction::flattenSize(void) const {
  uint32_t size = 0;
  if (command == RUN_CMD || command == QUERYSP) {
    size = (3 * sizeof(uint32_t)) + commandToRun.size() + 1;
  } else if (command == AUTH) {
    size = (4 * sizeof(uint32_t));
  } else if (command == ADDAUTH) {
    size = (4 * sizeof(uint32_t)) + contactInfo.size() + 1;
  } else if (command == SNDISTEPMSG && version >= 0x4) {
    size = (6 * sizeof(uint32_t));
  } else {
    size = (3 * sizeof(uint32_t));
  }
  return size;
}

std::string ControlInstruction::dumpInstruction(void) const {
  std::ostringstream oss;
  oss << "ControlInstruction" << std::endl;
  oss << "version       : " << version << std::endl;
  oss << "command       : " << InstructionCommandToString(command) << std::endl;
  oss << "type          : " << InstructionTypeToString(type) << std::endl;
  oss << "flags         : " << InstructionFlagToString(flags) << std::endl;
  if (command == RUN_CMD || command == QUERYSP) {
    oss << "commandToRun  : " << commandToRun << std::endl;
  } else if (command == AUTH) {
    oss << "key           : " << std::hex << std::setw(8) << std::setfill('0') << key << std::dec << std::endl;
  } else if (command == ADDAUTH) {
    oss << "key           : " << std::hex << std::setw(8) << std::setfill('0') << key << std::dec << std::endl;
    oss << "contactInfo   : " << contactInfo << std::endl;
  } else if (command == SNDISTEPMSG && version >= 0x4) {
    oss << "majorIstepNum : " << std::dec << majorIstepNum << std::endl;
    oss << "minorIstepNum : " << std::dec << minorIstepNum << std::endl;
    oss << "timeout       : " << std::dec << timeout << std::endl;
  }

  return oss.str();
}

std::string ControlInstruction::dumpInstructionShort(void) const {
  std::ostringstream oss;

  if (command == RUN_CMD || command == QUERYSP) {
    oss << commandToRun;
  } else if (command == AUTH) {
    oss << std::hex << std::setw(8) << std::setfill('0') << key;
  } else if (command == ADDAUTH) {
    oss << std::hex << std::setw(8) << std::setfill('0') << key << ", ";
    oss << contactInfo;
  } else if (command == SNDISTEPMSG && version >= 0x4) {
    oss << std::dec << majorIstepNum << ", ";
    oss << std::dec << minorIstepNum << ", ";
    oss << std::dec << timeout;
  }

  return oss.str();
}

uint64_t ControlInstruction::getHash(void) const {
  uint32_t hash = 0x0;
  switch (command) {
    case INFO:
    case RUN_CMD:
    case QUERYSP:
    case SNDISTEPMSG:
    case VERSION:
    case FLIGHTRECORDER:
      hash = 0x0;
      break;
    default:
      hash |= ((0x0000000F & type) << 28);
      break;
  }
  return hash;
}
std::string ControlInstruction::getInstructionVars(const InstructionStatus & i_status) const {
  std::ostringstream oss;

  oss << std::hex << std::setfill('0');
  if (command == RUN_CMD || command == QUERYSP) {
    oss << "rc: " << std::setw(8) << i_status.rc;
    oss << " commandToRun: " << commandToRun;
  } else if (command == AUTH || command == ADDAUTH) {
    oss << "rc: " << std::setw(8) << i_status.rc;
    oss << " key: " << std::setw(8) << key;
  } else if (command == SNDISTEPMSG) {
    oss << "rc: " << std::setw(8) << i_status.rc;
    oss << " major: " << majorIstepNum << " minor: " << minorIstepNum << " timeout: " << timeout;
  } else {
    return Instruction::getInstructionVars(i_status);
  }

  return oss.str();
}

uint32_t ControlInstruction::populateTypeInfo(server_type_info & o_typeInfo, ecmdDataBuffer & i_data) {
  /* check version */
  if(i_data.getWord(0) == 0x1) {
    /* They gave us real data, let's get going */
    o_typeInfo.type = (SERVER_MACHINE_TYPE) i_data.getWord(1);
    o_typeInfo.tms_mask = i_data.getWord(2); o_typeInfo.tck_mask = i_data.getWord(3);
    o_typeInfo.tdi_mask = i_data.getWord(4); o_typeInfo.tdo_mask = i_data.getWord(5);
    o_typeInfo.flags = i_data.getWord(6);
  } else {
    out.error("ControlInstruction::populateTypeInfo","Unknown version of hw info\n");
    return 1;
  }
  return 0;
}

std::string ControlInstruction::serverTypeToString( SERVER_MACHINE_TYPE i_type )
{
    switch( i_type )
    {
        case(SERVER_CSP):
            return std::string("CSP");
        case(SERVER_BPC):
            return std::string("BPC");
        case(SERVER_FSP):
            return std::string("FSP");
        case(SERVER_SJM):
            return std::string("SJM");
        case(SERVER_PROXY):
            return std::string("PROXY");
        case(SERVER_SIM):
            return std::string("SIM");
        case(SERVER_LOFT):
            return std::string("LOFT");
        case(SERVER_6682TESTER):
            return std::string("6682TESTER");
        case(SERVER_SIMDISPATCHER):
            return std::string("SIMDISPATCHER");
        case(SERVER_UNDEFINED):
            return std::string("UNDEFINED");
        case(SERVER_ICON):
            return std::string("ICON");
        case(SERVER_FTDI):
            return std::string("FTDI");
        case(SERVER_GSD2PIB):
            return std::string("GSD2PIB");
        case(SERVER_D2C):
            return std::string("D2C");
        case(SERVER_BMC):
            return std::string("BMC");
        default:
            return std::string("UNKNOWN");
    }
    return std::string("UNKNOWN");
}
