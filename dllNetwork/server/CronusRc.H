#ifndef CRONUSRC_H
#define CRONUSRC_H /* define */
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
#ifndef ECMD_ERR_CRONUS
 #define ECMD_ERR_CRONUS                         0x02000000 ///< Error came from Cronus
#endif

/* ----------------------------------------------------------------- */
/* The following return codes are for the C++ chip class, as well as */
/* all of its member functions, child classes, and their member      */
/* functions.                                                        */
/* ----------------------------------------------------------------- */

/* ----------------------------------------------------------------- */
/* Return codes for FSP Server                                       */
/* ----------------------------------------------------------------- */ 
/* Device Driver Error Defines */
#define TRANSFER_INVALID_COMMAND_ARG              (ECMD_ERR_CRONUS | 0x400100)
#define TRANSFER_INVALID_RECEIVE_ARG              (ECMD_ERR_CRONUS | 0x400110)
#define TRANSFER_MISMATCH_ARG_KEYS                (ECMD_ERR_CRONUS | 0x400120)
#define TRANSFER_INVALID_CRC_FLAGS                (ECMD_ERR_CRONUS | 0x400130)
#define TRANSFER_UNINITIALIZED_DRIVER             (ECMD_ERR_CRONUS | 0x400140)
#define TRANSFER_DEVICE_DRIVER_INIT_FAIL          (ECMD_ERR_CRONUS | 0x400150)
#define TRANSFER_ACK_TIMEOUT                      (ECMD_ERR_CRONUS | 0x400160)
#define TRANSFER_PIPE_SEND_FAIL                   (ECMD_ERR_CRONUS | 0x400170)
#define TRANSFER_COMMPTR_OVERFLOW                 (ECMD_ERR_CRONUS | 0x400180)
#define TRANSFER_DATAPTR_OVERFLOW                 (ECMD_ERR_CRONUS | 0x400190)
#define TRANSFER_RECEIVE_TIMEOUT                  (ECMD_ERR_CRONUS | 0x400200)
#define TRANSFER_RECEIVE_FAIL                     (ECMD_ERR_CRONUS | 0x400210)
#define TRANSFER_RECEIVE_KEY_MISMATCH             (ECMD_ERR_CRONUS | 0x400220)
#define TRANSFER_MEMALLOC_FAIL                    (ECMD_ERR_CRONUS | 0x400230)
#define TRANSFER_SEND_OVERFLOW                    (ECMD_ERR_CRONUS | 0x400240)
#define TRANSFER_VERSION_MISMATCH                 (ECMD_ERR_CRONUS | 0x400250)
#define TRANSFER_FUNCTION_NOT_DEFINED             (ECMD_ERR_CRONUS | 0x400260)
#define TRANSFER_INVALID_TRANSFER_TYPE            (ECMD_ERR_CRONUS | 0x400270)
#define TRANSFER_FSP_BUSY                         (ECMD_ERR_CRONUS | 0x400280)
#define TRANSFER_PTHREAD_FAIL                     (ECMD_ERR_CRONUS | 0x400290)

/* Control Store Load Error Defines */
#define SERVER_INVALID_INSTRUCTION_OPTION              (ECMD_ERR_CRONUS | 0x401000)

#define SERVER_INVALID_COMMAND_BLOCK_FIELD_COMMAND     (ECMD_ERR_CRONUS | 0x401010)
#define SERVER_INVALID_COMMAND_BLOCK_FIELD_JTAG        (ECMD_ERR_CRONUS | 0x401020)
#define SERVER_INVALID_COMMAND_BLOCK_FIELD_FLAGS       (ECMD_ERR_CRONUS | 0x401030)
#define SERVER_INVALID_COMMAND_BLOCK_FIELD_KEY         (ECMD_ERR_CRONUS | 0x401050)
#define SERVER_INVALID_COMMAND_BLOCK_FIELD_COMMLEN     (ECMD_ERR_CRONUS | 0x401060)
#define SERVER_INVALID_COMMAND_BLOCK_FIELD_COMMPTR     (ECMD_ERR_CRONUS | 0x401070)
#define SERVER_INVALID_COMMAND_BLOCK_FIELD_COMMPOS     (ECMD_ERR_CRONUS | 0x401040)
#define SERVER_INVALID_COMMAND_BLOCK_FIELD_ACTCOMM     (ECMD_ERR_CRONUS | 0x401100)
#define SERVER_INVALID_COMMAND_BLOCK_FIELD_ACTCOMMLEN  (ECMD_ERR_CRONUS | 0x401110)
#define SERVER_INVALID_COMMAND_BLOCK_FIELD_TOTALSHFT   (ECMD_ERR_CRONUS | 0x401120)
#define SERVER_INVALID_COMMAND_BLOCK_FIELD_DATALEN     (ECMD_ERR_CRONUS | 0x401080)
#define SERVER_INVALID_COMMAND_BLOCK_FIELD_DATAPTR     (ECMD_ERR_CRONUS | 0x401090)


#define SERVER_INVALID_TAP_STATE                       (ECMD_ERR_CRONUS | 0x401200)
#define SERVER_UNABLE_TO_READ_SPMEM                    (ECMD_ERR_CRONUS | 0x401210)
#define SERVER_UNABLE_TO_WRITE_SPMEM                   (ECMD_ERR_CRONUS | 0x401220)
#define SERVER_FAILED_TITAN_SCAN                       (ECMD_ERR_CRONUS | 0x401230)
#define SERVER_TITAN_SCAN_TIMEOUT                      (ECMD_ERR_CRONUS | 0x401240)
#define SERVER_TAP_TIMEOUT                             (ECMD_ERR_CRONUS | 0x401250)
#define SERVER_SENDCMD_TIMEOUT                         (ECMD_ERR_CRONUS | 0x401260)
#define SERVER_LONGIN_TIMEOUT                          (ECMD_ERR_CRONUS | 0x401270)
#define SERVER_LONGOUT_TIMEOUT                         (ECMD_ERR_CRONUS | 0x401280)
#define SERVER_READ_TIMEOUT                            (ECMD_ERR_CRONUS | 0x401290)
#define SERVER_WRITE_TIMEOUT                           (ECMD_ERR_CRONUS | 0x401300)
#define SERVER_READ_WRITE_TIMEOUT                      (ECMD_ERR_CRONUS | 0x401310)
#define SERVER_READVPD_FAIL                            (ECMD_ERR_CRONUS | 0x401320)
#define SERVER_WRITEVPD_FAIL                           (ECMD_ERR_CRONUS | 0x401321)
#define SERVER_READKEYWORD_FAIL                        (ECMD_ERR_CRONUS | 0x401322)
#define SERVER_WRITEKEYWORD_FAIL                       (ECMD_ERR_CRONUS | 0x401323)
#define SERVER_FRUSTATUS_FAIL                          (ECMD_ERR_CRONUS | 0x401324)
#define SERVER_DMAIN_TIMEOUT                           (ECMD_ERR_CRONUS | 0x401330)
#define SERVER_DMAOUT_TIMEOUT                          (ECMD_ERR_CRONUS | 0x401340)
#define SERVER_DATAPTR_OVERFLOW                        (ECMD_ERR_CRONUS | 0x401350)
#define SERVER_POWER_OFF                               (ECMD_ERR_CRONUS | 0x401360)
#define SERVER_DMA_ENGINE_LOCK_GET_FAIL                (ECMD_ERR_CRONUS | 0x401370)
#define SERVER_DMAEXER_THREAD_CREATE_FAIL              (ECMD_ERR_CRONUS | 0x401380)
#define SERVER_DMAEXER_IN_PROGRESS_CMD_FAILED          (ECMD_ERR_CRONUS | 0x401390)
#define SERVER_DASDEXER_IN_PROGRESS_CMD_FAILED         (ECMD_ERR_CRONUS | 0x401600)
#define SERVER_DASDEXER_THREAD_CREATE_FAIL             (ECMD_ERR_CRONUS | 0x401610)
#define SERVER_MEMALLOC_FAIL                           (ECMD_ERR_CRONUS | 0x401620)
#define SERVER_IOCTL_RECEIVE_UNDERFLOW                 (ECMD_ERR_CRONUS | 0x401630)
#define SERVER_IOCTL_SEND_OVERFLOW                     (ECMD_ERR_CRONUS | 0x401640)
#define SERVER_IOCTL_RECEIVE_OVERFLOW                  (ECMD_ERR_CRONUS | 0x401650)
#define SERVER_JTAG_SEMGET_FAILURE                     (ECMD_ERR_CRONUS | 0x401660)
#define SERVER_PCI_PROBE_FAILURE                       (ECMD_ERR_CRONUS | 0x401670)
#define SERVER_BAD_JTAGDEVICE_ID                       (ECMD_ERR_CRONUS | 0x401680)
#define SERVER_HOST_PCI_SETUP_FAILURE                  (ECMD_ERR_CRONUS | 0x401690)
#define SERVER_COMMAND_BUFFER_OVERFLOW                 (ECMD_ERR_CRONUS | 0x401700)
#define SERVER_STSR_WORKAROUND_TIMEOUT                 (ECMD_ERR_CRONUS | 0x401710)
#define SERVER_DEVICE_DRIVER_OPEN_FAILURE              (ECMD_ERR_CRONUS | 0x401720)
#define SERVER_SCOM_NOT_SUPPORTED_NON_FSI              (ECMD_ERR_CRONUS | 0x401730)
#define SERVER_READ_WRITE_NOT_SUPPORTED_NON_JTAG       (ECMD_ERR_CRONUS | 0x401731)
#define SERVER_SENDCMD_NOT_SUPPORTED_NON_JTAG          (ECMD_ERR_CRONUS | 0x401732)
#define SERVER_TOTAP_NOT_SUPPORTED_NON_JTAG            (ECMD_ERR_CRONUS | 0x401733)
#define SERVER_NO_FSI_SUPPORT                          (ECMD_ERR_CRONUS | 0x401734)
#define SERVER_INVALID_FSI_DEVICE                      (ECMD_ERR_CRONUS | 0x401735)
#define SERVER_FSI_DEVICE_BUSY                         (ECMD_ERR_CRONUS | 0x401736)
#define SERVER_FSI_GENERAL_ERROR                       (ECMD_ERR_CRONUS | 0x401737)
#define SERVER_SCAN_ADAL_RESET_FAIL                    (ECMD_ERR_CRONUS | 0x401738)
#define SERVER_SCAN_FFDC_EXTRACT_FAIL                  (ECMD_ERR_CRONUS | 0x401739)
#define SERVER_SCAN_FFDC_UNLOCK_FAIL                   (ECMD_ERR_CRONUS | 0x401740)
#define SERVER_FSI_SCOM_WRITE_FAIL                     (ECMD_ERR_CRONUS | 0x401741)
#define SERVER_FSI_SCOM_READ_FAIL                      (ECMD_ERR_CRONUS | 0x401742)
#define SERVER_FSI_SCAN_WRITE_FAIL                     (ECMD_ERR_CRONUS | 0x401743)
#define SERVER_FSI_SCAN_READ_FAIL                      (ECMD_ERR_CRONUS | 0x401744)
#define SERVER_FSI_REGISTER_NOT_SUPPORTED              (ECMD_ERR_CRONUS | 0x401745)
#define SERVER_FSI_HW_DRA_NEEDED                       (ECMD_ERR_CRONUS | 0x401746)
#define SERVER_SCOM_ADAL_RESET_FAIL                    (ECMD_ERR_CRONUS | 0x401747)
#define SERVER_SCOM_FFDC_EXTRACT_FAIL                  (ECMD_ERR_CRONUS | 0x401749)
#define SERVER_SCOM_FFDC_UNLOCK_FAIL                   (ECMD_ERR_CRONUS | 0x401751)
#define SERVER_SCAN_CLOSE_FAIL                         (ECMD_ERR_CRONUS | 0x401753)
#define SERVER_SCOM_CLOSE_FAIL                         (ECMD_ERR_CRONUS | 0x401755)
#define SERVER_FSI_SCOM_ERROR                          (ECMD_ERR_CRONUS | 0x401757)
#define SERVER_FSI_INIT_FAIL                           (ECMD_ERR_CRONUS | 0x401759)
#define SERVER_FSI_USE_FSP_FAIL                        (ECMD_ERR_CRONUS | 0x40175A)
#define SERVER_FSI_DRA_READ_FAIL                       (ECMD_ERR_CRONUS | 0x401760)
#define SERVER_FSI_DRA_WRITE_FAIL                      (ECMD_ERR_CRONUS | 0x401761)
#define SERVER_FSI_SHIFT_SUBMIT_FAIL                   (ECMD_ERR_CRONUS | 0x401762)
#define SERVER_FSI_SHIFT_ADD_OP_FAIL                   (ECMD_ERR_CRONUS | 0x401763)
#define SERVER_FSI_SHIFT_STREAM_NOT_SETUP              (ECMD_ERR_CRONUS | 0x401764)
#define SERVER_FSI_SBEFIFO_READ_FAIL                   (ECMD_ERR_CRONUS | 0x401765)
#define SERVER_FSI_SBEFIFO_WRITE_FAIL                  (ECMD_ERR_CRONUS | 0x401766)


#define SERVER_JTAG_GENERAL_ERROR                      (ECMD_ERR_CRONUS | 0x401800)
#define SERVER_JTAG_INVALID_DEVICE                     (ECMD_ERR_CRONUS | 0x401801)
#define SERVER_JTAG_DEVICE_BUSY                        (ECMD_ERR_CRONUS | 0x401802)
#define SERVER_JTAG_GETLOCK_FAIL                       (ECMD_ERR_CRONUS | 0x401803)
#define SERVER_JTAG_UNLOCK_FAIL                        (ECMD_ERR_CRONUS | 0x401804)
#define SERVER_JTAG_SENDCMD_FAIL                       (ECMD_ERR_CRONUS | 0x401805)
#define SERVER_JTAG_CLOSE_FAIL                         (ECMD_ERR_CRONUS | 0x401806)
#define SERVER_JTAG_SCANIN_FAIL                        (ECMD_ERR_CRONUS | 0x401807)
#define SERVER_JTAG_SCANOUT_FAIL                       (ECMD_ERR_CRONUS | 0x401808)
#define SERVER_JTAG_TOTAP_FAIL                         (ECMD_ERR_CRONUS | 0x401809)
#define SERVER_JTAG_READ_SPMEM_FAIL                    (ECMD_ERR_CRONUS | 0x401810)
#define SERVER_JTAG_WRITE_SPMEM_FAIL                   (ECMD_ERR_CRONUS | 0x401811)
#define SERVER_JTAG_GENERIC_SCAN_FAIL                  (ECMD_ERR_CRONUS | 0x401813)
#define SERVER_JTAG_FFDC_UNLOCK_FAIL                   (ECMD_ERR_CRONUS | 0x401815)
#define SERVER_JTAG_USE_FSP_FAIL                       (ECMD_ERR_CRONUS | 0x401817)

#define SERVER_DMA_GENERAL_ERROR                       (ECMD_ERR_CRONUS | 0x401900)
#define SERVER_DMA_INVALID_DEVICE                      (ECMD_ERR_CRONUS | 0x401901)
#define SERVER_DMA_DEVICE_BUSY                         (ECMD_ERR_CRONUS | 0x401902)
#define SERVER_DMA_OPEN_FAIL                           (ECMD_ERR_CRONUS | 0x401904)
#define SERVER_DMA_CLOSE_FAIL                          (ECMD_ERR_CRONUS | 0x401906)
#define SERVER_DMA_FFDC_EXTRACT_FAIL                   (ECMD_ERR_CRONUS | 0x401907)
#define SERVER_DMA_FFDC_UNLOCK_FAIL                    (ECMD_ERR_CRONUS | 0x401909)
#define SERVER_DMA_USE_FSP_FAIL                        (ECMD_ERR_CRONUS | 0x40190A)
#define SERVER_DMA_ECMD_PPC_MISSING                    (ECMD_ERR_CRONUS | 0x40190B)
#define SERVER_DMA_TEMP_FILE_ERROR_MKSTEMP             (ECMD_ERR_CRONUS | 0x40190C)
#define SERVER_DMA_TEMP_FILE_ERROR_CLOSE               (ECMD_ERR_CRONUS | 0x40190D)
#define SERVER_DMA_COMMAND_START_FAIL                  (ECMD_ERR_CRONUS | 0x40190E)
#define SERVER_DMA_COMMAND_FAIL                        (ECMD_ERR_CRONUS | 0x40190F)
#define SERVER_DMA_TEMP_FILE_ERROR_UNLINK              (ECMD_ERR_CRONUS | 0x401910)


#define SERVER_I2C_GENERAL_ERROR                       (ECMD_ERR_CRONUS | 0x401950)
#define SERVER_I2C_OPEN_FAIL                           (ECMD_ERR_CRONUS | 0x401951)
#define SERVER_I2C_CLOSE_FAIL                          (ECMD_ERR_CRONUS | 0x401952)
#define SERVER_I2C_WRITE_FAIL                          (ECMD_ERR_CRONUS | 0x401953)
#define SERVER_I2C_READ_FAIL                           (ECMD_ERR_CRONUS | 0x401954)
#define SERVER_I2C_CONFIG_FAIL                         (ECMD_ERR_CRONUS | 0x401955)
#define SERVER_I2C_FFDC_UNLOCK_FAIL                    (ECMD_ERR_CRONUS | 0x401956)
#define SERVER_I2C_SCOM_WRITE_FAIL                     (ECMD_ERR_CRONUS | 0x401957)
#define SERVER_I2C_SCOM_READ_FAIL                      (ECMD_ERR_CRONUS | 0x401958)
#define SERVER_I2C_SCAN_WRITE_FAIL                     (ECMD_ERR_CRONUS | 0x401959)
#define SERVER_I2C_SCAN_READ_FAIL                      (ECMD_ERR_CRONUS | 0x40195A)
#define SERVER_I2C_USE_FSP_FAIL                        (ECMD_ERR_CRONUS | 0x40195B)
#define SERVER_I2C_RESET_FULL_FAIL                     (ECMD_ERR_CRONUS | 0x40195C)
#define SERVER_I2C_RESET_LIGHT_FAIL                    (ECMD_ERR_CRONUS | 0x40195D)
#define SERVER_I2C_RESET_SUCCESS                       (ECMD_ERR_CRONUS | 0x40195E)

#define SERVER_GPIO_GENERAL_ERROR                      (ECMD_ERR_CRONUS | 0x401960)
#define SERVER_GPIO_OPEN_FAIL                          (ECMD_ERR_CRONUS | 0x401961)
#define SERVER_GPIO_CLOSE_FAIL                         (ECMD_ERR_CRONUS | 0x401962)
#define SERVER_GPIO_WRITE_FAIL                         (ECMD_ERR_CRONUS | 0x401963)
#define SERVER_GPIO_READ_FAIL                          (ECMD_ERR_CRONUS | 0x401964)
#define SERVER_GPIO_CONFIG_FAIL                        (ECMD_ERR_CRONUS | 0x401965)
#define SERVER_GPIO_INVALID_TYPE                       (ECMD_ERR_CRONUS | 0x401967)
#define SERVER_GPIO_FFDC_UNLOCK_FAIL                   (ECMD_ERR_CRONUS | 0x401968)

#define SERVER_PSI_GENERAL_ERROR                       (ECMD_ERR_CRONUS | 0x401970)
#define SERVER_PSI_OPEN_FAIL                           (ECMD_ERR_CRONUS | 0x401971)
#define SERVER_PSI_CLOSE_FAIL                          (ECMD_ERR_CRONUS | 0x401972)
#define SERVER_PSI_EI_REG_WRITE_FAIL                   (ECMD_ERR_CRONUS | 0x401973)
#define SERVER_PSI_EI_REG_READ_FAIL                    (ECMD_ERR_CRONUS | 0x401974)
#define SERVER_PSI_SCOM_WRITE_FAIL                     (ECMD_ERR_CRONUS | 0x401975)
#define SERVER_PSI_SCOM_READ_FAIL                      (ECMD_ERR_CRONUS | 0x401976)
#define SERVER_PSI_RESET_FAIL                          (ECMD_ERR_CRONUS | 0x401977)
#define SERVER_PSI_CALIB_FAIL                          (ECMD_ERR_CRONUS | 0x401978)
#define SERVER_PSI_VERIFY_FAIL                         (ECMD_ERR_CRONUS | 0x401979)
#define SERVER_PSI_INIT_FAIL                           (ECMD_ERR_CRONUS | 0x40197A)
#define SERVER_PSI_ENABLE_FAIL                         (ECMD_ERR_CRONUS | 0x40197B)
#define SERVER_PSI_SET_SPEED_FAIL                      (ECMD_ERR_CRONUS | 0x40197C)
#define SERVER_PSI_FFDC_UNLOCK_FAIL                    (ECMD_ERR_CRONUS | 0x40197D)
#define SERVER_PSI_USE_FSP_FAIL                        (ECMD_ERR_CRONUS | 0x40197E)
#define SERVER_PSI_CMU_REG_WRITE_FAIL                  (ECMD_ERR_CRONUS | 0x40197F)
#define SERVER_PSI_CMU_REG_READ_FAIL                   (ECMD_ERR_CRONUS | 0x401980)

#define SERVER_IIC_NON_SUPPORTED_PORT_ON_ENGINE_3      (ECMD_ERR_CRONUS | 0x401400)
#define SERVER_IIC_NON_SUPPORTED_PORT_ON_ENGINE_4      (ECMD_ERR_CRONUS | 0x401410)
#define SERVER_IIC_NON_SUPPORTED_ENGINE                (ECMD_ERR_CRONUS | 0x401420)
#define SERVER_IIC_NO_SLAVE_ADDRESS                    (ECMD_ERR_CRONUS | 0x401420)
#define SERVER_IIC_NON_SUPPORTED_COMMAND               (ECMD_ERR_CRONUS | 0x401440)
#define SERVER_IIC_OPERATION_TIME_OUT_ON_START_BYTE    (ECMD_ERR_CRONUS | 0x401450)
#define SERVER_IIC_OPERATION_TIME_OUT_ON_NEXT_BYTE     (ECMD_ERR_CRONUS | 0x401460)
#define SERVER_IIC_OPERATION_TIME_OUT_ON_STOP_BYTE     (ECMD_ERR_CRONUS | 0x401470)
#define SERVER_IIC_BAD_REQUESTED_DATA_LENGTH           (ECMD_ERR_CRONUS | 0x401480)
#define SERVER_IIC_NO_ADDRESS_LENGHT_SUPPLIED          (ECMD_ERR_CRONUS | 0x401490)
#define SERVER_IIC_BUSY                                (ECMD_ERR_CRONUS | 0x401492)
#define SERVER_IIC_SLAVE_TIMEOUT                       (ECMD_ERR_CRONUS | 0x401494)
#define SERVER_IIC_MASTER_TIMEOUT                      (ECMD_ERR_CRONUS | 0x401496)
#define SERVER_IIC_SLAVE_NO_RESP                       (ECMD_ERR_CRONUS | 0x401498)
#define SERVER_IIC_BUS_OPEN_FAILURE                    (ECMD_ERR_CRONUS | 0x401530)
#define SERVER_IIC_WRITE_FAILURE                       (ECMD_ERR_CRONUS | 0x401532)
#define SERVER_IIC_LOCK_TIMEOUT                        (ECMD_ERR_CRONUS | 0x401533)
#define SERVER_IIC_SPEED_INVALID                       (ECMD_ERR_CRONUS | 0x401535)
#define SERVER_IIC_INVALID_OFFSET_SIZE                 (ECMD_ERR_CRONUS | 0x401537)

#define SERVER_POWER_TRANSITION_FAILED                 (ECMD_ERR_CRONUS | 0x401500)
#define SERVER_COMMAND_NOT_SUPPORTED                   (ECMD_ERR_CRONUS | 0x401510)
#define SERVER_PROXY_INTERFACE_FAILURE                 (ECMD_ERR_CRONUS | 0x401520)

#define SERVER_FT_INVALID_HANDLE                       (ECMD_ERR_CRONUS | 0x401A01)
#define SERVER_FT_DEVICE_NOT_FOUND                     (ECMD_ERR_CRONUS | 0x401A02)
#define SERVER_FT_DEVICE_NOT_OPENED                    (ECMD_ERR_CRONUS | 0x401A03)
#define SERVER_FT_IO_ERROR                             (ECMD_ERR_CRONUS | 0x401A04)
#define SERVER_FT_INSUFFICIENT_RESOURCES               (ECMD_ERR_CRONUS | 0x401A05)
#define SERVER_FT_INVALID_PARAMETER                    (ECMD_ERR_CRONUS | 0x401A06)
#define SERVER_FT_INVALID_BAUD_RATE                    (ECMD_ERR_CRONUS | 0x401A07)
#define SERVER_FT_DEVICE_NOT_OPENED_FOR_ERASE          (ECMD_ERR_CRONUS | 0x401A08)
#define SERVER_FT_DEVICE_NOT_OPENED_FOR_WRITE          (ECMD_ERR_CRONUS | 0x401A09)
#define SERVER_FT_FAILED_TO_WRITE_DEVICE               (ECMD_ERR_CRONUS | 0x401A0A)
#define SERVER_FT_EEPROM_READ_FAILED                   (ECMD_ERR_CRONUS | 0x401A0B)
#define SERVER_FT_EEPROM_WRITE_FAILED                  (ECMD_ERR_CRONUS | 0x401A0C)
#define SERVER_FT_EEPROM_ERASE_FAILED                  (ECMD_ERR_CRONUS | 0x401A0D)
#define SERVER_FT_EEPROM_NOT_PRESENT                   (ECMD_ERR_CRONUS | 0x401A0E)
#define SERVER_FT_EEPROM_NOT_PROGRAMMED                (ECMD_ERR_CRONUS | 0x401A0F)
#define SERVER_FT_INVALID_ARGS                         (ECMD_ERR_CRONUS | 0x401A10)
#define SERVER_FT_NOT_SUPPORTED                        (ECMD_ERR_CRONUS | 0x401A11)
#define SERVER_FT_OTHER_ERROR                          (ECMD_ERR_CRONUS | 0x401A12)
#define SERVER_FT_FTDIJTAG_NULL                        (ECMD_ERR_CRONUS | 0x401A20)

#define SERVER_ENABLE_MEM_VOLTAGES_FAIL                (ECMD_ERR_CRONUS | 0x401B00)
#define SERVER_DISABLE_MEM_VOLTAGES_FAIL               (ECMD_ERR_CRONUS | 0x401B01)

#define SERVER_INVALID_SBEFIFO_DEVICE                  (ECMD_ERR_CRONUS | 0x401C00)
#define SERVER_SBEFIFO_CLOSE_FAIL                      (ECMD_ERR_CRONUS | 0x401C01)
#define SERVER_SBEFIFO_ADAL_RESET_FAIL                 (ECMD_ERR_CRONUS | 0x401C02)
#define SERVER_SBEFIFO_SUBMIT_FAIL                     (ECMD_ERR_CRONUS | 0x401C03)

#define SERVER_MEMPROC_CHECK_ARGS_FAIL                 (ECMD_ERR_CRONUS | 0x401D00)
#define SERVER_MEMPROC_CHECK_FBC_STATE_FAIL            (ECMD_ERR_CRONUS | 0x401D01)
#define SERVER_MEMPROC_ADU_LOCK_FAIL                   (ECMD_ERR_CRONUS | 0x401D02)
#define SERVER_MEMPROC_ADU_DO_STREAM_FAIL              (ECMD_ERR_CRONUS | 0x401D03)
#define SERVER_MEMPROC_ADU_SEND_FBC_OP_FAIL            (ECMD_ERR_CRONUS | 0x401D04)
#define SERVER_MEMPROC_ADU_CHECK_STATUS_ADDRESS_ERROR  (ECMD_ERR_CRONUS | 0x401D05)
#define SERVER_MEMPROC_ADU_CHECK_STATUS_FAIL           (ECMD_ERR_CRONUS | 0x401D06)

#define SERVER_CONTROL_UNKNOWN_QUERYSP                 (ECMD_ERR_CRONUS | 0x401E00)

      /* This is return upon successful completion of command */
#define SERVER_COMMAND_COMPLETE                        (ECMD_ERR_CRONUS | 0x402000)
#define SERVER_UNKNOWN_INSTRUCTION                     (ECMD_ERR_CRONUS | 0x402010)
#define SERVER_UNKNOWN_INSTRUCTION_VERSION             (ECMD_ERR_CRONUS | 0x402020)
#define SERVER_AUTHORIZATION_NEEDED                    (ECMD_ERR_CRONUS | 0x402030)
#define SERVER_AUTHORIZATION_INVALID                   (ECMD_ERR_CRONUS | 0x402040)
#define SERVER_PREVIOUS_INSTRUCTION_FAILED             (ECMD_ERR_CRONUS | 0x402050)
#define SERVER_PREVIOUS_COMMAND_FAILED                 (ECMD_ERR_CRONUS | 0x402060)
#define SERVER_RESOURCE_IN_USE                         (ECMD_ERR_CRONUS | 0x402070)
#define SERVER_CHICDOIPL_FAILURE                       (ECMD_ERR_CRONUS | 0x402080)
#define SERVER_MAILBOX_START_FAILURE                   (ECMD_ERR_CRONUS | 0x402082)
#define SERVER_MAILBOX_ISTEP_FAILURE                   (ECMD_ERR_CRONUS | 0x402084)
#define SERVER_MAILBOX_TRACE_ENABLE_FAILURE            (ECMD_ERR_CRONUS | 0x402085)
#define SERVER_MAILBOX_TRACE_DISABLE_FAILURE           (ECMD_ERR_CRONUS | 0x402086)
#define SERVER_MAILBOX_TRACE_FILE_NOT_FOUND            (ECMD_ERR_CRONUS | 0x402087)
#define SERVER_MAILBOX_STRING_FILE_NOT_FOUND           (ECMD_ERR_CRONUS | 0x402088)
#define SERVER_MAILBOX_TRACE_FILE_MOVE_FAILURE         (ECMD_ERR_CRONUS | 0x402089)

#define SERVER_PNOR_ERROR                              (ECMD_ERR_CRONUS | 0x402100)
#define SERVER_PNOR_DEVICE_FAIL_OPEN                   (ECMD_ERR_CRONUS | 0x402101)
#define SERVER_PNOR_DEVICE_FAIL_CLOSE                  (ECMD_ERR_CRONUS | 0x402102)
#define SERVER_PNOR_PARTITION_NOT_FOUND                (ECMD_ERR_CRONUS | 0x402103)
#define SERVER_PNOR_PARTITION_TYPE_INVALID             (ECMD_ERR_CRONUS | 0x402104)
#define SERVER_PNOR_PARTITION_EMPTY                    (ECMD_ERR_CRONUS | 0x402105)
#define SERVER_PNOR_READ_ERROR                         (ECMD_ERR_CRONUS | 0x402106)
#define SERVER_PNOR_WRITE_ERROR                        (ECMD_ERR_CRONUS | 0x402107)
#define SERVER_PNOR_FSYNC_ERROR                        (ECMD_ERR_CRONUS | 0x402108)



#endif /* CRONUSRC_H */


