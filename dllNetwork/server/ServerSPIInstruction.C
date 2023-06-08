//IBM_PROLOG_BEGIN_TAG
/* 
 * Copyright 2003,2018 IBM International Business Machines Corp.
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

#include <ServerSPIInstruction.H>
#include <OutputLite.H>
#include <stdio.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <delay.h>

#include <adal_base.h>

extern bool global_server_debug;

uint32_t ServerSPIInstruction::spi_open(Handle ** handle, InstructionStatus & o_status)
{
    uint32_t rc = 0;
    char device[50];
    char errstr[200];

    if(*handle != NULL)
      return rc;

    /* Open the device */
    snprintf(device, 50, "/sys/class/spi_master/spi%s%d/spi%s%d.0/eeprom", deviceString.c_str(), engineId, deviceString.c_str(), engineId);

    errno = 0;

    if (flags & INSTRUCTION_FLAG_SERVER_DEBUG)
    {
        snprintf(errstr, 200, "SERVER_DEBUG : adal_base_open(%s, O_RDWR | O_SYNC)\n", device);
        o_status.errorMessage.append(errstr);
    }

#ifdef TESTING
    TEST_PRINT("*handle = adal_base_open(%s, O_RDWR | O_SYNC);\n", device);
    *handle = (Handle *) 55;
#else
    *handle = (Handle *) adal_base_open(device, O_RDWR | O_SYNC);
#endif
    if (flags & INSTRUCTION_FLAG_SERVER_DEBUG) {
        snprintf(errstr, 200, "SERVER_DEBUG : adal_base_open() *handle = %p, errno %d\n", *handle, errno);
        o_status.errorMessage.append(errstr);
    }

    if ( *handle == NULL || (*handle != NULL && ((adal_t *)handle)->fd < 0) ) {
        snprintf(errstr, 200, "ServerSPIInstruction::spi_open Problem opening SPI device %s : errno %d\n", device, errno);
        o_status.errorMessage.append(errstr);
        rc = o_status.rc = SERVER_SPI_OPEN_FAIL;
        return rc;
    }

    return rc;
}

void ServerSPIInstruction::spi_ffdc(Handle ** handle, InstructionStatus & o_status)
{
    // adal_iic_ffdc_extract
    //  append to o_status.errorMessage
    // adal_iic_ffdc_unlock

    uint32_t rc = spi_close(*handle);
    if (rc) o_status.rc = SERVER_SPI_CLOSE_FAIL;
}

uint32_t ServerSPIInstruction::spi_close(Handle * handle)
{
    uint32_t rc = 0;
    // close iic device
#ifdef TESTING
    TEST_PRINT("adal_base_close()\n");
#else
    rc = adal_base_close((adal_t *) handle);
#endif
    if( rc )
        rc = SERVER_SPI_CLOSE_FAIL;

    return rc;
}

ssize_t ServerSPIInstruction::spi_command(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status)
{
    ssize_t rc = 0;
    char errstr[200];

    int fd = ((adal_t *)i_handle)->fd;
    uint32_t readBytes = readLength % 8 ? (readLength / 8) + 1 : readLength / 8;
    uint8_t *l_buf = NULL;

    do
    {
        if ( (data == NULL) || (data->getByteLength() < commandLengthBytes) )
        {
            snprintf(errstr, 200, "ServerSPIInstruction::spi_command no command data\n");
            o_status.errorMessage.append(errstr);
            rc = -1;
            break;
        }

        errno = 0;

        uint32_t addr = data->getWord(0) & 0xFFFFFF;
        if (flags & INSTRUCTION_FLAG_SERVER_DEBUG)
        {
            snprintf(errstr, 200, "SERVER_DEBUG : lseek(fd, 0x%08X, SEEK_SET)\n", addr);
            o_status.errorMessage.append(errstr);
        }
#ifdef TESTING
        TEST_PRINT("off_t l_skVal = lseek(fd, (off_t)%08X, SEEK_SET)\n", addr);
#else
        off_t l_skVal = lseek(fd, (off_t)addr, SEEK_SET);
#endif

        if (flags & INSTRUCTION_FLAG_SERVER_DEBUG)
        {
            snprintf(errstr, 200, "SERVER_DEBUG : lseek() rc = %08X, errno = %d\n", (uint32_t)l_skVal, errno);
            o_status.errorMessage.append(errstr);
        }

        // read stuff here
        if ( readLength )
        {
            errno = 0;

            l_buf = new uint8_t[readBytes];
            if (l_buf == NULL)
            {
                snprintf(errstr, 200, "ServerSPIInstruction::spi_command could not allocate l_buf\n");
                o_status.errorMessage.append(errstr);
                rc = -1;
                break;
            }
            
            if (flags & INSTRUCTION_FLAG_SERVER_DEBUG)
            {
                snprintf(errstr, 200, "SERVER_DEBUG : read(fd, buf, %d)\n", readBytes);
                o_status.errorMessage.append(errstr);
            }

#ifdef TESTING
            TEST_PRINT("rc = read(fd, l_buf, (size_t) %d)\n", readBytes);
            rc = readBytes;
#else
            rc = read(fd, l_buf, (size_t) readBytes);
#endif

            if (flags & INSTRUCTION_FLAG_SERVER_DEBUG)
            {
                snprintf(errstr, 200, "SERVER_DEBUG : read() rc = %zd, errno = %d\n", rc, errno);
                o_status.errorMessage.append(errstr);
            }

            if ( rc < 0 )
            {
                snprintf(errstr, 200, "ServerSPIInstruction::spi_command error during read: rc %zd, errno: %d\n", rc, errno);
                o_status.errorMessage.append(errstr);
                break;
            }

            if ( rc != (ssize_t)readBytes )
            {
                snprintf(errstr, 200, "ServerSPIInstruction::spi_command error during read: rc %zd != readBytes(%d)\n", rc, readBytes);
                o_status.errorMessage.append(errstr);
                break;
            }

            rc = readBytes;
        }
        else
        // write stuff here
        {
            errno = 0;

            // correct byte length to remove the command word
            uint32_t writeBytes = data->getByteLength() - commandLengthBytes;
            l_buf = new uint8_t[writeBytes];
            uint32_t rc_ecmd = data->extract((uint8_t *) l_buf, commandLengthBytes*8, writeBytes*8);
            if (rc_ecmd)
            {
                snprintf(errstr, 200, "ServerSPIInstruction::spi_command error during extract: %08X, length %d\n", rc_ecmd, writeBytes);
                o_status.errorMessage.append(errstr);
                rc = -1;
                break;
            }

            if (flags & INSTRUCTION_FLAG_SERVER_DEBUG)
            {
                snprintf(errstr, 200, "SERVER_DEBUG : write(fd, buf, %d)\n", writeBytes);
                o_status.errorMessage.append(errstr);
            }

#ifdef TESTING
            TEST_PRINT("rc = write(fd, l_buf, (size_t) %d)\n", writeBytes);
            rc = writeBytes;
#else
            rc = write(fd, l_buf, (size_t) writeBytes);
#endif

            if (flags & INSTRUCTION_FLAG_SERVER_DEBUG)
            {
                snprintf(errstr, 200, "SERVER_DEBUG : write() rc = %zd, errno = %d\n", rc, errno);
                o_status.errorMessage.append(errstr);
            }

            if ( rc < 0 )
            {
                snprintf(errstr, 200, "ServerSPIInstruction::spi_command error during write: rc %zd, errno: %d\n", rc, errno);
                o_status.errorMessage.append(errstr);
                break;
            }

            if ( rc != (ssize_t)writeBytes )
            {
                snprintf(errstr, 200, "ServerSPIInstruction::spi_command error during write: rc %zd != writeBytes(%d)\n", rc, writeBytes);
                o_status.errorMessage.append(errstr);
                break;
            }

            rc = writeBytes;
        }

        if (readLength)
        {
            uint32_t rc_ecmd = o_data.insert((uint8_t *) l_buf, 0, readLength);
            if (rc_ecmd)
            {
                snprintf(errstr, 200, "ServerSPIInstruction::spi_command error during insert: %08X, readLength %d\n", rc_ecmd, readLength);
                o_status.errorMessage.append(errstr);
                rc = -1;
                break;
            }
        }

        if (msDelay)
        {
            uint64_t l_nsDelay = 1000000 * msDelay;
            delay(l_nsDelay);
        }
    } while (0);

    if (l_buf)
        delete [] l_buf;

    return rc;
}
