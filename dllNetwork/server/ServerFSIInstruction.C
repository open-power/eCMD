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

#include <ServerFSIInstruction.H>
#include <OutputLite.H>
#include <sstream>
#include <stdio.h>
#include <cstring>

#include <adal_scom.h>
#include <adal_scan.h>
#include <adal_mbx.h>
#include <adal_sbefifo.h>
#include <adal_iic.h>

extern bool global_server_debug;

//FIXME remove these
adal_t * system_gp_reg_open(const char * device, int flags, int type)
{
    return adal_mbx_open(device, flags);
}
int system_gp_reg_close(adal_t * adal)
{
    return adal_mbx_close(adal);
}
ssize_t system_gp_reg_get(adal_t * adal, unsigned long offset, uint32_t * data)
{
    ssize_t rc = adal_mbx_get_register(adal, offset, data);
    if (rc == 4) rc = 0;
    return rc;
}
ssize_t system_gp_reg_set(adal_t * adal, unsigned long offset, unsigned long data)
{
    ssize_t rc = adal_mbx_set_register(adal, offset, data);
    if (rc == 4) rc = 0;
    return rc;
}
#define EDAL_GP_TYPE_MBX 0

uint32_t ServerFSIInstruction::scan_open(Handle ** handle, InstructionStatus & o_status) {
  uint32_t rc = 0;

  char device[50];
  char errstr[200];

  /* already have a handle lets reuse it */
  if(*handle != NULL)
    return rc;

  /* We need to open the device*/
  if (flags & INSTRUCTION_FLAG_DEVSTR) {
    snprintf(device, 50, "/dev/scan%s", deviceString.c_str());
  } else {
    //ERROR
    *handle = NULL;
    snprintf(errstr, 200, "ServerFSIInstruction::scan_open INSTRUCTION_FLAG_DEVSTR must be set\n");
    o_status.errorMessage.append(errstr);
    return SERVER_INVALID_FSI_DEVICE;
  }
  errno = 0;

  if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
    snprintf(errstr, 200, "SERVER_DEBUG : adal_scan_open(%s, O_RDWR | O_SYNC)\n", device);
    o_status.errorMessage.append(errstr);
  }

#ifdef TESTING
  TEST_PRINT("*handle = adal_scan_open(%s, O_RDWR | O_SYNC);\n", device);
  *handle = (Handle *) 0x1;
#else
  *handle = (Handle *) adal_scan_open(device, O_RDWR | O_SYNC);
#endif

  if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
    snprintf(errstr, 200, "SERVER_DEBUG : adal_scan_open() *handle = %p\n", *handle);
    o_status.errorMessage.append(errstr);
  }

  if (*handle == NULL) {
    snprintf(errstr, 200, "ServerFSIInstruction::scan_open Problem opening FSI device %s : errno %d\n", device, errno);
    o_status.errorMessage.append(errstr);
    return SERVER_INVALID_FSI_DEVICE;
  }

  return rc;
}

uint32_t ServerFSIInstruction::scom_open(Handle** handle, InstructionStatus & o_status) {
  uint32_t rc = 0;

  char device[50];
  char errstr[200];

  /* already have a handle lets reuse it */
  if(*handle != NULL)
    return rc;

  /* We need to open the device*/
  if (flags & INSTRUCTION_FLAG_DEVSTR) {
    snprintf(device, 50, "/dev/scom%s", deviceString.c_str());
  } else {
    //ERROR
    *handle = NULL;
    snprintf(errstr, 200, "ServerFSIInstruction::scom_open INSTRUCTION_FLAG_DEVSTR must be set\n");
    o_status.errorMessage.append(errstr);
    return SERVER_INVALID_FSI_DEVICE;
  }
  errno = 0;

  if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
    snprintf(errstr, 200, "SERVER_DEBUG : adal_scom_open(%s, O_RDWR | O_SYNC)\n", device);
    o_status.errorMessage.append(errstr);
  }

#ifdef TESTING
  TEST_PRINT("*handle = adal_scom_open(%s, O_RDWR | O_SYNC);\n", device);
  *handle = (Handle *) 0x1;
#else
  *handle = (Handle *) adal_scom_open(device, O_RDWR | O_SYNC);
#endif

  if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
    snprintf(errstr, 200, "SERVER_DEBUG : adal_scom_open() *handle = %p\n", *handle);
    o_status.errorMessage.append(errstr);
  }

  if (*handle == NULL) {
    snprintf(errstr, 200, "ServerFSIInstruction::scom_open Problem opening FSI device %s : errno %d\n", device, errno);
    o_status.errorMessage.append(errstr);
    return SERVER_INVALID_FSI_DEVICE;
  }

  return rc;
}

uint32_t ServerFSIInstruction::gp_reg_open(Handle** handle, InstructionStatus & o_status) {
  uint32_t rc = 0;

  char device[100];
  char errstr[200];

  /* already have a handle lets reuse it */
  if(*handle != NULL)
    return rc;

  /* We need to open the device*/
  if (flags & INSTRUCTION_FLAG_DEVSTR) {
    //snprintf(device, 50, "/dev/mbx%s", deviceString.c_str());
    snprintf(device, 100, "/sys/bus/platform/devices/fsi-master/slave@00:00/raw");
  } else {
    //ERROR
    *handle = NULL;
    snprintf(errstr, 200, "ServerFSIInstruction::gp_reg_open INSTRUCTION_FLAG_DEVSTR must be set\n");
    o_status.errorMessage.append(errstr);
    return SERVER_INVALID_FSI_DEVICE;
  }
  errno = 0;

  if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
    snprintf(errstr, 200, "SERVER_DEBUG : system_gp_reg_open(%s, O_RDWR | O_SYNC, EDAL_GP_TYPE_MBX)\n", device);
    o_status.errorMessage.append(errstr);
  }

#ifdef TESTING
  TEST_PRINT("*handle = system_gp_reg_open(%s, O_RDWR | O_SYNC, EDAL_GP_TYPE_MBX);\n", device);
  *handle = (Handle *) 0x1;
#else
  *handle = (Handle *) system_gp_reg_open(device, O_RDWR | O_SYNC, EDAL_GP_TYPE_MBX);
#endif

  if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
    snprintf(errstr, 200, "SERVER_DEBUG : system_gp_reg_open() *handle = %p\n", *handle);
    o_status.errorMessage.append(errstr);
  }

  if (*handle == NULL) {
    snprintf(errstr, 200, "ServerFSIInstruction::gp_reg_open Problem opening FSI device %s : errno %d\n", device, errno);
    o_status.errorMessage.append(errstr);
    return SERVER_INVALID_FSI_DEVICE;
  }

  return rc;
}

uint32_t ServerFSIInstruction::mbx_open(Handle** handle, InstructionStatus & o_status) {
  uint32_t rc = 0;

  char device[100];
  char errstr[200];

  /* already have a handle lets reuse it */
  if(*handle != NULL)
    return rc;

  const uint32_t numPaths = 5;
  const char * devices[9][5] = {
    {"", "", "", "", ""},
    {"/sys/class/fsi-master/fsi0/slave@00:00/raw",
     "/sys/devices/platform/gpio-fsi/fsi0/slave@00:00/raw",
     "/sys/devices/platform/fsi-master/slave@00:00/raw",
     "/sys/bus/platform/devices/fsi-master/slave@00:00/raw",
     ""},
    {"/sys/class/fsi-master/fsi1/slave@01:00/raw",
     "/sys/class/fsi-master/fsi0/slave@00:00/00:00:00:0a/fsi-master/fsi1/slave@01:00/raw",
     "/sys/devices/platform/gpio-fsi/fsi0/slave@00:00/00:00:00:0a/fsi1/slave@01:00/raw",
     "/sys/devices/platform/fsi-master/slave@00:00/hub@00/slave@01:00/raw",
     "/sys/devices/hub@00/slave@01:00/raw"},
    {"/sys/class/fsi-master/fsi1/slave@02:00/raw", "", "", "", ""},
    {"/sys/class/fsi-master/fsi1/slave@03:00/raw", "", "", "", ""},
    {"/sys/class/fsi-master/fsi1/slave@04:00/raw", "", "", "", ""},
    {"/sys/class/fsi-master/fsi1/slave@05:00/raw", "", "", "", ""},
    {"/sys/class/fsi-master/fsi1/slave@06:00/raw", "", "", "", ""},
    {"/sys/class/fsi-master/fsi1/slave@07:00/raw", "", "", "", ""}};
  int errnos[5] = { 0, 0, 0, 0, 0 };

  /* We need to open the device*/
  if ((flags & INSTRUCTION_FLAG_DEVSTR) == 0) {
    //ERROR
    *handle = NULL;
    snprintf(errstr, 200, "ServerFSIInstruction::mbx_open INSTRUCTION_FLAG_DEVSTR must be set\n");
    o_status.errorMessage.append(errstr);
    return SERVER_INVALID_FSI_DEVICE;
  }

  /* Figure out the slave device based on the deviceString */
  std::istringstream iss;
  iss.str(deviceString);
  uint32_t l_idx = 0;
  iss >> l_idx;

  if ( (l_idx < 1) || (l_idx > 8) ) {
    *handle = NULL;
    snprintf(errstr, 200, "ServerFSIInstruction::mbx_open deviceString %s is not valid\n", deviceString.c_str());
    o_status.errorMessage.append(errstr);
    return SERVER_INVALID_FSI_DEVICE;
  }

  for (uint32_t l_try = 0; l_try < numPaths; l_try++) {
    /* Validate and adjust the device string for the device to open */
    snprintf(device, 100, "%s", devices[l_idx][l_try]);
    if ( strlen(device) == 0 ) continue;

    errno = 0;

    if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
      snprintf(errstr, 200, "SERVER_DEBUG : adal_mbx_open(%s, O_RDWR | O_SYNC)\n", device);
      o_status.errorMessage.append(errstr);
    }

#ifdef TESTING
    TEST_PRINT("*handle = adal_mbx_open(%s, O_RDWR | O_SYNC);\n", device);
    *handle = (Handle *) 0x1;
#else
    *handle = (Handle *) adal_mbx_open(device, O_RDWR | O_SYNC);
#endif

    if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
      snprintf(errstr, 200, "SERVER_DEBUG : adal_mbx_open() *handle = %p\n", *handle);
      o_status.errorMessage.append(errstr);
    }

    if (*handle == NULL) {
      // suppress errors being appended to the o_status
      errnos[l_try] = errno;
    } else {
      break;
    }
  }

  if (*handle == NULL) {
    // exhausted all file paths to find a valid device, tell the user what we tried for FA
    for ( uint32_t l_try = 0; l_try < numPaths; l_try++ )
    {
      // print all paths we tried
      snprintf(errstr, 200, "ServerFSIInstruction::mbx_open Problem opening FSI device %s : errno %d\n", devices[l_idx][l_try], errnos[l_try]);
      o_status.errorMessage.append(errstr);
    }
    return SERVER_INVALID_FSI_DEVICE;
  }

  return rc;
}

uint32_t ServerFSIInstruction::sbefifo_open(Handle ** handle, InstructionStatus & o_status)
{
    int rc = 0;

    char device[50];
    char errstr[200];

    /* already have a handle lets reuse it */
    if(*handle != NULL)
        return rc;

    /* We need to open the device*/
    snprintf(device, 50, "/dev/sbefifo%s", deviceString.c_str());
    errno = 0;

    if (flags & INSTRUCTION_FLAG_SERVER_DEBUG)
    {
        snprintf(errstr, 200, "SERVER_DEBUG : adal_sbefifo_open(%s, O_RDWR | O_NONBLOCK)\n", device);
        o_status.errorMessage.append(errstr);
    }

#ifdef TESTING
    TEST_PRINT("*handle = adal_sbefifo_open(%s, O_RDWR | O_NONBLOCK);\n", device);
    *handle = (Handle *) 0x1;
#else
    *handle = (Handle *) adal_sbefifo_open(device, O_RDWR | O_NONBLOCK);
#endif

    if (flags & INSTRUCTION_FLAG_SERVER_DEBUG)
    {
        snprintf(errstr, 200, "SERVER_DEBUG : adal_sbefifo_open() *handle = %p\n", *handle);
        o_status.errorMessage.append(errstr);
    }

    if (*handle == NULL)
    {
        snprintf(errstr, 200, "ServerFSIInstruction::sbefifo_open Problem opening SBEFIFO device %s : errno %d\n", device, errno);
        o_status.errorMessage.append(errstr);
        return SERVER_INVALID_SBEFIFO_DEVICE;
    }

    return rc;
}

void ServerFSIInstruction::scan_ffdc_and_reset(Handle ** handle, InstructionStatus & o_status) {

    // system_scan_ffdc_extract
    //  append to o_status.errorMessage
    // system_scan_ffdc_unlock
    // system_scan_reset

    uint32_t rc = scan_close(*handle);
    *handle = NULL;
    if (rc) o_status.rc = SERVER_SCAN_CLOSE_FAIL;
}

void ServerFSIInstruction::scom_ffdc_and_reset(Handle ** handle, InstructionStatus & o_status) {

    uint32_t rc = 0;
    // system_scom_ffdc_extract
    //  append to o_status.errorMessage
    // system_scom_ffdc_unlock
    if (global_server_debug) printf("calling adal_scom_reset()\n");
#ifdef TESTING
    TEST_PRINT("adal_scom_reset((adal_t *) *handle, SCOMRESETENGINE);\n");
    rc = 0;
#else
    rc = adal_scom_reset((adal_t *) *handle, SCOMRESETENGINE);
#endif

    rc = scom_close(*handle);
    *handle = NULL;
    if (rc) o_status.rc = SERVER_SCOM_CLOSE_FAIL;
}

void ServerFSIInstruction::gp_reg_ffdc_and_reset(Handle ** handle, InstructionStatus & o_status) {

    // system_gp_reg_ffdc_extract
    //  append to o_status.errorMessage
    // system_gp_reg_ffdc_unlock
    // system_gp_reg_reset -- may not be available

    uint32_t rc = gp_reg_close(*handle);
    *handle = NULL;
    if (rc) o_status.rc = SERVER_SCOM_CLOSE_FAIL;

}

void ServerFSIInstruction::mbx_ffdc_and_reset(Handle ** handle, InstructionStatus & o_status) {

    // system_mbx_ffdc_extract
    //  append to o_status.errorMessage
    // system_mbx_ffdc_unlock
    // system_mbx_reset

    uint32_t rc = mbx_close(*handle);
    *handle = NULL;
    if (rc) o_status.rc = SERVER_SCOM_CLOSE_FAIL;

}

void ServerFSIInstruction::sbefifo_ffdc_and_reset(Handle ** handle, InstructionStatus & o_status)
{
    uint32_t rc = 0;
    // system_sbefifo_ffdc_extract
    //  append to o_status.errorMessage
    // system_sbefifo ffdc_unlock

    if (flags & INSTRUCTION_FLAG_SBEFIFO_RESET_ENABLE) {
        errno = 0;
#ifdef TESTING
        TEST_PRINT("adal_sbefifo_request_reset(*handle);\n");
#else
        rc = adal_sbefifo_request_reset((adal_t *) *handle);
#endif
        if (rc) {
            char errstr[200];
            snprintf(errstr, 200, "ServerFSIInstruction::sbefifo_ffdc_and_reset Reset of adal failed!\n");
            o_status.errorMessage.append(errstr);
            o_status.rc = SERVER_SBEFIFO_ADAL_RESET_FAIL;
        }
    }

    rc = sbefifo_close(*handle);
    *handle = NULL;
    if (rc) o_status.rc = SERVER_SBEFIFO_CLOSE_FAIL;
}

uint32_t ServerFSIInstruction::scan_close(Handle * handle)
{
#ifdef TESTING
    TEST_PRINT("adal_scan_close((adal_t *) handle);\n");
    return 0;
#else
    return adal_scan_close((adal_t *) handle);
#endif
}

uint32_t ServerFSIInstruction::scom_close(Handle * handle)
{
#ifdef TESTING
    TEST_PRINT("adal_scom_close((adal_t *) handle);\n");
    return 0;
#else
    return adal_scom_close((adal_t *) handle);
#endif
}

uint32_t ServerFSIInstruction::gp_reg_close(Handle * handle)
{
#ifdef TESTING
    TEST_PRINT("system_gp_reg_close((adal_t *) handle);\n");
    return 0;
#else
    return system_gp_reg_close((adal_t *) handle);
#endif
}

uint32_t ServerFSIInstruction::mbx_close(Handle * handle)
{
#ifdef TESTING
    TEST_PRINT("adal_mbx_close((adal_t *) handle);\n");
    return 0;
#else
    return adal_mbx_close((adal_t *) handle);
#endif
}

uint32_t ServerFSIInstruction::sbefifo_close(Handle * handle)
{
#ifdef TESTING
    TEST_PRINT("adal_sbefifo_close((adal_t *) handle);\n");
    return 0;
#else
    return adal_sbefifo_close((adal_t *) handle);
#endif
}

ssize_t ServerFSIInstruction::scan_write(Handle * i_handle, InstructionStatus & o_status)
{
        ssize_t rc = 0;
        unsigned long options = 0;
        unsigned long status = 0;

        if (!(flags & INSTRUCTION_FLAG_FSI_SCANHEADERCHECK)) options |= SCANNOHEADERCHECK;
        if (flags & INSTRUCTION_FLAG_FSI_SCANSETPULSE) options |= SCANSETPULSE;
        if (flags & INSTRUCTION_FLAG_FSI_SCANEXTRABCLOCK) options |= SCANEXTRABCLOCK;
        if (flags & INSTRUCTION_FLAG_FSI_SCANVIAPIB) options |= SCANVIAPIB;

#ifdef TESTING
        TEST_PRINT("adal_scan_write((adal_t *) i_handle, ecmdDataBufferImplementationHelper::getDataPtr(&data), %08X, %u, options, &status);\n", address, length);
        rc = length % 8 ? (length / 8) + 1 : length / 8;
#else
        rc = adal_scan_write((adal_t *) i_handle,
            ecmdDataBufferImplementationHelper::getDataPtr(&data),
            address, length, options, &status);
#endif

        o_status.data.setBitLength(32);
        o_status.data.setWord(0, status);

        return rc;
}

ssize_t ServerFSIInstruction::scan_read(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status)
{
        ssize_t rc = 0;
        unsigned long options = 0;
        unsigned long status = 0;

        if (!(flags & INSTRUCTION_FLAG_FSI_SCANHEADERCHECK)) options |= SCANNOHEADERCHECK;
        if (flags & INSTRUCTION_FLAG_FSI_SCANSETPULSE) options |= SCANSETPULSE;
        if (flags & INSTRUCTION_FLAG_FSI_SCANEXTRABCLOCK) options |= SCANEXTRABCLOCK;
        if (flags & INSTRUCTION_FLAG_FSI_SCANVIAPIB) options |= SCANVIAPIB;

#ifdef TESTING
        TEST_PRINT("adal_scan_read((adal_t *) i_handle, ecmdDataBufferImplementationHelper::getDataPtr(&o_data), %08X, %u, options, &status);\n", address, length);
        rc = length % 8 ? (length / 8) + 1 : length / 8;
#else
        rc = adal_scan_read((adal_t *) i_handle,
            ecmdDataBufferImplementationHelper::getDataPtr(&o_data),
            address, length, options, &status);
#endif

        o_status.data.setBitLength(32);
        o_status.data.setWord(0, status);

        return rc;
}

ssize_t ServerFSIInstruction::scom_write(Handle * i_handle, InstructionStatus & o_status, uint32_t i_index)
{
        ssize_t rc = 0;
        uint64_t l_address = address;
        if (flags & INSTRUCTION_FLAG_64BIT_ADDRESS) {
          l_address = address64;
        }
	uint64_t l_data = data.getDoubleWord(i_index);
	unsigned long l_status = 0;
#ifdef TESTING
        TEST_PRINT("adal_scom_write((adal_t *) i_handle, %016llX, %016llX, &status);\n", l_data, l_address);
        rc = 8;
#else
        rc = adal_scom_write((adal_t *) i_handle, &l_data, l_address, &l_status);
#endif

        // FIXME may need to check scom status from 1007 register
        o_status.data.setBitLength(32);
        o_status.data.setWord(0, l_status);

        return rc;
}

ssize_t ServerFSIInstruction::scom_write_under_mask(Handle * i_handle, InstructionStatus & o_status)
{
        ssize_t rc = 0;
        uint64_t l_address = address;
        if (flags & INSTRUCTION_FLAG_64BIT_ADDRESS) {
          l_address = address64;
        }
	uint64_t l_data = data.getDoubleWord(0);
	uint64_t l_mask = mask.getDoubleWord(0);
	unsigned long l_status = 0;
#ifdef TESTING
        TEST_PRINT("adal_scom_write_under_mask((adal_t *) i_handle, %016llX, %016llX, %016llX, &status);\n", l_data, l_address, l_mask);
        rc = 8;
#else
        rc = adal_scom_write_under_mask((adal_t *) i_handle, &l_data, l_address, &l_mask, &l_status);
#endif

        // FIXME may need to check scom status from 1007 register
        o_status.data.setBitLength(32);
        o_status.data.setWord(0, l_status);

        return rc;
}

ssize_t ServerFSIInstruction::scom_read(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status, uint32_t i_index)
{
        ssize_t rc = 0;
        uint64_t l_address = address;
        if (flags & INSTRUCTION_FLAG_64BIT_ADDRESS) {
          l_address = address64;
        }
	uint64_t l_data = 0;
	unsigned long l_status = 0;
#ifdef TESTING
        TEST_PRINT("adal_scom_read((adal_t *) i_handle, &l_data, %016llX, &status);\n", l_address);
        rc = 8;
#else
        rc = adal_scom_read((adal_t *) i_handle, &l_data, l_address, &l_status);
#endif

        // FIXME may need to check scom status from 1007 register
        o_status.data.setBitLength(32);
        o_status.data.setWord(0, l_status);

	o_data.setDoubleWord(i_index, l_data);

        return rc;
}

ssize_t ServerFSIInstruction::scan_get_register(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status)
{
        ssize_t rc = 0;
        unsigned long l_data = 0;
#ifdef TESTING
        TEST_PRINT("adal_scan_get_register((adal_t *) i_handle, %08X, &l_data);\n", address);
        rc = 4;
#else
        rc = adal_scan_get_register((adal_t *) i_handle, address, &l_data);
#endif
        o_data.setWord(0, l_data);
        return rc;
}

ssize_t ServerFSIInstruction::scom_get_register(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status)
{
        ssize_t rc = 0;
        unsigned long l_data = 0;
#ifdef TESTING
        TEST_PRINT("adal_scom_get_register((adal_t *) i_handle, %08X, &l_data);\n", address);
        rc = 4;
#else
        rc = adal_scom_get_register((adal_t *) i_handle, address, &l_data);
#endif
        o_data.setWord(0, l_data);
        return rc;
}

ssize_t ServerFSIInstruction::mbx_get_scratch_register(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status)
{
          ssize_t rc = 0;

          adal_mbx_scratch_t scratch = ADAL_MBX_SCRATCH_0;
          switch (address & 0x0FFFFFFF)
          {
            case 0x2838:
              scratch = ADAL_MBX_SCRATCH_0;
              break;
            case 0x2839:
              scratch = ADAL_MBX_SCRATCH_1;
              break;
            case 0x283A:
              scratch = ADAL_MBX_SCRATCH_2;
              break;
            case 0x283B:
              scratch = ADAL_MBX_SCRATCH_3;
              break;
            case 0x283C:
              scratch = ADAL_MBX_SCRATCH_4;
              break;
            case 0x283D:
              scratch = ADAL_MBX_SCRATCH_5;
              break;
            case 0x283E:
              scratch = ADAL_MBX_SCRATCH_6;
              break;
            case 0x283F:
              scratch = ADAL_MBX_SCRATCH_7;
              break;

            default:
              rc = o_status.rc = SERVER_FSI_SCOM_READ_FAIL;
              break;
          }

	  if (!rc)
          {
#ifdef TESTING
              TEST_PRINT("adal_mbx_scratch((adal_t *) i_handle, %d, MBX_IOCTL_READ_REG, ecmdDataBufferImplementationHelper::getDataPtr(&o_data));\n", scratch);
              rc = 0;
#else
              rc = adal_mbx_scratch((adal_t *) i_handle, scratch, MBX_IOCTL_READ_REG, ecmdDataBufferImplementationHelper::getDataPtr(&o_data));
              if (rc >= 0) rc = 0;
#endif
          }
          return rc;
}

ssize_t ServerFSIInstruction::mbx_get_register(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status)
{
        ssize_t rc = 0;
        uint32_t l_data = 0;
#ifdef TESTING
        TEST_PRINT("adal_mbx_get_register((adal_t *) i_handle, %08X, &l_data);\n", address);
        rc = 4;
#else
        rc = adal_mbx_get_register((adal_t *) i_handle, address, &l_data);
#endif
        o_data.setWord(0, l_data);
        return rc;
}

ssize_t ServerFSIInstruction::gp_reg_get(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status)
{
        ssize_t rc = 0;
        uint32_t l_data = 0;
#ifdef TESTING
        TEST_PRINT("system_gp_reg_get((adal_t *) i_handle, %08X, &l_data);\n", address);
#else
        rc = system_gp_reg_get((adal_t *) i_handle, address, &l_data);
#endif
        o_data.setWord(0, l_data);
        return rc;
}

ssize_t ServerFSIInstruction::sbefifo_get_register(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status)
{
        ssize_t rc = 0;
        unsigned long l_data = 0;
#ifdef TESTING
        TEST_PRINT("adal_sbefifo_get_register((adal_t *) i_handle, %08X, &l_data);\n", address);
        rc = 4;
#else
        rc = adal_sbefifo_get_register((adal_t *) i_handle, address, &l_data);
#endif
        o_data.setWord(0, l_data);
        return rc;
}

ssize_t ServerFSIInstruction::scan_set_register(Handle * i_handle, InstructionStatus & o_status)
{
        ssize_t rc = 0;
        unsigned long l_data = data.getWord(0);
#ifdef TESTING
        TEST_PRINT("adal_scan_set_register((adal_t *) i_handle, %08X, %08lX);\n", address, l_data);
        rc = 4;
#else
        rc = adal_scan_set_register((adal_t *) i_handle, address, l_data);
#endif
        return rc;
}

ssize_t ServerFSIInstruction::scom_set_register(Handle * i_handle, InstructionStatus & o_status)
{
        ssize_t rc = 0;
        unsigned long l_data = data.getWord(0);
#ifdef TESTING
        TEST_PRINT("adal_scom_set_register((adal_t *) i_handle, %08X, %08lX);\n", address, l_data);
        rc = 4;
#else
        rc = adal_scom_set_register((adal_t *) i_handle, address, l_data);
#endif
        return rc;
}

ssize_t ServerFSIInstruction::iic_set_register(Handle * i_handle, InstructionStatus & o_status)
{
        ssize_t rc = 0;
        unsigned long l_data = data.getWord(0);
#ifdef TESTING
        TEST_PRINT("adal_iic_set_register((adal_t *) i_handle, %08X, %08lX);\n", address, l_data);
        rc = 4;
#else
        rc = adal_iic_set_register((adal_t *) i_handle, address, l_data);
#endif
        return rc;
}

ssize_t ServerFSIInstruction::iic_get_register(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status)
{
        ssize_t rc = 0;
        unsigned long l_data = 0;
#ifdef TESTING
        TEST_PRINT("adal_iic_get_register((adal_t *) i_handle, %08X, &l_data);\n", address);
        rc = 4;
#else
        rc = adal_iic_get_register((adal_t *) i_handle, address, &l_data);
#endif
        o_data.setWord(0, l_data);
        return rc;
}

ssize_t ServerFSIInstruction::mbx_set_scratch_register(Handle * i_handle, InstructionStatus & o_status)
{
          ssize_t rc = 0;

          adal_mbx_scratch_t scratch = ADAL_MBX_SCRATCH_0;
          switch (address & 0x0FFFFFFF)
          {
            case 0x2838:
              scratch = ADAL_MBX_SCRATCH_0;
              break;
            case 0x2839:
              scratch = ADAL_MBX_SCRATCH_1;
              break;
            case 0x283A:
              scratch = ADAL_MBX_SCRATCH_2;
              break;
            case 0x283B:
              scratch = ADAL_MBX_SCRATCH_3;
              break;
            case 0x283C:
              scratch = ADAL_MBX_SCRATCH_4;
              break;
            case 0x283D:
              scratch = ADAL_MBX_SCRATCH_5;
              break;
            case 0x283E:
              scratch = ADAL_MBX_SCRATCH_6;
              break;
            case 0x283F:
              scratch = ADAL_MBX_SCRATCH_7;
              break;

            default:
              rc = o_status.rc = SERVER_FSI_SCOM_WRITE_FAIL;
              break;
          }

	  if (!rc)
          {
#ifdef TESTING
              TEST_PRINT("adal_mbx_scratch((adal_t *) i_handle, %d, MBX_IOCTL_WRITE_REG, %08X);\n", scratch, data.getWord(0));
              rc = 0;
#else
              rc = adal_mbx_scratch((adal_t *) i_handle, scratch, MBX_IOCTL_WRITE_REG, ecmdDataBufferImplementationHelper::getDataPtr(&data));
              if (rc >= 0) rc = 0;
#endif
          }
          return rc;
}

ssize_t ServerFSIInstruction::mbx_set_register(Handle * i_handle, InstructionStatus & o_status)
{
        ssize_t rc = 0;
        unsigned long l_data = data.getWord(0);
#ifdef TESTING
        TEST_PRINT("adal_mbx_set_register((adal_t *) i_handle, %08X, %08lX);\n", address, l_data);
        rc = 4;
#else
        rc = adal_mbx_set_register((adal_t *) i_handle, address, l_data);
#endif
        return rc;
}

ssize_t ServerFSIInstruction::gp_reg_set(Handle * i_handle, InstructionStatus & o_status)
{
        ssize_t rc = 0;
        unsigned long l_data = data.getWord(0);
#ifdef TESTING
        TEST_PRINT("system_gp_reg_set((adal_t *) i_handle, %08X, %08lX);\n", address, l_data);
#else
        rc = system_gp_reg_set((adal_t *) i_handle, address, l_data);
#endif
        return rc;
}

ssize_t ServerFSIInstruction::sbefifo_set_register(Handle * i_handle, InstructionStatus & o_status)
{
        ssize_t rc = 0;
        unsigned long l_data = data.getWord(0);
#ifdef TESTING
        TEST_PRINT("adal_sbefifo_set_register((adal_t *) i_handle, %08X, %08lX);\n", address, l_data);
        rc = 4;
#else
        rc = adal_sbefifo_set_register((adal_t *) i_handle, address, l_data);
#endif
        return rc;
}
