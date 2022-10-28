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



//--------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------
#include <inttypes.h>
#include <stdio.h>

#include <ecmdDllCapi.H>
#include <ecmdStructs.H>
#include <ecmdReturnCodes.H>
#include <ecmdDataBuffer.H>
#ifdef ECMD_FAPI2
#include <fapi2DllCapi.H>
#endif

//--------------------------------------------------------------------
//  Forward References                                                
//--------------------------------------------------------------------

//--------------------------------------------------------------------
//  Function Definitions                                               
//--------------------------------------------------------------------

// For use by dllQueryConfig and dllQueryExist
uint32_t queryConfigExist(const ecmdChipTarget & i_target, ecmdQueryData & o_queryData, ecmdQueryDetail_t i_detail, bool i_allowDisabled);
uint32_t queryConfigExistCages(const ecmdChipTarget & i_target, std::list<ecmdCageData> & o_cageData, ecmdQueryDetail_t i_detail, bool i_allowDisabled);
uint32_t queryConfigExistNodes(const ecmdChipTarget & i_target, std::list<ecmdNodeData> & o_nodeData, ecmdQueryDetail_t i_detail, bool i_allowDisabled);
uint32_t queryConfigExistSlots(const ecmdChipTarget & i_target, std::list<ecmdSlotData> & o_slotData, ecmdQueryDetail_t i_detail, bool i_allowDisabled);
uint32_t queryConfigExistChips(const ecmdChipTarget & i_target, std::list<ecmdChipData> & o_chipData, ecmdQueryDetail_t i_detail, bool i_allowDisabled);
uint32_t queryConfigExistChipUnits(const ecmdChipTarget & i_target, uint32_t & i_pos, std::list<ecmdChipUnitData> & o_chipUnitData, ecmdQueryDetail_t i_detail, bool i_allowDisabled);
uint32_t queryConfigExistThreads(const ecmdChipTarget & i_target, std::list<ecmdThreadData> & o_threadData, ecmdQueryDetail_t i_detail, bool i_allowDisabled);

//--------------------------------------------------------------------
//  Function Definitions                                               
//--------------------------------------------------------------------
//
//   These are just stubs, used for testing the out the DLL

uint32_t dllInitDll() {
  /* This is where we would init any local variables to the dll */
  return ECMD_SUCCESS;
}

uint32_t dllFreeDll() {
  return ECMD_SUCCESS;
}

uint32_t dllSpecificCommandArgs(int* , char** []) {
  return ECMD_SUCCESS;
}

/* Dll Specific Return Codes */
std::string dllSpecificParseReturnCode(uint32_t ) {
  return ""; 
}

uint32_t dllGetRingSparse(const ecmdChipTarget & , const char * , ecmdDataBuffer & , const ecmdDataBuffer & , uint32_t ) 
{ 
    return ECMD_SUCCESS; 
} 

uint32_t dllPutRingSparse(const ecmdChipTarget & , const char * , const ecmdDataBuffer & , const ecmdDataBuffer & , uint32_t ) 
{ 
    return ECMD_SUCCESS; 
} 

uint32_t dllGetRing(const ecmdChipTarget & , const char * , ecmdDataBuffer & , uint32_t )
{ 
    return ECMD_SUCCESS; 
} 

uint32_t dllPutRing(const ecmdChipTarget & , const char * , const ecmdDataBuffer & , uint32_t ) 
{ 
    return ECMD_SUCCESS; 
} 

uint32_t dllGetScom (const ecmdChipTarget & , uint64_t , ecmdDataBuffer & ) {
  return ECMD_SUCCESS;
}

uint32_t dllPutScom (const ecmdChipTarget & , uint64_t , const ecmdDataBuffer & ) {
  return ECMD_SUCCESS;
}

/* ##################################################################### */
/* Query Functions - Query Functions - Query Functions - Query Functions */
/* ##################################################################### */
uint32_t dllQueryConfig(const ecmdChipTarget & i_target, ecmdQueryData & o_queryData, ecmdQueryDetail_t i_detail) {
  return queryConfigExist(i_target, o_queryData, i_detail, false);
}

uint32_t dllQueryExist(const ecmdChipTarget & i_target, ecmdQueryData & o_queryData, ecmdQueryDetail_t i_detail) {
  return queryConfigExist(i_target, o_queryData, i_detail, true);
}

uint32_t queryConfigExist(const ecmdChipTarget & i_target, ecmdQueryData & o_queryData, ecmdQueryDetail_t i_detail, bool i_allowDisabled) {
  uint32_t rc = ECMD_SUCCESS;

  // Need to clear out the queryConfig data before pushing stuff in
  // This is in case there is stale data in there
  o_queryData.cageData.clear();

  // From here, we will recursively work our way through all levels of hierarchy in the target
  if (i_target.cageState == ECMD_TARGET_FIELD_VALID || i_target.cageState == ECMD_TARGET_FIELD_WILDCARD) {
    rc = queryConfigExistCages(i_target, o_queryData.cageData, i_detail, i_allowDisabled);
    if (rc) return rc;
  }

  return rc;
}

uint32_t queryConfigExistCages(const ecmdChipTarget & i_target, std::list<ecmdCageData> & o_cageData, ecmdQueryDetail_t i_detail, bool i_allowDisabled) {
  uint32_t rc = ECMD_SUCCESS;
  ecmdCageData cageData;

  // We only have 1 cage for this example, create that data
  // Then walk down through our nodes
  cageData.cageId = 0;

  // If the node states are set, see what nodes are in this cage
  if (i_target.nodeState == ECMD_TARGET_FIELD_VALID || i_target.nodeState == ECMD_TARGET_FIELD_WILDCARD) {
    rc = queryConfigExistNodes(i_target, cageData.nodeData, i_detail, i_allowDisabled);
    if (rc) return rc;
  }

  // Save what we got from recursing down, or just being happy at this level
  o_cageData.push_back(cageData);

  return rc;
}

uint32_t queryConfigExistNodes(const ecmdChipTarget & i_target, std::list<ecmdNodeData> & o_nodeData, ecmdQueryDetail_t i_detail, bool i_allowDisabled)  {
  uint32_t rc = ECMD_SUCCESS;
  ecmdNodeData nodeData;

  // We only have 1 node for example, create that data
  // Then walk down through our slots
  nodeData.nodeId = 0;

  // If the slot states are set, see what slots are in this node
  if (i_target.slotState == ECMD_TARGET_FIELD_VALID || i_target.slotState == ECMD_TARGET_FIELD_WILDCARD) {
    rc = queryConfigExistSlots(i_target, nodeData.slotData, i_detail, i_allowDisabled);
    if (rc) return rc;
  }

  // Save what we got from recursing down, or just being happy at this level
  o_nodeData.push_back(nodeData);

  return rc;
}

uint32_t queryConfigExistSlots(const ecmdChipTarget & i_target, std::list<ecmdSlotData> & o_slotData, ecmdQueryDetail_t i_detail, bool i_allowDisabled)  {
  uint32_t rc = ECMD_SUCCESS;
  ecmdSlotData slotData;

  // We only have 1 slot for example, create that data
  // Then walk down through our chips
  slotData.slotId = 0;

  // If the chipType states are set, see what chipTypes are in this slot
  if (i_target.chipTypeState == ECMD_TARGET_FIELD_VALID || i_target.chipTypeState == ECMD_TARGET_FIELD_WILDCARD) {
    rc = queryConfigExistChips(i_target, slotData.chipData, i_detail, i_allowDisabled);
    if (rc) return rc;
  }

  // Save what we got from recursing down, or just being happy at this level
  o_slotData.push_back(slotData);

  return rc;
}

uint32_t queryConfigExistChips(const ecmdChipTarget & i_target, std::list<ecmdChipData> & o_chipData, ecmdQueryDetail_t i_detail, bool i_allowDisabled)  {
  uint32_t rc = ECMD_SUCCESS;
  ecmdChipData chipData;

  // Create a sample with 4 positions
  for (uint32_t pos = 0; pos < 4; pos++) {

    // If posState is set to VALID, check that our values match
    // If posState is set to WILDCARD, we don't care
    if ((i_target.posState == ECMD_TARGET_FIELD_VALID) && (pos != i_target.pos))
      continue;

    // We passed our checks, load up our data
    chipData.chipUnitData.clear();
    chipData.chipType = "pu";
    chipData.pos = pos;

    // If the chipUnitType states are set, see what chipUnitTypes are in this chipType
    if (i_target.chipUnitTypeState == ECMD_TARGET_FIELD_VALID ||
        i_target.chipUnitTypeState == ECMD_TARGET_FIELD_WILDCARD) {

      // Look for chipunits
      rc = queryConfigExistChipUnits(i_target, pos, chipData.chipUnitData, i_detail, i_allowDisabled);
      if (rc) return rc;
    }

    // Save what we got from recursing down, or just being happy at this level
    o_chipData.push_back(chipData);
  }

  return rc;
}

uint32_t queryConfigExistChipUnits(const ecmdChipTarget & i_target, uint32_t & i_pos, std::list<ecmdChipUnitData> & o_chipUnitData, ecmdQueryDetail_t i_detail, bool i_allowDisabled)  {
  uint32_t rc = ECMD_SUCCESS;
  ecmdChipUnitData chipUnitData;

  // Positions 2 and 3 in our example won't have chip units, so bail
  if (i_pos == 2 || i_pos == 3) {
    return 0;
  }

  // Dummy up data for pos 0 and 1
  // This code would normally be much cleaner in a full featured plugin as you would be
  // generically querying the backend to find out what was and wasn't available
  // It would look very much like the levels up, instead we dummy up data here
  if (i_pos == 0 || i_pos == 1) {
    // Properly check if the ex chipunit was asked for and then create it
    bool addExUnit = false;
    if (i_target.chipUnitTypeState == ECMD_TARGET_FIELD_WILDCARD) {
      addExUnit = true;
    }
    if (i_target.chipUnitTypeState == ECMD_TARGET_FIELD_VALID &&
        i_target.chipUnitType == "ex") {
      addExUnit = true;
    }

    if (addExUnit) {
      if (i_target.chipUnitNumState == ECMD_TARGET_FIELD_WILDCARD ||
          i_target.chipUnitNumState == ECMD_TARGET_FIELD_VALID) {
        for (uint32_t exNum = 0; exNum < 24; exNum++) {
          if (i_target.chipUnitNumState == ECMD_TARGET_FIELD_VALID &&
              i_target.chipUnitNum != exNum) {
            // Not the requested chipunitnum, skip
            continue;
          }
          chipUnitData.threadData.clear();
          chipUnitData.chipUnitType = "ex";
          chipUnitData.chipUnitNum = exNum;
          chipUnitData.numThreads = 4;

          // If the thread states are set, see what thread are in this chipUnit
          if (i_target.threadState == ECMD_TARGET_FIELD_VALID ||
              i_target.threadState == ECMD_TARGET_FIELD_WILDCARD) {
            // Look for threads
            rc = queryConfigExistThreads(i_target, chipUnitData.threadData, i_detail, i_allowDisabled);
            if (rc) return rc;
          }

          o_chipUnitData.push_back(chipUnitData);
        }
      }
    }

    // Add a second non thread level chipunit, just on p0
    if (i_pos == 0) {
      if (i_target.chipUnitTypeState == ECMD_TARGET_FIELD_WILDCARD ||
          (i_target.chipUnitTypeState == ECMD_TARGET_FIELD_VALID &&
           i_target.chipUnitType == "phb")) {

        for (uint32_t phbNum = 0; phbNum < 5; phbNum++) {
          if (i_target.chipUnitNumState == ECMD_TARGET_FIELD_VALID &&
              i_target.chipUnitNum != phbNum) {
            // Not the requested chipunitnum, skip
            continue;
          }
          chipUnitData.threadData.clear();
          chipUnitData.chipUnitType = "phb";
          chipUnitData.chipUnitNum = phbNum;
          chipUnitData.numThreads = 0;

          // No threads here, just add it
          o_chipUnitData.push_back(chipUnitData);
        }
      }
    }
  }
  
  return rc;
}

uint32_t queryConfigExistThreads(const ecmdChipTarget & , std::list<ecmdThreadData> & o_threadData, ecmdQueryDetail_t , bool ) {
  uint32_t rc = ECMD_SUCCESS;
  ecmdThreadData threadData;

  for (uint32_t thread = 0; thread < 4; thread++) {
    threadData.threadId = thread;

    o_threadData.push_back(threadData);
  }

  return rc;
}

uint32_t dllQueryRing(const ecmdChipTarget & , std::list<ecmdRingData> & o_queryData, const char * , ecmdQueryDetail_t ) {

  ecmdRingData ringData;

  ringData.ringNames.push_back("ring1");
  ringData.address = 0x80000001;
  ringData.bitLength = 100;
  ringData.isCheckable = false;
  ringData.clockState = ECMD_CLOCKSTATE_ON;

  o_queryData.push_back(ringData);

  ringData.ringNames.clear();
  ringData.ringNames.push_back("ring2");
  ringData.ringNames.push_back("ring2long");
  ringData.address = 0x80000002;
  ringData.bitLength = 2000;
  ringData.isCheckable = true;
  ringData.clockState = ECMD_CLOCKSTATE_NA;

  o_queryData.push_back(ringData);

  return ECMD_SUCCESS;
}

uint32_t dllQueryArray(const ecmdChipTarget & , ecmdArrayData & , const char * ) {
  return ECMD_SUCCESS;
} 

uint32_t dllQueryFileLocation(const ecmdChipTarget & , ecmdFileType_t , std::list<ecmdFileLocation> & o_fileLocations, std::string & io_version) {
  o_fileLocations.push_back((ecmdFileLocation){ "bogus/file.text", "bogus/file.hash" });
  io_version = "v8";
  return ECMD_SUCCESS;
} 

uint32_t dllFlushSys () {
  return ECMD_SUCCESS;
} 

uint32_t dllIplSys () {
  return ECMD_SUCCESS;
}

void dllOutputError(const char* message) {
  printf("DLLSTUBERROR : %s\n",message);
}

void dllOutputWarning(const char* message) {
  printf("DLLSTUBWARNING : %s\n",message);
}

void dllOutput(const char* message) {
  printf("%s", message);
}

uint32_t dllGetChipData(const ecmdChipTarget & , ecmdChipData & ) {
  return ECMD_SUCCESS;
}

uint32_t dllEnableRingCache(const ecmdChipTarget & ) {
  return ECMD_SUCCESS;
}

uint32_t dllDisableRingCache(const ecmdChipTarget & ) {
  return ECMD_SUCCESS;
}

uint32_t dllFlushRingCache(const ecmdChipTarget & ) {
  return ECMD_SUCCESS;
}

bool dllIsRingCacheEnabled(const ecmdChipTarget & ) {
  return false;
}

uint32_t dllGetArray(const ecmdChipTarget & , const char * , const ecmdDataBuffer & , ecmdDataBuffer & , uint32_t ) {
  return ECMD_SUCCESS;
}

uint32_t dllPutArray(const ecmdChipTarget & , const char * , const ecmdDataBuffer & , const ecmdDataBuffer & ) {
  return ECMD_SUCCESS;
}

/* ################################################################# */
/* Misc Functions - Misc Functions - Misc Functions - Misc Functions */
/* ################################################################# */
uint32_t dllGetScandefOrder(const ecmdChipTarget & i_target, uint32_t & o_mode) {
  uint32_t rc = ECMD_SUCCESS;
  ecmdChipData chipData;

  /* Just get the chip data, we get extra data but this has never been a performance problem for us */
  rc = dllGetChipData(i_target, chipData);
  if (rc) return rc;

  o_mode = chipData.chipFlags & ECMD_CHIPFLAG_BUSMASK;

  return rc;
}

void dllSetTraceMode(ecmdTraceType_t , bool ) {
  dllOutputError("Not supported: dllSetTraceMode");
}

bool dllQueryTraceMode(ecmdTraceType_t ) {
  return false;
}

/* ######################################################################### */
/* UnitID Functions - UnitID Functions - UnitID Functions - UnitID Functions */
/* ######################################################################### */
/* Added to fix symbol errors - JTA 06/28/14 */
uint32_t dllTargetToUnitId(ecmdChipTarget & ) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}

uint32_t dllUnitIdStringToTarget(std::string , std::list<ecmdChipTarget> & ) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}

uint32_t dllUnitIdToTarget(uint32_t , std::list<ecmdChipTarget> & ) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}

uint32_t dllUnitIdToString(uint32_t , std::string & ) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}

uint32_t dllSequenceIdToTarget(uint32_t , ecmdChipTarget & , uint32_t ) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}

uint32_t dllTargetToSequenceId(const ecmdChipTarget & , uint32_t & , uint32_t & ) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}

uint32_t dllGetUnitIdVersion(uint32_t & o_unitIdVersion) {
  uint32_t rc = ECMD_SUCCESS;
  o_unitIdVersion = 0x10002030;
  return rc;
}

uint32_t dllQueryClockState(const ecmdChipTarget & , const char *, ecmdClockState_t &) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}

uint32_t dllRelatedTargets(const ecmdChipTarget & i_target, const std::string , std::list<ecmdChipTarget> & o_relatedTargets, const ecmdLoopMode_t ) {
  o_relatedTargets.push_back(i_target);
  return ECMD_SUCCESS;
}

#ifdef ECMD_FAPI2
uint32_t dllFapi2AttributeStringToId(std::string i_attrString, fapi2::AttributeId &o_attrId) {
  if (i_attrString != "ATTR_CHIP_ID")
    return 0x0206005A; // FAPI_UNSUPPORTED_ATTRIBUTE
  o_attrId = fapi2::ATTR_CHIP_ID;
  return ECMD_SUCCESS;
}

uint32_t dllFapi2GetAttribute (const ecmdChipTarget &i_target, const uint32_t i_id, fapi2::AttributeData &o_data)
{
  o_data.faUint32 = 42;
  return ECMD_SUCCESS;
}

uint32_t dllFapi2SetAttribute (const ecmdChipTarget &i_target, const uint32_t i_id, fapi2::AttributeData &i_data)
{
  return ECMD_SUCCESS;
}

uint32_t dllFapi2GetAttrInfo (fapi2::AttributeId i_attrId, uint32_t &o_attrType, uint32_t &o_numOfEntries, uint32_t &o_numOfBytes, bool &o_attrEnum)
{
  o_attrType = FAPI_ATTRIBUTE_TYPE_UINT32;
  o_numOfEntries = 1;
  o_numOfBytes = 4;
  o_attrEnum = false;
  return ECMD_SUCCESS;
}
#endif // ECMD_FAPI2
