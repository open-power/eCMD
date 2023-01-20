#ifndef _FSIInstruction_H
#define _FSIInstruction_H
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
 * @file FSIInstruction.H
 * @brief Provides a class for FSI instructions being passed to the fsp server
 *
*/


//--------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------
#include <Instruction.H>
#include <unistd.h>

/**
 @brief FSIInstruction class
*/
class FSIInstruction : public Instruction {
  public:
    /**
     @brief All the types of CFAM register accesses
     */
    typedef enum {
    CFAM_TYPE_INVALID,
    CFAM_TYPE_SCAN,
    CFAM_TYPE_SCOM,
    CFAM_TYPE_MBX,
    CFAM_TYPE_MBX_SCRATCH,
    CFAM_TYPE_GP_REG,
    CFAM_TYPE_SBEFIFO,
    } CFAMType;

    /** @name FSIInstruction Constructors */
    //@{
    /**
     * @brief Default Constructor
     * @post Instruction() is called, version is set to 0x6, and type is set to FSI 
     */
    FSIInstruction(void);

    /**
     * @brief Constructor with all variables passed in
     * @post Instruction() is called, command, cfamid, linkid, cmaster, address, length, and flags are set, version is set to 0x4, and type is set to FSI, if i_data is not NULL it is copied to data, if i_mask is not NULL it is copied to mask
     * @post flag has INSTRUCTION_FLAG_64BIT_ADDRESS set if address is greater than 32-bits
     * @post version is set to 0x2 if 32-bit address used
     * @post version is set to 0x6 if bulk scom
     */
    FSIInstruction(InstructionCommand i_command, uint32_t i_cfamid, uint32_t i_linkid, uint32_t i_cmaster, uint64_t i_address, uint32_t i_length, uint32_t i_flags, ecmdDataBuffer * i_data = NULL, ecmdDataBuffer * i_mask = NULL);

    /**
     * @brief Default Destructor
     */
    ~FSIInstruction(void);
    //@}

    // Member Functions
    /** @name Setup Function */
    //@{
    /**
     * @brief Used as alternate way to set variables normally set on construction
     * @retval ECMD_SUCCESS on success
     * @retval nonzero on failure
     * @post command, cfamid, linkid, cmaster, address, length, and flags are set, if i_data is not NULL it is copied to data, if i_mask is not NULL it is copied to mask
     * @post flag has INSTRUCTION_FLAG_64BIT_ADDRESS set if address is greater than 32-bits
     * @post version is set to 0x4 by default
     * @post version is set to 0x2 if 32-bit address used
     * @post version is set to 0x6 if bulk scom
     */
    virtual uint32_t setup(InstructionCommand i_command, uint32_t i_cfamid, uint32_t i_linkid, uint32_t i_cmaster, uint64_t i_address, uint32_t i_length, uint32_t i_flags, ecmdDataBuffer * i_data = NULL, ecmdDataBuffer * i_mask = NULL);

    /**
     * @brief Used as alternate way to set variables normally set on construction
     * @retval ECMD_SUCCESS on success
     * @retval nonzero on failure
     * @post command, deviceString, address, length, and flags are set, if i_data is not NULL it is copied to data, if i_mask is not NULL it is copied to mask
     * @post flag has INSTRUCTION_FLAG_64BIT_ADDRESS set if address is greater than 32-bits
     * @post flag has INSTRUCTION_FLAG_DEVSTR set
     * @post version is set to 0x5 if not bulk scom
     */
    virtual uint32_t setup(InstructionCommand i_command, std::string &i_deviceString, uint64_t i_address, uint32_t i_length, uint32_t i_flags, ecmdDataBuffer * i_data = NULL, ecmdDataBuffer * i_mask = NULL);
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
     * Operates on the following InstructionCommand types: LONGIN, LONGOUT, SCOMIN, SCOMOUT, READSPMEM, WRITESPMEM, SCOMIN_MASK, BULK_SCOMIN, BULK_SCOMOUT
     */
    uint32_t execute(ecmdDataBuffer & o_data, InstructionStatus & o_status, Handle ** io_handle);
    //@}

    /** @name Transport Functions */
    //@{
    /**
     * @brief Flatten all the object data into a uint8_t buffer
     * @param o_data Byte buffer to write the flattened data to
     * @param i_len Number of bytes in the o_data buffer
     * @post o_data buffer has a flattened version of the FSIInstruction - must be pre-allocated
     * Data format (all in network byte order):
     * First Word:      version
     * Second Word:     command
     * Third Word:      flags
     * Fourth Word:     cfamid
     * Fifth Word:      linkid
     * Sixth Word:      cmaster
     * ======= // 32-bit address
     * Seventh Word:    address
     * Eighth Word:     length
     * Ninth Word:      data size       // only used for LONGIN, SCOMIN, WRITESPMEM, SCOMIN_MASK, BULK_SCOMIN
     * TenthWord:       mask size       // only used for SCOMIN_MASK
     * ======= // 64-bit address
     * Seventh Word:    address64 upper 32 bits
     * Eighth Word:     address64 lower 32 bits
     * Ninth Word:      length
     * Tenth Word:      data size       // only used for SCOMIN, SCOMIN_MASK, BULK_SCOMIN
     * Eleventh Word:   mask size       // only used for SCOMIN_MASK
     * =======
     * Multiple Words:  data            // only used for LONGIN, SCOMIN, WRITESPMEM, SCOMIN_MASK, BULK_SCOMIN
     * Multiple Words:  mask            // only used for SCOMIN_MASK
     * =========== Device String Format (flag & INSTRUCTION_FLAG_DEVSTR)
     * First Word:      version
     * Second Word:     command
     * Third Word:      flags
     * ======= // 64-bit address (flag & INSTRUCTION_FLAG_64BIT_ADDRESS)
     * XXXXX Word:      address64 upper 32 bits
     * XXXXX Word:      address64 lower 32 bits
     * ======= // 32-bit address
     * XXXXX Word:      address
     * XXXXX Word:      length
     * XXXXX Word:      deviceString size
     * XXXXX Word:      data size       // only used for LONGIN, SCOMIN, WRITESPMEM, SCOMIN_MASK, BULK_SCOMIN
     * XXXXX Word:      mask size       // only used for SCOMIN_MASK
     * =======
     * Multiple Words:  deviceString
     * Multiple Words:  data            // only used for LONGIN, SCOMIN, WRITESPMEM, SCOMIN_MASK, BULK_SCOMIN
     * Multiple Words:  mask            // only used for SCOMIN_MASK
     */
    uint32_t flatten(uint8_t * o_data, uint32_t i_len) const;

    /**
     * @brief Unflatten object data from a uint8_t buffer into this FSIInstruction
     * @param i_data Byte buffer to read the flattened data from
     * @param i_len Number of bytes in the i_data buffer
     * @post This FSIInstruction is allocated and initialized with the unflattened version of i_data
     * Data format (all in network byte order):
     * First Word:      version
     * Second Word:     command
     * Third Word:      flags
     * Fourth Word:     cfamid
     * Fifth Word:      linkid
     * Sixth Word:      cmaster
     * ======= // 32-bit address
     * Seventh Word:    address
     * Eighth Word:     length
     * Ninth Word:      data size       // only used for LONGIN, SCOMIN, WRITESPMEM, SCOMIN_MASK, BULK_SCOMIN
     * TenthWord:       mask size       // only used for SCOMIN_MASK
     * ======= // 64-bit address
     * Seventh Word:    address64 upper 32 bits
     * Eighth Word:     address64 lower 32 bits
     * Ninth Word:      length
     * Tenth Word:      data size       // only used for SCOMIN, SCOMIN_MASK, BULK_SCOMIN
     * Eleventh Word:   mask size       // only used for SCOMIN_MASK
     * =======
     * Multiple Words:  data            // only used for LONGIN, SCOMIN, WRITESPMEM, SCOMIN_MASK, BULK_SCOMIN
     * Multiple Words:  mask            // only used for SCOMIN_MASK
     * =========== Device String Format (flag & INSTRUCTION_FLAG_DEVSTR)
     * First Word:      version
     * Second Word:     command
     * Third Word:      flags
     * ======= // 64-bit address (flag & INSTRUCTION_FLAG_64BIT_ADDRESS)
     * XXXXX Word:      address64 upper 32 bits
     * XXXXX Word:      address64 lower 32 bits
     * ======= // 32-bit address
     * XXXXX Word:      address
     * XXXXX Word:      length
     * XXXXX Word:      deviceString size
     * XXXXX Word:      data size       // only used for LONGIN, SCOMIN, WRITESPMEM, SCOMIN_MASK, BULK_SCOMIN
     * XXXXX Word:      mask size       // only used for SCOMIN_MASK
     * =======
     * Multiple Words:  deviceString
     * Multiple Words:  data            // only used for LONGIN, SCOMIN, WRITESPMEM, SCOMIN_MASK, BULK_SCOMIN
     * Multiple Words:  mask            // only used for SCOMIN_MASK
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
type,     SCAN | SCOM | other, linkid(24:31), cfamid(24:31), cmaster(24:31)
bits 0:3  bits 4:7,            bits 8:15,     bits 16:23,    bits 24:31 </pre>
     */
    uint64_t getHash(void) const;

    /**
     * @brief closes Handle handle based upon the command type
     */
    uint32_t closeHandle(Handle ** i_handle);

    std::string getInstructionVars(const InstructionStatus & i_status) const;
    //@}

  protected:
    uint32_t cfamid;
    uint32_t linkid;
    uint32_t cmaster;
    uint32_t address;
    uint64_t address64;
    uint32_t length;
    ecmdDataBuffer data;
    ecmdDataBuffer mask;
    std::string deviceString;

    CFAMType getCFAMType(const uint32_t i_address, const uint32_t i_flags);

    virtual uint32_t scan_open(Handle ** handle, InstructionStatus & o_status) { return -1; }
    virtual uint32_t scom_open(Handle ** handle, InstructionStatus & o_status) { return -1; }
    virtual uint32_t fsi_open(Handle ** handle, InstructionStatus & o_status) { return -1; }

    virtual void scan_ffdc_and_reset(Handle ** handle, InstructionStatus & o_status) { }
    virtual void scom_ffdc_and_reset(Handle ** handle, InstructionStatus & o_status) { }
    virtual void gp_reg_ffdc_and_reset(Handle ** handle, InstructionStatus & o_status) { }
    virtual void mbx_ffdc_and_reset(Handle ** handle, InstructionStatus & o_status) { }
    virtual void sbefifo_ffdc_and_reset(Handle ** handle, InstructionStatus & o_status) { }

    virtual uint32_t scan_close(Handle * handle) { return -1; }
    virtual uint32_t scom_close(Handle * handle) { return -1; }
    virtual uint32_t fsi_close(Handle * handle) { return -1; }


    virtual ssize_t scan_write(Handle * i_handle, InstructionStatus & o_status) { return -1; }
    virtual ssize_t scan_read(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status) { return -1; }
    // scom write double word from data at i_index
    virtual ssize_t scom_write(Handle * i_handle, InstructionStatus & o_status, uint32_t i_index = 0) { return -1; }
    virtual ssize_t scom_write_under_mask(Handle * i_handle, InstructionStatus & o_status) { return -1; }
    // scom write double word to o_data at i_index
    virtual ssize_t scom_read(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status, uint32_t i_index = 0) { return -1; }

    virtual ssize_t scan_get_register(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status) { return -1; }
    virtual ssize_t scom_get_register(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status) { return -1; }
    virtual ssize_t mbx_get_scratch_register(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status) { return -1; }
    virtual ssize_t mbx_get_register(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status) { return -1; }
    virtual ssize_t gp_reg_get(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status) { return -1; }
    virtual ssize_t sbefifo_get_register(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status) { return -1; }
    virtual ssize_t iic_get_register(Handle * i_handle, ecmdDataBufferBase & o_data, InstructionStatus & o_status) { return -1; }

    virtual ssize_t scan_set_register(Handle * i_handle, InstructionStatus & o_status) { return -1; }
    virtual ssize_t scom_set_register(Handle * i_handle, InstructionStatus & o_status) { return -1; }
    virtual ssize_t mbx_set_scratch_register(Handle * i_handle, InstructionStatus & o_status) { return -1; }
    virtual ssize_t mbx_set_register(Handle * i_handle, InstructionStatus & o_status) { return -1; }
    virtual ssize_t gp_reg_set(Handle * i_handle, InstructionStatus & o_status) { return -1; }
    virtual ssize_t sbefifo_set_register(Handle * i_handle, InstructionStatus & o_status) { return -1; }
    virtual ssize_t iic_set_register(Handle * i_handle, InstructionStatus & o_status) { return -1; }
};

#endif // _FSIInstruction_H
