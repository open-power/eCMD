#ifndef _ControlInstruction_H
#define _ControlInstruction_H
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
 * @file ControlInstruction.H
 * @brief Provides a class for commands and queries being passed to the fsp server
 *
*/


//--------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------
#include <Instruction.H>
#include <map>
#include <list>

typedef struct {
  bool enabled;
  std::map<uint32_t, std::string> keyMap;
  uint32_t firstKey;
} Authorization;

typedef struct {
  Instruction::InstructionType type;
  Instruction::InstructionCommand command;
  std::string vars;
} FlightRecorderEntry;

/**
 @brief structure for holding variables owned by the server that the ControlInstruction needs access to
*/
typedef struct ServerControls_s {
  bool * global_exit_pointer;
  bool * global_multi_client_pointer;
  Authorization * global_auth_pointer;
  bool * threadKeyValid_pointer;
  uint32_t * threadKey_pointer;
  std::list<FlightRecorderEntry> * global_flight_recorder_pointer;
  std::map<std::string, uint32_t> * global_version_map_pointer;
} ServerControls;

/* ------------------------------------------------------------------ */
/*  This is the Server Info Type Structure Format                     */
/* ------------------------------------------------------------------ */

typedef enum { // SERVER_MACHINE_TYPE
  SERVER_CSP,
  SERVER_BPC,
  SERVER_FSP,
  SERVER_SJM,
  SERVER_PROXY,            ///< Bluegene IDO
  SERVER_SIM,              ///< Sim SISRTX
  SERVER_LOFT,
  SERVER_6682TESTER,       ///< This is the VLSI tester used for wafer test - contact Todd Burdine
  SERVER_SIMDISPATCHER,    ///< Sim Dispatcher
  SERVER_UNDEFINED,        /* This is set on the client side if it was unable to query this info */
  SERVER_ICON,              ///< Bluegene Icon library
  SERVER_FTDI,
  SERVER_GSD2PIB,
  SERVER_D2C,
  SERVER_BMC,
} SERVER_MACHINE_TYPE;

/* Results for the INFO command will be formated as follows in the o_data ecmdDataBuffer */
/*
  version 1 data format
  ********************************
  Word 0 uint32_t version;
  Word 1 SERVER_MACHINE_TYPE type;
  Word 2 uint32_t tms_mask;
  Word 3 uint32_t tck_mask;
  Word 4 uint32_t tdi_mask;
  Word 5 uint32_t tdo_mask;
  Word 6 uint32_t flags;
*/

struct server_type_info {
  SERVER_MACHINE_TYPE type;
  uint32_t tms_mask;
  uint32_t tck_mask;
  uint32_t tdi_mask;
  uint32_t tdo_mask;
  uint32_t flags;
};

/* this class needs to have special server side constructor to get information about the server */

/**
 @brief ControlInstruction class
*/
class ControlInstruction : public Instruction {
  public:
    /** @name ControlInstruction Constructors */
    //@{
    /**
     * @brief Default Constructor
     * @post Instruction() is called, version is set to 0x1, and type is set to CONTROL
     */
    ControlInstruction(void);

    /**
     * @brief Constructor with all variables passed in
     * @post Instruction() is called, command, and flags are set, version is set to 0x1, and type is set to CONTROL, if i_commandToRun is not NULL it is copied to commandToRun
     */
    ControlInstruction(InstructionCommand i_command, uint32_t i_flags, const char * i_commandToRun = NULL);

    /**
     * @brief Server-side constructor with all variables passed in
     * @post Instruction() is called, version is set to 0x1, and type is set to CONTROL, controls is also set
     */
    ControlInstruction(ServerControls * i_controls);

    /**
     * @brief Default Destructor
     */
    ~ControlInstruction(void);
    //@}

    // Member Functions
    /** @name Setup Function */
    //@{
    /**
     * @brief Used as alternate way to set variables normally set on construction
     * @retval ECMD_SUCCESS on success
     * @retval nonzero on failure
     * @post command and flags are set, if i_commandToRun is not NULL it is copied to commandToRun
     */
    uint32_t setup(InstructionCommand i_command, uint32_t i_flags, const char * i_commandToRun = NULL);

    /**
     * @brief Used for authorization settings
     * @retval ECMD_SUCCESS on success
     * @retval nonzero on failure
     * @post command, key, and flags are set, if i_contactInfo is not NULL it is copied to contactInfo
     */
    uint32_t setup(InstructionCommand i_command, uint32_t i_key , uint32_t i_flags, const char * i_contactInfo = NULL);

    /**
     * @brief Used for sim info and sndistepmsg
     * @retval ECMD_SUCCESS on success
     * @retval nonzero on failure
     * @post if command == SNDISTEPMSG, majorIstepNum = i_arg1, minorIstepNum = i_arg2, timeout = i_arg3, and flags are set, sets version to 0x4
     */
    uint32_t setup(InstructionCommand i_command, uint32_t i_arg1 , uint32_t i_arg2, uint32_t i_arg3, uint32_t i_flags);
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
     * Operates on the following InstructionCommand types: INFO, RUN_CMD, AUTH, ADDAUTH, CLEARAUTH, VERSION, FLIGHTRECORDER, EXIT, CHICDOIPL
     */
    uint32_t execute(ecmdDataBuffer & o_data, InstructionStatus & o_status, Handle ** io_handle);
    //@}

    /** @name Transport Functions */
    //@{
    /**
     * @brief Flatten all the object data into a uint8_t buffer
     * @param o_data Byte buffer to write the flattened data to
     * @param i_len Number of bytes in the o_data buffer
     * @post o_data buffer has a flattened version of the ControlInstruction - must be pre-allocated
     * Data format (all in network byte order):
     * First Word:      version
     * Second Word:     command
     * Third Word:      flags
     * Multiple Words:  commandToRun    (Sent with RUN_CMD)
     * Fourth Word:     key             (Sent with AUTH and ADDAUTH)
     * Multiple Words:  contactInfo     (Sent with ADDAUTH)
     */
    uint32_t flatten(uint8_t * o_data, uint32_t i_len) const;

    /**
     * @brief Unflatten object data from a uint8_t buffer into this ControlInstruction
     * @param i_data Byte buffer to read the flattened data from
     * @param i_len Number of bytes in the i_data buffer
     * @post This ControlInstruction is allocated and initialized with the unflattened version of i_data
     * Data format (all in network byte order):
     * First Word:      version
     * Second Word:     command
     * Third Word:      flags
     * Multiple Words:  commandToRun    (Sent with RUN_CMD)
     * Fourth Word:     key             (Sent with AUTH and ADDAUTH)
     * Multiple Words:  contactInfo     (Sent with ADDAUTH)
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

    /**
     * @brief Creates a string with the instruction information in a -debug5.1|C format
     * @retval String with instruction information
     */
    std::string dumpInstructionShort(void) const;
    //@}

    /** @name Server Utility Function */
    //@{
    /**
     * @brief Creates a hash based upon the instruction type
     * retval 0 for INFO, RUN_CMD, VERSION, FLIGHTRECORDER. Normal getHash for all others
     */
    uint64_t getHash(void) const;

    std::string getInstructionVars(const InstructionStatus & i_status) const;
    //@}

    /** @name Client Utility Function */
    //@{
    /**
     * @brief expands data returned in ecmdDataBuffer to a server_type_info structure
     */
    uint32_t populateTypeInfo(server_type_info & o_typeInfo, ecmdDataBuffer & i_data);
    //@}

    /** @name Utility Function */
    //@{
    /**
     * @brief converts the SERVER_TYPE to a string
     */
    static std::string serverTypeToString( SERVER_MACHINE_TYPE i_type );
    //@}

  protected:
    // RUN_CMD, CHICDOIPL
    std::string commandToRun;
    // AUTH, ADDAUTH, CLEARAUTH
    uint32_t key;
    std::string contactInfo;
    //SNDISTEPMSG
    uint16_t majorIstepNum;
    uint16_t minorIstepNum;
    int32_t timeout;

    ServerControls * controls; // Do not flatten or unflatten, Server side variable only
};

#endif // _ControlInstruction_H
