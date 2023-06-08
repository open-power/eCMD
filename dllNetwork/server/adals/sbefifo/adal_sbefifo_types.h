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

#ifndef _ADAL_SBEFIFO_TYPES_H_
#define _ADAL_SBEFIFO_TYPES_H_

#include <stddef.h>
#include <adal_base.h>

#include <linux/types.h>
#include <linux/ioctl.h>

struct adal_sbefifo_op
{
	void * data;     		/* data pointer - "in" for request , "out" for reply */
	size_t wordcount;  		/* "in" */
	unsigned long status;	/* "out" containing upstream fifo status for request, downstream for reply */
};


/*! @struct	ioctl_scan_register
 * use this struct to read or write register
 */
typedef struct ioctl_sbefifo_register
{
        unsigned long address;
        unsigned long value;
} ioctl_sbefifo_register_t;

typedef struct adal_sbefifo_op adal_sbefifo_request;
typedef struct adal_sbefifo_op adal_sbefifo_reply;

/* ioctl commands */
#define IOCTL_SUBMIT		0x1
#define IOCTL_REQUEST_RESET	0x2
#define IOCTL_WRITEREG          0x0005
#define IOCTL_READREG           0x0006
#define IOCTL_RESET             0x0007


/* the registers of the SBEFIFO Engine */
#define ADAL_SBEFIFO_UPSTREAM_STATUS  0x01
#define ADAL_SBEFIFO_DOWNSTREAM_STATUS  0x11

#define ADAL_SBEFIFO_UPSTREAM_EOT  0x02
#define ADAL_SBEFIFO_DOWNSTREAM_EOT  0x12

#define ADAL_SBEFIFO_UPSTREAM_REQUEST_RESET  0x03
#define ADAL_SBEFIFO_DOWNSTREAM_REQUEST_RESET  0x13

#define ADAL_SBEFIFO_UPSTREAM_RESET  0x04
#define ADAL_SBEFIFO_DOWNSTREAM_RESET  0x14

#define ADAL_SBEFIFO_UPSTREAM_ACK_EOT  0x05
#define ADAL_SBEFIFO_DOWNSTREAM_ACK_EOT  0x15

#define ADAL_SBEFIFO_MAXMESSAGELEN  0x16  /* not yet defined in spec */

/* Bits in status register */
#define ADAL_SBEFIFO_REQUESTRESET_BIT 0x02000000   /* Bit 6 is the request reset bit */

/* Cmd timeout in seconds ioctl specification*/
#define FSI_SBEFIFO_CMD_TIMEOUT_SECONDS  _IOW('s', 0x01, __u32)
#define FSI_SBEFIFO_READ_TIMEOUT_SECONDS _IOW('s', 0x00, __u32)

#endif
