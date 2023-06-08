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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include "adal_scom.h"
#include "../fsi.h"

#define container_of(ptr, type, member) ({                      \
	const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
	(type *)( (char *)__mptr - offsetof(type,member) );})

enum SCOM_REGS {
	DATA0 = 0,
	DATA1,
	CMD,
	FSI2PIB_RESET = 6,
	STATUS,
	RESET = STATUS,
	JTAG_EMU = 9,
	CHIPID,
	IRQ,
	COMP_MASK,
	TRUE_MASK
};

#define SCOM_ENGINE_OFFSET	0x1000
#define P1_SLAVE_OFFSET		0x100000
#define PX_SLAVE_OFFSET         0x80000

#define DIRECT_SCOM_ADDR	0x7FFFFFFFULL
#define FORM1_SCOM		0x1000000000000000ULL
#define FORM1_SCOM_ADDR		0x00000FFF00000000ULL
#define FORM1_SCOM_SHIFT	20
#define FORM1_SCOM_DATA		0x000FFFFFFFFFFFFFULL
#define INDIRECT_SCOM_READ	0x8000000000000000ULL
#define INDIRECT_SCOM_FLAG	0x8000000000000000ULL
#define INDIRECT_SCOM_ADDR	0x000FFFFF00000000ULL
#define INDIRECT_SCOM_READ_DATA	0xFFFFULL

struct adal_scom {
	adal_t adal;
	bool has_ioctl;
};
typedef struct adal_scom adal_scom_t;

#define to_scom_adal(x) container_of((x), struct adal_scom, adal)

/* PIB Status register bits */
#define SCOM_STATUS_ERR_SUMMARY		0x80000000
#define SCOM_STATUS_PROTECTION		0x01000000
#define SCOM_STATUS_PARITY		0x04000000
#define SCOM_STATUS_PIB_ABORT		0x00100000
#define SCOM_STATUS_PIB_RESP_MASK	0x00007000
#define SCOM_STATUS_PIB_RESP_SHIFT	12

/* Rebuild the "PIB status register" based on kernel response */
static void adal_scom_make_status(struct scom_access *acc, unsigned long *status)
{
	*status = ((unsigned long)acc->pib_status) << SCOM_STATUS_PIB_RESP_SHIFT;
	if (acc->intf_errors & SCOM_INTF_ERR_PROTECTION)
		*status |= SCOM_STATUS_PROTECTION;
	if (acc->intf_errors & SCOM_INTF_ERR_PARITY)
		*status |= SCOM_STATUS_PARITY;
	if (acc->intf_errors & SCOM_INTF_ERR_ABORT)
		*status |= SCOM_STATUS_PIB_ABORT;
	if (acc->intf_errors & SCOM_INTF_ERR_UNKNOWN)
		*status |= SCOM_STATUS_ERR_SUMMARY;
}

adal_t * adal_scom_open(const char * device, int flags)
{
	adal_scom_t *scom;

	scom = (adal_scom_t *)malloc(sizeof(*scom));
	if (scom == NULL) {
		return NULL;
	}

	memset(scom, 0, sizeof(*scom));

	scom->adal.fd = open(device, flags);
	if (scom->adal.fd == -1) {
		free(scom);
		scom = NULL;
		return NULL;
	}

    // checking if ioctl works here 
	uint32_t check = 0;
	int rc = ioctl(scom->adal.fd, FSI_SCOM_CHECK, &check);
	if (rc == 0 && (check & SCOM_CHECK_SUPPORTED))
		scom->has_ioctl = true;

	return &scom->adal;
}

int adal_scom_close(adal_t * adal)
{
	int rc = -1;
	adal_scom_t *scom = to_scom_adal(adal);

	if (!adal)
		return 0;

	rc = close(adal->fd);

	free(scom);
	adal = NULL;

	return rc;

}

ssize_t adal_scom_legacy_read(adal_t * adal, void * buf, uint64_t scom_address, unsigned long * status)
{
	int rc = 0;

	if (scom_address & INDIRECT_SCOM_FLAG) {
		uint64_t read_data;
		uint64_t data = (scom_address & INDIRECT_SCOM_ADDR) | INDIRECT_SCOM_READ;

		lseek(adal->fd, (uint32_t)(scom_address & DIRECT_SCOM_ADDR), SEEK_SET);
		rc = write(adal->fd, &data, SCOM_DATA_LEN);

		lseek(adal->fd, (uint32_t)(scom_address & DIRECT_SCOM_ADDR), SEEK_SET);
		rc = read(adal->fd, &data, SCOM_DATA_LEN);

		read_data = data & INDIRECT_SCOM_READ_DATA;

		memcpy(buf, &read_data, SCOM_DATA_LEN);
	}
	else {
		lseek64(adal->fd, scom_address, SEEK_SET);
		rc = read(adal->fd, buf, SCOM_DATA_LEN);
	}

        // ignore rc as status is checked later
	adal_scom_get_register(adal, STATUS, status);

	return rc;
}

ssize_t adal_scom_read(adal_t * adal, void * buf, uint64_t scom_address, unsigned long * status)
{
	adal_scom_t *scom = to_scom_adal(adal);
	struct scom_access acc = {};
	int rc;

	if (!scom->has_ioctl)
		return adal_scom_legacy_read(adal, buf, scom_address, status);

	acc.addr = scom_address;
	acc.mask = 0;
	rc = ioctl(adal->fd, FSI_SCOM_READ, &acc);
	if (rc)
		return rc;
	memcpy(buf, &acc.data, sizeof(__u64));
	adal_scom_make_status(&acc, status);

	return 8;
}

static ssize_t adal_scom_legacy_write(adal_t * adal, void * buf, uint64_t scom_address,  unsigned long * status)
{
	int rc = 0;

	if (scom_address & INDIRECT_SCOM_FLAG) {
		uint64_t data;

		if (scom_address & FORM1_SCOM) {
			uint64_t form1_write_data;

			memcpy(&form1_write_data, buf, 8);

			data = (scom_address & FORM1_SCOM_ADDR) << FORM1_SCOM_SHIFT;
			data |= (form1_write_data & FORM1_SCOM_DATA);
		}
		else {
			uint64_t write_data;

			memcpy(&write_data, buf, SCOM_DATA_LEN);

			data = scom_address & INDIRECT_SCOM_ADDR;
			data |= write_data;
		}

		lseek(adal->fd, (uint32_t)(scom_address & DIRECT_SCOM_ADDR), SEEK_SET);
		rc = write(adal->fd, &data, SCOM_DATA_LEN);
	}
	else {
		lseek64(adal->fd, scom_address, SEEK_SET);
		rc = write(adal->fd, buf, SCOM_DATA_LEN);
	}

        // ignore rc as status is checked later
	adal_scom_get_register(adal, STATUS, status);

	return rc;
}

ssize_t adal_scom_write(adal_t * adal, void * buf, uint64_t scom_address,  unsigned long * status)
{
	adal_scom_t *scom = to_scom_adal(adal);
	struct scom_access acc = {};
	int rc;

	if (!scom->has_ioctl)
		return adal_scom_legacy_write(adal, buf, scom_address, status);

	acc.addr = scom_address;
	memcpy(&acc.data, buf, sizeof(__u64));
	acc.mask = 0;
	rc = ioctl(adal->fd, FSI_SCOM_WRITE, &acc);
	if (rc)
		return rc;
	adal_scom_make_status(&acc, status);

	return 8;
}

int adal_scom_reset_with_fsi(adal_t * adal, adal_t * adal_fsi, scom_adal_reset_t type)
{
	adal_scom_t *scom = to_scom_adal(adal);
	__u32 reset_type;
	int rc;

	switch(type) {
	case SCOMRESETENGINE:
		reset_type = SCOM_RESET_INTF;
		break;
	case SCOMRESETFULL:
		reset_type = SCOM_RESET_INTF | SCOM_RESET_PIB;
		break;
	default:
		return -1;
	}

	if (scom->has_ioctl) {
		rc = ioctl(adal->fd, FSI_SCOM_RESET, &reset_type);
	} else {
		// use the raw fsi device here.
		if (reset_type & SCOM_RESET_PIB)
			rc = adal_scom_set_register(adal_fsi, RESET, 0xFFFFFFFF);
		if (reset_type & SCOM_RESET_INTF)
			rc = adal_scom_set_register(adal_fsi, FSI2PIB_RESET, 0xFFFFFFFF);
	}

	return rc;
}

static ssize_t adal_scom_legacy_write_under_mask(adal_t * adal, void * buf, uint64_t scom_address, void * mask, unsigned long * status)
{
	int rc = 0;
	unsigned long old_data[SCOM_DATA_WORDS];
	unsigned long new_data[SCOM_DATA_WORDS];
	unsigned long userbuffer[SCOM_DATA_WORDS];
	unsigned long usermask[SCOM_DATA_WORDS];

	memcpy(userbuffer, buf, SCOM_DATA_LEN);
	memcpy(usermask, mask, SCOM_DATA_LEN);

	rc = adal_scom_read(adal, &old_data, scom_address, status);
	if (rc < 0) return rc;

	new_data[0] = old_data[0] & ~(usermask[0]);
	new_data[0] = new_data[0] | ((userbuffer[0]) & (usermask[0]));
	new_data[1] = old_data[1] & ~(usermask[1]);
	new_data[1] = new_data[1] | ((userbuffer[1]) & (usermask[1]));

	rc = adal_scom_write(adal, &new_data, scom_address, status);

	return rc;
}

ssize_t adal_scom_write_under_mask(adal_t * adal, void * buf, uint64_t scom_address, void * mask, unsigned long * status)
{
	adal_scom_t *scom = to_scom_adal(adal);
	struct scom_access acc = {};
	int rc;

	if (!scom->has_ioctl)
		return adal_scom_legacy_write_under_mask(adal, buf, scom_address, mask, status);

	acc.addr = scom_address;
	memcpy(&acc.data, buf, sizeof(__u64));
	memcpy(&acc.mask, mask, sizeof(__u64));
	rc = ioctl(adal->fd, FSI_SCOM_WRITE, &acc);
	if (rc)
		return rc;
	adal_scom_make_status(&acc, status);
	return 8;
}

ssize_t adal_scom_ffdc_extract(adal_t * adal, int scope, void ** buf)
{
    *buf = NULL;
    return 0;
}

int adal_scom_ffdc_unlock(adal_t * adal, int scope)
{
	return 0;
}

/* this method expects the adal to be opened against the raw fsi device for this target */
ssize_t adal_scom_get_register(adal_t *adal, int registerNo,
			       unsigned long *data)
{
	int rc;
    
	lseek(adal->fd, SCOM_ENGINE_OFFSET + ((registerNo & ~0xFFFFFF00) * 4), SEEK_SET);
	rc = read(adal->fd, data, 4);

	if (adal_is_byte_swap_needed())
	{
		// based on device and openbmc version we know the driver is not swapping endianess
		(*data) = ntohl((*data));
	}
	
	return rc;
}

/* this method expects the adal to be opened against the raw fsi device for this target */
ssize_t adal_scom_set_register(adal_t *adal, int registerNo,
			       unsigned long data)
{
	int rc;

	lseek(adal->fd, SCOM_ENGINE_OFFSET + ((registerNo & ~0xFFFFFF00) * 4), SEEK_SET);
	if (adal_is_byte_swap_needed())
	{
		// based on device and openbmc version we know the driver is not swapping endianess
		data = htonl(data);
	}
	
	rc = write(adal->fd, &data, 4);
	
	return rc;
}
