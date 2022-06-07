//IBM_PROLOG_BEGIN_TAG
/* 
 * Copyright 2017 IBM International Business Machines Corp.
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

#ifndef fapi2Interpreter_h
#define fapi2Interpreter_h
/**
 @file fapi2Interpreter.H
 @brief eCMD Fapi2 Extension Command Interpreter function prototypes
*/

//--------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------
#include <inttypes.h>


//--------------------------------------------------------------------
//  Forward References                                                
//--------------------------------------------------------------------
/**
 @brief Process user args and execute command requested
 @param argc Command line arguments
 @param argv Command line arguments
 @retval ECMD_SUCCESS On successful command
 @retval ECMD_INT_UNKNOWN_COMMAND If command is not valid
 @retval nonzero On command failure
 @pre Dll must be loaded - if applicable

 NOTE - should only be called for Fapi extension functions
*/
 
uint32_t fapi2CommandInterpreter(int argc, char* argv[]);



/** @name User Functions */
//@{

/* fapi2MiscUser.C */
uint32_t fapi2GetAttributeUser(int argc, char * argv[]);
uint32_t fapi2SetAttributeUser(int argc, char * argv[]);
uint32_t fapi2DumpAttributeUser(int argc, char * argv[]);


//@}

#endif /* fapi2Interpreter_h */
