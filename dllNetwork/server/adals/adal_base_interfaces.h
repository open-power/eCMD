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

//#include <ffdc_chain.h>

#ifndef __ADAL_BASE_INTERFACES_H__
#define __ADAL_BASE_INTERFACES_H__

/*
 * Interface Overview:
 *
 * Interface(s):
 * -------------
 *        adal_base_open - Request access to an ADAL device.
 *       adal_base_close - Release access of an ADAL device.
 *       adal_base_get_name - Returns the device name string of an ADAL device.
 *       adal_base_reset - Stop activity and force a device into a "known"
 *                         state.
 *
 *        adal_base_read - Read data from a device.
 *       adal_base_readv - Read an array of buffers from a device.
 *       adal_base_write - Write data to a device.
 *      adal_base_writev - Write an array of buffers to a device.
 *        adal_base_seek - Change the read/write position of a device.
 *       adal_base_flush - Suspend activity on a device until all pending
 *                         or queued operations are complete.
 *
 *        adal_base_lctl - Manage the exclusive access lock of a device.
 *        adal_base_wait - Wait and/or check for events to occur on a device.
 *      adal_base_config - Change a device's operating parameters.
 *adal_base_ffdc_extract - Retreive any pending FFDC information a
 *                         a device may have.
 *adal_base_ffdc_extract_identifier - Retreive any pending FFDC information a
 *                         a device may have with its location code
 * adal_base_ffdc_unlock - Remove from black list
 *  adal_base_ffdc_clear - Clear FFDC data
 *
 */


#ifdef __cplusplus
extern "C" {
#endif

#define adal_base_ffdc_get_type dd_ffdc_get_type

/*!@brief
 * The adal_base_open() interface is used to open a device and associate it
 * with an adal_t.  It must be the first interface called to ensure that the
 * device has been opened. The device is closed when the application
 * terminates or when adal_base_close() is called on the device.
 *
 * @post: When the device is opened it is configured with the default set of
 *       parameter values.  The adal_base_config() can be used to modify the
 *       device's operating characteristics.
 *
 * @post: When the device is opened, sizeof(adal_t) bytes is
 *       allocated on the caller's heap.
 *
 * @param: 'device' NULL-terminated string of the target device
 *        (i.e. /dev/<xxxx>/<n>).
 *
 * @param: 'flags' Optional device specific open flags, use 0 for the
 *        device default.  One of the required flags can be bitwise or'd
 *        with zero or more optional flags.
 *
 * @note: The low-level device drivers will support only the flags
 *        which are appropiate for the underlying device.
 *
 * @note   [Required]
 * @note    O_RDONLY    Open the device for read-only access.
 * @note    O_WRONLY    Open the device for write-only access.
 * @note    O_RDWR      Open the device for read and write access.
 *
 * @note   [Optional]
 * @note    O_EXCL      Request exclusive access to the device
 *                      (i.e. blocking single-threaded access).
 * @note    O_SYNC      Any write()'s on the device will block the caller until
 *                      the data has been physically written to the underlying
 *                      device.
 * @note    O_ASYNC     Generate a signal (SIGIO by default) when input or
 *                      output becomes possible on this device.
 * @note    O_NONBLOCK,
 * @note    O_NDELAY    The device is opened in non-blocking mode.  Neither
 *                      the open nor any subsequent operations on the device
 *                      will cause the caller to wait.
 *
 * @note    See 'man 2 open' for a details about all open flags.
 *
 * @return: Pointer to an adal_t on success, NULL on failure with errno set as
 *         follows.
 * @return: ENXIO The device in the /dev filesystem can not be found.
 * @return: EIO A low-level device driver or hardware I/O error occurred.
 * @return: EINVAL At least one of the input parameters is invalid.
 * @return: EBUSY The request would block waiting for the device,
 *         if O_NONBLOCK was specified in the 'flags' parameter.
 */
adal_t * adal_base_open(const char * device, int flags);

/*!@brief
 * The adal_base_close() interface is used to disassociate an adal_t from
 * its underlying device.  If this interface is not called before the process
 * exits, the device will be closed by the operating system.

 * @pre: The device was opened with a prior call to adal_base_open().
 *
 * @note: Prior to closing the device, the adal_base_close() interface will
 *       ensure any buffered data is written to the device by calling the
 *       adal_base_flush() interface.
 *
 * @note: It's important to check the return code from the adal_base_close()
 *       interface because device operation errors are guaranteed to be
 *       returned as the return code from this interface.  Not checking
 *       the return code can lead to silent data lossage.
 *
 * @param: 'adal' adal_t of the target device.
 *
 * @return: 0 on success, -1 on failure with errno set as follows.
 * @return: EIO A low-level device driver or hardware I/O error occurred.
 * @return: EINVAL At least one of the input parameters is invalid.
 * @return: EBADF The file descriptor of the underlying device is not valid.
 * @return: ENODEV The operation requires the FSI local bus to complete.
 */
int adal_base_close(adal_t * adal);

/*!@brief
 * The adal_base_get_name() interface is used to retrieve the device name given
 * the adal handle adal_t. 
 * @pre: The device was opened with a prior call to adal_base_open().
 *
 * @note: It's important to check the return code from the adal_base_get_name()
 *       interface because device operation errors are guaranteed to be
 *       returned as the return code from this interface.  Not checking
 *       the return code can lead to silent data lossage.
 *
 * @param: 'adal' adal_t of the target device.
 *
 * @return: 0 on success, -1 on failure with errno set as follows.
 * @return: EINVAL At least one of the input parameters is invalid.
 * @return: EBADF The file descriptor of the underlying device is not valid.
 */
int adal_base_get_name(adal_t *adal, char *buf, size_t len);

/*!@brief
 * The adal_base_reset() interface is used to force the underdying device,
 * associated with an adal_t, into a known state.
 *
 * @pre: The device was opened with a prior call to adal_base_open().
 *
 * @note: The adal_base_reset() interface can be used to ensure the device is
 *       in a "sane" state suitable for operation.
 *
 * @param: 'adal' adal_t of the target device.
 * @param: 'type' Reset type.
 *
 * @return: 0 on success, -1 on failure with errno set as follows.
 * @return: EIO A low-level device driver or hardware I/O error occurred.
 * @return: EINVAL At least one of the input parameters is invalid.
 * @return: EBADF The file descriptor of the underlying device is not valid.
 * @return: ENODEV The operation requires the FSI local bus to complete.
 * @return: EDEADLK Device is locked due to unextracted DD_FFDC.
 * @return: ENOSYS Function not implemented
 */
int adal_base_reset(adal_t * adal, int type);

/*!@brief
 * The adal_base_config() interface is used to access the current configuration
 * parameters of the underlying device associated with an adal_t.
 *
 * @pre: The device was opened with a prior call to adal_base_open().
 *
 * @param: 'adal' adal_t of the target device.
 * @param: 'type' The access type (i.e. read or write the config value).
 * @param: 'cfg' The target config parameter.
 * @param: 'value' A multipurpose input/output variable defined by each
 *         specific config operation.
 * @param: 'sz' Size of the variable pointed to by value
 *
 * @return: 0 on success, -1 on failure with errno set as follows.
 * @return: EIO A low-level device driver or hardware I/O error occurred.
 * @return: EINVAL At least one of the input parameters is invalid.
 * @return: EFAULT There was an internal failure or 'data' is invalid
 * @return: EBADF The file descriptor of the underlying device is not valid.
 * @return: ENODEV The operation requires the FSI local bus to complete.
 * @return: EDEADLK Device is locked due to unextracted DD_FFDC.
 * @return: ENOSYS Function not implemented
 */
int adal_base_config(adal_t * adal, int type, int cfg, void * value, size_t sz);

/*!@brief
 * The adal_base_read() interface is used to "read" data from the underlying
 * device associated with an adal_t.
 *
 * @pre: The device was opened with a prior call to adal_base_open().
 *
 * @note: In general, the adal_base_read() interface implements a blocking
 *       "read" interface.  However, if the device is opened with the
 *       O_NONBLOCK or O_NDELAY, neither the adal_base_open() nor any
 *       subsequent operations on the file descriptor will cause the
 *       calling process to wait.  The O_NONBLOCK or O_NDELAY options
 *       will only be implemented for devices where appropiate, all
 *       other devices will return EINVAL if O_NONBLOCK or O_NDELAY are
 *       specified.
 *
 * @note: The maximun length for a data operation is determined by the
 *       capabilities of the target device hardware. For devices which
 *       have data size limitations, EFBIG will be returned for requests
 *       which exceed the device capabilities.
 *
 * @param: 'adal' adal_t of the target device.
 * @param: 'buf' The destination buffer of the data read from the device.
 * @param: 'count' The maximum size of the buffer.
 *
 *         @note: The meaning of 'count' is defined by the underlying device.
 *               For most devices, 'count' will represent bytes but it could
 *               represent bits or 32-bit words or whatever is appropiate.
 *
 * @return: The number of bytes copied into 'buf' on success, -1 on failure
 *         with errno set as follows.
 * @return: EIO A low-level device driver or hardware I/O error occurred.
 * @return: EINVAL At least one of the input parameters is invalid.
 * @return: EFAULT There was an internal failure or 'buf' is invalid.
 * @return: EBADF The file descriptor of the underlying device is not valid.
 * @return: EFBIG If 'count' is larger then the maximum supported device
 *         operation length.
 * @return: ENODEV The operation requires the FSI local bus to complete.
 * @return: EDEADLK Device is locked due to unextracted DD_FFDC.
 */
ssize_t adal_base_read(adal_t * adal, void * buf, size_t count);

/*!@brief
 * The adal_base_readv() interface is used to "read" an array of
 * buffers from the underlying device associated with an adal_t.
 *
 * @pre: The device was opened with a prior call to adal_base_open().
 *
 * @note: In general, the adal_base_readv() interface implements a blocking
 *       "read" interface.  However, if the device is opened with the
 *       O_NONBLOCK or O_NDELAY, neither the adal_base_open() nor any
 *       subsequent operations on the file descriptor will cause the
 *       calling process to wait.  The O_NONBLOCK or O_NDELAY options
 *       will only be implemented for devices where appropiate, all
 *       other devices will return EINVAL if O_NONBLOCK or O_NDELAY are
 *       specified.
 *
 * @note: The maximun length for a data operation is determined by the
 *       capabilities of the target device hardware. For devices which
 *       have data size limitations, EFBIG will be returned for requests
 *       which exceed the device capabilities.
 *
 * @param: 'adal' adal_t of the target device.
 * @param: 'vector' points to an array of struct iovec elements/.
 *           struct iovec {
 *               void *iov_base;   -- buffer starting address
 *               size_t iov_len;   -- number of bytes
 *           };
 * @param: 'count' The number of struct iovec array entries.
 *
 *
 * @return: The total number of bytes read from the device on success,
 *        -1 on failure with errno set as follows.
 * @return: EIO A low-level device driver or hardware I/O error occurred.
 * @return: EINVAL The sum of '.iov_len' values overflows a ssize_t or
 *         'count' is 0 or greater than MAX_IOVEC.
 * @return: EFAULT There was an internal failure or 'vector' is invalid.
 * @return: EBADF The file descriptor of the underlying device is not valid.
 * @return: EFBIG If any '.iov_len' values are larger then the maximum
 *         supported device operation length.
 * @return: ENODEV The operation requires the FSI local bus to complete.
 * @return: EDEADLK Device is locked due to unextracted DD_FFDC.
 */
ssize_t adal_base_readv(adal_t * adal, const struct iovec * vector, int count);

/*!@brief
 * The adal_base_write() interface is used to write data to the underlying
 * device associated with an adal_t.
 *
 * @pre: The device was opened with a prior call to adal_base_open().
 *
 * @note: If the device is opened with the O_SYNC flag, the adal_base_write()
 *       interface will block the caller until the device has completly
 *       processed the data.  All devices should support the O_SYNC flag,
 *       unless posted writes don't make sense for the device.
 *
 * @note: If the device is opened with the O_NONBLOCK or O_NDELAY, neither
 *       the adal_base_open() nor any subsequent operations on the file
 *       descriptor will cause the calling process to wait.  The O_NONBLOCK
 *       or O_NDELAY options will only be implemented for devices where
 *       appropiate, all other devices will return EINVAL if O_NONBLOCK or
 *       O_NDELAY are specified.
 *
 * @note: The maximum length for a data operation is determined by the
 *       capabilities of the target device hardware. For devices which
 *       have data size limitations, EFBIG will be returned for requests
 *       which exceed the device capabilities.
 *
 * @param: 'adal' adal_t of the target device.
 * @param: 'buf' The source buffer of the data to be written to the device.
 * @param: 'count' The actual size of the write data.
 *
 * @note: The meaning of 'count' is defined by the underlying device.
 *        For most devices, 'count' will represent bytes but it could
 *        represent bits or 32-bit words or whatever is appropiate.
 *
 * @return: The number of bytes copied to the device on success, -1 on failure
 *         with errno set as follows.
 * @return: EIO A low-level device driver or hardware I/O error occurred.
 * @return: EINVAL At least one of the input parameters is invalid.
 * @return: EFAULT There was an internal failure or 'buf' is invalid.
 * @return: EBADF The file descriptor of the underlying device is not valid.
 * @return: EFBIG If 'count' is larger then the maximum supported device
 *         operation length.
 * @return: ENODEV The operation requires the FSI local bus to complete.
 * @return: EDEADLK Device is locked due to unextracted DD_FFDC.
 */
ssize_t adal_base_write(adal_t * adal, const void * buf, size_t count);

/*!@brief
 * The adal_base_writev() interface is used to write an array of buffers
 * to the underlying device associated with an adal_t.
 *
 * @pre: The device was opened with a prior call to adal_base_open().
 *
 * @note: If the device is opened with the O_SYNC flag, the adal_base_write()
 *       interface will block the caller until the device has completly
 *       processed the data.  All devices should support the O_SYNC flag,
 *       unless posted writes don't make sense for the device.
 *
 * @note: If the device is opened with the O_NONBLOCK or O_NDELAY, neither
 *       the adal_base_open() nor any subsequent operations on the file
 *       descriptor will cause the calling process to wait.  The O_NONBLOCK
 *       or O_NDELAY options will only be implemented for devices where
 *       appropiate, all other devices will return EINVAL if O_NONBLOCK or
 *       O_NDELAY are specified.
 *
 * @note: The maximun length for a data operation is determined by the
 *       capabilities of the target device hardware. For devices which
 *       have data size limitations, EFBIG will be returned for requests
 *       which exceed the device capabilities.
 *
 * @param: 'adal' adal_t of the target device.
 * @param: 'vector' points to an array of struct iovec elements.
 *           struct iovec {
 *               void *iov_base;   -- buffer starting address
 *               size_t iov_len;   -- number of bytes
 *           };
 * @param: 'count' The number of struct iovec array entries.
 *
 *
 * @return: The total number of bytes written to the device on success,
 *        -1 on failure with errno set as follows.
 * @return: EIO A low-level device driver or hardware I/O error occurred.
 * @return: EINVAL The sum of '.iov_len' values overflows a ssize_t or
 *         'count' is 0 or greater than MAX_IOVEC.
 * @return: EFAULT There was an internal failure or 'vector' is invalid.
 * @return: EBADF The file descriptor of the underlying device is not valid.
 * @return: EFBIG If any '.iov_len' values are larger then the maximum
 *         supported device operation length.
 * @return: ENODEV The operation requires the FSI local bus to complete.
 * @return: EDEADLK Device is locked due to unextracted DD_FFDC.
 */
ssize_t adal_base_writev(adal_t * adal, const struct iovec * vector, int count);

/*!@brief
 * The adal_base_seek() interface is used to reposition the read/write
 * offset of the underlying device associated with an adal_t.
 *
 * @note: Typically, only devices that support random access implement the
 *       lseek() system call which can be used to reposition the file
 *       position.  Some devices may require one of more ioctl() calls
 *       to change the read/write position.
 *
 * @param: 'adal' adal_t of the target device.
 * @param: 'offset' New read/write file offset.
 * @param: 'whence' Offset reference point:
 * @note   [Required]
 * @note    SEEK_SET	The read/write offset is set relative to the
 *                      beginning of the device.
 * @note    SEEK_CUR	The read/write offset is set relative to the
 *                      current offset.
 * @note    SEEK_END	The read/write offset is set to relative to the
 *                      end of the device.
 *
 * @return: The new read/write offset relative to the beginning of the device,
 *         -1 on failure with errno set as follows.
 * @return: EIO A low-level device driver or hardware I/O error occurred.
 * @return: EINVAL At least one of the input parameters is invalid.
 * @return: EBADF The file descriptor of the underlying device is not valid.
 * @return: ENODEV The operation requires the FSI local bus to complete.
 * @return: EDEADLK Device is locked due to unextracted DD_FFDC.
 */
long long adal_base_seek(adal_t * adal, long long offset, int whence);

/*!@brief
 * The adal_base_flush() interface is used to block the caller until all
 * pending operations are processed by the underlying device.  For some
 * devices, that means waiting until posted/queued writes are written to
 * the device medium.
 *
 * @pre: The device was opened with a prior call to adal_base_open().
 *
 * @param: 'adal' adal_t of the target device.
 *
 * @return: 0 on success, -1 on failure with errno set as follows.
 * @return: EIO A low-level device driver or hardware I/O error occurred.
 * @return: EINVAL At least one of the input parameters is invalid.
 * @return: EBADF The file descriptor of the underlying device is not valid.
 * @return: ENODEV The operation requires the FSI local bus to complete.
 * @return: EDEADLK Device is locked due to unextracted DD_FFDC.
 */
int adal_base_flush(adal_t * adal);

/*!@brief
 * The adal_base_ffdc_extract() interface is used to access the device's FFDC
 * information.
 *
 * @param: 'adal' adal_t of the target device.
 * @param: 'scope' Specifies the FFDC search mode.
 *         ADAL_FFDC_THREAD is used to extract the queued FFDC
 *         data only for the calling thread.
 *         ADAL_FFDC_PROCESS is used to extract the queued FFDC
 *         data only for the calling process.
 * @param: 'buf' The output destination buffer of the FFDC data read from
 *         the device.  This buffer is allocated on the callers heap.  To
 *         avoid memory leaks, the caller should be sure to free(3) 'buf'
 *         between each calls to adal_base_ffdc_extract.
 *
 * @return: The number of bytes copied into 'buf' on success, -1 on failure
 *         with errno set as follows. A value of 0 means that there is no
 *         FFDC information available.
 * @return: EIO A low-level device driver or hardware I/O error occurred.
 * @return: EINVAL At least one of the input parameters is invalid.
 * @return: EFAULT There was an internal failure or 'buf' is invalid.
 * @return: EBADF The file descriptor of the underlying device is not valid.
 */
ssize_t adal_base_ffdc_extract(adal_t *, int, void **);

/*!@brief
 * The adal_base_ffdc_extract_identifier() interface is used to access the device's FFDC
 * information with its location code.
 *
 * @param: 'adal' adal_t of the target device.
 * @param: 'scope' Specifies the FFDC search mode.
 *         ADAL_FFDC_THREAD is used to extract the queued FFDC
 *         data only for the calling thread.
 *         ADAL_FFDC_PROCESS is used to extract the queued FFDC
 *         data only for the calling process.
 * @param: 'buf' The output destination buffer of the FFDC data read from
 *         the device.  This buffer is allocated on the callers heap.  To
 *         avoid memory leaks, the caller should be sure to free(3) 'buf'
 *         between each calls to adal_base_ffdc_extract.
 * @param: 'ident' The output value indicating where in hardware the failure
 *	   occurred. 'ident' is set to one of adal_base_err_identifier.
 *
 * @return: The number of bytes copied into 'buf' on success, -1 on failure
 *         with errno set as follows. A value of 0 means that there is no
 *         FFDC information available.
 * @return: EIO A low-level device driver or hardware I/O error occurred.
 * @return: EINVAL At least one of the input parameters is invalid.
 * @return: EFAULT There was an internal failure or 'buf' is invalid.
 * @return: EBADF The file descriptor of the underlying device is not valid.
 */
ssize_t adal_base_ffdc_extract_identifier(adal_t *, int, void **,
					enum adal_base_err_identifier *);

/*!@brief
 * The adal_base_ffdc_wait() interface is used to access the device's FFDC
 * information.  It will wait until FFDC is available and extract it, before
 * returning.
 *
 * @param: 'adal' adal_t of the target device.
 * @param: 'scope' Specifies the FFDC search mode.
 *         ADAL_FFDC_THREAD is used to extract the queued FFDC
 *         data only for the calling thread.
 *         ADAL_FFDC_PROCESS is used to extract the queued FFDC
 *         data only for the calling process.
 * @param: 'buf' The output destination buffer of the FFDC data read from
 *         the device.  This buffer is allocated on the callers heap.  To
 *         avoid memory leaks, the caller should be sure to free(3) 'buf'
 *         between each calls to adal_base_ffdc_extract.
 *
 * @return: The number of bytes copied into 'buf' on success, -1 on failure
 *         with errno set as follows. A value of 0 means that there is no
 *         FFDC information available.
 * @return: EIO A low-level device driver or hardware I/O error occurred.
 * @return: EINVAL At least one of the input parameters is invalid.
 * @return: EFAULT There was an internal failure or 'buf' is invalid.
 * @return: EBADF The file descriptor of the underlying device is not valid.
 */
ssize_t adal_base_ffdc_wait(adal_t *, int, void **);

/*!@brief
 * The adal_base_ffdc_dev_extract() interface is used to access the FFDC of the
 * device pointed to by 'char * dev'.
 *
 * @param: 'dev' Null terminated character array, points to target device.
 * @param: 'scope' Specifies the FFDC search mode.
 *         ADAL_FFDC_THREAD is used to extract the queued FFDC
 *         data only for the calling thread.
 *         ADAL_FFDC_PROCESS is used to extract the queued FFDC
 *         data only for the calling process.
 * @param: 'buf' The output destination buffer of the FFDC data read from
 *         the device.  This buffer is allocated on the callers heap.  To
 *         avoid memory leaks, the caller should be sure to free(3) 'buf'
 *         between each calls to adal_base_ffdc_extract.
 *
 * @return: The number of bytes copied into 'buf' on success, -1 on failure
 *         with errno set as follows. A value of 0 means that there is no
 *         FFDC information available.
 * @return: EIO A low-level device driver or hardware I/O error occurred.
 * @return: EINVAL At least one of the input parameters is invalid.
 * @return: EFAULT There was an internal failure or 'buf' is invalid.
 * @return: EBADF The file descriptor of the underlying device is not valid.
 */
ssize_t adal_base_ffdc_dev_extract(char *, int, void **);

/*!@brief
 * The adal_base_ffdc_dev_wait() interface is used to access the FFDC of the
 * device pointed to by 'char * dev'.  It will wait until FFDC is available
 * and extract it, before returning.
 *
 * @param: 'dev' Null terminated character array, points to target device.
 * @param: 'scope' Specifies the FFDC search mode.
 *         ADAL_FFDC_THREAD is used to extract the queued FFDC
 *         data only for the calling thread.
 *         ADAL_FFDC_PROCESS is used to extract the queued FFDC
 *         data only for the calling process.
 * @param: 'buf' The output destination buffer of the FFDC data read from
 *         the device.  This buffer is allocated on the callers heap.  To
 *         avoid memory leaks, the caller should be sure to free(3) 'buf'
 *         between each calls to adal_base_ffdc_extract.
 *
 * @return: The number of bytes copied into 'buf' on success, -1 on failure
 *         with errno set as follows. A value of 0 means that there is no
 *         FFDC information available.
 * @return: EIO A low-level device driver or hardware I/O error occurred.
 * @return: EINVAL At least one of the input parameters is invalid.
 * @return: EFAULT There was an internal failure or 'buf' is invalid.
 * @return: EBADF The file descriptor of the underlying device is not valid.
 */
ssize_t adal_base_ffdc_dev_wait(char *, int, void **);

/*!@brief
 * The adal_base_ffdc_unlock() interface is used to remove a process/thread
 * from a device's "blacklist".
 *
 * @pre: The device was opened with a prior call to adal_base_open().
 *
 * @param: 'adal' adal_t of the target device.
 * @param: 'scope' Specifies the FFDC search mode.
 *         ADAL_FFDC_THREAD is used to extract the queued FFDC
 *         data only for the calling thread.
 *         ADAL_FFDC_PROCESS is used to extract the queued FFDC
 *         data only for the calling process.
 *
 * @return: 0 on success, -1 on failure with errno set as follows.
 * @return: EIO A low-level device driver or hardware I/O error occurred.
 * @return: EAGAIN There is more FFDC data pending.
 * @return: EINVAL At least one of the input parameters is invalid.
 * @return: EBADF The file descriptor of the underlying device is not valid.
 * @return: ENODEV The device is currently unreachable and/or unavailable.
 */
int adal_base_ffdc_unlock(adal_t * adal, int scope);

/*!@brief
 * The adal_base_ffdc_dev_unlock() interface is used to remove a process/thread
 * from a device's "blacklist".
 *
 * @pre: The device was opened with a prior call to adal_base_open().
 *
 * @param: 'dev' string of the target device.
 * @param: 'scope' Specifies the FFDC search mode.
 *         ADAL_FFDC_THREAD is used to extract the queued FFDC
 *         data only for the calling thread.
 *         ADAL_FFDC_PROCESS is used to extract the queued FFDC
 *         data only for the calling process.
 *
 * @return: 0 on success, -1 on failure with errno set as follows.
 * @return: EIO A low-level device driver or hardware I/O error occurred.
 * @return: EAGAIN There is more FFDC data pending.
 * @return: EINVAL At least one of the input parameters is invalid.
 * @return: EBADF The file descriptor of the underlying device is not valid.
 * @return: ENODEV The device is currently unreachable and/or unavailable.
 */
int adal_base_ffdc_dev_unlock(char * dev, int scope);

/*!@brief
 * The adal_base_ffdc_clear() interface is used to clear FFDC
 *
 * @pre: The device was opened with a prior call to adal_base_open().
 *
 * @param: 'adal' adal_t of the target device.
 * @param: 'scope' Specifies the FFDC search mode.
 *         ADAL_FFDC_THREAD is used to clear the queued FFDC
 *         data only for the calling thread.
 *         ADAL_FFDC_PROCESS is used to clear the queued FFDC
 *         data only for the calling process.
 *
 * @return: 0 on success, -1 on failure with errno set as follows.
 * @return: EIO A low-level device driver or hardware I/O error occurred.
 * @return: EINVAL At least one of the input parameters is invalid.
 * @return: EBADF The file descriptor of the underlying device is not valid.
 * @return: ENODEV The device is currently unreachable and/or unavailable.
 */
int adal_base_ffdc_clear(adal_t * adal, int scope);

/*!@brief
 * The adal_base_ffdc() interface is used to access other attributes of
 * the FFDC subsystem.
 *
 * @pre: The device was opened with a prior call to adal_base_open().
 *
 * @param: 'adal' adal_t of the target device.
 * @param: 'scope' Specifies the FFDC search mode.
 *         ADAL_FFDC_THREAD - perform specified action on the FFDC
 *         queue only for the calling thread.
 *         ADAL_FFDC_PROCESS - perform the specified action on the FFDC
 *         queue only for the calling process.
 * @param: 'action' Specifies the action to perform
 *         ADAL_FFDC_DISABLE is used to disable blacklisting
 *         ADAL_FFDC_ENABLE is used to enable blacklisting
 *
 * @return: 0 on success, -1 on failure with errno set as follows.
 * @return: EIO A low-level device driver or hardware I/O error occurred.
 * @return: EINVAL At least one of the input parameters is invalid.
 * @return: EBADF The file descriptor of the underlying device is not valid.
 * @return: ENODEV The device is currently unreachable and/or unavailable.
 */
int adal_base_ffdc(adal_t * adal, int scope, int action);

/*!@brief
 * The adal_base_wait() interface is used to wait for events to occur on
 * one or more devices.
 *
 * FIX ME!! Allow a timeout for each adal_wait_t element??
 *
 * @pre: The devices were opened with a prior call to adal_base_open().
 *
 * @note: This is a general interface which will block the caller until
 *       a particular events occur on the underlying devices.  Some
 *       examples are, waiting for data to become available for reading
 *       or waiting for buffer space to become available for writing.
 *
 * @note: The adal_wait_t structure is a control block with the following
 *       definition:
 *
 *           typedef struct {
 *                      int   event;
 *                   adal_t * adal;
 *                      int   results;
 *           } adal_wait_t;
 *
 * @note 'event' is the device event to "wait" for.
 *           ADAL_EVENT_WRITE - wait until a write to the device will not
 *                              block the calling thread (e.g. "buffer space"
 *                              is available).
 *           ADAL_EVENT_READ  - wait until a read from the device will not
 *                              block the calling thread (e.g. data is
 *                              available).
 * @note 'adal' is a pointer to the target ADAL device.
 *              if a NULL value is encountered, the call will return -1
 *              with errno set to EINVAL.
 * @note 'results' is the result of the operation, defined as:
 *          >0 if the event occured (will be set to event that occured)
 *           0 if the event did not occur before the timeout expired,
 *
 * @param: 'evt' Array of adal_wait_t control blocks.
 * @param: 'evt_nr' Number of elements in 'evt'.
 * @param: 'timeout' timeout value, see 'man 2 select'
 *        If 'timeout' is NULL this interface will wait forever.
 *
 * @return: N number of event that occured before the timeout expired,
 *         0 if no events occurred before the timeout expired,
 *        -1 on failure with errno set as follows:
 * @return: EIO A low-level device driver or hardware I/O error occurred.
 * @return: EINVAL At least one of the input parameters is invalid.
 * @return: EINTR A non-blocked signal was caught.
 * @return: EBADF The file descriptor of the underlying device is not valid.
 * @return: ENODEV The operation requires the FSI local bus to complete.
 * @return: EDEADLK Device is locked due to unextracted DD_FFDC.
 */
int adal_base_wait(adal_wait_t * evt, int evt_nr, struct timeval * timeout);

/*!@brief
 * The adal_base_setup_wait interface is used to intialize a adal_wait_t
 * structure prior to use.
 *
 * @param: 'fd' file descriptor
 * @param: 'evt_type' Type of event to wait for
 * @param: 'evt' pointer to adal_wait_t structure to initialize
 *
 * @return: 0 if successful, -1 otherwise with ERRNO set as follows:
 * @return: EINVAL An invalid parameter was supplied
 */
int adal_base_setup_wait(int fd, int evt_type, adal_wait_t * evt);

/*!@brief
 * The adal_base_wait_results interface is used to extract the results
 * from an adal_wait_t structure.
 *
 * @param: 'evt' pointer to adal_wait_t structure to retrieve results from
 *
 * @return: >0 if successful, -1 otherwise with ERRNO set as follows:
 * @return: EINVAL An invalid parameter was supplied
 */
int adal_base_wait_results(adal_wait_t * evt);

/*!@brief
 * the adal_base_wait_get_adal_t interface is used to retrieve the adal_t
 * from a adal_wait_t structure
 *
 * @param: 'evt' pointer to adal_wait_t structure to retrieve adal_t from
 *
 * @return: non-NULL if successful, NULL otherwise with ERRNO set as follows:
 * @return: EINVAL An invalid parameter was supplied
 */
adal_t * adal_base_wait_get_adal_t(adal_wait_t * evt);

/*!@brief
 * The adal_base_lctl interface is used to manage the exclusive access lock
 * on the underlying device.
 *
 * @param: 'adal' adal_t of the target device.
 * @param: 'type' lock operation to perform.
 *         ADAL_LCTL_LOCK - request the exclusive access lock.
 *         ADAL_LCTL_UNLOCK - release the exclusive access lock.
 *         ADAL_LCTL_TEST - test whether the lock is held or not.
 *
 * @return: 0 if successful, -1 otherwise with ERRNO set as follows:
 * @return: EDEADLK Locking this file would cause a deadlock condition.
 * @return: EINTR A non-blocked signal was caught.
 * @return: ENOLCK Too many segment locks open, lock table is full.
 * @return: EINVAL An invalid operation was specified in adal_t. *
 * @return: ENOSYS Function not implemented
 */
int adal_base_lctl(adal_t * adal, int type);


/* adal method to determine if a endian byte swap is needed */
/* based on the openbmc version */
bool adal_is_byte_swap_needed(void);

/* adal method to open a raw fsi device, shared across many adal implementations like sbefifo, scom, etc... */
adal_t * adal_fsi_open(const uint32_t i_device, int i_flags);

#ifdef __cplusplus
}
#endif

#endif /* __ADAL_BASE_INTERFACES_H__ */
