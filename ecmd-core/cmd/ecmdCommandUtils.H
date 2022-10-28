//IBM_PROLOG_BEGIN_TAG
/* 
 * Copyright 2003,2016 IBM International Business Machines Corp.
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

#ifndef ecmdCommandUtils_h
#define ecmdCommandUtils_h

/**
 @file ecmdCommandUtils.H
 @brief eCMD Command Utilities
*/

//--------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------
#include <ecmdStructs.H>
#include <ecmdDataBuffer.H>

//--------------------------------------------------------------------
//  Forward References                                                
//--------------------------------------------------------------------


/**
 @brief Compares the two data buffers to see if they contain the same data
 @retval 1 if match, 0 if mismatch
 @param i_data The actual data fetched from the chip
 @param i_expected The user-supplied data to match the fetched data to
 @param o_mismatchBit Return the first bit that mismatched in the data, set to ECMD_UNSET to not calculate when calling, will be set to ECMD_UNSET if wasn't a bit mismatch but was a length difference
 */
uint32_t ecmdCheckExpected (const ecmdDataBuffer & i_data, const ecmdDataBuffer & i_expected, uint32_t & o_mismatchBit);

/**
 @brief Applies the buffer i_newData to the io_data buffer based upon the specified i_modifier for length of i_newData
 @retval ECMD_SUCCESS on success
 @retval non-zero on failure
 @param io_data Data buffer to modify
 @param i_newData Data to insert into io_data based on modifier
 @param i_startbit Startbit of io_data to begin modification
 @param i_modifier Type of data insertion to do 'and', 'or', 'insert'
*/
uint32_t ecmdApplyDataModifier(ecmdDataBuffer & io_data, const ecmdDataBuffer & i_newData, uint32_t i_startbit, std::string i_modifier, ecmdEndianMode_t i_endianMode = ECMD_BIG_ENDIAN);

/* STGC01276442 - MKL 03/04/10 */
/**
 @brief Uses the buffer i_newData to create the buffers o_data and o_mask based upon the specified i_modifier for length of i_newData
 @retval ECMD_SUCCESS on success
 @retval non-zero on failure
 @param o_data Output data buffer
 @param o_mask Output mask buffer
 @param i_newData Data to use to creat o_data and o_mask based on modifier
 @param i_startbit Startbit of o_data to begin creation
 @param i_modifier Type of data insertion to do 'and', 'or', 'insert'
*/
uint32_t ecmdCreateDataMaskModifier(ecmdDataBuffer & o_data, ecmdDataBuffer & o_mask, const ecmdDataBuffer & i_newData, uint32_t i_startBit, std::string i_modifier, ecmdEndianMode_t i_endianMode = ECMD_BIG_ENDIAN);

/**
 @brief Write help text file for command
 @param i_command Command to lookup help text for
 @retval ECMD_SUCCESS if successful
 @retval nonzero if failure
*/
uint32_t ecmdPrintHelp(const char* i_command);


/**
 @brief Returns true if all chars of str are decimal numbers
 @retval true if all chars are decimal
 @retval false if any non-decimal character is detected
 @param str Input string to perform comparison against
*/
bool ecmdIsAllDecimal(const char* str);

/**
 @brief Returns true if all chars of str are hex numbers
 @retval true if all chars are hex
 @retval false if any non-hex character is detected
 @param str Input string to perform comparison against
*/
bool ecmdIsAllHex(const char* str);

/**
 @brief Returns a list of commands that were read from stdin
 @param o_commands
 @retval ECMD_SUCCESS when EOF is reached (list may still contain valid commands
 @retval list size
*/
uint32_t ecmdParseStdinCommands(std::vector< std::string > & o_commands);


/**
 @brief Returns a list of commands that were read from stdin if in shell Mode
 @param o_commands
 @retval ECMD_SUCCESS when EOF is reached (list may still contain valid commands
 @retval list size
*/
uint32_t ecmdParseShellCommands(std::vector< std::string > & o_commands);


/**
 @brief Parses the specific target field list specified by the user and updates the target States
 @param io_argc Pointer to number of elements in io_argv array
 @param io_argv Array of strings passed in from command line
 @param i_targetField Specifies target field-cage,node,slot etc
 @param io_target Target to be updated based on the user input
 @param o_targetFieldType Specifies One versus Many. E.g. Whether one pos was specified or more than 1
 @param o_targetFieldList Update this string with the list of targets specified
 @retval ECMD_SUCCESS or ECMD_INVALID_ARGS
*/
uint32_t ecmdParseTargetFields(int *io_argc, char ** io_argv[], const char *i_targetField, ecmdChipTarget &io_target, uint8_t &o_targetFieldType, std::string &o_targetFieldList);

/**
 @brief Checks if the string is a valid target
 @param i_str String to be checked for decimal characters
 @retval Returns true if all chars of str are decimal numbers
*/
bool isTargetStringValid(std::string i_str);

/**
 @brief Convert the target string into a uint32_t list
 @param i_userArgs Input target String to be parsed
 @param o_targetList Output list to be updated with the targets
*/
void getTargetList(std::string i_userArgs, std::list<uint32_t> &targetList);

/**
 @brief returns the ecmd prompt.. which consists of  the currend working directory  this is used for the ecmd shell
 @retval prompt sting
*/
std::string  getEcmdPrompt();


/**
 @brief returns the ecmd prompt.. which consists of  the currend working directory  this is used for the ecmd shell
 @retval prompt sting
*/
void setupEcmds();

/**
 @brief returns the ecmd prompt.. which consists of  the currend working directory  this is used for the ecmd shell
 @retval prompt sting
*/
std::string  getBestEcmd(std::string i_ecmd);

/**
 @brief Will get the processing unit name from the DLL
 @param i_target ecmdChipTarget containing data to format into string
 @param o_processingUnitName Return processing unit name.
 @retval ECMD_SUCCESS on success
 @retval non-zero on failure
*/
uint32_t ecmdGetProcessingUnit(const ecmdChipTarget & i_target, std::string & o_processingUnitName);

/**
 @brief Will get the processing unit name from the DLL
 @param i_target ecmdChipTarget containing data to format into string
 @param o_pbaUnitName Return processing unit name.
 @retval ECMD_SUCCESS on success
 @retval non-zero on failure
*/
uint32_t ecmdGetPbaUnit(const ecmdChipTarget & i_target, std::string & o_pbaUnitName);

#endif /* ecmdCommandUtils_h */
