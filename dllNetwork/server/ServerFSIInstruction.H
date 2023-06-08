#ifndef _ServerFSIInstruction_H
#define _ServerFSIInstruction_H
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

/**
 * @file ServerFSIInstruction.H
 * @brief Provides a class for FSI instructions to run on the fsp server
 *
*/

//--------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------
#include <FSIInstruction.H>

class ServerFSIInstruction : public FSIInstruction
{
    protected:
    uint32_t scan_open(Handle ** handle, InstructionStatus & o_status);
    uint32_t scom_open(Handle ** handle, InstructionStatus & o_status);
    uint32_t fsi_open(Handle ** handle, InstructionStatus & o_status);

    protected:
    void scan_ffdc_and_reset(Handle ** handle, InstructionStatus & o_status);
    void scom_ffdc_and_reset(Handle ** handle, InstructionStatus & o_status);
    void gp_reg_ffdc_and_reset(Handle ** handle, InstructionStatus & o_status);
    void mbx_ffdc_and_reset(Handle ** handle, InstructionStatus & o_status);
    void sbefifo_ffdc_and_reset(Handle ** handle, InstructionStatus & o_status);

    uint32_t scan_close(Handle * handle);
    uint32_t scom_close(Handle * handle);
    uint32_t fsi_close(Handle * handle);
    ssize_t scan_write(Handle * i_handle, InstructionStatus & o_status);
    ssize_t scan_read(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status);
    ssize_t scom_write(Handle * i_handle, InstructionStatus & o_status, uint32_t i_index = 0);
    ssize_t scom_write_under_mask(Handle * i_handle, InstructionStatus & o_status);
    ssize_t scom_read(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status, uint32_t i_index = 0);

    ssize_t scan_get_register(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status);
    ssize_t scom_get_register(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status);
    ssize_t mbx_get_scratch_register(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status);
    ssize_t mbx_get_register(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status);
    ssize_t gp_reg_get(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status);
    ssize_t sbefifo_get_register(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status);
    ssize_t iic_get_register(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status);

    ssize_t scan_set_register(Handle * i_handle, InstructionStatus & o_status);
    ssize_t scom_set_register(Handle * i_handle, InstructionStatus & o_status);
    ssize_t mbx_set_scratch_register(Handle * i_handle, InstructionStatus & o_status);
    ssize_t mbx_set_register(Handle * i_handle, InstructionStatus & o_status);
    ssize_t gp_reg_set(Handle * i_handle, InstructionStatus & o_status);
    ssize_t sbefifo_set_register(Handle * i_handle, InstructionStatus & o_status);
    ssize_t iic_set_register(Handle * i_handle, InstructionStatus & o_status);
};

#endif // _ServerFSIInstruction_H
