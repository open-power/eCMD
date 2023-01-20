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
 *   Desc: Access Device Abstraction Layer (a.k.a ADAL) for the scom device.
 *
 * Author: International Business Machines, Inc. (c) Copyright IBM Corporation 2003.
 *         Shaun Wetzstein <shaun@us.ibm.com>
 *
 * Change: 10/23/03 <shaun@us.ibm.com>
 *         Initial creation.
 *         12/12/03 xxpetri@de.ibm.com
 *         adaption for scom
 */

#ifndef __ADAL_SCOM_INTERFACES_H__
#define __ADAL_SCOM_INTERFACES_H__

#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/poll.h>

#include <adal_base.h>
//#include <libaio.h>
//#include <adal_aio.h>

/*
 * Interface Overview:
 *
 * Common Type(s):
 * ---------------
 *              adal_t - Abstract Data Type for ADAL devices.
 *       adal_config_t - Get or Set configuration specifier.
 *        adal_reset_t - Reset type.
 *        adal_event_t - Wait even type.
 *
 * scom Specific Type(s):
 * ------------------------
 *  adal_scom_config_t - scom Configuration parameters.
 *
 *
 * Interface(s):
 * -------------
 *        adal_scom_open - Request access to an ADAL device.
 *       adal_scom_close - Release access of an ADAL device.
 *       adal_scom_reset - Stop activity and force device into a "known" state.
 *
 *        adal_scom_read - Read data from a device.
 *       adal_scom_write - Write data to a device.
 *      adal_scom_offset - Change the read/write position of a device.
 *       adal_scom_flush - Suspend activity on a device until all pending
 *                         or queued operations are complete.
 *
 *      adal_scom_config - Change a device's operating parameters.
 *adal_scom_ffdc_extract - Retreive any pending FFDC information a
 *                         a device may have.
 *adal_scom_ffdc_extract_identifier - Retreive any pending FFDC information a
 *                         a device may have with its location code.
 * adal_scom_ffdc_unlock - Remove from black list.
 */

/* Abstract Data Type for ADAL devices */

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief	This interface is used to open a device and associate it with an
 *		adal_t. It must be the first interface called to ensure that the
 *		device has been opened. The device is closed when the application
 *		terminates or when adal_scom_close() is called on the device.
 *
 * @post	When the device is opened it is configured with the default set
 *		of parameter values.  The adal_scom_config() can be used to
 *		modify the device's operating characteristics.
 *
 * @post	When the device is opened, sizeof(adal_t) bytes is allocated on
 *		the caller's heap.
 *
 * @param	device	NULL-terminated string of the target device
 *			(i.e. /dev/scom/<n>).
 *
 * @param	flags	Optional device specific open flags, use 0 for the
 *			device default. One of the required flags can be bitwise
 *			or'd with zero or more optional flags.
 *
 * @note	The low-level device drivers will support only the flags which
 *		are appropriate for the underlying device.
 *
 * @note	[Required]
 * @note	O_RDWR	Open the device for read and write access.
 *
 * @note	[Optional]
 * @note	O_EXCL	Request exclusive access to the device
 *
 * @return	Pointer to an adal_t on success, NULL on failure with 'ERRNO'
 *		set as follows.
 * @return	ENXIO	The device in the /dev filesystem can not be found.
 * @return	EIO	A low-level device driver or hardware I/O error occurred.
 * @return	EINVAL	At least one of the input parameters is invalid.
 * @return	EBUSY	The request would block waiting for the device,
 *			if O_NONBLOCK was specified in the 'flags' parameter.
 */
adal_t * adal_scom_open(const char * device, int flags);

/*!
 * @brief	This interface is used to dissociate an adal_t from its
 *		underlying device. If this interface is not called before the
 *		process exits, the device will be closed by the operating system.

 * @pre		The device was opened with a prior call to adal_scom_open().
 *
 * @note	Prior to closing the device, the adal_scom_close() interface
 *		will ensure any buffered data is written to the device by
 *		calling the adal_scom_flush() interface.
 *
 * @note	It's important to check the return code from the adal_scom_close()
 *		interface because device operation errors are guaranteed to be
 *		returned as the return code from this interface.  Not checking
 *		the return code can lead to silent data lossage.
 *
 * @param	adal	adal_t of the target device.
 *
 * @return	0 on success, -1 on failure with 'ERRNO' set as follows.
 * @return	EIO	A low-level device driver or hardware I/O error occurred.
 * @return	EINVAL	At least one of the input parameters is invalid.
 * @return	EBADF	The file descriptor of the underlying device is not valid.
 */
int adal_scom_close(adal_t * adal);

/*!
 * @brief	This interface is used to force the underlying device,
 *		associated with an adal_t, into a known state.
 *
 * @pre		The device was opened with a prior call to adal_scom_open().
 *
 * @note	The adal_scom_reset() interface can be used to ensure the device
 *		is in a "sane" state suitable for operation.
 *
 * @param	adal	adal_t of the target device.
 * @param	type	Reset type.
 *
 * @return	0 on success, -1 on failure with 'ERRNO' set as follows.
 * @return	EIO	A low-level device driver or hardware I/O error occurred.
 * @return	EINVAL	At least one of the input parameters is invalid.
 * @return	EBADF	The file descriptor of the underlying device is not valid.
 */
int adal_scom_reset(adal_t * adal, scom_adal_reset_t type);
int adal_scom_reset_with_fsi(adal_t * adal, adal_t * adal_fsi, scom_adal_reset_t type);

/*!
 * @brief	This interface is used to read data from the underlying device
 *		associated with an adal_t.
 *
 * @pre		The device was opened with a prior call to adal_scom_open().
 *
 * @note	In general, the adal_scom_read() interface implements a blocking
 *		"read" interface.  However, if the device is opened with the
 *		O_NONBLOCK or O_NDELAY, neither the adal_scom_open() nor any
 *		subsequent operations on the file descriptor will cause the
 *		calling process to wait.  The O_NONBLOCK or O_NDELAY options
 *		will only be implemented for devices where appropiate, all
 *		other devices will return EINVAL if O_NONBLOCK or O_NDELAY are
 *		specified.
 *
 * @note	The maximum length for a data operation is determined by the
 *		capabilities of the target device hardware. For devices which
 *		have data size limitations, EFBIG will be returned for requests
 *		which exceed the device capabilities.
 *
 * @param	adal		adal_t of the target device.
 * @param	buf		The destination buffer of the data read from
 *				the device.
 * @param	scom_address	scom address.
 * @param	status		status register returned by call.
 *
 * @return	The number of bytes copied into 'buf' on success, -1 on failure
 *		with 'ERRNO' set as follows.
 * @return	EIO	A low-level device driver or hardware I/O error occurred.
 * @return	EINVAL	At least one of the input parameters is invalid.
 * @return	EFAULT	There was an internal failure or 'buf' is invalid.
 * @return	EBADF	The file descriptor of the underlying device is not valid.
 * @return	EFBIG	If 'count' is larger then the maximum supported device
 *			operation length.
 */

ssize_t adal_scom_read(adal_t * adal, void * buf, uint64_t scom_address, unsigned long * status);

/*!
 * @brief	This interface is used to write data to the underlying device
 *		associated with an adal_t.
 *
 * @pre		The device was opened with a prior call to adal_scom_open().
 *
 * @note	If the device is opened with the O_SYNC flag, the adal_scom_write()
 *		interface will block the caller until the device has completely
 *		processed the data.  All devices should support the O_SYNC flag,
 *		unless posted writes don't make sense for the device.
 *
 * @note	If the device is opened with the O_NONBLOCK or O_NDELAY, neither
 *		the adal_scom_open() nor any subsequent operations on the file
 *		descriptor will cause the calling process to wait.  The O_NONBLOCK
 *		or O_NDELAY options will only be implemented for devices where
 *		appropriate, all other devices will return EINVAL if O_NONBLOCK or
 *		O_NDELAY are specified.
 *
 * @note	The maximum length for a data operation is determined by the
 *		capabilities of the target device hardware. For devices which
 *		have data size limitations, EFBIG will be returned for requests
 *		which exceed the device capabilities.
 *
 * @param	adal		adal_t of the target device.
 * @param	buf		The source buffer of the data to be written to
 *				the device.
 * @param	scom_address	scom address.
 * @param	status		status register returned by call.
 *
 * @return	The number of bytes copied to the device on success, -1 on
 *		failure with 'ERRNO' set as follows.
 * @return	EIO	A low-level device driver or hardware I/O error occurred.
 * @return	EINVAL	At least one of the input parameters is invalid.
 * @return	EFAULT	There was an internal failure or 'buf' is invalid.
 * @return	EBADF	The file descriptor of the underlying device is not valid.
 * @return	EFBIG	If 'count' is larger then the maximum supported device
 *			operation length.
 */

ssize_t adal_scom_write(adal_t * adal, void * buf, uint64_t scom_address,  unsigned long * status);
ssize_t adal_scom_write_under_mask(adal_t * adal, void * buf, uint64_t scom_address, void * mask, unsigned long * status);

/* get the FFDC Data */
/*!
 * @brief	This interface is used to access the device's FFDC information.
 *
 * @pre		The device was opened with a prior call to adal_scom_open().
 *
 * @param	adal	adal_t of the target device.
 * @param	scope	Specifies the FFDC search mode.
 *	@note	ADAL_FFDC_THREAD is used to extract the queued FFDC data only
 *		for the calling thread.
 *	@note	ADAL_FFDC_PROCESS is used to extract the queued FFDC data only
 *		for the calling process.
 * @param	buf	The output destination buffer of the FFDC data read from
 *			the device. This buffer is allocated on the callers heap.
 *			To avoid memory leaks, the caller should be sure to
 *			free(3) 'buf' between each calls to adal_scan_ffdc_extract.
 *
 * @return	The number of bytes copied into 'buf' on success, -1 on failure
 *		with 'ERRNO' set as follows. A value of 0 means that there is no
 *		FFDC information available.
 * @return	EIO	A low-level device driver or hardware I/O error occurred.
 * @return	EINVAL	At least one of the input parameters is invalid.
 * @return	EFAULT	There was an internal failure or 'buf' is invalid.
 * @return	EBADF	The file descriptor of the underlying device is not valid.
 */
ssize_t adal_scom_ffdc_extract(adal_t * adal, int scope, void ** buf);

/*!
 * @brief	This interface is used to access the device's FFDC information
 *		with location.
 *
 * @pre		The device was opened with a prior call to adal_scom_open().
 *
 * @param	adal	adal_t of the target device.
 * @param	scope	Specifies the FFDC search mode.
 *	@note	ADAL_FFDC_THREAD is used to extract the queued FFDC data only
 *		for the calling thread.
 *	@note	ADAL_FFDC_PROCESS is used to extract the queued FFDC data only
 *		for the calling process.
 * @param	buf	The output destination buffer of the FFDC data read from
 *			the device. This buffer is allocated on the callers heap.
 *			To avoid memory leaks, the caller should be sure to
 *			free(3) 'buf' between each calls to
 *			adal_scan_ffdc_extract.
 * @param	ident	The output value specifying the error location
			identifier code associated with the returned FFDC.
 *
 * @return	The number of bytes copied into 'buf' on success, -1 on failure
 *		with 'ERRNO' set as follows. A value of 0 means that there is no
 *		FFDC information available.
 * @return	EIO	A low-level device driver or hardware I/O error occurred.
 * @return	EINVAL	At least one of the input parameters is invalid.
 * @return	EFAULT	There was an internal failure or 'buf' is invalid.
 * @return	EBADF	The file descriptor of the underlying device is not valid.
 */
ssize_t adal_scom_ffdc_extract_identifier(adal_t * adal, int scope, void ** buf,
			       enum adal_base_err_identifier * ident);

/*!
 * @brief	This interface is used to remove a process/thread from a
 *		device's "blacklist".
 *
 * @pre		The device was opened with a prior call to adal_scom_open().
 *
 * @param	adal	adal_t of the target device.
 * @param	scope	Specifies the FFDC search mode.
 *	@note	ADAL_FFDC_THREAD is used to extract the queued FFDC data only
 *		for the calling thread.
 *	@note	ADAL_FFDC_PROCESS is used to extract the queued FFDC data only
 *		for the calling process.
 *
 * @return	0 on success, -1 on failure with errno set as follows.
 * @return	EIO	A low-level device driver or hardware I/O error occurred.
 * @return	EAGAIN	There is more FFDC data pending.
 * @return	EINVAL	At least one of the input parameters is invalid.
 * @return	EBADF	The file descriptor of the underlying device is not valid.
 * @return	ENODEV	The device is currently unreachable and/or unavailable.
 */
int adal_scom_ffdc_unlock(adal_t * adal, int scope);

/*!
 * @brief	Read a single register in SCAN Engine.
 *		This is for debug only.
 *
 * @pre		The device was opened with a prior call to adal_scom_open().
 *
 *
 * @param	adal		adal_t of the target device.
 * @param	registerNo	Address of the register.
 * @param	data		Data read out of the register.
 *
 * @return	'0' on success, negative on error.
 */
ssize_t adal_scom_get_register(adal_t * adal, int registerNo, unsigned long * data);

/*!
 * @brief	Write a single register in SCAN Engine.
 *		This is for debug only.
 *
 * @pre		The device was opened with a prior call to adal_scom_open().
 *
 *
 * @param	adal		adal_t of the target device.
 * @param	registerNo	Address of the register.
 * @param	data		Data written to the register.
 *
 * @return	'0' on success, negative on error..
 */
ssize_t adal_scom_set_register(adal_t * adal, int registerNo, unsigned long data);

/*
 * The following get and set functions will read and write the according register
 * in the CFAM
 *
 * Pre: The device was opened with a prior call to adal_scom_open().
 *
 *
 * Param: 'adal' adal_t of the target device.
 * for read
 * Param: 'data' - data read out of the register
 * for write
 * Param: 'data' - data written into the register
 *
 * Return: number of Bytes read (4), negativ on error
 */


ssize_t adal_scom_get_truemask(adal_t * adal, unsigned long * data);
ssize_t adal_scom_set_truemask(adal_t * adal, unsigned long data);

ssize_t adal_scom_get_compmask(adal_t * adal, unsigned long * data);
ssize_t adal_scom_set_compmask(adal_t * adal, unsigned long data);

ssize_t adal_scom_get_gp1(adal_t * adal, unsigned long * data);
ssize_t adal_scom_set_gp1(adal_t * adal, unsigned long data);
ssize_t adal_scom_get_gp2(adal_t * adal, unsigned long * data);
ssize_t adal_scom_set_gp2(adal_t * adal, unsigned long data);
ssize_t adal_scom_get_gp3(adal_t * adal, unsigned long * data);    //p7/z7 only
ssize_t adal_scom_set_gp3(adal_t * adal, unsigned long data);      //p7/z7 only

ssize_t adal_scom_set_gp(adal_t * adal, unsigned long gp_no, unsigned long data);
ssize_t adal_scom_get_gp(adal_t * adal, unsigned long gp_no, unsigned long * data);

ssize_t adal_scom_get_sns(adal_t * adal, unsigned long sns_no, unsigned long  * data);

/*!
 * @brief	Gets register 12 or 18.
 *		zioc + sn only + p7
 */
ssize_t adal_scom_get_writeprotect(adal_t * adal, unsigned long * data);

/*!
 * @brief	sets Register 12 or 18 with given value.
 *		zioc + sn only + p7
 */
ssize_t adal_scom_set_writeprotect(adal_t * adal, unsigned long data);
/*!
 * @brief	write 0x000000 to Register 12 or 18 to enable write protect
 *		for GP1/2. zioc + sn only + p7 */
ssize_t adal_scom_enable_writeprotect(adal_t * adal);

/*!
 * @brief	Write 0x4453FFFF to register 12 or 18 to disable write protect
 *		for GP1/2 permanently. zioc + sn only + p7 */
ssize_t adal_scom_disable_writeprotect(adal_t * adal);

ssize_t adal_scom_get_toadmode(adal_t * adal, unsigned long * data);
ssize_t adal_scom_set_toadmode(adal_t * adal, unsigned long data);
ssize_t adal_scom_get_arbiter(adal_t * adal, unsigned long * data);
ssize_t adal_scom_set_arbiter(adal_t * adal, unsigned long data);

ssize_t adal_scom_get_status(adal_t * adal, unsigned long * data);
ssize_t adal_scom_get_chipid(adal_t * adal, unsigned long * data);


/*!
 * @brief	This interface will call a flushstop issued with address
 *		parameter 0x000000.
 *
 * @pre		The device was opened with a prior call to adal_scom_open().
 *
 * @param	adal	adal_t of the target device.
 * @param	status	status register returned by call
 *
 * @return	0 on success, negative on error.
 */

ssize_t adal_scom_flushstop(adal_t * adal, unsigned long * status);

/*!
 * @brief	This interface will call a flushstart issued with given address.
 *
 * @note	A flushstart has to be stopped using the adal_scom_flushstop.
 *
 * @pre		The device was opened with a prior call to adal_scom_open().
 *
 * @param	adal		adal_t of the target device.
 * @param	scom_address	address to do the flush on
 * @param	status		status register returned by call.
 *
 * @return	0 on success, negative on error.
 */
ssize_t adal_scom_flushstart(adal_t * adal, uint64_t scom_address, unsigned long * status);

/* following functions are based on AIO */

/*!
 * @brief	This interface will initialise the given adal_scom_event_t with
 *		the given true and comp mask.
 *
 * @pre		adal_scom_evnet_t event is already allocated.
 *
 * @param	event		pointer to allready allocated event
 * @param	true_mask	true mask used for initialisation
 * @param	comp_mask	comp mask used for initialisation
 *
 * @return	O on success, -1 on error with 'ERRNO' set.
 * @return	EINVAL	invalid evnet pointer
 */

int adal_scom_event_init(adal_scom_event_t * event, unsigned long true_mask, unsigned long comp_mask);

/*!
 * @brief	The interface extract status from given event pointer.
 *
 * @param	event		pointer to a event
 * @param	status		status in the event returned
 *
 * @return	O on success, -1 on error with 'ERRNO' set.
 * @return	EINVAL	invalid pointer as input
 */
int adal_scom_event_extract(adal_scom_event_t * event, unsigned long * status);

/*!
 * @brief	The interface extract status and for all also the true and
 *		complement mask from given event pointer. Functions are used
 *		after irq happened to get the status and masks from the returned data.
 *
 * @param	event		pointer to a event
 * @param	status		status in the event returned
 * @param	trueMask	true Mask in the event returned
 * @param	compMask	complement Mask in the event returned
 *
 * @return	O on success, -1 on error with 'ERRNO' set.
 * @return	EINVAL	invalid pointer as input
 */
int adal_scom_event_extract_all(adal_scom_event_t * event,
                                unsigned long * status,
                                unsigned long * trueMask,
                                unsigned long * compMask);
int adal_scom_event_extract_all_pib(adal_scom_event_t * event,
                                unsigned long * status,
                                unsigned long * irq,
                                unsigned long * trueMask,
                                unsigned long * compMask);

/*!
 * @brief	This call we get mapped to adal_base_config only ...
 *		see adal_base_config to details.
 */
int adal_scom_config(adal_t * adal, int type, int config, void * value, size_t sz);


/*!
 * @brief	The interface adal_scom_setup_vop will initialise the gives
 *		'struct ioctl_scom_readwrite_v' and allocate needed memory for
 *		the operations.
 *
 * @param	vop	pointer to a struct scom_v_op
 * @param	nr_ops	specifies how many operation should be inserted later-in
 * @param	flags	currently unused
 *
 * @return	O on success, -1 on error with 'ERRNO' set.
 * @return	EINVAL	invalid evnet pointer
 */
int adal_scom_setup_vop(scom_v_op * vop, size_t nr_ops, unsigned long flags);


/*!
 * @brief	This interface will initialise the n-th element in the vop data
 *
 * @note	This call is usually done in a loop from 0 to < numberOfElements.
 *
 * @pre		vop was initialised with adal_scom_setup_vop.
 *
 * @param	op		pointer to a struct scom_v_op
 * @param	opNo		which op number should be set up
 * @param	mode		SCOMREAD or SCOMWRITE
 * @param	scom_address	the scom address
 * @param	data		pointer to 8Bytes data field
 *
 * @return	O on success, -1 on error with 'ERRNO' set.
 * @return	EINVAL, EIO
 */
int adal_scom_setup_op(scom_v_op * op, int opNo, int mode, uint64_t scom_address,
		unsigned long * data);
int adal_scom_setup_op_mask(scom_v_op * op, int opNo, int mode,
		uint64_t scom_address, unsigned long * data, unsigned long * mask);


/*!
 * @brief	This interface will execute all operations with the
 *		'struct ioctl_scom_readwrite_v'.
 *
 * @pre		vop was initialised with adal_scom_setup_vop
 * @pre		adal_scom_setup_op was called for every element
 * @pre		The device was opened with a prior call to adal_scom_open().
 *
 * @param	adal	adal_t of the target device.
 * @param	data	pointer to a struct ioctl_scom_readwrite_v
 *
 * @return	O on success, -1 on error with 'ERRNO' set.
 * @return	EINVAL, EIO, ENOMEN
 */
ssize_t adal_scom_vop(adal_t * adal, scom_v_op * data);


/*!
 * @brief	The interface will extract all return values from structure.
 *
 * @pre		adal_scom_vop was called.
 *
 * @param	vop	pointer to a struct scom_v_op
 * @param	nr_ops	number of Operations executed
 * @param	flags	O for sccess of all Operations
 *
 * @return	O on success, -1 on error with 'ERRNO' set.
 * @return	EINVAL
 */
int adal_scom_get_vop(scom_v_op * vop, size_t * nr_ops, unsigned long * flags);


/*!
 * @brief	The interface will extract all return values from one operation.
 *
 * @pre		adal_scom_vop was called.
 *
 * @param[in]	vop		pointer to a struct scom_v_op
 * @param[in]	opNo		number of the operation to get data from
 * @param[out]	mode		read or write
 * @param[out]	scom_address	scom address
 * @param[out]	data		contain data on read
 * @param[out]	returnCode	of this op - 0 on success
 * @param[out]	status		scom status register from this op
 *
 * @return	O on success, -1 on error with 'ERRNO' set.
 * @return	EINVAL
 */
int adal_scom_get_op(scom_v_op * vop, int opNo, int * mode, uint64_t *scom_address,
		unsigned long * data, int * returnCode, unsigned long * status);


/*!
 * @brief	The interface will clean up the data.
 *
 * @pre		vop was initialised with adal_scom_setup_vop
 *
 * @param[in]	vop	pointer to a struct scom_v_op
 *
 * @return	O on success, -1 on error with 'ERRNO' set.
 * @return	EINVAL
 */
int adal_scom_free_vop(scom_v_op * vop);

/*!
 * @brief	The interface will set the errormask used by the device driver
 *		to check for errors.
 *
 * @pre		The device was opened with a prior call to adal_scom_open().
 *
 * @param	adal		adal_t of the target device.
 * @param	errormask	new errormask to be used
 *
 * @return length of mask (4) on success, -1 on error with 'ERRNO' set.
 * @return	EINVAL, EIO
 */
ssize_t adal_scom_set_errormask(adal_t * adal, unsigned long errormask);

#ifdef __cplusplus
}
#endif

#endif /* __ADAL_SCOM_H__ */
