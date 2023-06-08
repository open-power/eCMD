#ifndef _XDMAInstruction_H
#define _XDMAInstruction_H
//IBM_PROLOG_BEGIN_TAG
/* 
 * Copyright 2003,2020 IBM International Business Machines Corp.
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
 * @file XDMAInstruction.H
 * @brief Provides a class for DMA instructions being passed to the server
 *
*/


//--------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------
#include <Instruction.H>
#include <unistd.h>
#include <list>

#ifndef OTHER_USE
typedef adal_t Handle;
#endif

/**
 @brief XDMAInstruction class
*/
class XDMAInstruction : public Instruction {
  public:
    /** @name XDMAInstruction Constructors */
    //@{
    /**
     * @brief Default Constructor
     * @post Instruction() is called, version is set to 0x1, and type is set to XDMA 
     */
    XDMAInstruction(void);

    /**
     * @brief Default Destructor
     */
    ~XDMAInstruction(void);
    //@}

    // Member Functions
    /** @name Setup Function */
    //@{
    /**
     * @brief Used as alternate way to set variables normally set on construction
     * @retval ECMD_SUCCESS on success
     * @retval nonzero on failure
     * @post command, deviceString, address, readlength, msDelay and flags are set, if i_data is not NULL it is set to dat
     * @post flag has INSTRUCTION_FLAG_DEVSTR set
     * @post version is set to 0x1
     */
    uint32_t setup(InstructionCommand i_command, std::string &i_deviceString, uint32_t i_address, uint32_t i_readLength, uint32_t i_msDelay, uint32_t i_flags, const ecmdDataBuffer * i_data = NULL);
    
    /**
     * @brief Used as alternate way to set variables normally set on construction
     * @retval ECMD_SUCCESS on success
     * @retval nonzero on failure
     * @post command, deviceString, address, msDelay and flags are set, if i_data is not NULL it is set to dat
     * @post flag has INSTRUCTION_FLAG_DEVSTR set
     * @post version is set to 0x1
     */
    uint32_t setup(InstructionCommand i_command, std::string &i_deviceString, uint32_t i_address, uint32_t i_msDelay, uint32_t i_flags, const ecmdDataBuffer * i_data = NULL);
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
     * Operates on the following InstructionCommand types: DMA_COMMAND
     */
    uint32_t execute(ecmdDataBuffer & o_data, InstructionStatus & o_status, Handle ** io_handle);
    //@}

    /** @name Transport Functions */
    //@{
    /**
     * @brief Flatten all the object data into a uint8_t buffer
     * @param o_data Byte buffer to write the flattened data to
     * @param i_len Number of bytes in the o_data buffer
     * @post o_data buffer has a flattened version of the XDMAInstruction - must be pre-allocated
     * Data format (all in network byte order):
     * First Word:      version
     * Second Word:     command
     * Third Word:      flags
     * Fourth Word:     address
     * Fifth Word:      readLength
     * Sixth Word:      msDelay
     * Seventh Word:    deviceString size
     * Eigth Word:      data size
     * Multiple Words:  deviceString
     * Multiple Words:  data
     */
    uint32_t flatten(uint8_t * o_data, uint32_t i_len) const;

    /**
     * @brief Unflatten object data from a uint8_t buffer into this XDMAInstruction
     * @param i_data Byte buffer to read the flattened data from
     * @param i_len Number of bytes in the i_data buffer
     * @post This XDMAInstruction is allocated and initialized with the unflattened version of i_data
     * Data format -- see flatten()
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
     * <pre>
type,     address,        deviceString
bits 0:3  bits 4:31,      bits 32:63 </pre>
     */
    uint64_t getHash(void) const;

    /**
     * @brief closes Handle handle with adal_iic_close()
     */
    uint32_t closeHandle(Handle ** i_handle);

    std::string getInstructionVars(const InstructionStatus & i_status) const;
    //@}

  protected:
    uint32_t address;
    uint32_t readLength;
    uint32_t msDelay;
    const ecmdDataBuffer * data;
    std::list<ecmdDataBuffer *> mydata;
    std::string deviceString;

    virtual uint32_t xdma_open(Handle ** handle, InstructionStatus & o_status) { return -1; }
    virtual void xdma_ffdc(Handle ** handle, InstructionStatus & o_status) { }
    virtual void xdma_reset(Handle ** handle, InstructionStatus & o_status) { }
    virtual uint32_t xdma_close(Handle * handle) { return -1; }

    virtual ssize_t xdma_command(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status) { return -1; }

};

#endif // _XDMAInstruction_H
