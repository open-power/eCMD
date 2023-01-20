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
#include <FSIInstruction.H>
#include <arpa/inet.h>
#include <stdio.h>
#include <sstream>
#include <iomanip>
#include <string.h>
#include <errno.h>

#ifdef OTHER_USE
#include <OutputLite.H>
extern OutputLite out;
#endif

/*****************************************************************************/
/* FSIInstruction Implementation *********************************************/
/*****************************************************************************/
FSIInstruction::FSIInstruction(void) : Instruction(),
cfamid(0),
linkid(0),
cmaster(0),
address(0),
address64(0),
length(0),
deviceString("")
{
  version = 0x6;
  type = FSI;
}

FSIInstruction::FSIInstruction(InstructionCommand i_command, uint32_t i_cfamid, uint32_t i_linkid, uint32_t i_cmaster, uint64_t i_address, uint32_t i_length, uint32_t i_flags, ecmdDataBuffer * i_data, ecmdDataBuffer * i_mask) : Instruction(),
cfamid(i_cfamid),
linkid(i_linkid),
cmaster(i_cmaster),
address(0),
address64(0),
length(i_length),
deviceString("")
{
  version = 0x4; // revert to version 4 if no deviceString
  type = FSI;
  command = i_command;
  flags = i_flags;
  if (((i_address & 0xFFFFFFFF00000000ull) != 0x0ull) && ((command == SCOMIN) || (command == SCOMOUT) || (command == SCOMIN_MASK) || (command == BULK_SCOMIN) || (command == BULK_SCOMOUT))) {
    address64 = i_address;
    flags |= INSTRUCTION_FLAG_64BIT_ADDRESS;
  } else {
    address = (uint32_t) i_address;
    version = 0x2; // revert to version 2 for 32-bit addresses
  }
  if(i_data != NULL) {
    i_data->shareBuffer(&data);
  }
  if(i_mask != NULL) {
    i_mask->shareBuffer(&mask);
  }
  if ((command == BULK_SCOMIN) || (command == BULK_SCOMOUT)) {
    version = 0x6;
  }
}

FSIInstruction::~FSIInstruction(void) {
}

uint32_t FSIInstruction::setup(InstructionCommand i_command, uint32_t i_cfamid, uint32_t i_linkid, uint32_t i_cmaster, uint64_t i_address, uint32_t i_length, uint32_t i_flags, ecmdDataBuffer * i_data, ecmdDataBuffer * i_mask) {
  version = 0x4; // revert to version 4 if no deviceString
  cfamid = i_cfamid;
  linkid = i_linkid;
  cmaster = i_cmaster;
  length = i_length;
  command = i_command;
  flags = i_flags;
  if (((i_address & 0xFFFFFFFF00000000ull) != 0x0ull) && ((command == SCOMIN) || (command == SCOMOUT) || (command == SCOMIN_MASK) || (command == BULK_SCOMIN) || (command == BULK_SCOMOUT))) {
    address64 = i_address;
    flags |= INSTRUCTION_FLAG_64BIT_ADDRESS;
  } else {
    address = (uint32_t) i_address;
    version = 0x2; // revert to version 2 for 32-bit addresses
  }
  if(i_data != NULL) {
    i_data->shareBuffer(&data);
  }
  if(i_mask != NULL) {
    i_mask->shareBuffer(&mask);
  }
  if ((command == BULK_SCOMIN) || (command == BULK_SCOMOUT)) {
    version = 0x6;
  }
  return 0;
}

uint32_t FSIInstruction::setup(InstructionCommand i_command, std::string &i_deviceString, uint64_t i_address, uint32_t i_length, uint32_t i_flags, ecmdDataBuffer * i_data, ecmdDataBuffer * i_mask) {
  deviceString = i_deviceString;
  length = i_length;
  command = i_command;
  flags = i_flags | INSTRUCTION_FLAG_DEVSTR;
  if (((i_address & 0xFFFFFFFF00000000ull) != 0x0ull) && ((command == SCOMIN) || (command == SCOMOUT) || (command == SCOMIN_MASK) || (command == BULK_SCOMIN) || (command == BULK_SCOMOUT))) {
    address64 = i_address;
    flags |= INSTRUCTION_FLAG_64BIT_ADDRESS;
  } else {
    address = (uint32_t) i_address;
  }
  if(i_data != NULL) {
    i_data->shareBuffer(&data);
  }
  if(i_mask != NULL) {
    i_mask->shareBuffer(&mask);
  }
  if ((command != BULK_SCOMIN) && (command != BULK_SCOMOUT)) {
    version = 0x5; // revert to version 5 if not bulk scom
  }
  return 0;
}


uint32_t FSIInstruction::execute(ecmdDataBuffer & o_data, InstructionStatus & o_status, Handle ** io_handle) {
  int rc = 0;

  /* set the version of this instruction in the status */
  o_status.instructionVersion = version;

  /* check for any previous errors to report back */
  if (error) {
    rc = o_status.rc = error;
    return rc;
  }

  switch(command) {
    case LONGIN:
      {
        ssize_t bytelen = 0;
        char errstr[200];

        if (length <= 0) {
          rc = o_status.rc = SERVER_INVALID_COMMAND_BLOCK_FIELD_DATALEN;
          break;
        }

        /* Open the Handle */
        rc = scan_open(io_handle, o_status);
        if (rc) {
          o_status.rc = rc;
          break;
        }

        bytelen = length % 8 ? (length / 8) + 1 : length / 8;

        /* Actually write to the device */
        errno = 0;

        if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
          std::string words;
          genWords(data, words);
          snprintf(errstr, 200, "SERVER_DEBUG : scan_write() data = %s, address = 0x%08X, length = %u, flags =  0x%08X\n", words.c_str(), address, length, flags);
          o_status.errorMessage.append(errstr);
        }

        rc = scan_write(*io_handle, o_status);

        if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
          snprintf(errstr, 200, "SERVER_DEBUG : scan_write() rc = %u\n", rc);
          o_status.errorMessage.append(errstr);
        }

        if (rc != bytelen) {
          snprintf(errstr, 200, "FSIInstruction::execute(LONGIN) Write length exp (%zd) actual (%d)\n",bytelen, rc);
          o_status.errorMessage.append(errstr);
          snprintf(errstr, 200, "FSIInstruction::execute(LONGIN) Problem writing to FSI device : errno %d\n", errno);
          o_status.errorMessage.append(errstr);
          rc = o_status.rc = SERVER_FSI_SCAN_WRITE_FAIL;
          scan_ffdc_and_reset(io_handle, o_status);
          break;
        } else {
          rc = o_status.rc = SERVER_COMMAND_COMPLETE;
        }
      }
      break;
    case LONGOUT:
      {
        ssize_t bytelen = 0;
        char errstr[200];

        if (length <= 0) {
          rc = o_status.rc = SERVER_INVALID_COMMAND_BLOCK_FIELD_DATALEN;
          break;
        }

        /* Open the Handle */
        rc = scan_open(io_handle, o_status);
        if (rc) {
          o_status.rc = rc;
          break;
        }

        bytelen = length % 8 ? (length / 8) + 1 : length / 8;

        /* Actually read from the device */
        errno = 0;
        o_data.setBitLength(length);

        if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
          snprintf(errstr, 200, "SERVER_DEBUG : scan_read() address = 0x%08X, length = %u, flags = 0x%08X\n", address, length, flags);
          o_status.errorMessage.append(errstr);
        }
          
        rc = scan_read(*io_handle, o_data, o_status);

        if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
          std::string words;
          genWords(o_data, words);
          snprintf(errstr, 200, "SERVER_DEBUG : scan_read() o_data = %s, rc = %u\n", words.c_str(), rc);
          o_status.errorMessage.append(errstr);
        }

        if (rc != bytelen) {
          snprintf(errstr, 200, "FSIInstruction::execute(LONGOUT) Read length exp (%zd) actual (%d)\n", bytelen, rc);
          o_status.errorMessage.append(errstr);
          snprintf(errstr, 200, "FSIInstruction::execute(LONGOUT) Problem reading from FSI device : errno %d\n", errno);
          o_status.errorMessage.append(errstr);
          rc = o_status.rc = SERVER_FSI_SCAN_READ_FAIL;
          scan_ffdc_and_reset(io_handle, o_status);
          break;
        } else {
          rc = o_status.rc = SERVER_COMMAND_COMPLETE;
        }
      }
      break;
    case SCOMIN:
      {
        int bytelen = 0;
        char errstr[200];

        if (length <= 0) {
          rc = o_status.rc = SERVER_INVALID_COMMAND_BLOCK_FIELD_DATALEN;
          break;
        }

        /* Open the Handle */
        rc = scom_open(io_handle, o_status);
        if (rc) {
          o_status.rc = rc;
          break;
        }

        bytelen = length % 8 ? (length / 8) + 1 : length / 8;

        /* Actually write to the device */
        errno = 0;

        if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
          std::string words;
          genWords(data, words);
          if (flags & INSTRUCTION_FLAG_64BIT_ADDRESS) {
            snprintf(errstr, 200, "SERVER_DEBUG : scom_write() data = %s, address64 = 0x" UINT64_HEX_VARIN_FORMAT(%016) "\n", words.c_str(), address64);
          } else {
            snprintf(errstr, 200, "SERVER_DEBUG : scom_write(*io_handle, data = %s, address = 0x%08X\n", words.c_str(), address);
          }
          o_status.errorMessage.append(errstr);
        }

        rc = scom_write(*io_handle, o_status);

        if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
          snprintf(errstr, 200, "SERVER_DEBUG : scom_write() rc = %u\n", rc);
          o_status.errorMessage.append(errstr);
        }

        if (rc != bytelen) {
          snprintf(errstr, 200, "FSIInstruction::execute(SCOMIN) Write length exp (%d) actual (%d)\n",bytelen, rc);
          o_status.errorMessage.append(errstr);
          snprintf(errstr, 200, "FSIInstruction::execute(SCOMIN) Problem writing to FSI device : errno %d\n", errno);
          o_status.errorMessage.append(errstr);
          rc = o_status.rc = SERVER_FSI_SCOM_WRITE_FAIL;
          scom_ffdc_and_reset(io_handle, o_status);
          break;
        } else {
          rc = o_status.rc = SERVER_COMMAND_COMPLETE;
        }
      }
      break;
    case SCOMOUT:
      {
        int bytelen = 0;
        char errstr[200];

        if (length <= 0) {
          rc = o_status.rc = SERVER_INVALID_COMMAND_BLOCK_FIELD_DATALEN;
          break;
        }

        /* Open the Handle */
        rc = scom_open(io_handle, o_status);
        if (rc) {
          o_status.rc = rc;
          break;
        }

        /* Build up our buffer, we need to tack the header at the beginning */
        bytelen = length % 8 ? (length / 8) + 1 : length / 8;

        /* Actually read from the device */
        errno = 0;
        o_data.setBitLength(length);

        if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
          if (flags & INSTRUCTION_FLAG_64BIT_ADDRESS) {
            snprintf(errstr, 200, "SERVER_DEBUG : scom_read() address64 = 0x" UINT64_HEX_VARIN_FORMAT(%016) "\n", address64);
          } else {
            snprintf(errstr, 200, "SERVER_DEBUG : scom_read() address = 0x%08X\n", address);
          }
          o_status.errorMessage.append(errstr);
        }

        rc = scom_read(*io_handle, o_data, o_status);

        if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
          std::string words;
          genWords(o_data, words);
          snprintf(errstr, 200, "SERVER_DEBUG : scom_read() o_data = %s, rc = %u\n", words.c_str(), rc);
          o_status.errorMessage.append(errstr);
        }

        if (rc != bytelen) {
          snprintf(errstr, 200, "FSIInstruction::execute(SCOMOUT) Read length exp (%d) actual (%d)\n",bytelen, rc);
          o_status.errorMessage.append(errstr);
          snprintf(errstr, 200, "FSIInstruction::execute(SCOMOUT) Problem reading from FSI device : errno %d\n", errno);
          o_status.errorMessage.append(errstr);
          rc = o_status.rc = SERVER_FSI_SCOM_READ_FAIL;
          scom_ffdc_and_reset(io_handle, o_status);
          break;
        } else {
          rc = o_status.rc = SERVER_COMMAND_COMPLETE;
        }
      }
      break;
    case READSPMEM:
      {
        ssize_t len = 0;
        char errstr[200];

        if (length != 32) {
          rc = o_status.rc = SERVER_INVALID_COMMAND_BLOCK_FIELD_DATALEN;
          break;
        }

        
        CFAMType l_type = getCFAMType(address, flags);

        if (l_type == CFAM_TYPE_SCAN)
        {
          /* Open the Handle */
          rc = scan_open(io_handle, o_status);
          if (rc) {
            o_status.rc = rc;
            break;
          }

          errno = 0;
          o_data.setBitLength(32);

          if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
            snprintf(errstr, 200, "SERVER_DEBUG : scan_get_register() address = 0x%08X\n", address);
            o_status.errorMessage.append(errstr);
          }

          len = scan_get_register(*io_handle, o_data, o_status);

          if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
            std::string words;
            genWords(o_data, words);
            snprintf(errstr, 200, "SERVER_DEBUG : scan_get_register() o_data = %s, rc = %zd\n", words.c_str(), len);
            o_status.errorMessage.append(errstr);
          }

          if (len != 4 && len != 0) {
            snprintf(errstr, 200, "FSIInstruction::execute(READSPMEM) Read length exp (%d) actual (%zd)\n", 4, len);
            o_status.errorMessage.append(errstr);
            snprintf(errstr, 200, "FSIInstruction::execute(READSPMEM) Problem reading from FSI device : errno %d\n", errno);
            o_status.errorMessage.append(errstr);
            rc = o_status.rc = SERVER_FSI_SCAN_READ_FAIL;
            scan_ffdc_and_reset(io_handle, o_status);
            break;
          } else {
            rc = o_status.rc = SERVER_COMMAND_COMPLETE;
          }

        }
        else if (l_type == CFAM_TYPE_SCOM)
        {

          /* Open the Handle */
          rc = fsi_open(io_handle, o_status);
          if (rc) {
            o_status.rc = rc;
            break;
          }

          errno = 0;
          o_data.setBitLength(32);

          if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
            snprintf(errstr, 200, "SERVER_DEBUG : scom_get_register() address = 0x%08X\n", address);
            o_status.errorMessage.append(errstr);
          }

          len = scom_get_register(*io_handle, o_data, o_status);

          if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
            std::string words;
            genWords(o_data, words);
            snprintf(errstr, 200, "SERVER_DEBUG : scom_get_register() o_data = %s, rc = %zd\n", words.c_str(), len);
            o_status.errorMessage.append(errstr);
          }

          if (len != 4 && len != 0) {
            snprintf(errstr, 200, "FSIInstruction::execute(READSPMEM) Read length exp (%d) actual (%zd)\n", 4, len);
            o_status.errorMessage.append(errstr);
            snprintf(errstr, 200, "FSIInstruction::execute(READSPMEM) Problem reading from FSI device : errno %d\n", errno);
            o_status.errorMessage.append(errstr);
            rc = o_status.rc = SERVER_FSI_SCOM_READ_FAIL;
            fsi_close(*io_handle);
          } else {
            rc = o_status.rc = SERVER_COMMAND_COMPLETE;
          }

        }
        else if (l_type == CFAM_TYPE_MBX_SCRATCH)
        {

          /* Open the Handle */
          rc = fsi_open(io_handle, o_status);
          if (rc) {
            o_status.rc = rc;
            break;
          }

          errno = 0;
          o_data.setBitLength(32);

          if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
            snprintf(errstr, 200, "SERVER_DEBUG : mbx_get_scratch_register() address = 0x%08X\n", address);
            o_status.errorMessage.append(errstr);
          }

          len = mbx_get_scratch_register(*io_handle, o_data, o_status);

          if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
            std::string words;
            genWords(o_data, words);
            snprintf(errstr, 200, "SERVER_DEBUG : mbx_get_scratch_register() o_data = %s, rc = %zd\n", words.c_str(), len);
            o_status.errorMessage.append(errstr);
          }

          if (len != 0) {
            snprintf(errstr, 200, "FSIInstruction::execute(READSPMEM) Problem reading from FSI device : errno %d\n", errno);
            o_status.errorMessage.append(errstr);
            rc = o_status.rc = SERVER_FSI_SCOM_READ_FAIL;
            fsi_close(*io_handle);
          } else {
            rc = o_status.rc = SERVER_COMMAND_COMPLETE;
          }

        }
        else if (l_type == CFAM_TYPE_MBX)
        {

          /* Open the Handle */
          rc = fsi_open(io_handle, o_status);
          if (rc) {
            o_status.rc = rc;
            break;
          }

          errno = 0;
          o_data.setBitLength(32);

          if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
            snprintf(errstr, 200, "SERVER_DEBUG : mbx_get_register() address = 0x%08X\n", address);
            o_status.errorMessage.append(errstr);
          }

          len = mbx_get_register(*io_handle, o_data, o_status);

          if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
            std::string words;
            genWords(o_data, words);
            snprintf(errstr, 200, "SERVER_DEBUG : mbx_get_register() o_data = %s, rc = %zd\n", words.c_str(), len);
            o_status.errorMessage.append(errstr);
          }

          if (len != 4 && len != 0) {
            snprintf(errstr, 200, "FSIInstruction::execute(READSPMEM) Read length exp (%d) actual (%zd)\n", 4, len);
            o_status.errorMessage.append(errstr);
            snprintf(errstr, 200, "FSIInstruction::execute(READSPMEM) Problem reading from FSI device : errno %d\n", errno);
            o_status.errorMessage.append(errstr);
            rc = o_status.rc = SERVER_FSI_SCOM_READ_FAIL;
            fsi_close(*io_handle);
          } else {
            rc = o_status.rc = SERVER_COMMAND_COMPLETE;
          }

        }
        else if (l_type == CFAM_TYPE_GP_REG)
        {

          /* Open the Handle */
          rc = fsi_open(io_handle, o_status);
          if (rc) {
            o_status.rc = rc;
            break;
          }

          errno = 0;
          o_data.setBitLength(32);

          if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
            snprintf(errstr, 200, "SERVER_DEBUG : gp_reg_get() address = 0x%08X\n", address);
            o_status.errorMessage.append(errstr);
          }

          len = gp_reg_get(*io_handle, o_data, o_status);

          if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
            std::string words;
            genWords(o_data, words);
            snprintf(errstr, 200, "SERVER_DEBUG : gp_reg_get() o_data = %s, rc = %zd\n", words.c_str(), len);
            o_status.errorMessage.append(errstr);
          }

          if (len != 0) {
            snprintf(errstr, 200, "FSIInstruction::execute(READSPMEM) Problem reading from FSI device : errno %d\n", errno);
            o_status.errorMessage.append(errstr);
            rc = o_status.rc = SERVER_FSI_SCOM_READ_FAIL;
            fsi_close(*io_handle);
          } else {
            rc = o_status.rc = SERVER_COMMAND_COMPLETE;
          }

        }
        else if (l_type == CFAM_TYPE_SBEFIFO)
        {
          /* Open the Handle */
          rc = fsi_open(io_handle, o_status);
          if (rc) {
            o_status.rc = rc;
            break;
          }

          errno = 0;
          o_data.setBitLength(32);

          if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
            snprintf(errstr, 200, "SERVER_DEBUG : sbefifo_get_register() address = 0x%08X\n", address);
            o_status.errorMessage.append(errstr);
          }

          len = sbefifo_get_register(*io_handle, o_data, o_status);

          if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
            std::string words;
            genWords(o_data, words);
            snprintf(errstr, 200, "SERVER_DEBUG : sbefifo_get_register() o_data = %s, rc = %zd\n", words.c_str(), len);
            o_status.errorMessage.append(errstr);
          }

          if (len != 4 && len != 0) {
            snprintf(errstr, 200, "FSIInstruction::execute(READSPMEM) Read length exp (%d) actual (%zd)\n", 4, len);
            o_status.errorMessage.append(errstr);
            snprintf(errstr, 200, "FSIInstruction::execute(READSPMEM) Problem reading from FSI device : errno %d\n", errno);
            o_status.errorMessage.append(errstr);
            rc = o_status.rc = SERVER_FSI_SBEFIFO_READ_FAIL;
            fsi_close(*io_handle);
          } else {
            rc = o_status.rc = SERVER_COMMAND_COMPLETE;
          }
        }

        /* Debug interfaces */
        else if (flags & INSTRUCTION_FLAG_FSI_USE_DRA) {
          /* The register isn't part of the officially supported registers, so we are going into the back door here */
          rc = o_status.rc = SERVER_FSI_HW_DRA_NEEDED;
        } else {
          rc = o_status.rc = SERVER_FSI_REGISTER_NOT_SUPPORTED;
        }
      }
      break;
    case WRITESPMEM:
      {
        ssize_t len = 0;
        char errstr[200];
        uint32_t localData = 0;

        if (length != 32) {
          rc = o_status.rc = SERVER_INVALID_COMMAND_BLOCK_FIELD_DATALEN;
          break;
        }

        rc = data.extract(&localData, 0, length);
        if (rc) {
          o_status.rc = rc;
          break;
        }

        CFAMType l_type = getCFAMType(address, flags);

        if (l_type == CFAM_TYPE_SCAN)
        {

          /* Open the Handle */
          rc = scan_open(io_handle, o_status);
          if (rc) {
            o_status.rc = rc;
            break;
          }

          errno = 0;

          if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
            snprintf(errstr, 200, "SERVER_DEBUG : scan_set_register() address = 0x%08X data = %08X)\n", address, localData);
            o_status.errorMessage.append(errstr);
          }

          len = scan_set_register(*io_handle, o_status);

          if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
            snprintf(errstr, 200, "SERVER_DEBUG : scan_set_register() rc = %zd\n", len);
            o_status.errorMessage.append(errstr);
          }

          if (len != 4 && len != 0) {
            snprintf(errstr, 200, "FSIInstruction::execute(WRITESPMEM) Write length exp (%d) actual (%zd)\n", 4, len);
            o_status.errorMessage.append(errstr);
            snprintf(errstr, 200, "FSIInstruction::execute(WRITESPMEM) Problem writing to FSI device : errno %d\n", errno);
            o_status.errorMessage.append(errstr);
            rc = o_status.rc = SERVER_FSI_SCAN_WRITE_FAIL;
            scan_ffdc_and_reset(io_handle, o_status);
          } else {
            rc = o_status.rc = SERVER_COMMAND_COMPLETE;
          }

        }
        else if (l_type == CFAM_TYPE_SCOM)
        {

          /* Open the Handle */
          rc = fsi_open(io_handle, o_status);
          if (rc) {
            o_status.rc = rc;
            break;
          }

          errno = 0;

          if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
            snprintf(errstr, 200, "SERVER_DEBUG : scom_set_register() address = 0x%08X, data = %08X)\n", address, localData);
            o_status.errorMessage.append(errstr);
          }

          len = scom_set_register(*io_handle, o_status);

          if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
            snprintf(errstr, 200, "SERVER_DEBUG : scom_set_register() rc = %zd\n", len);
            o_status.errorMessage.append(errstr);
          }

          if (len != 4 && len != 0) {
            snprintf(errstr, 200, "FSIInstruction::execute(WRITESPMEM) Write length exp (%d) actual (%zd)\n", 4, len);
            o_status.errorMessage.append(errstr);
            snprintf(errstr, 200, "FSIInstruction::execute(WRITESPMEM) Problem writing to FSI device : errno %d\n", errno);
            o_status.errorMessage.append(errstr);
            rc = o_status.rc = SERVER_FSI_SCOM_WRITE_FAIL;
            fsi_close(*io_handle);
          } else {
            rc = o_status.rc = SERVER_COMMAND_COMPLETE;
          }
        }
        else if (l_type == CFAM_TYPE_MBX_SCRATCH)
        {

          /* Open the Handle */
          rc = fsi_open(io_handle, o_status);
          if (rc) {
            o_status.rc = rc;
            break;
          }

          errno = 0;

          if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
            snprintf(errstr, 200, "SERVER_DEBUG : mbx_set_scratch_register() address = 0x%08X, data = %08X)\n", address, localData);
            o_status.errorMessage.append(errstr);
          }

          len = mbx_set_scratch_register(*io_handle, o_status);

          if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
            snprintf(errstr, 200, "SERVER_DEBUG : mbx_set_scratch_register() rc = %zd\n", len);
            o_status.errorMessage.append(errstr);
          }

          if (len != 0) {
            snprintf(errstr, 200, "FSIInstruction::execute(WRITESPMEM) Problem writing to FSI device : errno %d\n", errno);
            o_status.errorMessage.append(errstr);
            rc = o_status.rc = SERVER_FSI_SCOM_WRITE_FAIL;
            fsi_close(*io_handle);
          } else {
            rc = o_status.rc = SERVER_COMMAND_COMPLETE;
          }
        }
        else if (l_type == CFAM_TYPE_MBX)
        {
          /* Open the Handle */
          rc = fsi_open(io_handle, o_status);
          if (rc) {
            o_status.rc = rc;
            break;
          }

          errno = 0;
          if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
            snprintf(errstr, 200, "SERVER_DEBUG : mbx_set_register() address = 0x%08X, data = %08X)\n", address, localData);
            o_status.errorMessage.append(errstr);
          }

          len = mbx_set_register(*io_handle, o_status);

          if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
            snprintf(errstr, 200, "SERVER_DEBUG : mbx_set_register() rc = %zd\n", len);
            o_status.errorMessage.append(errstr);
          }

          if (len != 4 && len != 0) {
            snprintf(errstr, 200, "FSIInstruction::execute(WRITESPMEM) Write length exp (%d) actual (%zd)\n", 4, len);
            o_status.errorMessage.append(errstr);
            snprintf(errstr, 200, "FSIInstruction::execute(WRITESPMEM) Problem writing to FSI device : errno %d\n", errno);
            o_status.errorMessage.append(errstr);
            rc = o_status.rc = SERVER_FSI_SCOM_WRITE_FAIL;
            fsi_close(*io_handle);
          } else {
            rc = o_status.rc = SERVER_COMMAND_COMPLETE;
          }
        }
        else if (l_type == CFAM_TYPE_GP_REG)
        {

          /* Open the Handle */
          rc = fsi_open(io_handle, o_status);
          if (rc) {
            o_status.rc = rc;
            break;
          }

          errno = 0;

          if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
            snprintf(errstr, 200, "SERVER_DEBUG : gp_reg_set() address = 0x%08X, data = %08X)\n", address, localData);
            o_status.errorMessage.append(errstr);
          }

          len = gp_reg_set(*io_handle, o_status);

          if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
            snprintf(errstr, 200, "SERVER_DEBUG : gp_reg_set() rc = %zd\n", len);
            o_status.errorMessage.append(errstr);
          }

          if (len != 0) {
            snprintf(errstr, 200, "FSIInstruction::execute(WRITESPMEM) Problem writing to FSI device : errno %d\n", errno);
            o_status.errorMessage.append(errstr);
            rc = o_status.rc = SERVER_FSI_SCOM_WRITE_FAIL;
            fsi_close(*io_handle);
          } else {
            rc = o_status.rc = SERVER_COMMAND_COMPLETE;
          }

        }
        else if (l_type == CFAM_TYPE_SBEFIFO)
        {

          /* Open the Handle */
          rc = fsi_open(io_handle, o_status);
          if (rc) {
            o_status.rc = rc;
            break;
          }

          errno = 0;

          if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
            snprintf(errstr, 200, "SERVER_DEBUG : sbefifo_set_register() address = 0x%08X data = %08X)\n", address, localData);
            o_status.errorMessage.append(errstr);
          }

          len = sbefifo_set_register(*io_handle, o_status);

          if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
            snprintf(errstr, 200, "SERVER_DEBUG : sbefifo_set_register() rc = %zd\n", len);
            o_status.errorMessage.append(errstr);
          }

          if (len != 4 && len != 0) {
            snprintf(errstr, 200, "FSIInstruction::execute(WRITESPMEM) Write length exp (%d) actual (%zd)\n", 4, len);
            o_status.errorMessage.append(errstr);
            snprintf(errstr, 200, "FSIInstruction::execute(WRITESPMEM) Problem writing to FSI device : errno %d\n", errno);
            o_status.errorMessage.append(errstr);
            rc = o_status.rc = SERVER_FSI_SBEFIFO_WRITE_FAIL;
            fsi_close(*io_handle);
          } else {
            rc = o_status.rc = SERVER_COMMAND_COMPLETE;
          }
        }
        /* Debug interfaces */
        else if (flags & INSTRUCTION_FLAG_FSI_USE_DRA) {
          /* The register isn't part of the officially supported registers, so we are going into the back door here */
          rc = o_status.rc = SERVER_FSI_HW_DRA_NEEDED;
        } else {
          rc = o_status.rc = SERVER_FSI_REGISTER_NOT_SUPPORTED;
        }
      }
      break;
    case SCOMIN_MASK:
      if (version == 0x1) {
        rc = o_status.rc = SERVER_COMMAND_NOT_SUPPORTED;
      } else {
        int bytelen = 0;
        unsigned long status = 0;
        char errstr[200];

        if (length <= 0) {
          rc = o_status.rc = SERVER_INVALID_COMMAND_BLOCK_FIELD_DATALEN;
          break;
        }

        /* Open the Handle */
        rc = scom_open(io_handle, o_status);
        if (rc) {
          o_status.rc = rc;
          break;
        }

        bytelen = length % 8 ? (length / 8) + 1 : length / 8;

        /* Actually write to the device */
        errno = 0;

        if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
          std::string dataWords;
          std::string maskWords;
          genWords(data, dataWords);
          genWords(mask, maskWords);
          if (flags & INSTRUCTION_FLAG_64BIT_ADDRESS) {
            snprintf(errstr, 200, "SERVER_DEBUG : scom_write_under_mask() data = %s, address64 = 0x" UINT64_HEX_VARIN_FORMAT(%016) ", mask = %s\n", dataWords.c_str(), address64, maskWords.c_str());
          } else {
            snprintf(errstr, 200, "SERVER_DEBUG : scom_write_under_mask() data = %s, address = 0x%08X, mask = %s\n", dataWords.c_str(), address, maskWords.c_str());
          }
          o_status.errorMessage.append(errstr);
        }

        rc = scom_write_under_mask(*io_handle, o_status);

        if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
          snprintf(errstr, 200, "SERVER_DEBUG : scom_write_under_mask() o_status = 0x%08lX, rc = %u\n", status, rc);
          o_status.errorMessage.append(errstr);
        }

        o_status.data.setBitLength(32);
        o_status.data.insert((uint32_t) status, 0, 32);

        if (rc != bytelen) {
          snprintf(errstr, 200, "FSIInstruction::execute(SCOMIN_MASK) Write length exp (%d) actual (%d)\n",bytelen, rc);
          o_status.errorMessage.append(errstr);
          snprintf(errstr, 200, "FSIInstruction::execute(SCOMIN_MASK) Problem writing to FSI device : errno %d\n", errno);
          o_status.errorMessage.append(errstr);
          rc = o_status.rc = SERVER_FSI_SCOM_WRITE_FAIL;
          scom_ffdc_and_reset(io_handle, o_status);
          break;
        } else {
          rc = o_status.rc = SERVER_COMMAND_COMPLETE;
        }
      }
      break;

    case BULK_SCOMIN:
      {
        int bytelen = 0;
        char errstr[200];

        if (length <= 0) {
          rc = o_status.rc = SERVER_INVALID_COMMAND_BLOCK_FIELD_DATALEN;
          break;
        }

        /* Open the Handle */
        rc = scom_open(io_handle, o_status);
        if (rc) {
          o_status.rc = rc;
          break;
        }

        uint32_t doubleWordLength = length / 64;
        for (uint32_t doubleWordIndex = 0; doubleWordIndex < doubleWordLength; doubleWordIndex++) {
          bytelen = 8;

          /* Actually write to the device */
          errno = 0;

          if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
            std::string words;
            words = data.genHexLeftStr(doubleWordIndex * 64, 64);
            if (flags & INSTRUCTION_FLAG_64BIT_ADDRESS) {
              snprintf(errstr, 200, "SERVER_DEBUG : scom_write() data[%d] = %s, address64 = 0x" UINT64_HEX_VARIN_FORMAT(%016) "\n", doubleWordIndex, words.c_str(), address64);
            } else {
              snprintf(errstr, 200, "SERVER_DEBUG : scom_write() data[%d] = %s, address = 0x%08X\n", doubleWordIndex, words.c_str(), address);
            }
            o_status.errorMessage.append(errstr);
          }

          rc = scom_write(*io_handle, o_status, doubleWordIndex);

          if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
            snprintf(errstr, 200, "SERVER_DEBUG : scom_write() rc = %u\n", rc);
            o_status.errorMessage.append(errstr);
          }

          if (rc != bytelen) {
            snprintf(errstr, 200, "FSIInstruction::execute(BULK_SCOMIN) Write length exp (%d) actual (%d)\n",bytelen, rc);
            o_status.errorMessage.append(errstr);
            snprintf(errstr, 200, "FSIInstruction::execute(BULK_SCOMIN) Problem writing to FSI device : errno %d\n", errno);
            o_status.errorMessage.append(errstr);
            rc = o_status.rc = SERVER_FSI_SCOM_WRITE_FAIL;
            scom_ffdc_and_reset(io_handle, o_status);
            break;
          } else {
            rc = o_status.rc = SERVER_COMMAND_COMPLETE;
          }

          if (rc == SERVER_COMMAND_COMPLETE) {
            if (flags & INSTRUCTION_FLAG_FSI_CFAM2_0) {
              if (o_status.data.isBitClear(7) || o_status.data.getNumBitsSet(17, 3)) {
                rc = o_status.rc = SERVER_FSI_SCOM_ERROR;
                break;
              }
            }
          }
        }
      }
      break;

    case BULK_SCOMOUT:
      {
        int bytelen = 0;
        char errstr[200];

        if (length <= 0) {
          rc = o_status.rc = SERVER_INVALID_COMMAND_BLOCK_FIELD_DATALEN;
          break;
        }

        /* Open the Handle */
        rc = scom_open(io_handle, o_status);
        if (rc) {
          o_status.rc = rc;
          break;
        }

        o_data.setBitLength(length);
        uint32_t doubleWordLength = length / 64;
        for (uint32_t doubleWordIndex = 0; doubleWordIndex < doubleWordLength; doubleWordIndex++) {
          bytelen = 8;

          /* Actually read from the device */
          errno = 0;

          if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
            if (flags & INSTRUCTION_FLAG_64BIT_ADDRESS) {
              snprintf(errstr, 200, "SERVER_DEBUG : scom_read() address64 = 0x" UINT64_HEX_VARIN_FORMAT(%016) "\n", address64);
            } else {
              snprintf(errstr, 200, "SERVER_DEBUG : scom_read() address =  0x%08X\n", address);
            }
            o_status.errorMessage.append(errstr);
          }

          rc = scom_read(*io_handle, o_data, o_status, doubleWordIndex);

          if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
            std::string words;
            words = o_data.genHexLeftStr(doubleWordIndex * 64, 64);
            snprintf(errstr, 200, "SERVER_DEBUG : scom_read() o_data[%d] = %s, rc = %u\n", doubleWordIndex, words.c_str(), rc);
            o_status.errorMessage.append(errstr);
          }

          if (rc != bytelen) {
            snprintf(errstr, 200, "FSIInstruction::execute(BULK_SCOMOUT) Read length exp (%d) actual (%d)\n",bytelen, rc);
            o_status.errorMessage.append(errstr);
            snprintf(errstr, 200, "FSIInstruction::execute(BULK_SCOMOUT) Problem reading from FSI device : errno %d\n", errno);
            o_status.errorMessage.append(errstr);
            rc = o_status.rc = SERVER_FSI_SCOM_READ_FAIL;
            scom_ffdc_and_reset(io_handle, o_status);
            break;
          } else {
            rc = o_status.rc = SERVER_COMMAND_COMPLETE;
          }

          if (rc == SERVER_COMMAND_COMPLETE) {
            if (flags & INSTRUCTION_FLAG_FSI_CFAM2_0) {
              if (o_status.data.isBitClear(7) || o_status.data.getNumBitsSet(17, 3)) {
                rc = o_status.rc = SERVER_FSI_SCOM_ERROR;
                break;
              }
            }
          }
        }
      }
      break;

    default:
      rc = o_status.rc = SERVER_COMMAND_NOT_SUPPORTED;
      break;
  }

  /* check for scom errors */
  switch(command) {
    case SCOMIN:
    case SCOMOUT:
    case SCOMIN_MASK:
      if (rc == SERVER_COMMAND_COMPLETE) {
        if (flags & INSTRUCTION_FLAG_FSI_CFAM2_0) {
          if (o_status.data.isBitClear(7) || o_status.data.getNumBitsSet(17, 3)) {
            rc = o_status.rc = SERVER_FSI_SCOM_ERROR;
          }
        }
      }
      break;
    default:
      break;
  }

  return (uint32_t) rc;
}

uint32_t FSIInstruction::flatten(uint8_t * o_data, uint32_t i_len) const {
  uint32_t rc = 0;
  uint32_t * o_ptr = (uint32_t *) o_data;
  
  if (i_len < flattenSize()) {
    out.error("FSIInstruction::flatten", "i_len %d bytes is too small to flatten\n", i_len);
    rc = 1;
  } else {
    // clear memory
    memset(o_data, 0, flattenSize());
    o_ptr[0] = htonl(version);
    o_ptr[1] = htonl(command);
    o_ptr[2] = htonl(flags);
    if (flags & INSTRUCTION_FLAG_DEVSTR) {
      uint32_t offset = 0;
      if (flags & INSTRUCTION_FLAG_64BIT_ADDRESS) {
        o_ptr[3] = htonl((uint32_t) (address64 >> 32));
        o_ptr[4] = htonl((uint32_t) (address64 & 0xFFFFFFFF));
        offset = 1;
      } else {
        o_ptr[3] = htonl(address);
        offset = 0;
      }
      o_ptr[4 + offset] = htonl(length);
      uint32_t deviceStringSize = deviceString.size() + 1;
      if (deviceStringSize % sizeof(uint32_t)) {
        deviceStringSize += (sizeof(uint32_t) - (deviceStringSize % sizeof(uint32_t)));
      }
      o_ptr[5 + offset] = htonl(deviceStringSize);
      if ((command == SCOMIN) || (command == LONGIN) || (command == WRITESPMEM) || (command == SCOMIN_MASK) || (command == BULK_SCOMIN)) {
          uint32_t dataSize = data.flattenSize();
          o_ptr[6 + offset] = htonl(dataSize);
          if (command == SCOMIN_MASK) {
            uint32_t maskSize = mask.flattenSize();
            o_ptr[7 + offset] = htonl(maskSize);
            mask.flatten((uint8_t *) (o_ptr + 8 + offset+ (deviceStringSize / sizeof(uint32_t)) + (dataSize / sizeof(uint32_t))), maskSize);
            offset++;
          }
          data.flatten((uint8_t *) (o_ptr + 7 + offset + (deviceStringSize / sizeof(uint32_t))), dataSize);
          offset++;
      }
      if (deviceString.size() > 0) {
        strcpy(((char *)(o_ptr + 6 + offset)), deviceString.c_str());
      }
    } else {
      o_ptr[3] = htonl(cfamid);
      o_ptr[4] = htonl(linkid);
      o_ptr[5] = htonl(cmaster);
      if ((flags & INSTRUCTION_FLAG_64BIT_ADDRESS) && ((command == SCOMIN) || (command == SCOMOUT) || (command == SCOMIN_MASK) || (command == BULK_SCOMIN))) {
        o_ptr[6] = htonl((uint32_t) (address64 >> 32));
        o_ptr[7] = htonl((uint32_t) (address64 & 0xFFFFFFFF));
        o_ptr[8] = htonl(length);
        if ((command == SCOMIN) || (command == BULK_SCOMIN)) {
          uint32_t dataSize = data.flattenSize();
          o_ptr[9] = htonl(dataSize);
          data.flatten((uint8_t *) (o_ptr + 10), dataSize);
        } else if (command == SCOMIN_MASK) {
          uint32_t dataSize = data.flattenSize();
          uint32_t maskSize = mask.flattenSize();
          o_ptr[9] = htonl(dataSize);
          o_ptr[10] = htonl(maskSize);
          data.flatten((uint8_t *) (o_ptr + 11), dataSize);
          mask.flatten((uint8_t *) (o_ptr + 11 + (dataSize / sizeof(uint32_t))), maskSize);
        }
      } else {
        o_ptr[6] = htonl(address);
        o_ptr[7] = htonl(length);
        if ((command == SCOMIN) || (command == LONGIN) || (command == WRITESPMEM) || (command == BULK_SCOMIN)) {
          uint32_t dataSize = data.flattenSize();
          o_ptr[8] = htonl(dataSize);
          data.flatten((uint8_t *) (o_ptr + 9), dataSize);
        } else if (command == SCOMIN_MASK) {
          uint32_t dataSize = data.flattenSize();
          uint32_t maskSize = mask.flattenSize();
          o_ptr[8] = htonl(dataSize);
          o_ptr[9] = htonl(maskSize);
          data.flatten((uint8_t *) (o_ptr + 10), dataSize);
          mask.flatten((uint8_t *) (o_ptr + 10 + (dataSize / sizeof(uint32_t))), maskSize);
        }
      }
    }
  }
  return rc;
}

uint32_t FSIInstruction::unflatten(const uint8_t * i_data, uint32_t i_len) {
  uint32_t rc = 0;
  uint32_t * i_ptr = (uint32_t *) i_data;

  version = ntohl(i_ptr[0]);
  if(version >= 0x1 && version <= 0x6) {
    command = (InstructionCommand) ntohl(i_ptr[1]);
    flags = ntohl(i_ptr[2]);
    if ((version >= 0x5) && (flags & INSTRUCTION_FLAG_DEVSTR)) {
      uint32_t offset = 0;
      if (flags & INSTRUCTION_FLAG_64BIT_ADDRESS) {
        address64 = ((uint64_t) ntohl(i_ptr[3])) << 32;
        address64 |= ((uint64_t) ntohl(i_ptr[4]));
        offset = 1;
      } else {
        address = ntohl(i_ptr[3]);
        offset = 0;
      }
      length = ntohl(i_ptr[4 + offset]);
      uint32_t deviceStringSize = ntohl(i_ptr[5 + offset]);
      if ((command == SCOMIN) || (command == LONGIN) || (command == WRITESPMEM) || (command == SCOMIN_MASK) || (command == BULK_SCOMIN)) {
          uint32_t dataSize = ntohl(i_ptr[6 + offset]);
          if (command == SCOMIN_MASK) {
            uint32_t maskSize = ntohl(i_ptr[7 + offset]);
            rc = mask.unflatten((uint8_t *) (i_ptr + 8 + offset + (deviceStringSize / sizeof(uint32_t)) + (dataSize / sizeof(uint32_t))), maskSize);
            if (rc) { error = rc; }

            offset++;
          }
          rc = data.unflatten((uint8_t *) (i_ptr + 7 + offset + (deviceStringSize / sizeof(uint32_t))), dataSize);
          if (rc) { error = rc; }
          offset++;
      }
      if (deviceStringSize > 0) {
        deviceString = ((char *)(i_ptr + 6 + offset));
      }
    } else {
      cfamid = ntohl(i_ptr[3]);
      linkid = ntohl(i_ptr[4]);
      cmaster = ntohl(i_ptr[5]);
      if ((version >= 0x4) && (flags & INSTRUCTION_FLAG_64BIT_ADDRESS) && ((command == SCOMIN) || (command == SCOMOUT) || (command == SCOMIN_MASK) || (command == BULK_SCOMIN))) {
        address64 = ((uint64_t) ntohl(i_ptr[6])) << 32;
        address64 |= ((uint64_t) ntohl(i_ptr[7]));
        length = ntohl(i_ptr[8]);
        if ((command == SCOMIN) || (command == BULK_SCOMIN)) {
          uint32_t dataSize = ntohl(i_ptr[9]);
          rc = data.unflatten((uint8_t *) (i_ptr + 10), dataSize);
          if (rc) { error = rc; }
        } else if (command == SCOMIN_MASK) {
          uint32_t dataSize = ntohl(i_ptr[9]);
          uint32_t maskSize = ntohl(i_ptr[10]);
          rc = data.unflatten((uint8_t *) (i_ptr + 11), dataSize);
          if (rc) { error = rc; }
          rc = mask.unflatten((uint8_t *) (i_ptr + 11 + (dataSize / sizeof(uint32_t))), maskSize);
          if (rc) { error = rc; }
        }
      } else {
        address = ntohl(i_ptr[6]);
        length = ntohl(i_ptr[7]);
        if ((version == 0x1) || ((version >= 0x2) && ((command == SCOMIN) || (command == LONGIN) || (command == WRITESPMEM) || (command == BULK_SCOMIN)))) {
          uint32_t dataSize = ntohl(i_ptr[8]);
          rc = data.unflatten((uint8_t *) (i_ptr + 9), dataSize);
          if (rc) { error = rc; }
        } else if ((version >= 0x2) && (command == SCOMIN_MASK)) {
          uint32_t dataSize = ntohl(i_ptr[8]);
          uint32_t maskSize = ntohl(i_ptr[9]);
          rc = data.unflatten((uint8_t *) (i_ptr + 10), dataSize);
          if (rc) { error = rc; }
          rc = mask.unflatten((uint8_t *) (i_ptr + 10 + (dataSize / sizeof(uint32_t))), maskSize);
          if (rc) { error = rc; }
        }
      }
    }
  } else {
    error = rc = SERVER_UNKNOWN_INSTRUCTION_VERSION;
  }
  return rc;
}

uint32_t FSIInstruction::flattenSize(void) const {
  if ((version >= 0x5) && (flags & INSTRUCTION_FLAG_DEVSTR)) {
    uint32_t size = 6 * sizeof(uint32_t); // version, command, flags, length, deviceStringSize, address
    if (flags & INSTRUCTION_FLAG_64BIT_ADDRESS) {
      size += sizeof(uint32_t); // address64
    }
    uint32_t deviceStringSize = deviceString.size() + 1;
    if (deviceStringSize % sizeof(uint32_t)) {
      deviceStringSize += (sizeof(uint32_t) - (deviceStringSize % sizeof(uint32_t)));
    }
    size += deviceStringSize; // deviceString
    if ((command == SCOMIN) || (command == LONGIN) || (command == WRITESPMEM) || (command == SCOMIN_MASK) || (command == BULK_SCOMIN)) {
      size += sizeof(uint32_t); // dataSize
      size += data.flattenSize(); // data
      if (command == SCOMIN_MASK) {
        size += sizeof(uint32_t); // maskSize
        size += mask.flattenSize(); // mask
      }
    }
    return size;
  } else if ((version >= 0x4) && ((command == SCOMIN) || (command == BULK_SCOMIN)) && (flags & INSTRUCTION_FLAG_64BIT_ADDRESS)) {
    return (10 * sizeof(uint32_t)) + data.flattenSize();
  } else if ((version >= 0x4) && (command == SCOMOUT) && (flags & INSTRUCTION_FLAG_64BIT_ADDRESS)) {
    return (9 * sizeof(uint32_t));
  } else if ((version >= 0x4) && (command == SCOMIN_MASK) && (flags & INSTRUCTION_FLAG_64BIT_ADDRESS)) {
    return (11 * sizeof(uint32_t)) + data.flattenSize() + mask.flattenSize();
  } else if ((version == 0x1) || (version >= 0x2 && ((command == SCOMIN) || (command == LONGIN) || (command == WRITESPMEM) || (command == BULK_SCOMIN)))) {
    return (9 * sizeof(uint32_t)) + data.flattenSize();
  } else if ((version >= 0x2) && (command == SCOMIN_MASK)) {
    return (10 * sizeof(uint32_t)) + data.flattenSize() + mask.flattenSize();
  } else { // version 0x2 all other cases
    return (8 * sizeof(uint32_t));
  }
}

std::string FSIInstruction::dumpInstruction(void) const {
  std::ostringstream oss;
  oss << "FSIInstruction" << std::endl;
  oss << "version       : " << version << std::endl;
  oss << "command       : " << InstructionCommandToString(command) << std::endl;
  oss << "type          : " << InstructionTypeToString(type) << std::endl;
  oss << "flags         : " << InstructionFlagToString(flags) << std::endl;
  if (flags & INSTRUCTION_FLAG_DEVSTR) {
    oss << "deviceString  : " << deviceString << std::endl;
  } else {
    oss << "cfamid        : " << std::hex << std::setw(8) << std::setfill('0') << cfamid << std::dec << std::endl;
    oss << "linkid        : " << std::hex << std::setw(8) << std::setfill('0') << linkid << std::dec << std::endl;
    oss << "cmaster       : " << std::hex << std::setw(8) << std::setfill('0') << cmaster << std::dec << std::endl;
  }
  if (flags & INSTRUCTION_FLAG_64BIT_ADDRESS) {
    oss << "address64     : " << std::hex << std::setw(16) << std::setfill('0') << address64 << std::dec << std::endl;
  } else {
    oss << "address       : " << std::hex << std::setw(8) << std::setfill('0') << address << std::dec << std::endl;
  }
  oss << "length        : " << length << std::endl;
  oss << "data length   : " << data.getBitLength() << std::endl;
  oss << "data          : ";
  for(uint32_t j = 0; j < data.getWordLength(); j++) {
    oss << std::hex << std::setw(8) << std::setfill('0') << data.getWord(j) << " ";
    if (!((j+1) % 5)) oss << "\n\t\t";
  }
  oss << std::dec << std::endl;
  if (command == SCOMIN_MASK) {
    oss << "mask length   : " << mask.getBitLength() << std::endl;
    oss << "mask          : ";
    for(uint32_t j = 0; j < mask.getWordLength(); j++) {
      oss << std::hex << std::setw(8) << std::setfill('0') << mask.getWord(j) << " ";
      if (!((j+1) % 5)) oss << "\n\t\t";
    }
    oss << std::dec << std::endl;
  }

  return oss.str();
}

uint64_t FSIInstruction::getHash(void) const {
  //FSI,      SCAN | SCOM | other, linkid(24:31), cfamid(24:31), cmaster(24:31)
  //bits 0:3  bits 4:7,            bits 8:15,     bits 16:23,    bits 24:31
  uint32_t hash = 0x0;
  uint32_t scanscom = 0xF;
  if ((command == SCOMIN) || (command == SCOMOUT) || (command == SCOMIN_MASK) || (command == BULK_SCOMIN) || (command == BULK_SCOMOUT)) {
    scanscom = 0x4;
  } else if ((command == LONGIN) || (command == LONGOUT)) {
    scanscom = 0x8;
  } else if ((command == READSPMEM) || (command == WRITESPMEM)) {
    if ((address & 0x0FFFFF00) == 0x0000C00) {
      scanscom = 0x8;
    } else if (((address & 0x0FFFFF00) == 0x0001000) || ((address & 0x0FFFFF00) == 0x0002800) || ((address & 0x0FFFFF00) == 0x0002900)) {
      scanscom = 0x4;
    } else {
      scanscom = 0x2;
    }
  } else {
    scanscom = 0x0;
  }
  if (flags & INSTRUCTION_FLAG_DEVSTR) {
    uint32_t devstrhash = 0x0;
    uint32_t rc = devicestring_genhash(deviceString, devstrhash);
    uint64_t hash64 = 0x0ull;
    hash64 |= ((0x0000000Full & type)     << 60);
    hash64 |= ((0x0000000Full & scanscom) << 56);
    if (rc == 0) {
        hash64 |= ((uint64_t) devstrhash);
    }
    return hash64;
  }
  // for case where cmaster = 0x40000000 and we can't tell if cmaster is enabled
  if (cmaster != 0x0) {
    scanscom |= 0x1;
  }
  hash |= ((0x0000000F & type)     << 28);
  hash |= ((0x0000000F & scanscom) << 24);
  hash |= ((0x000000FF & linkid)   << 16);
  hash |= ((0x000000FF & cfamid)   << 8);
  hash |= ((0x000000FF & cmaster)  << 0);
  return hash;
}

uint32_t FSIInstruction::closeHandle(Handle ** i_handle) {
  uint32_t rc = 0;

  switch (command) {
    case NOCOMMAND:
      break;
    case LONGIN:
    case LONGOUT:
      rc = scan_close(*i_handle);
      break;
    case SCOMIN:
    case SCOMOUT:
    case SCOMIN_MASK:
    case BULK_SCOMIN:
    case BULK_SCOMOUT:
      rc = scom_close(*i_handle);
      break;
    case READSPMEM:
    case WRITESPMEM:
      {
        CFAMType l_type = getCFAMType(address, flags);
        switch (l_type)
        {
          case CFAM_TYPE_SCAN:
            rc = scan_close(*i_handle);
            break;
          case CFAM_TYPE_SCOM:
          case CFAM_TYPE_MBX:
          case CFAM_TYPE_MBX_SCRATCH:
          case CFAM_TYPE_GP_REG:
          case CFAM_TYPE_SBEFIFO:
            rc = fsi_close(*i_handle);
            break;
          case CFAM_TYPE_INVALID:
          default:
            break;
        }
      }
      break;
    default:
      break;
  }

  *i_handle = NULL;

  return rc;
}

std::string FSIInstruction::getInstructionVars(const InstructionStatus & i_status) const {
  std::ostringstream oss;

  oss << std::hex << std::setfill('0');
  oss << "rc: " << std::setw(8) << i_status.rc;
  if (i_status.data.getWordLength() > 0) {
    oss << " status: " << std::setw(8) << i_status.data.getWord(0);
  }
  if (flags & INSTRUCTION_FLAG_DEVSTR) {
    oss << " devstr: " << deviceString;
  } else {
    oss << " cfamid: " << std::setw(8) << cfamid;
    oss << " linkid: " << std::setw(8) << linkid;
    oss << " cmaster: " << std::setw(8) << cmaster;
  }
  if (flags & INSTRUCTION_FLAG_64BIT_ADDRESS) {
    oss << " address64: " << std::setw(16) << address64;
  } else {
    oss << " address: " << std::setw(8) << address;
  }
  if (command == WRITESPMEM) {
    oss << " data: " << std::setw(8) << data.getWord(0);
  } else if (command == SCOMIN || command == SCOMIN_MASK) {
    oss << " data: " << std::setw(16) << data.getDoubleWord(0);
    if (command == SCOMIN_MASK) {
      oss << " mask: " << std::setw(16) << mask.getDoubleWord(0);
    }
  }

  return oss.str();
}

FSIInstruction::CFAMType FSIInstruction::getCFAMType(const uint32_t i_address, const uint32_t i_flags)
{
    CFAMType l_type = CFAM_TYPE_INVALID;

    if ((i_address & 0x0FFFFF00) == 0x0000C00)
    {
        l_type = CFAM_TYPE_SCAN;
    }
    else if ((i_address & 0x0FFFFF00) == 0x0001000)
    {
        l_type = CFAM_TYPE_SCOM;
    }
    else if ((i_address & 0x0FFFF00) == 0x0002400)
    {
        l_type = CFAM_TYPE_SBEFIFO;
    }
    else if (((i_address & 0x0FFFFFFF) >= 0x0002838) && ((i_address & 0x0FFFFFFF) <= 0x000283F))
    {
        l_type = CFAM_TYPE_MBX_SCRATCH;
    }
    else if (((i_address & 0x0FFFFF00) == 0x0002800) || ((i_address & 0x0FFFFF00) == 0x0002900))
    {
        if (i_flags & INSTRUCTION_FLAG_CFAM_MAILBOX)
        {
            l_type = CFAM_TYPE_MBX;
        }
        else
        {
            l_type = CFAM_TYPE_GP_REG;
        }
    }

    return l_type;
}
