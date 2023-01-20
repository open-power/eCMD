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

#include "adal_mbx.h"
#include <sys/ioctl.h>
#include <stdlib.h>             /* NULL, malloc, free */
#include <unistd.h>             /* close, read, write, fsync */
#include <string.h>             /* memset, memcpy */
#include <arpa/inet.h>


/* GLOBAL VARIABLES ARE BAD IN SHARED LIBRARIES, DON'T USE THEM! */
#define ADAL_MBX_MAX_STR	128
#define ADAL_MBX_FSI_OFFSET 0x2800;

adal_t * adal_mbx_open(const char * device, int flags) {
	return adal_base_open(device, flags);
}

int adal_mbx_close(adal_t * adal) {
  return adal_base_close(adal);
}


int adal_mbx_reset(adal_t * adal, int type) {
  return 0;
}
ssize_t adal_mbx_ffdc_extract(adal_t * adal, int scope, void ** buf)
{
  *buf = NULL;
	return 0;
}

int adal_mbx_ffdc_unlock(adal_t * adal, int scope)
{
	return 0;
}

struct mbx_reg_data {
       uint32_t offset;        /* What address */
       uint32_t value;         /* value at address */
};

int adal_mbx_scratch(adal_t *adal,
					 adal_mbx_scratch_t  scratch,
					 adal_mbx_gpreg_mode_t mode,
					 uint32_t * value) {
	int rc = -1;
  unsigned long reg_address;

	if( (NULL != value) ) {
		reg_address = (unsigned long)scratch;
    // multiply is done in function calls after
    //reg_address *=4;
    reg_address += 0x38;

		if( MBX_IOCTL_READ_REG == mode ) {
        rc = adal_mbx_get_register(adal, reg_address, value);
    }
    else {
      rc = adal_mbx_set_register(adal, reg_address, *value);
    }

	} else {
		errno = EBADF;
	}
	return rc;
}

/* this method expects the adal to be opened against the raw fsi device for this target */
int adal_mbx_get_register(adal_t * adal, unsigned long reg,
		       uint32_t * value)
{
    int rc = -1;
    unsigned long reg_address;
    struct mbx_reg_data parms;
    memset(&parms, 0, sizeof(parms));

    if( (NULL != value) ) {
        reg_address = reg;
        reg_address &= ~0xFFFFFE00;
        reg_address *=4;
        reg_address += ADAL_MBX_FSI_OFFSET;
        //  parms.offset = reg_address;
        /**
        * Poison the read value in case we fail and user does not
        * check return codes.
        */
        *value = 0xA5A5A5A5;
        //parms.value = *value;
        lseek(adal->fd, reg_address, SEEK_SET);
        rc = read(adal->fd, value, sizeof(uint32_t));

        //rc = ioctl(adal->fd, MBX_IOCTL_READ_REG, &parms);
        //if( 0 == rc ){
        //	*value = parms.value;
        //    }a
        if (adal_is_byte_swap_needed())
        {
            // based on version we know the driver is not swapping endianess
            (*value) = ntohl((*value));
        }
    } else {
        errno = EBADF;
    }
    return rc;
}

/* this method expects the adal to be opened against the raw fsi device for this target */
int adal_mbx_set_register(adal_t *adal, unsigned long reg,
		       uint32_t value)
{
    int rc = -1;
    unsigned long reg_address;
    struct mbx_reg_data parms;
    memset(&parms, 0, sizeof(parms));

    reg_address = reg;
    reg_address &= ~0xFFFFFE00;
    reg_address *=4;
    reg_address += ADAL_MBX_FSI_OFFSET;
    lseek(adal->fd, reg_address, SEEK_SET);
    if (adal_is_byte_swap_needed())
    {
        // based on device we know the driver is not swapping endianess
        value = htonl(value);
    }
    rc = write(adal->fd, &value, sizeof(uint32_t));


//  parms.offset = reg_address;
//  parms.value = value;
//  rc = ioctl(adal->fd, MBX_IOCTL_WRITE_REG, &parms);

    return rc;
}


