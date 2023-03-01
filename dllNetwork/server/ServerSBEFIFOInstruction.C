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

#include <ServerSBEFIFOInstruction.H>
#include <OutputLite.H>
#include <stdio.h>
#include <errno.h>
#include <sstream>

#include <adal_sbefifo.h>

uint32_t ServerSBEFIFOInstruction::sbefifo_open(Handle ** handle, InstructionStatus & o_status)
{
    int rc = 0;

    char device[50];
    char errstr[200];

    /* already have a handle lets reuse it */
    if(*handle != NULL)
        return rc;

    /* We need to open the device*/
    if( version == 0x3 ) 
    {
        if( port != UINT32_MAX )
            snprintf(device, 50, "/dev/sbefifo%s%02d", deviceString.c_str(), port);
        else
            snprintf(device, 50, "/dev/sbefifo%s", deviceString.c_str());
    }
    else snprintf(device, 50, "/dev/sbefifo%s", deviceString.c_str());
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
        snprintf(errstr, 200, "ServerSBEFIFOInstruction::sbefifo_open Problem opening SBEFIFO device %s : errno %d\n", device, errno);
        o_status.errorMessage.append(errstr);
        return SERVER_INVALID_SBEFIFO_DEVICE;
    }

    return rc;
}

uint32_t ServerSBEFIFOInstruction::fsi_open(Handle** handle, InstructionStatus & o_status) {
  uint32_t rc = 0;

  char errstr[200];

  /* already have a handle lets reuse it */
  if(*handle != NULL)
    return rc;

  /* We need to open the device*/
  if ((flags & INSTRUCTION_FLAG_DEVSTR) == 0) {
    //ERROR
    *handle = NULL;
    snprintf(errstr, 200, "ServerSBEFIFOInstruction::fsi_open INSTRUCTION_FLAG_DEVSTR must be set\n");
    o_status.errorMessage.append(errstr);
    return SERVER_INVALID_FSI_DEVICE;
  }

  if (version == 0x3)
  {
    *handle = NULL;
    snprintf(errstr, 200, "ServerSBEFIFOInstruction::fsi_open instruction version %d is invalid for raw fsi access\n", version);
    o_status.errorMessage.append(errstr);
    return SERVER_UNKNOWN_INSTRUCTION_VERSION;
  }

  /* Figure out the slave device based on the deviceString */
  std::istringstream iss;
  iss.str(deviceString);
  uint32_t l_idx = 0;
  iss >> l_idx;

  if ( (l_idx < 1) || (l_idx > 8) ) {
    *handle = NULL;
    snprintf(errstr, 200, "ServerSBEFIFOInstruction::fsi_open deviceString %s is not valid\n", deviceString.c_str());
    o_status.errorMessage.append(errstr);
    return SERVER_INVALID_FSI_DEVICE;
  }

  if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
    snprintf(errstr, 200, "SERVER_DEBUG : adal_fsi_open()\n");
    o_status.errorMessage.append(errstr);
  }

  *handle = (Handle *) adal_fsi_open(l_idx, O_RDWR | O_SYNC);

  if (*handle == NULL) {
    return SERVER_FSI_OPEN_FAIL;
  }

  if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
    snprintf(errstr, 200, "SERVER_DEBUG : adal_fsi_open() opened = %s\n", (char *)((adal_t *)*handle)->priv);
    o_status.errorMessage.append(errstr);
  }

  return rc;
}

uint32_t ServerSBEFIFOInstruction::fsi_close(Handle * handle)
{
#ifdef TESTING
    TEST_PRINT("adal_base_close((adal_t *) handle);\n");
    return 0;
#else
    return adal_base_close((adal_t *) handle);
#endif
}

void ServerSBEFIFOInstruction::sbefifo_ffdc_and_reset(Handle ** handle, InstructionStatus & o_status)
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
        Handle * l_fsiHandle = NULL;
        rc = fsi_open(&l_fsiHandle, o_status);
        if( l_fsiHandle != NULL )
            rc = adal_sbefifo_request_reset((adal_t *) l_fsiHandle);
#endif
        if (rc) {
            char errstr[200];
            snprintf(errstr, 200, "ServerSBEFIFOInstruction::sbefifo_ffdc_and_reset Reset of adal failed!\n");
            o_status.errorMessage.append(errstr);
            o_status.rc = SERVER_SBEFIFO_ADAL_RESET_FAIL;
        }
        fsi_close(l_fsiHandle);
    }

    rc = sbefifo_close(*handle);
    *handle = NULL;
    if (rc) o_status.rc = SERVER_SBEFIFO_CLOSE_FAIL;
}

uint32_t ServerSBEFIFOInstruction::sbefifo_close(Handle * handle)
{
#ifdef TESTING
    TEST_PRINT("adal_sbefifo_close((adal_t *) handle);\n");
    return 0;
#else
    return adal_sbefifo_close((adal_t *) handle);
#endif
}

ssize_t ServerSBEFIFOInstruction::sbefifo_submit(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status, uint32_t & o_reply_wordcount)
{
            ssize_t rc = 0;

            adal_sbefifo_request l_request;
            adal_sbefifo_reply l_reply;

            l_request.data = ecmdDataBufferImplementationHelper::getDataPtr(&data);
            l_request.wordcount = data.getWordLength();
            l_request.status = 0;

            l_reply.data = ecmdDataBufferImplementationHelper::getDataPtr(&o_data);
            l_reply.wordcount = o_data.getWordLength();
            l_reply.status = 0;

#ifdef TESTING
            TEST_PRINT("adal_sbefifo_submit((adal_t *) i_handle, &l_request, &l_reply, %d);\n", timeout);
            rc = 0;
#else
            rc = adal_sbefifo_submit((adal_t *) i_handle, &l_request, &l_reply, timeout);
#endif

            if (flags & INSTRUCTION_FLAG_SERVER_DEBUG)
            {
                char errstr[200];
                snprintf(errstr, 200, "SERVER_DEBUG : adal_sbefifo_submit() rc = %u\n", (uint32_t) rc);
                o_status.errorMessage.append(errstr);
                snprintf(errstr, 200, "SERVER_DEBUG : l_request.status = %08X\n", (uint32_t) l_request.status);
                o_status.errorMessage.append(errstr);
                snprintf(errstr, 200, "SERVER_DEBUG : l_request.wordcount = %zu\n", l_request.wordcount);
                o_status.errorMessage.append(errstr);
                snprintf(errstr, 200, "SERVER_DEBUG : l_reply.status = %08X\n", (uint32_t) l_reply.status);
                o_status.errorMessage.append(errstr);
                snprintf(errstr, 200, "SERVER_DEBUG : l_reply.wordcount = %zu\n", l_reply.wordcount);
                o_status.errorMessage.append(errstr);
            }

            o_status.data.setBitLength(64);
            o_status.data.setWord(0, (uint32_t) l_request.status);
            o_status.data.setWord(1, (uint32_t) l_reply.status);

            o_reply_wordcount = l_reply.wordcount;
            return rc;
}
