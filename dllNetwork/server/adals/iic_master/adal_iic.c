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

/*
 *   Desc: IIC Access Device Abstraction Layer (a.k.a ADAL).
 */

#include <adal_iic.h>
#include <iic_dd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <arpa/inet.h>

#define ADAL_IIC_ENGINE_OFFSET	0x1800

/*
 * Secure IIC
 */
#define SIIC_CMD_COPY 0xE1      /* copy main to staging */
#define SIIC_CMD_STORE 0xD2     /* copy staging to main */
#define SIIC_CMD_CLRINT 0xB4    /* clear selected IRQ's */
#define SIIC_CMD_OFFSET 0x00FF  /* Command register offset */
#define SIIC_CMD_LEN    1       /* Size of a Secure IIC command */

#define SIIC_MK_OFFSET(t) (((t) << 8) | ((~(t)) & 0x000000ff))

/*******************************************************************************/


adal_t *adal_iic_open(const char *device, int flags)
{
	return adal_base_open(device, flags);
}

int adal_iic_close(adal_t * adal)
{
	return adal_base_close(adal);
}

ssize_t adal_iic_read(adal_t * adal, void * buf, size_t count)
{
	int rc = -1;
	int total = 0;
	uint8_t * l_buf = (uint8_t *) buf;

	/* buf/count are validated in libc (sysdeps/generic/read.c) */

	do {
		rc = read(adal->fd, l_buf, count) ;
		if (rc == -1) return rc;
		if (rc == 0) break;

		count -= rc;

		l_buf += rc;
		total += rc;
	} while (count != 0);
	rc = total;

	return rc;
}

ssize_t adal_iic_write(adal_t * adal, const void * buf, size_t count)
{
	int rc = -1;
	int total = 0;
	uint8_t * l_buf = (uint8_t *) buf;

	/* buf/count are validated in libc (sysdeps/generic/write.c) */

	do {
		rc = write(adal->fd, l_buf, count);
		if (rc == -1) return rc;
		if (rc == 0) break;

		count -= rc;
		l_buf += rc;
		total += rc;

	} while (count != 0);
	rc = total;

	return rc;
}


ssize_t adal_iic_ffdc_extract(adal_t * adal, int scope, void ** buf)
{
	return 0;
}

int adal_iic_ffdc_unlock(adal_t * adal, int scope)
{
	return 0;
}

int adal_iic_reset(adal_t * adal, int type)
{
        return 0;
}



/******************************************************************************
 * size parameter is ignored by adal and device driver.
 */
int adal_iic_config(adal_t * adal, int type, adal_iic_cfg_t cfg, void * value,
		    size_t size)
{
    int rc = -1;
    errno = ENOSYS;
    int _ioc;

    /* things to consider: */
    /*   1) interrupted syscall (ie. errno == EINTR) */
    /*   2) unusual errno's: EAGAIN, EFBIG, ENOSPC, EIO */

    if(cfg > (adal_iic_cfg_t)IIC_IOC_MAXNR)
    {
	    /* Can't handle the cfg value
	     */
	    goto exit;
    }

    if(type == ADAL_CONFIG_WRITE)
    {
	    _ioc = _IOW(IIC_IOC_MAGIC, cfg, long);
    }
    else if(type == ADAL_CONFIG_READ)
    {
	    _ioc = _IOR(IIC_IOC_MAGIC, cfg, long);
    }
    else
    {
        errno = -EINVAL;
        goto exit;
    }
    do{
        rc = ioctl(adal->fd, _ioc, value);
    }while ((rc < 0) && (errno == EINTR));

exit:
    return rc;
}

/* this method expects the adal to be opened against the raw fsi device for this target */
ssize_t adal_iic_get_register(adal_t *adal, int registerNo,
			       unsigned long *data)
{

	int rc;
   
	lseek(adal->fd, ADAL_IIC_ENGINE_OFFSET + ((registerNo & ~0xFFFFFF00) * 4), SEEK_SET);
	rc = read(adal->fd, data, 4);

        if (adal_is_byte_swap_needed())
        {
                // based on device and openbmc version we know the driver is not swapping endianess
                (*data) = ntohl((*data));
        }
	
	return rc;
}

/* this method expects the adal to be opened against the raw fsi device for this target */
ssize_t adal_iic_set_register(adal_t *adal, int registerNo,
	                        unsigned long data)
{
	int rc;
        
	lseek(adal->fd, ADAL_IIC_ENGINE_OFFSET + ((registerNo & ~0xFFFFFF00) * 4), SEEK_SET);
	if (adal_is_byte_swap_needed())
	{
			// based on device and openbmc version we know the driver is not swapping endianess
			data = htonl(data);
	}
	
	rc = write(adal->fd, &data, 4);
	
	return rc;
}


