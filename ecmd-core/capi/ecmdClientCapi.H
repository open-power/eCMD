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

#ifndef ecmdClientCapi_H 
#define ecmdClientCapi_H

/**
 * @file ecmdClientCapi.H
 * @brief eCMD C/C++ Client Interface
*/

//--------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------
#include <ecmdDefines.H>
#include <ecmdReturnCodes.H>
#include <ecmdStructs.H>
#include <ecmdDataBuffer.H>

//--------------------------------------------------------------------
//  Forward References                                                
//--------------------------------------------------------------------

/* Functions in here are defined as extern C for the following reasons:
 1)  Keeps Function names small by preventing C++ "mangling"
 2)  Allows (C-based) perl interpreter to access these functions

*/
#ifndef DOCUMENTATION
extern "C" {
#endif

/** @name Load/Unload Functions */
//@{
#if !defined(ECMD_PERLAPI) && !defined(ECMD_PYAPI)
/**
 @brief Load the eCMD DLL
 @param i_dllName Specify the full path and name of the dll to load, 
 @retval ECMD_SUCCESS if successful load
 @retval ECMD_INVALID_DLL_VERSION if Dll version loaded doesn't match client version
 @retval ECMD_INVALID_DLL_FILENAME if dllName and ECMD_DLL_FILE are not specified
 @retval ECMD_DLL_LOAD_FAILURE if failure occurs on call to dlopen
 @retval nonzero if unsuccessful
 @post eCMD DLL is loaded into memory and initialized
 @see unloadDll

       - This function loads the DLL based on dllName if specified, otherwise the env var ECMD_DLL_FILE is used
       - Name limit of 255 characters.
       - Errors in loading are printed to STDERR.
*/
uint32_t ecmdLoadDll(std::string i_dllName);
#endif

#ifndef ECMD_PERLAPI
/**
 @brief Unload the eCMD DLL
 @retval ECMD_SUCCESS if successful unload
 @retval ECMD_DLL_LOAD_FAILURE if failure occurs on call to dlclose
 @retval nonzero if failure on dll's unload
 @see loadDll

 - Errors in unloading are printed to STDERR
*/
uint32_t ecmdUnloadDll(); 
#endif

#if !defined(ECMD_PERLAPI) && !defined(ECMD_PYAPI)
/**
 @brief Pass any unknown command line paramaters to the DLL for processing (ex. -p#, Cronus -debug)
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @param i_argc Passed from Command line Arguments
 @param i_argv Passed from Command line Arguments
 @pre loadDll must have been called
 @post Global options (ex. -debug, -p#, -c#) will be removed from arg list
 @see loadDll

 - argc/argv get passed to the eCMD DLL.
 - Global options such as -debug flags and -p#, -c# will be parsed out.<br>
 NOTE : This function does not affect ring caching
*/
uint32_t ecmdCommandArgs(int* i_argc, char** i_argv[]); 
#endif

/**
 @brief Set the eCMD environment variables
 @param i_args The args that determine the ecmd release, plugin, etc..
 @retval ECMD_SUCCESS if successful
 @retval nonzero on failure

 Even though you have to pass in the eCMD Release, you are not able to actually change it (because eCMD is already loaded)<br>
*/
uint32_t ecmdSetup(const char* i_args);

#ifndef DOCUMENTATION
/**
 @brief Allow the plugin to recover from certain ecmdLoadDll errors that aren't fatal, as determined by the plugin
 @param i_commandLine The command line for the command being run
 @param io_rc The rc from the ecmdLoadDll call that can be modified by the plugin if it determines necessary
 @retval ECMD_SUCCESS if successful load
 @retval nonzero if unsuccessful
*/
void ecmdLoadDllRecovery(std::string i_commandLine, uint32_t & io_rc);
#endif
//@}

/** @name Looper Functions */
//@{
/**
 @brief A wrapper function over ecmdConfigLooperInit or ecmdConfigLooperNext
 @param io_target Initial ecmdChipTarget that may contain information used in building the struct to loop over
 @param i_looptype Specify type of all, all chips in system or all chips selected by user
 @param io_state Used internally by ConfigLooper to keep track of state, unique instance must be passed into each loop and must be passed to ecmdConfigLooperNext
 @param i_mode Specify if an ecmdConfigLooperInit or ecmdExistLooperInit should be done.  If not set, the global var is used
 @retval ECMD_SUCCESS if initialization succeeded, error code if otherwise
 @see ecmdLooperNext

 This function is designed to allow the user to make dynamic choices between a ConfigLoop or ExistLoop.  By using the i_mode default value of ECMD_DYNAMIC_LOOP, the user can change the type of loop via the commandline option -exist.

 The other values of i_mode (ECMD_CONFIG_LOOP and ECMD_EXIST_LOOP) are the exact same as calling ecmdConfigLooperInit or ecmdExistLooperInit directly.

 TARGET DEPTH  : cage, node, slot, pos, chipUnit, thread<br>
 TARGET STATES : Must Be Initialized<br>
 */
uint32_t ecmdLooperInit (ecmdChipTarget & io_target, ecmdLoopType_t i_looptype, ecmdLooperData& io_state, ecmdLoopMode_t i_mode = ECMD_DYNAMIC_LOOP);

/**
 @brief Initializes data structures and code to loop over configured and selected elements of the system
 @param io_target Initial ecmdChipTarget that may contain information used in building the struct to loop over
 @param i_looptype Specify type of all, all chips in system or all chips selected by user
 @param io_state Used internally by ConfigLooper to keep track of state, unique instance must be passed into each loop and must be passed to ecmdConfigLooperNext
 @retval ECMD_SUCCESS if initialization succeeded, error code if otherwise
 @see ecmdConfigLooperNext

 TARGET DEPTH  : cage, node, slot, pos, chipUnit, thread<br>
 TARGET STATES : Must Be Initialized<br>
 */
uint32_t ecmdConfigLooperInit (ecmdChipTarget & io_target, ecmdLoopType_t i_looptype, ecmdLooperData& io_state);

/**
 @brief Initializes data structures and code to loop over existing and selected elements of the system
 @param io_target Initial ecmdChipTarget that may contain information used in building the struct to loop over
 @param i_looptype Specify type of all, all chips in system or all chips selected by user
 @param io_state Used internally by ExistLooper to keep track of state, unique instance must be passed into each loop and must be passed to ecmdConfigLooperNext
 @retval ECMD_SUCCESS if initialization succeeded, error code if otherwise
 @see ecmdExistLooperNext

 TARGET DEPTH  : cage, node, slot, pos, chipUnit, thread<br>
 TARGET STATES : Must Be Initialized<br>
 */
uint32_t ecmdExistLooperInit(ecmdChipTarget & io_target, ecmdLoopType_t i_looptype, ecmdLooperData& io_state);

/**
 @brief A wrapper function over ecmdConfigLooperNext or ecmdConfigLooperNext
 @param io_target ecmdChipTarget that contains info about next target to process
 @param io_state Used internally to keep track of state, must be passed from output of ecmdConfigLooperInit
 @retval 1 if io_target is valid, 0 if it is not
 @param i_mode Specify if an ecmdConfigLooperNext or ecmdExistLooperNext should be done.  If not set, the global var is used
 @see ecmdLooperInit

 This function is designed to allow the user to make dynamic choices between a ConfigLoop or ExistLoop.  By using the i_mode default value of ECMD_DYNAMIC_LOOP, the user can change the type of loop via the commandline option -exist.

 The other values of i_mode (ECMD_CONFIG_LOOP and ECMD_EXIST_LOOP) are the exact same as calling ecmdConfigLooperNext or ecmdExistLooperNext directly.


 TARGET DEPTH  : cage, node, slot, pos, chipUnit, thread<br>
 TARGET STATES : Must be Initialized (from ecmdConfigLooperInit)<br>
 */
uint32_t ecmdLooperNext (ecmdChipTarget & io_target, ecmdLooperData& io_state, ecmdLoopMode_t i_mode = ECMD_DYNAMIC_LOOP);

/**
 @brief Loops over configured and selected elements of the system, updating target to point to them
 @param io_target ecmdChipTarget that contains info about next target to process
 @param io_state Used internally to keep track of state, must be passed from output of ecmdConfigLooperInit
 @retval 1 if io_target is valid, 0 if it is not
 @see ecmdConfigLooperInit

 TARGET DEPTH  : cage, node, slot, pos, chipUnit, thread<br>
 TARGET STATES : Must be Initialized (from ecmdConfigLooperInit)<br>
 */
uint32_t ecmdConfigLooperNext (ecmdChipTarget & io_target, ecmdLooperData& io_state);

/**
 @brief Loops over configured and selected elements of the system, updating target to point to them
 @param io_target ecmdChipTarget that contains info about next target to process
 @param io_state Used internally to keep track of state, must be passed from output of ecmdExistLooperInit
 @retval 1 if io_target is valid, 0 if it is not
 @see ecmdExistLooperInit

 TARGET DEPTH  : cage, node, slot, pos, chipUnit, thread<br>
 TARGET STATES : Must be Initialized (from ecmdExistLooperInit)<br>
 */
uint32_t ecmdExistLooperNext(ecmdChipTarget & io_target, ecmdLooperData& io_state);

//@}


/** @name Query Functions */
//@{

/**
 @brief Query information about the Dll that is loaded
 @param o_dllInfo Return data with data from the current dll loaded
 @retval ECMD_SUCCESS if successful
 @retval nonzero on failure

 This interface allows you to query what particular instance of the DLL is loaded (i.e Cronus/IP/Z),
  along with additional information.<br>
 NOTE : This function does not affect ring caching
*/

uint32_t ecmdQueryDllInfo(ecmdDllInfo & o_dllInfo);

/**
 @brief Query to see if plugin is greater or equal to release specified
 @param version eCMD Release (ex "5.1", "6.3")

 @retval true If the plugin release is >= version specified
 @retval false If the plugin release is < version specified

 The eCMD C-api won't allow your code to run if the major numbers mismatch, but at times you may want to use a
  new function that was released in a minor release.  This api let's you see if the plugin is at least
  or greater then the minor number where the new function was made available.

 The eCMD Perl-api allows a script to support multiple major and minor versions of eCMD, but at times api's you use may change or new interfaces are added.  This interface allows you to query at runtime which version of eCMD you are running against and make calls appropriately.
 

 So if new function X was dropped in eCMD release v6.2 then if you include the following check in your code
  you can neatly handle if a user is trying to run a plugin that is 6.1 or less:

  if (!ecmdQueryVersionGreater("6.2")) {<br>
    ecmdOutputWarning("Plugin doesn't support function X , skipping the new stuff\n");<br>
  }<br>
*/
bool ecmdQueryVersionGreater(const char* version);

/**
 @brief Query configured target information from the DLL
 @param i_target Struct that contains partial information to limit query results
 @param o_queryData Return data from query
 @param i_detail Specify the level of detail that should be returned with the query
 @retval ECMD_SUCCESS if successful
 @retval nonzero on failure

 The Valid bits of the target are used to refine the query

 The target paramater should be filled in with as much data as you know to limit the query, (including the chipType).

 When a field state is set to ECMD_TARGET_FIELD_WILDCARD the query function will iterate on all possible values for that
 entry and return the relevant data.

 When a field state is set to ECMD_TARGET_FIELD_UNUSED the query function will stop iterating at that level and below

 If nothing is found that matches your query an empty list will be returned with an ECMD_SUCCESS return code.


 Ex: to query what positions of the Nova chip are on cage 1, node 2:<br>
   cage = 1, node = 2, pos = 'wildcard', chipType = 'Nova', core = 'wildcard', thread = 'wildcard'<br>
 Ex: to query what positions of the Nova chip are in the entire system:<br>
   cage = 'wildcard', node = 'wildcard', pos = 'wildcard', chipType = 'Nova', core = 'wildcard', thread = 'wildcard'<br>
 Ex: to query all the chips on cage 3, node 0:<br>
   cage = 3, node = 0, pos = 'wildcard', chipType = 'wildcard', core = 'wildcard', thread = 'wildcard'<br>
 Ex: to query all the chips in the entire system:<br>
   cage = 'wildcard', node = 'wildcard', pos = 'wildcard', chipType = 'wildcard', core = 'wildcard', thread = 'wildcard'<br>
 Ex: to query the total nodes in a system:<br>
   cage = 'wildcard', node = 'wildcard', pos = 'ignore', chipType = 'unused', core = 'unused', thread = 'unused'<br>
 NOTE : This function does not affect ring caching<br>

 TARGET DEPTH  : cage, node, slot, pos, chipUnit, thread<br>
 TARGET STATES : Must be Initialized<br>
*/
   
uint32_t ecmdQueryConfig(const ecmdChipTarget & i_target, ecmdQueryData & o_queryData, ecmdQueryDetail_t i_detail = ECMD_QUERY_DETAIL_HIGH);

/**
 @brief Query existing target information from the DLL
 @param i_target Struct that contains partial information to limit query results
 @param o_queryData Return data from query
 @param i_detail Specify the level of detail that should be returned with the query
 @retval ECMD_SUCCESS if successful
 @retval nonzero on failure

 See the documentation on ecmdQueryConfig for examples on how to set target states.<br>
 NOTE: If a target is existing, but not configured, it cannot be operated on by most functions
*/
   
uint32_t ecmdQueryExist(const ecmdChipTarget & i_target, ecmdQueryData & o_queryData, ecmdQueryDetail_t i_detail = ECMD_QUERY_DETAIL_HIGH);

/**
 @brief Query User Selected Targeting information from the DLL, i.e (-p#,-c#,-t#) for all configured chips
 @param io_target Struct that contains partial information to limit query results - chipType is unused
 @param o_queryData Return data from query
 @param i_looptype (Optional) Used by config looper to specify different query modes
 @retval ECMD_SUCCESS if successful
 @retval nonzero on failure

 This function acts just like ecmdQueryConfig except it operates on what targets were selected by the
  user args -n#, -p#, -c#, -t#

 Use of this function is the same as ecmdQueryConfig<br>
 When -talive is specified all threads configured will be returned in o_queryData and io_target.threadState
   will be set to ECMD_TARGET_THREAD_ALIVE.<br>
 NOTE : This function does not affect ring caching<br>

 TARGET DEPTH  : cage, node, slot, pos, chipUnit, thread<br>
 TARGET STATES : Must be Initialized<br>
*/
uint32_t ecmdQueryConfigSelected(ecmdChipTarget & io_target, ecmdQueryData & o_queryData, ecmdLoopType_t i_looptype = ECMD_SELECTED_TARGETS_LOOP);

/**
 @brief Query User Selected Targeting information from the DLL, i.e (-p#,-c#,-t#) for all existing chips
 @param io_target Struct that contains partial information to limit query results - chipType is unused
 @param o_queryData Return data from query
 @param i_looptype (Optional) Used by config looper to specify different query modes
 @retval ECMD_SUCCESS if successful
 @retval nonzero on failure

 This function acts just like ecmdQueryConfig except it operates on what targets were selected by the
  user args -n#, -p#, -c#, -t#

 Use of this function is the same as ecmdQueryConfig<br>
 When -talive is specified all threads configured will be returned in o_queryData and io_target.threadState
   will be set to ECMD_TARGET_THREAD_ALIVE.<br>
 NOTE : This function does not affect ring caching<br>

 TARGET DEPTH  : cage, node, slot, pos, chipUnit, thread<br>
 TARGET STATES : Must be Initialized<br>
*/
uint32_t ecmdQueryExistSelected(ecmdChipTarget & io_target, ecmdQueryData & o_queryData, ecmdLoopType_t i_looptype = ECMD_SELECTED_TARGETS_LOOP);

/**
 @brief Query related targets
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_relatedType String (format of chip[.unit]) that specifies the desired relationship of the output targets to the input
 @param o_relatedTargets List of targets that fulfill the relatedType relationship to the input target
 @param i_mode Select existing or configured targets through the ecmdLoopMode_t
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval nonzero on failure

 NOTE : This function does not affect ring caching<br>
 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>
*/
uint32_t ecmdRelatedTargets(const ecmdChipTarget & i_target, const std::string i_relatedType, std::list<ecmdChipTarget> & o_relatedTargets, const ecmdLoopMode_t i_mode = ECMD_DYNAMIC_LOOP);

#ifndef ECMD_REMOVE_RING_FUNCTIONS
/**
 @brief Query Ring information from the DLL
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param o_queryData Return list from query
 @param i_ringName if != NULL used to refine query to a single ring
 @param i_detail Specify the level of detail that should be returned with the query
 @retval ECMD_INVALID_RING if i_ringName is not valid for target
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval nonzero on failure

 NOTE : This function does not affect ring caching<br>
 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused<br>
*/
uint32_t ecmdQueryRing(const ecmdChipTarget & i_target, std::list<ecmdRingData> & o_queryData, const char * i_ringName = NULL, ecmdQueryDetail_t i_detail = ECMD_QUERY_DETAIL_HIGH);

/**
 @brief Query Ring Inversion Mask from the DLL
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_ringName used to refine query to a single ring
 @param o_inversionMask inversion mask data for the ring if it exists
 @retval ECMD_INVALID_RING if i_ringName is not valid for target
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval nonzero on failure

 NOTE : This function does not affect ring caching<br>
 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>
*/
uint32_t ecmdQueryRingInversionMask(const ecmdChipTarget & i_target, const std::string i_ringName, ecmdDataBuffer & o_inversionMask);
/**
 @brief Query Ring Ignore Mask from the DLL
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_ringName used to refine query to a single ring
 @param o_ignoreMask ignore mask data for the ring if it exists
 @retval ECMD_INVALID_RING if i_ringName is not valid for target
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval nonzero on failure

 NOTE : This function does not affect ring caching<br>
 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>
*/
uint32_t ecmdQueryRingIgnoreMask(const ecmdChipTarget & i_target, const std::string i_ringName, ecmdDataBuffer & o_ignoreMask);
#endif // ECMD_REMOVE_RING_FUNCTIONS

#ifndef ECMD_REMOVE_LATCH_FUNCTIONS
/**
 @brief Query Latch information from the DLL
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param o_queryData Return list from query
 @param i_ringName if != NULL used to refine query to a single ring
 @param i_latchName if != NULL used to refine query to a single latch
 @param i_mode LatchName search mode (full or partial names)
 @param i_detail Specify the level of detail that should be returned with the query
 @retval ECMD_INVALID_RING if i_ringName is not valid for target
 @retval ECMD_INVALID_LATCHNAME if latchname not found in scandef
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval nonzero on failure

 NOTE : i_latchName or i_ringName MUST be used, they can't both be NULL<br>

 NOTE : This function does not affect ring caching<br>
 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused<br>
*/
uint32_t ecmdQueryLatch(const ecmdChipTarget & i_target, std::list<ecmdLatchData> & o_queryData, ecmdLatchMode_t i_mode, const char * i_latchName, const char * i_ringName = NULL, ecmdQueryDetail_t i_detail = ECMD_QUERY_DETAIL_HIGH);

/**
 @brief Query Latch information from the DLL
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param o_queryData Return structure from query
 @param i_ringName if != NULL used to refine query to a single ring
 @param i_latchName if != NULL used to refine query to a single latch
 @param i_mode LatchName search mode (full or partial names)
 @param i_detail Specify the level of detail that should be returned with the query
 @retval ECMD_INVALID_RING if i_ringName is not valid for target
 @retval ECMD_INVALID_LATCHNAME if latchname not found in scandef
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval nonzero on failure

 NOTE : i_latchName or i_ringName MUST be used, they can't both be NULL<br>

 NOTE : This function does not affect ring caching<br>
 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused<br>
*/
uint32_t ecmdQueryLatchInfo(const ecmdChipTarget & i_target, ecmdLatchQueryData & o_queryData, ecmdLatchMode_t i_mode, const char * i_latchName, const char * i_ringName = NULL, ecmdQueryDetail_t i_detail = ECMD_QUERY_DETAIL_HIGH);
#endif // ECMD_REMOVE_LATCH_FUNCTIONS

#ifndef ECMD_REMOVE_ARRAY_FUNCTIONS
/**
 @brief Query Array information from the DLL
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param o_queryData Return list from query
 @param i_arrayName if != NULL used to refine query to a single array
 @param i_detail Specify the level of detail that should be returned with the query
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_ARRAY if i_arrayName is not valid for target
 @retval ECMD_SUCCESS if successful
 @retval nonzero on failure

 NOTE : This function does not affect ring caching<br>
 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused<br>
*/
uint32_t ecmdQueryArray(const ecmdChipTarget & i_target, std::list<ecmdArrayData> & o_queryData, const char * i_arrayName = NULL, ecmdQueryDetail_t i_detail = ECMD_QUERY_DETAIL_HIGH);
#endif // ECMD_REMOVE_ARRAY_FUNCTIONS

#ifndef ECMD_REMOVE_SPY_FUNCTIONS
/**
 @brief Query Spy information from the DLL
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param o_queryData Return data from query
 @param i_spyName if != NULL used to refine query to a single spy
 @param i_detail Specify the level of detail that should be returned with the query
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval ECMD_INVALID_SPY if spy name is not valid for target
 @retval nonzero on failure

 NOTE : This function does not affect ring caching<br>
 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused<br>
*/
uint32_t ecmdQuerySpy(const ecmdChipTarget & i_target, std::list<ecmdSpyData> & o_queryData, const char * i_spyName = NULL, ecmdQueryDetail_t i_detail = ECMD_QUERY_DETAIL_HIGH);

/**
 @brief Determine all rings associated with spy - including any rings from associated parity checkers
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_spyName Name of spy to get associated ring names for 
 @param o_ringNames Returned list of rings associated with spy 
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval ECMD_INVALID_SPY if spy name is not valid for target
 @retval nonzero on failure

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused<br>
*/
uint32_t ecmdQuerySpyRings(const ecmdChipTarget & i_target, const char * i_spyName, std::list<std::string> & o_ringNames);
#endif // ECMD_REMOVE_SPY_FUNCTIONS

#ifndef ECMD_REMOVE_SCOM_FUNCTIONS
/** @name Query Functions  */
//@{
/**
 @brief Query Scom information from the DLL
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param o_queryData Return data from query
 @param i_address if != 0xFFFFFFFF used to refine query to a single scom
 @param i_detail Specify the level of detail that should be returned with the query
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval nonzero on failure

 NOTE : This function does not affect ring caching<br>
 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused<br>
*/
uint32_t ecmdQueryScom(const ecmdChipTarget & i_target, std::list<ecmdScomData> & o_queryData, uint64_t i_address = 0xFFFFFFFFFFFFFFFFull, ecmdQueryDetail_t i_detail = ECMD_QUERY_DETAIL_HIGH);

//@}
#endif // ECMD_REMOVE_SCOM_FUNCTIONS

#ifndef ECMD_REMOVE_TRACEARRAY_FUNCTIONS
/** @name Query Functions  */
//@{
/**
 @brief Query Trace Array information from the DLL
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param o_queryData Return data from query
 @param i_traceArrayName if != NULL used to refine query to a single trace array
 @param i_detail Specify the level of detail that should be returned with the query
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval nonzero on failure

 NOTE : This function does not affect ring caching<br>
 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused<br>
*/
uint32_t ecmdQueryTraceArray(const ecmdChipTarget & i_target, std::list<ecmdTraceArrayData> & o_queryData, const char * i_traceArrayName = NULL, ecmdQueryDetail_t i_detail = ECMD_QUERY_DETAIL_HIGH);
//@}
#endif // ECMD_REMOVE_TRACEARRAY_FUNCTIONS

/** @name Query Functions  */
//@{
/**
 @brief Query the location of a specific file type for the selected target
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_fileType Enum that specifies which type of file you are looking for scandef/spydef/arraydef
 @param o_fileLocations Return list of locations with full path and filename to location.
 @param io_version Specify version number of file; set to version number by plugin
 @retval ECMD_SUCCESS if successful
 @retval ECMD_UNKNOWN_FILE if unable to find requested file
 @retval nonzero if unsuccessful

 NOTE : This function does not affect ring caching<br>
 TARGET DEPTH  : cage, node, slot, pos (as applicable based on i_fileType)<br>
 TARGET STATES : Unused<br>
*/
uint32_t ecmdQueryFileLocation(const ecmdChipTarget & i_target, ecmdFileType_t i_fileType, std::list<ecmdFileLocation> & o_fileLocations, std::string & io_version);

/**
 @brief Query if a particular target is configured in the system
 @param i_target Target to query in system configuration
 @param i_queryData If specified this data will be used, otherwise a call to ecmdQueryConfig will be made
 @retval true if Target is configured in system
 @retval false if Target is not configured in system

 NOTE : This function calls ecmdQueryConfig and searchs for the specified target<br>
 NOTE : The target State fields must be filled in as either VALID or UNUSED<br>
 TARGET DEPTH  : cage, node, slot, pos, chipUnit, thread dependent  on what should be queried<br>
 TARGET STATES : Must be Initialized. Note: Query depth ends on the first state not set to ECMD_TARGET_FIELD_VALID <br>
 *For example if slotState is set to ECMD_TARGET_FIELD_UNUSED, but cageState and nodeState are set to ECMD_TARGET_FIELD_VALID
 * then the function checks whether the specified node is configured independent on the other state settings.
*/
bool ecmdQueryTargetConfigured(const ecmdChipTarget & i_target, const ecmdQueryData * i_queryData = NULL);

/**
 @brief Query if a particular target exists in the system
 @param i_target Target to query in system configuration
 @param i_queryData If specified this data will be used, otherwise a call to ecmdQueryExist will be made
 @retval true if Target is configured in system
 @retval false if Target is not configured in system

 NOTE : This function calls ecmdQueryExist and searchs for the specified target<br>
 NOTE : The target State fields must be filled in as either VALID or UNUSED<br>
 TARGET DEPTH  : cage, node, slot, pos, chipUnit, thread dependent  on what should be queried<br>
 TARGET STATES : Must be Initialized. Note: Check depth ends on the first state not set to ECMD_TARGET_FIELD_VALID <br>
 *For example if slotState is set to ECMD_TARGET_FIELD_UNUSED, but cageState and nodeState are set to ECMD_TARGET_FIELD_VALID
 * then the function checks whether the specified node exists independent on the other state settings.
*/
bool ecmdQueryTargetExist(const ecmdChipTarget & i_target, const ecmdQueryData * i_queryData = NULL);

/**
 @brief Query what connections a particular target has to other targets in the system
 @param i_target Target to query connections of
 @param i_connectionType The connection, or bus, to find connected targets on.  "ALL" will return all connection types.
 @param o_connections A list containing all the connections found.
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful

 <pre>
 -----                                      -----                                      -----
 |   | Port A1   Connection Type    Port B1 |   | Port B2   Connection Type    Port C1 |   |
 | A |--------- -----------------  ---------| B |--------- -----------------  ---------| C |
 |   |                                      |   |                                      |   |
 -----                                      -----                                      -----
 </pre>

 NOTE: This interface will return only direction connections.  In the scenario above, passing in the target for A will only return B and the A1B1 connection. If you want to get C, you would have to recall the interface using B as the input target to get B2C1
 TARGET DEPTH  : cage, node, slot, pos<br>
 TARGET STATES : Must be Initialized<br>
*/
uint32_t ecmdQueryConnectedTargets(const ecmdChipTarget & i_target, const char * i_connectionType, std::list<ecmdConnectionData> & o_connections); 

/**
 @brief Fetch the detailed chip data structure for the selected target
 @param i_target ecmdChipTarget that information is requested for
 @param o_data ecmdChipData struct that contains detailed info on chip ec level, etc.
 @retval ECMD_SUCCESS if chip data for target is found, non-zero otherwise

 TARGET DEPTH  : Pos
 TARGET STATES : Unused
 */
uint32_t ecmdGetChipData(const ecmdChipTarget & i_target, ecmdChipData & o_data);

/**
 @brief Make a query of the processor to determine the mode in which the core is operating.
 @param i_target processor to query
 @param o_coreMode core mode of the processor
 @param o_coreChipUnit name of the chipunit by which to reference the operating core.

 TARGET DEPTH : pos<br>
 TARGET STATES : Unused

*/

uint32_t ecmdQueryMode(const ecmdChipTarget & i_target, std::string & o_coreMode, std::string & o_coreChipUnit);
//@}


/** @name Scan Functions 
 * Some functions of this section work with different scanmodes. 
 * Using the ecmdSetConfiguration() API allows to change the scanmode different to the default scanmode.
 * Please see documentation at ecmdSetConfiguration() for supported scanmodes. 
*/
//@{

#ifndef ECMD_REMOVE_RING_FUNCTIONS
/**
 @brief Scans the ring from the selected chip into the data buffer
 @retval ECMD_INVALID_RING if ringname is not valid for target
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_ringName Name of ring to read from
 @param o_data DataBuffer object that holds data read from ring
 @param i_mode Ring mode bit-field using ECMD_RING_MODE* defines.  Default value is 0.
 @see putRing


 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>

*/
uint32_t getRing(const ecmdChipTarget & i_target, const char * i_ringName, ecmdDataBuffer & o_data, uint32_t i_mode = 0); 

/**
 @brief Scans ring from the data buffer into the selected chip in the selected ring
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @retval ECMD_DATA_UNDERFLOW  Too little data was provided to a write function
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_RING if ringname is not valid for target
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_ringName Name of ring to write to
 @param i_data DataBuffer object that holds data to write into ring
 @param i_mode Ring mode bit-field using ECMD_RING_MODE* defines.  Default value is 0.
 @see getRing


 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>

*/
uint32_t putRing(const ecmdChipTarget & i_target, const char * i_ringName, const ecmdDataBuffer & i_data, uint32_t i_mode = 0); 

/**
 @brief Performs sparse scan of the ring from the selected chip into the data buffer.
 A sparse scan uses the chip's scom interface to only get the bits in the ring that are set in the mask value.
 @retval ECMD_INVALID_RING if ringname is not valid for target
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_CHIP_SPARSE_SCAN_NOT_SUPPORTED This chip does not support sparse scans via scom interface
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_ringName Name of ring to read from
 @param o_data DataBuffer object that holds data read from ring
 @param i_mask DataBuffer object that holds mask value of bits to read from the ring
 @param i_mode Ring mode bit-field using ECMD_RING_MODE* defines.  Default value is 0.
 @see putRingSparse


 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>

*/
uint32_t getRingSparse(const ecmdChipTarget & i_target, const char * i_ringName, ecmdDataBuffer & o_data, const ecmdDataBuffer & i_mask, uint32_t i_mode = 0); 

/**
 @brief Performs sparse scan of ring from the data buffer into the selected chip in the selected ring.
 A sparse scan uses the chip's scom interface to only write the bits in the ring that are set in the mask value.
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @retval ECMD_DATA_UNDERFLOW  Too little data was provided to a write function
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_RING if ringname is not valid for target
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_CHIP_SPARSE_SCAN_NOT_SUPPORTED This chip does not support sparse scans via scom interface
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_ringName Name of ring to write to
 @param i_data DataBuffer object that holds data to write into ring
 @param i_mask DataBuffer object that holds mask value of bits to write to the ring
 @param i_mode Ring mode bit-field using ECMD_RING_MODE* defines.  Default value is 0.
 @see getRingSparse


 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>

*/
uint32_t putRingSparse(const ecmdChipTarget & i_target, const char * i_ringName, const ecmdDataBuffer & i_data, const ecmdDataBuffer & i_mask, uint32_t i_mode = 0);

/**
 @brief Performs sparse scan of the ring from the selected chip into the data buffer.
 A sparse scan uses the chip's scom interface to only get the bits in the ring that are set in the mask value.
 @retval ECMD_INVALID_RING if ringname is not valid for target
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_CHIP_SPARSE_SCAN_NOT_SUPPORTED This chip does not support sparse scans via scom interface
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_ringName Name of ring to read from
 @param o_data DataBuffer object that holds data read from ring
 @param i_mask DataBuffer object that holds mask value of bits to read from the ring
 @param i_traceMask DataBuffer object that holds bit positions of interest to be ANDed with i_mask and displayed in DFT MASKOUT traces
 @param i_mode Ring mode bit-field using ECMD_RING_MODE* defines.  Default value is 0.
 @see putRingSparse


 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>

*/
uint32_t getRingSparseWithTraceMask(const ecmdChipTarget & i_target, const char * i_ringName, ecmdDataBuffer & o_data, const ecmdDataBuffer & i_mask, const ecmdDataBuffer & i_traceMask, uint32_t i_mode = 0);
 
/**
 @brief Scans the specified number of bits from the selected chip and ring address into the data buffer.
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_address Address of ring to read from
 @param i_bitLength Bit Length to scan for
 @param o_data DataBuffer object that holds data read from ring
 @see putRingWithModifier
s
 NOTE : This is a debug interface and should not be used in normal situations<br>
 NOTE : This function does not handle processor chipUnits for you, the i_address will be taken and used with no modifications
          so you are responsible for specifying the correct chipUnit address<br>
 NOTE : This function will only scan for the length provided, if this length doesn't match the actual length of the ring
          corruption may occur<br>
 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused<br>
*/
uint32_t getRingWithModifier(const ecmdChipTarget & i_target, uint32_t i_address, uint32_t i_bitLength, ecmdDataBuffer & o_data); 

/**
 @brief Scans the specified number of bits from the data buffer into the selected chip in the selected ring address
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @retval ECMD_DATA_UNDERFLOW  Too little data was provided to a write function
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_address Address of ring to write to
 @param i_bitLength Bit Length to scan for
 @param i_data DataBuffer object that holds data to write into ring
 @see getRingWithModifier

 NOTE : This is a debug interface and should not be used in normal situations<br>
 NOTE : This function does not handle processor chipUnits for you, the i_address will be taken and used with no modifications
          so you are responsible for specifying the correct chipUnit address<br>
 NOTE : This function will only scan for the length provided, if this length doesn't match the actual length of the ring
          corruption may occur<br>
 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused<br>
*/
uint32_t putRingWithModifier(const ecmdChipTarget & i_target, uint32_t i_address, uint32_t i_bitLength, const ecmdDataBuffer & i_data); 
#endif // ECMD_REMOVE_RING_FUNCTIONS



//@}



#ifndef ECMD_REMOVE_SCOM_FUNCTIONS
/** @name Scom Functions  */
//@{

/**
 @brief Scoms bits from the selected address into the data buffer
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_address Scom address to read from
 @param o_data DataBuffer object that holds data read from address
 @see putScom

 NOTE : For processor chipUnits, only "chipUnit0" addresses are supported, other chipUnit addresses cause a failure<br>
 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>
*/
uint32_t getScom(const ecmdChipTarget & i_target, uint64_t i_address, ecmdDataBuffer & o_data);

/**
 @brief Scoms bits from the data buffer into the selected address
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @retval ECMD_DATA_UNDERFLOW  Too little data was provided to a write function
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_address Scom address to write to
 @param i_data DataBuffer object that holds data to write into address
 @see getScom

 NOTE : For processor chipUnits, only "chipUnit0" addresses are supported, other chipUnit addresses cause a failure<br>
 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>

*/
uint32_t putScom(const ecmdChipTarget & i_target, uint64_t i_address, const ecmdDataBuffer & i_data);

/**
 @brief Perform a read/modify/write scom operation using the mask and data
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @retval ECMD_DATA_UNDERFLOW  Too little data was provided to a write function
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_address Scom address to write to
 @param i_data DataBuffer object that holds data to write into address
 @param i_mask DataBuffer of the bits to apply from i_data to the data read from the address
 @see getScom

 NOTE : For processor chipUnits, only "chipUnit0" addresses are supported, other chipUnit addresses cause a failure<br>
 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>

*/
uint32_t putScomUnderMask(const ecmdChipTarget & i_target, uint64_t i_address, const ecmdDataBuffer & i_data, const ecmdDataBuffer & i_mask);

/**
 @brief Do a sequence of reads and writes in one operation
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @retval ECMD_DATA_UNDERFLOW  Too little data was provided to a write function
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param io_entries The list of scom operations to do
 @see getScom

 The return value of this function is set to the first non-zero return code found when
  retrieving multiple entries.  The entry that caused the failure in the list will also be marked with
  the same return code. That data and all subsequent entries in the list will not be fetched and the data
  should be considered invalid.

 NOTE : For processor chipUnits, only "chipUnit0" addresses are supported, other chipUnit addresses cause a failure<br>
 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>

*/
uint32_t doScomMultiple(const ecmdChipTarget & i_target, std::list<ecmdScomEntry> & io_entries);

/**
   @brief  Converts zero chip unit address into chip unit address that is being passed
   @retval returns ECMD_SUCCESS if successful
   @retval returns ECMD_FUNCTION_NOT_SUPPORTED if it is no supported
   @retval nonzero if unsuccessful
   @param  i_target Struct that contains cage information
   @param  i_address conains zero chip unit address
   @param  o_address contains the corresponding address of the chip unit
*/
uint32_t ecmdCreateChipUnitScomAddress(const ecmdChipTarget & i_target, uint64_t i_address, uint64_t & o_address);


//@}
/* End Scom Functions */
#endif // ECMD_REMOVE_SCOM_FUNCTIONS



#ifndef ECMD_REMOVE_JTAG_FUNCTIONS
/** @name Jtag Functions */
//@{

/**
 @brief Send a JTAG instruction and modifier to the specified chip
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_instruction Right aligned instruction to send to chip
 @param i_modifier Right aligned instruction modifier to send
 @param o_status Instruction status register value retrieved
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval ECMD_NON_JTAG_CHIP Chip Target is a non-jtag attached chip
 @retval nonzero if unsuccessful

 NOTE : Proper parity will be generated on the command and modifier<br>
 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused<br>
*/
uint32_t sendCmd(const ecmdChipTarget & i_target, uint32_t i_instruction, uint32_t i_modifier, ecmdDataBuffer & o_status);

//@}
/* End Jtag Functions */
#endif // ECMD_REMOVE_JTAG_FUNCTIONS



#ifndef ECMD_REMOVE_FSI_FUNCTIONS
/** @name FSI Functions */
//@{

/**
 @brief Read data from the selected CFAM register address into the data buffer
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_NON_FSI_CHIP Targetted chip is not attached via FSI
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_address CFAM address to read from
 @param o_data DataBuffer object that holds data read from address

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused<br>

*/
uint32_t getCfamRegister(const ecmdChipTarget & i_target, uint32_t i_address, ecmdDataBuffer & o_data);

/**
 @brief Write data into the selected CFAM register address 
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @retval ECMD_DATA_UNDERFLOW  Too little data was provided to a write function
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_SUCCESS if successful
 @retval ECMD_NON_FSI_CHIP Targetted chip is not attached via FSI
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_address CFAM address to write to
 @param i_data DataBuffer object that holds data to write into address

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused<br>

*/
uint32_t putCfamRegister(const ecmdChipTarget & i_target, uint32_t i_address, const ecmdDataBuffer & i_data);

/**
 @brief Calls HWP to read ecid from the targeted chip
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param o_ecidData DataBuffer object that holds ecid

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused<br>

*/
uint32_t getEcid(const ecmdChipTarget & i_target, ecmdDataBuffer & o_ecidData);

/**
 @brief Calls HWP to read ecid from the targeted chip and print out extra information
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param o_ecidData DataBuffer object that holds ecid
 @param o_additionalInfo Vector of strings that store additional verbose output to be displayed

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused<br>

*/
uint32_t getEcidVerbose(const ecmdChipTarget & i_target, ecmdDataBuffer & o_ecidData, std::vector<std::string> & o_additionalInfo);

//@}
/* End FSI Functions */
#endif // ECMD_REMOVE_FSI_FUNCTIONS




/** @name Spy Functions */
//@{
#ifndef ECMD_REMOVE_SPY_FUNCTIONS
/**
 @brief Reads the selected spy into the data buffer
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SPY_FAILED_ECC_CHECK if invalid ECC detected on Spy read
 @retval ECMD_INVALID_SPY Spy name is invalid or Spy is an ECC Grouping
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_SPY_IS_EDIAL Spy is an edial have to use getSpyEnum
 @retval ECMD_SPY_GROUP_MISMATCH A mismatch was found reading a group spy not all groups set the same
 @retval ECMD_SUCCESS if successful read
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_spyName Name of spy to read from
 @param o_data DataBuffer object that holds data read from spy
 @param i_flags Ring mode bit-field using ECMD_RING_MODE* defines.  Default value is 0. Use ECMD_RING_MODE_SPARSE_ACCESS for sparse ring access.

 NOTE : This function is ring cache enabled<br>
 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>

*/
uint32_t getSpy(const ecmdChipTarget & i_target, const char * i_spyName, ecmdDataBuffer & o_data, uint32_t i_flags = 0);

/**
 @brief Reads the selected spy and returns it's assocaiated enum
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SPY_FAILED_ECC_CHECK if invalid ECC detected on Spy read - valid Spy Data still returned
 @retval ECMD_INVALID_SPY Spy name is invalid or Spy is an ECC Grouping
 @retval ECMD_INVALID_SPY_ENUM if value in hardware doesn't map to a valid enum
 @retval ECMD_SPY_NOT_ENUMERATED Spy is not enumerated must use getSpy
 @retval ECMD_SPY_GROUP_MISMATCH A mismatch was found reading a group spy not all groups set the same
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_SUCCESS if successful read
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_spyName Name of spy to read from
 @param o_enumValue Enum value read from the spy
 @param i_flags Ring mode bit-field using ECMD_RING_MODE* defines.  Default value is 0. Use ECMD_RING_MODE_SPARSE_ACCESS for sparse ring access.

 NOTE : This function is ring cache enabled<br>
 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>

*/
uint32_t getSpyEnum(const ecmdChipTarget & i_target, const char * i_spyName, std::string & o_enumValue, uint32_t i_flags = 0);

/**
 @brief Read an ECC grouping and return the in and out bits as well as a error mask if any out bits are invalid
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval ECMD_INVALID_SPY Spy name is invalid or Spy is not an ECC Grouping
 @retval ECMD_SPY_FAILED_ECC_CHECK if invalid ECC detected on Spy read - valid Spy Data still returned
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_spyEpCheckersName Name of spy to read from
 @param o_inLatchData Return the data for the input to the eccGroup
 @param o_outLatchData Return the Ecc data associated with the outbits of the eccGroup
 @param o_eccErrorMask Return a mask for the Ecc data a 1 in the mask means the associated eccData was in error
 @param i_flags Ring mode bit-field using ECMD_RING_MODE* defines.  Default value is 0. Use ECMD_RING_MODE_SPARSE_ACCESS for sparse ring access.
 @retval nonzero if unsuccessful

 NOTE : This function is ring cache enabled<br>
 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>

*/
uint32_t getSpyEpCheckers(const ecmdChipTarget & i_target, const char * i_spyEpCheckersName, ecmdDataBuffer & o_inLatchData, ecmdDataBuffer & o_outLatchData, ecmdDataBuffer & o_eccErrorMask, uint32_t i_flags = 0);

/**
 @brief Reads the selected spy and load all the spy groups into provided list
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SPY_FAILED_ECC_CHECK if invalid ECC detected on Spy read
 @retval ECMD_INVALID_SPY Spy name is invalid or Spy is an ECC Grouping
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_SPY_IS_EDIAL Spy is an edial have to use getSpyEnum
 @retval ECMD_SUCCESS if successful read
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_spyName Name of spy to read from
 @param o_groups List of structures containing the group data and deadbits mask
 @param i_flags Ring mode bit-field using ECMD_RING_MODE* defines.  Default value is 0. Use ECMD_RING_MODE_SPARSE_ACCESS for sparse ring access.

 NOTE : This function is ring cache enabled<br>
 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>

*/
uint32_t getSpyGroups(const ecmdChipTarget & i_target, const char * i_spyName, std::list < ecmdSpyGroupData > & o_groups, uint32_t i_flags = 0);

/**
 @brief Writes the data buffer into the selected spy
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval ECMD_INVALID_SPY Spy name is invalid or Spy is an ECC Grouping
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @retval ECMD_DATA_UNDERFLOW  Too little data was provided to a write function
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_SPY_IS_EDIAL Spy is an edial have to use putSpyEnum
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_spyName Name of spy to write to
 @param i_data DataBuffer object that holds data to write into spy
 @param i_flags Ring mode bit-field using ECMD_RING_MODE* defines.  Default value is 0. Use ECMD_RING_MODE_SPARSE_ACCESS for sparse ring access.

 NOTE : This function is ring cache enabled<br>
 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>

*/
uint32_t putSpy(const ecmdChipTarget & i_target, const char * i_spyName, const ecmdDataBuffer & i_data, uint32_t i_flags = 0);

/**
 @brief Writes the enum into the selected spy
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval ECMD_INVALID_SPY Spy name is invalid or Spy is an ECC Grouping
 @retval ECMD_SPY_NOT_ENUMERATED Spy is not enumerated must use putSpy
 @retval ECMD_INVALID_SPY_ENUM if enum value specified is not valid
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_spyName Name of spy to write to
 @param i_enumValue String enum value to load into the spy
 @param i_flags Ring mode bit-field using ECMD_RING_MODE* defines.  Default value is 0. Use ECMD_RING_MODE_SPARSE_ACCESS for sparse ring access.

 NOTE : This function is ring cache enabled<br>
 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>

*/
uint32_t putSpyEnum(const ecmdChipTarget & i_target, const char * i_spyName, const std::string i_enumValue, uint32_t i_flags = 0);

/**
 @brief Reads the selected spy into the data buffer from provided ring image
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SPY_FAILED_ECC_CHECK if invalid ECC detected on Spy read
 @retval ECMD_INVALID_SPY Spy name is invalid or Spy is an ECC Grouping
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_SPY_IS_EDIAL Spy is an edial have to use getSpyEnum
 @retval ECMD_SPY_GROUP_MISMATCH A mismatch was found reading a group spy not all groups set the same
 @retval ECMD_SUCCESS if successful read
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_spyName Name of spy to read from
 @param i_ringImage DataBuffer object that holds the ring image (Buffer must be of the correct length).
 @param o_data DataBuffer object that holds data read from spy

 NOTE : This function is ring cache enabled<br>
 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>

*/
uint32_t getSpyImage(const ecmdChipTarget & i_target, const char * i_spyName, const ecmdDataBuffer & i_ringImage, ecmdDataBuffer & o_data);

/**
 @brief Reads the selected spy and returns it's assocaiated enum from provided ring image
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SPY_FAILED_ECC_CHECK if invalid ECC detected on Spy read - valid Spy Data still returned
 @retval ECMD_INVALID_SPY Spy name is invalid or Spy is an ECC Grouping
 @retval ECMD_INVALID_SPY_ENUM if value in hardware doesn't map to a valid enum
 @retval ECMD_SPY_NOT_ENUMERATED Spy is not enumerated must use getSpy
 @retval ECMD_SPY_GROUP_MISMATCH A mismatch was found reading a group spy not all groups set the same
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_SUCCESS if successful read
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_spyName Name of spy to read from
 @param i_ringImage DataBuffer object that holds the ring image (Buffer must be of the correct length).
 @param o_enumValue Enum value read from the spy

 NOTE : This function is ring cache enabled<br>
 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>

*/
uint32_t getSpyEnumImage(const ecmdChipTarget & i_target, const char * i_spyName, const ecmdDataBuffer & i_ringImage, std::string & o_enumValue);

/**
 @brief Reads the selected spy into the data buffer from provided ring images
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SPY_FAILED_ECC_CHECK if invalid ECC detected on Spy read
 @retval ECMD_INVALID_SPY Spy name is invalid or Spy is an ECC Grouping
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_SPY_IS_EDIAL Spy is an edial have to use getSpyEnum
 @retval ECMD_SPY_GROUP_MISMATCH A mismatch was found reading a group spy not all groups set the same
 @retval ECMD_MULTIPLE_RING_IMAGE_SPY Spy spans multiple rings and not enough or incorrect ring images were input
 @retval ECMD_SUCCESS if successful read
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_spyName Name of spy to read from
 @param i_ringImages Map of ring names and corresponding DataBuffer objects that hold the ring images used by the requested spy (Buffer must be of the correct length).
 @param o_data DataBuffer object that holds data read from spy

 NOTE : This function is ring cache enabled<br>
 NOTE : The rings associated with the spy can be retrieved by calling ecmdQuerySpyRings.<br>
 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>

*/
uint32_t getSpyImages(const ecmdChipTarget & i_target, const char * i_spyName, const std::map<std::string, ecmdDataBuffer> & i_ringImages, ecmdDataBuffer & o_data);

/**
 @brief Reads the selected spy and returns it's assocaiated enum from provided ring images
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SPY_FAILED_ECC_CHECK if invalid ECC detected on Spy read - valid Spy Data still returned
 @retval ECMD_INVALID_SPY Spy name is invalid or Spy is an ECC Grouping
 @retval ECMD_INVALID_SPY_ENUM if value in hardware doesn't map to a valid enum
 @retval ECMD_SPY_NOT_ENUMERATED Spy is not enumerated must use getSpy
 @retval ECMD_SPY_GROUP_MISMATCH A mismatch was found reading a group spy not all groups set the same
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_MULTIPLE_RING_IMAGE_SPY Spy spans multiple rings and not enough or incorrect ring images were input
 @retval ECMD_SUCCESS if successful read
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_spyName Name of spy to read from
 @param i_ringImages Map of ring names and corresponding DataBuffer objects that hold the ring images used by the requested spy (Buffer must be of the correct length).
 @param o_enumValue Enum value read from the spy

 NOTE : This function is ring cache enabled<br>
 NOTE : The rings associated with the spy can be retrieved by calling ecmdQuerySpyRings.<br>
 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>

*/
uint32_t getSpyEnumImages(const ecmdChipTarget & i_target, const char * i_spyName, const std::map<std::string, ecmdDataBuffer> & i_ringImages, std::string & o_enumValue);

/**
 @brief Writes the data buffer into the selected spy within the provided ring image
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval ECMD_INVALID_SPY Spy name is invalid or Spy is an ECC Grouping
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @retval ECMD_DATA_UNDERFLOW  Too little data was provided to a write function
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_SPY_IS_EDIAL Spy is an edial have to use putSpyEnum
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_spyName Name of spy to write to
 @param i_data DataBuffer object that holds data to write into spy
 @param io_ringImage DataBuffer object that holds the ring image (Buffer must be of the correct length).

 NOTE : This function is ring cache enabled<br>
 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>

*/
uint32_t putSpyImage(const ecmdChipTarget & i_target, const char * i_spyName, const ecmdDataBuffer & i_data, ecmdDataBuffer & io_ringImage);

/**
 @brief Writes the enum into the selected spy within the provided ring image
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval ECMD_INVALID_SPY Spy name is invalid or Spy is an ECC Grouping
 @retval ECMD_SPY_NOT_ENUMERATED Spy is not enumerated must use putSpy
 @retval ECMD_INVALID_SPY_ENUM if enum value specified is not valid
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_spyName Name of spy to write to
 @param i_enumValue String enum value to load into the spy
 @param io_ringImage DataBuffer object that holds the ring image (Buffer must be of the correct length).

 NOTE : This function is ring cache enabled<br>
 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>

*/

uint32_t putSpyEnumImage(const ecmdChipTarget & i_target, const char * i_spyName, const std::string i_enumValue, ecmdDataBuffer & io_ringImage);

/**
 @brief Writes the data buffer into the selected spy within the provided ring images
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval ECMD_INVALID_SPY Spy name is invalid or Spy is an ECC Grouping
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @retval ECMD_DATA_UNDERFLOW  Too little data was provided to a write function
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_SPY_IS_EDIAL Spy is an edial have to use putSpyEnum
 @retval ECMD_MULTIPLE_RING_IMAGE_SPY Spy spans multiple rings and not enough or incorrect ring images were input
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_spyName Name of spy to write to
 @param i_data DataBuffer object that holds data to write into spy
 @param io_ringImages Map of ring names and cooresponding DataBuffer objects that hold the ring images used by the requested spy (Buffer must be of the correct length).

 NOTE : This function is ring cache enabled<br>
 NOTE : The rings associated with the spy can be retrieved by calling ecmdQuerySpyRings.<br>
 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>

*/
uint32_t putSpyImages(const ecmdChipTarget & i_target, const char * i_spyName, const ecmdDataBuffer & i_data, std::map<std::string, ecmdDataBuffer> & io_ringImages);

/**
 @brief Writes the enum into the selected spy within the provided ring images
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval ECMD_INVALID_SPY Spy name is invalid or Spy is an ECC Grouping
 @retval ECMD_SPY_NOT_ENUMERATED Spy is not enumerated must use putSpy
 @retval ECMD_INVALID_SPY_ENUM if enum value specified is not valid
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_MULTIPLE_RING_IMAGE_SPY Spy spans multiple rings and not enough or incorrect ring images were input
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_spyName Name of spy to write to
 @param i_enumValue String enum value to load into the spy
 @param io_ringImages Map of ring names and corresponding DataBuffer objects that hold the ring images used by the requested spy (Buffer must be of the correct length).

 NOTE : This function is ring cache enabled<br>
 NOTE : The rings associated with the spy can be retrieved by calling ecmdQuerySpyRings.<br>
 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>

*/
uint32_t putSpyEnumImages(const ecmdChipTarget & i_target, const char * i_spyName, const std::string i_enumValue, std::map<std::string, ecmdDataBuffer> & io_ringImages);

#endif // ECMD_REMOVE_SPY_FUNCTIONS
//@}



/** @name Ring Cache Functions */
//@{

/**
 @brief Enables internal caching of read/writes of scan rings to the chip for functions like getring/getspy/getspr
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @post Ring caching is enabled on cache enabled functions
 - Functions that support caching are documented in the detailed description of the function
 - Functions that do not affect the state of the cache are documented in the detailed description of the function
 - Any non-cache enabled function will force a flush of the cache before performing the operation
 - Some Dll's may not support ring caching, they will not fail on these functions but you will not see the performance gains
 - Some Dll's may not support all target levels of caching.  If it doesn't support the level you picked
   it will chose the next highest level.  i.e.  If you enable caching on a chip, and your plugin only supports
   it on a cage, caching will be enabled on the cage level.
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful

 TARGET DEPTH  : cage, node, slot, pos<br>
 TARGET STATES : Must be initialized<br>

*/
uint32_t ecmdEnableRingCache(const ecmdChipTarget & i_target);

/**
 @brief Disable internal caching of reads/writes of scan rings
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful

 NOTE: A Flush of the cache is performed before disabling the cache<br>

 TARGET DEPTH  : cage, node, slot, pos<br>
 TARGET STATES : Must be initialized<br>
*/
uint32_t ecmdDisableRingCache(const ecmdChipTarget & i_target);

/**
 @brief Flush all modified data from the internal cache to the hardware, then remove all rings from cache
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful

 TARGET DEPTH  : cage, node, slot, pos<br>
 TARGET STATES : Must be initialized<br>
*/
uint32_t ecmdFlushRingCache(const ecmdChipTarget & i_target);

/**
 @brief Returns true/false to signify if caching is currently enabled
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @retval true if ring caching is enabled
 @retval false if ring caching is disabled

 TARGET DEPTH  : cage, node, slot, pos<br>
 TARGET STATES : Must be initialized<br>
*/
bool ecmdIsRingCacheEnabled(const ecmdChipTarget & i_target);

//@}




/** @name Array Functions */
//@{
#ifndef ECMD_REMOVE_ARRAY_FUNCTIONS
/**
 @brief Reads bits from the selected array into the data buffer
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_ARRAY if i_arrayName is not valid for target
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_arrayName Name of array to read from
 @param i_address Array Address to read from - length of DataBuffer should be set to length of valid address data
 @param o_data DataBuffer object that holds data read from address
 @param i_width Width of array to return
 @see putArray
 @see getArrayMultiple

 NOTE : If i_width = 0, use the default array width.<br> 
 NOTE : If width < default array width, then truncate data to width.<br>
 NOTE : If width > default arrady width, then zeroes will be added at the end of the data.<br>
 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>

*/
uint32_t getArray(const ecmdChipTarget & i_target, const char * i_arrayName, const ecmdDataBuffer & i_address, ecmdDataBuffer & o_data, uint32_t i_width = 0);

/**
 @brief Reads bits from multiple array addresses/elements into the list of data buffers
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_ARRAY if i_arrayName is not valid for target
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_arrayName Name of array to read from
 @param io_entries list of array entries to fetch
 @param i_width Width of array to return
 @see putArray
 @see getArray

 NOTE : To use this function the io_entries list should be pre-loaded with the addresses to fetch, the associated dataBuffers will be loaded upon return<br>
 NOTE : To fetch all addresses for the specified array pass io_entries as an empty list and it will be populated for you.<br>

 NOTE : If i_width = 0, use the default array width.<br> 
 NOTE : If width < default array width, then truncate data to width.<br>
 NOTE : If width > default arrady width, then zeroes will be added at the end of the data.<br>

 The return value of this function is set to the first non-zero return code found when
  retrieving multiple entries.  The entry that caused the failure in the list will also be marked with
  the same return code. That data and all subsequent entries in the list will not be fetched and the data
  should be considered invalid.

 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>

*/
uint32_t getArrayMultiple(const ecmdChipTarget & i_target, const char * i_arrayName, std::list<ecmdArrayEntry> & io_entries,  uint32_t i_width = 0);

/**
 @brief Writes bits from the data buffer into the selected array
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_ARRAY if i_arrayName is not valid for target
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @retval ECMD_DATA_UNDERFLOW  Too little data was provided to a write function
 @retval ECMD_SUCCESS if successful
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval nonzero if unsuccessful
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_arrayName Name of array to write to
 @param i_data DataBuffer object that holds data to write into array
 @param i_address Array Address to write to - length of DataBuffer should be set to length of valid address data
 @see getArray

 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>

*/
uint32_t putArray(const ecmdChipTarget & i_target, const char * i_arrayName, const ecmdDataBuffer & i_address, const ecmdDataBuffer & i_data);

/**
 @brief Writes bits from the list of entries into the selected array
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_ARRAY if i_arrayName is not valid for target
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @retval ECMD_DATA_UNDERFLOW  Too little data was provided to a write function
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_SUCCESS if successful
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval nonzero if unsuccessful
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_arrayName Name of array to write to
 @param i_entries List of addresses and data to write to chip
 @see getArray

 NOTE : i_entries should be pre-loaded with address and data<br>

 The return value of this function is set to the first non-zero return code found when
  writing multiple entries.  The function will NOT continue through all subsequent entries.<br>

 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>

*/
uint32_t putArrayMultiple(const ecmdChipTarget & i_target, const char * i_arrayName, std::list<ecmdArrayEntry> & i_entries);
#endif // ECMD_REMOVE_ARRAY_FUNCTIONS
//@}



/** @name Clock Functions */
//@{
#ifndef ECMD_REMOVE_CLOCK_FUNCTIONS
/**
 @brief Query the state of the clocks for a domain
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @retval ECMD_INVALID_CLOCK_DOMAIN An invalid clock domain name was specified
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_clockDomain Clock domain to query - as defined in scandef - use "ALL" to check all domains
 @param o_clockState State of clocks for that domain

 TARGET DEPTH  : cage, node, slot, pos, chipUnit<br>
 TARGET STATES : Must be Initialized<br>

*/

uint32_t ecmdQueryClockState(const ecmdChipTarget & i_target, const char * i_clockDomain, ecmdClockState_t & o_clockState);


/**
 @brief Start the clocks in the domain specified
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @retval ECMD_INVALID_CLOCK_DOMAIN An invalid clock domain name was specified
 @retval ECMD_CLOCKS_ALREADY_ON The clocks in the specified domain are already on
 @retval ECMD_CLOCKS_IN_INVALID_STATE The clock in the specified domain are in an unknown state (not all on/off)
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_clockDomain Clock domain to start - as defined in scandef - use "ALL" to start all domains
 @param i_forceState Force the clocks into the appropriate state - ignore if not in correct state to start

 * NOTE : If i_target refers to a particular chip object the i_clockDomain has to be "ALL" or a clock domain as defined in the scandef
          If i_target refers to a Cage/node then i_clockDomain has to be "ALL" or one of the predefined convenience clock domains as
            documented in the eCMD system spec for your particular product.

 TARGET DEPTH  : cage, node, slot, pos, chipUnit<br>
 TARGET STATES : Must be Initialized<br>

*/
uint32_t startClocks(const ecmdChipTarget & i_target, const char * i_clockDomain, bool i_forceState = false);

/**
 @brief Stop the clocks in the domain specified
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @retval ECMD_INVALID_CLOCK_DOMAIN An invalid clock domain name was specified
 @retval ECMD_CLOCKS_ALREADY_OFF The clocks in the specified domain are already off
 @retval ECMD_CLOCKS_IN_INVALID_STATE The clock in the specified domain are in an unknown state (not all on/off)
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_clockDomain Clock domain to stop - as defined in scandef - use "ALL" to stop all domains
 @param i_forceState Force the clocks into the appropriate state - ignore if not in correct state to start
 @param i_mode Bit-field for new options

 * NOTE : If i_target refers to a particular chip object the i_clockDomain has to be "ALL" or a clock domain as defined in the scandef
          If i_target refers to a Cage/node then i_clockDomain has to be "ALL" or one of the predefined convenience clock domains as
            documented in the eCMD system spec for your particular product.<br>
 TARGET DEPTH  : cage, node, slot, pos, chipUnit<br>
 TARGET STATES : Must be Initialized<br>

*/
uint32_t stopClocks(const ecmdChipTarget & i_target, const char * i_clockDomain, bool i_forceState = false, uint32_t i_mode = 0x0);
#endif // ECMD_REMOVE_CLOCK_FUNCTIONS
#ifndef ECMD_REMOVE_REFCLOCK_FUNCTIONS
/**
 @brief Read a current system clock speed
 @param i_target Struct that contains chip and cage information
 @param i_type Clock type to read in system
 @param i_speedType Specifies if o_speed is provided in frequency or cycle time
 @param o_speed Read speed value, specified in Mhz or micro-seconds based on i_speedType
 @param o_freqMode Read mode(single/min-max) set in the system
 @param o_freqmin Read minimum frequency value set for minmax mode
 @param o_freqmax Read maximum frequency value set for min-max mode
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful

 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Must be Initialized<br>
*/
uint32_t ecmdGetClockSpeed(const ecmdChipTarget & i_target, ecmdClockType_t i_type, ecmdClockSpeedType_t i_speedType, uint32_t & o_speed, ecmdClockFreqMode_t & o_freqMode, uint32_t & o_freqmin, uint32_t & o_freqmax);

/**
 @brief Change a system clock speed without adjusting system initialization settings using speed value
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_type Clock type to change in system
 @param i_speed based on i_speedType: could be specified in Mhz, micro-seconds, or percentage points
 @param i_speedType Specifies if i_speed is provided in frequency, cycle time, or % of nominal or current value
 @param i_mode Do adjustment in one step or steer the clock to the new value
 @param i_range Adjustments for the clock steer procedure
 @param i_freqMode To configure speed/min,max values based on frequency mode 
 @param i_freqMinMhz To set minimum frequency value for minmax mode in Mhz
 @param i_freqMaxMhz To set maximum frequency value for minmax mode in Mhz
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful

 TARGET DEPTH  : cage, pos, chipUnit<br>
 TARGET STATES : Must be Initialized<br>
*/
uint32_t ecmdSetClockSpeed(const ecmdChipTarget & i_target, ecmdClockType_t i_type, uint32_t i_speed, ecmdClockSpeedType_t i_speedType, ecmdClockSetMode_t i_mode, ecmdClockRange_t i_range, ecmdClockFreqMode_t i_freqMode = ECMD_CLOCK_SINGLE_FREQ_MODE, uint32_t i_freqMinMhz = 0, uint32_t i_freqMaxMhz = 0);

/**
 @brief Change a system clock speed without adjusting system initialization settings using speed value
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_type Clock type to change in system
 @param i_multiplier Multiplier to use to program clock
 @param i_divider Divider value to use to program clock
 
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful

 TARGET DEPTH  : cage, chipUnit<br>
 TARGET STATES : Must be Initialized<br>
*/
uint32_t ecmdSetClockMultDiv(const ecmdChipTarget & i_target, ecmdClockType_t i_type, uint32_t i_multiplier, uint32_t i_divider);

#endif // ECMD_REMOVE_REFCLOCK_FUNCTIONS
//@}


#ifndef ECMD_REMOVE_INIT_FUNCTIONS
/** @name Initialization Functions */
//@{

/**
 @brief Run iSteps by number
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_ISTEPS_INVALID_STEP An invalid step number was provided
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @post iSteps specified  are complete
 @param i_steps Bit mask defining which steps to run

 NOTE - function returns on first failure and remaining steps are not run<br>
*/
uint32_t iStepsByNumber(const ecmdDataBuffer & i_steps);

/**
 @brief Run a single iStep by name
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_ISTEPS_INVALID_STEP An invalid step name was provided
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @post iStep specified is complete
 @param i_stepName List of iStep names to run
*/
uint32_t iStepsByName(std::string i_stepName);

/**
 @brief Run multiple iSteps by name
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_ISTEPS_INVALID_STEP An invalid step name was provided
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @post iSteps specified  are complete
 @param i_stepNames List of iStep names to run

 NOTE - Steps are run in order as is appropriate for proper system configuration, not by order provided in list<br>
 NOTE - function returns on first failure and remaining steps are not run<br>
*/
uint32_t iStepsByNameMultiple(std::list< std::string > i_stepNames);

/**
 @brief Run all iSteps by name starting with i_stepNameBegin and ending with i_stepNameEnd
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_ISTEPS_INVALID_STEP An invalid step name was provided
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @post iSteps specified  are complete
 @param i_stepNameBegin Starting iStep to run
 @param i_stepNameEnd Ending iStep to run

 NOTE - function returns on first failure and remaining steps are not run<br>
*/
uint32_t iStepsByNameRange(std::string i_stepNameBegin, std::string i_stepNameEnd);

/**
 @brief Run chip initializations stored in a file
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_initFile The file containing the initialization data
 @param i_initId The id in the initialization file to use
 @param i_mode The mode to run the initialization file in
 @param i_ringMode Ring mode bit-field using ECMD_RING_MODE* defines.  Default value is 0.
 
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful

 TARGET DEPTH  : pos<br>
 TARGET STATES : Must be Initialized<br>
*/
uint32_t initChipFromFile(const ecmdChipTarget & i_target, const char* i_initFile, const char* i_initId, const char* i_mode, uint32_t i_ringMode = 0);

//@}
#endif // ECMD_REMOVE_INIT_FUNCTIONS



/** @name Processor Functions */
//@{

#ifndef ECMD_REMOVE_PROCESSOR_FUNCTIONS
/**
 @brief Query Information about a Processor Register (SPR/GPR/FPR)
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_name Name of the Register to fetch data about (can be either a specific SPR or GPR/FPR)
 @param o_data Data retrieved about the register
 @retval ECMD_TARGET_INVALID_TYPE if target is not a processor
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_SPR Spr name is invalid
 @retval ECMD_SUCCESS if successful read
 @retval nonzero on failure

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused<br>
*/
uint32_t ecmdQueryProcRegisterInfo(const ecmdChipTarget & i_target, const char* i_name, ecmdProcRegisterInfo & o_data);

/**
 @brief Reads the selected Processor Architected Special Purpose Register (SPR) into the data buffer
 @retval ECMD_TARGET_INVALID_TYPE if target is not a processor
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_SPR Spr name is invalid
 @retval ECMD_INACTIVE_THREAD if the thread is inactive and data should be ignored.
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_SUCCESS if successful read
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_sprName Name of spr to read from
 @param o_data DataBuffer object that holds data read from spr

 TARGET DEPTH  : chipUnit, thread<br>
 TARGET STATES : Unused<br>

*/
uint32_t getSpr (const ecmdChipTarget & i_target, const char * i_sprName, ecmdDataBuffer & o_data);

/**
 @brief Reads the selected Processor Architected Special Purpose Register (SPR) into the data buffer
 @retval ECMD_TARGET_INVALID_TYPE if target is not a processor
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_SPR Spr name is invalid
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_SUCCESS if successful read
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param io_entries List of entries to fetch ecmdNameEntry.name field must be filled in

 - NOTE : There are special keywords that can be specified to fetch groups of entries, they
          are used by adding only an entry to io_entries and setting ecmdNameEntry.name = -keyword-
   - "ALLTHREADED" : To fetch all threaded (replicated) SPR's for particular target
   - "ALLSHARED"   : To fetch all non-threaded SPR's for particular target
   - "LPARSHARED"  : To fetch all LPAR shared SPR's for particular target

 The return value of this function is set to the first non-zero return code found when
  retrieving multiple entries.  The entry that caused the failure in the list will also be marked with
  the same return code. That data and all subsequent entries in the list will not be fetched and the data
  should be considered invalid.  If an entry is for an inactive thread, the function return code will not
  be set, but the entry rc will be set to ECMD_INACTIVE_THREAD.  The caller should ignore that entry data
  and continue to the next entry.<br>

 TARGET DEPTH  : chipUnit, thread<br>
 TARGET STATES : Unused<br>

*/
uint32_t getSprMultiple (const ecmdChipTarget & i_target, std::list<ecmdNameEntry> & io_entries);


/**
 @brief Writes the data buffer into the selected Processor Architected Special Purpose Register (SPR)
 @retval ECMD_TARGET_INVALID_TYPE if target is not a processor
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval ECMD_INVALID_SPR Spr name is invalid
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @retval ECMD_DATA_UNDERFLOW  Too little data was provided to a write function
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_sprName Name of spr to write to
 @param i_data DataBuffer object that holds data to write into spr

 TARGET DEPTH  : chipUnit, thread<br>
 TARGET STATES : Unused<br>

*/
uint32_t putSpr (const ecmdChipTarget & i_target, const char * i_sprName, const ecmdDataBuffer & i_data);

/**
 @brief Writes the data buffer into the selected Processor Architected Special Purpose Register (SPR)
 @retval ECMD_TARGET_INVALID_TYPE if target is not a processor
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval ECMD_INVALID_SPR Spr name is invalid
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @retval ECMD_DATA_UNDERFLOW  Too little data was provided to a write function
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param io_entries List of entries to write all ecmdNameEntry fields must be filled in

 The return value of this function is set to the first non-zero return code found when
  writing multiple entries.  The function will NOT continue through all subsequent entries.<br>

 TARGET DEPTH  : chipUnit, thread<br>
 TARGET STATES : Unused<br>

*/
uint32_t putSprMultiple (const ecmdChipTarget & i_target, std::list<ecmdNameEntry> & io_entries);

/**
 @brief Reads the selected Processor Architected General Purpose Register (GPR) into the data buffer
 @retval ECMD_TARGET_INVALID_TYPE if target is not a processor
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_GPR Gpr number is invalid
 @retval ECMD_INACTIVE_THREAD if the thread is inactive and data should be ignored.
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_SUCCESS if successful read
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_gprNum Number of gpr to read from
 @param o_data DataBuffer object that holds data read from gpr

 TARGET DEPTH  : chipUnit, thread<br>
 TARGET STATES : Unused<br>

*/
uint32_t getGpr (const ecmdChipTarget & i_target, uint32_t i_gprNum, ecmdDataBuffer & o_data);

/**
 @brief Reads the selected Processor Architected General Purpose Register (GPR) into the data buffer
 @retval ECMD_TARGET_INVALID_TYPE if target is not a processor
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_GPR Gpr number is invalid
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_SUCCESS if successful read
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param io_entries List of entries to fetch ecmdIndexEntry.index field must be filled in

 The return value of this function is set to the first non-zero return code found when
  retrieving multiple entries.  The entry that caused the failure in the list will also be marked with
  the same return code. That data and all subsequent entries in the list will not be fetched and the data
  should be considered invalid. If an entry is for an inactive thread, the function return code will not
  be set, but the entry rc will be set to ECMD_INACTIVE_THREAD.  The caller should ignore that entry data
  and continue to the next entry.<br>

 TARGET DEPTH  : chipUnit, thread<br>
 TARGET STATES : Unused<br>

*/
uint32_t getGprMultiple (const ecmdChipTarget & i_target, std::list<ecmdIndexEntry> & io_entries);


/**
 @brief Writes the data buffer into the selected Processor Architected General Purpose Register (GPR)
 @retval ECMD_TARGET_INVALID_TYPE if target is not a processor
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_GPR Gpr number is invalid
 @retval ECMD_SUCCESS if successful
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @retval ECMD_DATA_UNDERFLOW  Too little data was provided to a write function
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_gprNum Number of gpr to write to
 @param i_data DataBuffer object that holds data to write into gpr

 TARGET DEPTH  : chipUnit, thread<br>
 TARGET STATES : Unused<br>

*/
uint32_t putGpr (const ecmdChipTarget & i_target, uint32_t i_gprNum, const ecmdDataBuffer & i_data);

/**
 @brief Writes the data buffer into the selected Processor Architected General Purpose Register (GPR)
 @retval ECMD_TARGET_INVALID_TYPE if target is not a processor
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_GPR Gpr number is invalid
 @retval ECMD_SUCCESS if successful
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @retval ECMD_DATA_UNDERFLOW  Too little data was provided to a write function
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param io_entries List of entries to write all ecmdIndexEntry fields must be filled in


 The return value of this function is set to the first non-zero return code found when
  writing multiple entries.  The function will NOT continue through all subsequent entries.<br>

 TARGET DEPTH  : chipUnit, thread<br>
 TARGET STATES : Unused<br>

*/
uint32_t putGprMultiple (const ecmdChipTarget & i_target, std::list<ecmdIndexEntry> & io_entries);

/**
 @brief Reads the selected Processor Architected Floating Point Register (FPR) into the data buffer
 @retval ECMD_TARGET_INVALID_TYPE if target is not a processor
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_INVALID_FPR Fpr number is invalid
 @retval ECMD_INACTIVE_THREAD if the thread is inactive and data should be ignored.
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_SUCCESS if successful read
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_fprNum Number of fpr to read from
 @param o_data DataBuffer object that holds data read from fpr

 TARGET DEPTH  : chipUnit, thread<br>
 TARGET STATES : Unused<br>

*/
uint32_t getFpr (const ecmdChipTarget & i_target, uint32_t i_fprNum, ecmdDataBuffer & o_data);

/**
 @brief Reads the selected Processor Architected Floating Point Register (FPR) into the data buffer
 @retval ECMD_TARGET_INVALID_TYPE if target is not a processor
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_FPR Fpr number is invalid
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_SUCCESS if successful read
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param io_entries List of entries to fetch ecmdIndexEntry.index field must be filled in

 The return value of this function is set to the first non-zero return code found when
  retrieving multiple entries.  The entry that caused the failure in the list will also be marked with
  the same return code. That data and all subsequent entries in the list will not be fetched and the data
  should be considered invalid. If an entry is for an inactive thread, the function return code will not
  be set, but the entry rc will be set to ECMD_INACTIVE_THREAD.  The caller should ignore that entry data
  and continue to the next entry.<br>

 TARGET DEPTH  : chipUnit, thread<br>
 TARGET STATES : Unused<br>

*/
uint32_t getFprMultiple (const ecmdChipTarget & i_target, std::list<ecmdIndexEntry> & io_entries);

/**
 @brief Writes the data buffer into the selected Processor Architected Floating Point Register (FPR)
 @retval ECMD_TARGET_INVALID_TYPE if target is not a processor
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval ECMD_INVALID_FPR Fpr number is invalid
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @retval ECMD_DATA_UNDERFLOW  Too little data was provided to a write function
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_fprNum Number of fpr to write to
 @param i_data DataBuffer object that holds data to write into fpr

 TARGET DEPTH  : chipUnit, thread<br>
 TARGET STATES : Unused<br>

*/
uint32_t putFpr (const ecmdChipTarget & i_target, uint32_t i_fprNum, const ecmdDataBuffer & i_data);

/**
 @brief Writes the data buffer into the selected Processor Architected Floating Point Register (FPR)
 @retval ECMD_TARGET_INVALID_TYPE if target is not a processor
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_FPR Fpr number is invalid
 @retval ECMD_SUCCESS if successful
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @retval ECMD_DATA_UNDERFLOW  Too little data was provided to a write function
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param io_entries List of entries to write all ecmdIndexEntry fields must be filled in

 The return value of this function is set to the first non-zero return code found when
  writing multiple entries.  The function will NOT continue through all subsequent entries.<br>

 TARGET DEPTH  : chipUnit, thread<br>
 TARGET STATES : Unused<br>

*/
uint32_t putFprMultiple (const ecmdChipTarget & i_target, std::list<ecmdIndexEntry> & io_entries);


/**
 @brief Reads the selected Processor SLB Entry into the data buffer
 @retval ECMD_TARGET_INVALID_TYPE if target is not a processor
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_INVALID_ENTRY_REQUESTED Slb number is invalid
 @retval ECMD_INACTIVE_THREAD if the thread is inactive and data should be ignored.
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_SUCCESS if successful read
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_slbNum Number of fpr to read from
 @param o_data DataBuffer object that holds data read from fpr

 TARGET DEPTH  : chipUnit, thread<br>
 TARGET STATES : Unused<br>

*/
uint32_t getSlb (const ecmdChipTarget & i_target, uint32_t i_slbNum, ecmdDataBuffer & o_data);

/**
 @brief Reads the selected Processor SLB Entry into the data buffer
 @retval ECMD_TARGET_INVALID_TYPE if target is not a processor
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_ENTRY_REQUESTED Slb number is invalid
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_SUCCESS if successful read
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param io_entries List of entries to fetch ecmdIndexEntry.index field must be filled in with slb number

 The return value of this function is set to the first non-zero return code found when
  retrieving multiple entries.  The entry that caused the failure in the list will also be marked with
  the same return code. That data and all subsequent entries in the list will not be fetched and the data
  should be considered invalid. If an entry is for an inactive thread, the function return code will not
  be set, but the entry rc will be set to ECMD_INACTIVE_THREAD.  The caller should ignore that entry data
  and continue to the next entry.

 TARGET DEPTH  : chipUnit, thread<br>
 TARGET STATES : Unused<br>

*/
uint32_t getSlbMultiple (const ecmdChipTarget & i_target, std::list<ecmdIndexEntry> & io_entries);

/**
 @brief Writes the data buffer into the selected Processor SLB Entry
 @retval ECMD_TARGET_INVALID_TYPE if target is not a processor
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval ECMD_INVALID_ENTRY_REQUESTED Slb number is invalid
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @retval ECMD_DATA_UNDERFLOW  Too little data was provided to a write function
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_slbNum Number of fpr to write to
 @param i_data DataBuffer object that holds data to write into fpr

 TARGET DEPTH  : chipUnit, thread<br>
 TARGET STATES : Unused<br>

*/
uint32_t putSlb (const ecmdChipTarget & i_target, uint32_t i_slbNum, const ecmdDataBuffer & i_data);

/**
 @brief Writes the data buffer into the selected Processor SLB Entry
 @retval ECMD_TARGET_INVALID_TYPE if target is not a processor
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_ENTRY_REQUESTED Slb number is invalid
 @retval ECMD_SUCCESS if successful
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @retval ECMD_DATA_UNDERFLOW  Too little data was provided to a write function
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param io_entries List of entries to write all ecmdIndexEntry fields must be filled in with slb number

 The return value of this function is set to the first non-zero return code found when
  writing multiple entries.  The function will NOT continue through all subsequent entries.<br>

 TARGET DEPTH  : chipUnit, thread<br>
 TARGET STATES : Unused<br>

*/
uint32_t putSlbMultiple (const ecmdChipTarget & i_target, std::list<ecmdIndexEntry> & io_entries);
#endif // ECMD_REMOVE_PROCESSOR_FUNCTIONS

//@}
/* End Processor Functions */


/** @name Memory Preserving IPL (MPIPL) Functions */
//@{
#ifndef ECMD_REMOVE_MPIPL_FUNCTIONS
/**
 @brief Clear checkstops to allow an MPIPL to take place
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_ARRAY Invalid trace array name specified
 @retval ECMD_SUCCESS if successful

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused<br>

*/
uint32_t mpiplClearCheckstop(const std::vector<ecmdChipTarget> & i_target);

/**
 @brief Force special winkle wakeup (i.e., no loss of ex/core voltage) for MPIPL
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_ARRAY Invalid trace array name specified
 @retval ECMD_SUCCESS if successful

 The return value of this function is set to the first non-zero return code found when
  retrieving multiple entries.  The entry that caused the failure in the list will also be marked with
  the same return code. That data and all subsequent entries in the list will not be fetched and the data
  should be considered invalid.<br>

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused<br>

*/
uint32_t mpiplForceWinkle(const ecmdChipTarget & i_target);
#endif // ECMD_REMOVE_MPIPL_FUNCTIONS
//@}



/** @name Trace Array Functions */
//@{
#ifndef ECMD_REMOVE_TRACEARRAY_FUNCTIONS
/**
 @brief Dump all entries of specified trace array
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_name Name of trace array - names may vary for each product/chip
 @param i_doTraceStopStart If ECMD_TRACE_ARRAY_STOP is set disable trace arrays before logging and if ECMD_TRACE_ARRAY_START is set renable after completion of all trace arrays dumped
 @param o_data List of trace array data retrieved.  Optional name and iteration members can be assigned.
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_ARRAY Invalid trace array name specified
 @retval ECMD_SUCCESS if successful

 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>

*/
uint32_t getTraceArray(const ecmdChipTarget & i_target, const char* i_name, uint32_t i_doTraceStopStart, ecmdNameVectorEntry & o_data);

/**
 @brief Dump all entries of specified trace array
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_doTraceStopStart If ECMD_TRACE_ARRAY_STOP is set disable trace arrays before logging and if ECMD_TRACE_ARRAY_START is set renable after completion of all trace arrays dumped
 @param o_data List of trace array data retrieved.  Optional name and iteration members can be assigned.
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_ARRAY Invalid trace array name specified
 @retval ECMD_SUCCESS if successful

 The return value of this function is set to the first non-zero return code found when
  retrieving multiple entries.  The entry that caused the failure in the list will also be marked with
  the same return code. That data and all subsequent entries in the list will not be fetched and the data
  should be considered invalid.<br>

 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>

*/
uint32_t getTraceArrayMultiple(const ecmdChipTarget & i_target, uint32_t i_doTraceStopStart, std::list <ecmdNameVectorEntry> & o_data);
#endif // ECMD_REMOVE_TRACEARRAY_FUNCTIONS
//@}



/** @name Memory Functions */
//@{
#ifndef ECMD_REMOVE_MEMORY_FUNCTIONS
/**
 @brief Reads System Mainstore through the processor chip using a real address
 @retval ECMD_TARGET_INVALID_TYPE if target is not a processor
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_SUCCESS if successful read
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_address Starting address to read from
 @param i_bytes Number of bytes to write
 @param o_data DataBuffer object that holds data read from memory
 @param i_mode 0 is default, or set to MEMPROC_CACHE_INHIBIT for cache inhibit mode

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused<br>

*/
uint32_t getMemProc(const ecmdChipTarget & i_target, uint64_t i_address, uint32_t i_bytes, ecmdDataBuffer & o_data, uint32_t i_mode = 0x0);

/**
 @brief Writes System Mainstore through the processor chip using a real address
 @retval ECMD_TARGET_INVALID_TYPE if target is not a processor
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_address Starting address to write to
 @param i_bytes Number of bytes to write
 @param i_data DataBuffer object that holds data to write into memory
 @param i_mode 0 is default, or set to MEMPROC_CACHE_INHIBIT for cache inhibit mode

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused<br>

*/
uint32_t putMemProc(const ecmdChipTarget & i_target, uint64_t i_address, uint32_t i_bytes, const ecmdDataBuffer & i_data, uint32_t i_mode = 0x0);

/**
 @brief Reads System Mainstore through the PSI or DMA interface (whichever is avialable) using a real address
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_SUCCESS if successful read
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_address Starting address to read from
 @param i_bytes Number of bytes to write
 @param o_data DataBuffer object that holds data read from memory

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused<br>

*/
uint32_t getMemDma(const ecmdChipTarget & i_target, uint64_t i_address, uint32_t i_bytes, ecmdDataBuffer & o_data);


/**
 @brief Writes System Mainstore through the PSI or DMA interface (whichever is avialable) using a real address
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_address Starting address to write to
 @param i_bytes Number of bytes to write
 @param i_data DataBuffer object that holds data to write into memory

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused<br>

*/
uint32_t putMemDma(const ecmdChipTarget & i_target, uint64_t i_address, uint32_t i_bytes, const ecmdDataBuffer & i_data);

/**
 @brief Reads System Mainstore through the memory controller using a real address
 @retval ECMD_TARGET_INVALID_TYPE if target is not a memory controller
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_SUCCESS if successful read
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_address Starting address to read from
 @param i_bytes Number of bytes to write
 @param o_data DataBuffer object that holds data read from memory

 WARNING : This operation is typically not cache-coherent<br>
 TARGET DEPTH  : cage<br>
 TARGET STATES : Unused<br>

*/
uint32_t getMemMemCtrl(const ecmdChipTarget & i_target, uint64_t i_address, uint32_t i_bytes, ecmdDataBuffer & o_data);


/**
 @brief Writes System Mainstore through the memory controller using a real address
 @retval ECMD_TARGET_INVALID_TYPE if target is not a memory controller
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_address Starting address to write to
 @param i_bytes Number of bytes to write
 @param i_data DataBuffer object that holds data to write into memory

 WARNING : This operation is typically not cache-coherent<br>
 TARGET DEPTH  : cage<br>
 TARGET STATES : Unused<br>

*/
uint32_t putMemMemCtrl(const ecmdChipTarget & i_target, uint64_t i_address, uint32_t i_bytes, const ecmdDataBuffer & i_data);

/**
 @brief Reads SRAM through the processor chip using a real address
 @retval ECMD_TARGET_INVALID_TYPE if target is not a processor
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_SUCCESS if successful read
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_mode Mask to specify api information
 @param i_address Starting address to read from
 @param i_bytes Number of bytes to write
 @param o_data DataBuffer object that holds data read from memory

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused<br>

*/
uint32_t getSram(const ecmdChipTarget & i_target, uint32_t i_mode, uint64_t i_address, uint32_t i_bytes, ecmdDataBuffer & o_data);


/**
 @brief Writes SRAM through the processor chip using a real address
 @retval ECMD_TARGET_INVALID_TYPE if target is not a processor
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_mode Mask to specify api information
 @param i_address Starting address to write to
 @param i_bytes Number of bytes to write
 @param i_data DataBuffer object that holds data to write into memory

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused<br>

*/
uint32_t putSram(const ecmdChipTarget & i_target, uint32_t i_mode, uint64_t i_address, uint32_t i_bytes, const ecmdDataBuffer & i_data);

/**
 @brief Query information about the cache levels
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_cacheType ecmdCacheType_t struct defines what type of cache gets flushed
 @param o_data Information about the cache type queried.
 @retval ECMD_TARGET_NOT_CONFIGURED  if target is not available in the system  
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos<br>
 TARGET STATES : Must be Initialized<br>
*/
uint32_t ecmdQueryCache(const ecmdChipTarget & i_target, ecmdCacheType_t i_cacheType, ecmdCacheData & o_data);

/**
 @brief Cache Flush
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_cacheType ecmdCacheType_t struct defines what type of cache gets flushed
 @retval ECMD_TARGET_NOT_CONFIGURED  if target is not available in the system  
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>
*/
uint32_t ecmdCacheFlush(const ecmdChipTarget & i_target, ecmdCacheType_t i_cacheType);

/**
 @brief Reads System Mainstore through the processor chip using PBA
 @retval ECMD_TARGET_INVALID_TYPE if target is not a processor
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_INVALID_MEMORY_ADDRESS Memory Address was not on a 8-byte boundary
 @retval ECMD_SUCCESS if successful read
 @retval nonzero if unsuccessful
 @param i_target Struct that contains chip and cage/node/slot/position information
 @param i_address Starting address to read from
 @param i_bytes Number of bytes to write
 @param o_data DataBuffer object that holds data read from memory
 @param i_mode Future use

 TARGET DEPTH  : Pos<br>
 TARGET STATES : Unused<br>
*/
uint32_t getMemPba(const ecmdChipTarget & i_target, uint64_t i_address, uint32_t i_bytes, ecmdDataBuffer & o_data, uint32_t i_mode = 0x0);
  
/**
 @brief Writes System Mainstore through the processor chip using PBA
 @retval ECMD_TARGET_INVALID_TYPE if target is not a processor
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_INVALID_MEMORY_ADDRESS Memory Address was not on a 8-byte boundary
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @param i_target Struct that contains chip and cage/node/slot/position information 
 @param i_address Starting address to write to
 @param i_bytes Number of bytes to write
 @param i_data DataBuffer object that holds data to write into memory
 @param i_mode PBA_MODE_LCO or PBA_MODE_INJECT

 NOTE : When PBA_MODE_LCO mode is used, i_target must have ex chipunit<br>

 TARGET DEPTH  : Pos<br>
 TARGET STATES : Unused<br>
*/
uint32_t putMemPba (const ecmdChipTarget & i_target, uint64_t i_address, uint32_t i_bytes, const ecmdDataBuffer & i_data, uint32_t i_mode);

/**
 @brief Query the memory information of the Host processor
 @retval ECMD_TARGET_INVALID_TYPE if target is not a processor
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @param i_targets Vector of targets in physical node (i.e. ecmdChipTarget.node)
 @param o_target Host target
 @param o_address Address where Host memory is located
 @param o_size Size of Host memory
 @param i_mode Input modes - for future use/expansion

 TARGET DEPTH  : Pos<br>
 TARGET STATES : Unused<br>
*/
uint32_t queryHostMemInfo( const std::vector<ecmdChipTarget> & i_targets, ecmdChipTarget & o_target,  uint64_t & o_address, uint64_t & o_size, const uint32_t i_mode = 0x0);

/**
 @brief Query the memory information of the Host processor
 @retval ECMD_TARGET_INVALID_TYPE if target is not a processor
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @param i_targets Vector of targets in physical node (i.e. ecmdChipTarget.node)
 @param o_target Host target
 @param o_ranges Vector of address/size pairs of Host memory locations
 @param i_mode Input modes - for future use/expansion

 TARGET DEPTH  : Pos<br>
 TARGET STATES : Unused<br>
*/
uint32_t queryHostMemInfoRanges( const std::vector<ecmdChipTarget> & i_targets, ecmdChipTarget & o_target, std::vector<std::pair<uint64_t, uint64_t> > & o_ranges, const uint32_t i_mode = 0x0);

#endif // ECMD_REMOVE_MEMORY_FUNCTIONS
//@}
/* End Memory Functions */

/** @name Error Handling Functions */
//@{

/**
 @brief Retrieve additional error information for errorcode
 @param i_returnCode Error code to lookup up message for
 @param i_parseReturnCode If true will search through return codes definitions to return define name of error code
 @param i_deleteMessage If true, the message will be deleted when the user retrieves it.  True is the default.
 @param i_messageBorder If true, the return mesage will have a top and bottom border.  True is the default.
 @retval String containing the error data, empty of error occurred
*/
std::string ecmdGetErrorMsg(uint32_t i_returnCode, bool i_parseReturnCode = true, bool i_deleteMessage = true, bool i_messageBorder = true);

/**
 @brief Register an error message that has occured
 @param i_returnCode The error code to store a message for
 @param i_whom Who is storing the error message
 @param i_message The message to be stored
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful
*/
uint32_t ecmdRegisterErrorMsg(uint32_t i_returnCode, const char* i_whom, const char* i_message);

/**
 @brief Flush registered error message for a specific error code
 @param i_returnCode Error code of the message to erase
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful
*/
uint32_t ecmdFlushRegisteredErrorMsgs(uint32_t i_returnCode);

/**
 @brief Flush registered error message for a specific error code and message
 @param i_returnCode Error code of the message to erase
 @param i_searchString Sub-string to match from error message
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful
*/
uint32_t ecmdFlushRegisteredErrorMsgsString(uint32_t i_returnCode, std::string i_searchString);


/**
 @brief Retrieve the target associated with the errorcode, if there is one
 @param i_returnCode Error code to lookup up message for
 @param o_errorTargets A list of the target associated with the error code, if it exists
 @param i_deleteTarget If true, the target will be deleted when the user retrieves it.  True is the default.
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful
*/
uint32_t ecmdGetErrorTarget(uint32_t i_returnCode, std::list<ecmdChipTarget> & o_errorTargets, bool i_deleteTarget = true);

/**
 @brief Register a target with an error code
 @param i_returnCode The error code to store a message for
 @param i_errorTarget The target associated with the error code
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful
*/
uint32_t ecmdRegisterErrorTarget(uint32_t i_returnCode, const ecmdChipTarget & i_errorTarget);

/**
 @brief Flush registered error targets for a specific error code
 @param i_returnCode Error code of the message to erase
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful
*/
uint32_t ecmdFlushRegisteredErrorTargets(uint32_t i_returnCode);

/**
 @brief Returns the string define name for specified returnCode (ie "ECMD_SPY_GROUP_MISMATCH")
 @param i_returnCode Return code to lookup
 @retval String name of return code ("UNDEFINED" if unknown)
*/
std::string ecmdParseReturnCode(uint32_t i_returnCode);

/**
 @brief return error from last ecmd executed in current thread
*/
std::string ecmdLastError();

//@}


#ifndef ECMD_REMOVE_BLOCK_FUNCTIONS
/** @name Block Functions */
//@{
/**
 @brief Query block entries in system dump
 @param i_target Struct that specifies the target to operate on 
 @param o_metadataBlockEntry Vector of metadata strings for block entries found in system dump
 @param i_metadataHWPName if != all used to refine query to a single HWP type
 @retval ECMD_SUCCESS on success
 @retval non-zero on failure
 */
uint32_t ecmdQueryBlock(const ecmdChipTarget & i_target, std::vector<std::string>& o_metadataBlockEntry, const std::string & i_metadataHWPName = "all");

/**
 @brief Extract block data from hardware or system dump
 @param i_target Struct that specifies the target to operate on 
 @param i_metadataBlockEntry Name of metabdata block entry. This should be in exact metadata string
 @param o_data Data extracted from hardware or dump 
 @retval ECMD_SUCCESS on success
 @retval non-zero on failure
 */
uint32_t ecmdGetBlock(const ecmdChipTarget & i_target, const std::string & i_metadataBlockEntry,  std::vector<ecmdDataBuffer>& o_data);
//@}
#endif // ECMD_REMOVE_BLOCK_FUNCTIONS

/** @name Output Functions */
//@{

/**
 @brief Output a message related to an error
 @param i_message String to output

*/
void ecmdOutputError(const char* i_message);

/**
 @brief Output a message related to an warning
 @param i_message String to output

*/
void ecmdOutputWarning(const char* i_message);

/**
 @brief Output a message to the screen or logs
 @param i_message String to output

*/
void ecmdOutput(const char* i_message);

//@}

/** @name Misc Functions */
//@{

#ifndef ECMD_REMOVE_FSI_FUNCTIONS
/**
 @brief Reads a GP Register from CFAM/Access on the selected chip into the data buffer
 @retval ECMD_INVALID_GP_REGISTER if GP Regiter is not valid for target
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_gpRegister Number of the GP Resiter to read from
 @param o_data DataBuffer object that holds data read from ring


 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused<br>

*/
uint32_t getGpRegister(const ecmdChipTarget & i_target, uint32_t i_gpRegister, ecmdDataBuffer & o_data); 

/**
 @brief Writes a GP Register in CFAM/Access from the data buffer into the selected chip in the selected GP Register
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @retval ECMD_DATA_UNDERFLOW  Too little data was provided to a write function
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_GP_REGISTER if GP Regiter is not valid for target
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_gpRegister Number of the GP Resiter to read from
 @param i_data DataBuffer object that holds data to write into ring


 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused<br>

*/
uint32_t putGpRegister(const ecmdChipTarget & i_target, uint32_t i_gpRegister, const ecmdDataBuffer & i_data); 

/**
 @brief Perform a read/modify/write GP Register operation using the mask and data
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @retval ECMD_DATA_UNDERFLOW  Too little data was provided to a write function
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_GP_REGISTER if GP Regiter is not valid for target
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_gpRegister Number of the GP Resiter to read from
 @param i_data DataBuffer object that holds data to write into address
 @param i_mask DataBuffer of the bits to apply from i_data to the data read from the address

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused<br>

*/
uint32_t putGpRegisterUnderMask(const ecmdChipTarget & i_target, uint32_t i_gpRegister, const ecmdDataBuffer & i_data, const ecmdDataBuffer & i_mask); 
#endif // ECMD_REMOVE_FSI_FUNCTIONS

/**
 @brief Retrieve the value of some ecmdGlobalVars
 @param i_type Specifies which global var you are looking for
 @retval Value of global var

*/
uint32_t ecmdGetGlobalVar(ecmdGlobalVarType_t i_type);

/**
 @brief Set the value of some ecmdGlobalVars
 @param i_type Specifies which global var you are looking for
 @param i_value Value of global var
 @retval non-zero on failure

*/
uint32_t ecmdSetGlobalVar(ecmdGlobalVarType_t i_type, uint32_t i_value);

/**
 @brief Enable/Disable a trace mode
 @param i_type Specifies which trace mode to enable
 @param i_enable Enable or disable
*/
void ecmdSetTraceMode(ecmdTraceType_t i_type, bool i_enable);

/**
 @brief Query the state of a trace mode
 @param i_type Specifies which trace mode to query
 @retval Value of trace mode enable
*/
bool ecmdQueryTraceMode(ecmdTraceType_t i_type);

/** 
 @brief Function to delay a procedure either by running sim cycles or by doing a millisecond delay
 @param i_simCycles Number of sim cycles to run in simulation mode
 @param i_msDelay Number of milliseconds to delay in hardware mode
 @retval ECMD_SUCCESS on success
 @retval non-zero on failure
*/
uint32_t ecmdDelay(uint32_t i_simCycles, uint32_t i_msDelay);

#ifndef ECMD_REMOVE_SP_FUNCTIONS
/**
 @brief Make a query of the targeted Service Processor or Service Element
 @param i_target SP to query
 @param i_commandStr query type to make, LIST command will return supported queries
 @param o_result query result

 TARGET DEPTH : node<br>
 TARGET STATES : Unused

*/
uint32_t ecmdQuerySP(const ecmdChipTarget & i_target, std::string i_commandStr, std::string & o_result);

/**
 @brief Make a system call on the targetted Service Processor or Service Element
 @param i_target SP to run command on
 @param i_command Command line call to make
 @param o_stdout Standard out captured by running command

 TARGET DEPTH : node<br>
 TARGET STATES : Unused

*/
uint32_t makeSPSystemCall(const ecmdChipTarget & i_target, const std::string & i_command, std::string & o_stdout);
#endif // ECMD_REMOVE_SP_FUNCTIONS
  
/**
 @brief Synchronizes the plugin state information with the actual HW states
 This command is intended for situations where no automatic plugin re-sync takes place.  
 This is if a state change  (clock state / chip state) is forced directly via scom/ring accesses.
 @param i_target Target which needs to be updated. 

 TARGET DEPTH  : cage, node, slot, pos, chipUnit, thread<br>
 TARGET STATES : Must be Initialized<br>

*/
uint32_t syncPluginState(const ecmdChipTarget & i_target);

#ifndef ECMD_REMOVE_INIT_FUNCTIONS
/**
 @brief Synchronizes the ipl mode between fsp and
 *      hostboot b/w normal and debug modes

*/
uint32_t syncIplMode(int i_unused);
#endif // ECMD_REMOVE_INIT_FUNCTIONS

/**
 @brief Cleans up the chip to enable DA of memory after a checkstop
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval nonzero if unsuccessful
 @retval ECMD_RING_CACHE_ENABLED Ring Cache enabled function - must be disabled to use this function
 @param i_target Struct that contains cage information 
 @param i_mode The cleanup mode to run 


 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused<br>

*/
uint32_t ecmdChipCleanup(const ecmdChipTarget & i_target, uint32_t i_mode = ECMD_CHIP_CLEANUP_MEMDA);

//@}


/** @name Configuration Functions */
//@{

/**
 @brief Retrieve the value of a Configuration Setting
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_name Name of setting as defined by eCMD Api
 @param o_validOutput Indicator if o_valueAlpha, o_valueNumeric (or both) are valid.
 @param o_valueAlpha Alpha value of setting (if appropriate)
 @param o_valueNumeric Numeric value of setting (if appropriate)
 @retval ECMD_INVALID_CONFIG_NAME Name specified is not valid
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful

 TARGET DEPTH  : cage, node, slot, pos, chipUnit, thread (depending on i_name)<br>
 TARGET STATES : Unused<br>
*/

uint32_t ecmdGetConfiguration(const ecmdChipTarget & i_target, std::string i_name, ecmdConfigValid_t & o_validOutput, std::string & o_valueAlpha, uint32_t & o_valueNumeric);

/**
 @brief Retrieve the value of a Configuration Setting
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_name Name of setting as defined by eCMD Api
 @param o_configData Structure containing the valid data
 @retval ECMD_INVALID_CONFIG_NAME Name specified is not valid
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful

 TARGET DEPTH  : cage, node, slot, pos, chipUnit, thread (depending on i_name)<br>
 TARGET STATES : Unused<br>
*/
uint32_t ecmdGetConfigurationComplex(const ecmdChipTarget & i_target, std::string i_name, ecmdConfigData & o_configData);

/**
 @brief Set the value of a supported Configuration Setting like for example scanmode
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_name Name of setting as defined by eCMD Api - please see plugin documentation for list of supported strings
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_name Name of setting as defined by eCMD Api
 @param i_validInput Indicator if i_valueAlpha, i_valueNumeric (or both) are valid.
 @param i_valueAlpha Alpha value of setting (if appropriate)
 @param i_valueNumeric Numeric value of setting (if appropriate)
 @retval ECMD_DBUF_INVALID_DATA_FORMAT Value is not in correct format for specified configuration setting
 @retval ECMD_INVALID_CONFIG_NAME Name specified is not valid
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval nonzero on failure

 TARGET DEPTH  : cage, node, slot, pos, chipUnit, thread (depending on i_name)<br>
 TARGET STATES : Unused<br>
*/
uint32_t ecmdSetConfiguration(const ecmdChipTarget & i_target, std::string i_name, ecmdConfigValid_t i_validInput, std::string i_valueAlpha, uint32_t i_valueNumeric);

/**
 @brief Set the value of a supported Configuration Setting like for example scanmode
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_name Name of setting as defined by eCMD Api
 @param i_configData Structure containing the valid data
 @retval ECMD_INVALID_CONFIG_NAME Name specified is not valid
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful

 TARGET DEPTH  : cage, node, slot, pos, chipUnit, thread (depending on i_name)<br>
 TARGET STATES : Unused<br>
*/
uint32_t ecmdSetConfigurationComplex(const ecmdChipTarget & i_target, std::string i_name, ecmdConfigData i_configData);


/**
 @brief Deconfigure a target in the system
 @param i_target Target info to specify what to deconfigure (target states must be set)
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval nonzero on failure

 NOTE - lowest state that is valid is level that is deconfigured.<br>
   ex - if chipUnitState is VALID the chipUnit selected is deconfigured<br>
   ex - if chipUnitState is UNUSED and posState is VALID then the pos is deconfigured<br>

  This interface allows you to deconfigure all levels cages, nodes, slots, pos's, chipUnits<br>

 TARGET DEPTH  : cage, node, slot, pos, chipUnit<br>
 TARGET STATES : Must be Initialized<br>

*/
uint32_t ecmdDeconfigureTarget(const ecmdChipTarget & i_target);

/**
 @brief Configure a target in the system - must be previously known to the system
 @param i_target Target info to specify what to configure (target states must be set)
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system, or was not previously deconfigured
 @retval ECMD_SUCCESS if successful
 @retval nonzero on failure

 NOTE - lowest state that is valid is level that is configured.<br>
   ex - if chipUnitState is VALID the chipUnit selected is configured<br>
   ex - if chipUnitState is UNUSED and posState is VALID then the pos is configured<br>

  This interface allows you to configure all levels cages, nodes, slots, pos's, chipUnits<br>

 TARGET DEPTH  : cage, node, slot, pos, chipUnit<br>
 TARGET STATES : Must be Initialized<br>

*/
uint32_t ecmdConfigureTarget(const ecmdChipTarget & i_target);

#ifndef ECMD_REMOVE_UNITID_FUNCTIONS
/**
 *  @brief Converts an eCmd (physical) Target to a HOM Unit Id
 *  @param io_target an ecmdChipTarget struct representing a specific eCmd target
 *  @retval ECMD_SUCCESS if conversion successful
 *  @retval ECMD_INVALID_ARGS if unsuccessful in finding a matching Unit ID
 *  @post HOM Unit Ids in ecmdChipTarget struct are set and valid

 TARGET DEPTH  : cage, node, slot, pos, chipUnit, thread<br>
 TARGET STATES : Must be Initialized<br>

 */
uint32_t ecmdTargetToUnitId(ecmdChipTarget & io_target);

/**
 *  @brief Converts a Unit Id String to an eCmd (physical) Target
 *  @param i_unitId a string representing the name of a unitId
 *  @param o_targetList a list of targets that match the input unitId string
 *  @retval ECMD_SUCCESS if conversion successful
 *  @retval ECMD_INVALID_ARGS if unsuccessful in matching the string to a target
 *  @post There will be a list ecmdChipTargets that represent the passed in unitId string

 */
uint32_t ecmdUnitIdStringToTarget(std::string i_unitId, std::list<ecmdChipTarget> & o_targetList);

/**
 *  @brief Converts a Unit Id to an eCmd (physical) Target
 *  @param i_unitId a uint32_t representing an unitID 
 *  @param o_targetList a list of targets that match the unitId input 
 *  @retval ECMD_SUCCESS if conversion successful
 *  @retval ECMD_INVALID_ARGS if unsuccessful in matching the string to a target
 *  @post ecmdChipTarget Fields are set and represent the passed in unitId string
 */
uint32_t ecmdUnitIdToTarget(uint32_t i_unitId, std::list<ecmdChipTarget> & o_targetList);

/**
 *  @brief Converts a Unit Id into its String representation
 *  @param i_unitId a uint32_t representing an unitID 
 *  @param o_unitIdStr a string to match the unitId input 
 *  @retval ECMD_SUCCESS if conversion successful
 *  @retval ECMD_INVALID_ARGS if unsuccessful in matching the unitID to a String
 *  @post HOM Unit Id String is set and represents the passed in uint32_t unitId 
 */
uint32_t ecmdUnitIdToString(uint32_t i_unitId, std::string & o_unitIdStr);

/**
 *  @brief Gets Unit Id Version Number Used in the Dll
 *  @param o_unitIdVersion uint32_t representing the Unit Id Version Number
 *  @retval ECMD_SUCCESS if Unit Id Version Number retreived successfully
 *  @retval non-zero on failure

NOTE : o_unitIdVersion = 0xAAAABBCC, where 0xAAAA is reserved, 0xBB is major number, and 0xCC is minor number.<br>
NOTE:  Different Major Versions are NOT compatible.  Some Unit Ids might be missing between different minor versions.<br>
 */
uint32_t ecmdGetUnitIdVersion(uint32_t & o_unitIdVersion);

/**
 @brief Sequence ID of Cores and Threads converted to ecmdChipTarget struct
 @param i_core_seq_num Sequence ID number of the core 
 @param io_target ecmdChipTarget struct set to the result of the conversion
 @param i_thread_seq_num (OPTIONAL, default to 0) Sequence ID number of thread relative to the core parm
 @retval ECMD_INVALID_ARGS the inputs could not be mapped to an ecmdChipTarget struct
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful
 @pre io_target must have states defined to either core or thread level
 @pre io_target must have an acceptable processor chipType
 @post io_target fields are set accordingly

 TARGET DEPTH  : core, thread<br>
 TARGET STATES : Must be Initialized <br>

*/
uint32_t ecmdSequenceIdToTarget(uint32_t i_core_seq_num, ecmdChipTarget & io_target, uint32_t i_thread_seq_num=0);


/**
 @brief ecmdChipTarget struct converted to Sequence ID of Cores and Threads
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param o_core_seq_num Sequence ID number of the core 
 @param o_thread_seq_num Sequence ID number of thread relative to the core parm
 @retval ECMD_INVALID_ARGS the ecmdChipTarget inputs could not be mapped to a core and thread
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful
 @pre i_target must have states defined to either core or thread level
 @pre i_target must have an acceptable processor chipType
 @post o_core_seq_num set accordingly based on i_target
 @post if i_target depth is thread, o_thread_seq_num set to relative thread of o_core_seq_enum; else, o_thread_seq_num set to 0xFF

 TARGET DEPTH  : core, thread<br>
 TARGET STATES : Must be Initialized <br>

*/
uint32_t ecmdTargetToSequenceId(const ecmdChipTarget & i_target, uint32_t & o_core_seq_num, uint32_t & o_thread_seq_num);
#endif // ECMD_REMOVE_UNITID_FUNCTIONS
//@}



/** @name Module VPD Functions */
//@{
#ifndef ECMD_REMOVE_VPD_FUNCTIONS
/**
 @brief Read Module VPD Keyword Interface
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_record_name Name of VPD Record for given keyword
 @param i_keyword Name of VPD Keyword to Read
 @param i_bytes Byte length to read. 
 Note: If i_bytes > actual VPD keyword size: function will return all data of the actual VPD Keyword  .. No Error is flagged.  
 @param o_data Data buffer to copy data to
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_ARGS the inputs could not be mapped to a keyword
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>

*/
uint32_t getModuleVpdKeyword(const ecmdChipTarget & i_target, const char * i_record_name, const char * i_keyword, uint32_t i_bytes, ecmdDataBuffer & o_data);

/**
 @brief Write Module VPD Keyword Interface
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_record_name Name of VPD Record for given keyword
 @param i_keyword Name of VPD Keyword to Write
 @param i_data Data buffer of data to write
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_ARGS the inputs could not be mapped to a keyword
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>

*/
uint32_t putModuleVpdKeyword(const ecmdChipTarget & i_target, const char * i_record_name, const char * i_keyword, const ecmdDataBuffer & i_data);

/**
 @brief Read Module VPD Image Interface
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_bytes Byte length to read
 Note: If i_bytes > actual VPD Image size: function will return all data of the actual VPD Image .. No Error is flagged.  
 @param o_data Data buffer of data read from module
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_ARGS the inputs could not be mapped to a module
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>

*/
uint32_t getModuleVpdImage(const ecmdChipTarget & i_target, uint32_t i_bytes, ecmdDataBuffer & o_data);

/**
 @brief Write Module VPD Image Interface
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_data Data buffer of data to write
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_ARGS the inputs could not be mapped to a module
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @retval ECMD_DATA_UNDERFLOW  Too little data was provided to a write function
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>

*/
uint32_t putModuleVpdImage(const ecmdChipTarget & i_target, const ecmdDataBuffer & i_data);

/**
 @brief Get Module VPD Keyword From Image Interface
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_record_name Name of VPD Record for given keyword
 @param i_keyword Name of VPD Keyword to Read
 @param i_bytes Byte length to read
 Note: If i_bytes > actual VPD Keyword size: function will return all data of the actual VPD Keyword .. No Error is flagged.  
 @param i_image_data Data buffer containing VPD Image
 @param o_keyword_data Data buffer to copy keyword data to
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_ARGS the inputs could not be mapped to a module
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

NOTE : The HW is not accessed in this operation. Instead, the keyword data is pulled from the i_image_data ecmdDataBuffer.<br>
 
 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>

*/
uint32_t getModuleVpdKeywordFromImage(const ecmdChipTarget & i_target, const char * i_record_name, const char * i_keyword, uint32_t i_bytes, const ecmdDataBuffer & i_image_data, ecmdDataBuffer & o_keyword_data);

/**
 @brief Put Module VPD Keyword To An Image Interface
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_record_name Name of VPD Record for given keyword
 @param i_keyword Name of VPD Keyword to Read
 @param io_image_data Data buffer containing VPD Image that gets updated 
 @param i_keyword_data Data buffer containing keyword data
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_ARGS the inputs could not be mapped to a module
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @retval ECMD_DATA_UNDERFLOW  Too little data was provided to a write function
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

NOTE : The HW is not accessed in this operation. Instead, the keyword data in i_keyword_data is used to update the image data in io_image_data.<br>
 
 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>

*/
uint32_t putModuleVpdKeywordToImage(const ecmdChipTarget & i_target, const char * i_record_name, const char * i_keyword, ecmdDataBuffer & io_image_data, const ecmdDataBuffer & i_keyword_data);
#endif // ECMD_REMOVE_VPD_FUNCTIONS
//@}

/** @name FRU VPD Functions */
//@{
#ifndef ECMD_REMOVE_VPD_FUNCTIONS
/**
 @brief Read Fru VPD Keyword Interface
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_record_name Name of VPD Record for given keyword
 @param i_keyword Name of VPD Keyword to Read
 @param i_bytes Byte length to read
 Note: If if i_bytes > actual VPD Keyword size: function will return all data of the actual VPD Keyword .. No Error is flagged.  
 @param o_data Data buffer to copy data to
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_ARGS the inputs could not be mapped to a keyword
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>

*/
uint32_t getFruVpdKeyword(const ecmdChipTarget & i_target, const char * i_record_name, const char * i_keyword, uint32_t i_bytes, ecmdDataBuffer & o_data);

/**
 @brief Read Fru VPD Keyword Interface
 @param i_rid Number that specifies the resource to operate on
 @param i_record_name Name of VPD Record for given keyword
 @param i_keyword Name of VPD Keyword to Read
 @param i_bytes Byte length to read
 Note: If if i_bytes > actual VPD Keyword size: function will return all data of the actual VPD Keyword .. No Error is flagged.  
 @param o_data Data buffer to copy data to
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_ARGS the inputs could not be mapped to a keyword
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

*/
uint32_t getFruVpdKeywordWithRid(uint32_t i_rid, const char * i_record_name, const char * i_keyword, uint32_t i_bytes, ecmdDataBuffer & o_data);

/**
 @brief Write Fru VPD Keyword Interface
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_record_name Name of VPD Record for given keyword
 @param i_keyword Name of VPD Keyword to Write
 @param i_data Data buffer of data to write
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_ARGS the inputs could not be mapped to a keyword
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>

*/
uint32_t putFruVpdKeyword(const ecmdChipTarget & i_target, const char * i_record_name, const char * i_keyword, const ecmdDataBuffer & i_data);

/**
 @brief Write Fru VPD Keyword Interface
 @param i_rid Number that specifies the resource to operate on
 @param i_record_name Name of VPD Record for given keyword
 @param i_keyword Name of VPD Keyword to Write
 @param i_data Data buffer of data to write
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_ARGS the inputs could not be mapped to a keyword
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

*/
uint32_t putFruVpdKeywordWithRid(uint32_t i_rid, const char * i_record_name, const char * i_keyword, const ecmdDataBuffer & i_data);

/**
 @brief Read Fru VPD Image Interface
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_bytes Byte length to read
 Note: If if i_bytes > actual VPD Image size: function will return all data of the actual VPD Image .. No Error is flagged.  
 @param o_data Data buffer of data read from fru
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_ARGS the inputs could not be mapped to a fru
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>

*/
uint32_t getFruVpdImage(const ecmdChipTarget & i_target, uint32_t i_bytes, ecmdDataBuffer & o_data);

/**
 @brief Write Fru VPD Image Interface
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_data Data buffer of data to write
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_ARGS the inputs could not be mapped to a fru
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @retval ECMD_DATA_UNDERFLOW  Too little data was provided to a write function
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>

*/
uint32_t putFruVpdImage(const ecmdChipTarget & i_target, const ecmdDataBuffer & i_data);

/**
 @brief Get FRU VPD Keyword From Image Interface
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_record_name Name of VPD Record for given keyword
 @param i_keyword Name of VPD Keyword to Read
 @param i_bytes Byte length to read
 Note: If i_bytes > actual VPD Keyword size: function will return all data of the actual VPD Keyword .. No Error is flagged.  
 @param i_image_data Data buffer containing VPD Image
 @param o_keyword_data Data buffer to copy keyword data to
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_ARGS the inputs could not be mapped to a module
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

NOTE : The HW is not accessed in this operation. Instead, the keyword data is pulled from the i_image_data ecmdDataBuffer.<br>
 
 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>

*/
uint32_t getFruVpdKeywordFromImage(const ecmdChipTarget & i_target, const char * i_record_name, const char * i_keyword, uint32_t i_bytes, const ecmdDataBuffer & i_image_data, ecmdDataBuffer & o_keyword_data);

/**
 @brief Put FRU VPD Keyword To An Image Interface
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_record_name Name of VPD Record for given keyword
 @param i_keyword Name of VPD Keyword to Read
 @param io_image_data Data buffer containing VPD Image that gets updated 
 @param i_keyword_data Data buffer containing keyword data
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_INVALID_ARGS the inputs could not be mapped to a module
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @retval ECMD_DATA_UNDERFLOW  Too little data was provided to a write function
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

NOTE : The HW is not accessed in this operation. Instead, the keyword data in i_keyword_data is used to update the image data in io_image_data.<br>
 
 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>

*/
uint32_t putFruVpdKeywordToImage(const ecmdChipTarget & i_target, const char * i_record_name, const char * i_keyword, ecmdDataBuffer & io_image_data, const ecmdDataBuffer & i_keyword_data);
#endif // ECMD_REMOVE_VPD_FUNCTIONS
//@}


/** @name I2C Functions */
//@{
#ifndef ECMD_REMOVE_I2C_FUNCTIONS

/**
 @brief Resets the specified engine port
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_engineId I2c engine to use
 @param i_port I2C port to use
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_TARGET_INVALID_TYPE if target doesn't support I2c
 @retval ECMD_INVALID_ARGS Invalid argument values found
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>

*/
uint32_t ecmdI2cReset(const ecmdChipTarget & i_target, uint32_t i_engineId, uint32_t i_port);


/**
 @brief Read data from an I2C device
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_engineId I2c engine to use
 @param i_port I2C port to use
 @param i_slaveAddress I2C slave device address to use
 @param i_busSpeed I2C Bus speed to use
 @param i_bytes Byte length to read
 @param o_data Data read from device
 @param i_flags Bitmask of i2c flags
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_TARGET_INVALID_TYPE if target doesn't support I2c
 @retval ECMD_INVALID_ARGS Invalid argument values found
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>
*/
uint32_t ecmdI2cRead(const ecmdChipTarget & i_target, uint32_t i_engineId, uint32_t i_port, uint32_t i_slaveAddress, ecmdI2cBusSpeed_t i_busSpeed , uint32_t i_bytes, ecmdDataBuffer & o_data, uint32_t i_flags = 0);

/**
 @brief Read data from an I2C device at the given offset
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_engineId I2c engine to use
 @param i_port I2C port to use
 @param i_slaveAddress I2C slave device address to use
 @param i_busSpeed I2C Bus speed to use
 @param i_offset Byte offset in the device
 @param i_offsetFieldSize Specifies the field size used in the I2C protocol of the slave device
 @param i_bytes Byte length to read
 @param o_data Data read from device
 @param i_flags Bitmask of i2c flags
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_TARGET_INVALID_TYPE if target doesn't support I2c
 @retval ECMD_INVALID_ARGS Invalid argument values found
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>
*/
uint32_t ecmdI2cReadOffset(const ecmdChipTarget & i_target, uint32_t i_engineId, uint32_t i_port, uint32_t i_slaveAddress, ecmdI2cBusSpeed_t i_busSpeed , uint64_t i_offset, uint32_t i_offsetFieldSize, uint32_t i_bytes, ecmdDataBuffer & o_data, uint32_t i_flags = 0);

/**
 @brief Write the provided data into the I2C device
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_engineId I2c engine to use
 @param i_port I2C port to use
 @param i_slaveAddress I2C slave device address to use
 @param i_busSpeed I2C Bus speed to use
 @param i_data Data to write to device
 @param i_flags Bitmask of i2c flags
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_TARGET_INVALID_TYPE if target doesn't support I2c
 @retval ECMD_INVALID_ARGS Invalid argument values found
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>
*/
uint32_t ecmdI2cWrite(const ecmdChipTarget & i_target, uint32_t i_engineId, uint32_t i_port, uint32_t i_slaveAddress, ecmdI2cBusSpeed_t i_busSpeed , const ecmdDataBuffer & i_data, uint32_t i_flags = 0);

/**
 @brief Write the provided data into the I2C device at the given offset
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_engineId I2c engine to use
 @param i_port I2C port to use
 @param i_slaveAddress I2C slave device address to use
 @param i_busSpeed I2C Bus speed to use
 @param i_offset Byte offset in the device
 @param i_offsetFieldSize Specifies the field size used in the I2C protocol of the slave device
 @param i_data Data to write to device
 @param i_flags Bitmask of i2c flags
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_TARGET_INVALID_TYPE if target doesn't support I2c
 @retval ECMD_INVALID_ARGS Invalid argument values found
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>
*/
uint32_t ecmdI2cWriteOffset(const ecmdChipTarget & i_target, uint32_t i_engineId, uint32_t i_port, uint32_t i_slaveAddress, ecmdI2cBusSpeed_t i_busSpeed , uint64_t i_offset, uint32_t i_offsetFieldSize, const ecmdDataBuffer & i_data, uint32_t i_flags = 0);

/**
 @brief Let the user to run a series of command, while the i2c device is locked for the duration
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param io_cmdsEntries The series of commands to be run
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_TARGET_INVALID_TYPE if target doesn't support I2c
 @retval ECMD_INVALID_ARGS Invalid argument values found
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 NOTE : The i2c device targeted is locked at the first use of the device in the list and then unlocked when the command is complete.<br>
 NOTE : The output data on reads is filled into the ecmdDataBuffer in the list for that operation

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>
*/
uint32_t ecmdI2CMultipleCmds(const ecmdChipTarget & i_target, std::list<ecmdI2CCmdEntry> & io_cmdsEntries);
#endif /* ifndef ECMD_REMOVE_I2C_FUNCTIONS (this comment has to be here for makedll.pl to work */
//@}

/** @name GPIO Functions */
//@{
#ifndef ECMD_REMOVE_GPIO_FUNCTIONS
/**
 @brief Configures mode of pin
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_engineId Gpio engine to use
 @param i_pin Pin to use
 @param i_mode Mode to configure pin
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_TARGET_INVALID_TYPE if target doesn't support Gpio
 @retval ECMD_INVALID_ARGS Invalid argument values found
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 NOTE : Configuring a pin explicitly as output is not necessary since the write latch commands implicitly perform the required settings.<br>

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>

*/
uint32_t ecmdGpioConfigPin(const ecmdChipTarget & i_target, uint32_t i_engineId, uint32_t i_pin, ecmdDioMode_t i_mode);

/**
 @brief Read the state of the specified pin (0/1)
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_engineId Gpio engine to use
 @param i_pin Pin to use
 @param o_state State read on pin (0/1)
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_TARGET_INVALID_TYPE if target doesn't support Gpio
 @retval ECMD_INVALID_ARGS Invalid argument values found
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>

*/
uint32_t ecmdGpioReadPin(const ecmdChipTarget & i_target, uint32_t i_engineId, uint32_t i_pin, uint32_t & o_state);

/**
 @brief Read the state of the latch
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_engineId Gpio engine to use
 @param i_pin Pin to use
 @param i_mode Mode to configure pin
 @param o_state State read on pin (0/1)
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_TARGET_INVALID_TYPE if target doesn't support Gpio
 @retval ECMD_INVALID_ARGS Invalid argument values found
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>

*/
uint32_t ecmdGpioReadLatch(const ecmdChipTarget & i_target, uint32_t i_engineId, uint32_t i_pin, ecmdDioMode_t i_mode, uint32_t & o_state);

/**
 @brief Write value to the specified pin
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_engineId Gpio engine to use
 @param i_pin Pin to use
 @param i_mode Mode to configure pin
 @param i_state State to write to pin
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_TARGET_INVALID_TYPE if target doesn't support Gpio
 @retval ECMD_INVALID_ARGS Invalid argument values found
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>

*/
uint32_t ecmdGpioWriteLatch(const ecmdChipTarget & i_target, uint32_t i_engineId, uint32_t i_pin, ecmdDioMode_t i_mode, uint32_t i_state);

/**
 @brief Read the GPIO input register and AND with i_mask 
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_engineId Gpio engine to use
 @param i_mask Mask to apply to pins
 @param o_value Value read from pins with mask applied
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_TARGET_INVALID_TYPE if target doesn't support Gpio
 @retval ECMD_INVALID_ARGS Invalid argument values found
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>

*/
uint32_t ecmdGpioReadPins(const ecmdChipTarget & i_target, uint32_t i_engineId, uint32_t i_mask, uint32_t & o_value);

/**
 @brief Write several pins specified by i_mask
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_engineId Gpio engine to use
 @param i_mask Mask to apply to pins
 @param i_mode Mode to configure pin
 @param i_value Value to write to pins with mask applied
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_TARGET_INVALID_TYPE if target doesn't support Gpio
 @retval ECMD_INVALID_ARGS Invalid argument values found
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>

*/
uint32_t ecmdGpioWriteLatches(const ecmdChipTarget & i_target, uint32_t i_engineId, ecmdDioMode_t i_mode, uint32_t i_mask, uint32_t i_value);

/**
 @brief Provides read accesses to the GPIO/DIO configuration registers
 @param i_target Struct that contains cage/node/slot/position of device to access
 @param i_engineId Gpio engine to use
 @param i_configReg The config mode register to talk to 
 @param o_value The value returned by your read
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_TARGET_INVALID_TYPE if target doesn't support Gpio
 @retval ECMD_INVALID_ARGS Invalid argument values found
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>

*/
uint32_t ecmdGpioReadConfigRegister(const ecmdChipTarget & i_target, uint32_t i_engineId, uint32_t i_configReg, uint32_t & o_value);

/**
 @brief Provides write accesses to the GPIO/DIO configuration registers
 @param i_target Struct that contains cage/node/slot/position of device to access
 @param i_engineId Gpio engine to use
 @param i_mode The mode of write operation to do
 @param i_configReg The config mode register to talk to 
 @param i_value The value to set the specified config freg to
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_TARGET_INVALID_TYPE if target doesn't support Gpio
 @retval ECMD_INVALID_ARGS Invalid argument values found
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>

*/
uint32_t ecmdGpioWriteConfigRegister(const ecmdChipTarget & i_target, uint32_t i_engineId, ecmdGpioWriteMode_t i_mode, uint32_t i_configReg, uint32_t i_value);
#endif // ECMD_REMOVE_GPIO_FUNCTIONS
//@}

/** @name Power Functions */
//@{
#ifndef ECMD_REMOVE_POWER_FUNCTIONS
/**
 @brief Turns power on to the system
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 NOTE: This command does not start the system isteps, it only turns on power.

*/
uint32_t ecmdSystemPowerOn();

/**
 @brief Turns power off to the system
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

*/
uint32_t ecmdSystemPowerOff();

/**
 @brief Queries for the current System Power State
 @param o_state Enum that contains the current system power state
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

*/
uint32_t ecmdQuerySystemPower(ecmdPowerState_t & o_state);


/**
 @brief Turns power on to the given fru
 @param i_target Struct that contains cage/node/slot/position of device to access
 @param i_smart Dependent fru's will be powered on when smart is true
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : slot, pos<br>
 TARGET STATES : Unused <br>
*/
uint32_t ecmdFruPowerOn(const ecmdChipTarget & i_target, bool i_smart = false);

/**
 @brief Turns power off to the given fru
 @param i_target Struct that contains cage/node/slot/position of device to access
 @param i_smart Dependent fru's will be powered off when smart is true
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : slot, pos<br>
 TARGET STATES : Unused <br>
*/
uint32_t ecmdFruPowerOff(const ecmdChipTarget & i_target, bool i_smart = false);

/**
 @brief Queries for the current Fru Power State
 @param i_target Struct that contains cage/node/slot/position of device to access
 @param o_state Enum that contains the current fru power state
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

*/
uint32_t ecmdQueryFruPower(const ecmdChipTarget & i_target, ecmdPowerState_t & o_state);


/**
 @brief Sets the power mode of the target
 @param i_target Struct that contains cage/node/slot/position od device to access
 @param i_powerMode Enum that specifies which typ of powermode to set
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

*/
uint32_t ecmdSetPowerMode(const ecmdChipTarget & i_target, ecmdPowerMode_t i_powerMode);   


/**
 @brief Returns the power mode of the target
 @param i_target Struct that contains cage/node/slot/position od device to access
 @param o_powerMode Enum that specifies which typ of powermode to set
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

*/
uint32_t ecmdGetPowerMode(const ecmdChipTarget & i_target, ecmdPowerMode_t & o_powerMode);

/**
 @brief Provides the ability to bias a voltage on a per target basis.
 @param i_target Struct that contains cage/node/slot/position of device to access
 @param i_voltageLevel  Voltage level on the target we want to bias
 @param i_direction Direction the voltage command is to go
 @param i_biasValue The voltage bias value in 10 percent
 @param i_wait Whether or not to wait for the operation to complete before return.  Default is to wait.
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_TARGET_INVALID_TYPE if target doesn't support Gpio
 @retval ECMD_INVALID_ARGS Invalid argument values found
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : cage,node<br>
 TARGET STATES : Unused <br>
*/
uint32_t ecmdBiasVoltage(const ecmdChipTarget & i_target, uint32_t i_voltageLevel, ecmdVoltageType_t i_direction, uint32_t i_biasValue, bool i_wait = true);

/**
 @brief Provides the ability to query the bias state to see if the operation is complete or any are pending.
 @param i_target Struct that contains cage/node/slot/position of device to access
 @param i_voltageLevel  Voltage level on the target we want to bias
 @param o_currentVoltage The current voltage value
 @param o_targetVoltage  The voltage value we are trying to get to
 @param o_timeValue The time in ms until the operation completes
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_TARGET_INVALID_TYPE if target doesn't support Gpio
 @retval ECMD_INVALID_ARGS Invalid argument values found
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : cage,node<br>
 TARGET STATES : Unused <br>
*/
uint32_t ecmdQueryBiasState(const ecmdChipTarget & i_target, uint32_t i_voltageLevel, uint32_t & o_currentVoltage, uint32_t & o_targetVoltage, uint32_t & o_timeValue);
#endif // ECMD_REMOVE_POWER_FUNCTIONS
//@}

/** @name Adal Functions */
//@{
#ifndef ECMD_REMOVE_ADAL_FUNCTIONS
/**
 @brief Allow the user to call adal_psi_init
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_psiPort PSI Port to init
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_TARGET_INVALID_TYPE if target doesn't support PSI
 @retval ECMD_INVALID_ARGS Invalid argument values found
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>

*/
uint32_t ecmdAdalPsiInit(const ecmdChipTarget & i_target, uint32_t i_psiPort);

/**
 @brief Allow the user to call adal_psi_link_enable
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_psiPort     PSI Port to enable
 @param i_enableState 0 disables the link, 1 enables the link
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_TARGET_INVALID_TYPE if target doesn't support PSI
 @retval ECMD_INVALID_ARGS Invalid argument values found
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>

*/
uint32_t ecmdAdalPsiLinkEnable(const ecmdChipTarget & i_target, uint32_t i_psiPort, uint32_t i_enableState);

/**
 @brief Allow the user to call adal_psi_link_verify
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_psiPort     PSI Port to enable
 @param o_enableState The state returned by the call, possible values are:
 0x00 PSI link state is invalid
 0x01 PSI link is disabled and in an unconfigured state
 0x02 PSI link is in standby
 0x04 PSI link is active
 0x08 PSI link is disabled and in a failure state
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_TARGET_INVALID_TYPE if target doesn't support PSI
 @retval ECMD_INVALID_ARGS Invalid argument values found
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>

*/
uint32_t ecmdAdalPsiLinkVerify(const ecmdChipTarget & i_target, uint32_t i_psiPort, uint32_t & o_enableState);

/**
 @brief Allow the user to call adal_psi_set_speed
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_psiPort     PSI Port to enable
 @param i_speed The speed to pass to the adal.  Not all values are valid.  83 and 167 are known good.
 @param i_mode The mode to set it to.  0 = slow, !0 = fast
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_TARGET_INVALID_TYPE if target doesn't support PSI
 @retval ECMD_INVALID_ARGS Invalid argument values found
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>

*/
uint32_t ecmdAdalPsiSetSpeed(const ecmdChipTarget & i_target, uint32_t i_psiPort, uint32_t i_speed, uint32_t i_mode);

#endif // ECMD_REMOVE_ADAL_FUNCTIONS
//@}

/** @name Sensor Functions */
//@{
#ifndef ECMD_REMOVE_SENSOR_FUNCTIONS

/**
 @brief Read Thermal Sensor Values
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_ecmdThermalSensorId String that specifies the thermal sensor on the target ("THERMAL_DEFAULT" always supported)
 @param o_temperature Temperature value returned
 @param i_thermalUnit enum to control the return unit
 @param i_sensorReadMode enum to control the sensor read mode (ECMD_SENSOR_READ_MODE_DEFAULT always supported)
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Must be Initialized<br>

*/
uint32_t ecmdGetThermalSensor(const ecmdChipTarget & i_target, const char * i_ecmdThermalSensorId, uint32_t & o_temperature, ecmdThermalUnit_t i_thermalUnit=ECMD_THERMAL_UNIT_C, ecmdSensorReadMode_t i_sensorReadMode=ECMD_SENSOR_READ_MODE_DEFAULT);


/**
 @brief Read Clockspeed Sensor Values
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_ecmdClockSpeedSensorId String that specifies the Clockspeed sensor on the target ("CLOCKSPEEDL_DEFAULT" always supported)
 @param o_clockSpeed Clockspeed value returned
 @param i_clockspeedUnit enum to control the return unit
 @param i_sensorReadMode enum to control the sensor read mode (ECMD_SENSOR_READ_MODE_DEFAULT always supported)
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Must be Initialized<br>

*/
uint32_t ecmdGetClockSpeedSensor(const ecmdChipTarget & i_target, const char * i_ecmdClockSpeedSensorId, uint32_t & o_clockSpeed, ecmdClockSpeedType_t i_clockspeedUnit, ecmdSensorReadMode_t i_sensorReadMode=ECMD_SENSOR_READ_MODE_DEFAULT);


/**
 @brief Read Voltage Sensor Values
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_ecmdVoltageSensorId String that specifies the Voltage sensor on the target ("VOLTAGE_DEFAULT" always supported)
 @param o_voltage Voltage value returned
 @param i_voltageUnit enum to control the return unit
 @param i_sensorReadMode enum to control the sensor read mode (ECMD_SENSOR_READ_MODE_DEFAULT always supported)
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : cage, node, slot, pos<br>
 TARGET STATES : Must be Initialized<br>

*/
uint32_t ecmdGetVoltageSensor(const ecmdChipTarget & i_target, const char * i_ecmdVoltageSensorId, uint32_t & o_voltage, ecmdVoltageUnit_t i_voltageUnit=ECMD_VOLTAGE_UNIT_V, ecmdSensorReadMode_t i_sensorReadMode=ECMD_SENSOR_READ_MODE_DEFAULT);


/**
 @brief Read Current Sensor Values
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_ecmdCurrentSensorId String that specifies the Current sensor on the target ("CURRENT_DEFAULT" always supported)
 @param o_current Current value returned
 @param i_currentUnit enum to control the return unit
 @param i_sensorReadMode enum to control the sensor read mode (ECMD_SENSOR_READ_MODE_DEFAULT always supported)
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : cage, node, slot, pos<br>
 TARGET STATES : Must be Initialized<br>

*/
uint32_t ecmdGetCurrentSensor(const ecmdChipTarget & i_target, const char * i_ecmdCurrentSensorId, uint32_t & o_current, ecmdCurrentUnit_t i_currentUnit=ECMD_CURRENT_UNIT_A, ecmdSensorReadMode_t i_sensorReadMode=ECMD_SENSOR_READ_MODE_DEFAULT);

/**
 @brief Read Air Density Sensor Values
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_ecmdAirDensitySensorId String that specifies the Air Density sensor on the target ("AIRDENSITY_DEFAULT" always supported)
 @param o_airDensity Air Density value returned
 @param i_airDensityUnit enum to control the return unit
 @param i_sensorReadMode enum to control the sensor read mode (ECMD_SENSOR_READ_MODE_DEFAULT always supported)
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : cage, node<br>
 TARGET STATES : Must be Initialized<br>

*/
uint32_t ecmdGetAirDensitySensor(const ecmdChipTarget & i_target, const char * i_ecmdAirDensitySensorId, uint32_t & o_airDensity, ecmdAirDensityUnit_t i_airDensityUnit=ECMD_AIRDENSITY_UNIT_KG_M3, ecmdSensorReadMode_t i_sensorReadMode=ECMD_SENSOR_READ_MODE_DEFAULT);


/**
 @brief Read Humidity Sensor Values
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_ecmdHumiditySensorId String that specifies the Humidity sensor on the target ("HUMIDITY_DEFAULT" always supported)
 @param o_humidity Humidity value returned
 @param i_humidityUnit enum to control the return unit
 @param i_sensorReadMode enum to control the sensor read mode (ECMD_SENSOR_READ_MODE_DEFAULT always supported)
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : cage, node<br>
 TARGET STATES : Must be Initialized<br>

*/
uint32_t ecmdGetHumiditySensor(const ecmdChipTarget & i_target, const char * i_ecmdHumiditySensorId, uint32_t & o_humidity, ecmdHumidityUnit_t i_humidityUnit=ECMD_HUMIDITY_UNIT_G_M3, ecmdSensorReadMode_t i_sensorReadMode=ECMD_SENSOR_READ_MODE_DEFAULT);


/**
 @brief Read Power Sensor Values
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_ecmdPowerSensorId String that specifies the Power sensor on the target ("POWER_DEFAULT" always supported)
 @param o_power Power value returned
 @param i_powerUnit enum to control the return unit
 @param i_sensorReadMode enum to control the sensor read mode (ECMD_SENSOR_READ_MODE_DEFAULT always supported)
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : cage, node, slot, pos<br>
 TARGET STATES : Must be Initialized<br>

*/
uint32_t ecmdGetPowerSensor(const ecmdChipTarget & i_target, const char * i_ecmdPowerSensorId, uint32_t & o_power, ecmdPowerUnit_t i_powerUnit=ECMD_POWER_UNIT_mW, ecmdSensorReadMode_t i_sensorReadMode=ECMD_SENSOR_READ_MODE_DEFAULT);


/**
 @brief Read Utilization Sensor Values
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_ecmdUtilizationSensorId String that specifies the Power sensor on the target ("UTILIZATION_DEFAULT" always supported)
 @param o_utilization Utilization value returned
 @param i_utilizationUnit enum to control the return unit
 @param i_sensorReadMode enum to control the sensor read mode (ECMD_SENSOR_READ_MODE_DEFAULT always supported)
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Must be Initialized<br>

*/
uint32_t ecmdGetUtilizationSensor(const ecmdChipTarget & i_target, const char * i_ecmdUtilizationSensorId, uint32_t & o_utilization, ecmdUtilizationUnit_t i_utilizationUnit=ECMD_UTILIZATION_UNIT_P, ecmdSensorReadMode_t i_sensorReadMode=ECMD_SENSOR_READ_MODE_DEFAULT);

/**
 @brief Read FFDCBW Sensor Values
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_ecmdBandwidthSensorId String that specifies the Memory BandWidth sensor on the target
 @param o_bandwidth Memory BandWidth value returned
 @param i_BandwidthUnit enum to control the return unit
 @param i_sensorReadMode enum to control the sensor read mode (ECMD_SENSOR_READ_MODE_DEFAULT always supported)
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Must be Initialized<br>

*/
uint32_t ecmdGetBandwidthSensor(const ecmdChipTarget & i_target, const char * i_ecmdBandwidthSensorId, uint32_t & o_bandwidth, ecmdBandwidthUnit_t i_BandwidthUnit=ECMD_BW_UNIT_MRW, ecmdSensorReadMode_t i_sensorReadMode=ECMD_SENSOR_READ_MODE_DEFAULT);

/**
 @brief Read DTS Sensor Values
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_ecmdDTSSensorId String that specifies the type of DTS sensor 
 @param o_dtsReadings DTS values returned
 @param i_thermalUnit enum to control the return unit
 @param i_sensorReadMode enum to control the sensor read mode (ECMD_SENSOR_READ_MODE_DEFAULT always supported)
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Must be Initialized<br>

*/
uint32_t ecmdReadDigitalThermalSensor(const ecmdChipTarget & i_target, const char * i_ecmdDTSSensorId, ecmdDTSData & o_dtsReadings, ecmdThermalUnit_t i_thermalUnit, ecmdSensorReadMode_t i_sensorReadMode=ECMD_SENSOR_READ_MODE_DEFAULT);

#endif // ECMD_REMOVE_SENSOR_FUNCTIONS
//@}

#ifndef DOCUMENTATION
/** @name BackDoor Target Arg functions */
//@{
/**
 * @brief Push current arg state onto a stack so it can be restored
 NOTE : This is meant to be used by the command line extension where ecmdCommandArgs is called multiple times
  This causes the previous state of the target args to get lost, the command line extension will do a push/pop
  to save restore the state
*/
void ecmdPushCommandArgs();
/**
 * @brief Pop current arg state off the stack so it can be restored
*/
void ecmdPopCommandArgs();
//@}

/** @name Other BackDoor Functions */
//@{
#ifndef ECMD_REMOVE_LATCH_FUNCTIONS
/**
 @brief Find out if the JTAG or FSI column if the scandef should be used
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param o_mode Returns either ECMD_CHIPFLAG_JTAG or ECMD_CHIPFLAG_FSI
 @retval ECMD_SUCCESS on success
 @retval non-zero on failure
*/
uint32_t ecmdGetScandefOrder(const ecmdChipTarget & i_target, uint32_t & o_mode);
#endif // ECMD_REMOVE_LATCH_FUNCTIONS
  
/**
 @brief Get Current Cmdline String 
 @retval String representing current cmdline string being processed
*/
std::string ecmdGetCurrentCmdline();

/**
 @brief Convert list of Cmdline Args to String and Save in Dll 
 @param argc Command line arguments
 @param argv Command line arguments
*/
void ecmdSetCurrentCmdline(int argc, char* argv[]);

//@}
#endif

#ifndef ECMD_REMOVE_LATCH_FUNCTIONS
/** @name Scan Functions  */
//@{
/**
 @brief Reads the selected latches into the data buffer
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_SUCCESS if successful read
 @retval ECMD_UNABLE_TO_OPEN_SCANDEF eCMD was unable to open the scandef
 @retval ECMD_INVALID_RING if ringname is not valid for target
 @retval ECMD_INVALID_LATCHNAME if latchname not found in scandef
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_latchName Name of latch to read (can be a partial or full name based on i_mode)
 @param o_data list of Entries containing all latches found matching i_latchName
 @param i_ringName Name of ring to search for latch if == NULL, entire scandef is searched
 @param i_mode LatchName search mode (full or partial names)
 @param i_ring_mode Ring mode bit-field using ECMD_RING_MODE* defines.  Default value is 0. Use ECMD_RING_MODE_SPARSE_ACCESS for sparse ring access.

 NOTE : This function is ring cache enabled<br>
 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>
*/
uint32_t getLatch(const ecmdChipTarget & i_target, const char* i_ringName, const char * i_latchName, std::list<ecmdLatchEntry> & o_data, ecmdLatchMode_t i_mode, uint32_t i_ring_mode = 0);

/**
 @brief Writes the data buffer into the all latches matching i_latchName
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_UNABLE_TO_OPEN_SCANDEF eCMD was unable to open the scandef
 @retval ECMD_INVALID_RING if ringname is not valid for target
 @retval ECMD_INVALID_LATCHNAME if latchname not found in scandef
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @retval ECMD_DATA_UNDERFLOW  Too little data was provided to a write function
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_latchName Name of latch to write (can be a partial or full name based on i_mode)
 @param i_data DataBuffer object that holds data to write into latch
 @param i_ringName Name of ring to search for latch if == NULL, entire scandef is searched
 @param i_mode LatchName search mode
 @param i_ring_mode Ring mode bit-field using ECMD_RING_MODE* defines.  Default value is 0. Use ECMD_RING_MODE_SPARSE_ACCESS for sparse ring access.
 @param i_startBit Startbit in latchname to insert data
 @param i_numBits Number of bits to insert from startbit
 @param o_matches Number of latchs found that matched your name and data was inserted

 NOTE : This function is ring cache enabled<br>
 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>
*/
uint32_t putLatch(const ecmdChipTarget & i_target, const char* i_ringName, const char * i_latchName, const ecmdDataBuffer & i_data, uint32_t i_startBit, uint32_t i_numBits, uint32_t & o_matches, ecmdLatchMode_t i_mode, uint32_t i_ring_mode = 0);

/**
 @brief Reads the selected latch from the input ring image into the data buffer
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_SUCCESS if successful read
 @retval ECMD_UNABLE_TO_OPEN_SCANDEF eCMD was unable to open the scandef
 @retval ECMD_INVALID_RING if ringname is not valid for target
 @retval ECMD_INVALID_LATCHNAME if latchname not found in scandef
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_latchName Name of latch to read (can be a partial or full name based on i_mode)
 @param o_data list of Entries containing all latches found matching i_latchName
 @param i_ringName Name of ring to search for latch if == NULL, entire scandef is searched
 @param i_mode LatchName search mode (full or partial names)
 @param i_ringImage DataBuffer object that holds the ring image (Buffer must be of the correct length).

 NOTE : This function is ring cache enabled<br>
 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>
*/
uint32_t getLatchImage(const ecmdChipTarget & i_target, const char* i_ringName, const char * i_latchName, std::list<ecmdLatchEntry> & o_data, ecmdLatchMode_t i_mode, const ecmdDataBuffer & i_ringImage);

/**
 @brief Writes the data buffer into the all latches matching i_latchName within the provided ring image
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_UNABLE_TO_OPEN_SCANDEF eCMD was unable to open the scandef
 @retval ECMD_INVALID_RING if ringname is not valid for target
 @retval ECMD_INVALID_LATCHNAME if latchname not found in scandef
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @retval ECMD_DATA_UNDERFLOW  Too little data was provided to a write function
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_latchName Name of latch to write (can be a partial or full name based on i_mode)
 @param i_data DataBuffer object that holds data to write into latch
 @param i_ringName Name of ring to search for latch if == NULL, entire scandef is searched
 @param i_mode LatchName search mode
 @param i_startBit Startbit in latchname to insert data
 @param i_numBits Number of bits to insert from startbit
 @param o_matches Number of latchs found that matched your name and data was inserted
 @param io_ringImage DataBuffer object that holds the ring image (Buffer must be of the correct length).

 NOTE : This function is ring cache enabled<br>
 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>
*/
uint32_t putLatchImage(const ecmdChipTarget & i_target, const char* i_ringName, const char * i_latchName, const ecmdDataBuffer & i_data, uint32_t i_startBit, uint32_t i_numBits, uint32_t & o_matches, ecmdLatchMode_t i_mode, ecmdDataBuffer & io_ringImage);

/**
 @brief Creates an ecmdDataBuffer bit mask value of bits within the ring that are part of the requested latch
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval ECMD_UNABLE_TO_OPEN_SCANDEF eCMD was unable to open the scandef
 @retval ECMD_INVALID_RING if ringname is not valid for target
 @retval ECMD_INVALID_LATCHNAME if latchname not found in scandef
 @retval ECMD_INVALID_ARGS if only one of i_startBit and i_numBits are ECMD_UNSET
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_ringName Name of ring to search for latch if == NULL, entire scandef is searched
 @param i_latchName Name of latch to read (can be a partial or full name based on i_mode)
 @param o_mask bit mask of positions in the ring that are part of the input latch
 @param i_mode LatchName search mode (full or partial names) - Default is ECMD_LATCHMODE_FULL
 @param i_startBit Startbit in latchname to insert data - Default is ECMD_UNSET and causes i_startBit to be ignored.
 @param i_numBits Number of bits to insert from startbit - Default is ECMD_UNSET and causes i_numBits to be ignored.

 NOTE : The o_mask ecmdDataBuffer will be set by this method to the size of the ring that was passed in.
 NOTE : If passing in a value for i_startBit or i_numBits, both values need to be set.  
*/
uint32_t createSparseMaskFromLatch(const ecmdChipTarget & i_target, const char* i_ringName, const char * i_latchName, ecmdDataBuffer & o_mask, ecmdLatchMode_t i_mode = ECMD_LATCHMODE_FULL, uint32_t i_startBit = ECMD_UNSET, uint32_t i_numBits = ECMD_UNSET); 

/**
 @brief Reads the selected latches into the data buffer.  Uses latch information from i_scandefLatchInfo that is passed in to avoid another scandef lookup.
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_SUCCESS if successful read
 @retval ECMD_UNABLE_TO_OPEN_SCANDEF eCMD was unable to open the scandef
 @retval ECMD_INVALID_RING if ringname is not valid for target
 @retval ECMD_INVALID_LATCHNAME if latchname not found in scandef
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_scandefLatchInfo Latch information from scandef to avoid another scandef lookup.  Can be obtained by calling ecmdQueryLatchInfo and using the ecmdLatchQueryData->scandefLatchInfo member of the structure.
 @param o_data list of Entries containing all latches found matching i_latchName
 @param i_ring_mode Ring mode bit-field using ECMD_RING_MODE* defines.  Default value is 0. Use ECMD_RING_MODE_SPARSE_ACCESS for sparse ring access.

 NOTE : This function is ring cache enabled<br>
 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>
*/
uint32_t getLatchOpt(const ecmdChipTarget & i_target, const std::list<ecmdLatchEntry> & i_scandefLatchInfo, std::list<ecmdLatchEntry> & o_data, uint32_t i_ring_mode = 0);

/**
 @brief Writes the data buffer into the all latches matching i_latchName. Uses latch information from i_scandefLatchInfo that is passed in to avoid another scandef lookup.
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_UNABLE_TO_OPEN_SCANDEF eCMD was unable to open the scandef
 @retval ECMD_INVALID_RING if ringname is not valid for target
 @retval ECMD_INVALID_LATCHNAME if latchname not found in scandef
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @retval ECMD_DATA_UNDERFLOW  Too little data was provided to a write function
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_data DataBuffer object that holds data to write into latch
 @param i_scandefLatchInfo Latch information from scandef to avoid another scandef lookup.  Can be obtained by calling ecmdQueryLatchInfo and using the ecmdLatchQueryData->scandefLatchInfo member of the structure.
 @param i_ring_mode Ring mode bit-field using ECMD_RING_MODE* defines.  Default value is 0. Use ECMD_RING_MODE_SPARSE_ACCESS for sparse ring access.
 @param i_startBit Startbit in latchname to insert data
 @param i_numBits Number of bits to insert from startbit
 @param o_matches Number of latchs found that matched your name and data was inserted

 NOTE : This function is ring cache enabled<br>
 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>
*/
uint32_t putLatchOpt(const ecmdChipTarget & i_target, const ecmdDataBuffer & i_data, uint32_t i_startBit, uint32_t i_numBits, uint32_t & o_matches, const std::list<ecmdLatchEntry> & i_scandefLatchInfo, uint32_t i_ring_mode = 0);

/**
 @brief Reads the selected latch from the input ring image into the data buffer.  Uses latch information from i_scandefLatchInfo that is passed in to avoid another scandef lookup.
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_SUCCESS if successful read
 @retval ECMD_UNABLE_TO_OPEN_SCANDEF eCMD was unable to open the scandef
 @retval ECMD_INVALID_RING if ringname is not valid for target
 @retval ECMD_INVALID_LATCHNAME if latchname not found in scandef
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param o_data list of Entries containing all latches found matching i_latchName
 @param i_scandefLatchInfo Latch information from scandef to avoid another scandef lookup.  Can be obtained by calling ecmdQueryLatchInfo and using the ecmdLatchQueryData->scandefLatchInfo member of the structure.
 @param i_ringImage DataBuffer object that holds the ring image (Buffer must be of the correct length).

 NOTE : This function is ring cache enabled<br>
 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>
*/
uint32_t getLatchImageOpt(const ecmdChipTarget & i_target, std::list<ecmdLatchEntry> & o_data, const std::list<ecmdLatchEntry> & i_scandefLatchInfo, const ecmdDataBuffer & i_ringImage);

/**
 @brief Writes the data buffer into the all latches matching i_latchName within the provided ring image.  Uses latch information from i_scandefLatchInfo that is passed in to avoid another scandef lookup.
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval ECMD_CLOCKS_IN_INVALID_STATE Chip Clocks were in an invalid state to perform the operation
 @retval ECMD_UNABLE_TO_OPEN_SCANDEF eCMD was unable to open the scandef
 @retval ECMD_INVALID_RING if ringname is not valid for target
 @retval ECMD_INVALID_LATCHNAME if latchname not found in scandef
 @retval ECMD_DATA_OVERFLOW Too much data was provided for a write
 @retval ECMD_DATA_UNDERFLOW  Too little data was provided to a write function
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_data DataBuffer object that holds data to write into latch
 @param i_startBit Startbit in latchname to insert data
 @param i_numBits Number of bits to insert from startbit
 @param o_matches Number of latchs found that matched your name and data was inserted
 @param i_scandefLatchInfo Latch information from scandef to avoid another scandef lookup.  Can be obtained by calling ecmdQueryLatchInfo and using the ecmdLatchQueryData->scandefLatchInfo member of the structure.
 @param io_ringImage DataBuffer object that holds the ring image (Buffer must be of the correct length).

 NOTE : This function is ring cache enabled<br>
 TARGET DEPTH  : pos, chipUnit<br>
 TARGET STATES : Unused<br>
*/
uint32_t putLatchImageOpt(const ecmdChipTarget & i_target, const ecmdDataBuffer & i_data, uint32_t i_startBit, uint32_t i_numBits, uint32_t & o_matches, const std::list<ecmdLatchEntry> & i_scandefLatchInfo, ecmdDataBuffer & io_ringImage);

/**
 @brief Creates an ecmdDataBuffer bit mask value of bits within the ring that are part of the requested latch.  Uses latch information from i_scandefLatchInfo that is passed in to avoid another scandef lookup.
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful
 @retval ECMD_UNABLE_TO_OPEN_SCANDEF eCMD was unable to open the scandef
 @retval ECMD_INVALID_RING if ringname is not valid for target
 @retval ECMD_INVALID_LATCHNAME if latchname not found in scandef
 @retval ECMD_INVALID_ARGS if only one of i_startBit and i_numBits are ECMD_UNSET
 @retval nonzero if unsuccessful
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_scandefLatchInfo Latch information from scandef to avoid another scandef lookup.  Can be obtained by calling ecmdQueryLatchInfo and using the ecmdLatchQueryData->scandefLatchInfo member of the structure.
 @param io_mask bit mask of positions in the ring that are part of the input latch.  
 @param i_startBit Startbit in latchname to insert data - Default is ECMD_UNSET and causes i_startBit to be ignored.
 @param i_numBits Number of bits to insert from startbit - Default is ECMD_UNSET and causes i_numBits to be ignored.

 NOTE : The io_mask ecmdDataBuffer must be set to the length of the ring the latch is part of prior to calling this function.
 NOTE : If passing in a value for i_startBit or i_numBits, both values need to be set.  
*/
uint32_t createSparseMaskFromLatchOpt(const ecmdChipTarget & i_target, const std::list<ecmdLatchEntry> & i_scandefLatchInfo, ecmdDataBuffer & io_mask, uint32_t i_startBit = ECMD_UNSET, uint32_t i_numBits = ECMD_UNSET); 
//@}
#endif // ECMD_REMOVE_LATCH_FUNCTIONS


#ifndef ECMD_REMOVE_PNOR_FUNCTIONS
/** @name PNOR Functions  */
/**
 @brief Get pnor information from the dll
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_partitionName Name of the partition to read out
 @param i_partitionOffset Offset of the pnor image to read the header
 @param i_blockSize The block size to use when reading the pnor image
 @param o_data A ecmdDataBuffer that is populated with the image that was requested to be read
 @retval ECMD_SUCCESS if successful
 @retval nonzero on failure
*/
uint32_t ecmdGetPnor(const ecmdChipTarget & i_target, std::string i_partitionName, uint32_t i_partitionOffset, uint32_t i_blockSize, ecmdDataBuffer & o_data);

/**
 @brief Get pnor partition list from the dll
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_partitionName Name of the partition to pull information about
 @param i_partitionOffset Offset of the pnor image to read the header
 @param o_outData A ecmdPnorListData structure that contains the information about the pnor 
 @retval ECMD_SUCCESS if successful
 @retval nonzero on failure
*/
uint32_t ecmdGetPnorList(const ecmdChipTarget & i_target, std::string i_partitionName, uint32_t i_partitionOffset, ecmdPnorListData & o_outData);

/**
 @brief Put pnor information to the dll
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_partitionName Name of the partition to read out
 @param i_partitionOffset Offset of the pnor image to read the header
 @param i_blockSize The block size to use when reading the pnor image
 @param i_data A ecmdDataBuffer that contains the image that will be written to the pnor image specified by other parameter data
 @param i_flags A ECMD_PNOR_MODE* defines.  Default value ECMD_PNOR_MODE_DEFAULT to write the data in i_data.
 @retval ECMD_SUCCESS if successful
 @retval nonzero on failure
*/
uint32_t ecmdPutPnor(const ecmdChipTarget & i_target, std::string i_partitionName, uint32_t i_partitionOffset, uint32_t i_blockSize, const ecmdDataBuffer & i_data, uint32_t i_flags);
//@}
#endif // ECMD_REMOVE_PNOR_FUNCTIONS

/** @name SPI Functions */
//@{
#ifndef ECMD_REMOVE_SPI_FUNCTIONS
/**
 @brief Read data from a SPI device at the given address
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_engine SPI engine to use
 @param i_select SPI select to use
 @param i_address Address to read from on the device
 @param i_bytes Byte length to read
 @param i_mode Options to control behavior
 @param o_data Data read from device
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_TARGET_INVALID_TYPE if target doesn't support SPI
 @retval ECMD_INVALID_ARGS Invalid argument values found
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>
*/
uint32_t spiRead(const ecmdChipTarget & i_target, uint32_t i_engine, uint32_t i_select, uint64_t i_address, uint32_t i_bytes, uint32_t i_mode, ecmdDataBuffer & o_data);

/**
 @brief Write the provided data into the SPI device at the given address
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_engine SPI engine to use
 @param i_select SPI select to use
 @param i_address Address to write to on the device
 @param i_mode Options to control behavior
 @param i_data Data to write to device
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_TARGET_INVALID_TYPE if target doesn't support SPI
 @retval ECMD_INVALID_ARGS Invalid argument values found
 @retval ECMD_SUCCESS if successful
 @retval non-zero if unsuccessful

 TARGET DEPTH  : pos<br>
 TARGET STATES : Unused <br>
*/
uint32_t spiWrite(const ecmdChipTarget & i_target, uint32_t i_engine, uint32_t i_select, uint64_t i_address, uint32_t i_mode, const ecmdDataBuffer & i_data);

#endif /* ifndef ECMD_REMOVE_SPI_FUNCTIONS (this comment has to be here for makedll.pl to work */
//@}

/** @name Configuration Functions */
//@{
/**
 @brief Set the value of a supported Configuration Setting like for example scanmode
 @param i_target Struct that specifies the target to operate on (see target depth and states below)
 @param i_name Name of setting as defined by eCMD Api
 @param i_configData Structure containing the valid data
 @param i_mode Bit-field using ECMD_SETCONFIG_MODE* defines (see ecmdStructs.H) 
 @retval ECMD_INVALID_CONFIG_NAME Name specified is not valid
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful

 NOTE : i_mode is either ECMD_SETCONFIG_MODE_REPLACE, ECMD_SETCONFIG_MODE_ADD, or ECMD_SETCONFIG_MODE_REMOVE <br>
 NOTE : i_mode value will determine whether to add or remove the value in i_configData from the config setting, <br> 
        or replace the config setting entirely with the new value specified in i_configData <br>

 TARGET DEPTH  : cage, node, slot, pos, chipUnit, thread (depending on i_name)<br>
 TARGET STATES : Unused<br>
*/
uint32_t ecmdSetConfigurationComplexHidden(const ecmdChipTarget & i_target, std::string i_name, ecmdConfigData i_configData, uint32_t i_mode = ECMD_SETCONFIG_MODE_REPLACE);
//@}

#ifndef DOCUMENTATION
}  //extern "C"
#endif

#endif /* ecmdClientCapi_H */

#if! defined(ECMD_PERLAPI) && !defined(ECMD_PYAPI)
/*** -------------------------------------- ***/
/*** The following is doxygen documentation ***/
/*** -------------------------------------- ***/

/**
 * @mainpage

@section int Introduction

Common Hardware Access Programming Interface (eCMD)

This is the documentation of the eCMD C/C++ Programming Api

@section inc eCMD Core Include Files

To compile client code to use the C++ API, the following header files are required:
<ul>
<li> ecmdClientCapi.H
<li> ecmdDataBuffer.H
<li> ecmdStructs.H
<li> ecmdReturnCodes.H
<li> ecmdUtils.H
<li> ecmdSharedUtils.H
</ul>

@section link Link objects
To link the client code on AIX, the following is required:
<ul>
<li>libecmd.so
<li>xlC 9.0.0.0
</ul>

To link 64-bit client code on AIX, the following is required:
<ul>
<li>libecmd.so
<li>g++ 4.8.5
</ul>

To create Linux x86_64 binaries, the following is required:
<ul>
<li>libecmd.so
<li>g++ 4.8.1
</ul>

To create Linux ppc64le binaries, to following is required:
<ul>
<li>libecmd.so
<li>g++ 4.8.5
</ul>

<hr>
@section ext eCMD Extensions
Check ecmdExtCapi.H to see the supported extensions and how to use them

<hr>
@section dllv DLL Version
The eCMD Capi client code is built with a ECMD_CAPI_VERSION that gets passed into the DLL with the initDll function. If the version passed in does not match the version compiled into the DLL, the init will fail. The programmer needs to get a new copy of the .so library and rebuild their client to correct this problem.

<br>
<hr>
@section bvc The ecmdDataBuffer class
Data is passed between the client and the DLL with the ecmdDataBuffer class.  The ecmdDataBuffer object is linked on both the client side and the DLL side.

The ecmdDataBuffer maintains data both as unsigned integers and as a character string.  The class contains methods for accessing and modifying data as well as converting data to strings (e.g. hex, left-aligned).  The ecmdDataBuffer class allocates the memory for the conversion-to-string routines and returns a char* pointer to the memory.  The client should allocate its own memory and do a strcpy if the string is to be preserved upon the next ecmdDataBuffer conversion-to-string call.

<br>
<hr>
@section example Examples

<b>Makefile example for Linux x86_64 using cmd extension:</b>
\include makefile


<b>C/C++ Client Example</b>
\include ecmdclient.C



*/
#endif // ECMD_PERLAPI && ECMD_PYAPI
