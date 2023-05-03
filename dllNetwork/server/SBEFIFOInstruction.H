#ifndef _SBEFIFOInstruction_H
#define _SBEFIFOInstruction_H
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
 * @file SBEFIFOInstruction.H
 * @brief Provides a class for SBEFIFO instructions being passed to the fsp server
 *
*/


//--------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------
#include <Instruction.H>
#include <unistd.h>

/**
 @brief SBEFIFOInstruction class
*/
class SBEFIFOInstruction : public Instruction {
  public:
    /** @name SBEFIFOInstruction Constructors */
    //@{
    /**
     * @brief Default Constructor
     * @post Instruction() is called, version is set to 0x1, and type is set to SBEFIFO 
     */
    SBEFIFOInstruction(void);

    /**
     * @brief Default Destructor
     */
    ~SBEFIFOInstruction(void);
    //@}

    // Member Functions
    /** @name Setup Function */
    //@{
    /**
     * @brief Used as alternate way to set variables normally set on construction
     * @retval ECMD_SUCCESS on success
     * @retval nonzero on failure
     * @post command, deviceString, timeout, replyLength and flags are set, if i_data is not NULL it is copied to data, if i_port is not UINT32_MAX it is copied to port
     * @post flag has INSTRUCTION_FLAG_DEVSTR set
     */
    virtual uint32_t setup(InstructionCommand i_command, std::string &i_deviceString, uint32_t i_timeout, uint32_t i_replyLength, uint32_t i_flags, ecmdDataBuffer * i_data = NULL, uint32_t i_port = UINT32_MAX);
    //@}

    /** @name Execution Function */
    //@{
    /**
     * @brief Called by the fsp server or sim transfer class
     * @param o_data ecmdDataBuffer with output data
     * @param o_status InstructionStatus with error information
     * @param io_handle Handle * pointer for handle reuse between instructions
     * @retval ECMD_SUCCESS on success
     * @retval nonzero on failure
     * @post executes the appropriate command and sets o_data and o_status accordingly
     * Operates on the following InstructionCommand types: SUBMIT, REQUEST_RESET
     */
    uint32_t execute(ecmdDataBuffer & o_data, InstructionStatus & o_status, Handle ** io_handle);
    //@}

    /** @name Transport Functions */
    //@{
    /**
     * @brief Flatten all the object data into a uint8_t buffer
     * @param o_data Byte buffer to write the flattened data to
     * @param i_len Number of bytes in the o_data buffer
     * @post o_data buffer has a flattened version of the SBEFIFOInstruction - must be pre-allocated
     * Data format (all in network byte order):
     * First Word:      version
     * Second Word:     command
     * Third Word:      flags
     * Fourth Word:     timeout
     * Fifth Word:      deviceString size
     * Sixth Word:      data size
     * Multiple Words:  deviceString
     * Multiple Words:  data
     * ======= // SIM only
     * n - 2 Word:      cage
     * n - 1 Word:      node
     * n Word:          slot
     */
    uint32_t flatten(uint8_t * o_data, uint32_t i_len) const;

    /**
     * @brief Unflatten object data from a uint8_t buffer into this SBEFIFOInstruction
     * @param i_data Byte buffer to read the flattened data from
     * @param i_len Number of bytes in the i_data buffer
     * @post This SBEFIFOInstruction is allocated and initialized with the unflattened version of i_data
     * Data format (all in network byte order):
     * First Word:      version
     * Second Word:     command
     * Third Word:      flags
     * Fourth Word:     timeout
     * Fifth Word:      deviceString size
     * Sixth Word:      data size
     * Multiple Words:  deviceString
     * Multiple Words:  data
     * ======= // SIM only
     * n - 2 Word:      cage
     * n - 1 Word:      node
     * n Word:          slot
     */
    uint32_t unflatten(const uint8_t * i_data, uint32_t i_len);

    /**
     * @brief Return number of bytes needed for a buffer to flatten the object
     * @retval Number of bytes needed
     */
    uint32_t flattenSize(void) const;
    //@}

    /** @name Debug Function */
    //@{
    /**
     * @brief Creates a string with the instruction information in a -debug5.f format
     * @retval String with instruction information
     */
    std::string dumpInstruction(void) const;
    //@}

    /** @name Server Utility Function */
    //@{
    /**
     * @brief Creates a hash based upon the instruction type
     */
    uint64_t getHash(void) const;

    /**
     * @brief closes Handle handle based upon the command type
     */
    uint32_t closeHandle(Handle ** i_handle);

    std::string getInstructionVars(const InstructionStatus & i_status) const;
    //@}

  protected:
    ecmdDataBuffer data;
    std::string deviceString;
    uint32_t timeout;
    uint32_t replyLength;
    uint32_t port;

    virtual uint32_t sbefifo_open(Handle ** handle, InstructionStatus & o_status) { return -1; }
    virtual void sbefifo_ffdc_and_reset(Handle ** handle, InstructionStatus & o_status) { }
    virtual uint32_t sbefifo_close(Handle * handle) { return -1; }
    virtual ssize_t sbefifo_submit(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status, uint32_t & o_reply_wordcount) { return -1; }
    virtual uint32_t sbefifo_set_cmd_timeout(Handle * i_handle, uint32_t i_timeoutInSeconds, InstructionStatus & o_status) { return -1; }
    virtual uint32_t sbefifo_set_read_timeout(Handle * i_handle, uint32_t i_timeoutInSeconds, InstructionStatus & o_status) { return -1; }

};

#endif // _SBEFIFOInstruction_H
