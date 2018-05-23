#ifndef _Instruction_H
#define _Instruction_H
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
 * @file Instruction.H
 * @brief Provides a base class for instructions being passed to the fsp server
 *
 * Instruction base class can be used for unknown instrutions
 * Can send message back that current server does not support the instruction
*/


//--------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------
#include <string>
#include <ostream>
#include <ecmdDataBuffer.H>
#include <InstructionStatus.H>
#include <CronusRc.H>
#include <InstructionFlag.H>

#if defined(TESTING) && !defined(TESTING_NO_DEBUG)
  #define TEST_PRINT(format, args...) printf (format , ##args)
#else
  #define TEST_PRINT(format, args...)
#endif


/**
 @brief Header information for each message in communication
*/
typedef struct DataTransferInfo
{
    uint32_t key;
    uint32_t type;
    uint32_t size;
} DataTransferInfo_t;

class Handle;

/**
 @brief All the types of result information are defined here
*/
typedef enum
{
    ECMD_DBUF,
    INSTRUCTION_STATUS,
} ResultType;


/**
 @brief Instruction base class
*/
class Instruction{
  public:
    /**
     @brief All the types of instructions are defined here
     */
    typedef enum
    {
        NOINSTRUCTION,
        FSI,
        JTAG,
        MULTIPLEJTAG,
        PSI,
        GPIO,
        I2C,
        VPD,
        DMA,
        CONTROL, // commands to the server to certain things: end session, get info, run command,
        DMAEXER,
        POWR,
        FSISTREAM,
        SBEFIFO,
        GSD2PIB,
        FSIMEMPROC,
        PNOR,
        D2C,
    } InstructionType;

    /**
     @brief All the types of commands for instructions are defined here
     */
    typedef enum
    {
        NOCOMMAND,
        SENDCMD,
        READ,
        WRITE,

        LONGIN,
        LONGOUT,
        READ_WRITE,

        SHIFTOUT,

        DMAIN,
        DMAOUT,
        MISC,

        TOTAP,
        READVPD,
        READSPMEM,
        WRITESPMEM,

        SCOMIN,
        SCOMOUT,
        WRITEVPD,

        PSI_RESET,
        PSI_LINK_CALIB,
        PSI_EI_REG_READ,
        PSI_EI_REG_WRITE,
        PSI_VERIFY,
        PSI_SCOM_READ,
        PSI_SCOM_WRITE,
        PSI_READ,
        PSI_WRITE,
        PSI_INIT,
        PSI_LINK_ENABLE,
        PSI_SET_SPEED,
        PSI_LINK_VERIFY,

        I2CWRITE,
        I2CREAD,

        GPIO_CONFIGPIN,
        GPIO_READPIN,
        GPIO_READPINS,
        GPIO_READLATCH,
        GPIO_WRITELATCH,
        GPIO_WRITELATCHES,
        GPIO_READCONFIG,
        GPIO_WRITECONFIG,
        GPIO_WRITECNFGSET,
        GPIO_WRITECNFGCLR,

        DMAEXER_START,
        DMAEXER_REPORT,
        DMAEXER_STOP,

        INFO,
        RUN_CMD,
        MULTI_ENABLE,
        AUTH,
        ADDAUTH,
        CLEARAUTH,
        VERSION,
        FLIGHTRECORDER,
        EXIT,

        SCOMIN_MASK,
        MASK_PERSISTENT,
        SET_PERSISTENT,
        GET_PERSISTENT,

        READKEYWORD,
        WRITEKEYWORD,
        FRUSTATUS,
        CHICDOIPL,

        ENABLE_MEM_VOLTAGES,
        DISABLE_MEM_VOLTAGES,

        SNDISTEPMSG,
        MBXTRACEENABLE,
        MBXTRACEDISABLE,
        MBXTRACEREAD,

        PUTMEMPBA,
        GETMEMPBA,

        BULK_SCOMOUT,
        BULK_SCOMIN,

        STREAM_SETUP,
        STREAM_FINISH,

        FSPDMAIN,
        FSPDMAOUT,

        SUBMIT,
        REQUEST_RESET,

        SEND_TAPI_CMD,

        PUTMEMPROC,
        GETMEMPROC,

        PNORGETLIST,
        PNORGET,
        PNORPUT,

        QUERYSP,

        ADJUST_PROC_VOLTAGES,

        I2CRESETLIGHT,
        I2CRESETFULL,

        PSI_CMU_REG_READ,
        PSI_CMU_REG_WRITE,
    } InstructionCommand;

    /** @name Instruction Constructor */
    //@{
    /**
     * @brief Default Constructor
     * @post version is set to 0x0, command is set to NOCOMMAND, and type is set to NOINSTRUCTION
     */
    Instruction(void);

    /**
     * @brief Default Destructor
     */
    virtual ~Instruction(void) { };
    //@}

    // Member Functions
    /** @name Execution Function */
    //@{
    /**
     * @brief Called by the fsp server or sim transfer class
     * @param o_data ecmdDataBuffer with output data
     * @param o_status InstructionStatus with error information
     * @param io_handle Handle * pointer for handle reuse between instructions
     * @retval ECMD_SUCCESS on success
     * @retval nonzero on failure
     * @post Sets error information in o_status because this is the default class for unknown instructions
     */
    virtual uint32_t execute(ecmdDataBuffer & o_data, InstructionStatus & o_status, Handle ** io_handle);
    //@}

    /** @name decrementVersion Function */
    //@{
    /**
     * @brief Called by the Client side to try using a previous version of the function
     * @retval ECMD_SUCCESS on success
     * @retval nonzero on failure
     * @post version will be decremented to a value of one less than the previous value. A value of one is the lowest value for version.
     */
    virtual uint32_t decrementVersion(void);

    /** @name Transport Functions */
    //@{
    /**
     * @brief Flatten all the object data into a uint8_t buffer
     * @param o_data Byte buffer to write the flattened data to
     * @param i_len Number of bytes in the o_data buffer
     * @post o_data buffer has a flattened version of the Instruction - must be pre-allocated
     * Data format (all in network byte order):
     * First Word:      version
     * Second Word:     command
     * Third Word:      flags
     */
    virtual uint32_t flatten(uint8_t * o_data, uint32_t i_len) const;

    /**
     * @brief Unflatten object data from a uint8_t buffer into this Instruction
     * @param i_data Byte buffer to read the flattened data from
     * @param i_len Number of bytes in the i_data buffer
     * @post This Instruction is allocated and initialized with the unflattened version of i_data
     * Data format (all in network byte order):
     * First Word:      version
     * Second Word:     command
     * Third Word:      flags
     */
    virtual uint32_t unflatten(const uint8_t * i_data, uint32_t i_len);

    /**
     * @brief Return number of bytes needed for a buffer to flatten the object
     * @retval Number of bytes needed
     */
    virtual uint32_t flattenSize(void) const;

  protected:
    /**
     * @brief Append fsp info to end of data buffer
     * @param o_data Byte buffer to write the flattened data to
     * @param i_len Number of bytes in the o_data buffer
     * @post end of o_data buffer will contain cage, node, and slot data
     */
    void flattenFspInfo(uint8_t * o_data, uint32_t i_len) const;

    /**
     * @brief Extract fsp info from end of data buffer
     * @param i_data Byte buffer to read the flattened data from
     * @param i_len Number of bytes in the i_data buffer
     * @post cage, node, and slot will be set
     */
    void unflattenFspInfo(const uint8_t * i_data, uint32_t i_len);
    //@}

  public:
    /** @name Debug Function */
    //@{
    /**
     * @brief Creates a string with the instruction information in a -debug5.f format
     * @retval String with instruction information
     */
    virtual std::string dumpInstruction(void) const;
    //@}

    /** @name Server Utility Function */
    //@{
    /**
     * @brief Creates a hash based upon the instruction type
     * <pre>
type
bits 0:3 </pre>
     */
    virtual uint64_t getHash(void) const;

    /**
     * @brief closes Handle handle
     */
    virtual uint32_t closeHandle(Handle ** i_handle);

    /**
     * @brief returns a short string with the variables used by the instruction, used by server flight recorder
     * @param i_status InstructionStatus with error information
     */
    virtual std::string getInstructionVars(const InstructionStatus & i_status) const;

    /**
     * @brief generates a hexLeftStr <= 128 bits
     */
    int genWords(const ecmdDataBuffer &data, std::string &words) const;
    //@}

    /** @name Access Functions */
    //@{
    /**
     * @brief returns the instruction type
     */
    InstructionType getType(void) const;

    /**
     * @brief returns the instruction command
     */
    InstructionCommand getCommand(void) const;

    /**
     * @brief returns the instruction flags
     */
    uint32_t getFlags(void) const;

    /**
     * @brief returns the instruction Version
     */
    uint32_t getVersion(void) const;
    //@}

    /**
     * @brief Calls the dumpInstruction method and outputs to the ostream
     */
    friend std::ostream& operator<<( std::ostream&, const Instruction& );

  protected:
    uint32_t version;
    InstructionCommand command;
    InstructionType type;
    uint32_t flags;

    uint32_t error; // used to capture errors that would prevent correct operation later on
                    // set flag if error during unflatten
};

uint32_t devicestring_genhash(const std::string & i_string, uint32_t & o_hash);

/**
 @brief function to create string from instruction type
*/
std::string InstructionTypeToString(Instruction::InstructionType i_type);

/**
 @brief function to create string from type
*/
std::string InstructionCommandToString(Instruction::InstructionCommand i_command);


#endif //_Instruction_H
