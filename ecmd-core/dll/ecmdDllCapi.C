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


//lint -e825 We deliberately want to fall through in ecmdIncrementLooperIterators - JTA

//----------------------------------------------------------------------
//  Includes
//----------------------------------------------------------------------
#include <fstream>
#include <stdio.h>
#include <list>
#include <algorithm>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h> /* for htonl */
#include <pthread.h>
#include <map>

#include <ecmdDllCapi.H>
#include <ecmdStructs.H>
#include <ecmdSharedUtils.H>

#ifndef _AIX
  #include <byteswap.h>
  #ifndef htonll
    #if BYTE_ORDER == BIG_ENDIAN
      #define htonll(x) (x)
    #else
      #define htonll(x) bswap_64(x)
    #endif
  #endif
#endif

//----------------------------------------------------------------------
//  User Types
//----------------------------------------------------------------------
/** @brief Used to hold info out of the scandef for get/putlatch etc. */
#ifndef ECMD_REMOVE_LATCH_FUNCTIONS
/** @brief Used to hold latchname keys and offsets into the scandef **/
struct ecmdLatchHashInfo {
  //  uint32_t latchHashKey;                 ///< HashKey for the Latchname in Uppercase
  uint32_t latchOffset;                       ///< Offset for the latchname in the scandef file
  uint32_t ringBeginOffset;                   ///< Begin Offset for the latch Ring in the scandef file
  uint32_t ringEndOffset;                     ///< End Offset for the latch Ring in the scandef file
  bool ringFound;
};


/** @brief Used to buffer scandef data to avoid searching for each chip ec */
struct ecmdLatchBufferEntry {
  std::list<ecmdLatchEntry> entry;       ///< Data from Scandef
  std::string latchName;                 ///< Latch name used to search for this data
  uint64_t    latchNameHashKey;          ///< Latch name used to search for this data
  std::string ringName;                  ///< Ring name used to search for this data (empty string if == NULL)

  inline int operator==(const ecmdLatchBufferEntry &rhs) const {
    return (latchNameHashKey == rhs.latchNameHashKey);
  };

  inline int operator<(const ecmdLatchBufferEntry &rhs) const {
    return (latchNameHashKey < rhs.latchNameHashKey);
  };
};

struct ecmdLatchCacheEntry {
  uint64_t scandefHashKey;
    std::map<uint64_t, ecmdLatchBufferEntry> latches;

  inline int operator==(const ecmdLatchCacheEntry &rhs) const {
    return (scandefHashKey==rhs.scandefHashKey);
  };

};

/* @brief Used by get/putlatch to buffer scandef entries in memory to improve performance */
std::list<ecmdLatchCacheEntry> latchCache;
pthread_mutex_t latchCacheMutex = PTHREAD_MUTEX_INITIALIZER;

class LockGuard
{
    public:
    LockGuard(pthread_mutex_t * i_mutex) : iv_mutex(i_mutex)
    {
        pthread_mutex_lock(iv_mutex);
    }
    ~LockGuard()
    {
        pthread_mutex_unlock(iv_mutex);
    }
    private:
    pthread_mutex_t * iv_mutex;
};


/** @brief Used to sort latch entries from the scandef */
bool operator<(const ecmdLatchEntry & lhs, const ecmdLatchEntry & rhs) {

  if (lhs.ringName != rhs.ringName)
    return lhs.ringName < rhs.ringName;

  size_t lhsLeftParen = lhs.latchName.find_last_of('(');
  size_t rhsLeftParen = rhs.latchName.find_last_of('(');

  if (lhsLeftParen == std::string::npos || rhsLeftParen == std::string::npos || lhsLeftParen != rhsLeftParen) {
    return lhs.latchName < rhs.latchName;
  }

  std::string lhsSub = lhs.latchName.substr(0, lhsLeftParen);
  std::string rhsSub = rhs.latchName.substr(0, rhsLeftParen);

  if (lhsSub != rhsSub) {
    return lhs.latchName < rhs.latchName;
  }

  return lhs.latchStartBit < rhs.latchStartBit;
}

/** @brief Used to sort latch entries from the scandef */
bool operator!= (const ecmdLatchEntry & lhs, const ecmdLatchEntry & rhs) {

  if (lhs.ringName != rhs.ringName)
    return true;

  uint32_t lhsLeftParen = lhs.latchName.find_last_of('(');
  uint32_t rhsLeftParen = rhs.latchName.find_last_of('(');

  if (lhsLeftParen != rhsLeftParen) {
    return true;
  }

  return (lhs.latchName.substr(0, lhsLeftParen) != rhs.latchName.substr(0,rhsLeftParen));
}
#endif // ECMD_REMOVE_LATCH_FUNCTIONS


/**
 * @brief Used for storing error messages internally to dll
 */
struct ecmdErrorMsg {
  uint32_t returnCode;          ///< Numeric error code- see ecmdReturnCodes.H
  std::string whom;             ///< Function that registered error
  std::string message;          ///< Message about the error
  bool accessed;                ///< This message has been accessed
};

std::list<ecmdErrorMsg> ecmdErrorMsgList;
pthread_mutex_t ecmdErrorMsgListMutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief Used for storing error messages internally to dll
 */
struct ecmdErrorTarget {
  uint32_t returnCode;          ///< Numeric error code- see ecmdReturnCodes.H
  ecmdChipTarget target;        ///< The target with the error
};

std::list<ecmdErrorTarget> ecmdErrorTargetList;
pthread_mutex_t ecmdErrorTargetListMutex = PTHREAD_MUTEX_INITIALIZER;

struct ecmdUserInfo {
  std::string cage;
  std::string node;
  std::string slot;
  std::string pos;
  std::string chipUnitNum;
  std::string thread;
} ecmdUserArgs;

/* This is used by the ecmdPush/PopCommandArgs functions */
std::list<ecmdUserInfo> ecmdArgsStack;

//----------------------------------------------------------------------
//  Constants
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//  Macros
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//  Internal Function Prototypes
//----------------------------------------------------------------------
#ifndef ECMD_REMOVE_LATCH_FUNCTIONS
/* @brief Parse out the scandef for the specified latch name */
uint32_t readScandef(const ecmdChipTarget & target, const char* i_ringName, const char* i_latchName, ecmdLatchMode_t i_mode, ecmdLatchBufferEntry & o_latchdata);
/* @brief Look up the provided latch name in the scandef hash */
uint32_t readScandefHash(const ecmdChipTarget & target, const char* i_ringName,const char* i_latchName, ecmdLatchBufferEntry & o_latchdata) ;
#endif // ECMD_REMOVE_LATCH_FUNCTIONS

/* @brief Returns true if curPos is not in userArgs */
uint8_t removeCurrentElement(int curPos, std::string userArgs);
/* @brief Returns true if all chars of str are decimal numbers */
bool isValidTargetString(std::string &str);
/* @brief used by TargetConfigured/TargetExist functions */
bool queryTargetConfigExist(const ecmdChipTarget & i_target, const ecmdQueryData * i_queryData, bool i_existQuery);
/* @brief used by QuerySelected/QuerySelectedExist functions */
uint32_t queryConfigExistSelected(ecmdChipTarget & i_target, ecmdQueryData & o_queryData, ecmdLoopType_t i_looptype, bool i_existMode);

/* @brief used by dllCommonCommandArgs when ":" found, sets ecmdUserArgs */
uint32_t ecmdTargetExpansion(std::string arg_string , const char * input_target);


//----------------------------------------------------------------------
//  Global Variables
//----------------------------------------------------------------------
#ifndef ECMD_STRIP_DEBUG
/* @brief This is used to output Debug messages on the DLL side */
uint32_t ecmdGlobal_DllDebug = 0;
#endif
/* @brief This is a global var set by -quiet */
uint32_t ecmdGlobal_quiet = 0;

/* @brief This is a global error var set by -quieterror */
uint32_t ecmdGlobal_quietError = 1;

/* @brief This is a global var set by -coe */
uint32_t ecmdGlobal_continueOnError = 0;

/* @brief This is a global var to determine how the looper runs */
/* ECMD_CONFIG_LOOP, the default */
/* ECMD_EXIST_LOOP, turned on by -exist */
uint32_t ecmdGlobal_looperMode = ECMD_CONFIG_LOOP;

/* @brief This is a global var set by ecmdSetCurrentCmdline() */
std::string ecmdGlobal_currentCmdline = "";

/* @brief This is a global var set by ecmdMain.C to say we are in a cmdline program */
uint32_t ecmdGlobal_cmdLineMode = 0;


// Eliminate the follow unavoidable lint message for everywhere 'major' and
// 'minor' are declared in this file.
//lint -e123
//
//   char major[10];
// imp_ecmd/dll/ecmdDllCapi.C  279  Error 123: Macro 'major' defined with arguments at line 28, file /usr/include/sys/sysmacros.h -- this is just a warning
// /usr/include/sys/sysmacros.h  28  Info 830: Location cited in prior message
//       _
//   char minor[10];
// imp_ecmd/dll/ecmdDllCapi.C  280  Error 123: Macro 'minor' defined with arguments at line 29, file /usr/include/sys/sysmacros.h -- this is just a warning
// /usr/include/sys/sysmacros.h  29  Info 830: Location cited in prior message

//---------------------------------------------------------------------
// Member Function Specifications
//---------------------------------------------------------------------
uint32_t dllLoadDll (const char* i_clientVersion, uint32_t debugLevel) {
  uint32_t rc;

#if !defined(_LP64) && defined(AIX)
  std::ignore debugLevel;
#endif

  /* First off let's check our version */
  /* Let's found our '.' char because we only fail if the Major number changes */
  //lint -e613 i_clientVersion is ECMD_CAPI_VERSION so it can't be NULL @02a
  uint32_t majorlength = (uint32_t)(strchr(i_clientVersion, '.') - i_clientVersion);

  if (strncmp(i_clientVersion,ECMD_CAPI_VERSION,majorlength)) {
    fprintf(stderr,"**** FATAL : eCMD DLL and your client Major version numbers don't match, they are not compatible\n");
    fprintf(stderr,"**** FATAL : Client Version : %s   : DLL Version : %s\n",i_clientVersion, ECMD_CAPI_VERSION);

    if (atoi(i_clientVersion) < atoi(ECMD_CAPI_VERSION)) {
      fprintf(stderr,"**** FATAL : Your client is older than the eCMD Dll Plugin you are running\n");
      fprintf(stderr,"**** FATAL : You must grab the latest client libraries and rebuild your client to continue\n");
    } else {
      fprintf(stderr,"**** FATAL : It appears your client is newer than the eCMD Dll Plugin you are running\n");
      fprintf(stderr,"**** FATAL : Contact the eCMD team to have the Plugin rebuilt to match your client\n");
      fprintf(stderr,"**** FATAL : Or get ahold of down level client libraries and rebuild your client to match\n");

    }

    return ECMD_FATAL_FAILURE;
  }

  /* Now we are going to check the version of the shared lib we loaded */
  if (strncmp(ecmdGetSharedLibVersion().c_str(),ECMD_CAPI_VERSION,majorlength)) {
    fprintf(stderr,"**** FATAL : eCMD Shared Library and your Plugin Major version numbers don't match, they are not compatible\n");
    fprintf(stderr,"**** FATAL : Shared Library Version : %s   : DLL Version : %s\n",ecmdGetSharedLibVersion().c_str(), ECMD_CAPI_VERSION);

    if (atoi(ecmdGetSharedLibVersion().c_str()) < atoi(ECMD_CAPI_VERSION)) {
      fprintf(stderr,"**** FATAL : Your shared library is older than the eCMD Dll Plugin you are running\n");
      fprintf(stderr,"**** FATAL : You must grab the latest library to continue\n");
    } else {
      fprintf(stderr,"**** FATAL : It appears your shared library is newer than the eCMD Dll Plugin you are running\n");
      fprintf(stderr,"**** FATAL : Contact the eCMD team to have the Plugin rebuilt to match\n");
      fprintf(stderr,"**** FATAL : Or get ahold of a down level shared library and rerun\n");

    }

    return ECMD_FATAL_FAILURE;
  }

  rc = dllInitDll();

#ifndef ECMD_STRIP_DEBUG
  ecmdGlobal_DllDebug = debugLevel;
  char printstr[128];

  if (ecmdGlobal_DllDebug >= 8) {
    sprintf(printstr,"ECMD DEBUG : Client Version     '%s'\n", i_clientVersion);
    dllOutput(printstr);
    sprintf(printstr,"ECMD DEBUG : Plugin Version     '%s'\n", ECMD_CAPI_VERSION);
    dllOutput(printstr);
    sprintf(printstr,"ECMD DEBUG : Shared Lib Version '%s'\n", ecmdGetSharedLibVersion().c_str());
    dllOutput(printstr);
  }
#endif

  return rc;
}

uint32_t dllUnloadDll() {
  uint32_t rc = 0;
  rc = dllFreeDll();
  return rc;
}

#if defined (_AIX) && not defined (_LP64)
uint32_t dllCheckDllVersion (const char* options) {
#else
__attribute__((noreturn)) uint32_t dllCheckDllVersion (const char* options) {
#endif
  char ver[20];
  strcpy(ver, ECMD_CAPI_VERSION);
  char major[10];
  char minor[10];

  //lint -e613 ver is set by strcpy above so can't be NULL  @02a
  uint32_t majorlength = (uint32_t)(strchr(ver, '.') - ver);
  strncpy(major, ver, majorlength);
  major[majorlength] = '\0';
  strncpy(minor, &(ver[majorlength + 1]), strlen(ver) - majorlength - 1);
  minor[strlen(ver) - majorlength - 1] = '\0';

  /* Default is just the major number */
  if ((options == NULL) || (strlen(options) == 0)) {
    printf("ver%s",major);
  } else if (!strcmp(options,"full")) {
    printf("ver-%s-%s",major,minor);
  }

  /* Force an exit here as the dll is not properly initialized we can't allow things to continue */
  exit(0);
#if defined (_AIX) && not defined (_LP64)
  return 0; // JTA 09/27/06 - added to shut down a compiler warning.  It's not smart enough to see the exit above
#endif
}

bool dllQueryVersionGreater(const char* version) {

  std::string plver = ECMD_CAPI_VERSION;
  std::string clver = version;
  int plmajor, plminor;
  int clmajor, clminor;

  plmajor = atoi(plver.substr(0,plver.find('.')).c_str());
  plminor = atoi(plver.substr(plver.find('.')+1,std::string::npos).c_str());

  clmajor = atoi(clver.substr(0,clver.find('.')).c_str());
  clminor = atoi(clver.substr(clver.find('.')+1,std::string::npos).c_str());

  if (plmajor <  clmajor) return false;
  if (plmajor >  clmajor) return true;
  if (plminor >= clminor) return true;

  return false;
}

void ecmdIncrementLooperIterators (uint8_t level, ecmdLooperData& io_state);

// dllConfigLooperInit and dllExistLooperInit just call dllLooperInit in the correct mode
uint32_t dllConfigLooperInit(ecmdChipTarget & io_target, ecmdLoopType_t i_looptype, ecmdLooperData& io_state) {
  return dllLooperInit(io_target, i_looptype, io_state, ECMD_CONFIG_LOOP);
}

uint32_t dllExistLooperInit(ecmdChipTarget & io_target, ecmdLoopType_t i_looptype, ecmdLooperData& io_state) {
  return dllLooperInit(io_target, i_looptype, io_state, ECMD_EXIST_LOOP);
}

uint32_t dllLooperInit(ecmdChipTarget & io_target, ecmdLoopType_t i_looptype, ecmdLooperData& io_state, ecmdLoopMode_t i_mode) {
  uint32_t rc = ECMD_SUCCESS;
  ecmdChipTarget queryTarget;

  /* If we aren't told what mode to run in, figure it out */
  if (i_mode == ECMD_DYNAMIC_LOOP) {
    i_mode = (ecmdLoopMode_t)ecmdGlobal_looperMode;
  }
  /* If it's a dynamic reverse, we need to set things up properly based on the default */
  if (i_mode == ECMD_DYNAMIC_REVERSE_LOOP) {
    if (ecmdGlobal_looperMode == ECMD_CONFIG_LOOP) {
      i_mode = ECMD_CONFIG_REVERSE_LOOP;
    } else {
      i_mode = ECMD_EXIST_REVERSE_LOOP;
    }
  }

  // Set to unknown so we can error check later
  io_state.initialized = false;

#ifndef ECMD_REMOVE_UNITID_FUNCTIONS
  /* Are we using a unitid ? */
  if ((io_target.chipTypeState == ECMD_TARGET_FIELD_VALID) && (io_target.chipType.length() > 0) && (io_target.chipType[0] == 'u')) {

    /* Ok, we need to strip the u off the front for this call */
    std::string unitid = io_target.chipType.substr(1);
    io_state.unitIdTargets.clear();
    rc = dllUnitIdStringToTarget(unitid, io_state.unitIdTargets);
    if (rc == ECMD_INVALID_ARGS) {
      dllOutputError("ecmdConfigLooperInit - Invalid Unitid specified\n");
    } else if (rc == ECMD_FUNCTION_NOT_SUPPORTED) {
      dllOutputError("ecmdConfigLooperInit - Current plugin doesn't support Unitids\n");
    }

    if (rc) return rc;
    io_state.ecmdUseUnitid = true;
    io_state.ecmdLooperInitFlag = true;
    io_state.prevTarget = io_target;
    io_state.curUnitIdTarget = io_state.unitIdTargets.begin();


    /* Ok, we still need to call queryconfig, we will use this to make sure the targets that come back actually exist */
    /* Lets look at the first target to see what fields are valid - which tells us which fields we need to set to     */
    /*  wildcard so that we get all possible targets in the system                                                    */

    if ((*io_state.curUnitIdTarget).cageState != ECMD_TARGET_FIELD_UNUSED) queryTarget.cageState = ECMD_TARGET_FIELD_WILDCARD;
    else queryTarget.cageState = ECMD_TARGET_FIELD_UNUSED;
    if ((*io_state.curUnitIdTarget).nodeState != ECMD_TARGET_FIELD_UNUSED) queryTarget.nodeState = ECMD_TARGET_FIELD_WILDCARD;
    else queryTarget.nodeState = ECMD_TARGET_FIELD_UNUSED;
    if ((*io_state.curUnitIdTarget).slotState != ECMD_TARGET_FIELD_UNUSED) queryTarget.slotState = ECMD_TARGET_FIELD_WILDCARD;
    else queryTarget.slotState = ECMD_TARGET_FIELD_UNUSED;
    if ((*io_state.curUnitIdTarget).chipTypeState != ECMD_TARGET_FIELD_UNUSED) queryTarget.chipTypeState = ECMD_TARGET_FIELD_WILDCARD;
    else queryTarget.chipTypeState = ECMD_TARGET_FIELD_UNUSED;
    if ((*io_state.curUnitIdTarget).posState != ECMD_TARGET_FIELD_UNUSED) queryTarget.posState = ECMD_TARGET_FIELD_WILDCARD;
    else queryTarget.posState = ECMD_TARGET_FIELD_UNUSED;
    if ((*io_state.curUnitIdTarget).chipUnitTypeState != ECMD_TARGET_FIELD_UNUSED) queryTarget.chipUnitTypeState = ECMD_TARGET_FIELD_WILDCARD;
    else queryTarget.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;
    if ((*io_state.curUnitIdTarget).chipUnitNumState != ECMD_TARGET_FIELD_UNUSED) queryTarget.chipUnitNumState = ECMD_TARGET_FIELD_WILDCARD;
    else queryTarget.chipUnitNumState = ECMD_TARGET_FIELD_UNUSED;
    if ((*io_state.curUnitIdTarget).threadState != ECMD_TARGET_FIELD_UNUSED) queryTarget.threadState = ECMD_TARGET_FIELD_WILDCARD;
    else queryTarget.threadState = ECMD_TARGET_FIELD_UNUSED;

    if (i_mode == ECMD_EXIST_LOOP || i_mode == ECMD_EXIST_REVERSE_LOOP) {
      rc = dllQueryExist(queryTarget, io_state.ecmdSystemConfigData, ECMD_QUERY_DETAIL_LOW);
    } else {
      rc = dllQueryConfig(queryTarget, io_state.ecmdSystemConfigData, ECMD_QUERY_DETAIL_LOW);
    }
    if (rc) return rc;

    /* Standard physical targets */
  } else {
#endif // ECMD_REMOVE_UNITID_FUNCTIONS

    io_state.ecmdUseUnitid = false;

    queryTarget = io_target;

    /* Initialize defaults into the incoming target */
    if (io_target.cageState == ECMD_TARGET_FIELD_WILDCARD)         io_target.cage = 0;
    if (io_target.nodeState == ECMD_TARGET_FIELD_WILDCARD)         io_target.node = 0;
    if (io_target.slotState == ECMD_TARGET_FIELD_WILDCARD)         io_target.slot = 0;
    if (io_target.chipTypeState == ECMD_TARGET_FIELD_WILDCARD)     io_target.chipType = "na";
    if (io_target.posState == ECMD_TARGET_FIELD_WILDCARD)          io_target.pos = 0;
    if (io_target.chipUnitTypeState == ECMD_TARGET_FIELD_WILDCARD) io_target.chipUnitType = "na";
    if (io_target.chipUnitNumState == ECMD_TARGET_FIELD_WILDCARD)  io_target.chipUnitNum = 0;
    if (io_target.threadState == ECMD_TARGET_FIELD_WILDCARD)       io_target.thread = 0;

    /* Set all the states to valid, unless they are unused */
    if (io_target.cageState != ECMD_TARGET_FIELD_UNUSED)          io_target.cageState = ECMD_TARGET_FIELD_VALID;
    if (io_target.nodeState != ECMD_TARGET_FIELD_UNUSED)          io_target.nodeState = ECMD_TARGET_FIELD_VALID;
    if (io_target.slotState != ECMD_TARGET_FIELD_UNUSED)          io_target.slotState = ECMD_TARGET_FIELD_VALID;
    if (io_target.chipTypeState != ECMD_TARGET_FIELD_UNUSED)      io_target.chipTypeState = ECMD_TARGET_FIELD_VALID;
    if (io_target.posState != ECMD_TARGET_FIELD_UNUSED)           io_target.posState = ECMD_TARGET_FIELD_VALID;
    if (io_target.chipUnitTypeState != ECMD_TARGET_FIELD_UNUSED)  io_target.chipUnitTypeState = ECMD_TARGET_FIELD_VALID;
    if (io_target.chipUnitNumState != ECMD_TARGET_FIELD_UNUSED)   io_target.chipUnitNumState = ECMD_TARGET_FIELD_VALID;
    if (io_target.threadState != ECMD_TARGET_FIELD_UNUSED)        io_target.threadState = ECMD_TARGET_FIELD_VALID;

    if (i_looptype == ECMD_ALL_TARGETS_LOOP) {
      if (i_mode == ECMD_EXIST_LOOP || i_mode == ECMD_EXIST_REVERSE_LOOP) {
        rc = dllQueryExist(queryTarget, io_state.ecmdSystemConfigData, ECMD_QUERY_DETAIL_LOW);
      } else {
        rc = dllQueryConfig(queryTarget, io_state.ecmdSystemConfigData, ECMD_QUERY_DETAIL_LOW);
      }
    } else {
      if (i_mode == ECMD_EXIST_LOOP || i_mode == ECMD_EXIST_REVERSE_LOOP) {
        rc = dllQueryExistSelected(queryTarget, io_state.ecmdSystemConfigData, i_looptype);
      } else {
        rc = dllQueryConfigSelected(queryTarget, io_state.ecmdSystemConfigData, i_looptype);
      }

      /* Selected queries can change our states, so let's update them */
      if (queryTarget.cageState == ECMD_TARGET_FIELD_UNUSED)         io_target.cageState = ECMD_TARGET_FIELD_UNUSED;
      if (queryTarget.nodeState == ECMD_TARGET_FIELD_UNUSED)         io_target.nodeState = ECMD_TARGET_FIELD_UNUSED;
      if (queryTarget.slotState == ECMD_TARGET_FIELD_UNUSED)         io_target.slotState = ECMD_TARGET_FIELD_UNUSED;
      if (queryTarget.chipTypeState == ECMD_TARGET_FIELD_UNUSED)     io_target.chipTypeState = ECMD_TARGET_FIELD_UNUSED;
      if (queryTarget.posState == ECMD_TARGET_FIELD_UNUSED)          io_target.posState = ECMD_TARGET_FIELD_UNUSED;
      if (queryTarget.chipUnitTypeState == ECMD_TARGET_FIELD_UNUSED) io_target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;
      if (queryTarget.chipUnitNumState == ECMD_TARGET_FIELD_UNUSED)  io_target.chipUnitNumState = ECMD_TARGET_FIELD_UNUSED;
      if (queryTarget.threadState == ECMD_TARGET_FIELD_UNUSED)       io_target.threadState = ECMD_TARGET_FIELD_UNUSED;
    }
    if (rc) return rc;

    /* If the reverse option is set, go through and flip all the data structures */
    if (i_mode == ECMD_EXIST_REVERSE_LOOP || i_mode == ECMD_CONFIG_REVERSE_LOOP) {
      std::list<ecmdCageData>::iterator curCage;
      std::list<ecmdNodeData>::iterator curNode;
      std::list<ecmdSlotData>::iterator curSlot;
      std::list<ecmdChipData>::iterator curChip;
      std::list<ecmdChipUnitData>::iterator curChipUnit;
      std::list<ecmdThreadData>::iterator curThread;

      for (curCage = io_state.ecmdSystemConfigData.cageData.begin(); curCage != io_state.ecmdSystemConfigData.cageData.end(); curCage++) {
        for (curNode = curCage->nodeData.begin(); curNode != curCage->nodeData.end(); curNode++) {
          for (curSlot = curNode->slotData.begin(); curSlot != curNode->slotData.end(); curSlot++) {
            for (curChip = curSlot->chipData.begin(); curChip != curSlot->chipData.end(); curChip++) {
              for (curChipUnit = curChip->chipUnitData.begin(); curChipUnit != curChip->chipUnitData.end(); curChipUnit++) {
                /* All done at this level, now reverse it */
                curChipUnit->threadData.reverse();
              }
              /* All done at this level, now reverse it */
              curChip->chipUnitData.reverse();
            }
            /* All done at this level, now reverse it */
            curSlot->chipData.reverse();
          }
          /* All done at this level, now reverse it */
          curNode->slotData.reverse();
        }
        /* All done at this level, now reverse it */
        curCage->nodeData.reverse();
      }
      /* All done at this level, now reverse it */
      io_state.ecmdSystemConfigData.cageData.reverse();
    }

    io_state.ecmdCurCage = io_state.ecmdSystemConfigData.cageData.begin();
    io_state.ecmdLooperInitFlag = true;
    io_state.prevTarget = io_target;

#ifndef ECMD_REMOVE_UNITID_FUNCTIONS
  }
#endif // ECMD_REMOVE_UNITID_FUNCTIONS

  /* Success! */
  io_state.initialized = true;

  return rc;
}

// ConfigLooperNext and ExistLooperNext can share the same code with just a switch to call the right function
uint32_t dllConfigLooperNext(ecmdChipTarget & io_target, ecmdLooperData& io_state) {
  return dllLooperNext(io_target, io_state, ECMD_CONFIG_LOOP);
}

uint32_t dllExistLooperNext(ecmdChipTarget & io_target, ecmdLooperData& io_state) {
  return dllLooperNext(io_target, io_state, ECMD_EXIST_LOOP);
}

uint32_t dllLooperNext(ecmdChipTarget & io_target, ecmdLooperData& io_state, ecmdLoopMode_t i_mode) {
  uint32_t rc = ECMD_SUCCESS;
  const uint8_t CAGE = 0;
  const uint8_t NODE = 1;
  const uint8_t SLOT = 2;
  const uint8_t CHIP = 3;
  const uint8_t CHIPUNIT = 4;
  const uint8_t THREAD = 5;
  bool done = false;
  uint8_t level = 0;;

  if (!io_state.initialized) {
    dllOutputError("ecmdConfigLooperNext - Invalid io_state passed, verify ecmdConfigLooperInit was run successfully\n");
    /* We return 0 which stops the loop, we can't return any failure from here */
    return 0;
  }

  /* If we aren't told what mode to run in, figure it out */
  if (i_mode == ECMD_DYNAMIC_LOOP) {
    i_mode = (ecmdLoopMode_t)ecmdGlobal_looperMode;
  }
  /* If it's a dynamic reverse, we need to set things up properly based on the default */
  if (i_mode == ECMD_DYNAMIC_REVERSE_LOOP) {
    if (ecmdGlobal_looperMode == ECMD_CONFIG_LOOP) {
      i_mode = ECMD_CONFIG_REVERSE_LOOP;
    } else {
      i_mode = ECMD_EXIST_REVERSE_LOOP;
    }
  }

  /* Are we using unitids ? */
  if (io_state.ecmdUseUnitid) {
    /* We at the end ? */
    while (!done) {
      if (io_state.curUnitIdTarget == io_state.unitIdTargets.end()) {
        return 0;
      }

      io_target = *(io_state.curUnitIdTarget);
      io_state.curUnitIdTarget++;

      /* Is this target actually configured, if not try the next one */
      if (i_mode == ECMD_EXIST_LOOP) {
        if (dllQueryTargetExist(io_target, &(io_state.ecmdSystemConfigData))) {
          done = true;
        }
      } else {
        if (dllQueryTargetConfigured(io_target, &(io_state.ecmdSystemConfigData))) {
          done = true;
        }
      }
    } /* while !done */


    /* Not using unitid's use physical targets */
  } else {
    while (!done) {
      level = CAGE;
      uint8_t valid = 1;


      /* We are at the end of the loop, nothing left to loop on, get out of here */
      if (io_state.ecmdCurCage == io_state.ecmdSystemConfigData.cageData.end()) {
        return rc;
      }

      /* ******** NOTE : The iterators in io_state always point to the next instance to use */
      /*           (the one that should be returned from this function ****************     */

      /* Enter if : */
      /* First time in config looper */
      /* last cage != current cage */
      if (io_state.ecmdLooperInitFlag ||
          io_target.cage != (*io_state.ecmdCurCage).cageId
          ) {

        /* Data is valid, let's setup this part of the target */
        io_target.cage = (*io_state.ecmdCurCage).cageId;
        io_state.ecmdCurNode = (*io_state.ecmdCurCage).nodeData.begin();
        valid = 0;

        /* If next level is unused we default to 0 */
        if ((io_state.prevTarget.nodeState == ECMD_TARGET_FIELD_UNUSED)) {

          /* If the next level is required but empty, this position isn't valid we need to restart */
        } else if ((io_state.prevTarget.nodeState != ECMD_TARGET_FIELD_UNUSED) && (io_state.ecmdCurNode == (*io_state.ecmdCurCage).nodeData.end())) {
          /* Increment the iterators to point to the next target (at the level above us) */
          ecmdIncrementLooperIterators(level - 1, io_state);
          continue;

          /* Everything is grand, let's continue to the next level */
        } else {
          level = NODE;
        }


      }
      else {
        level = NODE;
      }

      /* Enter if : */
      /* Level == Node (the user is looping with nodes  */
      /* !valid - current node iterator isn't valid */
      /* last node != current node */
      if (level == NODE &&
          (!valid ||
           io_target.node != (*io_state.ecmdCurNode).nodeId)) {

        /* Data is valid, let's setup this part of the target */
        io_target.node = (*io_state.ecmdCurNode).nodeId;
        io_state.ecmdCurSlot = (*io_state.ecmdCurNode).slotData.begin();
        valid = 0;

        /* If next level is unused we default to 0 */
        if ((io_state.prevTarget.slotState == ECMD_TARGET_FIELD_UNUSED)) {

          /* If the next level is required but empty, this position isn't valid we need to restart */
        } else if ((io_state.prevTarget.slotState != ECMD_TARGET_FIELD_UNUSED) && (io_state.ecmdCurSlot == (*io_state.ecmdCurNode).slotData.end())) {
          /* Increment the iterators to point to the next target (at the level above us) */
          ecmdIncrementLooperIterators(level - 1, io_state);
          continue;

          /* Everything is grand, let's continue to the next level */
        } else {
          level = SLOT;
        }


      }
      else if (valid) {
        level = SLOT;
      }

      /* Enter if : */
      /* Level == Slot (the user is looping with Slots  */
      /* !valid - current Slot iterator isn't valid */
      /* last Slot != current Slot */
      if (level == SLOT &&
          (!valid ||
           io_target.slot != (*io_state.ecmdCurSlot).slotId)) {

        /* Data is valid, let's setup this part of the target */
        io_target.slot = (*io_state.ecmdCurSlot).slotId;
        io_state.ecmdCurChip = (*io_state.ecmdCurSlot).chipData.begin();
        valid = 0;


        /* If next level is unused we default to 0 */
        if ((io_state.prevTarget.chipTypeState == ECMD_TARGET_FIELD_UNUSED || io_state.prevTarget.posState == ECMD_TARGET_FIELD_UNUSED)) {

          /* If the next level is required but empty, this position isn't valid we need to restart */
        } else if ((io_state.prevTarget.chipTypeState == ECMD_TARGET_FIELD_UNUSED || io_state.prevTarget.posState == ECMD_TARGET_FIELD_UNUSED) && (io_state.ecmdCurChip == (*io_state.ecmdCurSlot).chipData.end())) {
          /* Increment the iterators to point to the next target (at the level above us) */
          ecmdIncrementLooperIterators(level - 1, io_state);
          continue;

          /* Everything is grand, let's continue to the next level */
        } else {
          level = CHIP;
        }


      }
      else if (valid) {
        level = CHIP;
      }


      /* Enter if : */
      /* Level == Chip (the user is looping with Chips  */
      /* !valid - current Chip iterator isn't valid */
      /* last ChipType != current ChipType */
      /* last Chip pos != current Chip pos */
      if (level == CHIP &&
          (!valid ||
           io_target.chipType != (*io_state.ecmdCurChip).chipType ||
           io_target.pos != (*io_state.ecmdCurChip).pos  )) {

        /* Data is valid, let's setup this part of the target */
        io_target.chipType = (*io_state.ecmdCurChip).chipType;
        io_target.pos = (*io_state.ecmdCurChip).pos;
        io_state.ecmdCurChipUnit = (*io_state.ecmdCurChip).chipUnitData.begin();
        valid = 0;

        /* If next level is unused we default to 0 */
        if ((io_state.prevTarget.chipUnitNumState == ECMD_TARGET_FIELD_UNUSED)) {

          /* If the next level is required but empty, this position isn't valid we need to restart */
        } else if ((io_state.prevTarget.chipUnitNumState != ECMD_TARGET_FIELD_UNUSED) && (io_state.ecmdCurChipUnit == (*io_state.ecmdCurChip).chipUnitData.end())) {
          /* Increment the iterators to point to the next target (at the level above us) */
          ecmdIncrementLooperIterators(level - 1, io_state);
          continue;

          /* Everything is grand, let's continue to the next level */
        } else {
          level = CHIPUNIT;
        }

      }
      else if (valid) {
        level = CHIPUNIT;
      }

      /* Enter if : */
      /* Level == ChipUnit (the user is looping with ChipUnits  */
      /* !valid - current ChipUnit iterator isn't valid */
      /* last ChipUnitType != current ChipUnitType */
      /* last ChipUnitNum != current ChipUnitNum */
      if (level == CHIPUNIT &&
          (!valid ||
           io_target.chipUnitType != (*io_state.ecmdCurChipUnit).chipUnitType ||
           io_target.chipUnitNum != (*io_state.ecmdCurChipUnit).chipUnitNum)) {

        /* Data is valid, let's setup this part of the target */
        io_target.chipUnitType = (*io_state.ecmdCurChipUnit).chipUnitType;
        io_target.chipUnitNum = (*io_state.ecmdCurChipUnit).chipUnitNum;
        io_state.ecmdCurThread = (*io_state.ecmdCurChipUnit).threadData.begin();
        valid = 0;


        /* If next level is unused we default to 0 */
        if (io_state.prevTarget.threadState == ECMD_TARGET_FIELD_UNUSED) {

          /* If the next level is required but empty, this position isn't valid we need to restart */
        } else if ((io_state.prevTarget.threadState != ECMD_TARGET_FIELD_UNUSED) && (io_state.ecmdCurThread == (*io_state.ecmdCurChipUnit).threadData.end())) {
          /* Increment the iterators to point to the next target (at the level above us) */
          ecmdIncrementLooperIterators(level - 1, io_state);
          continue;

          /* Everything is grand, let's continue to the next level */
        } else {
          level = THREAD;
        }


      }
      else if (valid) {
        level = THREAD;
      }

      /* Enter if : */
      /* Level == Thread (the user is looping with Threads  */
      /* !valid - current Thread iterator isn't valid */
      /* last Thread != current Thread */
      if (level == THREAD && (!valid || io_target.thread != (*io_state.ecmdCurThread).threadId)) {

        /* Data is valid, let's setup this part of the target */
        io_target.thread = (*io_state.ecmdCurThread).threadId;

      }

      /* We got here, must be done */
      done = true;

    } /* End while */

    /* Increment the iterators to point to the next target */
    ecmdIncrementLooperIterators(level, io_state);

  } /* end phys/unitid if */

  /* We are through the first init loop */
  if (io_state.ecmdLooperInitFlag) {
    io_state.ecmdLooperInitFlag = false;
  }

#ifndef ECMD_STRIP_DEBUG
  if (ecmdGlobal_DllDebug >= 8) {
    std::string printed = "ECMD DEBUG (ecmdConfigLooperNext) : Found next Target : " + ecmdWriteTarget(io_target, ECMD_DISPLAY_TARGET_DEFAULT); printed += "\n";
    dllOutput(printed.c_str());
  }
#endif

  /* We got here, we have more to do, let's tell the client */
  rc = 1;

  return rc;

}

void ecmdIncrementLooperIterators (uint8_t level, ecmdLooperData& io_state) {
  /* Let's start incrementing our lowest pointer so it points to the next object for the subsequent call to this function */
  const uint8_t CAGE = 0;
  const uint8_t NODE = 1;
  const uint8_t SLOT = 2;
  const uint8_t CHIP = 3;
  const uint8_t CHIPUNIT = 4;
  const uint8_t THREAD = 5;

  // The following switch statement makes deliberate use of falling through from
  // one case statement to the next.  So tell Beam to not flag those ass errors
  // with the /*fall through*/ comments. @02a
  switch (level) {

    case THREAD:  //thread
      io_state.ecmdCurThread++;
      /* Did we find another thread, if not we will try chipUnit */
      if (io_state.ecmdCurThread != (*io_state.ecmdCurChipUnit).threadData.end()) {
        break;
      }
      /*fall through*/
    case CHIPUNIT:  //chipUnit
      io_state.ecmdCurChipUnit++;
      /* Did we find another chipUnit, if not we will try chip */
      if (io_state.ecmdCurChipUnit != (*io_state.ecmdCurChip).chipUnitData.end()) {
        break;
      }
      /*fall through*/
    case CHIP:  //chip
      io_state.ecmdCurChip++;
      /* Did we find another chip, if not we will try slot */
      if (io_state.ecmdCurChip != (*io_state.ecmdCurSlot).chipData.end()) {
        break;
      }
      /*fall through*/
    case SLOT:  //slot
      io_state.ecmdCurSlot++;
      /* Did we find another slot, if not we will try node */
      if (io_state.ecmdCurSlot != (*io_state.ecmdCurNode).slotData.end()) {
        break;
      }
      /*fall through*/
    case NODE:  //node
      io_state.ecmdCurNode++;
      /* Did we find another node, if not we will try cage */
      if (io_state.ecmdCurNode != (*io_state.ecmdCurCage).nodeData.end()) {
        break;
      }
      /*fall through*/
    case CAGE:  //cage
      io_state.ecmdCurCage++;
      break;

    default:
      //shouldn't get here
      break;
  }
}

std::string dllGetErrorMsg(uint32_t i_returnCode, bool i_parseReturnCode, bool i_deleteMessage, bool i_messageBorder) {
  std::string ret;
  std::list<ecmdErrorMsg>::iterator cur;
  char tmp[200];
  bool first = true;
  size_t headerLength;

  pthread_mutex_lock(&ecmdErrorMsgListMutex);
  for (cur = ecmdErrorMsgList.begin(); cur != ecmdErrorMsgList.end(); cur++) {
    if ((cur->returnCode == i_returnCode) || ((i_returnCode == ECMD_GET_ALL_REMAINING_ERRORS) && (!cur->accessed))) {
      if (first && i_messageBorder) {
        ret  = "=============== EXTENDED ERROR MSG : " + cur->whom + " ===============\n";
        headerLength = ret.length();
        first = false;
      }
      ret = ret + cur->message;
      cur->accessed = true;
    }
  }
  pthread_mutex_unlock(&ecmdErrorMsgListMutex);
  if (!first) {
    /* We must have found something */
    if (i_parseReturnCode) {
      sprintf(tmp,"RETURN CODE (0x%X): %s\n",i_returnCode,dllParseReturnCode(i_returnCode).c_str());
      ret += tmp;
    }
    if (i_messageBorder) {
      ret += std::string((headerLength - 1), '=') + '\n';
    }
  }

  /* We want to parse the return code even if we didn't finded extended error info */
  if ((ret.length() == 0) && (i_parseReturnCode)) {
    sprintf(tmp,"ecmdGetErrorMsg - RETURN CODE (0x%X): %s\n",i_returnCode, dllParseReturnCode(i_returnCode).c_str());
    ret = tmp;
  }

  // Now delete any error messages we retrieved, if the user asked us to
  if (i_deleteMessage) {
    uint32_t flushRc;
    flushRc = dllFlushRegisteredErrorMsgs(i_returnCode);
    if (flushRc) {
      // Error occurred, must return empty string
      return "";
    }
  }

  return ret;
}

uint32_t dllRegisterErrorMsg(uint32_t i_returnCode, const char* i_whom, const char* i_message) {
  uint32_t rc = ECMD_SUCCESS;

  ecmdErrorMsg curError;
  curError.returnCode = i_returnCode;
  curError.whom = i_whom;
  curError.message = i_message;
  curError.accessed = false;

  pthread_mutex_lock(&ecmdErrorMsgListMutex);
  ecmdErrorMsgList.push_back(curError);
  pthread_mutex_unlock(&ecmdErrorMsgListMutex);

  return rc;
}

uint32_t dllFlushRegisteredErrorMsgs(uint32_t i_returnCode) {
  uint32_t rc = ECMD_SUCCESS;

  pthread_mutex_lock(&ecmdErrorMsgListMutex);
  std::list<ecmdErrorMsg>::iterator errorIter = ecmdErrorMsgList.begin();
  std::list<ecmdErrorMsg>::iterator deleteIter;

  while (errorIter != ecmdErrorMsgList.end()) {
    if (errorIter->returnCode == i_returnCode) {
      deleteIter = errorIter;
      errorIter++; // Walk our iter forward before we delete were we are
      ecmdErrorMsgList.erase(deleteIter);
    } else {
      // Didn't find a match, so just advance forward
      errorIter++;
    }
  }
  pthread_mutex_unlock(&ecmdErrorMsgListMutex);

  return rc;
}


uint32_t dllFlushRegisteredErrorMsgsString(uint32_t i_returnCode, std::string i_searchString) {
  uint32_t rc = ECMD_SUCCESS;

  pthread_mutex_lock(&ecmdErrorMsgListMutex);
  std::list<ecmdErrorMsg>::iterator errorIter = ecmdErrorMsgList.begin();
  std::list<ecmdErrorMsg>::iterator deleteIter;

  while (errorIter != ecmdErrorMsgList.end()) {
    if ( (errorIter->returnCode == i_returnCode) && (errorIter->message.find(i_searchString) != std::string::npos) ) {
      deleteIter = errorIter;
      errorIter++; // Walk our iter forward before we delete were we are
      ecmdErrorMsgList.erase(deleteIter);
    } else {
      // Didn't find a match, so just advance forward
      errorIter++;
    }
  }
  pthread_mutex_unlock(&ecmdErrorMsgListMutex);

  return rc;
}

uint32_t dllGetErrorTarget(uint32_t i_returnCode, std::list<ecmdChipTarget> & o_errorTargets, bool i_deleteTarget) {
  uint32_t rc = ECMD_SUCCESS;
  std::list<ecmdErrorTarget>::iterator cur;

  pthread_mutex_lock(&ecmdErrorTargetListMutex);
  for (cur = ecmdErrorTargetList.begin(); cur != ecmdErrorTargetList.end(); cur++) {
    if (cur->returnCode == i_returnCode) {
      /* The error code matches, push it onto the list */
      o_errorTargets.push_back(cur->target);
    }
  }
  pthread_mutex_unlock(&ecmdErrorTargetListMutex);

  // Now delete any error messages we retrieved, if the user asked us to
  if (i_deleteTarget) {
    uint32_t flushRc;
    flushRc = dllFlushRegisteredErrorTargets(i_returnCode);
    if (flushRc) {
      return flushRc;
    }
  }

  return rc;
}

uint32_t dllRegisterErrorTarget(uint32_t i_returnCode, const ecmdChipTarget & i_errorTarget) {
  uint32_t rc = ECMD_SUCCESS;

  ecmdErrorTarget curError;
  curError.returnCode = i_returnCode;
  curError.target = i_errorTarget;

  pthread_mutex_lock(&ecmdErrorTargetListMutex);
  ecmdErrorTargetList.push_back(curError);
  pthread_mutex_unlock(&ecmdErrorTargetListMutex);

  return rc;
}

uint32_t dllFlushRegisteredErrorTargets(uint32_t i_returnCode) {
  uint32_t rc = ECMD_SUCCESS;

  pthread_mutex_lock(&ecmdErrorTargetListMutex);
  std::list<ecmdErrorTarget>::iterator errorIter = ecmdErrorTargetList.begin();
  std::list<ecmdErrorTarget>::iterator deleteIter;

  while (errorIter != ecmdErrorTargetList.end()) {
    if (errorIter->returnCode == i_returnCode) {
      deleteIter = errorIter;
      errorIter++; // Walk our iter forward before we delete were we are
      ecmdErrorTargetList.erase(deleteIter);
    } else {
      // Didn't find a match, so just advance forward
      errorIter++;
    }
  }
  pthread_mutex_unlock(&ecmdErrorTargetListMutex);

  return rc;
}

uint32_t dllQuerySelected(ecmdChipTarget & i_target, ecmdQueryData & o_queryData, ecmdLoopType_t i_looptype) {
  return queryConfigExistSelected(i_target, o_queryData, i_looptype, false);
}

uint32_t dllQueryConfigSelected(ecmdChipTarget & i_target, ecmdQueryData & o_queryData, ecmdLoopType_t i_looptype) {
  return queryConfigExistSelected(i_target, o_queryData, i_looptype, false);
}

uint32_t dllQueryExistSelected(ecmdChipTarget & i_target, ecmdQueryData & o_queryData, ecmdLoopType_t i_looptype) {
  return queryConfigExistSelected(i_target, o_queryData, i_looptype, true);
}

uint32_t queryConfigExistSelected(ecmdChipTarget & i_target, ecmdQueryData & o_queryData, ecmdLoopType_t i_looptype, bool i_existMode) {
  uint32_t rc = ECMD_SUCCESS;

  uint8_t SINGLE = 0;
  uint8_t ALL = 1;
  uint8_t MULTI = 2;
  uint8_t FT = 3;
  uint8_t LT = 4;
  uint8_t ET = 5;
  uint8_t OT = 6;

  //@01c Add init to 2
  uint8_t cageType = 2;
  uint8_t nodeType = 2;
  uint8_t slotType = 2;
  uint8_t posType  = 2;
  uint8_t chipUnitNumType = 2;
  uint8_t threadType = 2;

  std::string patterns = ",.";

  /* Let's setup for the Variable depth, walk up until we find something specified */
  if ((i_looptype == ECMD_SELECTED_TARGETS_LOOP_VD) || (i_looptype == ECMD_SELECTED_TARGETS_LOOP_VD_DEFALL)) {
    if ((i_target.threadState == ECMD_TARGET_FIELD_UNUSED) || (i_target.threadState != ECMD_TARGET_FIELD_VALID && ecmdUserArgs.thread == "")) {
      i_target.threadState = ECMD_TARGET_FIELD_UNUSED;

      if ((i_target.chipUnitNumState == ECMD_TARGET_FIELD_UNUSED) || (i_target.chipUnitNumState != ECMD_TARGET_FIELD_VALID && ecmdUserArgs.chipUnitNum == "")) {
        i_target.chipUnitNumState = ECMD_TARGET_FIELD_UNUSED;

        if ((i_target.posState == ECMD_TARGET_FIELD_UNUSED) || (i_target.posState != ECMD_TARGET_FIELD_VALID && ecmdUserArgs.pos == "")) {
          i_target.posState = ECMD_TARGET_FIELD_UNUSED;

          if ((i_target.slotState == ECMD_TARGET_FIELD_UNUSED) || (i_target.slotState != ECMD_TARGET_FIELD_VALID && ecmdUserArgs.slot == "")) {
            i_target.slotState = ECMD_TARGET_FIELD_UNUSED;

            if ((i_target.nodeState == ECMD_TARGET_FIELD_UNUSED) || (i_target.nodeState != ECMD_TARGET_FIELD_VALID && ecmdUserArgs.node == "")) {
              i_target.nodeState = ECMD_TARGET_FIELD_UNUSED;

              if ((i_target.cageState == ECMD_TARGET_FIELD_UNUSED) || (i_target.cageState != ECMD_TARGET_FIELD_VALID && ecmdUserArgs.cage == "")) {
                i_target.cageState = ECMD_TARGET_FIELD_UNUSED;


              } /* cage */
            } /* node */
          } /* slot */
        } /* pos */
      } /* chipUnitNum */
    } /* thread */
    /* Go back to a standard loop now that states are set */
    if (i_looptype == ECMD_SELECTED_TARGETS_LOOP_VD) {
      i_looptype = ECMD_SELECTED_TARGETS_LOOP;
    } else {
      i_looptype = ECMD_SELECTED_TARGETS_LOOP_DEFALL;
    }
  }

  /* If the cage is set to ignore we can't return anything so let's just short circuit */
  if (i_target.cageState == ECMD_TARGET_FIELD_UNUSED) {
    return ECMD_SUCCESS;
  }

  /* ----------------------------------------------------------- */
  /* update target with useful info from the ecmdUserArgs struct */
  /* ----------------------------------------------------------- */

  //cage
  /* If the state is already valid we just continue on */
  if (i_target.cageState == ECMD_TARGET_FIELD_VALID) {
    cageType = SINGLE;

  } else if (i_target.cageState != ECMD_TARGET_FIELD_UNUSED) {

    /* Did the user specify any cage args */
    if (ecmdUserArgs.cage.length()) {
      /* If the user used any sort of list 0,1,2,4 or range 2..5 then we do multi */
      if (ecmdUserArgs.cage.find_first_of(patterns) < ecmdUserArgs.cage.length()) {
        if (!isValidTargetString(ecmdUserArgs.cage)) {
          dllOutputError("dllQuerySelected - cage (-k#) argument contained invalid characters\n");
          return ECMD_INVALID_ARGS;
        }
        i_target.cageState = ECMD_TARGET_FIELD_WILDCARD;
        cageType = MULTI;
      }

      /* See if we have a single entry -k1 */
      else if (isValidTargetString(ecmdUserArgs.cage)) {
        i_target.cageState = ECMD_TARGET_FIELD_VALID;
        i_target.cage = (uint32_t)atoi(ecmdUserArgs.cage.c_str());
        cageType = SINGLE;
      }

      /* See if the user specified -aall or -kall */
     else if (ecmdUserArgs.cage == "all") {
        i_target.cageState = ECMD_TARGET_FIELD_WILDCARD;
        cageType = ALL;
      }

      /* See if the user specified -aft or -kft */
      else if (ecmdUserArgs.cage == "ft") {
        i_target.cageState = ECMD_TARGET_FIELD_WILDCARD;
        cageType = FT;
      }

      /* See if the user specified -alt or -klt */
      else if (ecmdUserArgs.cage == "lt") {
        i_target.cageState = ECMD_TARGET_FIELD_WILDCARD;
        cageType = LT;
      }

      /* See if the user specified -aet or -ket */
      else if (ecmdUserArgs.cage == "et") {
        i_target.cageState = ECMD_TARGET_FIELD_WILDCARD;
        cageType = ET;
      }

      /* See if the user specified -aot or -kot */
      else if (ecmdUserArgs.cage == "ot") {
        i_target.cageState = ECMD_TARGET_FIELD_WILDCARD;
        cageType = OT;
      }

      /* See if the user specified or -k- */
      else if (ecmdUserArgs.cage == "-") {
        dllOutputError("dllQuerySelected - argument -k- not supported\n");
        return ECMD_INVALID_ARGS;
      }

      /* Finally, we can error out */
      else {
        dllOutputError("dllQuerySelected - unknown -k options found!\n");
        return ECMD_INVALID_ARGS;
      }
    }
    /* Nothing was specified, set some defaults */
    else {
      if (i_looptype == ECMD_SELECTED_TARGETS_LOOP_DEFALL) {
        /* Default to all */
        i_target.cageState = ECMD_TARGET_FIELD_WILDCARD;
        cageType = ALL;
      } else {
        /* User didn't specify anything and we default to 0 */
        i_target.cage = 0;
        cageType = SINGLE;
        i_target.cageState = ECMD_TARGET_FIELD_VALID;
      }
    }
  }

  //node
  /* If the state is already valid we just continue on */
  if (i_target.nodeState == ECMD_TARGET_FIELD_VALID) {
    nodeType = SINGLE;

  } else if (i_target.nodeState != ECMD_TARGET_FIELD_UNUSED) {

    /* Did the user specify any node args */
    if (ecmdUserArgs.node.length()) {
      /* If the user used any sort of list 0,1,2,4 or range 2..5 then we do multi */
      if (ecmdUserArgs.node.find_first_of(patterns) < ecmdUserArgs.node.length()) {
        if (!isValidTargetString(ecmdUserArgs.node)) {
          dllOutputError("dllQuerySelected - node (-n#) argument contained invalid characters\n");
          return ECMD_INVALID_ARGS;
        }
        i_target.nodeState = ECMD_TARGET_FIELD_WILDCARD;
        nodeType = MULTI;
      }

      /* See if we have a single entry -n1 */
      else if (isValidTargetString(ecmdUserArgs.node)) {
        i_target.nodeState = ECMD_TARGET_FIELD_VALID;
        i_target.node = (uint32_t)atoi(ecmdUserArgs.node.c_str());
        nodeType = SINGLE;
      }

      /* See if the user specified -aall or -nall */
      else if (ecmdUserArgs.node == "all") {
        i_target.nodeState = ECMD_TARGET_FIELD_WILDCARD;
        nodeType = ALL;
      }

      /* See if the user specified -aft or -nft */
      else if (ecmdUserArgs.node == "ft") {
        i_target.nodeState = ECMD_TARGET_FIELD_WILDCARD;
        nodeType = FT;
      }

      /* See if the user specified -alt or -nlt */
      else if (ecmdUserArgs.node == "lt") {
        i_target.nodeState = ECMD_TARGET_FIELD_WILDCARD;
        nodeType = LT;
      }

      /* See if the user specified -aet or -net */
      else if (ecmdUserArgs.node == "et") {
        i_target.nodeState = ECMD_TARGET_FIELD_WILDCARD;
        nodeType = ET;
      }

      /* See if the user specified -aot or -not */
      else if (ecmdUserArgs.node == "ot") {
        i_target.nodeState = ECMD_TARGET_FIELD_WILDCARD;
        nodeType = OT;
      }

      /* See if the user specified or -n- */
      else if (ecmdUserArgs.node == "-") {
        i_target.nodeState = ECMD_TARGET_FIELD_VALID;
        i_target.node = ECMD_TARGETDEPTH_NA;
        nodeType = SINGLE;
      }

      /* Finally, we can error out */
      else {
        dllOutputError("dllQuerySelected - unknown -n options found!\n");
        return ECMD_INVALID_ARGS;
      }
    }
    /* Nothing was specified, set some defaults */
    else {
      if (i_looptype == ECMD_SELECTED_TARGETS_LOOP_DEFALL) {
        /* Default to all */
        i_target.nodeState = ECMD_TARGET_FIELD_WILDCARD;
        nodeType = ALL;
      } else {
        /* User didn't specify anything and we default to 0 */
        i_target.node = 0;
        nodeType = SINGLE;
        i_target.nodeState = ECMD_TARGET_FIELD_VALID;
      }
    }
  }

  //slot
  /* If the state is already valid we just continue on */
  if (i_target.slotState == ECMD_TARGET_FIELD_VALID) {
    slotType = SINGLE;

  } else if (i_target.slotState != ECMD_TARGET_FIELD_UNUSED) {

    /* Did the user specify any slot args */
    if (ecmdUserArgs.slot.length()) {
      /* If the user used any sort of list 0,1,2,4 or range 2..5 then we do multi */
      if (ecmdUserArgs.slot.find_first_of(patterns) < ecmdUserArgs.slot.length()) {
        if (!isValidTargetString(ecmdUserArgs.slot)) {
          dllOutputError("dllQuerySelected - slot (-s#) argument contained invalid characters\n");
          return ECMD_INVALID_ARGS;
        }
        i_target.slotState = ECMD_TARGET_FIELD_WILDCARD;
        slotType = MULTI;
      }

      /* See if we have a single entry -s1 */
      else if (isValidTargetString(ecmdUserArgs.slot)) {
        i_target.slotState = ECMD_TARGET_FIELD_VALID;
        i_target.slot = (uint32_t)atoi(ecmdUserArgs.slot.c_str());
        slotType = SINGLE;
      }

      /* See if the user specified -aall or -sall */
      else if (ecmdUserArgs.slot == "all") {
        i_target.slotState = ECMD_TARGET_FIELD_WILDCARD;
        slotType = ALL;
      }

      /* See if the user specified -aft or -sft */
      else if (ecmdUserArgs.slot == "ft") {
        i_target.slotState = ECMD_TARGET_FIELD_WILDCARD;
        slotType = FT;
      }

      /* See if the user specified -alt or -slt */
      else if (ecmdUserArgs.slot == "lt") {
        i_target.slotState = ECMD_TARGET_FIELD_WILDCARD;
        slotType = LT;
      }

      /* See if the user specified -aet or -set */
      else if (ecmdUserArgs.slot == "et") {
        i_target.slotState = ECMD_TARGET_FIELD_WILDCARD;
        slotType = ET;
      }

      /* See if the user specified -aot or -sot */
      else if (ecmdUserArgs.slot == "ot") {
        i_target.slotState = ECMD_TARGET_FIELD_WILDCARD;
        slotType = OT;
      }

      /* See if the user specified or -s- */
      else if (ecmdUserArgs.slot == "-") {
        i_target.slotState = ECMD_TARGET_FIELD_VALID;
        i_target.slot = ECMD_TARGETDEPTH_NA;
        slotType = SINGLE;
      }

      /* Finally, we can error out */
      else {
        dllOutputError("dllQuerySelected - unknown -s options found!\n");
        return ECMD_INVALID_ARGS;
      }
    }
    /* Nothing was specified, set some defaults */
    else {
      if (i_looptype == ECMD_SELECTED_TARGETS_LOOP_DEFALL) {
        /* Default to all */
        i_target.slotState = ECMD_TARGET_FIELD_WILDCARD;
        slotType = ALL;
      } else {
        /* User didn't specify anything and we default to 0 */
        i_target.slot = 0;
        slotType = SINGLE;
        i_target.slotState = ECMD_TARGET_FIELD_VALID;
      }
    }
  }

  //pos
  /* If the state is already valid we just continue on */
  if (i_target.posState == ECMD_TARGET_FIELD_VALID) {
    posType = SINGLE;

  } else if (i_target.posState != ECMD_TARGET_FIELD_UNUSED) {

    /* Did the user specify any pos args */
    if (ecmdUserArgs.pos.length()) {
      /* If the user used any sort of list 0,1,2,4 or range 2..5 then we do multi */
      if (ecmdUserArgs.pos.find_first_of(patterns) < ecmdUserArgs.pos.length()) {
        if (!isValidTargetString(ecmdUserArgs.pos)) {
          dllOutputError("dllQuerySelected - pos (-p#) argument contained invalid characters\n");
          return ECMD_INVALID_ARGS;
        }
        i_target.posState = ECMD_TARGET_FIELD_WILDCARD;
        posType = MULTI;
      }

      /* See if we have a single entry -p1 */
      else if (isValidTargetString(ecmdUserArgs.pos)) {
        i_target.posState = ECMD_TARGET_FIELD_VALID;
        i_target.pos = (uint32_t)atoi(ecmdUserArgs.pos.c_str());
        posType = SINGLE;
      }

      /* See if the user specified -aall or -pall */
      else if (ecmdUserArgs.pos == "all") {
        i_target.posState = ECMD_TARGET_FIELD_WILDCARD;
        posType = ALL;
      }

      /* See if the user specified -aft or -pft */
      else if (ecmdUserArgs.pos == "ft") {
        i_target.posState = ECMD_TARGET_FIELD_WILDCARD;
        posType = FT;
      }

      /* See if the user specified -alt or -plt */
      else if (ecmdUserArgs.pos == "lt") {
        i_target.posState = ECMD_TARGET_FIELD_WILDCARD;
        posType = LT;
      }

      /* See if the user specified -aet or -pet */
      else if (ecmdUserArgs.pos == "et") {
        i_target.posState = ECMD_TARGET_FIELD_WILDCARD;
        posType = ET;
      }

      /* See if the user specified -aot or -pot */
      else if (ecmdUserArgs.pos == "ot") {
        i_target.posState = ECMD_TARGET_FIELD_WILDCARD;
        posType = OT;
      }

      /* See if the user specified or -p- */
      else if (ecmdUserArgs.pos == "-") {
        dllOutputError("dllQuerySelected - argument -p- not supported\n");
        return ECMD_INVALID_ARGS;
      }

      /* Finally, we can error out */
      else {
        dllOutputError("dllQuerySelected - unknown -p options found!\n");
        return ECMD_INVALID_ARGS;
      }
    }
    /* Nothing was specified, set some defaults */
    else {
      if (i_looptype == ECMD_SELECTED_TARGETS_LOOP_DEFALL) {
        /* Default to all */
        i_target.posState = ECMD_TARGET_FIELD_WILDCARD;
        posType = ALL;
      } else {
        /* User didn't specify anything and we default to 0 */
        i_target.pos = 0;
        posType = SINGLE;
        i_target.posState = ECMD_TARGET_FIELD_VALID;
      }
    }
  }

  //chipUnitNum
  /* If the state is already valid we just continue on */
  if (i_target.chipUnitNumState == ECMD_TARGET_FIELD_VALID) {
    chipUnitNumType = SINGLE;

  } else if (i_target.chipUnitNumState != ECMD_TARGET_FIELD_UNUSED) {

    /* Did the user specify any chipUnitNum args */
    if (ecmdUserArgs.chipUnitNum.length()) {
      /* If the user used any sort of list 0,1,2,4 or range 2..5 then we do multi */
      if (ecmdUserArgs.chipUnitNum.find_first_of(patterns) < ecmdUserArgs.chipUnitNum.length()) {
        if (!isValidTargetString(ecmdUserArgs.chipUnitNum)) {
          dllOutputError("dllQuerySelected - chipUnitNum/core (-c#) argument contained invalid characters\n");
          return ECMD_INVALID_ARGS;
        }
        i_target.chipUnitNumState = ECMD_TARGET_FIELD_WILDCARD;
        chipUnitNumType = MULTI;
      }

      /* See if we have a single entry -c1 */
      else if (isValidTargetString(ecmdUserArgs.chipUnitNum)) {
        i_target.chipUnitNumState = ECMD_TARGET_FIELD_VALID;
        i_target.chipUnitNum = (uint32_t)atoi(ecmdUserArgs.chipUnitNum.c_str());
        chipUnitNumType = SINGLE;
      }

      /* See if the user specified -aall or -call */
      else if (ecmdUserArgs.chipUnitNum == "all") {
        i_target.chipUnitNumState = ECMD_TARGET_FIELD_WILDCARD;
        chipUnitNumType = ALL;
      }

      /* See if the user specified -aft or -cft */
      else if (ecmdUserArgs.chipUnitNum == "ft") {
        i_target.chipUnitNumState = ECMD_TARGET_FIELD_WILDCARD;
        chipUnitNumType = FT;
      }

      /* See if the user specified -alt or -clt */
      else if (ecmdUserArgs.chipUnitNum == "lt") {
        i_target.chipUnitNumState = ECMD_TARGET_FIELD_WILDCARD;
        chipUnitNumType = LT;
      }

      /* See if the user specified -aet or -cet */
      else if (ecmdUserArgs.chipUnitNum == "et") {
        i_target.chipUnitNumState = ECMD_TARGET_FIELD_WILDCARD;
        chipUnitNumType = ET;
      }

      /* See if the user specified -aot or -cot */
      else if (ecmdUserArgs.chipUnitNum == "ot") {
        i_target.chipUnitNumState = ECMD_TARGET_FIELD_WILDCARD;
        chipUnitNumType = OT;
      }

      /* See if the user specified or -c- */
      else if (ecmdUserArgs.chipUnitNum == "-") {
        dllOutputError("dllQuerySelected - argument -c- not supported\n");
        return ECMD_INVALID_ARGS;
      }

      /* Finally, we can error out */
      else {
        dllOutputError("dllQuerySelected - unknown -c options found!\n");
        return ECMD_INVALID_ARGS;
      }
    }
    /* Nothing was specified, set some defaults */
    else {
      if (i_looptype == ECMD_SELECTED_TARGETS_LOOP_DEFALL) {
        /* Default to all */
        i_target.chipUnitNumState = ECMD_TARGET_FIELD_WILDCARD;
        chipUnitNumType = ALL;
      } else {
        /* User didn't specify anything and we default to 0 */
        i_target.chipUnitNum = 0;
        chipUnitNumType = SINGLE;
        i_target.chipUnitNumState = ECMD_TARGET_FIELD_VALID;
      }
    }
  }

  //thread
  /* If the state is already valid we just continue on */
  if (i_target.threadState == ECMD_TARGET_FIELD_VALID) {
    threadType = SINGLE;

  } else if (i_target.threadState != ECMD_TARGET_FIELD_UNUSED) {

    /* Did the user specify any thread args */
    if (ecmdUserArgs.thread.length()) {
      /* If the user used any sort of list 0,1,2,4 or range 2..5 then we do multi */
      if (ecmdUserArgs.thread.find_first_of(patterns) < ecmdUserArgs.thread.length()) {
        if (!isValidTargetString(ecmdUserArgs.thread)) {
          dllOutputError("dllQuerySelected - thread (-t#) argument contained invalid characters\n");
          return ECMD_INVALID_ARGS;
        }
        i_target.threadState = ECMD_TARGET_FIELD_WILDCARD;
        threadType = MULTI;
      }

      /* See if we have a single entry -t1 */
      else if (isValidTargetString(ecmdUserArgs.thread)) {
        i_target.threadState = ECMD_TARGET_FIELD_VALID;
        i_target.thread = (uint32_t)atoi(ecmdUserArgs.thread.c_str());
        threadType = SINGLE;
      }

      /* See if the user specified -aall or -tall */
      else if (ecmdUserArgs.thread == "all" || ecmdUserArgs.thread == "alive") {
        i_target.threadState = ECMD_TARGET_FIELD_WILDCARD;
        threadType = ALL;
      }

      /* See if the user specified -aft or -tft */
      else if (ecmdUserArgs.thread == "ft") {
        i_target.threadState = ECMD_TARGET_FIELD_WILDCARD;
        threadType = FT;
      }

      /* See if the user specified -alt or -tlt */
      else if (ecmdUserArgs.thread == "lt") {
        i_target.threadState = ECMD_TARGET_FIELD_WILDCARD;
        threadType = LT;
      }

      /* See if the user specified -aet or -tet */
      else if (ecmdUserArgs.thread == "et") {
        i_target.threadState = ECMD_TARGET_FIELD_WILDCARD;
        threadType = ET;
      }

      /* See if the user specified -aot or -tot */
      else if (ecmdUserArgs.thread == "ot") {
        i_target.threadState = ECMD_TARGET_FIELD_WILDCARD;
        threadType = OT;
      }

      /* See if the user specified or -t- */
      else if (ecmdUserArgs.thread == "-") {
        dllOutputError("dllQuerySelected - argument -t- not supported\n");
        return ECMD_INVALID_ARGS;
      }

      /* Finally, we can error out */
      else {
        dllOutputError("dllQuerySelected - unknown -t options found!\n");
        return ECMD_INVALID_ARGS;
      }
    }
    /* Nothing was specified, set some defaults */
    else {
      if (i_looptype == ECMD_SELECTED_TARGETS_LOOP_DEFALL) {
        /* Default to all */
        i_target.threadState = ECMD_TARGET_FIELD_WILDCARD;
        threadType = ALL;
      } else {
        /* User didn't specify anything and we default to 0 */
        i_target.thread = 0;
        threadType = SINGLE;
        i_target.threadState = ECMD_TARGET_FIELD_VALID;
      }
    }
  }

  /* Okay, target setup as best we can, let's go out to query cnfg with it */
  if (i_existMode) {
    rc = dllQueryExist(i_target, o_queryData, ECMD_QUERY_DETAIL_LOW);
  } else {
    rc = dllQueryConfig(i_target, o_queryData, ECMD_QUERY_DETAIL_LOW);
  }
  if (rc) return rc;

#ifndef ECMD_STRIP_DEBUG
  if (ecmdGlobal_DllDebug >= 10) {
    std::string printed;
    #define frontFPPTxt "ECMD DEBUG"

    printed = frontFPPTxt " Return Value from dllQueryConfig =============\n";
    dllOutput(printed.c_str());

    std::list<ecmdCageData>::iterator ecmdCurCage;
    std::list<ecmdNodeData>::iterator ecmdCurNode;
    std::list<ecmdSlotData>::iterator ecmdCurSlot;
    std::list<ecmdChipData>::iterator ecmdCurChip;
    std::list<ecmdChipUnitData>::iterator ecmdCurChipUnit;
    std::list<ecmdThreadData>::iterator ecmdCurThread;
    char buf[100];
    if (o_queryData.cageData.empty()) {
      printed = frontFPPTxt "\t \t value = EMPTY\n"; dllOutput(printed.c_str());
    } else {

      for (ecmdCurCage = o_queryData.cageData.begin(); ecmdCurCage != o_queryData.cageData.end(); ecmdCurCage ++) {
        sprintf(buf,frontFPPTxt "\t \t k%d\n", ecmdCurCage->cageId); dllOutput(buf);
        for (ecmdCurNode = ecmdCurCage->nodeData.begin(); ecmdCurNode != ecmdCurCage->nodeData.end(); ecmdCurNode ++) {
          sprintf(buf,frontFPPTxt "\t \t   n%d\n", ecmdCurNode->nodeId); dllOutput(buf);

          for (ecmdCurSlot = ecmdCurNode->slotData.begin(); ecmdCurSlot != ecmdCurNode->slotData.end(); ecmdCurSlot ++) {
            sprintf(buf,frontFPPTxt "\t \t     s%d\n", ecmdCurSlot->slotId); dllOutput(buf);

            for (ecmdCurChip = ecmdCurSlot->chipData.begin(); ecmdCurChip != ecmdCurSlot->chipData.end(); ecmdCurChip ++) {
              sprintf(buf,frontFPPTxt "\t \t       %s:p%d\n", ecmdCurChip->chipType.c_str(), ecmdCurChip->pos); dllOutput(buf);

              for (ecmdCurChipUnit = ecmdCurChip->chipUnitData.begin(); ecmdCurChipUnit != ecmdCurChip->chipUnitData.end(); ecmdCurChipUnit ++) {
                sprintf(buf,frontFPPTxt "\t \t         %s:c%d\n", ecmdCurChipUnit->chipUnitType.c_str(), ecmdCurChipUnit->chipUnitNum); dllOutput(buf);

                for (ecmdCurThread = ecmdCurChipUnit->threadData.begin(); ecmdCurThread != ecmdCurChipUnit->threadData.end(); ecmdCurThread ++) {
                  sprintf(buf,frontFPPTxt "\t \t           t%d\n", ecmdCurThread->threadId); dllOutput(buf);
                } /* curThreadIter */

              } /* curChipUnitIter */

            } /* curChipIter */

          } /* curSlotIter */

        } /* curNodeIter */

      } /* curCageIter */
    }
  }
#endif

  // Within this loop the first for calls to removeCurrentElement()
  // generates lint msg 713 & 820: Loss of precision (arg. no. 1) 
  // So disabling that message within this scope (re-enable at end of 
  // loop).      @02a
  //lint -e713
  //lint -e820

  /* now I need to go in and clean out any excess stuff */
  std::list<ecmdCageData>::iterator curCage = o_queryData.cageData.begin();
  while (curCage != o_queryData.cageData.end()) {

    /* If cageType >= MULTI, they specified one of the special queries where items need to be removed */
    if (cageType >= MULTI) {
      /* Is the current element in the list of numbers the user provided, if not remove it */
      if (cageType == MULTI) {
        if (removeCurrentElement(curCage->cageId, ecmdUserArgs.cage)) {
          curCage = o_queryData.cageData.erase(curCage);
          continue;
        }
      }
      /* Is the current element in the list is first, keep it.  Otherwise, remove */
      else if (cageType == FT) {
        if (curCage != o_queryData.cageData.begin()) {
          curCage = o_queryData.cageData.erase(curCage);
          continue;
        }
      }
      /* Is the current element in the list is last, keep it.  Otherwise, remove */
      else if (cageType == LT) {
        std::list<ecmdCageData>::iterator lastCage = curCage;
        lastCage++;
        if (lastCage != o_queryData.cageData.end()) {
          curCage = o_queryData.cageData.erase(curCage);
          continue;
        }
      }
      /* If the current element in the list is even, keep it.  Otherwise, remove */
      else if (cageType == ET) {
        if ((curCage->cageId % 2) == 1) {
          curCage = o_queryData.cageData.erase(curCage);
          continue;
        }
      }
      /* If the current element in the list is odd, keep it.  Otherwise, remove */
      else if (cageType == OT) {
        if ((curCage->cageId % 2) == 0) {
          curCage = o_queryData.cageData.erase(curCage);
          continue;
        }
      }
    }

    /* Walk through the nodes */
    std::list<ecmdNodeData>::iterator curNode = curCage->nodeData.begin();
    while (curNode != curCage->nodeData.end()) {

      /* If nodeType >= MULTI, they specified one of the special queries where items need to be removed */
      if (nodeType >= MULTI) {
        /* Is the current element in the list of numbers the user provided, if not remove it */
        if (nodeType == MULTI) {
          if (removeCurrentElement(curNode->nodeId, ecmdUserArgs.node)) {
            curNode = curCage->nodeData.erase(curNode);
            continue;
          }
        }
        /* Is the current element in the list is first, keep it.  Otherwise, remove */
        else if (nodeType == FT) {
          if (curNode != curCage->nodeData.begin()) {
            curNode = curCage->nodeData.erase(curNode);
            continue;
          }
        }
        /* Is the current element in the list is last, keep it.  Otherwise, remove */
        else if (nodeType == LT) {
          std::list<ecmdNodeData>::iterator lastNode = curNode;
          lastNode++;
          if (lastNode != curCage->nodeData.end()) {
            curNode = curCage->nodeData.erase(curNode);
            continue;
          }
        }
        /* If the current element in the list is even, keep it.  Otherwise, remove */
        else if (nodeType == ET) {
          if ((curNode->nodeId % 2) == 1) {
            curNode = curCage->nodeData.erase(curNode);
            continue;
          }
        }
        /* If the current element in the list is odd, keep it.  Otherwise, remove */
        else if (nodeType == OT) {
          if ((curNode->nodeId % 2) == 0) {
            curNode = curCage->nodeData.erase(curNode);
            continue;
          }
        }
      }

      /* Walk through the slots */
      std::list<ecmdSlotData>::iterator curSlot = curNode->slotData.begin();
      while (curSlot != curNode->slotData.end()) {

        /* If slotType >= MULTI, they specified one of the special queries where items need to be removed */
        if (slotType >= MULTI) {
          /* Is the current element in the list of numbers the user provided, if not remove it */
          if (slotType == MULTI) {
            if (removeCurrentElement(curSlot->slotId, ecmdUserArgs.slot)) {
              curSlot = curNode->slotData.erase(curSlot);
              continue;
            }
          }
          /* Is the current element in the list is first, keep it.  Otherwise, remove */
          else if (slotType == FT) {
            if (curSlot != curNode->slotData.begin()) {
              curSlot = curNode->slotData.erase(curSlot);
              continue;
            }
          }
          /* Is the current element in the list is last, keep it.  Otherwise, remove */
          else if (slotType == LT) {
            std::list<ecmdSlotData>::iterator lastSlot = curSlot;
            lastSlot++;
            if (lastSlot != curNode->slotData.end()) {
              curSlot = curNode->slotData.erase(curSlot);
              continue;
            }
          }
          /* If the current element in the list is even, keep it.  Otherwise, remove */
          else if (slotType == ET) {
            if ((curSlot->slotId % 2) == 1) {
              curSlot = curNode->slotData.erase(curSlot);
              continue;
            }
          }
          /* If the current element in the list is odd, keep it.  Otherwise, remove */
          else if (slotType == OT) {
            if ((curSlot->slotId % 2) == 0) {
              curSlot = curNode->slotData.erase(curSlot);
              continue;
            }
          }
        }

        /* Walk through all the chip positions */
        std::list<ecmdChipData>::iterator curChip = curSlot->chipData.begin();
        while (curChip != curSlot->chipData.end()) {

          /* If posType >= MULTI, they specified one of the special queries where items need to be removed */
          if (posType >= MULTI) {
            /* Is the current element in the list of numbers the user provided, if not remove it */
            if (posType == MULTI) {
              if (removeCurrentElement(curChip->pos, ecmdUserArgs.pos)) {
                curChip = curSlot->chipData.erase(curChip);
                continue;
              }
            }
            /* Is the current element in the list is first, keep it.  Otherwise, remove */
            else if (posType == FT) {
              if (curChip != curSlot->chipData.begin()) {
                curChip = curSlot->chipData.erase(curChip);
                continue;
              }
            }
            /* Is the current element in the list is last, keep it.  Otherwise, remove */
            else if (posType == LT) {
              std::list<ecmdChipData>::iterator lastChip = curChip;
              lastChip++;
              if (lastChip != curSlot->chipData.end()) {
                curChip = curSlot->chipData.erase(curChip);
                continue;
              }
            }
            /* If the current element in the list is even, keep it.  Otherwise, remove */
            else if (posType == ET) {
              if ((curChip->pos % 2) == 1) {
                curChip = curSlot->chipData.erase(curChip);
                continue;
              }
            }
            /* If the current element in the list is odd, keep it.  Otherwise, remove */
            else if (posType == OT) {
              if ((curChip->pos % 2) == 0) {
                curChip = curSlot->chipData.erase(curChip);
                continue;
              }
            }
          }

          /* Walk through all the chipUnits */
          std::list<ecmdChipUnitData>::iterator curChipUnit = curChip->chipUnitData.begin();
          while (curChipUnit != curChip->chipUnitData.end()) {

            /* If chipUnitNumType >= MULTI, they specified one of the special queries where items need to be removed */
            if (chipUnitNumType >= MULTI) {
              /* Is the current element in the list of numbers the user provided, if not remove it */
              if (chipUnitNumType == MULTI) {
                if (removeCurrentElement(curChipUnit->chipUnitNum, ecmdUserArgs.chipUnitNum)) {
                  curChipUnit = curChip->chipUnitData.erase(curChipUnit);
                  continue;
                }
              }
              /* Is the current element in the list is first, keep it.  Otherwise, remove */
              else if (chipUnitNumType == FT) {
                if (curChipUnit != curChip->chipUnitData.begin()) {
                  curChipUnit = curChip->chipUnitData.erase(curChipUnit);
                  continue;
                }
              }
              /* Is the current element in the list is last, keep it.  Otherwise, remove */
              else if (chipUnitNumType == LT) {
                std::list<ecmdChipUnitData>::iterator lastChipUnit = curChipUnit;
                lastChipUnit++;
                if (lastChipUnit != curChip->chipUnitData.end()) {
                  curChipUnit = curChip->chipUnitData.erase(curChipUnit);
                  continue;
                }
              }
              /* If the current element in the list is even, keep it.  Otherwise, remove */
              else if (chipUnitNumType == ET) {
                if ((curChipUnit->chipUnitNum % 2) == 1) {
                  curChipUnit = curChip->chipUnitData.erase(curChipUnit);
                  continue;
                }
              }
              /* If the current element in the list is odd, keep it.  Otherwise, remove */
              else if (chipUnitNumType == OT) {
                if ((curChipUnit->chipUnitNum % 2) == 0) {
                  curChipUnit = curChip->chipUnitData.erase(curChipUnit);
                  continue;
                }
              }
            }

            /* Walk through the threads */
            std::list<ecmdThreadData>::iterator curThread = curChipUnit->threadData.begin();
            while (curThread != curChipUnit->threadData.end()) {

              /* If threadType >= MULTI, they specified one of the special queries where items need to be removed */
              if (threadType >= MULTI) {
                /* Is the current element in the list of numbers the user provided, if not remove it */
                if (threadType == MULTI) {
                  if (removeCurrentElement(curThread->threadId, ecmdUserArgs.thread)) {
                    curThread = curChipUnit->threadData.erase(curThread);
                    continue;
                  }
                }
                /* Is the current element in the list is first, keep it.  Otherwise, remove */
                else if (threadType == FT) {
                  if (curThread != curChipUnit->threadData.begin()) {
                    curThread = curChipUnit->threadData.erase(curThread);
                    continue;
                  }
                }
                /* Is the current element in the list is last, keep it.  Otherwise, remove */
                else if (threadType == LT) {
                  std::list<ecmdThreadData>::iterator lastThread = curThread;
                  lastThread++;
                  if (lastThread != curChipUnit->threadData.end()) {
                    curThread = curChipUnit->threadData.erase(curThread);
                    continue;
                  }
                }
                /* If the current element in the list is even, keep it.  Otherwise, remove */
                else if (threadType == ET) {
                  if ((curThread->threadId % 2) == 1) {
                    curThread = curChipUnit->threadData.erase(curThread);
                    continue;
                  }
                }
                /* If the current element in the list is odd, keep it.  Otherwise, remove */
                else if (threadType == OT) {
                  if ((curThread->threadId % 2) == 0) {
                    curThread = curChipUnit->threadData.erase(curThread);
                    continue;
                  }
                }
              }

              curThread++;
            }  /* while curThread */
            if ((i_target.threadState != ECMD_TARGET_FIELD_UNUSED) &&
                curChipUnit->threadData.empty()) {
              curChipUnit = curChip->chipUnitData.erase(curChipUnit);
            } else {
              curChipUnit++;
            }
          }  /* while curChipUnit */

          if ((i_target.chipUnitNumState != ECMD_TARGET_FIELD_UNUSED) &&
              curChip->chipUnitData.empty()) {
            curChip = curSlot->chipData.erase(curChip);
          } else {
            curChip++;
          }
        }  /* while curChip */

        /* Let's check to make sure there is something left here after we removed everything */
        if (((i_target.chipTypeState != ECMD_TARGET_FIELD_UNUSED) ||
             (i_target.posState != ECMD_TARGET_FIELD_UNUSED)) &&
            curSlot->chipData.empty()) {
          curSlot = curNode->slotData.erase(curSlot);
        } else {
          curSlot++;
        }
      }  /* while curSlot */

      /* Let's check to make sure there is something left here after we removed everything */
      if ((i_target.slotState != ECMD_TARGET_FIELD_UNUSED) &&
          curNode->slotData.empty()) {
        curNode = curCage->nodeData.erase(curNode);
      } else {
        curNode++;
      }
    }  /* while curNode */

    /* Let's check to make sure there is something left here after we removed everything */
    if ((i_target.nodeState != ECMD_TARGET_FIELD_UNUSED) &&
        curCage->nodeData.empty()) {
      curCage = o_queryData.cageData.erase(curCage);
    } else {
      curCage++;
    }
  }  /* while curCage */

  //lint +e713   @02a
  //lint +e820   @02a

  return rc;
}

uint32_t dllCommonCommandArgs(int*  io_argc, char** io_argv[]) {
  uint32_t rc = ECMD_SUCCESS;

  /* We need to pull out the targeting options here, and
   store them away for future use */
  char * curArg = ecmdParseOptionWithArgs(io_argc, io_argv, "-trace=");

  //-trace
  // Moved assignment of 'curArg' up to declaration stmt for lint filter.  @02c
  //  if ((curArg = ecmdParseOptionWithArgs(io_argc, io_argv, "-trace="))) {
  if (curArg) {
    /* Grab all the tokens */
    std::vector<std::string> tokens;
    std::vector<std::string>::iterator tokit;

    ecmdParseTokens(curArg," \t\n,", tokens);
    for (tokit = tokens.begin(); tokit != tokens.end(); tokit ++) {
      if (!strcmp(tokit->c_str(), "scan")) {
        dllSetTraceMode(ECMD_TRACE_SCAN,true);
      } else if (!strcmp(tokit->c_str(), "prcd")) {
        dllSetTraceMode(ECMD_TRACE_PROCEDURE,true);
      } else {
        dllOutputWarning(("dllCommonCommandArgs - Unknown Trace type detected '" + *tokit + "' : Ignoring\n").c_str());
      }

    }
  }

  /* Grab the quiet mode flag */
  if (ecmdParseOption(io_argc, io_argv, "-quiet")) {
    ecmdGlobal_quiet = 1;
  } else {
    /* Check for quiet mode env variable */
    const char * quiet_mode = getenv("ECMD_QUIETMODE");
    if ((quiet_mode != NULL) && (strncmp("quiet", quiet_mode, 5) == 0)) {
      ecmdGlobal_quiet = 1;
    }
  }

  /* Grab the quiet error mode flag */
  if (ecmdParseOption(io_argc, io_argv, "-quieterror")) {
    ecmdGlobal_quietError = 1;
  }

  /* Grab the coe mode flag */
  if (ecmdParseOption(io_argc, io_argv, "-coe")) {
    ecmdGlobal_continueOnError = 1;
  }

  /* Grab the exist loop flag */
  if (ecmdParseOption(io_argc, io_argv, "-exist")) {
    ecmdGlobal_looperMode = ECMD_EXIST_LOOP;
  }


  /*************************************/
  /* Parse command line targeting args */
  /*************************************/

  /* This is left in for backwards comptability, preference is to use the option below */
  bool allFound = false;
  if (ecmdParseOption(io_argc, io_argv, "-all")) {
    ecmdUserArgs.cage = "all";
    ecmdUserArgs.node = "all";
    ecmdUserArgs.slot = "all";
    ecmdUserArgs.pos = "all";
    ecmdUserArgs.chipUnitNum = "all";
    ecmdUserArgs.thread = "all";
    allFound = true;
  }

  //all targets
  curArg = ecmdParseOptionWithArgs(io_argc, io_argv, "-a");
  if (curArg) {
    ecmdUserArgs.cage = curArg;
    ecmdUserArgs.node = curArg;
    ecmdUserArgs.slot = curArg;
    ecmdUserArgs.pos = curArg;
    ecmdUserArgs.chipUnitNum = curArg;
    ecmdUserArgs.thread = curArg;
    allFound = true;
  }

  
  // Target Short-Hand:  For -k, -n, -s, -p, -c, -t options, look for ':' in case multiple
  //  target fields were put together in 1 arg
  uint32_t l_find = 0;
  std::string l_tmp_string;

  //cage - the "-k" was Larry's idea, I just liked it - 
  curArg = ecmdParseOptionWithArgs(io_argc, io_argv, "-k");
  if (curArg) {
    if (allFound) {
      dllOutputError("dllCommonCommandArgs - Cannot specify -a target parm and -k at the same time\n");
      return ECMD_INVALID_ARGS;
    } else  {
      // For target expansion, look for ':'
      l_tmp_string = curArg;
      l_find = l_tmp_string.find_first_of(":");
      if (l_find == std::string::npos) {
        // No ":" found - just set ecmdUserArgs target directly
        ecmdUserArgs.cage = curArg;
      } else {
        // Found ":"; Make sure there's something before first ':'
        if (l_find == 0) {
          dllOutputError("dllCommonCommandArgs - No Target Info Found Before First ':'\n");
          return ECMD_INVALID_ARGS;
        } else {
          // Call Expansion function to process arg with ':'
          rc = ecmdTargetExpansion(l_tmp_string, "-k");
          if (rc) return rc;
        }
      }
    }
  }

  //node
  curArg = ecmdParseOptionWithArgs(io_argc, io_argv, "-n");
  if (curArg) {
    if (allFound) {
      dllOutputError("dllCommonCommandArgs - Cannot specify -a target parm and -n at the same time\n");
      return ECMD_INVALID_ARGS;
    } else {
      // For target expansion, look for ':'
      l_tmp_string = curArg;
      l_find = l_tmp_string.find_first_of(":");
      if (l_find == std::string::npos) {
        // No ":" found - just set ecmdUserArgs target directly
        ecmdUserArgs.node = curArg;
      } else {
        // Found ":"; Make sure there's something before first ':'
        if (l_find == 0) {
          dllOutputError("dllCommonCommandArgs - No Target Info Found Before First ':'\n");
          return ECMD_INVALID_ARGS;
        } else {
          // Call Expansion function to process arg with ':'
          rc = ecmdTargetExpansion(l_tmp_string, "-n");
          if (rc) return rc;
        }
      }
    }
  }

  //slot
  curArg = ecmdParseOptionWithArgs(io_argc, io_argv, "-s");
  if (curArg) {
    if (allFound) {
      dllOutputError("dllCommonCommandArgs - Cannot specify -a target parm and -s at the same time\n");
      return ECMD_INVALID_ARGS;
    } else {
      // For target expansion, look for ':'
      l_tmp_string = curArg;
      l_find = l_tmp_string.find_first_of(":");
      if (l_find == std::string::npos) {
        // No ":" found - just set ecmdUserArgs target directly
        ecmdUserArgs.slot = curArg;
      } else {
        // Found ":"; Make sure there's something before first ':'
        if (l_find == 0) {
          dllOutputError("dllCommonCommandArgs - No Target Info Found Before First ':'\n");
          return ECMD_INVALID_ARGS;
        } else {
          // Call Expansion function to process arg with ':'
          rc = ecmdTargetExpansion(l_tmp_string, "-s");
          if (rc) return rc;
        }
      }
    }
  }

  //position
  curArg = ecmdParseOptionWithArgs(io_argc, io_argv, "-p");
  if (curArg) {
    if (allFound) {
      dllOutputError("dllCommonCommandArgs - Cannot specify -a target parm and -p at the same time\n");
      return ECMD_INVALID_ARGS;
    } else {
      // For target expansion, look for ':'
      l_tmp_string = curArg;
      l_find = l_tmp_string.find_first_of(":");
      if (l_find == std::string::npos) {
        // No ":" found - just set ecmdUserArgs target directly
        ecmdUserArgs.pos = curArg;
      } else {
        // Found ":"; Make sure there's something before first ':'
        if (l_find == 0) {
          dllOutputError("dllCommonCommandArgs - No Target Info Found Before First ':'\n");
          return ECMD_INVALID_ARGS;
        } else {
          // Call Expansion function to process arg with ':'
          rc = ecmdTargetExpansion(l_tmp_string, "-p");
          if (rc) return rc;
        }
      }
    }
  }

  //chipUnit
  curArg = ecmdParseOptionWithArgs(io_argc, io_argv, "-c");
  if (curArg) {
    if (allFound) {
      dllOutputError("dllCommonCommandArgs - Cannot specify -a target parm and -c at the same time\n");
      return ECMD_INVALID_ARGS;
    } else {
      // For target expansion, look for ':'
      l_tmp_string = curArg;
      l_find = l_tmp_string.find_first_of(":");
      if (l_find == std::string::npos) {
        // No ":" found - just set ecmdUserArgs target directly
        ecmdUserArgs.chipUnitNum = curArg;
      } else {
        // Found ":"; Make sure there's something before first ':'
        if (l_find == 0) {
          dllOutputError("dllCommonCommandArgs - No Target Info Found Before First ':'\n");
          return ECMD_INVALID_ARGS;
        } else {
          // Call Expansion function to process arg with ':'
          rc = ecmdTargetExpansion(l_tmp_string, "-c");
          if (rc) return rc;
        }
      }
    }
  }

  //thread
  curArg = ecmdParseOptionWithArgs(io_argc, io_argv, "-t");
  if (curArg) {
    if (allFound) {
      dllOutputError("dllCommonCommandArgs - Cannot specify -a target parm and -t at the same time\n");
      return ECMD_INVALID_ARGS;
    } else {
      // For target expansion, look for ':'
      l_tmp_string = curArg;
      l_find = l_tmp_string.find_first_of(":");
      if (l_find == std::string::npos) {
        // No ":" found - just set ecmdUserArgs target directly
        ecmdUserArgs.thread = curArg;
      } else {
        // Found ":"; Make sure there's something before first ':'
        if (l_find == 0) {
          dllOutputError("dllCommonCommandArgs - No Target Info Found Before First ':'\n");
          return ECMD_INVALID_ARGS;
        } else {
          // Call Expansion function to process arg with ':'
          rc = ecmdTargetExpansion(l_tmp_string, "-t");
          if (rc) return rc;
        }
      }
    }
  }


  /* Call the dllSpecificFunction */
  rc = dllSpecificCommandArgs(io_argc,io_argv);

  return rc;
}

/* @brief used by dllCommonCommandArgs when ":" found, sets ecmdUserArgs */
uint32_t ecmdTargetExpansion(std::string arg_string , const char * input_target) {
  uint32_t rc = ECMD_SUCCESS;
  std::string l_tmp_string;

  // We know curArg has ':', so tokenize the string
  std::vector<std::string> tokens;
  std::vector<std::string>::iterator tokit;
  ecmdParseTokens(arg_string,":", tokens);

  for (tokit = tokens.begin(); tokit != tokens.end(); tokit ++) {

    if (tokit == tokens.begin()) {

      // the first arg belongs to the input_target passed in
      if (!strcmp(input_target, "-k")) ecmdUserArgs.cage=tokit->c_str();
      else if (!strcmp(input_target, "-n")) ecmdUserArgs.node=tokit->c_str();
      else if (!strcmp(input_target, "-s")) ecmdUserArgs.slot=tokit->c_str();
      else if (!strcmp(input_target, "-p")) ecmdUserArgs.pos=tokit->c_str();
      else if (!strcmp(input_target, "-c")) ecmdUserArgs.chipUnitNum=tokit->c_str();
      else if (!strcmp(input_target, "-t")) ecmdUserArgs.thread=tokit->c_str();

    } else {

      // Set this here for string operations below
      l_tmp_string=tokit->c_str();

      // Look for additional targets
      // If found, remove first char of l_tmp_string and then set ecmdUserArgs

      if (!strncmp(tokit->c_str(), "k", 1)) {
        l_tmp_string.erase(0,1);          
        ecmdUserArgs.cage = l_tmp_string;
      } else if (!strncmp(tokit->c_str(), "n", 1)) {
        l_tmp_string.erase(0,1);
        ecmdUserArgs.node = l_tmp_string;
      } else if (!strncmp(tokit->c_str(), "s", 1)) {
        l_tmp_string.erase(0,1);
        ecmdUserArgs.slot = l_tmp_string;
      } else if (!strncmp(tokit->c_str(), "p", 1)) {
        l_tmp_string.erase(0,1);
        ecmdUserArgs.pos = l_tmp_string;
      } else if (!strncmp(tokit->c_str(), "c", 1)) {
        l_tmp_string.erase(0,1);
        ecmdUserArgs.chipUnitNum = l_tmp_string;
      } else if (!strncmp(tokit->c_str(), "t", 1)) {
        l_tmp_string.erase(0,1);
        ecmdUserArgs.thread = l_tmp_string;
      } else {
        dllOutputError("ecmdTargetExpansion - Found non-target data after ':'\n");
        return ECMD_INVALID_ARGS;
      }
    }  // end of if (tokit == tokens.begin() ) check
  }  // end of tokens for loop

  return rc;
}



void dllPushCommandArgs() {
  ecmdArgsStack.push_back(ecmdUserArgs);
  ecmdUserArgs.cage = ecmdUserArgs.node = ecmdUserArgs.slot = ecmdUserArgs.pos = ecmdUserArgs.chipUnitNum = ecmdUserArgs.thread = "";
}


void dllPopCommandArgs() {
  if (!ecmdArgsStack.empty()) {
    ecmdUserArgs = ecmdArgsStack.back();
    ecmdArgsStack.pop_back();
  }
}


uint8_t removeCurrentElement (int curPos, std::string userArgs) {
  uint8_t l_remove = 1;
  std::string curSubstr;
  size_t curOffset = 0;
  size_t nextOffset = 0;
  size_t tmpOffset = 0;

  while (curOffset <  userArgs.length()) {

    nextOffset = userArgs.find(',',curOffset);
    if (nextOffset == std::string::npos) {
      nextOffset = userArgs.length();
    }

    curSubstr = userArgs.substr(curOffset, nextOffset - curOffset);

    if (( tmpOffset = curSubstr.find("..",0)) < curSubstr.length()) {

      int lowerBound = atoi(curSubstr.substr(0,tmpOffset).c_str());
      int upperBound = atoi(curSubstr.substr(tmpOffset+2, curSubstr.length()).c_str());

      if (lowerBound <= curPos && curPos <= upperBound) {
        l_remove = 0;
        break;
      }
    }
    else {

      int curValidPos = atoi(curSubstr.c_str());
      if (curValidPos == curPos) {
        l_remove = 0;
        break;
      }
    }

    curOffset = nextOffset+1;

  }

  return l_remove;
}

/* Returns true if all chars of str are decimal numbers */
bool isValidTargetString(std::string &str) {

  bool ret = true;

  /* This is the code that allows hex input positions on the command line */
  /* Since all of the cmdline target args are checked via this function, it's the easiest place */
  /* to convert any hex numbers to decimal.  This will allow all the rest of the code after this */
  /* point to behave as it currently does with decimal numbers - JTA 09/11/07 */
  size_t startPos = str.find("0x");
  size_t endPos;
  char decimalString[10];
  size_t matches, decimalNum;

  while (startPos != std::string::npos) {
    endPos = str.find_first_of(",.", startPos);  // These are the special seperators
    matches = sscanf(str.substr((startPos+2),endPos).c_str(),"%zx",&decimalNum);
    if (!matches) { // sscanf didn't find anything
      return false;
    }
    /* Turn our number into a string and stick it back into the target string */
    sprintf(decimalString,"%zd",decimalNum);
    str.replace(startPos, (endPos - startPos), decimalString);

    /* Find the next one, starting at the end of our replace */
    startPos = str.find("0x", (startPos+strlen(decimalString)));
  }

  for (uint32_t x = 0; x < str.length(); x ++) {
    if (isdigit(str[x])) {
    } else if (str[x] == ',') {
    } else if (str[x] == '.' && str[x+1] == '.') {
      x++;
    } else {
      ret = false;
      break;
    }
  }

  return ret;
}

uint32_t dllGetGlobalVar(ecmdGlobalVarType_t i_type) {

  uint32_t ret = 0;

  if (i_type == ECMD_GLOBALVAR_DEBUG) {
#ifndef ECMD_STRIP_DEBUG
    ret = ecmdGlobal_DllDebug;
#endif
  } else if (i_type == ECMD_GLOBALVAR_QUIETMODE) {
    ret = ecmdGlobal_quiet;
  } else if (i_type == ECMD_GLOBALVAR_QUIETERRORMODE) {
    ret = ecmdGlobal_quietError;
  } else if (i_type == ECMD_GLOBALVAR_COEMODE) {
    ret = ecmdGlobal_continueOnError;
  } else if (i_type == ECMD_GLOBALVAR_LOOPMODE) {
    ret = ecmdGlobal_looperMode;
  } else if (i_type == ECMD_GLOBALVAR_CMDLINEMODE) {
    ret = ecmdGlobal_cmdLineMode;
  }

  return ret;
}

uint32_t dllSetGlobalVar(ecmdGlobalVarType_t i_type, uint32_t i_value) {

  uint32_t rc = ECMD_SUCCESS;

  if (i_type == ECMD_GLOBALVAR_DEBUG) {
#ifndef ECMD_STRIP_DEBUG
    ecmdGlobal_DllDebug = i_value;
#endif
  } else if (i_type == ECMD_GLOBALVAR_QUIETMODE) {
    ecmdGlobal_quiet = i_value;
  } else if (i_type == ECMD_GLOBALVAR_QUIETERRORMODE) {
    ecmdGlobal_quietError = i_value;
  } else if (i_type == ECMD_GLOBALVAR_COEMODE) {
    ecmdGlobal_continueOnError = i_value;
  } else if (i_type == ECMD_GLOBALVAR_LOOPMODE) {
    ecmdGlobal_looperMode = i_value;
  } else if (i_type == ECMD_GLOBALVAR_CMDLINEMODE) {
    ecmdGlobal_cmdLineMode = i_value;
  } else {
    return ECMD_INVALID_ARGS;
  }

  return rc;
}

#ifndef ECMD_REMOVE_LATCH_FUNCTIONS
uint32_t dllQueryLatch(const ecmdChipTarget & target, std::list<ecmdLatchData> & o_queryData, ecmdLatchMode_t i_mode, const char * i_latchName,
		       const char * i_ringName, ecmdQueryDetail_t i_detail) {
  uint32_t rc = 0;

  ecmdLatchBufferEntry curEntry;
  std::list< ecmdLatchEntry >::iterator curLatchInfo;    ///< Iterator for walking through latches
  ecmdLatchData curLatchData;                           ///< Data to load into return list
  std::list<ecmdRingData> o_ringData;                   ///< Data from QueryRing
  std::string curLatch = "";                            ///< current unique latchname
  std::string printed;

  o_queryData.clear();       // Flush out current list
  
  if ((i_ringName == NULL) && (i_latchName == NULL)) {
    rc = ECMD_INVALID_ARGS;
    printed = "Input parameters Ringname and Latchname both cannot be null.\n";
    dllRegisterErrorMsg(rc, "dllQueryLatch", printed.c_str() );
    return rc;
  }
  if ((i_ringName != NULL) && (i_latchName == NULL)) {
    rc = readScandefHash(target, i_ringName, i_latchName, curEntry);
    if (rc) return rc;
  } else {
    if( i_mode == ECMD_LATCHMODE_FULL) {
      rc = readScandefHash(target, i_ringName, i_latchName, curEntry);
      if( rc && (((rc != ECMD_UNKNOWN_FILE) &&(rc != ECMD_UNABLE_TO_OPEN_SCANDEFHASH)) && ((rc == ECMD_INVALID_LATCHNAME)||(rc == ECMD_INVALID_RING)||(rc == ECMD_SCANDEFHASH_MULT_RINGS)))) {
        return rc;
      }
    }
    if (rc || (i_mode != ECMD_LATCHMODE_FULL)) {
      std::string errorParse;
      if (rc)
      {
        errorParse = dllGetErrorMsg(rc, false, true, true);
        errorParse = "WARNING: Latch is found in Scandef File but is missing in Hashfile \n" + errorParse;

      }

      rc = readScandef(target, i_ringName, i_latchName, i_mode, curEntry);
      if (rc) return rc;

      if (errorParse.length() > 0) {

       dllOutputWarning(errorParse.c_str());
      }

    }
  }

  for (curLatchInfo = curEntry.entry.begin(); curLatchInfo != curEntry.entry.end(); curLatchInfo++) {

    if (curLatch != curLatchInfo->latchName.substr(0, curLatchInfo->latchName.rfind('('))) {
      //Assumption- should all be under one ring
      if ( curLatchInfo == curEntry.entry.begin()) {
        rc = dllQueryRing(target, o_ringData, curLatchInfo->ringName.c_str(), i_detail);
        if (rc) {
	  dllRegisterErrorMsg(rc, "dllQueryLatch", "Problems querying ring from chip\n");
	  return rc;
        }
        curLatchData.ringName = curLatchInfo->ringName;
	if (!o_ringData.empty()) {
	if (i_detail == ECMD_QUERY_DETAIL_HIGH) {
	  curLatchData.clockDomain = o_ringData.begin()->clockDomain;
	  curLatchData.clockState = o_ringData.begin()->clockState;
	}
        curLatchData.isChipUnitRelated = o_ringData.begin()->isChipUnitRelated;
        curLatchData.relatedChipUnit = o_ringData.begin()->relatedChipUnit;
        curLatchData.relatedChipUnitShort = o_ringData.begin()->relatedChipUnitShort;
        }
      } else {
	o_queryData.push_back(curLatchData);
      }

      curLatch = curLatchData.latchName = curLatchInfo->latchName.substr(0, curLatchInfo->latchName.rfind('('));  
      curLatchData.bitLength = (int)curLatchInfo->length;
    } else {
      curLatchData.bitLength += (int)curLatchInfo->length;
    }
  }
  o_queryData.push_back(curLatchData);//push the last entry
  
  return rc;
}

uint32_t dllQueryLatchInfo(const ecmdChipTarget & i_target, ecmdLatchQueryData & o_queryData, ecmdLatchMode_t i_mode, const char * i_latchName, const char * i_ringName, ecmdQueryDetail_t i_detail)
{
    uint32_t rc = 0;
    ecmdLatchBufferEntry curEntry;
    std::list< ecmdLatchEntry >::iterator curLatchInfo;   ///< Iterator for walking through latches
    std::list< ecmdLatchEntry > latchInfo;                ///< Latch info from scandef to return 
    std::list<ecmdRingData> o_ringData;                   ///< Data from QueryRing
    std::string curLatch = "";                            ///< current unique latchname
    std::string printed;
    uint32_t bustype = 0;
    
    /* Let's find out if we are JTAG of FSI here */
    rc = dllGetScandefOrder(i_target, bustype);
    if (rc) {
        printed = "Problems retrieving chip information on target\n";
        dllRegisterErrorMsg(rc, "dllQueryLatchInfo", printed.c_str() );
        return rc;
    }
    /* Now make sure the plugin gave us some bus info */
    if ((bustype != ECMD_CHIPFLAG_JTAG) && (bustype != ECMD_CHIPFLAG_FSI) ) {
        dllRegisterErrorMsg(ECMD_DLL_INVALID, "dllQueryLatchInfo", "eCMD plugin returned an invalid bustype in dllGetScandefOrder\n");
        return ECMD_DLL_INVALID;
    }
    
    if ((i_ringName == NULL) && (i_latchName == NULL)) {
        rc = ECMD_INVALID_ARGS;
        printed = "Input parameters Ringname and Latchname both cannot be null.\n";
        dllRegisterErrorMsg(rc, "dllQueryLatchInfo", printed.c_str() );
        return rc;
    }
    if ((i_ringName != NULL) && (i_latchName == NULL)) {
        rc = readScandefHash(i_target, i_ringName, i_latchName, curEntry);
        if (rc) return rc;
    } else {
        if( i_mode == ECMD_LATCHMODE_FULL) {
            rc = readScandefHash(i_target, i_ringName, i_latchName, curEntry);
            if( rc && (((rc != ECMD_UNKNOWN_FILE) &&(rc != ECMD_UNABLE_TO_OPEN_SCANDEFHASH)) && ((rc == ECMD_INVALID_LATCHNAME)||(rc == ECMD_INVALID_RING)||(rc == ECMD_SCANDEFHASH_MULT_RINGS)))) {
                return rc;
            }
        }
        if (rc || (i_mode != ECMD_LATCHMODE_FULL)) {
            std::string errorParse;
            if (rc)
                {
                    errorParse = dllGetErrorMsg(rc, false, true, true);
                    errorParse = "WARNING: Latch is found in Scandef File but is missing in Hashfile \n" + errorParse;
                    
                }
            
            rc = readScandef(i_target, i_ringName, i_latchName, i_mode, curEntry);
            if (rc) return rc;
            
            if (errorParse.length() > 0) {
                
                dllOutputWarning(errorParse.c_str());
            }           
        }
    }

    // Return the latch entry from the scandef 
    o_queryData.scandefLatchInfo = curEntry.entry;

    /* Single exit point */
    while (1) {

        uint32_t bitsToFetch = ECMD_UNSET;       // Grab all bits
        uint32_t curLatchBit = ECMD_UNSET;       // This is the actual latch bit we are looking for next
        uint32_t curBufferBit = 0;               // Current bit to insert into buffered register
        uint32_t curBitsToFetch = bitsToFetch;   // This is the total number of bits left to fetch
        uint32_t dataStartBit = ECMD_UNSET;      // Actual start bit of buffered register
        uint32_t dataEndBit = ECMD_UNSET;        // Actual end bit of buffered register
        std::string latchname = "";
        uint32_t fsi_end = 0;
        uint32_t jtag_end = 0;
        
        for (curLatchInfo = curEntry.entry.begin(); (curLatchInfo != curEntry.entry.end()) && (curBitsToFetch > 0); curLatchInfo++) {
            
            if (curLatch != curLatchInfo->latchName.substr(0, curLatchInfo->latchName.rfind('('))) {
                //Assumption- should all be under one ring
                if ( curLatchInfo == curEntry.entry.begin()) {
                    rc = dllQueryRing(i_target, o_ringData, curLatchInfo->ringName.c_str(), i_detail);
                    if (rc) {
                        dllRegisterErrorMsg(rc, "dllQueryLatchInfo", "Problems querying ring from chip\n");
                        return rc;
                    }
                    if (!o_ringData.empty()) {
                        if (i_detail == ECMD_QUERY_DETAIL_HIGH) {
                            o_queryData.clockDomain = o_ringData.begin()->clockDomain;
                            o_queryData.clockState = o_ringData.begin()->clockState;
                        }
                        o_queryData.isChipUnitRelated = o_ringData.begin()->isChipUnitRelated;
                        o_queryData.relatedChipUnit = o_ringData.begin()->relatedChipUnit;
                        o_queryData.relatedChipUnitShort = o_ringData.begin()->relatedChipUnitShort;
                    }
                }
                
                curLatch = curLatchInfo->latchName.substr(0, curLatchInfo->latchName.rfind('('));  
            }
            
            /* Do we have previous data here , or some missing bits in the scandef latchs ?*/
            if (((dataStartBit != ECMD_UNSET) && (curLatchBit !=  curLatchInfo->latchStartBit) && (curLatchBit !=  curLatchInfo->latchEndBit)) ||
                ((latchname == "") || (latchname != curLatchInfo->latchName.substr(0, curLatchInfo->latchName.rfind('('))))) {
                /* I have some good data here */
                if (latchname != "") {
                    
                    o_queryData.latchStartBit = (int)dataStartBit;
                    o_queryData.latchEndBit = (int)dataEndBit;
                    o_queryData.bitLength = dataEndBit - dataStartBit + 1;
                    o_queryData.fsiEndRingOffset = fsi_end;
                    o_queryData.jtagEndRingOffset = jtag_end;  
                }
            
                /* If this is a fresh one we need to reset everything */
                if ((latchname == "") || (latchname != curLatchInfo->latchName.substr(0, latchname.length()))) {
                    dataStartBit = dataEndBit = ECMD_UNSET;
                    curBitsToFetch = ECMD_UNSET;
                    curBufferBit = 0;
                    latchname = curLatchInfo->latchName.substr(0, curLatchInfo->latchName.rfind('('));
                    curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchStartBit : curLatchInfo->latchEndBit;
                    o_queryData.latchName = latchname;
                    o_queryData.ringName = curLatchInfo->ringName;
                    o_queryData.fsiStartRingOffset = curLatchInfo->fsiRingOffset;
                    o_queryData.jtagStartRingOffset = curLatchInfo->jtagRingOffset;
                } else {
                    /* This is the case where the scandef had holes in the register, so we will continue with this latch, but skip some bits */
                    dataStartBit = dataEndBit = ECMD_UNSET;
                    curBufferBit = 0;
                    /* Decrement the bits to fetch by the hole in the latch */
                    curBitsToFetch -= (curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchStartBit : curLatchInfo->latchEndBit) - curLatchBit;
                    curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchStartBit : curLatchInfo->latchEndBit;
                }
            }
        
            /* Do we want anything in here */
            /* Check if the bits are ordered from:to (0:10) or just (1) */
            if (((curLatchInfo->latchEndBit >= curLatchInfo->latchStartBit) && (curLatchBit <=  curLatchInfo->latchEndBit)) ||
                /* Check if the bits are ordered to:from (10:0) */
                ((curLatchInfo->latchStartBit > curLatchInfo->latchEndBit)  && (curLatchBit <=  curLatchInfo->latchStartBit))) {
                
                bitsToFetch = (curLatchInfo->length  < curBitsToFetch) ? curLatchInfo->length : curBitsToFetch;
                
                /* Setup the actual data bits displayed */
                if (dataStartBit == ECMD_UNSET) {
                    dataStartBit = curLatchBit;
                    dataEndBit = dataStartBit - 1;
                }
                dataEndBit += bitsToFetch;
            
                /* ********* */
                /* FSI Order */
                /* ********* */
                
                if (bustype == ECMD_CHIPFLAG_FSI) {
                    /* Extract bits if ordered from:to (0:10) */
                    if (curLatchInfo->latchEndBit > curLatchInfo->latchStartBit) {
                        curLatchBit = curLatchInfo->latchEndBit + 1;
                        /* Extract if bits are ordered to:from (10:0) or just (1) */
                    } else {
                        curLatchBit = curLatchInfo->latchStartBit + 1;
                    }
                    curBufferBit += bitsToFetch;
                    
                    /* ********* */
                    /*JTAG Order */
                    /* ********* */
                } else {
                    /* Extract bits if ordered from:to (0:10) */
                    if (curLatchInfo->latchEndBit > curLatchInfo->latchStartBit) {
                        curLatchBit = curLatchInfo->latchEndBit + 1;
                    } else {
                        /* Extract if bits are ordered to:from (10:0) or just (1) */
                        curLatchBit = curLatchInfo->latchStartBit + 1;
                    }
                    curBufferBit += bitsToFetch;
                    
                }
            
                fsi_end = curLatchInfo->fsiRingOffset;
                jtag_end = curLatchInfo->jtagRingOffset;
                curBitsToFetch -= bitsToFetch;
            } else {
                /* Nothing was there that we needed, let's try the next entry */
                curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchEndBit + 1: curLatchInfo->latchStartBit + 1;             
            }
        
        } /* end latchinfo for loop */
        
        /* We have good data here, let's push it */
        if (latchname != "") {
            
            /* Display this if we aren't expecting or the expect failed */
            o_queryData.latchStartBit = (int)dataStartBit;
            o_queryData.latchEndBit = (int)dataEndBit;
            o_queryData.bitLength = dataEndBit - dataStartBit + 1;
            o_queryData.fsiEndRingOffset = fsi_end;
            o_queryData.jtagEndRingOffset = jtag_end;               
        }
    
        break;
    } /* end while (exit point) */
   
  
    return rc;
}

/* getRingDump in ecmdDllCapi.C didn't get done in time for eCMD 11.0, so pulling the interface - JTA 07/26/09 */
#if 0
uint32_t dllGetRingDump(ecmdChipTarget & i_target, const char* i_ringName, std::list<ecmdLatchEntry> & o_latches) {
  uint32_t rc = ECMD_FUNCTION_NOT_SUPPORTED;

  return rc;
}
#endif

uint32_t dllGetLatch(const ecmdChipTarget & i_target, const char* i_ringName, const char * i_latchName, std::list<ecmdLatchEntry> & o_data, ecmdLatchMode_t i_mode, uint32_t i_ring_mode) {
  uint32_t rc = 0;

  ecmdLatchBufferEntry curEntry;
  std::list< ecmdLatchEntry >::iterator curLatchInfo;    ///< Iterator for walking through latches
  ecmdDataBuffer ringBuffer;                    ///< Buffer to store entire ring
  ecmdDataBuffer buffer(5000 /* bits */);        ///< Space for extracted latch data
  ecmdDataBuffer buffertemp(5000 /* bits */);    ///< Temp space for extracted latch data
  bool enabledCache = false;                    ///< This is turned on if we enabled the cache, so we can disable on exit
  ecmdLatchEntry curData;                       ///< Data to load into return list
  std::string curRing;                          ///< Current ring being operated on
  uint32_t bustype;                             ///< Type of bus we are attached to JTAG vs FSI
  std::string printed;

  ecmdChipTarget cacheTarget;
  cacheTarget = i_target;
  ecmdSetTargetDepth(cacheTarget, ECMD_DEPTH_CHIP);
  if (!dllIsRingCacheEnabled(cacheTarget)) {
    enabledCache = true;
    dllEnableRingCache(cacheTarget);
  }

  o_data.clear();       // Flush out current list

  /* Let's find out if we are JTAG of FSI here */
  rc = dllGetScandefOrder(i_target, bustype);
  if (rc) {
    printed = "Problems retrieving chip information on target\n";
    dllRegisterErrorMsg(rc, "dllGetLatch", printed.c_str() );
    return rc;
  }
  /* Now make sure the plugin gave us some bus info */
  if ((bustype != ECMD_CHIPFLAG_JTAG) && (bustype != ECMD_CHIPFLAG_FSI) ) {
    dllRegisterErrorMsg(ECMD_DLL_INVALID, "dllGetLatch", "eCMD plugin returned an invalid bustype in dllGetScandefOrder\n");
    return ECMD_DLL_INVALID;
  }
  
  if( i_mode == ECMD_LATCHMODE_FULL) {
    rc = readScandefHash(i_target, i_ringName, i_latchName, curEntry);
    if( rc && (((rc != ECMD_UNKNOWN_FILE) &&(rc != ECMD_UNABLE_TO_OPEN_SCANDEFHASH)) && ((rc == ECMD_INVALID_LATCHNAME)||(rc ==
															  ECMD_INVALID_RING)||(rc == ECMD_SCANDEFHASH_MULT_RINGS)))) {
      return rc;
    }
  }
  if (rc || (i_mode != ECMD_LATCHMODE_FULL)) {
    rc = readScandef(i_target, i_ringName, i_latchName, i_mode, curEntry);
    if (rc) return rc;
  }

  /* Single exit point */
  while (1) {

    uint32_t bitsToFetch = ECMD_UNSET;       // Grab all bits
    uint32_t curLatchBit = ECMD_UNSET;       // This is the actual latch bit we are looking for next
    uint32_t curBufferBit = 0;               // Current bit to insert into buffered register
    uint32_t curBitsToFetch = bitsToFetch;   // This is the total number of bits left to fetch
    uint32_t dataStartBit = ECMD_UNSET;      // Actual start bit of buffered register
    uint32_t dataEndBit = ECMD_UNSET;        // Actual end bit of buffered register
    std::string latchname = "";
    curRing = "";

    for (curLatchInfo = curEntry.entry.begin(); (curLatchInfo != curEntry.entry.end()) && (curBitsToFetch > 0); curLatchInfo++) {


      /* Let's grab the ring for this latch entry */
      if (curRing != curLatchInfo->ringName) {
          if (i_ring_mode & ECMD_RING_MODE_SPARSE_ACCESS)
          {
              ecmdDataBuffer l_mask;
              std::list<ecmdRingData> l_ring_info;
              rc = dllQueryRing(i_target, l_ring_info, curLatchInfo->ringName.c_str(), ECMD_QUERY_DETAIL_LOW);
              if (!l_ring_info.empty())
              {
                  l_mask.setBitLength(l_ring_info.begin()->bitLength);
              }
              rc = dllCreateSparseMaskFromLatchOpt(i_target, curEntry.entry, l_mask, ECMD_UNSET, ECMD_UNSET);
              if (rc) {
                  dllRegisterErrorMsg(rc, "dllGetLatch", "Problems creating sparse mask from latch\n");
                  return rc;
              }
              
              rc = dllGetRingSparse(i_target, curLatchInfo->ringName.c_str(), ringBuffer, l_mask, i_ring_mode);
              if (rc) {
                  dllRegisterErrorMsg(rc, "dllGetLatch", "Problems reading ring from chip (sparse)\n");
                  return rc;
              }

          }
          else
          {
              rc = dllGetRing(i_target, curLatchInfo->ringName.c_str(), ringBuffer, i_ring_mode);
              if (rc) {
                  dllRegisterErrorMsg(rc, "dllGetLatch", "Problems reading ring from chip\n");
                  return rc;
              }
          }
          curRing = curLatchInfo->ringName;
      }
      /* Do we have previous data here , or some missing bits in the scandef latchs ?*/
      if (((dataStartBit != ECMD_UNSET) && (curLatchBit !=  curLatchInfo->latchStartBit) && (curLatchBit !=  curLatchInfo->latchEndBit)) ||
          ((latchname == "") || (latchname != curLatchInfo->latchName.substr(0, curLatchInfo->latchName.rfind('('))))) {
        /* I have some good data here */
        if (latchname != "") {

          curData.latchStartBit = (int)dataStartBit;
          curData.latchEndBit = (int)dataEndBit;
          rc = buffer.extract(curData.buffer, 0, dataEndBit - dataStartBit + 1); if (rc) return rc;
          curData.length = curData.buffer.getBitLength();
          curData.rc = ECMD_SUCCESS;
          o_data.push_back(curData);

        }

        /* If this is a fresh one we need to reset everything */
        if ((latchname == "") || (latchname != curLatchInfo->latchName.substr(0, latchname.length()))) {
          dataStartBit = dataEndBit = ECMD_UNSET;
          curBitsToFetch = ECMD_UNSET;
          curBufferBit = 0;
          latchname = curLatchInfo->latchName.substr(0, curLatchInfo->latchName.rfind('('));
          curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchStartBit : curLatchInfo->latchEndBit;
          curData.latchName = latchname;
          curData.ringName = curLatchInfo->ringName;
          curData.latchType = curLatchInfo->latchType;
          curData.fsiRingOffset = curLatchInfo->fsiRingOffset;
          curData.jtagRingOffset = curLatchInfo->jtagRingOffset;
        } else {
          /* This is the case where the scandef had holes in the register, so we will continue with this latch, but skip some bits */
          dataStartBit = dataEndBit = ECMD_UNSET;
          curBufferBit = 0;
          /* Decrement the bits to fetch by the hole in the latch */
          curBitsToFetch -= (curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchStartBit : curLatchInfo->latchEndBit) - curLatchBit;
          curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchStartBit : curLatchInfo->latchEndBit;
        }
      }

      /* Do we want anything in here */
      /* Check if the bits are ordered from:to (0:10) or just (1) */
      if (((curLatchInfo->latchEndBit >= curLatchInfo->latchStartBit) && (curLatchBit <=  curLatchInfo->latchEndBit)) ||
          /* Check if the bits are ordered to:from (10:0) */
          ((curLatchInfo->latchStartBit > curLatchInfo->latchEndBit)  && (curLatchBit <=  curLatchInfo->latchStartBit))) {

        bitsToFetch = (curLatchInfo->length  < curBitsToFetch) ? curLatchInfo->length : curBitsToFetch;

        /* Setup the actual data bits displayed */
        if (dataStartBit == ECMD_UNSET) {
          dataStartBit = curLatchBit;
          dataEndBit = dataStartBit - 1;
        }
        dataEndBit += bitsToFetch;

        /* ********* */
        /* FSI Order */
        /* ********* */

        if (bustype == ECMD_CHIPFLAG_FSI) {
          rc = ringBuffer.extract(buffertemp, curLatchInfo->fsiRingOffset, bitsToFetch); if (rc) return rc;

          /* Extract bits if ordered from:to (0:10) */
          if (curLatchInfo->latchEndBit > curLatchInfo->latchStartBit) {
            curLatchBit = curLatchInfo->latchEndBit + 1;
            /* Extract if bits are ordered to:from (10:0) or just (1) */
          } else {
            if (bitsToFetch > 1) buffertemp.reverse();
            curLatchBit = curLatchInfo->latchStartBit + 1;
          }
          rc = buffer.insert(buffertemp, curBufferBit, bitsToFetch); if (rc) return rc;
	  curBufferBit += bitsToFetch;

	  /* ********* */
	  /*JTAG Order */
	  /* ********* */
        } else {
          rc = ringBuffer.extract(buffertemp, curLatchInfo->jtagRingOffset - bitsToFetch + 1, bitsToFetch); if (rc) return rc;
          /* Extract bits if ordered from:to (0:10) */
          if (curLatchInfo->latchEndBit > curLatchInfo->latchStartBit) {
            buffertemp.reverse();
            curLatchBit = curLatchInfo->latchEndBit + 1;
          } else {
            /* Extract if bits are ordered to:from (10:0) or just (1) */
            curLatchBit = curLatchInfo->latchStartBit + 1;
          }
          rc = buffer.insert(buffertemp, curBufferBit, bitsToFetch); if (rc) return rc;
	  curBufferBit += bitsToFetch;

        }

        curBitsToFetch -= bitsToFetch;
      } else {
        /* Nothing was there that we needed, let's try the next entry */
        curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchEndBit + 1: curLatchInfo->latchStartBit + 1;

      }

    } /* end latchinfo for loop */

    /* We have good data here, let's push it */
    if (latchname != "") {

      /* Display this if we aren't expecting or the expect failed */
      curData.latchStartBit = (int)dataStartBit;
      curData.latchEndBit = (int)dataEndBit;
      rc = buffer.extract(curData.buffer, 0, dataEndBit - dataStartBit + 1); if (rc) return rc;
      curData.length = curData.buffer.getBitLength();
      curData.rc = ECMD_SUCCESS;
      o_data.push_back(curData);

    }

    break;
  } /* end while (exit point) */

  if (enabledCache) {
    rc = dllDisableRingCache(cacheTarget);
  }

  return rc;
}

uint32_t dllGetLatchOpt(const ecmdChipTarget & i_target, const std::list<ecmdLatchEntry> & i_scandefLatchInfo, std::list<ecmdLatchEntry> & o_data, uint32_t i_ring_mode) {
  uint32_t rc = 0;

  std::list< ecmdLatchEntry >::const_iterator curLatchInfo;    ///< Iterator for walking through latches
  ecmdDataBuffer ringBuffer;                    ///< Buffer to store entire ring
  ecmdDataBuffer buffer(5000 /* bits */);        ///< Space for extracted latch data
  ecmdDataBuffer buffertemp(5000 /* bits */);    ///< Temp space for extracted latch data
  bool enabledCache = false;                    ///< This is turned on if we enabled the cache, so we can disable on exit
  ecmdLatchEntry curData;                       ///< Data to load into return list
  std::string curRing;                          ///< Current ring being operated on
  uint32_t bustype;                             ///< Type of bus we are attached to JTAG vs FSI
  std::string printed;

  ecmdChipTarget cacheTarget;
  cacheTarget = i_target;
  ecmdSetTargetDepth(cacheTarget, ECMD_DEPTH_CHIP);
  if (!dllIsRingCacheEnabled(cacheTarget)) {
    enabledCache = true;
    dllEnableRingCache(cacheTarget);
  }

  o_data.clear();       // Flush out current list

  /* Let's find out if we are JTAG of FSI here */
  rc = dllGetScandefOrder(i_target, bustype);
  if (rc) {
    printed = "Problems retrieving chip information on target\n";
    dllRegisterErrorMsg(rc, "dllGetLatchOpt", printed.c_str() );
    return rc;
  }
  /* Now make sure the plugin gave us some bus info */
  if ((bustype != ECMD_CHIPFLAG_JTAG) && (bustype != ECMD_CHIPFLAG_FSI) ) {
    dllRegisterErrorMsg(ECMD_DLL_INVALID, "dllGetLatchOpt", "eCMD plugin returned an invalid bustype in dllGetScandefOrder\n");
    return ECMD_DLL_INVALID;
  }
  

  /* Single exit point */
  while (1) {

    uint32_t bitsToFetch = ECMD_UNSET;       // Grab all bits
    uint32_t curLatchBit = ECMD_UNSET;       // This is the actual latch bit we are looking for next
    uint32_t curBufferBit = 0;               // Current bit to insert into buffered register
    uint32_t curBitsToFetch = bitsToFetch;   // This is the total number of bits left to fetch
    uint32_t dataStartBit = ECMD_UNSET;      // Actual start bit of buffered register
    uint32_t dataEndBit = ECMD_UNSET;        // Actual end bit of buffered register
    std::string latchname = "";
    curRing = "";

    for (curLatchInfo = i_scandefLatchInfo.begin(); (curLatchInfo != i_scandefLatchInfo.end()) && (curBitsToFetch > 0); curLatchInfo++) {


      /* Let's grab the ring for this latch entry */
      if (curRing != curLatchInfo->ringName) {
          if (i_ring_mode & ECMD_RING_MODE_SPARSE_ACCESS)
          {
              ecmdDataBuffer l_mask;
              std::list<ecmdRingData> l_ring_info;
              rc = dllQueryRing(i_target, l_ring_info, curLatchInfo->ringName.c_str(), ECMD_QUERY_DETAIL_LOW);
              if (!l_ring_info.empty())
              {
                  l_mask.setBitLength(l_ring_info.begin()->bitLength);
              }
              rc = dllCreateSparseMaskFromLatchOpt(i_target, i_scandefLatchInfo, l_mask, ECMD_UNSET, ECMD_UNSET);
              if (rc) {
                  dllRegisterErrorMsg(rc, "dllGetLatchOpt", "Problems creating sparse mask from latch\n");
                  return rc;
              }
              
              rc = dllGetRingSparse(i_target, curLatchInfo->ringName.c_str(), ringBuffer, l_mask, i_ring_mode);
              if (rc) {
                  dllRegisterErrorMsg(rc, "dllGetLatchOpt", "Problems reading ring from chip (sparse)\n");
                  return rc;
              }

          }
          else
          {
              rc = dllGetRing(i_target, curLatchInfo->ringName.c_str(), ringBuffer, i_ring_mode);
              if (rc) {
                  dllRegisterErrorMsg(rc, "dllGetLatchOpt", "Problems reading ring from chip\n");
                  return rc;
              }
          }
          curRing = curLatchInfo->ringName;
      }
      /* Do we have previous data here , or some missing bits in the scandef latchs ?*/
      if (((dataStartBit != ECMD_UNSET) && (curLatchBit !=  curLatchInfo->latchStartBit) && (curLatchBit !=  curLatchInfo->latchEndBit)) ||
          ((latchname == "") || (latchname != curLatchInfo->latchName.substr(0, curLatchInfo->latchName.rfind('('))))) {
        /* I have some good data here */
        if (latchname != "") {

          curData.latchStartBit = (int)dataStartBit;
          curData.latchEndBit = (int)dataEndBit;
          rc = buffer.extract(curData.buffer, 0, dataEndBit - dataStartBit + 1); if (rc) return rc;
          curData.length = curData.buffer.getBitLength();
          curData.rc = ECMD_SUCCESS;
          o_data.push_back(curData);

        }

        /* If this is a fresh one we need to reset everything */
        if ((latchname == "") || (latchname != curLatchInfo->latchName.substr(0, latchname.length()))) {
          dataStartBit = dataEndBit = ECMD_UNSET;
          curBitsToFetch = ECMD_UNSET;
          curBufferBit = 0;
          latchname = curLatchInfo->latchName.substr(0, curLatchInfo->latchName.rfind('('));
          curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchStartBit : curLatchInfo->latchEndBit;
          curData.latchName = latchname;
          curData.ringName = curLatchInfo->ringName;
          curData.latchType = curLatchInfo->latchType;
          curData.fsiRingOffset = curLatchInfo->fsiRingOffset;
          curData.jtagRingOffset = curLatchInfo->jtagRingOffset;
        } else {
          /* This is the case where the scandef had holes in the register, so we will continue with this latch, but skip some bits */
          dataStartBit = dataEndBit = ECMD_UNSET;
          curBufferBit = 0;
          /* Decrement the bits to fetch by the hole in the latch */
          curBitsToFetch -= (curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchStartBit : curLatchInfo->latchEndBit) - curLatchBit;
          curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchStartBit : curLatchInfo->latchEndBit;
        }
      }

      /* Do we want anything in here */
      /* Check if the bits are ordered from:to (0:10) or just (1) */
      if (((curLatchInfo->latchEndBit >= curLatchInfo->latchStartBit) && (curLatchBit <=  curLatchInfo->latchEndBit)) ||
          /* Check if the bits are ordered to:from (10:0) */
          ((curLatchInfo->latchStartBit > curLatchInfo->latchEndBit)  && (curLatchBit <=  curLatchInfo->latchStartBit))) {

        bitsToFetch = (curLatchInfo->length  < curBitsToFetch) ? curLatchInfo->length : curBitsToFetch;

        /* Setup the actual data bits displayed */
        if (dataStartBit == ECMD_UNSET) {
          dataStartBit = curLatchBit;
          dataEndBit = dataStartBit - 1;
        }
        dataEndBit += bitsToFetch;

        /* ********* */
        /* FSI Order */
        /* ********* */

        if (bustype == ECMD_CHIPFLAG_FSI) {
          rc = ringBuffer.extract(buffertemp, curLatchInfo->fsiRingOffset, bitsToFetch); if (rc) return rc;

          /* Extract bits if ordered from:to (0:10) */
          if (curLatchInfo->latchEndBit > curLatchInfo->latchStartBit) {
            curLatchBit = curLatchInfo->latchEndBit + 1;
            /* Extract if bits are ordered to:from (10:0) or just (1) */
          } else {
            if (bitsToFetch > 1) buffertemp.reverse();
            curLatchBit = curLatchInfo->latchStartBit + 1;
          }
          rc = buffer.insert(buffertemp, curBufferBit, bitsToFetch); if (rc) return rc;
	  curBufferBit += bitsToFetch;

	  /* ********* */
	  /*JTAG Order */
	  /* ********* */
        } else {
          rc = ringBuffer.extract(buffertemp, curLatchInfo->jtagRingOffset - bitsToFetch + 1, bitsToFetch); if (rc) return rc;
          /* Extract bits if ordered from:to (0:10) */
          if (curLatchInfo->latchEndBit > curLatchInfo->latchStartBit) {
            buffertemp.reverse();
            curLatchBit = curLatchInfo->latchEndBit + 1;
          } else {
            /* Extract if bits are ordered to:from (10:0) or just (1) */
            curLatchBit = curLatchInfo->latchStartBit + 1;
          }
          rc = buffer.insert(buffertemp, curBufferBit, bitsToFetch); if (rc) return rc;
	  curBufferBit += bitsToFetch;

        }

        curBitsToFetch -= bitsToFetch;
      } else {
        /* Nothing was there that we needed, let's try the next entry */
        curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchEndBit + 1: curLatchInfo->latchStartBit + 1;

      }

    } /* end latchinfo for loop */

    /* We have good data here, let's push it */
    if (latchname != "") {

      /* Display this if we aren't expecting or the expect failed */
      curData.latchStartBit = (int)dataStartBit;
      curData.latchEndBit = (int)dataEndBit;
      rc = buffer.extract(curData.buffer, 0, dataEndBit - dataStartBit + 1); if (rc) return rc;
      curData.length = curData.buffer.getBitLength();
      curData.rc = ECMD_SUCCESS;
      o_data.push_back(curData);

    }

    break;
  } /* end while (exit point) */

  if (enabledCache) {
    rc = dllDisableRingCache(cacheTarget);
  }

  return rc;
}

uint32_t dllPutLatch(const ecmdChipTarget & i_target, const char* i_ringName, const char * i_latchName, const ecmdDataBuffer & i_data, uint32_t i_startBit, uint32_t i_numBits, uint32_t & o_matches, ecmdLatchMode_t i_mode, uint32_t i_ring_mode) {

  uint32_t rc = 0;
  ecmdLatchBufferEntry curEntry;
  std::list< ecmdLatchEntry >::iterator curLatchInfo;
  ecmdDataBuffer ringBuffer;            ///< Buffer to store entire ring
  ecmdDataBuffer bufferCopy;            ///< Copy of data to be inserted
  ecmdDataBuffer buffertemp;            ///< Temp buffer to allow reversing in JTAG mode
  std::string curRing;                  ///< Current ring being operated on
  std::string curLatchName;             ///< Current latch name being operated on
  uint32_t bustype;                             ///< Type of bus we are attached to JTAG vs FSI
  std::string printed;
  bool enabledCache = false;                    ///< This is turned on if we enabled the cache, so we can disable on exit
  ecmdDataBuffer l_mask;                ///< Sparse ring access bit mask
  std::list<ecmdRingData> l_ring_info;

  o_matches = 0;

  ecmdChipTarget cacheTarget;
  cacheTarget = i_target;
  ecmdSetTargetDepth(cacheTarget, ECMD_DEPTH_CHIP);
  if (!dllIsRingCacheEnabled(cacheTarget)) {
    enabledCache = true;
    dllEnableRingCache(cacheTarget);
  }

  /* Do we have the right amount of data ? */
  if (i_data.getBitLength() > i_numBits) {
    rc = ECMD_DATA_OVERFLOW;
    dllRegisterErrorMsg(rc, "dllPutLatch", "Data buffer length is greater than numBits requested to write\n");
    return rc;
  } else if (i_data.getBitLength() < i_numBits) {
    rc = ECMD_DATA_UNDERFLOW;
    dllRegisterErrorMsg(rc, "dllPutLatch", "Data buffer length is less than numBits requested to write\n");
    return rc;
  }

  /* Let's find out if we are JTAG of FSI here */
  rc = dllGetScandefOrder(i_target, bustype);
  if (rc) {
    dllRegisterErrorMsg(rc, "dllPutLatch", "Problems retrieving chip information on target\n");
    return rc;
  }
  /* Now make sure the plugin gave us some bus info */
  if ((bustype != ECMD_CHIPFLAG_JTAG) && (bustype != ECMD_CHIPFLAG_FSI)) {
    dllRegisterErrorMsg(ECMD_DLL_INVALID, "dllPutLatch", "eCMD plugin returned an invalid bustype in dllGetScandefOrder\n");
    return ECMD_DLL_INVALID;
  }

  if( i_mode == ECMD_LATCHMODE_FULL) {
    rc = readScandefHash(i_target, i_ringName, i_latchName, curEntry);
    if( rc && (((rc != ECMD_UNKNOWN_FILE) &&(rc != ECMD_UNABLE_TO_OPEN_SCANDEFHASH)) && ((rc == ECMD_INVALID_LATCHNAME)||(rc == ECMD_INVALID_RING)||(rc == ECMD_SCANDEFHASH_MULT_RINGS)))) {
      return rc;
    }
  }
  if (rc || (i_mode != ECMD_LATCHMODE_FULL)) {
    std::string errorParse;
    if (rc)
    {
      errorParse = dllGetErrorMsg(rc, false, true, true);
      errorParse = "WARNING: Latch is found in Scandef File but is missing in Hashfile \n" + errorParse;

    }
    rc = readScandef(i_target, i_ringName, i_latchName, i_mode, curEntry);
    if (rc) return rc;

    if (errorParse.length() > 0) {
       dllOutputWarning(errorParse.c_str());
    }
  }

  /* single exit point */
  do {
    uint32_t bitsToInsert = i_numBits;
    uint32_t curLatchBit = ECMD_UNSET;          // This is the actual latch bit we are looking for next
    uint32_t curBitsToInsert = bitsToInsert;    // This is the total number of bits left to Insert
    uint32_t curStartBitToInsert = 0;           // This is the offset into the current entry for insertion
    uint32_t curStart = 0;
    uint32_t curEnd = 0;

    for (curLatchInfo = curEntry.entry.begin(); curLatchInfo != curEntry.entry.end(); curLatchInfo++) {

      /* Let's grab the ring for this latch entry */
      if (curRing != curLatchInfo->ringName) {

        // This is a sparse ring access
        if (i_ring_mode & ECMD_RING_MODE_SPARSE_ACCESS)
        {
            rc = dllQueryRing(i_target, l_ring_info, curLatchInfo->ringName.c_str(), ECMD_QUERY_DETAIL_LOW);
            if (!l_ring_info.empty())
            {
                l_mask.setBitLength(l_ring_info.begin()->bitLength);
            }

            if (curRing.length() > 0) {
                rc = dllCreateSparseMaskFromLatchOpt(i_target, curEntry.entry, l_mask, i_startBit, i_numBits);
                if (rc) {
                    dllRegisterErrorMsg(rc, "dllPutLatch", "Problems creating sparse mask from latch\n");
                    break;
                }
                              

                rc = dllPutRingSparse(i_target, curRing.c_str(), ringBuffer, l_mask, i_ring_mode);
                if (rc) {
                    dllRegisterErrorMsg(rc, "dllPutLatch", "Problems writing ring into chip\n");
                    break;
                }
            }
            // Don't need to do a getRing first when doing a sparse ring.  Just set the size of our ringBuffer
            rc = dllQueryRing(i_target, l_ring_info, i_ringName, ECMD_QUERY_DETAIL_LOW);
            if (!l_ring_info.empty())
            {
                ringBuffer.setBitLength(l_ring_info.begin()->bitLength);
            }
        }
        else
        {
            if (curRing.length() > 0) {
                rc = dllPutRing(i_target, curRing.c_str(), ringBuffer, i_ring_mode);
                if (rc) {
                    dllRegisterErrorMsg(rc, "dllPutLatch", "Problems writing ring into chip\n");
                    break;
                }
            }
            rc = dllGetRing(i_target, curLatchInfo->ringName.c_str(), ringBuffer, i_ring_mode);
            if (rc) {
                dllRegisterErrorMsg(rc, "dllPutLatch", "Problems reading ring from chip\n");
                break;
            }
        }
        curRing = curLatchInfo->ringName;
      }

      if ((curLatchBit == ECMD_UNSET) ||(curLatchName != curLatchInfo->latchName.substr(0, curLatchInfo->latchName.rfind('(')))) {
        curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchStartBit : curLatchInfo->latchEndBit;
        curLatchName = curLatchInfo->latchName.substr(0, curLatchInfo->latchName.rfind('('));
        /* Make a copy of insert data so we don't lose it */
        bufferCopy = i_data;
        bitsToInsert = i_numBits;
        curBitsToInsert = bitsToInsert;
        curStart = i_startBit;
        curEnd = i_startBit + bitsToInsert - 1;

        /* We found something, bump the matches */
        o_matches++;
      }

      bool doWrite = false;
      /* Check if the bits are ordered from:to (0:10) or just (1) and we want to modify */
      if ((curLatchInfo->latchEndBit >= curLatchInfo->latchStartBit) && (curStart <= curLatchInfo->latchEndBit) && (curEnd >= curLatchInfo->latchStartBit)) {
        if(curStart < curLatchInfo->latchStartBit) { // move counter to start of latch if lower than current latch
          curStart = curLatchInfo->latchStartBit;
        }
        if (bustype == ECMD_CHIPFLAG_FSI) {
          if(curStart > curLatchInfo->latchStartBit) { // case where we start inside the latch
            curStartBitToInsert = curStart - curLatchInfo->latchStartBit;
          } else {
            curStartBitToInsert = 0;
          }
          bitsToInsert = ((curLatchInfo->length - curStartBitToInsert) < curBitsToInsert ? (curLatchInfo->length - curStartBitToInsert) : curBitsToInsert);
        } else {
          // JTAG -- not sure if this is correct for more than one bit
          //      -- this is a modified version of the FSI high:low order code
          //      -- with latchEndBit and latchStartbit swapped when appropriate
          //      -- mklight 03/13/2009
          // figure out where we should start
          int32_t tempStartBitToInsert = curLatchInfo->latchEndBit - (curStart + curBitsToInsert - 1);
          // if we go negative we should start at zero
          if(tempStartBitToInsert <= 0) { // we start at start bit
            curStartBitToInsert = 0;
            if(curStart > curLatchInfo->latchStartBit) { // case where we start inside the latch
              bitsToInsert = curLatchInfo->length - (curStart - curLatchInfo->latchStartBit);
            } else { // otherwise we start at the end of the latch
              bitsToInsert = curLatchInfo->length;
            }
          } else { // we start after start bit, end of latch will not be filled, this will be the last latch
            curStartBitToInsert = (uint32_t) tempStartBitToInsert;
            bitsToInsert = curBitsToInsert;
          }
        }
        curLatchBit = curLatchInfo->latchEndBit + 1;
        doWrite = true;
      }

      /* Check if the bits are ordered to:from (10:0) */
      if ((curLatchInfo->latchStartBit > curLatchInfo->latchEndBit) && (curStart <= curLatchInfo->latchStartBit) && (curEnd >= curLatchInfo->latchEndBit)) {
        if(curStart < curLatchInfo->latchEndBit) { // move counter to start of latch if lower than current latch
          curStart = curLatchInfo->latchEndBit;
        }
        if (bustype == ECMD_CHIPFLAG_FSI) {
          // figure out where we should start
         int32_t tempStartBitToInsert = curLatchInfo->latchStartBit - (curStart + curBitsToInsert - 1);
          // if we go negative we should start at zero
          if(tempStartBitToInsert <= 0) { // we start at start bit
            curStartBitToInsert = 0;
            if(curStart > curLatchInfo->latchEndBit) { // case where we start inside the latch
              bitsToInsert = curLatchInfo->length - (curStart - curLatchInfo->latchEndBit);
            } else { // otherwise we start at the end of the latch
              bitsToInsert = curLatchInfo->length;
            }
          } else { // we start after start bit, end of latch will not be filled, this will be the last latch
            curStartBitToInsert = (uint32_t) tempStartBitToInsert;
            bitsToInsert = curBitsToInsert;
          }
        } else {
          if(curStart > curLatchInfo->latchEndBit) { // case where we start inside the latch
            curStartBitToInsert = curStart - curLatchInfo->latchEndBit;
          } else {
            curStartBitToInsert = 0;
          }
          bitsToInsert = ((curLatchInfo->length - curStartBitToInsert) < curBitsToInsert ? (curLatchInfo->length - curStartBitToInsert) : curBitsToInsert);
        }
        curLatchBit = curLatchInfo->latchStartBit + 1;
        doWrite = true;
      }

      if (doWrite == true)
      {
        rc = bufferCopy.extract(buffertemp, 0, bitsToInsert); if (rc) return rc;
        //printf("curLatchBit = %d, startbit = %d, endbit = %d, length = %d, curStartBitToInsert = %d, bits = %d\n", curLatchBit, curLatchInfo->latchStartBit, curLatchInfo->latchEndBit, curLatchInfo->length, curStartBitToInsert,  bitsToInsert);
        /* ********* */
        /* FSI Order */
        /* ********* */
        if (bustype == ECMD_CHIPFLAG_FSI) {
          /* if bits are ordered to:from (10:0) or just (1) */
          if (curLatchInfo->latchEndBit < curLatchInfo->latchStartBit) {
            /* Reverse the data to go into the scanring */
            buffertemp.reverse();
          }

          //printf("inserting %d bits at offset %d\n", bitsToInsert, curLatchInfo->fsiRingOffset + curStartBitToInsert);
          rc = ringBuffer.insert(buffertemp, curLatchInfo->fsiRingOffset + curStartBitToInsert, bitsToInsert);
          if (rc) return rc;

        }
        /* ********** */
        /* JTAG Order */
        /* ********** */
        else {
          /* if ordered from:to (0:10) */
          if (curLatchInfo->latchEndBit > curLatchInfo->latchStartBit) {
            /* Reverse the data to go into the scanring */
            buffertemp.reverse();
          }

          //printf("inserting %d bits at offset %d\n", bitsToInsert, curLatchInfo->jtagRingOffset - curLatchInfo->length  + 1 + curStartBitToInsert);
          rc = ringBuffer.insert(buffertemp, curLatchInfo->jtagRingOffset - curLatchInfo->length  + 1 + curStartBitToInsert, bitsToInsert);
          if (rc) return rc;

        }

        /* Get rid of the data we just inserted, to line up the next piece */
        bufferCopy.shiftLeft(bitsToInsert);

        curBitsToInsert -= bitsToInsert;
      } else {
        /* Nothing was there that we needed, let's try the next entry */
        curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchEndBit + 1: curLatchInfo->latchStartBit + 1;
      }
    }

    if (curRing.length() > 0) {
        if (i_ring_mode & ECMD_RING_MODE_SPARSE_ACCESS)
        {
            // FIXME set bit length for mask???
            rc = dllCreateSparseMaskFromLatchOpt(i_target, curEntry.entry, l_mask, i_startBit, i_numBits);
            if (rc) {
                dllRegisterErrorMsg(rc, "dllPutLatch", "Problems creating sparse mask from latch\n");
                break;
            }

            rc = dllPutRingSparse(i_target, curRing.c_str(), ringBuffer, l_mask, i_ring_mode);
            if (rc) {
                dllRegisterErrorMsg(rc, "dllPutLatch", "Problems writing ring into chip\n");
                break;
            }

        }
        else
        {
            rc = dllPutRing(i_target, curRing.c_str(), ringBuffer, i_ring_mode);
            if (rc) {
                dllRegisterErrorMsg(rc, "dllPutLatch", "Problems writing ring into chip\n");
                break;
            }
        }
    }

  } while(0);

  if (enabledCache) {
    /* Write all the data to the chip */
    rc = dllDisableRingCache(cacheTarget);
  }

  return rc;
}

uint32_t dllPutLatchOpt(const ecmdChipTarget & i_target, const ecmdDataBuffer & i_data, uint32_t i_startBit, uint32_t i_numBits, uint32_t & o_matches, const std::list<ecmdLatchEntry> & i_scandefLatchInfo, uint32_t i_ring_mode) {

  uint32_t rc = 0;
  std::list< ecmdLatchEntry >::const_iterator curLatchInfo;
  ecmdDataBuffer ringBuffer;            ///< Buffer to store entire ring
  ecmdDataBuffer bufferCopy;            ///< Copy of data to be inserted
  ecmdDataBuffer buffertemp;            ///< Temp buffer to allow reversing in JTAG mode
  std::string curRing;                  ///< Current ring being operated on
  std::string curLatchName;             ///< Current latch name being operated on
  uint32_t bustype;                     ///< Type of bus we are attached to JTAG vs FSI
  std::string printed;
  bool enabledCache = false;            ///< This is turned on if we enabled the cache, so we can disable on exit
  ecmdDataBuffer l_mask;                ///< Sparse ring access bit mask
  std::list<ecmdRingData> l_ring_info;

  o_matches = 0;

  ecmdChipTarget cacheTarget;
  cacheTarget = i_target;
  ecmdSetTargetDepth(cacheTarget, ECMD_DEPTH_CHIP);
  if (!dllIsRingCacheEnabled(cacheTarget)) {
    enabledCache = true;
    dllEnableRingCache(cacheTarget);
  }

  /* Do we have the right amount of data ? */
  if (i_data.getBitLength() > i_numBits) {
    rc = ECMD_DATA_OVERFLOW;
    dllRegisterErrorMsg(rc, "dllPutLatchOpt", "Data buffer length is greater than numBits requested to write\n");
    return rc;
  } else if (i_data.getBitLength() < i_numBits) {
    rc = ECMD_DATA_UNDERFLOW;
    dllRegisterErrorMsg(rc, "dllPutLatchOpt", "Data buffer length is less than numBits requested to write\n");
    return rc;
  }

  /* Let's find out if we are JTAG of FSI here */
  rc = dllGetScandefOrder(i_target, bustype);
  if (rc) {
    dllRegisterErrorMsg(rc, "dllPutLatchOpt", "Problems retrieving chip information on target\n");
    return rc;
  }
  /* Now make sure the plugin gave us some bus info */
  if ((bustype != ECMD_CHIPFLAG_JTAG) && (bustype != ECMD_CHIPFLAG_FSI)) {
    dllRegisterErrorMsg(ECMD_DLL_INVALID, "dllPutLatchOpt", "eCMD plugin returned an invalid bustype in dllGetScandefOrder\n");
    return ECMD_DLL_INVALID;
  }


  /* single exit point */
  do {
    uint32_t bitsToInsert = i_numBits;
    uint32_t curLatchBit = ECMD_UNSET;          // This is the actual latch bit we are looking for next
    uint32_t curBitsToInsert = bitsToInsert;    // This is the total number of bits left to Insert
    uint32_t curStartBitToInsert = 0;           // This is the offset into the current entry for insertion
    uint32_t curStart = 0;
    uint32_t curEnd = 0;

    for (curLatchInfo = i_scandefLatchInfo.begin(); curLatchInfo != i_scandefLatchInfo.end(); curLatchInfo++) {

      /* Let's grab the ring for this latch entry */
      if (curRing != curLatchInfo->ringName) {

        // This is a sparse ring access
        if (i_ring_mode & ECMD_RING_MODE_SPARSE_ACCESS)
        {

            // Don't need to do a getRing first when doing a sparse ring.  Just set the size of our ringBuffer and mask
            rc = dllQueryRing(i_target, l_ring_info, curLatchInfo->ringName.c_str(), ECMD_QUERY_DETAIL_LOW);
            if (!l_ring_info.empty())
            {
                ringBuffer.setBitLength(l_ring_info.begin()->bitLength);
                l_mask.setBitLength(l_ring_info.begin()->bitLength);
            }

            if (curRing.length() > 0) {
                 rc = dllCreateSparseMaskFromLatchOpt(i_target, i_scandefLatchInfo, l_mask, i_startBit, i_numBits);
                if (rc) {
                    dllRegisterErrorMsg(rc, "dllGetLatchOpt", "Problems creating sparse mask from latch\n");
                    break;
                }
                              
                rc = dllPutRingSparse(i_target, curRing.c_str(), ringBuffer, l_mask, i_ring_mode);
                if (rc) {
                    dllRegisterErrorMsg(rc, "dllPutLatchOpt", "Problems writing ring into chip\n");
                    break;
                }
            }


        }
        else
        {
            if (curRing.length() > 0) {
                rc = dllPutRing(i_target, curRing.c_str(), ringBuffer, i_ring_mode);
                if (rc) {
                    dllRegisterErrorMsg(rc, "dllPutLatchOpt", "Problems writing ring into chip\n");
                    break;
                }
            }
            rc = dllGetRing(i_target, curLatchInfo->ringName.c_str(), ringBuffer, i_ring_mode);
            if (rc) {
                dllRegisterErrorMsg(rc, "dllPutLatchOpt", "Problems reading ring from chip\n");
                break;
            }
        }
        curRing = curLatchInfo->ringName;
      }

      if ((curLatchBit == ECMD_UNSET) ||(curLatchName != curLatchInfo->latchName.substr(0, curLatchInfo->latchName.rfind('(')))) {
        curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchStartBit : curLatchInfo->latchEndBit;
        curLatchName = curLatchInfo->latchName.substr(0, curLatchInfo->latchName.rfind('('));
        /* Make a copy of insert data so we don't lose it */
        bufferCopy = i_data;
        bitsToInsert = i_numBits;
        curBitsToInsert = bitsToInsert;
        curStart = i_startBit;
        curEnd = i_startBit + bitsToInsert - 1;

        /* We found something, bump the matches */
        o_matches++;
      }

      bool doWrite = false;
      /* Check if the bits are ordered from:to (0:10) or just (1) and we want to modify */
      if ((curLatchInfo->latchEndBit >= curLatchInfo->latchStartBit) && (curStart <= curLatchInfo->latchEndBit) && (curEnd >= curLatchInfo->latchStartBit)) {
        if(curStart < curLatchInfo->latchStartBit) { // move counter to start of latch if lower than current latch
          curStart = curLatchInfo->latchStartBit;
        }
        if (bustype == ECMD_CHIPFLAG_FSI) {
          if(curStart > curLatchInfo->latchStartBit) { // case where we start inside the latch
            curStartBitToInsert = curStart - curLatchInfo->latchStartBit;
          } else {
            curStartBitToInsert = 0;
          }
          bitsToInsert = ((curLatchInfo->length - curStartBitToInsert) < curBitsToInsert ? (curLatchInfo->length - curStartBitToInsert) : curBitsToInsert);
        } else {
          // JTAG -- not sure if this is correct for more than one bit
          //      -- this is a modified version of the FSI high:low order code
          //      -- with latchEndBit and latchStartbit swapped when appropriate
          //      -- mklight 03/13/2009
          // figure out where we should start
          int32_t tempStartBitToInsert = curLatchInfo->latchEndBit - (curStart + curBitsToInsert - 1);
          // if we go negative we should start at zero
          if(tempStartBitToInsert <= 0) { // we start at start bit
            curStartBitToInsert = 0;
            if(curStart > curLatchInfo->latchStartBit) { // case where we start inside the latch
              bitsToInsert = curLatchInfo->length - (curStart - curLatchInfo->latchStartBit);
            } else { // otherwise we start at the end of the latch
              bitsToInsert = curLatchInfo->length;
            }
          } else { // we start after start bit, end of latch will not be filled, this will be the last latch
            curStartBitToInsert = (uint32_t) tempStartBitToInsert;
            bitsToInsert = curBitsToInsert;
          }
        }
        curLatchBit = curLatchInfo->latchEndBit + 1;
        doWrite = true;
      }

      /* Check if the bits are ordered to:from (10:0) */
      if ((curLatchInfo->latchStartBit > curLatchInfo->latchEndBit) && (curStart <= curLatchInfo->latchStartBit) && (curEnd >= curLatchInfo->latchEndBit)) {
        if(curStart < curLatchInfo->latchEndBit) { // move counter to start of latch if lower than current latch
          curStart = curLatchInfo->latchEndBit;
        }
        if (bustype == ECMD_CHIPFLAG_FSI) {
          // figure out where we should start
         int32_t tempStartBitToInsert = curLatchInfo->latchStartBit - (curStart + curBitsToInsert - 1);
          // if we go negative we should start at zero
          if(tempStartBitToInsert <= 0) { // we start at start bit
            curStartBitToInsert = 0;
            if(curStart > curLatchInfo->latchEndBit) { // case where we start inside the latch
              bitsToInsert = curLatchInfo->length - (curStart - curLatchInfo->latchEndBit);
            } else { // otherwise we start at the end of the latch
              bitsToInsert = curLatchInfo->length;
            }
          } else { // we start after start bit, end of latch will not be filled, this will be the last latch
            curStartBitToInsert = (uint32_t) tempStartBitToInsert;
            bitsToInsert = curBitsToInsert;
          }
        } else {
          if(curStart > curLatchInfo->latchEndBit) { // case where we start inside the latch
            curStartBitToInsert = curStart - curLatchInfo->latchEndBit;
          } else {
            curStartBitToInsert = 0;
          }
          bitsToInsert = ((curLatchInfo->length - curStartBitToInsert) < curBitsToInsert ? (curLatchInfo->length - curStartBitToInsert) : curBitsToInsert);
        }
        curLatchBit = curLatchInfo->latchStartBit + 1;
        doWrite = true;
      }

      if (doWrite == true)
      {
        rc = bufferCopy.extract(buffertemp, 0, bitsToInsert); if (rc) return rc;
        //printf("curLatchBit = %d, startbit = %d, endbit = %d, length = %d, curStartBitToInsert = %d, bits = %d\n", curLatchBit, curLatchInfo->latchStartBit, curLatchInfo->latchEndBit, curLatchInfo->length, curStartBitToInsert,  bitsToInsert);
        /* ********* */
        /* FSI Order */
        /* ********* */
        if (bustype == ECMD_CHIPFLAG_FSI) {
          /* if bits are ordered to:from (10:0) or just (1) */
          if (curLatchInfo->latchEndBit < curLatchInfo->latchStartBit) {
            /* Reverse the data to go into the scanring */
            buffertemp.reverse();
          }

          //printf("inserting %d bits at offset %d\n", bitsToInsert, curLatchInfo->fsiRingOffset + curStartBitToInsert);
          rc = ringBuffer.insert(buffertemp, curLatchInfo->fsiRingOffset + curStartBitToInsert, bitsToInsert);
          if (rc) return rc;

        }
        /* ********** */
        /* JTAG Order */
        /* ********** */
        else {
          /* if ordered from:to (0:10) */
          if (curLatchInfo->latchEndBit > curLatchInfo->latchStartBit) {
            /* Reverse the data to go into the scanring */
            buffertemp.reverse();
          }

          //printf("inserting %d bits at offset %d\n", bitsToInsert, curLatchInfo->jtagRingOffset - curLatchInfo->length  + 1 + curStartBitToInsert);
          rc = ringBuffer.insert(buffertemp, curLatchInfo->jtagRingOffset - curLatchInfo->length  + 1 + curStartBitToInsert, bitsToInsert);
          if (rc) return rc;

        }

        /* Get rid of the data we just inserted, to line up the next piece */
        bufferCopy.shiftLeft(bitsToInsert);

        curBitsToInsert -= bitsToInsert;
      } else {
        /* Nothing was there that we needed, let's try the next entry */
        curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchEndBit + 1: curLatchInfo->latchStartBit + 1;
      }
    }

    if (curRing.length() > 0) {
        if (i_ring_mode & ECMD_RING_MODE_SPARSE_ACCESS)
        {
            // FIXME set bit length for mask???
            rc = dllCreateSparseMaskFromLatchOpt(i_target, i_scandefLatchInfo, l_mask, i_startBit, i_numBits);
            if (rc) {
                dllRegisterErrorMsg(rc, "dllPutLatchOpt", "Problems creating sparse mask from latch\n");
                break;
            }

            rc = dllPutRingSparse(i_target, curRing.c_str(), ringBuffer, l_mask, i_ring_mode);
            if (rc) {
                dllRegisterErrorMsg(rc, "dllPutLatchOpt", "Problems writing ring into chip\n");
                break;
            }

        }
        else
        {
            rc = dllPutRing(i_target, curRing.c_str(), ringBuffer, i_ring_mode);
            if (rc) {
                dllRegisterErrorMsg(rc, "dllPutLatchOpt", "Problems writing ring into chip\n");
                break;
            }
        }
    }

  } while(0);

  if (enabledCache) {
    /* Write all the data to the chip */
    rc = dllDisableRingCache(cacheTarget);
  }

  return rc;
}

uint32_t dllGetLatchImage(const ecmdChipTarget & i_target, const char* i_ringName, const char * i_latchName, std::list<ecmdLatchEntry> & o_data, ecmdLatchMode_t i_mode, const ecmdDataBuffer & i_ringImage)
{
  uint32_t rc = 0;

  ecmdLatchBufferEntry curEntry;
  std::list< ecmdLatchEntry >::iterator curLatchInfo;    ///< Iterator for walking through latches
  ecmdDataBuffer ringBuffer = i_ringImage;               ///< Buffer to store entire ring
  ecmdDataBuffer buffer(5000 /* bits */);        ///< Space for extracted latch data
  ecmdDataBuffer buffertemp(5000 /* bits */);    ///< Temp space for extracted latch data
  ecmdLatchEntry curData;                       ///< Data to load into return list
  std::string curRing;                          ///< Current ring being operated on
  uint32_t bustype;                             ///< Type of bus we are attached to JTAG vs FSI
  std::string printed;

  o_data.clear();       // Flush out current list

  std::list<ecmdRingData> l_ring_info;
  rc = dllQueryRing(i_target, l_ring_info, i_ringName, ECMD_QUERY_DETAIL_LOW);
  if (!l_ring_info.empty())
  {
      if (i_ringImage.getBitLength() != l_ring_info.begin()->bitLength)
      {
          rc = ECMD_DBUF_MISMATCH;
          dllRegisterErrorMsg(rc, "dllGetLatchImage", "Ring image data buffer isn't the size of the ring being operated on.\n");
          return rc;
      }
  }

  /* Let's find out if we are JTAG of FSI here */
  rc = dllGetScandefOrder(i_target, bustype);
  if (rc) {
    printed = "Problems retrieving chip information on target\n";
    dllRegisterErrorMsg(rc, "dllGetLatchImage", printed.c_str() );
    return rc;
  }
  /* Now make sure the plugin gave us some bus info */
  if ((bustype != ECMD_CHIPFLAG_JTAG) && (bustype != ECMD_CHIPFLAG_FSI) ) {
    dllRegisterErrorMsg(ECMD_DLL_INVALID, "dllGetLatchImage", "eCMD plugin returned an invalid bustype in dllGetScandefOrder\n");
    return ECMD_DLL_INVALID;
  }
  
  if( i_mode == ECMD_LATCHMODE_FULL) {
    rc = readScandefHash(i_target, i_ringName, i_latchName, curEntry);
    if( rc && (((rc != ECMD_UNKNOWN_FILE) &&(rc != ECMD_UNABLE_TO_OPEN_SCANDEFHASH)) && ((rc == ECMD_INVALID_LATCHNAME)||(rc ==
															  ECMD_INVALID_RING)||(rc == ECMD_SCANDEFHASH_MULT_RINGS)))) {
      return rc;
    }
  }
  if (rc || (i_mode != ECMD_LATCHMODE_FULL)) {
    rc = readScandef(i_target, i_ringName, i_latchName, i_mode, curEntry);
    if (rc) return rc;
  }

  /* Single exit point */
  while (1) {

    uint32_t bitsToFetch = ECMD_UNSET;       // Grab all bits
    uint32_t curLatchBit = ECMD_UNSET;       // This is the actual latch bit we are looking for next
    uint32_t curBufferBit = 0;               // Current bit to insert into buffered register
    uint32_t curBitsToFetch = bitsToFetch;   // This is the total number of bits left to fetch
    uint32_t dataStartBit = ECMD_UNSET;      // Actual start bit of buffered register
    uint32_t dataEndBit = ECMD_UNSET;        // Actual end bit of buffered register
    std::string latchname = "";
    curRing = "";

    for (curLatchInfo = curEntry.entry.begin(); (curLatchInfo != curEntry.entry.end()) && (curBitsToFetch > 0); curLatchInfo++) {


      /* Let's grab the ring for this latch entry */
      if (curRing != curLatchInfo->ringName) {
        curRing = curLatchInfo->ringName;
      }
      /* Do we have previous data here , or some missing bits in the scandef latchs ?*/
      if (((dataStartBit != ECMD_UNSET) && (curLatchBit !=  curLatchInfo->latchStartBit) && (curLatchBit !=  curLatchInfo->latchEndBit)) ||
          ((latchname == "") || (latchname != curLatchInfo->latchName.substr(0, curLatchInfo->latchName.rfind('('))))) {
        /* I have some good data here */
        if (latchname != "") {

          curData.latchStartBit = (int)dataStartBit;
          curData.latchEndBit = (int)dataEndBit;
          rc = buffer.extract(curData.buffer, 0, dataEndBit - dataStartBit + 1); if (rc) return rc;
          curData.length = curData.buffer.getBitLength();
          curData.rc = ECMD_SUCCESS;
          o_data.push_back(curData);

        }

        /* If this is a fresh one we need to reset everything */
        if ((latchname == "") || (latchname != curLatchInfo->latchName.substr(0, latchname.length()))) {
          dataStartBit = dataEndBit = ECMD_UNSET;
          curBitsToFetch = ECMD_UNSET;
          curBufferBit = 0;
          latchname = curLatchInfo->latchName.substr(0, curLatchInfo->latchName.rfind('('));
          curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchStartBit : curLatchInfo->latchEndBit;
          curData.latchName = latchname;
          curData.ringName = curLatchInfo->ringName;
          curData.latchType = curLatchInfo->latchType;
          curData.fsiRingOffset = curLatchInfo->fsiRingOffset;
          curData.jtagRingOffset = curLatchInfo->jtagRingOffset;
        } else {
          /* This is the case where the scandef had holes in the register, so we will continue with this latch, but skip some bits */
          dataStartBit = dataEndBit = ECMD_UNSET;
          curBufferBit = 0;
          /* Decrement the bits to fetch by the hole in the latch */
          curBitsToFetch -= (curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchStartBit : curLatchInfo->latchEndBit) - curLatchBit;
          curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchStartBit : curLatchInfo->latchEndBit;
        }
      }

      /* Do we want anything in here */
      /* Check if the bits are ordered from:to (0:10) or just (1) */
      if (((curLatchInfo->latchEndBit >= curLatchInfo->latchStartBit) && (curLatchBit <=  curLatchInfo->latchEndBit)) ||
          /* Check if the bits are ordered to:from (10:0) */
          ((curLatchInfo->latchStartBit > curLatchInfo->latchEndBit)  && (curLatchBit <=  curLatchInfo->latchStartBit))) {

        bitsToFetch = (curLatchInfo->length  < curBitsToFetch) ? curLatchInfo->length : curBitsToFetch;

        /* Setup the actual data bits displayed */
        if (dataStartBit == ECMD_UNSET) {
          dataStartBit = curLatchBit;
          dataEndBit = dataStartBit - 1;
        }
        dataEndBit += bitsToFetch;

        /* ********* */
        /* FSI Order */
        /* ********* */

        if (bustype == ECMD_CHIPFLAG_FSI) {
          rc = ringBuffer.extract(buffertemp, curLatchInfo->fsiRingOffset, bitsToFetch); if (rc) return rc;

          /* Extract bits if ordered from:to (0:10) */
          if (curLatchInfo->latchEndBit > curLatchInfo->latchStartBit) {
            curLatchBit = curLatchInfo->latchEndBit + 1;
            /* Extract if bits are ordered to:from (10:0) or just (1) */
          } else {
            if (bitsToFetch > 1) buffertemp.reverse();
            curLatchBit = curLatchInfo->latchStartBit + 1;
          }
          rc = buffer.insert(buffertemp, curBufferBit, bitsToFetch); if (rc) return rc;
	  curBufferBit += bitsToFetch;

	  /* ********* */
	  /*JTAG Order */
	  /* ********* */
        } else {
          rc = ringBuffer.extract(buffertemp, curLatchInfo->jtagRingOffset - bitsToFetch + 1, bitsToFetch); if (rc) return rc;
          /* Extract bits if ordered from:to (0:10) */
          if (curLatchInfo->latchEndBit > curLatchInfo->latchStartBit) {
            buffertemp.reverse();
            curLatchBit = curLatchInfo->latchEndBit + 1;
          } else {
            /* Extract if bits are ordered to:from (10:0) or just (1) */
            curLatchBit = curLatchInfo->latchStartBit + 1;
          }
          rc = buffer.insert(buffertemp, curBufferBit, bitsToFetch); if (rc) return rc;
	  curBufferBit += bitsToFetch;

        }

        curBitsToFetch -= bitsToFetch;
      } else {
        /* Nothing was there that we needed, let's try the next entry */
        curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchEndBit + 1: curLatchInfo->latchStartBit + 1;

      }

    } /* end latchinfo for loop */

    /* We have good data here, let's push it */
    if (latchname != "") {

      /* Display this if we aren't expecting or the expect failed */
      curData.latchStartBit = (int)dataStartBit;
      curData.latchEndBit = (int)dataEndBit;
      rc = buffer.extract(curData.buffer, 0, dataEndBit - dataStartBit + 1); if (rc) return rc;
      curData.length = curData.buffer.getBitLength();
      curData.rc = ECMD_SUCCESS;
      o_data.push_back(curData);

    }

    break;
  } /* end while (exit point) */

  return rc;
}

uint32_t dllPutLatchImage(const ecmdChipTarget & i_target, const char* i_ringName, const char * i_latchName, const ecmdDataBuffer & i_data, uint32_t i_startBit, uint32_t i_numBits, uint32_t & o_matches, ecmdLatchMode_t i_mode, ecmdDataBuffer & io_ringImage)
{
  uint32_t rc = 0;
  ecmdLatchBufferEntry curEntry;
  std::list< ecmdLatchEntry >::iterator curLatchInfo;
  ecmdDataBuffer bufferCopy;            ///< Copy of data to be inserted
  ecmdDataBuffer buffertemp;            ///< Temp buffer to allow reversing in JTAG mode
  std::string curRing;                  ///< Current ring being operated on
  std::string curLatchName;             ///< Current latch name being operated on
  uint32_t bustype;                             ///< Type of bus we are attached to JTAG vs FSI
  std::string printed;

  o_matches = 0;

  /* Do we have the right amount of data ? */
  if (i_data.getBitLength() > i_numBits) {
    rc = ECMD_DATA_OVERFLOW;
    dllRegisterErrorMsg(rc, "dllPutLatchImage", "Data buffer length is greater than numBits requested to write\n");
    return rc;
  } else if (i_data.getBitLength() < i_numBits) {
    rc = ECMD_DATA_UNDERFLOW;
    dllRegisterErrorMsg(rc, "dllPutLatchImage", "Data buffer length is less than numBits requested to write\n");
    return rc;
  }

  std::list<ecmdRingData> l_ring_info;
  rc = dllQueryRing(i_target, l_ring_info, i_ringName, ECMD_QUERY_DETAIL_LOW);
  if (!l_ring_info.empty())
  {
      if (io_ringImage.getBitLength() != l_ring_info.begin()->bitLength)
      {
          rc = ECMD_DBUF_MISMATCH;
          dllRegisterErrorMsg(rc, "dllPutLatchImage", "Ring image data buffer isn't the size of the ring being operated on.\n");
          return rc;
      }
  }

  /* Let's find out if we are JTAG of FSI here */
  rc = dllGetScandefOrder(i_target, bustype);
  if (rc) {
    dllRegisterErrorMsg(rc, "dllPutLatchImage", "Problems retrieving chip information on target\n");
    return rc;
  }
  /* Now make sure the plugin gave us some bus info */
  if ((bustype != ECMD_CHIPFLAG_JTAG) && (bustype != ECMD_CHIPFLAG_FSI)) {
    dllRegisterErrorMsg(ECMD_DLL_INVALID, "dllPutLatchImage", "eCMD plugin returned an invalid bustype in dllGetScandefOrder\n");
    return ECMD_DLL_INVALID;
  }

  if( i_mode == ECMD_LATCHMODE_FULL) {
    rc = readScandefHash(i_target, i_ringName, i_latchName, curEntry);
    if( rc && (((rc != ECMD_UNKNOWN_FILE) &&(rc != ECMD_UNABLE_TO_OPEN_SCANDEFHASH)) && ((rc == ECMD_INVALID_LATCHNAME)||(rc == ECMD_INVALID_RING)||(rc == ECMD_SCANDEFHASH_MULT_RINGS)))) {
      return rc;
    }
  }
  if (rc || (i_mode != ECMD_LATCHMODE_FULL)) {
    std::string errorParse;
    if (rc)
    {
      errorParse = dllGetErrorMsg(rc, false, true, true);
      errorParse = "WARNING: Latch is found in Scandef File but is missing in Hashfile \n" + errorParse;

    }
    rc = readScandef(i_target, i_ringName, i_latchName, i_mode, curEntry);
    if (rc) return rc;

    if (errorParse.length() > 0) {

       dllOutputWarning(errorParse.c_str());
    }
  }

  /* single exit point */
  do {
    uint32_t bitsToInsert = i_numBits;
    uint32_t curLatchBit = ECMD_UNSET;          // This is the actual latch bit we are looking for next
    uint32_t curBitsToInsert = bitsToInsert;    // This is the total number of bits left to Insert
    uint32_t curStartBitToInsert = 0;           // This is the offset into the current entry for insertion
    uint32_t curStart = 0;
    uint32_t curEnd = 0;

    for (curLatchInfo = curEntry.entry.begin(); curLatchInfo != curEntry.entry.end(); curLatchInfo++) {

      /* Let's grab the ring for this latch entry */
      if (curRing != curLatchInfo->ringName) {
        curRing = curLatchInfo->ringName;
      }

      if ((curLatchBit == ECMD_UNSET) ||(curLatchName != curLatchInfo->latchName.substr(0, curLatchInfo->latchName.rfind('(')))) {
        curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchStartBit : curLatchInfo->latchEndBit;
        curLatchName = curLatchInfo->latchName.substr(0, curLatchInfo->latchName.rfind('('));
        /* Make a copy of insert data so we don't lose it */
        bufferCopy = i_data;
        bitsToInsert = i_numBits;
        curBitsToInsert = bitsToInsert;
        curStart = i_startBit;
        curEnd = i_startBit + bitsToInsert - 1;

        /* We found something, bump the matches */
        o_matches++;
      }

      bool doWrite = false;
      /* Check if the bits are ordered from:to (0:10) or just (1) and we want to modify */
      if ((curLatchInfo->latchEndBit >= curLatchInfo->latchStartBit) && (curStart <= curLatchInfo->latchEndBit) && (curEnd >= curLatchInfo->latchStartBit)) {
        if(curStart < curLatchInfo->latchStartBit) { // move counter to start of latch if lower than current latch
          curStart = curLatchInfo->latchStartBit;
        }
        if (bustype == ECMD_CHIPFLAG_FSI) {
          if(curStart > curLatchInfo->latchStartBit) { // case where we start inside the latch
            curStartBitToInsert = curStart - curLatchInfo->latchStartBit;
          } else {
            curStartBitToInsert = 0;
          }
          bitsToInsert = ((curLatchInfo->length - curStartBitToInsert) < curBitsToInsert ? (curLatchInfo->length - curStartBitToInsert) : curBitsToInsert);
        } else {
          // JTAG -- not sure if this is correct for more than one bit
          //      -- this is a modified version of the FSI high:low order code
          //      -- with latchEndBit and latchStartbit swapped when appropriate
          //      -- mklight 03/13/2009
          // figure out where we should start
          int32_t tempStartBitToInsert = curLatchInfo->latchEndBit - (curStart + curBitsToInsert - 1);
          // if we go negative we should start at zero
          if(tempStartBitToInsert <= 0) { // we start at start bit
            curStartBitToInsert = 0;
            if(curStart > curLatchInfo->latchStartBit) { // case where we start inside the latch
              bitsToInsert = curLatchInfo->length - (curStart - curLatchInfo->latchStartBit);
            } else { // otherwise we start at the end of the latch
              bitsToInsert = curLatchInfo->length;
            }
          } else { // we start after start bit, end of latch will not be filled, this will be the last latch
            curStartBitToInsert = (uint32_t) tempStartBitToInsert;
            bitsToInsert = curBitsToInsert;
          }
        }
        curLatchBit = curLatchInfo->latchEndBit + 1;
        doWrite = true;
      }

      /* Check if the bits are ordered to:from (10:0) */
      if ((curLatchInfo->latchStartBit > curLatchInfo->latchEndBit) && (curStart <= curLatchInfo->latchStartBit) && (curEnd >= curLatchInfo->latchEndBit)) {
        if(curStart < curLatchInfo->latchEndBit) { // move counter to start of latch if lower than current latch
          curStart = curLatchInfo->latchEndBit;
        }
        if (bustype == ECMD_CHIPFLAG_FSI) {
          // figure out where we should start
         int32_t tempStartBitToInsert = curLatchInfo->latchStartBit - (curStart + curBitsToInsert - 1);
          // if we go negative we should start at zero
          if(tempStartBitToInsert <= 0) { // we start at start bit
            curStartBitToInsert = 0;
            if(curStart > curLatchInfo->latchEndBit) { // case where we start inside the latch
              bitsToInsert = curLatchInfo->length - (curStart - curLatchInfo->latchEndBit);
            } else { // otherwise we start at the end of the latch
              bitsToInsert = curLatchInfo->length;
            }
          } else { // we start after start bit, end of latch will not be filled, this will be the last latch
            curStartBitToInsert = (uint32_t) tempStartBitToInsert;
            bitsToInsert = curBitsToInsert;
          }
        } else {
          if(curStart > curLatchInfo->latchEndBit) { // case where we start inside the latch
            curStartBitToInsert = curStart - curLatchInfo->latchEndBit;
          } else {
            curStartBitToInsert = 0;
          }
          bitsToInsert = ((curLatchInfo->length - curStartBitToInsert) < curBitsToInsert ? (curLatchInfo->length - curStartBitToInsert) : curBitsToInsert);
        }
        curLatchBit = curLatchInfo->latchStartBit + 1;
        doWrite = true;
      }

      if (doWrite == true)
      {
        rc = bufferCopy.extract(buffertemp, 0, bitsToInsert); if (rc) return rc;
        //printf("curLatchBit = %d, startbit = %d, endbit = %d, length = %d, curStartBitToInsert = %d, bits = %d\n", curLatchBit, curLatchInfo->latchStartBit, curLatchInfo->latchEndBit, curLatchInfo->length, curStartBitToInsert,  bitsToInsert);
        /* ********* */
        /* FSI Order */
        /* ********* */
        if (bustype == ECMD_CHIPFLAG_FSI) {
          /* if bits are ordered to:from (10:0) or just (1) */
          if (curLatchInfo->latchEndBit < curLatchInfo->latchStartBit) {
            /* Reverse the data to go into the scanring */
            buffertemp.reverse();
          }

          //printf("inserting %d bits at offset %d\n", bitsToInsert, curLatchInfo->fsiRingOffset + curStartBitToInsert);
          rc = io_ringImage.insert(buffertemp, curLatchInfo->fsiRingOffset + curStartBitToInsert, bitsToInsert);
          if (rc) return rc;

        }
        /* ********** */
        /* JTAG Order */
        /* ********** */
        else {
          /* if ordered from:to (0:10) */
          if (curLatchInfo->latchEndBit > curLatchInfo->latchStartBit) {
            /* Reverse the data to go into the scanring */
            buffertemp.reverse();
          }

          //printf("inserting %d bits at offset %d\n", bitsToInsert, curLatchInfo->jtagRingOffset - curLatchInfo->length  + 1 + curStartBitToInsert);
          rc = io_ringImage.insert(buffertemp, curLatchInfo->jtagRingOffset - curLatchInfo->length  + 1 + curStartBitToInsert, bitsToInsert);
          if (rc) return rc;

        }

        /* Get rid of the data we just inserted, to line up the next piece */
        bufferCopy.shiftLeft(bitsToInsert);

        curBitsToInsert -= bitsToInsert;
      } else {
        /* Nothing was there that we needed, let's try the next entry */
        curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchEndBit + 1: curLatchInfo->latchStartBit + 1;
      }
    }

  } while(0);

  return rc;
}

uint32_t dllGetLatchImageOpt(const ecmdChipTarget & i_target, std::list<ecmdLatchEntry> & o_data, const std::list<ecmdLatchEntry> & i_scandefLatchInfo, const ecmdDataBuffer & i_ringImage)
{
  uint32_t rc = 0;

  std::list< ecmdLatchEntry >::const_iterator curLatchInfo;    ///< Iterator for walking through latches
  ecmdDataBuffer ringBuffer = i_ringImage;               ///< Buffer to store entire ring
  ecmdDataBuffer buffer(5000 /* bits */);        ///< Space for extracted latch data
  ecmdDataBuffer buffertemp(5000 /* bits */);    ///< Temp space for extracted latch data
  ecmdLatchEntry curData;                       ///< Data to load into return list
  std::string curRing;                          ///< Current ring being operated on
  uint32_t bustype;                             ///< Type of bus we are attached to JTAG vs FSI
  std::string printed;
  std::list<ecmdRingData> l_ring_info;

  o_data.clear();       // Flush out current list


  /* Let's find out if we are JTAG of FSI here */
  rc = dllGetScandefOrder(i_target, bustype);
  if (rc) {
    printed = "Problems retrieving chip information on target\n";
    dllRegisterErrorMsg(rc, "dllGetLatchImageOpt", printed.c_str() );
    return rc;
  }
  /* Now make sure the plugin gave us some bus info */
  if ((bustype != ECMD_CHIPFLAG_JTAG) && (bustype != ECMD_CHIPFLAG_FSI) ) {
    dllRegisterErrorMsg(ECMD_DLL_INVALID, "dllGetLatchImageOpt", "eCMD plugin returned an invalid bustype in dllGetScandefOrder\n");
    return ECMD_DLL_INVALID;
  }
 
  /* Single exit point */
  while (1) {

    uint32_t bitsToFetch = ECMD_UNSET;       // Grab all bits
    uint32_t curLatchBit = ECMD_UNSET;       // This is the actual latch bit we are looking for next
    uint32_t curBufferBit = 0;               // Current bit to insert into buffered register
    uint32_t curBitsToFetch = bitsToFetch;   // This is the total number of bits left to fetch
    uint32_t dataStartBit = ECMD_UNSET;      // Actual start bit of buffered register
    uint32_t dataEndBit = ECMD_UNSET;        // Actual end bit of buffered register
    std::string latchname = "";
    curRing = "";

    for (curLatchInfo = i_scandefLatchInfo.begin(); (curLatchInfo != i_scandefLatchInfo.end()) && (curBitsToFetch > 0); curLatchInfo++) {


      /* Let's grab the ring for this latch entry */
      if (curRing != curLatchInfo->ringName) {
        curRing = curLatchInfo->ringName;
        rc = dllQueryRing(i_target, l_ring_info, curRing.c_str(), ECMD_QUERY_DETAIL_LOW);
        if (!l_ring_info.empty())
        {
            if (i_ringImage.getBitLength() != l_ring_info.begin()->bitLength)
            {
                rc = ECMD_DBUF_MISMATCH;
                dllRegisterErrorMsg(rc, "dllPutLatchImageOpt", "Ring image data buffer isn't the size of the ring being operated on.\n");
                return rc;
            }
        }
      }
      /* Do we have previous data here , or some missing bits in the scandef latchs ?*/
      if (((dataStartBit != ECMD_UNSET) && (curLatchBit !=  curLatchInfo->latchStartBit) && (curLatchBit !=  curLatchInfo->latchEndBit)) ||
          ((latchname == "") || (latchname != curLatchInfo->latchName.substr(0, curLatchInfo->latchName.rfind('('))))) {
        /* I have some good data here */
        if (latchname != "") {

          curData.latchStartBit = (int)dataStartBit;
          curData.latchEndBit = (int)dataEndBit;
          rc = buffer.extract(curData.buffer, 0, dataEndBit - dataStartBit + 1); if (rc) return rc;
          curData.length = curData.buffer.getBitLength();
          curData.rc = ECMD_SUCCESS;
          o_data.push_back(curData);

        }

        /* If this is a fresh one we need to reset everything */
        if ((latchname == "") || (latchname != curLatchInfo->latchName.substr(0, latchname.length()))) {
          dataStartBit = dataEndBit = ECMD_UNSET;
          curBitsToFetch = ECMD_UNSET;
          curBufferBit = 0;
          latchname = curLatchInfo->latchName.substr(0, curLatchInfo->latchName.rfind('('));
          curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchStartBit : curLatchInfo->latchEndBit;
          curData.latchName = latchname;
          curData.ringName = curLatchInfo->ringName;
          curData.latchType = curLatchInfo->latchType;
          curData.fsiRingOffset = curLatchInfo->fsiRingOffset;
          curData.jtagRingOffset = curLatchInfo->jtagRingOffset;
        } else {
          /* This is the case where the scandef had holes in the register, so we will continue with this latch, but skip some bits */
          dataStartBit = dataEndBit = ECMD_UNSET;
          curBufferBit = 0;
          /* Decrement the bits to fetch by the hole in the latch */
          curBitsToFetch -= (curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchStartBit : curLatchInfo->latchEndBit) - curLatchBit;
          curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchStartBit : curLatchInfo->latchEndBit;
        }
      }

      /* Do we want anything in here */
      /* Check if the bits are ordered from:to (0:10) or just (1) */
      if (((curLatchInfo->latchEndBit >= curLatchInfo->latchStartBit) && (curLatchBit <=  curLatchInfo->latchEndBit)) ||
          /* Check if the bits are ordered to:from (10:0) */
          ((curLatchInfo->latchStartBit > curLatchInfo->latchEndBit)  && (curLatchBit <=  curLatchInfo->latchStartBit))) {

        bitsToFetch = (curLatchInfo->length  < curBitsToFetch) ? curLatchInfo->length : curBitsToFetch;

        /* Setup the actual data bits displayed */
        if (dataStartBit == ECMD_UNSET) {
          dataStartBit = curLatchBit;
          dataEndBit = dataStartBit - 1;
        }
        dataEndBit += bitsToFetch;

        /* ********* */
        /* FSI Order */
        /* ********* */

        if (bustype == ECMD_CHIPFLAG_FSI) {
          rc = ringBuffer.extract(buffertemp, curLatchInfo->fsiRingOffset, bitsToFetch); if (rc) return rc;

          /* Extract bits if ordered from:to (0:10) */
          if (curLatchInfo->latchEndBit > curLatchInfo->latchStartBit) {
            curLatchBit = curLatchInfo->latchEndBit + 1;
            /* Extract if bits are ordered to:from (10:0) or just (1) */
          } else {
            if (bitsToFetch > 1) buffertemp.reverse();
            curLatchBit = curLatchInfo->latchStartBit + 1;
          }
          rc = buffer.insert(buffertemp, curBufferBit, bitsToFetch); if (rc) return rc;
	  curBufferBit += bitsToFetch;

	  /* ********* */
	  /*JTAG Order */
	  /* ********* */
        } else {
          rc = ringBuffer.extract(buffertemp, curLatchInfo->jtagRingOffset - bitsToFetch + 1, bitsToFetch); if (rc) return rc;
          /* Extract bits if ordered from:to (0:10) */
          if (curLatchInfo->latchEndBit > curLatchInfo->latchStartBit) {
            buffertemp.reverse();
            curLatchBit = curLatchInfo->latchEndBit + 1;
          } else {
            /* Extract if bits are ordered to:from (10:0) or just (1) */
            curLatchBit = curLatchInfo->latchStartBit + 1;
          }
          rc = buffer.insert(buffertemp, curBufferBit, bitsToFetch); if (rc) return rc;
	  curBufferBit += bitsToFetch;

        }

        curBitsToFetch -= bitsToFetch;
      } else {
        /* Nothing was there that we needed, let's try the next entry */
        curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchEndBit + 1: curLatchInfo->latchStartBit + 1;

      }

    } /* end latchinfo for loop */

    /* We have good data here, let's push it */
    if (latchname != "") {

      /* Display this if we aren't expecting or the expect failed */
      curData.latchStartBit = (int)dataStartBit;
      curData.latchEndBit = (int)dataEndBit;
      rc = buffer.extract(curData.buffer, 0, dataEndBit - dataStartBit + 1); if (rc) return rc;
      curData.length = curData.buffer.getBitLength();
      curData.rc = ECMD_SUCCESS;
      o_data.push_back(curData);

    }

    break;
  } /* end while (exit point) */

  return rc;
}

uint32_t dllPutLatchImageOpt(const ecmdChipTarget & i_target, const ecmdDataBuffer & i_data, uint32_t i_startBit, uint32_t i_numBits, uint32_t & o_matches, const std::list<ecmdLatchEntry> & i_scandefLatchInfo, ecmdDataBuffer & io_ringImage)
{
  uint32_t rc = 0;
  std::list< ecmdLatchEntry >::const_iterator curLatchInfo;
  ecmdDataBuffer bufferCopy;            ///< Copy of data to be inserted
  ecmdDataBuffer buffertemp;            ///< Temp buffer to allow reversing in JTAG mode
  std::string curRing;                  ///< Current ring being operated on
  std::string curLatchName;             ///< Current latch name being operated on
  uint32_t bustype;                             ///< Type of bus we are attached to JTAG vs FSI
  std::string printed;
  std::list<ecmdRingData> l_ring_info;

  o_matches = 0;

  /* Do we have the right amount of data ? */
  if (i_data.getBitLength() > i_numBits) {
    rc = ECMD_DATA_OVERFLOW;
    dllRegisterErrorMsg(rc, "dllPutLatchImageOpt", "Data buffer length is greater than numBits requested to write\n");
    return rc;
  } else if (i_data.getBitLength() < i_numBits) {
    rc = ECMD_DATA_UNDERFLOW;
    dllRegisterErrorMsg(rc, "dllPutLatchImageOpt", "Data buffer length is less than numBits requested to write\n");
    return rc;
  }


  /* Let's find out if we are JTAG of FSI here */
  rc = dllGetScandefOrder(i_target, bustype);
  if (rc) {
    dllRegisterErrorMsg(rc, "dllPutLatchImageOpt", "Problems retrieving chip information on target\n");
    return rc;
  }
  /* Now make sure the plugin gave us some bus info */
  if ((bustype != ECMD_CHIPFLAG_JTAG) && (bustype != ECMD_CHIPFLAG_FSI)) {
    dllRegisterErrorMsg(ECMD_DLL_INVALID, "dllPutLatchImageOpt", "eCMD plugin returned an invalid bustype in dllGetScandefOrder\n");
    return ECMD_DLL_INVALID;
  }


  /* single exit point */
  do {
    uint32_t bitsToInsert = i_numBits;
    uint32_t curLatchBit = ECMD_UNSET;          // This is the actual latch bit we are looking for next
    uint32_t curBitsToInsert = bitsToInsert;    // This is the total number of bits left to Insert
    uint32_t curStartBitToInsert = 0;           // This is the offset into the current entry for insertion
    uint32_t curStart = 0;
    uint32_t curEnd = 0;

    for (curLatchInfo = i_scandefLatchInfo.begin(); curLatchInfo != i_scandefLatchInfo.end(); curLatchInfo++) {

      /* Let's grab the ring for this latch entry */
      if (curRing != curLatchInfo->ringName) {
        curRing = curLatchInfo->ringName;

        rc = dllQueryRing(i_target, l_ring_info, curRing.c_str(), ECMD_QUERY_DETAIL_LOW);
        if (!l_ring_info.empty())
        {
            if (io_ringImage.getBitLength() != l_ring_info.begin()->bitLength)
            {
                rc = ECMD_DBUF_MISMATCH;
                dllRegisterErrorMsg(rc, "dllPutLatchImageOpt", "Ring image data buffer isn't the size of the ring being operated on.\n");
                return rc;
            }
        }
      }

      if ((curLatchBit == ECMD_UNSET) ||(curLatchName != curLatchInfo->latchName.substr(0, curLatchInfo->latchName.rfind('(')))) {
        curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchStartBit : curLatchInfo->latchEndBit;
        curLatchName = curLatchInfo->latchName.substr(0, curLatchInfo->latchName.rfind('('));
        /* Make a copy of insert data so we don't lose it */
        bufferCopy = i_data;
        bitsToInsert = i_numBits;
        curBitsToInsert = bitsToInsert;
        curStart = i_startBit;
        curEnd = i_startBit + bitsToInsert - 1;

        /* We found something, bump the matches */
        o_matches++;
      }

      bool doWrite = false;
      /* Check if the bits are ordered from:to (0:10) or just (1) and we want to modify */
      if ((curLatchInfo->latchEndBit >= curLatchInfo->latchStartBit) && (curStart <= curLatchInfo->latchEndBit) && (curEnd >= curLatchInfo->latchStartBit)) {
        if(curStart < curLatchInfo->latchStartBit) { // move counter to start of latch if lower than current latch
          curStart = curLatchInfo->latchStartBit;
        }
        if (bustype == ECMD_CHIPFLAG_FSI) {
          if(curStart > curLatchInfo->latchStartBit) { // case where we start inside the latch
            curStartBitToInsert = curStart - curLatchInfo->latchStartBit;
          } else {
            curStartBitToInsert = 0;
          }
          bitsToInsert = ((curLatchInfo->length - curStartBitToInsert) < curBitsToInsert ? (curLatchInfo->length - curStartBitToInsert) : curBitsToInsert);
        } else {
          // JTAG -- not sure if this is correct for more than one bit
          //      -- this is a modified version of the FSI high:low order code
          //      -- with latchEndBit and latchStartbit swapped when appropriate
          //      -- mklight 03/13/2009
          // figure out where we should start
          int32_t tempStartBitToInsert = curLatchInfo->latchEndBit - (curStart + curBitsToInsert - 1);
          // if we go negative we should start at zero
          if(tempStartBitToInsert <= 0) { // we start at start bit
            curStartBitToInsert = 0;
            if(curStart > curLatchInfo->latchStartBit) { // case where we start inside the latch
              bitsToInsert = curLatchInfo->length - (curStart - curLatchInfo->latchStartBit);
            } else { // otherwise we start at the end of the latch
              bitsToInsert = curLatchInfo->length;
            }
          } else { // we start after start bit, end of latch will not be filled, this will be the last latch
            curStartBitToInsert = (uint32_t) tempStartBitToInsert;
            bitsToInsert = curBitsToInsert;
          }
        }
        curLatchBit = curLatchInfo->latchEndBit + 1;
        doWrite = true;
      }

      /* Check if the bits are ordered to:from (10:0) */
      if ((curLatchInfo->latchStartBit > curLatchInfo->latchEndBit) && (curStart <= curLatchInfo->latchStartBit) && (curEnd >= curLatchInfo->latchEndBit)) {
        if(curStart < curLatchInfo->latchEndBit) { // move counter to start of latch if lower than current latch
          curStart = curLatchInfo->latchEndBit;
        }
        if (bustype == ECMD_CHIPFLAG_FSI) {
          // figure out where we should start
         int32_t tempStartBitToInsert = curLatchInfo->latchStartBit - (curStart + curBitsToInsert - 1);
          // if we go negative we should start at zero
          if(tempStartBitToInsert <= 0) { // we start at start bit
            curStartBitToInsert = 0;
            if(curStart > curLatchInfo->latchEndBit) { // case where we start inside the latch
              bitsToInsert = curLatchInfo->length - (curStart - curLatchInfo->latchEndBit);
            } else { // otherwise we start at the end of the latch
              bitsToInsert = curLatchInfo->length;
            }
          } else { // we start after start bit, end of latch will not be filled, this will be the last latch
            curStartBitToInsert = (uint32_t) tempStartBitToInsert;
            bitsToInsert = curBitsToInsert;
          }
        } else {
          if(curStart > curLatchInfo->latchEndBit) { // case where we start inside the latch
            curStartBitToInsert = curStart - curLatchInfo->latchEndBit;
          } else {
            curStartBitToInsert = 0;
          }
          bitsToInsert = ((curLatchInfo->length - curStartBitToInsert) < curBitsToInsert ? (curLatchInfo->length - curStartBitToInsert) : curBitsToInsert);
        }
        curLatchBit = curLatchInfo->latchStartBit + 1;
        doWrite = true;
      }

      if (doWrite == true)
      {
        rc = bufferCopy.extract(buffertemp, 0, bitsToInsert); if (rc) return rc;
        //printf("curLatchBit = %d, startbit = %d, endbit = %d, length = %d, curStartBitToInsert = %d, bits = %d\n", curLatchBit, curLatchInfo->latchStartBit, curLatchInfo->latchEndBit, curLatchInfo->length, curStartBitToInsert,  bitsToInsert);
        /* ********* */
        /* FSI Order */
        /* ********* */
        if (bustype == ECMD_CHIPFLAG_FSI) {
          /* if bits are ordered to:from (10:0) or just (1) */
          if (curLatchInfo->latchEndBit < curLatchInfo->latchStartBit) {
            /* Reverse the data to go into the scanring */
            buffertemp.reverse();
          }

          //printf("inserting %d bits at offset %d\n", bitsToInsert, curLatchInfo->fsiRingOffset + curStartBitToInsert);
          rc = io_ringImage.insert(buffertemp, curLatchInfo->fsiRingOffset + curStartBitToInsert, bitsToInsert);
          if (rc) return rc;

        }
        /* ********** */
        /* JTAG Order */
        /* ********** */
        else {
          /* if ordered from:to (0:10) */
          if (curLatchInfo->latchEndBit > curLatchInfo->latchStartBit) {
            /* Reverse the data to go into the scanring */
            buffertemp.reverse();
          }

          //printf("inserting %d bits at offset %d\n", bitsToInsert, curLatchInfo->jtagRingOffset - curLatchInfo->length  + 1 + curStartBitToInsert);
          rc = io_ringImage.insert(buffertemp, curLatchInfo->jtagRingOffset - curLatchInfo->length  + 1 + curStartBitToInsert, bitsToInsert);
          if (rc) return rc;

        }

        /* Get rid of the data we just inserted, to line up the next piece */
        bufferCopy.shiftLeft(bitsToInsert);

        curBitsToInsert -= bitsToInsert;
      } else {
        /* Nothing was there that we needed, let's try the next entry */
        curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchEndBit + 1: curLatchInfo->latchStartBit + 1;
      }
    }

  } while(0);

  return rc;
}

uint32_t dllCreateSparseMaskFromLatch(const ecmdChipTarget & i_target, const char* i_ringName, const char * i_latchName, ecmdDataBuffer & o_mask, ecmdLatchMode_t i_mode, uint32_t i_startBit, uint32_t i_numBits)
{
    uint32_t rc = ECMD_SUCCESS;
    ecmdLatchBufferEntry curEntry;
    std::list< ecmdLatchEntry >::iterator curLatchInfo;    ///< Iterator for walking through latches
    ecmdDataBuffer ringBuffer;                    ///< Buffer to store entire ring
    ecmdDataBuffer buffer(5000 /* bits */);        ///< Space for extracted latch data
    std::string curRing;                          ///< Current ring being operated on
    uint32_t bustype;                             ///< Type of bus we are attached to JTAG vs FSI
    std::string curLatchName;                     ///< Current latch name being operated on
    std::string printed;

    // Both i_startBit and i_numBits have to be either ECMD_UNSET or not ECMD_UNSET
    if (((i_startBit == ECMD_UNSET) && (i_numBits != ECMD_UNSET)) || ((i_startBit != ECMD_UNSET) && (i_numBits == ECMD_UNSET)))
    {
        printed = "Both i_startBit and i_numBits have to be either ECMD_UNSET or not ECMD_UNSET\n";
        dllRegisterErrorMsg(ECMD_INVALID_ARGS, "dllCreateSparseMaskFromLatch", printed.c_str() );
        return ECMD_INVALID_ARGS;
    }

    std::list<ecmdRingData> l_ring_info;
    rc = dllQueryRing(i_target, l_ring_info, i_ringName, ECMD_QUERY_DETAIL_LOW);
    if (!l_ring_info.empty())
    {
        o_mask.setBitLength(l_ring_info.begin()->bitLength);
    }

    /* Let's find out if we are JTAG of FSI here */
    rc = dllGetScandefOrder(i_target, bustype);
    if (rc) {
        printed = "Problems retrieving chip information on target\n";
        dllRegisterErrorMsg(rc, "dllCreateSparseMaskFromLatch", printed.c_str() );
        return rc;
    }
    /* Now make sure the plugin gave us some bus info */
    if ((bustype != ECMD_CHIPFLAG_JTAG) && (bustype != ECMD_CHIPFLAG_FSI) ) {
        dllRegisterErrorMsg(ECMD_DLL_INVALID, "dllCreateSparseMaskFromLatch", "eCMD plugin returned an invalid bustype in dllGetScandefOrder\n");
        return ECMD_DLL_INVALID;
    }
  
    if( i_mode == ECMD_LATCHMODE_FULL) {
        rc = readScandefHash(i_target, i_ringName, i_latchName, curEntry);
        if( rc && (((rc != ECMD_UNKNOWN_FILE) &&(rc != ECMD_UNABLE_TO_OPEN_SCANDEFHASH)) && ((rc == ECMD_INVALID_LATCHNAME)||(rc ==
                                                                                                                              ECMD_INVALID_RING)||(rc == ECMD_SCANDEFHASH_MULT_RINGS)))) {
            return rc;
        }
    }
    if (rc || (i_mode != ECMD_LATCHMODE_FULL)) {
        std::string errorParse;
        if (rc)
        {
            errorParse = dllGetErrorMsg(rc, false, true, true);
            errorParse = "WARNING: Latch is found in Scandef File but is missing in Hashfile \n" + errorParse;
        }
        rc = readScandef(i_target, i_ringName, i_latchName, i_mode, curEntry);
        if (rc) return rc;
        
        if (errorParse.length() > 0) {            
            dllOutputWarning(errorParse.c_str());
        }
    }

    // We're going to use the logic copied from getLatch if these values weren't set
    if ((i_startBit == ECMD_UNSET) && (i_numBits == ECMD_UNSET))
    {
        /* Single exit point */
        while (1) {
            
            uint32_t bitsToFetch = ECMD_UNSET;       // Grab all bits
            uint32_t curLatchBit = ECMD_UNSET;       // This is the actual latch bit we are looking for next
            uint32_t curBufferBit = 0;               // Current bit to insert into buffered register
            uint32_t curBitsToFetch = bitsToFetch;   // This is the total number of bits left to fetch
            uint32_t dataStartBit = ECMD_UNSET;      // Actual start bit of buffered register
            uint32_t dataEndBit = ECMD_UNSET;        // Actual end bit of buffered register
            std::string latchname = "";
            curRing = "";

            for (curLatchInfo = curEntry.entry.begin(); (curLatchInfo != curEntry.entry.end()) && (curBitsToFetch > 0); curLatchInfo++)
            {    
                /* Let's grab the ring for this latch entry */
                if (curRing != curLatchInfo->ringName) {
                    curRing = curLatchInfo->ringName;
                }
                /* Do we have previous data here , or some missing bits in the scandef latchs ?*/
                if (((dataStartBit != ECMD_UNSET) && (curLatchBit !=  curLatchInfo->latchStartBit) && (curLatchBit !=  curLatchInfo->latchEndBit)) ||
                    ((latchname == "") || (latchname != curLatchInfo->latchName.substr(0, curLatchInfo->latchName.rfind('('))))) 
                {                    
                    /* If this is a fresh one we need to reset everything */
                    if ((latchname == "") || (latchname != curLatchInfo->latchName.substr(0, latchname.length()))) {
                        dataStartBit = dataEndBit = ECMD_UNSET;
                        curBitsToFetch = ECMD_UNSET;
                        curBufferBit = 0;
                        latchname = curLatchInfo->latchName.substr(0, curLatchInfo->latchName.rfind('('));
                        curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchStartBit : curLatchInfo->latchEndBit;
                    } else {
                        /* This is the case where the scandef had holes in the register, so we will continue with this latch, but skip some bits */
                        dataStartBit = dataEndBit = ECMD_UNSET;
                        curBufferBit = 0;
                        /* Decrement the bits to fetch by the hole in the latch */
                        curBitsToFetch -= (curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchStartBit : curLatchInfo->latchEndBit) - curLatchBit;
                        curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchStartBit : curLatchInfo->latchEndBit;
                    }
                }

                /* Do we want anything in here */
                /* Check if the bits are ordered from:to (0:10) or just (1) */
                if (((curLatchInfo->latchEndBit >= curLatchInfo->latchStartBit) && (curLatchBit <=  curLatchInfo->latchEndBit)) ||
                    /* Check if the bits are ordered to:from (10:0) */
                    ((curLatchInfo->latchStartBit > curLatchInfo->latchEndBit)  && (curLatchBit <=  curLatchInfo->latchStartBit))) 
                {                    
                    bitsToFetch = (curLatchInfo->length  < curBitsToFetch) ? curLatchInfo->length : curBitsToFetch;
                    
                    /* Setup the actual data bits displayed */
                    if (dataStartBit == ECMD_UNSET) {
                        dataStartBit = curLatchBit;
                        dataEndBit = dataStartBit - 1;
                    }
                    dataEndBit += bitsToFetch;

                    /* ********* */
                    /* FSI Order */
                    /* ********* */
                    
                    if (bustype == ECMD_CHIPFLAG_FSI) {
                        rc = o_mask.setBit(curLatchInfo->fsiRingOffset, bitsToFetch); if (rc) return rc;
                        
                        /* Extract bits if ordered from:to (0:10) */
                        if (curLatchInfo->latchEndBit > curLatchInfo->latchStartBit) {
                            curLatchBit = curLatchInfo->latchEndBit + 1;
                            /* Extract if bits are ordered to:from (10:0) or just (1) */
                        } else {
                            //if (bitsToFetch > 1) buffertemp.reverse();
                            curLatchBit = curLatchInfo->latchStartBit + 1;
                        }
                        curBufferBit += bitsToFetch;
                        
                        /* ********* */
                        /*JTAG Order */
                        /* ********* */
                    } else {
                        rc = o_mask.setBit(curLatchInfo->jtagRingOffset, bitsToFetch); if (rc) return rc;
                        /* Extract bits if ordered from:to (0:10) */
                        if (curLatchInfo->latchEndBit > curLatchInfo->latchStartBit) {
                            curLatchBit = curLatchInfo->latchEndBit + 1;
                        } else {
                            /* Extract if bits are ordered to:from (10:0) or just (1) */
                            curLatchBit = curLatchInfo->latchStartBit + 1;
                        }
                        curBufferBit += bitsToFetch;
                    }                    
                    curBitsToFetch -= bitsToFetch;
                } else {
                    /* Nothing was there that we needed, let's try the next entry */
                    curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchEndBit + 1: curLatchInfo->latchStartBit + 1;
                    
                }
                
            } /* end latchinfo for loop */                                
            break;
        } /* end while (exit point) */
    }
    // Use the logic from putLatch if we had values for i_numBits and i_startBit
    else
    {
        // Stuff from putLatch
        
        /* single exit point */
        do {
            uint32_t bitsToInsert = i_numBits;
            uint32_t curLatchBit = ECMD_UNSET;          // This is the actual latch bit we are looking for next
            uint32_t curBitsToInsert = bitsToInsert;    // This is the total number of bits left to Insert
            uint32_t curStartBitToInsert = 0;           // This is the offset into the current entry for insertion
            uint32_t curStart = 0;
            uint32_t curEnd = 0;
            
            for (curLatchInfo = curEntry.entry.begin(); curLatchInfo != curEntry.entry.end(); curLatchInfo++) {
                
                /* Let's grab the ring for this latch entry */
                if (curRing != curLatchInfo->ringName) {
                    curRing = curLatchInfo->ringName;
                }
                
                if ((curLatchBit == ECMD_UNSET) ||(curLatchName != curLatchInfo->latchName.substr(0, curLatchInfo->latchName.rfind('(')))) {
                    curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchStartBit : curLatchInfo->latchEndBit;
                    curLatchName = curLatchInfo->latchName.substr(0, curLatchInfo->latchName.rfind('('));
                    bitsToInsert = i_numBits;
                    curBitsToInsert = bitsToInsert;
                    curStart = i_startBit;
                    curEnd = i_startBit + bitsToInsert - 1;
                }
                
                bool doWrite = false;
                /* Check if the bits are ordered from:to (0:10) or just (1) and we want to modify */
                if ((curLatchInfo->latchEndBit >= curLatchInfo->latchStartBit) && (curStart <= curLatchInfo->latchEndBit) && (curEnd >= curLatchInfo->latchStartBit)) {
                    if(curStart < curLatchInfo->latchStartBit) { // move counter to start of latch if lower than current latch
                        curStart = curLatchInfo->latchStartBit;
                    }
                    if (bustype == ECMD_CHIPFLAG_FSI) {
                        if(curStart > curLatchInfo->latchStartBit) { // case where we start inside the latch
                            curStartBitToInsert = curStart - curLatchInfo->latchStartBit;
                        } else {
                            curStartBitToInsert = 0;
                        }
                        bitsToInsert = ((curLatchInfo->length - curStartBitToInsert) < curBitsToInsert ? (curLatchInfo->length - curStartBitToInsert) : curBitsToInsert);
                    } else {
                        // JTAG -- not sure if this is correct for more than one bit
                        //      -- this is a modified version of the FSI high:low order code
                        //      -- with latchEndBit and latchStartbit swapped when appropriate
                        //      -- mklight 03/13/2009
                        // figure out where we should start
                        int32_t tempStartBitToInsert = curLatchInfo->latchEndBit - (curStart + curBitsToInsert - 1);
                        // if we go negative we should start at zero
                        if(tempStartBitToInsert <= 0) { // we start at start bit
                            curStartBitToInsert = 0;
                            if(curStart > curLatchInfo->latchStartBit) { // case where we start inside the latch
                                bitsToInsert = curLatchInfo->length - (curStart - curLatchInfo->latchStartBit);
                            } else { // otherwise we start at the end of the latch
                                bitsToInsert = curLatchInfo->length;
                            }
                        } else { // we start after start bit, end of latch will not be filled, this will be the last latch
                            curStartBitToInsert = (uint32_t) tempStartBitToInsert;
                            bitsToInsert = curBitsToInsert;
                        }
                    }
                    curLatchBit = curLatchInfo->latchEndBit + 1;
                    doWrite = true;
                }
                
                /* Check if the bits are ordered to:from (10:0) */
                if ((curLatchInfo->latchStartBit > curLatchInfo->latchEndBit) && (curStart <= curLatchInfo->latchStartBit) && (curEnd >= curLatchInfo->latchEndBit)) {
                    if(curStart < curLatchInfo->latchEndBit) { // move counter to start of latch if lower than current latch
                        curStart = curLatchInfo->latchEndBit;
                    }
                    if (bustype == ECMD_CHIPFLAG_FSI) {
                        // figure out where we should start
                        int32_t tempStartBitToInsert = curLatchInfo->latchStartBit - (curStart + curBitsToInsert - 1);
                        // if we go negative we should start at zero
                        if(tempStartBitToInsert <= 0) { // we start at start bit
                            curStartBitToInsert = 0;
                            if(curStart > curLatchInfo->latchEndBit) { // case where we start inside the latch
                                bitsToInsert = curLatchInfo->length - (curStart - curLatchInfo->latchEndBit);
                            } else { // otherwise we start at the end of the latch
                                bitsToInsert = curLatchInfo->length;
                            }
                        } else { // we start after start bit, end of latch will not be filled, this will be the last latch
                            curStartBitToInsert = (uint32_t) tempStartBitToInsert;
                            bitsToInsert = curBitsToInsert;
                        }
                    } else {
                        if(curStart > curLatchInfo->latchEndBit) { // case where we start inside the latch
                            curStartBitToInsert = curStart - curLatchInfo->latchEndBit;
                        } else {
                            curStartBitToInsert = 0;
                        }
                        bitsToInsert = ((curLatchInfo->length - curStartBitToInsert) < curBitsToInsert ? (curLatchInfo->length - curStartBitToInsert) : curBitsToInsert);
                    }
                    curLatchBit = curLatchInfo->latchStartBit + 1;
                    doWrite = true;
                }
                
                if (doWrite == true)
                {
                    //printf("curLatchBit = %d, startbit = %d, endbit = %d, length = %d, curStartBitToInsert = %d, bits = %d\n", curLatchBit, curLatchInfo->latchStartBit, curLatchInfo->latchEndBit, curLatchInfo->length, curStartBitToInsert,  bitsToInsert);
                    /* ********* */
                    /* FSI Order */
                    /* ********* */
                    if (bustype == ECMD_CHIPFLAG_FSI) {
                        rc = o_mask.setBit(curLatchInfo->fsiRingOffset + curStartBitToInsert, bitsToInsert);
                        if (rc) return rc;                            
                    }
                    /* ********** */
                    /* JTAG Order */
                    /* ********** */
                    else {
                        rc = o_mask.setBit(curLatchInfo->jtagRingOffset - curLatchInfo->length  + 1 + curStartBitToInsert, bitsToInsert);
                        if (rc) return rc;                            
                    }
                        
                    curBitsToInsert -= bitsToInsert;
                } else {
                    /* Nothing was there that we needed, let's try the next entry */
                    curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchEndBit + 1: curLatchInfo->latchStartBit + 1;
                }
            }
            
        } while(0);
    }

    return rc;
}

uint32_t dllCreateSparseMaskFromLatchOpt(const ecmdChipTarget & i_target, const std::list<ecmdLatchEntry> & i_scandefLatchInfo, ecmdDataBuffer & io_mask, uint32_t i_startBit, uint32_t i_numBits)
{
    uint32_t rc = ECMD_SUCCESS;
    std::list< ecmdLatchEntry >::const_iterator curLatchInfo;    ///< Iterator for walking through latches
    ecmdDataBuffer ringBuffer;                    ///< Buffer to store entire ring
    ecmdDataBuffer buffer(5000 /* bits */);        ///< Space for extracted latch data
    std::string curRing;                          ///< Current ring being operated on
    uint32_t bustype;                             ///< Type of bus we are attached to JTAG vs FSI
    std::string curLatchName;                     ///< Current latch name being operated on
    std::string printed;
    std::list<ecmdRingData> l_ring_info;

    // Both i_startBit and i_numBits have to be either ECMD_UNSET or not ECMD_UNSET
    if (((i_startBit == ECMD_UNSET) && (i_numBits != ECMD_UNSET)) || ((i_startBit != ECMD_UNSET) && (i_numBits == ECMD_UNSET)))
    {
        printed = "Both i_startBit and i_numBits have to be either ECMD_UNSET or not ECMD_UNSET\n";
        dllRegisterErrorMsg(ECMD_INVALID_ARGS, "dllCreateSparseMaskFromLatchOpt", printed.c_str() );
        return ECMD_INVALID_ARGS;
    }

    /* Let's find out if we are JTAG of FSI here */
    rc = dllGetScandefOrder(i_target, bustype);
    if (rc) {
        printed = "Problems retrieving chip information on target\n";
        dllRegisterErrorMsg(rc, "dllCreateSparseMaskFromLatchOpt", printed.c_str() );
        return rc;
    }
    /* Now make sure the plugin gave us some bus info */
    if ((bustype != ECMD_CHIPFLAG_JTAG) && (bustype != ECMD_CHIPFLAG_FSI) ) {
        dllRegisterErrorMsg(ECMD_DLL_INVALID, "dllCreateSparseMaskFromLatchOpt", "eCMD plugin returned an invalid bustype in dllGetScandefOrder\n");
        return ECMD_DLL_INVALID;
    }
  

    // We're going to use the logic copied from getLatch if these values weren't set
    if ((i_startBit == ECMD_UNSET) && (i_numBits == ECMD_UNSET))
    {
        /* Single exit point */
        while (1) {
            
            uint32_t bitsToFetch = ECMD_UNSET;       // Grab all bits
            uint32_t curLatchBit = ECMD_UNSET;       // This is the actual latch bit we are looking for next
            uint32_t curBufferBit = 0;               // Current bit to insert into buffered register
            uint32_t curBitsToFetch = bitsToFetch;   // This is the total number of bits left to fetch
            uint32_t dataStartBit = ECMD_UNSET;      // Actual start bit of buffered register
            uint32_t dataEndBit = ECMD_UNSET;        // Actual end bit of buffered register
            std::string latchname = "";
            curRing = "";

            for (curLatchInfo = i_scandefLatchInfo.begin(); (curLatchInfo != i_scandefLatchInfo.end()) && (curBitsToFetch > 0); curLatchInfo++)
            {    
                /* Let's grab the ring for this latch entry */
                if (curRing != curLatchInfo->ringName) {
                    curRing = curLatchInfo->ringName;
                    rc = dllQueryRing(i_target, l_ring_info, curRing.c_str(), ECMD_QUERY_DETAIL_LOW);
                    if (!l_ring_info.empty())
                    {
                        if (io_mask.getBitLength() != l_ring_info.begin()->bitLength)
                        {
                            rc = ECMD_DBUF_MISMATCH;
                            dllRegisterErrorMsg(rc, "dllCreateSparseMaskFromLatchOpt", "Buffer for mask isn't the size of the ring being operated on.  You must set the mask to the size of the ring before calling this function.\n");
                            return rc;  
                        }
                    }
                }
                /* Do we have previous data here , or some missing bits in the scandef latchs ?*/
                if (((dataStartBit != ECMD_UNSET) && (curLatchBit !=  curLatchInfo->latchStartBit) && (curLatchBit !=  curLatchInfo->latchEndBit)) ||
                    ((latchname == "") || (latchname != curLatchInfo->latchName.substr(0, curLatchInfo->latchName.rfind('('))))) 
                {                    
                    /* If this is a fresh one we need to reset everything */
                    if ((latchname == "") || (latchname != curLatchInfo->latchName.substr(0, latchname.length()))) {
                        dataStartBit = dataEndBit = ECMD_UNSET;
                        curBitsToFetch = ECMD_UNSET;
                        curBufferBit = 0;
                        latchname = curLatchInfo->latchName.substr(0, curLatchInfo->latchName.rfind('('));
                        curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchStartBit : curLatchInfo->latchEndBit;
                    } else {
                        /* This is the case where the scandef had holes in the register, so we will continue with this latch, but skip some bits */
                        dataStartBit = dataEndBit = ECMD_UNSET;
                        curBufferBit = 0;
                        /* Decrement the bits to fetch by the hole in the latch */
                        curBitsToFetch -= (curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchStartBit : curLatchInfo->latchEndBit) - curLatchBit;
                        curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchStartBit : curLatchInfo->latchEndBit;
                    }
                }

                /* Do we want anything in here */
                /* Check if the bits are ordered from:to (0:10) or just (1) */
                if (((curLatchInfo->latchEndBit >= curLatchInfo->latchStartBit) && (curLatchBit <=  curLatchInfo->latchEndBit)) ||
                    /* Check if the bits are ordered to:from (10:0) */
                    ((curLatchInfo->latchStartBit > curLatchInfo->latchEndBit)  && (curLatchBit <=  curLatchInfo->latchStartBit))) 
                {                    
                    bitsToFetch = (curLatchInfo->length  < curBitsToFetch) ? curLatchInfo->length : curBitsToFetch;
                    
                    /* Setup the actual data bits displayed */
                    if (dataStartBit == ECMD_UNSET) {
                        dataStartBit = curLatchBit;
                        dataEndBit = dataStartBit - 1;
                    }
                    dataEndBit += bitsToFetch;

                    /* ********* */
                    /* FSI Order */
                    /* ********* */
                    
                    if (bustype == ECMD_CHIPFLAG_FSI) {
                        rc = io_mask.setBit(curLatchInfo->fsiRingOffset, bitsToFetch); if (rc) return rc;
                        
                        /* Extract bits if ordered from:to (0:10) */
                        if (curLatchInfo->latchEndBit > curLatchInfo->latchStartBit) {
                            curLatchBit = curLatchInfo->latchEndBit + 1;
                            /* Extract if bits are ordered to:from (10:0) or just (1) */
                        } else {
                            //if (bitsToFetch > 1) buffertemp.reverse();
                            curLatchBit = curLatchInfo->latchStartBit + 1;
                        }
                        curBufferBit += bitsToFetch;
                        
                        /* ********* */
                        /*JTAG Order */
                        /* ********* */
                    } else {
                        rc = io_mask.setBit(curLatchInfo->jtagRingOffset, bitsToFetch); if (rc) return rc;
                        /* Extract bits if ordered from:to (0:10) */
                        if (curLatchInfo->latchEndBit > curLatchInfo->latchStartBit) {
                            curLatchBit = curLatchInfo->latchEndBit + 1;
                        } else {
                            /* Extract if bits are ordered to:from (10:0) or just (1) */
                            curLatchBit = curLatchInfo->latchStartBit + 1;
                        }
                        curBufferBit += bitsToFetch;
                    }                    
                    curBitsToFetch -= bitsToFetch;
                } else {
                    /* Nothing was there that we needed, let's try the next entry */
                    curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchEndBit + 1: curLatchInfo->latchStartBit + 1;
                    
                }
                
            } /* end latchinfo for loop */                                
            break;
        } /* end while (exit point) */
    }
    // Use the logic from putLatch if we had values for i_numBits and i_startBit
    else
    {
        // Stuff from putLatch
        
        /* single exit point */
        do {
            uint32_t bitsToInsert = i_numBits;
            uint32_t curLatchBit = ECMD_UNSET;          // This is the actual latch bit we are looking for next
            uint32_t curBitsToInsert = bitsToInsert;    // This is the total number of bits left to Insert
            uint32_t curStartBitToInsert = 0;           // This is the offset into the current entry for insertion
            uint32_t curStart = 0;
            uint32_t curEnd = 0;
            
            for (curLatchInfo = i_scandefLatchInfo.begin(); curLatchInfo != i_scandefLatchInfo.end(); curLatchInfo++) {
                
                /* Let's grab the ring for this latch entry */
                if (curRing != curLatchInfo->ringName) {
                    curRing = curLatchInfo->ringName;
                    rc = dllQueryRing(i_target, l_ring_info, curRing.c_str(), ECMD_QUERY_DETAIL_LOW);
                    if (!l_ring_info.empty())
                    {
                        if (io_mask.getBitLength() != l_ring_info.begin()->bitLength)
                        {
                            rc = ECMD_DBUF_MISMATCH;
                            dllRegisterErrorMsg(rc, "dllCreateSparseMaskFromLatchOpt", "Buffer for mask isn't the size of the ring being operated on.  You must set the mask to the size of the ring before calling this function.\n");
                            return rc;  
                        }
                    }
                }
                
                if ((curLatchBit == ECMD_UNSET) ||(curLatchName != curLatchInfo->latchName.substr(0, curLatchInfo->latchName.rfind('(')))) {
                    curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchStartBit : curLatchInfo->latchEndBit;
                    curLatchName = curLatchInfo->latchName.substr(0, curLatchInfo->latchName.rfind('('));
                    bitsToInsert = i_numBits;
                    curBitsToInsert = bitsToInsert;
                    curStart = i_startBit;
                    curEnd = i_startBit + bitsToInsert - 1;
                }
                
                bool doWrite = false;
                /* Check if the bits are ordered from:to (0:10) or just (1) and we want to modify */
                if ((curLatchInfo->latchEndBit >= curLatchInfo->latchStartBit) && (curStart <= curLatchInfo->latchEndBit) && (curEnd >= curLatchInfo->latchStartBit)) {
                    if(curStart < curLatchInfo->latchStartBit) { // move counter to start of latch if lower than current latch
                        curStart = curLatchInfo->latchStartBit;
                    }
                    if (bustype == ECMD_CHIPFLAG_FSI) {
                        if(curStart > curLatchInfo->latchStartBit) { // case where we start inside the latch
                            curStartBitToInsert = curStart - curLatchInfo->latchStartBit;
                        } else {
                            curStartBitToInsert = 0;
                        }
                        bitsToInsert = ((curLatchInfo->length - curStartBitToInsert) < curBitsToInsert ? (curLatchInfo->length - curStartBitToInsert) : curBitsToInsert);
                    } else {
                        // JTAG -- not sure if this is correct for more than one bit
                        //      -- this is a modified version of the FSI high:low order code
                        //      -- with latchEndBit and latchStartbit swapped when appropriate
                        //      -- mklight 03/13/2009
                        // figure out where we should start
                        int32_t tempStartBitToInsert = curLatchInfo->latchEndBit - (curStart + curBitsToInsert - 1);
                        // if we go negative we should start at zero
                        if(tempStartBitToInsert <= 0) { // we start at start bit
                            curStartBitToInsert = 0;
                            if(curStart > curLatchInfo->latchStartBit) { // case where we start inside the latch
                                bitsToInsert = curLatchInfo->length - (curStart - curLatchInfo->latchStartBit);
                            } else { // otherwise we start at the end of the latch
                                bitsToInsert = curLatchInfo->length;
                            }
                        } else { // we start after start bit, end of latch will not be filled, this will be the last latch
                            curStartBitToInsert = (uint32_t) tempStartBitToInsert;
                            bitsToInsert = curBitsToInsert;
                        }
                    }
                    curLatchBit = curLatchInfo->latchEndBit + 1;
                    doWrite = true;
                }
                
                /* Check if the bits are ordered to:from (10:0) */
                if ((curLatchInfo->latchStartBit > curLatchInfo->latchEndBit) && (curStart <= curLatchInfo->latchStartBit) && (curEnd >= curLatchInfo->latchEndBit)) {
                    if(curStart < curLatchInfo->latchEndBit) { // move counter to start of latch if lower than current latch
                        curStart = curLatchInfo->latchEndBit;
                    }
                    if (bustype == ECMD_CHIPFLAG_FSI) {
                        // figure out where we should start
                        int32_t tempStartBitToInsert = curLatchInfo->latchStartBit - (curStart + curBitsToInsert - 1);
                        // if we go negative we should start at zero
                        if(tempStartBitToInsert <= 0) { // we start at start bit
                            curStartBitToInsert = 0;
                            if(curStart > curLatchInfo->latchEndBit) { // case where we start inside the latch
                                bitsToInsert = curLatchInfo->length - (curStart - curLatchInfo->latchEndBit);
                            } else { // otherwise we start at the end of the latch
                                bitsToInsert = curLatchInfo->length;
                            }
                        } else { // we start after start bit, end of latch will not be filled, this will be the last latch
                            curStartBitToInsert = (uint32_t) tempStartBitToInsert;
                            bitsToInsert = curBitsToInsert;
                        }
                    } else {
                        if(curStart > curLatchInfo->latchEndBit) { // case where we start inside the latch
                            curStartBitToInsert = curStart - curLatchInfo->latchEndBit;
                        } else {
                            curStartBitToInsert = 0;
                        }
                        bitsToInsert = ((curLatchInfo->length - curStartBitToInsert) < curBitsToInsert ? (curLatchInfo->length - curStartBitToInsert) : curBitsToInsert);
                    }
                    curLatchBit = curLatchInfo->latchStartBit + 1;
                    doWrite = true;
                }
                
                if (doWrite == true)
                {
                    //printf("curLatchBit = %d, startbit = %d, endbit = %d, length = %d, curStartBitToInsert = %d, bits = %d\n", curLatchBit, curLatchInfo->latchStartBit, curLatchInfo->latchEndBit, curLatchInfo->length, curStartBitToInsert,  bitsToInsert);
                    /* ********* */
                    /* FSI Order */
                    /* ********* */
                    if (bustype == ECMD_CHIPFLAG_FSI) {
                        rc = io_mask.setBit(curLatchInfo->fsiRingOffset + curStartBitToInsert, bitsToInsert);
                        if (rc) return rc;                            
                    }
                    /* ********** */
                    /* JTAG Order */
                    /* ********** */
                    else {
                        rc = io_mask.setBit(curLatchInfo->jtagRingOffset - curLatchInfo->length  + 1 + curStartBitToInsert, bitsToInsert);
                        if (rc) return rc;                            
                    }
                        
                    curBitsToInsert -= bitsToInsert;
                } else {
                    /* Nothing was there that we needed, let's try the next entry */
                    curLatchBit = curLatchInfo->latchStartBit < curLatchInfo->latchEndBit ? curLatchInfo->latchEndBit + 1: curLatchInfo->latchStartBit + 1;
                }
            }
            
        } while(0);
    }

    return rc;
}

bool findLatchInCache(const std::list<ecmdFileLocation> & i_fileLocs, uint64_t i_latchHashKey64, const std::string & i_ringName, ecmdLatchBufferEntry & o_latchdata)
{
    bool foundit = false;
    std::list<ecmdLatchCacheEntry>::iterator searchCacheIter;
    std::map<uint64_t, ecmdLatchBufferEntry>::iterator searchLatchIter;
    ecmdLatchCacheEntry searchCache;

    for (std::list<ecmdFileLocation>::const_iterator l_fileLoc = i_fileLocs.begin(); l_fileLoc != i_fileLocs.end(); l_fileLoc++)
    {
        /* Level one */
        searchCacheIter = latchCache.end();

        /* Set the hashkey for the lookup and then do it */
        searchCache.scandefHashKey = ecmdHashString64(l_fileLoc->textFile.c_str(), 0);

        searchCacheIter = find(latchCache.begin(), latchCache.end(), searchCache);

        /* If we found something, keep going down */
        /* Otherwise, add it to the list */
        if (searchCacheIter != latchCache.end())
        {
            /* Level two */
            /* We found the proper scandef entry, now actually search for the latch we need */
            //lets always use the 64 bit because this will only be used for comparison
            searchLatchIter = searchCacheIter->latches.find(i_latchHashKey64);
            if (searchLatchIter != searchCacheIter->latches.end())
            {
                /* If the user passed NULL we have to search entire scandef */
                if (i_ringName == "")
                {
                    if (searchLatchIter->second.ringName.length() == 0)
                    {
                        o_latchdata = (searchLatchIter->second);
                        foundit = true;
                        break;
                   }
                }
                else if (searchLatchIter->second.ringName == i_ringName)
                {
                    o_latchdata = (searchLatchIter->second);
                    foundit = true;
                    break;
                }
            }
        }
        // Add the scandef entry to the map
        else
        {
            latchCache.push_front(searchCache);
        }
    } // l_fileLocs loop

    return foundit;
}

/**
   @brief Parse the scandef for the latchname provided and load into latchBuffer for later retrieval
   @param target Chip target to operate on
   @param i_ringName Optional ring name, ignored if == NULL
   @param i_latchName latch name to search for in scandef
   @param i_mode Mode to search with, full or partial name matchs
   @param o_latchdata Return latch data read from scandef
*/
uint32_t readScandef(const ecmdChipTarget & target, const char* i_ringName, const char* i_latchName, ecmdLatchMode_t i_mode, ecmdLatchBufferEntry & o_latchdata)
{
    uint32_t rc = ECMD_SUCCESS;
    std::list<ecmdLatchEntry>::iterator entryIt;
    std::list<ecmdLatchEntry>::iterator entryIt1;
    std::string scandefFile;                      ///< Full path to scandef file
    std::list<ecmdFileLocation> l_fileLocs;       ///< List of scandef and scandefhash files
    std::string latchName = i_latchName;          ///< Store our latchname in a stl string
    std::string ringName = ((i_ringName == NULL) ? "" : i_ringName);            ///< Ring that caller specified
    std::string curRing;                          ///< Current ring being read in
    uint64_t latchHashKey64;                        ///< Hash Key for i_latchName
    std::list<ecmdLatchCacheEntry>::iterator searchCacheIter;
    bool foundRing = false;

    //used to set ecmdLatchBufferEntry's latchNameHashKey in * and that is only used in comparsion
    latchHashKey64 = ecmdHashString64(latchName.c_str(), 0);

    /* single exit point */
    std::string l_version = "default";
    /* Let's see if we have already looked up this info */
    /* find scandef file */
    rc = dllQueryFileLocation(target, ECMD_FILE_SCANDEF, l_fileLocs, l_version);
    if (rc) {
      dllRegisterErrorMsg(rc, "readScandef", "Error occured locating scandef file.\n");
      return rc;
    }

    LockGuard lg(&latchCacheMutex);
    if (findLatchInCache(l_fileLocs, latchHashKey64, ringName, o_latchdata))
    {
        /* We're done, get out of here */
        return rc;
    }

    /* We don't have it already, let's go looking */
    for (std::list<ecmdFileLocation>::const_iterator l_fileLoc = l_fileLocs.begin(); l_fileLoc != l_fileLocs.end(); l_fileLoc++)
    {
        /* find scandef file */
        scandefFile = l_fileLoc->textFile;

        std::ifstream ins(scandefFile.c_str());
        if (ins.fail())
        {
            rc = ECMD_UNABLE_TO_OPEN_SCANDEF;
            dllRegisterErrorMsg(rc, "readScandef", ("Error occured opening scandef file: " + scandefFile + "\n").c_str());
            break;
        }

        //let's go hunting in the scandef for this register (pattern)
        ecmdLatchEntry curLatch;

        std::string curLine;
        std::vector<std::string> curArgs(4);

        std::string temp;

        bool done = false;
        size_t  leftParen;
        size_t  colon;
        foundRing = false;

        while (getline(ins, curLine) && !done)
        {
            if (foundRing)
            {
                if (curLine[0] == 'E' && curLine.find("END") != std::string::npos)
                {
                    if (ringName != "") done = true;
                    continue;
                }
                else if ((ringName == "") && ((curLine[0] == 'N') && (curLine.substr(0,4) == "Name")))
                {
                    ecmdParseTokens(curLine, " \t\n=", curArgs);
                    if (curArgs.size() != 2)
                    {
                        rc = ECMD_SCANDEF_LOOKUP_FAILURE;
                        dllRegisterErrorMsg(rc, "readScandef", ("Parse failure reading ring name from line : '" + curLine + "'\n").c_str());
                        break;
                    }
                    transform(curArgs[1].begin(), curArgs[1].end(), curArgs[1].begin(), (int(*)(int)) tolower);
                    /* Get just the ringname */
                    curRing = curArgs[1];
                    continue;
                }
                else if (curLine.length() == 0 || curLine[0] == '\0' || curLine[0] == '*' || curLine[0] == '#')
                {
                    //do nothing
                    continue;
                }
                else if ((curLine[0] != ' ') && (curLine[0] != '\t'))
                {
                    // do nothing
                    continue;
                }
                else if (!(i_mode == ECMD_LATCHMODE_FULL) && (curLine.find(latchName) != std::string::npos))
                {
                    ecmdParseTokens(curLine, " \t\n", curArgs);
                    curLatch.length = (uint32_t)atoi(curArgs[0].c_str());
                    curLatch.fsiRingOffset = (uint32_t)atoi(curArgs[1].c_str());
                    curLatch.jtagRingOffset = (uint32_t)atoi(curArgs[2].c_str());
                    curLatch.latchName = curArgs[4];
                }
                else if (i_mode == ECMD_LATCHMODE_FULL)
                {
                    ecmdParseTokens(curLine, " \t\n", curArgs);
                    if ((curArgs.size() >= 5) && latchName == curArgs[4].substr(0,curArgs[4].find_last_of("(")))
                    {
                        curLatch.length = (uint32_t)atoi(curArgs[0].c_str());
                        curLatch.fsiRingOffset = (uint32_t)atoi(curArgs[1].c_str());
                        curLatch.jtagRingOffset = (uint32_t)atoi(curArgs[2].c_str());
                        curLatch.latchName = curArgs[4];
                    }
                    else
                    {
                        continue;
                    }
                }
                else
                {
                    /* Not one we want */
                    continue;
                }

                /* Let's parse out the start/end bit if they exist */
                leftParen = curLatch.latchName.rfind('(');
                if (leftParen == std::string::npos)
                {
                    /* This latch doesn't have any parens */
                    curLatch.latchStartBit = curLatch.latchEndBit = 0;
                    curLatch.latchType = ECMD_LATCHTYPE_NOBIT;
                }
                else
                {
                    temp = curLatch.latchName.substr(leftParen+1, curLatch.latchName.length() - leftParen - 1);
                    curLatch.latchStartBit = (uint32_t)atoi(temp.c_str());

                    /* Is this a multibit or single bit */
                    if ((colon = temp.find(':')) != std::string::npos)
                    {
                        curLatch.latchEndBit = (uint32_t)atoi(temp.substr(colon+1, temp.length()).c_str());
                        curLatch.latchType = ECMD_LATCHTYPE_MULTIBIT;
                    }
                    else if ((colon = temp.find(',')) != std::string::npos)
                    {
                        dllOutputError("readScandef - Arrays not currently supported with getlatch\n");
                        return ECMD_FUNCTION_NOT_SUPPORTED;
                    }
                    else
                    {
                        curLatch.latchEndBit = curLatch.latchStartBit;
                        curLatch.latchType = ECMD_LATCHTYPE_SINGLEBIT;
                    }
                }
                curLatch.ringName = curRing;
                o_latchdata.entry.push_back(curLatch);
            }
            /* The user specified a ring for use to look in */
            else if ((ringName != "") && ((curLine[0] == 'N') && (curLine.find("Name") != std::string::npos)))
            {
                ecmdParseTokens(curLine, " \t\n=", curArgs);
                /* Push the ring name to lower case */
                transform(curArgs[1].begin(), curArgs[1].end(), curArgs[1].begin(), (int(*)(int)) tolower);
                if ((curArgs.size() >= 2) && curArgs[1] == ringName)
                {
                    foundRing = true;
                    curRing = ringName;
                }
            }
            /* The user didn't specify a ring for us, we will search them all */
            else if ((ringName == "") && ((curLine[0] == 'N') && (curLine.substr(0,4) == "Name")))
            {
                ecmdParseTokens(curLine, " \t\n=", curArgs);
                if (curArgs.size() != 2)
                {
                    rc = ECMD_SCANDEF_LOOKUP_FAILURE;
                    dllRegisterErrorMsg(rc, "readScandef", ("Parse failure reading ring name from line : '" + curLine + "'\n").c_str());
                    break;
                }
                transform(curArgs[1].begin(), curArgs[1].end(), curArgs[1].begin(), (int(*)(int)) tolower);
                /* Get just the ringname */
                curRing = curArgs[1];
                foundRing = true;
            }
        }

        ins.close();

        //Check if latch present in multiple rings
        if ((i_mode != ECMD_LATCHMODE_PARTIAL_NO_RING_CHECK) && (ringName == ""))
        {
            std::string outerRing;
            std::string outerLatch;

            for (entryIt = o_latchdata.entry.begin(); entryIt != o_latchdata.entry.end(); entryIt++)
            {
                if (entryIt != o_latchdata.entry.begin())
                {
                    for (entryIt1 = o_latchdata.entry.begin(); entryIt1 != o_latchdata.entry.end(); entryIt1++)
                    {
                        if ( (entryIt1->latchName.substr(0,entryIt1->latchName.find_last_of("(")) == outerLatch) && (entryIt1->ringName != outerRing) )
                        {
                            rc = ECMD_SCANDEFHASH_MULT_RINGS;
                            dllRegisterErrorMsg(rc, "readScandef", ("Same latchname : '" + latchName + "' found in multiple rings in the scandef\nPlease specify a ringname\n").c_str());
                            return rc;
                        }
                    }
                }
                outerRing = entryIt->ringName;
                outerLatch = entryIt->latchName.substr(0,entryIt->latchName.find_last_of("("));
            }
        }

        if (ringName != "")
        {
            o_latchdata.ringName = ringName;
        }
        else
        {
            o_latchdata.ringName = "";
        }
        o_latchdata.latchName = latchName;
        o_latchdata.latchNameHashKey = latchHashKey64;
        o_latchdata.entry.sort();

        // We found the ring already, so bail 
        // Assumption is that each ring only shows up in a single scandef
        if ((!o_latchdata.entry.empty()) && (ringName != ""))
        {
            break; 
        }

    } /* end single exit point */

    if (!foundRing)
    {
        std::string tmp = ringName;
        rc = ECMD_INVALID_RING;
        dllRegisterErrorMsg(rc, "readScandef", ("Could not find ring name " + tmp + "\n").c_str());
        //break;
    }
    else if (o_latchdata.entry.empty())
    {
        rc = ECMD_INVALID_LATCHNAME;
        dllRegisterErrorMsg(rc, "readScandef", ("no registers found that matched " + latchName + "\n").c_str());
        //break;
    }
    // Add to cache in proper order
    else
    {
        ecmdLatchCacheEntry searchCache;
        searchCache.scandefHashKey = ecmdHashString64(scandefFile.c_str(), 0);
        searchCacheIter = find(latchCache.begin(), latchCache.end(), searchCache);
        if (searchCacheIter != latchCache.end())
        {
            searchCacheIter->latches[latchHashKey64] = o_latchdata;
        }
    }

    return rc;
}

/**
   @brief Lookup the scandef hashfile for the latchname and ring offset and then seek to the offset in the scandef for the latchname provided and load into latchBuffer for later retrieval
   @param target Chip target to operate on
   @param i_ringName Optional ring name, ignored if == NULL
   @param i_latchName latch name to search for in scandef
   @param o_latchdata Return latch data read from scandef
*/
template <typename T>
inline T t_htonl(T i_num);

template <>
inline uint64_t t_htonl(uint64_t i_num) { return htonll(i_num); }

template <>
inline uint32_t t_htonl(uint32_t i_num) { return htonl(i_num); }

template <typename T>
bool findLatch(T i_latchKey, uint32_t i_numRings, std::ifstream & i_hashFile, std::list< ecmdLatchHashInfo > & o_latchHashDet)
{
    T l_curKey = 0;
    uint32_t l_curOffset = 0;
    ecmdLatchHashInfo l_curLatchHashInfo;
    const std::streamsize HASH_SIZE = sizeof(T);
    const std::streamsize LOCATION_SIZE = sizeof(uint32_t);
    const std::streamsize STRUCT_SIZE = LOCATION_SIZE + HASH_SIZE;
    const std::streamsize READBACK_SIZE = STRUCT_SIZE + HASH_SIZE;
    const std::streamsize HEADER_SIZE = 2 * sizeof(uint32_t);
    const std::streamsize FOOTER_SIZE = 2 * sizeof(uint32_t);
    bool l_foundLatch = false;

    //Get Offset of the end of file
    i_hashFile.seekg (0, std::ios::end);
    std::streamoff l_end = (std::streamoff)i_hashFile.tellg();

    //Each ring is repeated twice - One for BEGIN offset and One for END
    std::streamoff l_low = i_numRings * 2;
    std::streamoff l_mid = 0;
    //end pos - 8 for the #rings, -8 for the random end)
    std::streamoff l_high = (l_end - HEADER_SIZE - FOOTER_SIZE) / STRUCT_SIZE;

    while (l_low <= l_high)
    {
        l_mid = (l_low + l_high) / 2;
        //each ring section is STRUCT_SIZE, need to account for the HEADER_SIZE
        i_hashFile.seekg((l_mid * STRUCT_SIZE) + HEADER_SIZE);
        i_hashFile.read((char *)& l_curKey, HASH_SIZE);
        l_curKey = t_htonl(l_curKey);

        if (i_latchKey == l_curKey)
        {
            //Goto the first occurence of this latch
            while(i_latchKey == l_curKey)
            {
                // go back READBACK_SIZE, HASH_SIZE for the hash we just read,
                // and for the entire previous latch
                i_hashFile.seekg(-READBACK_SIZE, std::ios::cur);
                i_hashFile.read((char *)& l_curKey, HASH_SIZE);
                l_curKey = t_htonl(l_curKey);
            }

            //Go back to the matching latch
            // don't adjust this because this is the size of the location
            i_hashFile.seekg(LOCATION_SIZE, std::ios::cur);
            i_hashFile.read((char *)& l_curKey, HASH_SIZE);
            l_curKey = t_htonl(l_curKey);
            while(i_latchKey == l_curKey)
            {
                i_hashFile.read((char *)& l_curOffset, LOCATION_SIZE);
                l_curLatchHashInfo.latchOffset = htonl(l_curOffset);
                l_curLatchHashInfo.ringFound = false;
                o_latchHashDet.push_back(l_curLatchHashInfo);
                l_foundLatch = true;
                //Read next latch
                i_hashFile.read((char *)& l_curKey, HASH_SIZE);
                l_curKey = t_htonl(l_curKey);
            }
            break;
        }
        if (i_latchKey < l_curKey)
        {
            l_high = l_mid - 1;
        }
        else
        {
            l_low = l_mid + 1;
        }
    }
    return l_foundLatch;
}

template <typename T>
bool findRing(uint32_t i_offset, uint32_t i_numRings, std::ifstream & i_hashFile, uint32_t & o_beginOffset, uint32_t & o_endOffset, T & o_ringKey)
{
    const std::streamsize HASH_SIZE = sizeof(T);
    const std::streamsize LOCATION_SIZE = sizeof(uint32_t);
    const std::streamsize STRUCT_SIZE = LOCATION_SIZE + HASH_SIZE;
    const std::streamsize HEADER_SIZE = 2 * sizeof(uint32_t);

    T l_ringKey = 0;
    uint32_t l_beginOffset = 0;
    uint32_t l_endOffset = 0;
    bool l_foundRing = false;

    std::streamoff l_end = (i_numRings * STRUCT_SIZE * 2) + HEADER_SIZE;
    while ((std::streamoff)i_hashFile.tellg() != l_end)
    {
        i_hashFile.read((char *)& l_ringKey, HASH_SIZE); //Read the ringKey
        i_hashFile.read((char *)& l_beginOffset, LOCATION_SIZE); //Read the begin offset
        i_hashFile.read((char *)& l_ringKey, HASH_SIZE); //Read the ringKey
        i_hashFile.read((char *)& l_endOffset, LOCATION_SIZE); //Read the end offset

        l_ringKey = t_htonl(l_ringKey);
        l_beginOffset = htonl(l_beginOffset);
        l_endOffset = htonl(l_endOffset);

        if ((i_offset > l_beginOffset) && (i_offset < l_endOffset))
        {
            o_beginOffset = l_beginOffset;
            o_endOffset = l_endOffset;
            o_ringKey = l_ringKey;
            l_foundRing = true;
            break;
        }
    }
    return l_foundRing;
}

template <typename T>
bool findRing(T i_ringKey, uint32_t i_numRings, std::ifstream & i_hashFile, std::list< ecmdLatchHashInfo > & o_latchHashDet)
{
    ecmdLatchHashInfo l_curLatchHashInfo;
    const std::streamsize HASH_SIZE = sizeof(T);
    const std::streamsize LOCATION_SIZE = sizeof(uint32_t);
    const std::streamsize STRUCT_SIZE = LOCATION_SIZE + HASH_SIZE;
    const std::streamsize HEADER_SIZE = 2 * sizeof(uint32_t);

    T l_ringKey = 0;
    uint32_t l_beginOffset = 0;
    uint32_t l_endOffset = 0;
    bool l_foundRing = false;

    std::streamoff l_end = (i_numRings * STRUCT_SIZE * 2) + HEADER_SIZE;
    while ((std::streamoff)i_hashFile.tellg() != l_end)
    {
        i_hashFile.read((char *)& l_ringKey, HASH_SIZE); //Read the ringKey
        i_hashFile.read((char *)& l_beginOffset, LOCATION_SIZE); //Read the begin offset
        i_hashFile.read((char *)& l_ringKey, HASH_SIZE); //Read the ringKey
        i_hashFile.read((char *)& l_endOffset, LOCATION_SIZE); //Read the end offset

        l_ringKey = t_htonl(l_ringKey);
        l_beginOffset = htonl(l_beginOffset);
        l_endOffset = htonl(l_endOffset);

        if (i_ringKey == l_ringKey)
        {
            l_curLatchHashInfo.ringBeginOffset = l_beginOffset;
            l_curLatchHashInfo.ringEndOffset = l_endOffset;
            l_curLatchHashInfo.ringFound = true;
            o_latchHashDet.push_back(l_curLatchHashInfo);
            l_foundRing = true;
            break;
        }
    }
    return l_foundRing;
}

uint32_t readScandefHash(const ecmdChipTarget & target, const char* i_ringName, const char*  i_latchName, ecmdLatchBufferEntry & o_latchdata)
{
    uint32_t rc = ECMD_SUCCESS;
    std::list<ecmdLatchBufferEntry>::iterator bufferit;
    std::list< ecmdLatchHashInfo > latchHashDet; ///< List of the LatchHash Keys and their offsets
    std::list< ecmdLatchHashInfo >::iterator latchHashDetIter;
    std::string scandefFile;                      ///< Full path to scandef file
    std::string scandefHashFile;                  ///< Full path to scandefhash file
    std::list<ecmdFileLocation> l_fileLocs;       ///< List of scandef and scandefhash files
    std::string latchName = i_latchName;          ///< Store our latchname in a stl string
    std::string ringName = ((i_ringName == NULL) ? "" : i_ringName);            ///< Ring that caller specified
    uint32_t latchHashKey32;                      ///< Hash Key for i_latchName
    uint32_t ringHashKey32;                       ///< Hash Key for i_ringName
    uint64_t latchHashKey64;                      ///< Hash Key for i_latchName
    uint64_t fullLatchHashKey64;                  ///< Hash Key for i_latchName without any modification - used for caching
    uint64_t ringHashKey64;                       ///< Hash Key for i_ringName
    std::string curRing;                          ///< Current ring being read in
    std::string curLine;                          ///< Current line in the scandef
    std::vector<std::string> curArgs;             ///< for tokenizing
    std::list<ecmdLatchCacheEntry>::iterator searchCacheIter;
    std::string l_version = "default";
    bool ringFound = false;
    bool foundLatch = false;

    // Base hash key off of base latch name, particularly for array entries.  Remove array index.
    latchHashKey32 = ecmdHashString32((latchName.substr(0,latchName.find_last_of("("))).c_str(), 0);
    latchHashKey64 = ecmdHashString64((latchName.substr(0,latchName.find_last_of("("))).c_str(), 0);
    fullLatchHashKey64 = ecmdHashString64(latchName.c_str(), 0);

    /* Transform to lower case for case-insensitive comparisons */
    transform(ringName.begin(), ringName.end(), ringName.begin(), (int(*)(int)) tolower);
    ringHashKey32 = ecmdHashString32(ringName.c_str(), 0);
    ringHashKey64 = ecmdHashString64(ringName.c_str(), 0);

    /* Let's see if we have already looked up this info */
    /* find scandef file */
    rc = dllQueryFileLocation(target, ECMD_FILE_SCANDEF, l_fileLocs, l_version);
    if (rc)
    {
        dllRegisterErrorMsg(rc, "readScandefHash", "Error occured locating scandef file.\n");
        return rc;
    }

    LockGuard lg(&latchCacheMutex);
    if (findLatchInCache(l_fileLocs, fullLatchHashKey64, ringName, o_latchdata))
    {
        /* We're done, get out of here */
        return rc;
    }

    for (std::list<ecmdFileLocation>::const_iterator l_fileLoc = l_fileLocs.begin(); l_fileLoc != l_fileLocs.end(); l_fileLoc++)
    {
        /* find scandef hash file */
        scandefHashFile = l_fileLoc->hashFile;

        std::ifstream insh;
        bool l_scandefHash64 = false;
        insh.open(scandefHashFile.c_str());
        if (insh.fail())
        {
            insh.close();
            rc = ECMD_UNABLE_TO_OPEN_SCANDEFHASH;
            dllRegisterErrorMsg(rc, "readScandefHash", ("Error occured opening scandef hash file: " + scandefHashFile + "\n").c_str());
            break;
        }

        if ( scandefHashFile.rfind("hash.64") != std::string::npos )
        {
            l_scandefHash64 = true;
        }
        else
        {
            l_scandefHash64 = false;
        }

        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        uint32_t curRingKey32;
        uint64_t curRingKey64;
        uint32_t ringBeginOffset, ringEndOffset;
        ringFound = false;
        foundLatch = false;
        uint32_t numRings = 0;
        insh.read((char *)& numRings, 4);
        numRings = htonl(numRings);

        //Can come from querylatch
        if (!((ringName != "") && (latchName.size() == 0)))
        {
            if (l_scandefHash64)
            {
                foundLatch = findLatch(latchHashKey64, numRings, insh, latchHashDet);
            }
            else
            {
                foundLatch = findLatch(latchHashKey32, numRings, insh, latchHashDet);
            }

            if (!foundLatch)
            {
                // look in next hash file
                insh.close();
                continue;
            }

            //Seek to the ring area in the hashfile
            insh.seekg ( 8 );

            std::list< ecmdLatchHashInfo >::iterator latchIter;

            scandefFile = l_fileLoc->textFile;
            std::ifstream ins(scandefFile.c_str());
            if (ins.fail())
            {
                rc = ECMD_UNABLE_TO_OPEN_SCANDEF;
                dllRegisterErrorMsg(rc, "readScandefHash", ("Error occured opening scandef file: " + scandefFile + "\n").c_str());
                break;
            }

            //Go back to the ring area and find out the ring for the latchname
            //Error out if latch is found in multiple rings
            for (latchHashDetIter = latchHashDet.begin(); latchHashDetIter != latchHashDet.end(); latchHashDetIter++)
            {
                //Check if the latchoffset is pointing to the user latch
                ins.seekg(latchHashDetIter->latchOffset);
                getline(ins, curLine);

                ecmdParseTokens(curLine, " \t\n", curArgs);
                if(curArgs.size() != 5)
                {
                    rc = ECMD_SCANDEF_LOOKUP_FAILURE;
                    dllRegisterErrorMsg(rc, "readScandefHash", ("Latch Offset pointer incorrect. Points to : '" + curLine + "'\n").c_str());
                    return rc;
                }
                //Latch Offset is not pointing to the latch we are looking for-skip this latch
                if (latchName != curArgs[4].substr(0,curArgs[4].find_last_of("(")))
                {
                    continue;
                }

                //Flag an error if the other latches dont fall into the same ring
                if(ringFound)
                {
                    // FIXME this could be an issue since latches could be in different files
                    //make sure the cur latch has offset falling in the right ring boundaries
                    if((latchHashDetIter->latchOffset > latchIter->ringBeginOffset) &&
                       (latchHashDetIter->latchOffset < latchIter->ringEndOffset))
                    {
                        latchHashDetIter->ringBeginOffset = latchIter->ringBeginOffset;
                        latchHashDetIter->ringEndOffset = latchIter->ringEndOffset;
                        latchHashDetIter->ringFound = true;
                        continue;
                    }
                    else
                    {
                        if (ringName != "")
                        {
                            //flag it since latch doesnt fall in the ring bdy, will be removed later
                            latchHashDetIter->ringFound = false;
                        }
                        else
                        {
                            rc = ECMD_SCANDEFHASH_MULT_RINGS;
                            dllRegisterErrorMsg(rc, "readScandefHash", ("Same latchname : '" +
                                latchName + "' found in multiple rings in the scandefhash\nPlease specify a ringname\n").c_str());
                            return rc;
                        }
                    }
                }
                else // !ringFound
                {

                    if (l_scandefHash64)
                    {
                        if (findRing(latchHashDetIter->latchOffset, numRings, insh, ringBeginOffset, ringEndOffset, curRingKey64))
                        {
                            if ((ringName == "") || (ringHashKey64 == curRingKey64))
                            {
                                ringFound = true;
                            }
                        }
                    }
                    else
                    {
                        if (findRing(latchHashDetIter->latchOffset, numRings, insh, ringBeginOffset, ringEndOffset, curRingKey32))
                        {
                            if ((ringName == "") || (ringHashKey32 == curRingKey32))
                            {
                                ringFound = true;
                            }
                        }
                    }

                    if (ringFound)
                    {
                        latchHashDetIter->ringBeginOffset = ringBeginOffset;
                        latchHashDetIter->ringEndOffset = ringEndOffset;
                        latchHashDetIter->ringFound = true;
                        latchIter = latchHashDetIter;
                    }
                    else
                    {
                        latchHashDetIter->ringFound = false;
                    }
                    //Seek to the ring area in the hashfile
                    //Need to set this back to the start in case it wasn't found the first time through.
                    insh.seekg ( 8 );
                }
            }
            ins.close();
        }
        else //ringname != NULL and latchname == NULL, Skip Latch Lookup
        {
            //Seek to the ring area in the hashfile
            insh.seekg ( 8 );
            if(l_scandefHash64)
            {
                ringFound = findRing(ringHashKey64, numRings, insh, latchHashDet);
            }
            else
            {
                ringFound = findRing(ringHashKey32, numRings, insh, latchHashDet);
            }
        }

        insh.close();

        if (!ringFound)
        {
            // look in next hash file
            continue;
        }

        //Get Rid of latches for which ring match was not found
        latchHashDetIter = latchHashDet.begin();
        std::list< ecmdLatchHashInfo >::iterator tmpIt; // save the pos before removing it

        while (latchHashDetIter != latchHashDet.end())
        {
            if (latchHashDetIter->ringFound == true)
            {
                latchHashDetIter++;
            }
            else
            {
                latchHashDetIter = latchHashDet.erase(latchHashDetIter);
            }
        }

        /**********Scandef World after this *****************/
        std::ifstream ins(scandefFile.c_str());
        if (ins.fail())
        {
            rc = ECMD_UNABLE_TO_OPEN_SCANDEF;
            dllRegisterErrorMsg(rc, "readScandefHash", ("Error occured opening scandef file: " + scandefFile + "\n").c_str());
            break;
        }

        //let's go hunting in the scandef for this register (pattern)
        ecmdLatchEntry curLatch;
        std::string temp;

        size_t  leftParen;
        size_t  colon;

        //Get the Ring Begin Offset and seek till there
        latchHashDetIter = latchHashDet.begin();
        ins.seekg(latchHashDetIter->ringBeginOffset);

        bool foundRing = false;

        while (getline(ins, curLine) && !foundRing)
        {
            /* The user specified a ring for use to look in */
            if ((ringName != "") &&
                ((curLine[0] == 'N') &&
                 (curLine.find("Name") != std::string::npos)))
            {
                ecmdParseTokens(curLine, " \t\n=", curArgs);
                /* Push the ring name to lower case */
                transform(curArgs[1].begin(), curArgs[1].end(), curArgs[1].begin(), (int(*)(int)) tolower);
                if ((curArgs.size() >= 2) && curArgs[1] == ringName)
                {
                    foundRing = true;
                    curRing = ringName;
                }
            }
            /* The user didn't specify a ring for us, we will search them all */
            else if ((ringName == "") &&
                     ((curLine[0] == 'N') &&
                      (curLine.substr(0,4) == "Name")))
            {
                ecmdParseTokens(curLine, " \t\n=", curArgs);
                if (curArgs.size() != 2)
                {
                    rc = ECMD_SCANDEF_LOOKUP_FAILURE;
                    dllRegisterErrorMsg(rc, "readScandefHash", ("Parse failure reading ring name from line : '" + curLine + "'\n").c_str());
                    break;
                }
                transform(curArgs[1].begin(), curArgs[1].end(), curArgs[1].begin(), (int(*)(int)) tolower);
                /* Get just the ringname */
                curRing = curArgs[1];
                foundRing = true;
            }
        }

        if (foundRing)
        {
            if (latchName.size() != 0)
            {
                for (latchHashDetIter = latchHashDet.begin(); latchHashDetIter != latchHashDet.end(); latchHashDetIter++)
                {
                    ins.seekg(latchHashDetIter->latchOffset);

                    getline(ins, curLine);

                    ecmdParseTokens(curLine, " \t\n", curArgs);
                    if(curArgs.size() != 5)
                    {
                        rc = ECMD_SCANDEF_LOOKUP_FAILURE;
                        dllRegisterErrorMsg(rc, "readScandefHash", ("Latch Offset pointer incorrect. Points to : '" + curLine + "'\n").c_str());
                        break;
                    }

                    if (latchName == curArgs[4].substr(0,curArgs[4].find_last_of("(")))
                    {
                        curLatch.length = (uint32_t)atoi(curArgs[0].c_str());
                        curLatch.fsiRingOffset = (uint32_t)atoi(curArgs[1].c_str());
                        curLatch.jtagRingOffset = (uint32_t)atoi(curArgs[2].c_str());
                        curLatch.latchName = curArgs[4];
                    }
                    else
                    {
                        continue;//Error here??
                    }

                    /* Let's parse out the start/end bit if they exist */
                    leftParen = curLatch.latchName.rfind('(');
                    if (leftParen == std::string::npos)
                    {
                        /* This latch doesn't have any parens */
                        curLatch.latchStartBit = curLatch.latchEndBit = 0;
                        curLatch.latchType = ECMD_LATCHTYPE_NOBIT;
                    }
                    else
                    {
                        temp = curLatch.latchName.substr(leftParen+1, curLatch.latchName.length() - leftParen - 1);
                        curLatch.latchStartBit = (uint32_t)atoi(temp.c_str());
                        /* Is this a multibit or single bit */
                        if ((colon = temp.find(':')) != std::string::npos)
                        {
                            curLatch.latchEndBit = (uint32_t)atoi(temp.substr(colon+1, temp.length()).c_str());
                            curLatch.latchType = ECMD_LATCHTYPE_MULTIBIT;
                        }
                        else if ((colon = temp.find(',')) != std::string::npos)
                        {
                            dllOutputError("readScandefHash - Arrays not currently supported with getlatch\n");
                            return ECMD_FUNCTION_NOT_SUPPORTED;
                        }
                        else
                        {
                            curLatch.latchEndBit = curLatch.latchStartBit;
                            curLatch.latchType = ECMD_LATCHTYPE_SINGLEBIT;
                        }
                    }
                    curLatch.ringName = curRing;
                    o_latchdata.entry.push_back(curLatch);
                }
            }
            //latchname = null for QueryLatch
            else
            {
                while (getline(ins, curLine) && (ins.tellg() < (int) latchHashDetIter->ringEndOffset))
                {
                    ecmdParseTokens(curLine, " \t\n", curArgs);
                    if(curArgs.size() != 5)
                    {
                        continue;
                    }
                    // only the following info is needed for querylatch purposes
                    curLatch.length = (uint32_t)atoi(curArgs[0].c_str());
                    curLatch.fsiRingOffset = (uint32_t)atoi(curArgs[1].c_str());
                    curLatch.jtagRingOffset = (uint32_t)atoi(curArgs[2].c_str());
                    curLatch.latchName = curArgs[4];

                    curLatch.ringName = curRing;
                    o_latchdata.entry.push_back(curLatch);

                }
            }
        }
        ins.close();

        if (!foundRing)
        {
            std::string tmp = ringName;
            rc = ECMD_INVALID_RING;
            dllRegisterErrorMsg(rc, "readScandefHash", ("Could not find ring name " + tmp + "\n").c_str());
            break;
        }

        if (o_latchdata.entry.empty())
        {
            rc = ECMD_INVALID_LATCHNAME;
            dllRegisterErrorMsg(rc, "readScandefHash", ("no registers found that matched " + latchName + "\n").c_str());
            break;
        }

        if (ringName != "")
        {
            o_latchdata.ringName = ringName;
        }
        else
        {
            o_latchdata.ringName = "";
        }
        o_latchdata.latchName = latchName;
        o_latchdata.latchNameHashKey = fullLatchHashKey64;
        o_latchdata.entry.sort();

        // We found the ring already, so bail
        if (foundRing)
        {
            break; 
        }
    } // l_fileLocs loop

    if (rc)
    {
        // let rc return
    }
    else if (!foundLatch)
    {
        rc = ECMD_INVALID_LATCHNAME;
        dllRegisterErrorMsg(rc, "readScandefHash", ("Could not find a Latchname '" + latchName + "'  Key Match in the scandef hash.\n").c_str());
    }
    else if (!ringFound)
    {
        rc = ECMD_INVALID_RING;
        dllRegisterErrorMsg(rc, "readScandefHash", ("Could not find a ring key match for latch '" + latchName + "'\n").c_str());
    }
    else
    {
        // Update the cache
        ecmdLatchCacheEntry searchCache;
        searchCache.scandefHashKey = ecmdHashString64(scandefFile.c_str(), 0);
        searchCacheIter = find(latchCache.begin(), latchCache.end(), searchCache);
        if (searchCacheIter != latchCache.end())
        {
            searchCacheIter->latches[fullLatchHashKey64] = o_latchdata;
        }
    }
    return rc;
}
#endif // ECMD_REMOVE_LATCH_FUNCTIONS

std::string dllParseReturnCode(uint32_t i_returnCode) {
  std::string ret = "";

  ecmdChipTarget dummy;
  std::list<ecmdFileLocation> paths;
  std::string filePath;
  std::string l_version = "default";
  uint32_t rc = dllQueryFileLocation(dummy, ECMD_FILE_HELPTEXT, paths, l_version); 

  if (rc || paths.empty() || (paths.begin()->textFile.length() == 0)) {
    ret = "ERROR FINDING DECODE FILE";
    return ret;
  }

  // Assume for now we only have one helptext path returned
  filePath = paths.begin()->textFile + "ecmdReturnCodes.H";

  std::string line;
  std::vector< std::string > tokens;
  std::string source, retdefine;
  int found = 0;
  uint32_t comprc;


  std::ifstream ins(filePath.c_str());

  if (ins.fail()) {
    ret = "ERROR OPENING DECODE FILE";
    return ret;
  }

  /* This is what I am trying to parse from ecmdReturnCodes.H */

  /* #define ECMD_ERR_UNKNOWN                        0x00000000 ///< This error code wasn't flagged to which plugin it came from        */
  /* #define ECMD_ERR_ECMD                           0x01000000 ///< Error came from eCMD                                               */
  /* #define ECMD_ERR_CRONUS                         0x02000000 ///< Error came from Cronus                                             */
  /* #define ECMD_ERR_IP                             0x04000000 ///< Error came from IP GFW                                             */
  /* #define ECMD_ERR_Z                              0x08000000 ///< Error came from Z GFW                                              */
  /* #define ECMD_INVALID_DLL_VERSION                (ECMD_ERR_ECMD | 0x1000) ///< Dll Version                                          */


  while (!ins.eof()) { /*  && (strlen(str) != 0) */
    getline(ins,line,'\n');
    /* Let's strip off any comments */
    line = line.substr(0, line.find_first_of("/"));
    ecmdParseTokens(line, " \n()|", tokens);

    /* Didn't find anything */
    if (line.size() < 2) continue;

    if (tokens[0] == "#define") {
      /* Let's see if we have one of they return code source defines */
      if ((tokens.size() == 3) && (tokens[1] != "ECMD_SUCCESS") && (tokens[1] != "ECMD_DBUF_SUCCESS")) {
        sscanf(tokens[2].c_str(),"0x%x",&comprc);
        if ((i_returnCode & 0xFF000000) == comprc) {
          /* This came from this source, we will save that as we may use it later */
          source = tokens[1];
        }
      } else if ((i_returnCode & 0xFF000000) != ECMD_ERR_ECMD) {
        /* We aren't going to find this return code in here since it didn't come from us */
      } else if (tokens.size() >= 4) {
        /* This is a standard return code define */
        sscanf(tokens[3].c_str(),"0x%x",&comprc);
        if ((i_returnCode & 0x00FFFFFF) == comprc) {
          /* This came from this source, we will save that as we may use it later */
          retdefine = tokens[1];
          found = 1;
          break;
        }
      }        
    }
  }

  ins.close();

  /* Couldn't find it in normal eCMD, let the plugin have a shot at it */
  if (!found) {
    retdefine = dllSpecificParseReturnCode(i_returnCode);
    if (retdefine.size()) {
      found = true;
    }
  }

  if (!found && source.length() == 0) {
    ret = "UNDEFINED";
  } else if (!found) {
    ret = "UNDEFINED FROM " + source;
  } else {
    ret = retdefine;
  }
  return ret;
}

/* TargetConfiged and TargetExist are the same function, except for having to call a different interface */
/* Wrapping them this way, to call the internal queryTargetConfiguredExist is the most efficient way */
bool dllQueryTargetConfigured(const ecmdChipTarget & i_target, const ecmdQueryData * i_queryData) {
  return queryTargetConfigExist(i_target, i_queryData, false);
}

bool dllQueryTargetExist(const ecmdChipTarget & i_target, const ecmdQueryData * i_queryData) {
  return queryTargetConfigExist(i_target, i_queryData, true);
}

bool queryTargetConfigExist(const ecmdChipTarget & i_target, const ecmdQueryData * i_queryData, bool i_existQuery) {
  uint32_t rc = ECMD_SUCCESS;
  bool ret = false;
  ecmdChipTarget queryTarget;
  const ecmdQueryData * l_queryData = i_queryData;
  ecmdQueryData * l_myQueryData = NULL;

  std::list<ecmdCageData>::const_iterator ecmdCurCage;
  std::list<ecmdNodeData>::const_iterator ecmdCurNode;
  std::list<ecmdSlotData>::const_iterator ecmdCurSlot;
  std::list<ecmdChipData>::const_iterator ecmdCurChip;
  std::list<ecmdChipUnitData>::const_iterator ecmdCurChipUnit;
  std::list<ecmdThreadData>::const_iterator ecmdCurThread;


  /* Do we need to do our own query ? */
  if (l_queryData == NULL) {
    l_myQueryData = new ecmdQueryData;
    queryTarget = i_target;

    /* Force the states to be right, if not set properly */
    if (queryTarget.cageState != ECMD_TARGET_FIELD_UNUSED) 
      queryTarget.cageState = ECMD_TARGET_FIELD_VALID;
    if (queryTarget.nodeState != ECMD_TARGET_FIELD_UNUSED) 
      queryTarget.nodeState = ECMD_TARGET_FIELD_VALID;
    if (queryTarget.slotState != ECMD_TARGET_FIELD_UNUSED) 
      queryTarget.slotState = ECMD_TARGET_FIELD_VALID;
    if (queryTarget.chipTypeState != ECMD_TARGET_FIELD_UNUSED) 
      queryTarget.chipTypeState = ECMD_TARGET_FIELD_VALID;
    if (queryTarget.posState != ECMD_TARGET_FIELD_UNUSED) 
      queryTarget.posState = ECMD_TARGET_FIELD_VALID;
    if (queryTarget.chipUnitTypeState != ECMD_TARGET_FIELD_UNUSED) 
      queryTarget.chipUnitTypeState = ECMD_TARGET_FIELD_VALID;
    if (queryTarget.chipUnitNumState != ECMD_TARGET_FIELD_UNUSED) 
      queryTarget.chipUnitNumState = ECMD_TARGET_FIELD_VALID;
    if (queryTarget.threadState != ECMD_TARGET_FIELD_UNUSED) 
      queryTarget.threadState = ECMD_TARGET_FIELD_VALID;

    if (i_existQuery) {
      rc = dllQueryExist(queryTarget, *l_myQueryData, ECMD_QUERY_DETAIL_LOW);
    } else {
      rc = dllQueryConfig(queryTarget, *l_myQueryData, ECMD_QUERY_DETAIL_LOW);
    }
    if (rc) {
      delete l_myQueryData;
      return ret;
    }
    l_queryData = l_myQueryData;
  }


  /* Now we have our data, let's start walking the data we have */
  for (ecmdCurCage = l_queryData->cageData.begin(); ecmdCurCage != l_queryData->cageData.end(); ecmdCurCage ++) {
    if (ecmdCurCage->cageId == i_target.cage) {
      if (i_target.nodeState == ECMD_TARGET_FIELD_UNUSED) {
        ret = true;
        break;
      }

      for (ecmdCurNode = ecmdCurCage->nodeData.begin(); ecmdCurNode != ecmdCurCage->nodeData.end(); ecmdCurNode ++) {
        if (ecmdCurNode->nodeId == i_target.node) {
          if (i_target.slotState == ECMD_TARGET_FIELD_UNUSED) {
            ret = true;
            break;
          }


          for (ecmdCurSlot = ecmdCurNode->slotData.begin(); ecmdCurSlot != ecmdCurNode->slotData.end(); ecmdCurSlot ++) {
            if (ecmdCurSlot->slotId == i_target.slot) {
              if (i_target.chipTypeState == ECMD_TARGET_FIELD_UNUSED || i_target.posState == ECMD_TARGET_FIELD_UNUSED) {
                ret = true;
                break;
              }

              for (ecmdCurChip = ecmdCurSlot->chipData.begin(); ecmdCurChip != ecmdCurSlot->chipData.end(); ecmdCurChip ++) {
                if (((ecmdCurChip->chipType == i_target.chipType) || (ecmdCurChip->chipCommonType == i_target.chipType) || (ecmdCurChip->chipShortType == i_target.chipType)) &&
                    (ecmdCurChip->pos == i_target.pos)) {
                  if (i_target.chipUnitNumState == ECMD_TARGET_FIELD_UNUSED) {
                    ret = true;
                    break;
                  }

                  for (ecmdCurChipUnit = ecmdCurChip->chipUnitData.begin(); ecmdCurChipUnit != ecmdCurChip->chipUnitData.end(); ecmdCurChipUnit ++) {
                    if (ecmdCurChipUnit->chipUnitNum == i_target.chipUnitNum) {
                      if (i_target.threadState == ECMD_TARGET_FIELD_UNUSED) {
                        ret = true;
                        break;
                      }

                      for (ecmdCurThread = ecmdCurChipUnit->threadData.begin(); ecmdCurThread != ecmdCurChipUnit->threadData.end(); ecmdCurThread ++) {
                        if (ecmdCurThread->threadId == i_target.thread) {
                          ret = true;
                          break;
                        } /* curThreadId == tarThreadId */
                      } /* for ecmdCurThread */

                      if (ret) break;
                    } /* curChipUnitNum == tarChipUnitId */
                  } /* for ecmdCurChipUnit */

                  if (ret) break;
                } /* curChipType == tarChipType && curChipPos == tarChipPos */
              } /* for ecmdCurChip */

              if (ret) break;
            } /* curSlotId == tarSlotId */
          } /* for ecmdCurSlot */

          if (ret) break;
        } /* curNodeId == tarNodeId */
      } /* for ecmdCurNode */

      if (ret) break;
    } /* curCageId == tarCageId */
  } /* for ecmdCurCage */


  if (l_myQueryData) {
    delete l_myQueryData;
  }

  return ret;
}



/**
 @brief Get Current Cmdline String 
 @retval String representing current cmdline string being processed
*/
std::string dllGetCurrentCmdline(){ return ecmdGlobal_currentCmdline; }


/**
 @brief Convert list of Cmdline Args to String and Save in Dll 
 @param argc Command line arguments
 @param argv Command line arguments
*/
void dllSetCurrentCmdline(int argc, char* argv[])
{
  // new string coming in, so erase/clear what was there first
  ecmdGlobal_currentCmdline="";

  // now create new string from argv[] array of size argc
  for (int i=0 ; i < argc ; i++ )
  {
    ecmdGlobal_currentCmdline += argv[i];
    ecmdGlobal_currentCmdline += " ";
  }
}
