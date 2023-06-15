#ifndef _ServerSBEFIFOInstruction_H
#define _ServerSBEFIFOInstruction_H
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

/**
 * @file ServerSBEFIFOInstruction.H
 * @brief Provides a class for SBEFIFO instructions to run on the fsp server
 *
*/

//--------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------
#include <SBEFIFOInstruction.H>

class ServerSBEFIFOInstruction : public SBEFIFOInstruction
{
    protected:
    uint32_t sbefifo_open(Handle ** handle, InstructionStatus & o_status);
    void sbefifo_ffdc_and_reset(Handle ** handle, InstructionStatus & o_status);
    uint32_t sbefifo_close(Handle * handle);
    ssize_t sbefifo_submit(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status, uint32_t & o_reply_wordcount);
    uint32_t sbefifo_set_cmd_timeout(Handle * i_handle, uint32_t i_timeoutInSeconds, InstructionStatus & o_status);
    uint32_t sbefifo_set_read_timeout(Handle * i_handle, uint32_t i_timeoutInSeconds, InstructionStatus & o_status);

    private:
    uint32_t fsi_open(Handle** handle, InstructionStatus & o_status);
    uint32_t fsi_close(Handle* handle);

};

#endif // _ServerFSIInstruction_H
