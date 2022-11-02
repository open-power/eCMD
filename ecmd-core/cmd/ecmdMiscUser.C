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


//----------------------------------------------------------------------
//  Includes
//----------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <ecmdCommandUtils.H>
#include <ecmdReturnCodes.H>
#include <ecmdClientCapi.H>
#include <ecmdUtils.H>
#include <ecmdDataBuffer.H>
#include <ecmdInterpreter.H>
#include <ecmdSharedUtils.H>
#include <algorithm>

//----------------------------------------------------------------------
//  User Types
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//  Constants
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//  Macros
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//  Internal Function Prototypes
//----------------------------------------------------------------------
// Prototype of fuction to format ecid to wafer ID
uint32_t formatEcidString( ecmdDataBuffer & i_ecidBuffer, std::string  & o_waferString);


//----------------------------------------------------------------------
//  Global Variables
//----------------------------------------------------------------------

//---------------------------------------------------------------------
// Member Function Specifications
//---------------------------------------------------------------------

uint32_t ecmdGetConfigUser(int argc, char * argv[]) {
  uint32_t rc = ECMD_SUCCESS, coeRc = ECMD_SUCCESS;

  ecmdChipTarget target;        ///< Current target
  std::string configName;       ///< Name of config variable to fetch
  bool validPosFound = false;   ///< Did we find something to actually execute on ?
  ecmdConfigData configData;    ///< Structure holding our return data
  ecmdDataBuffer numData;       ///< Initialise data buffer with the numeric value
  std::string printed;          ///< Print Buffer
  ecmdLooperData looperData;    ///< Store internal Looper data

  int CAGE = 1, NODE = 2, SLOT = 3, POS = 4, CHIPUNIT = 5;
  int depth = 0;                 ///< depth found from Command line parms

  /* get format flag, if it's there */
  std::string format;
  char * formatPtr = ecmdParseOptionWithArgs(&argc, &argv, "-o");
  if (formatPtr == NULL) {
    format = "x";
  } else {
    format = formatPtr;
  }

  if (ecmdParseOption(&argc, &argv, "-dk"))             depth = CAGE;
  else if (ecmdParseOption(&argc, &argv, "-dn"))        depth = NODE;
  else if (ecmdParseOption(&argc, &argv, "-ds"))        depth = SLOT;
  else if (ecmdParseOption(&argc, &argv, "-dp"))        depth = POS;
  else if (ecmdParseOption(&argc, &argv, "-dc"))        depth = CHIPUNIT;

  /************************************************************************/
  /* Parse Common Cmdline Args                                            */
  /************************************************************************/
  rc = ecmdCommandArgs(&argc, &argv);
  if (rc) return rc;

  /* Global args have been parsed, we can read if -coe was given */
  bool coeMode = ecmdGetGlobalVar(ECMD_GLOBALVAR_COEMODE); ///< Are we in continue on error mode

  /************************************************************************/
  /* Parse Local ARGS here!                                               */
  /************************************************************************/
  if (argc < 1) {
    ecmdOutputError("getconfig - Too few arguments specified; you need at least a ConfigName.\n");
    ecmdOutputError("getconfig - Type 'getconfig -h' for usage.\n");
    return ECMD_INVALID_ARGS;
  }


  //Setup the target that will be used to query the system config
  if (argc > 2) {
    ecmdOutputError("getconfig - Too many arguments specified; you probably added an unsupported option.\n");
    ecmdOutputError("getconfig - Type 'getconfig -h' for usage.\n");
    return ECMD_INVALID_ARGS;
  } else if( argc == 2) {
    std::string chipType, chipUnitType;
    rc = ecmdParseChipField(argv[0], chipType, chipUnitType);
    if (rc) { 
      ecmdOutputError("getconfig - Wildcard character detected however it is not supported by this command.\n");
      return rc;
    }

    /* Error check */
    if (depth) {
      if (chipUnitType == "" && depth < POS) {
        ecmdOutputError("getconfig - Invalid Depth parm specified when a chip was specified.  Try with -dp.\n");
        return ECMD_INVALID_ARGS;
      }

      if (chipUnitType != "" && depth < CHIPUNIT) {
        ecmdOutputError("getconfig - Invalid Depth parm specified when a chipUnit was specified.  Try with -dc.\n");
        return ECMD_INVALID_ARGS;
      }
    } else { /* No depth, set on for the code below */
      if (chipUnitType == "") {
        depth = POS;
      } else {
        depth = CHIPUNIT;
      }
    }
    target.chipType = chipType;
    target.chipTypeState = ECMD_TARGET_FIELD_VALID;
    target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;
    if (chipUnitType != "") {
      target.chipUnitType = chipUnitType;
      target.chipUnitTypeState = ECMD_TARGET_FIELD_VALID;
    }
    configName = argv[1];
  } else {
    if (depth == 0) depth = CAGE;
    configName = argv[0];
  }

  /* Now set our states based on depth */
  target.cageState = target.nodeState = target.slotState = target.posState = target.chipUnitNumState = ECMD_TARGET_FIELD_WILDCARD;
  target.threadState = ECMD_TARGET_FIELD_UNUSED;
  if (depth == POS) {
    target.chipUnitNumState = ECMD_TARGET_FIELD_UNUSED;
    target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;
  } else if (depth == SLOT) {
    target.chipTypeState = ECMD_TARGET_FIELD_UNUSED;
  } else if (depth == NODE) {
    target.slotState = ECMD_TARGET_FIELD_UNUSED;
  } else if (depth == CAGE) {
    target.nodeState = ECMD_TARGET_FIELD_UNUSED;
  }

  /************************************************************************/
  /* Kickoff Looping Stuff                                                */
  /************************************************************************/
  rc = ecmdLooperInit(target, ECMD_SELECTED_TARGETS_LOOP, looperData);
  if (rc) return rc;

  while (ecmdLooperNext(target, looperData) && (!coeRc || coeMode)) {

    /* Actually go fetch the data */
    rc = ecmdGetConfigurationComplex(target, configName, configData);
    if (rc) {
      printed = "getconfig - Error occured performing ecmdGetConfiguration on ";
      printed += ecmdWriteTarget(target) + "\n";
      ecmdOutputError( printed.c_str() );
      coeRc = rc;
      continue;
    }
    else {
      validPosFound = true;
    }

    printed = ecmdWriteTarget(target) + "\n";

    /* If there was multiple return data types, print them all to the screen */
    if (configData.validMask & ECMD_CONFIG_VALID_FIELD_STRING) {
      printed += configName + " = " + configData.cdString + "\n";
      ecmdOutput(printed.c_str());
    }
    if (configData.validMask & ECMD_CONFIG_VALID_FIELD_FLOAT) {
      char floatStr[100];
      sprintf(floatStr, "%f", configData.cdFloat);
      printed += configName + " = " + floatStr + "\n";
      ecmdOutput(printed.c_str());
    }
    if((configData.validMask & ECMD_CONFIG_VALID_FIELD_UINT32) || (configData.validMask & ECMD_CONFIG_VALID_FIELD_UINT64)) {
      if (configData.validMask & ECMD_CONFIG_VALID_FIELD_UINT32) {
        numData.setBitLength(32);
        numData.setWord(0, configData.cdUint32);
      } else {
        numData.setBitLength(64);
        numData.setDoubleWord(0, configData.cdUint64);
      }
      printed += configName + " = ";
      printed += ecmdWriteDataFormatted(numData, format);
      printed += "\n";
      ecmdOutput(printed.c_str()); 
    }

  }
  // coeRc will be the return code from in the loop, coe mode or not.
  if (coeRc) return coeRc;

  // This is an error common across all UI functions
  if (!validPosFound) {
    ecmdOutputError("getconfig - Unable to find a valid chip to execute command on\n");
    return ECMD_TARGET_NOT_CONFIGURED;
  }

  return rc;
}

uint32_t ecmdSetConfigUser(int argc, char * argv[]) {
  uint32_t rc = ECMD_SUCCESS, coeRc = ECMD_SUCCESS;

  ecmdChipTarget target;        ///< Current target
  std::string configName;       ///< Name of config variable to fetch
  bool validPosFound = false;   ///< Did we find something to actually execute on?
  ecmdConfigData configData;     ///< Holds the different config values
  configData.validMask = 0x0;
  ecmdDataBuffer inputBuffer;	 ///< Initialise data buffer with the numeric value(if input is Numeric)
  char * inputVal = NULL;		 ///< Pointer to argv with value
  std::string printed;           ///< Print Buffer
  ecmdLooperData looperData;     ///< Store internal Looper data
  uint32_t l_mode = ECMD_SETCONFIG_MODE_REPLACE;   ///< l_mode determines whether to replace, add, or remove the current value in the config file with the new value

  int CAGE = 1, NODE = 2, SLOT = 3, POS = 4, CHIPUNIT = 5;
  int depth = 0;                 ///< depth found from Command line parms
  int l_argOffset = 0;

  /* Flags that support multivalues can use the add and remove options */
  char * addPtr = ecmdParseOptionWithArgs(&argc, &argv, "add");
  if (addPtr != NULL)
  {
    l_mode = ECMD_SETCONFIG_MODE_ADD;
    inputVal = addPtr;
    l_argOffset = 1;
  }
  char * removePtr = ecmdParseOptionWithArgs(&argc, &argv, "remove");
  if (removePtr != NULL)
  {
    l_mode = ECMD_SETCONFIG_MODE_REMOVE;
    inputVal = removePtr;
    l_argOffset = 1;
  }

  /* get format flag, if it's there */
  std::string format;
  char * formatPtr = ecmdParseOptionWithArgs(&argc, &argv, "-i");
  if (formatPtr == NULL) {
    format = "a";
  } else {
    format = formatPtr;
  }

  if ((addPtr != NULL) && (removePtr != NULL)) {
    ecmdOutputError("setconfig - Cannot add and remove a value at the same time.\n");
    return ECMD_FAILURE;
  }

  if (ecmdParseOption(&argc, &argv, "-dk"))             depth = CAGE;
  else if (ecmdParseOption(&argc, &argv, "-dn"))        depth = NODE;
  else if (ecmdParseOption(&argc, &argv, "-ds"))        depth = SLOT;
  else if (ecmdParseOption(&argc, &argv, "-dp"))        depth = POS;
  else if (ecmdParseOption(&argc, &argv, "-dc"))        depth = CHIPUNIT;

  /************************************************************************/
  /* Parse Common Cmdline Args                                            */
  /************************************************************************/
  rc = ecmdCommandArgs(&argc, &argv);
  if (rc) return rc;

  /* Global args have been parsed, we can read if -coe was given */
  bool coeMode = ecmdGetGlobalVar(ECMD_GLOBALVAR_COEMODE); ///< Are we in continue on error mode

  /************************************************************************/
  /* Parse Local ARGS here!                                               */
  /************************************************************************/

  if (argc < (2 - l_argOffset)) {
    ecmdOutputError("setconfig - Too few arguments specified; you need at least a ConfigName, Value to set it to.\n");
    ecmdOutputError("setconfig - Type 'setconfig -h' for usage.\n");
    return ECMD_INVALID_ARGS;
  }

  //Setup the target that will be used to query the system config
  if (argc > (3 - l_argOffset)) {
    ecmdOutputError("setconfig - Too many arguments specified; you probably added an unsupported option.\n");
    ecmdOutputError("setconfig - Type 'setconfig -h' for usage.\n");
    return ECMD_INVALID_ARGS;
  } else if (argc == (3 - l_argOffset)) {
    std::string chipType, chipUnitType;
    rc = ecmdParseChipField(argv[0], chipType, chipUnitType);
    if (rc) { 
      ecmdOutputError("setconfig - Wildcard character detected however it is not supported by this command.\n");
      return rc;
    }

    /* Error check */
    if (depth) {
      if (chipUnitType == "" && depth < POS) {
        ecmdOutputError("setconfig - Invalid Depth parm specified when a chip was specified.  Try with -dp.\n");
        return ECMD_INVALID_ARGS;
      }

      if (chipUnitType != "" && depth < CHIPUNIT) {
        ecmdOutputError("setconfig - Invalid Depth parm specified when a chipUnit was specified.  Try with -dc.\n");
        return ECMD_INVALID_ARGS;
      }
    } else { /* No depth, set on for the code below */
      if (chipUnitType == "") {
        depth = POS;
      } else {
        depth = CHIPUNIT;
      }
    }
    target.chipType = chipType;
    target.chipTypeState = ECMD_TARGET_FIELD_VALID;
    target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;
    if (chipUnitType != "") {
      target.chipUnitType = chipUnitType;
      target.chipUnitTypeState = ECMD_TARGET_FIELD_VALID;
    }
    configName = argv[1];
    if ((addPtr == NULL) && (removePtr == NULL)) inputVal = argv[2];
  }
  else {
    if (depth == 0) depth = CAGE;
    target.chipTypeState = ECMD_TARGET_FIELD_UNUSED;
    configName = argv[0];
    if ((addPtr == NULL) && (removePtr == NULL)) inputVal = argv[1];    
  }

  if (format == "a") {
    configData.cdString = inputVal;
    configData.validMask |= ECMD_CONFIG_VALID_FIELD_STRING;
  } else if (format == "f") {
    configData.cdFloat = atof(inputVal);
    configData.validMask |= ECMD_CONFIG_VALID_FIELD_FLOAT;
  } else {
    rc = ecmdReadDataFormatted(inputBuffer, inputVal, format);
    if (rc) {
      ecmdOutputError("setconfig - Problems occurred parsing input data, must be an invalid format\n");
      return rc;
    }
    uint32_t numBits = inputBuffer.getBitLength();
    if (numBits > 64) {
      ecmdOutputError("setconfig - Problems occurred parsing input data, Bitlength should be <= 64 bits\n");
      return ECMD_INVALID_ARGS;
    } else if (numBits > 32) {
      inputBuffer.shiftRightAndResize(64-numBits);
      configData.cdUint64 = inputBuffer.getDoubleWord(0);
      configData.validMask |= ECMD_CONFIG_VALID_FIELD_UINT64;
    } else {
      inputBuffer.shiftRightAndResize(32-numBits);
      configData.cdUint32 = inputBuffer.getWord(0);
      configData.validMask |= ECMD_CONFIG_VALID_FIELD_UINT32;
    }
  }
    
   /* Now set our states based on depth */
  target.cageState = target.nodeState = target.slotState = target.posState = target.chipUnitNumState = ECMD_TARGET_FIELD_WILDCARD;
  target.threadState = ECMD_TARGET_FIELD_UNUSED;
  if (depth == POS) {
    target.chipUnitNumState = ECMD_TARGET_FIELD_UNUSED;
    target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;
  } else if (depth == SLOT) {
    target.chipTypeState = ECMD_TARGET_FIELD_UNUSED;
  } else if (depth == NODE) {
    target.slotState = ECMD_TARGET_FIELD_UNUSED;
  } else if (depth == CAGE) {
    target.nodeState = ECMD_TARGET_FIELD_UNUSED;
  }
  
  /************************************************************************/
  /* Kickoff Looping Stuff                                                */
  /************************************************************************/
  rc = ecmdLooperInit(target, ECMD_SELECTED_TARGETS_LOOP, looperData);
  if (rc) return rc;

  while (ecmdLooperNext(target, looperData) && (!coeRc || coeMode)) {
    rc = ecmdSetConfigurationComplexHidden(target, configName, configData, l_mode);
    if (rc) {
      printed = "setconfig - Error occured performing ecmdSetConfiguration on ";
      printed += ecmdWriteTarget(target) + "\n";
      ecmdOutputError( printed.c_str() );
      coeRc = rc;
      continue;
    } else {
      validPosFound = true;
    }
    printed = ecmdWriteTarget(target) + "\n";
    ecmdOutput( printed.c_str() );
  }
  // coeRc will be the return code from in the loop, coe mode or not.
  if (coeRc) return coeRc;

  // This is an error common across all UI functions
  if (!validPosFound) {
    ecmdOutputError("setconfig - Unable to find a valid chip to execute command on\n");
    return ECMD_TARGET_NOT_CONFIGURED;
  }

  return rc;
}

#ifndef ECMD_REMOVE_FSI_FUNCTIONS
uint32_t ecmdGetCfamUser(int argc, char* argv[]) {
  uint32_t rc = ECMD_SUCCESS, coeRc = ECMD_SUCCESS;

  bool expectFlag = false;
  bool maskFlag = false;
  char* expectPtr = NULL;                       ///< Pointer to expected data in arg list
  char* maskPtr = NULL;                         ///< Pointer to mask data in arg list
  const char* verbosePtr = NULL;                      ///< Pointer to -v[s0,s1] in arg list
  ecmdDataBuffer expected;                      ///< Buffer to store expected data
  ecmdDataBuffer mask;                          ///< Buffer for mask of expected data
  std::string outputformat = "x";               ///< Output Format to display
  std::string inputformat = "x";                ///< Input format of data
  ecmdChipTarget target;                        ///< Current target being operated on
  ecmdDataBuffer buffer;                        ///< Buffer to hold Cfam data
  bool validPosFound = false;                   ///< Did the looper find anything?
  ecmdLooperData looperData;                    ///< Store internal Looper data
  std::string printed;                          ///< Output data

  /************************************************************************/
  /* Parse Local FLAGS here!                                              */
  /************************************************************************/
  //expect and mask flags check
  if ((expectPtr = ecmdParseOptionWithArgs(&argc, &argv, "-exp")) != NULL) {
    expectFlag = true;

    if ((maskPtr = ecmdParseOptionWithArgs(&argc, &argv, "-mask")) != NULL) {
      maskFlag = true;
    }
  }
  
  if (ecmdParseOption(&argc, &argv, "-v")) {
    verbosePtr = "-v";
  }
  else if (ecmdParseOption(&argc, &argv, "-vs0")) {
    verbosePtr = "-vs0";
  }
  else if (ecmdParseOption(&argc, &argv, "-vs1")) {
    verbosePtr = "-vs1";
  }
  



  /* get format flag, if it's there */
  char * formatPtr = ecmdParseOptionWithArgs(&argc, &argv, "-o");
  if (formatPtr != NULL) {
    outputformat = formatPtr;
  }
  formatPtr = ecmdParseOptionWithArgs(&argc, &argv, "-i");
  if (formatPtr != NULL) {
    inputformat = formatPtr;
  }

  /************************************************************************/
  /* Parse Common Cmdline Args                                            */
  /************************************************************************/
  rc = ecmdCommandArgs(&argc, &argv);
  if (rc) return rc;

  /* Global args have been parsed, we can read if -coe was given */
  bool coeMode = ecmdGetGlobalVar(ECMD_GLOBALVAR_COEMODE); ///< Are we in continue on error mode

  /************************************************************************/
  /* Parse Local ARGS here!                                               */
  /************************************************************************/
  if (argc < 2) {  //chip + address
    ecmdOutputError("getcfam - Too few arguments specified; you need at least a chip and an address.\n");
    ecmdOutputError("getcfam - Type 'getcfam -h' for usage.\n");
    return ECMD_INVALID_ARGS;
  }

  //Setup the target that will be used to query the system config
  std::string chipType, chipUnitType;
  rc = ecmdParseChipField(argv[0], chipType, chipUnitType, true /* supports wildcard usage */);
  if (rc) { 
    ecmdOutputError("getcfam - Wildcard character detected however it is not being used correctly.\n");
    return rc;
  }

  if (chipUnitType != "") {
    ecmdOutputError("getcfam - chipUnit specified on the command line, this function doesn't support chipUnits.\n");
    return ECMD_INVALID_ARGS;
  }

  if (chipType == "x") {
    target.chipTypeState = ECMD_TARGET_FIELD_WILDCARD;
  } else {
    target.chipType = chipType;
    target.chipTypeState = ECMD_TARGET_FIELD_VALID;
  }

  target.cageState = target.nodeState = target.slotState = target.posState = ECMD_TARGET_FIELD_WILDCARD;
  target.chipUnitTypeState = target.chipUnitNumState = target.threadState = ECMD_TARGET_FIELD_UNUSED;

  //get address to fetch
  if (!ecmdIsAllHex(argv[1])) {
    ecmdOutputError("getcfam - Non-hex characters detected in address field\n");
    return ECMD_INVALID_ARGS;
  } else if (strlen(argv[1]) > 6) {
    ecmdOutputError("getcfam - Cfam addresses must be <= 24 bits in length\n"); 
    return ECMD_INVALID_ARGS;
  }
  uint32_t address = ecmdGenB32FromHexRight(&address, argv[1]);


  if (expectFlag) {

    rc = ecmdReadDataFormatted(expected, expectPtr, inputformat);
    if (rc) {
      ecmdOutputError("getcfam - Problems occurred parsing expected data, must be an invalid format\n");
      return rc;
    }

    if (maskFlag) {
      rc = ecmdReadDataFormatted(mask, maskPtr, inputformat);
      if (rc) {
        ecmdOutputError("getcfam - Problems occurred parsing mask data, must be an invalid format\n");
        return rc;
      }

    }


  }
  if (argc > 3) { 
    ecmdOutputError("getcfam - Too many arguments specified; you probably added an option that wasn't recognized.\n");
    ecmdOutputError("getcfam - Type 'getcfam -h' for usage.\n");
    return ECMD_INVALID_ARGS;
  }

  /************************************************************************/
  /* Kickoff Looping Stuff                                                */
  /************************************************************************/

  rc = ecmdLooperInit(target, ECMD_SELECTED_TARGETS_LOOP, looperData);
  if (rc) return rc;


  while (ecmdLooperNext(target, looperData) && (!coeRc || coeMode)) {

    rc = getCfamRegister(target, address, buffer);
    if (rc) {
        printed = "getcfam - Error occured performing getcfam on ";
        printed += ecmdWriteTarget(target);
        printed += "\n";
        ecmdOutputError( printed.c_str() );
        coeRc = rc;
        continue;
    }
    else {
      validPosFound = true;
    }

    if (expectFlag) {

      if (maskFlag) {
        buffer.setAnd(mask, 0, buffer.getBitLength());
      }

      uint32_t mismatch = ECMD_UNSET;
      if (!ecmdCheckExpected(buffer, expected, mismatch)) {

        //@ make this stuff sprintf'd
        char outstr[75];
        printed = ecmdWriteTarget(target);
        sprintf(outstr, "\ngetcfam - Data miscompare occured at address: %.8X\n", address);
        printed += outstr;
        ecmdOutputError( printed.c_str() );


        printed = "getcfam - Actual";
        if (maskFlag) {
          printed += " (with mask): ";
        }
        else {
          printed += "            : ";
        }

        printed += ecmdWriteDataFormatted(buffer, outputformat);
        ecmdOutputError( printed.c_str() );

        printed = "getcfam - Expected          : ";
        printed += ecmdWriteDataFormatted(expected, outputformat);
        ecmdOutputError( printed.c_str() );
        coeRc = ECMD_EXPECT_FAILURE;
        continue;
      }

    }
    else {

      printed = ecmdWriteTarget(target);
      printed += ecmdWriteDataFormatted(buffer, outputformat);
      ecmdOutput( printed.c_str() );

      if ((verbosePtr != NULL) && !expectFlag) {
      //even if rc returned is non-zero we want to continue to the next chip
#if !(defined (ECMD_REMOVE_SEDC_SUPPORT) || defined (ECMD_REMOVE_SCOM_FUNCTIONS))
        ecmdScomData scomData;   ///< Scom data 
        scomData.address = address;
        ecmdDisplayScomData(target, scomData, buffer, verbosePtr);
#else
	ecmdOutputWarning("ecmdDisplayScomData is not supported in this getscom implementation (ECMD_REMOVE_SEDC_SUPPORT has been defined)\n");
#endif
      }

    }
  }
  // coeRc will be the return code from in the loop, coe mode or not.
  if (coeRc) return coeRc;

  // This is an error common across all UI functions
  if (!validPosFound) {
    ecmdOutputError("getcfam - Unable to find a valid chip to execute command on\n");
    return ECMD_TARGET_NOT_CONFIGURED;
  }

  return rc;
}

uint32_t ecmdPutCfamUser(int argc, char* argv[]) {

  uint32_t rc = ECMD_SUCCESS, coeRc = ECMD_SUCCESS;
  std::string inputformat = "x";                ///< Default input format
  std::string dataModifier = "insert";          ///< Default data Modifier (And/Or/insert)
  ecmdDataBuffer fetchBuffer;                   ///< Buffer to store read/modify/write data
  ecmdLooperData looperData;                    ///< Store internal Looper data
  ecmdChipTarget target;                        ///< Chip target being operated on
  uint32_t address;                             ///< Cfam address
  ecmdDataBuffer buffer;                        ///< Container to store write data
  bool validPosFound = false;                   ///< Did the config looper actually find a chip ?
  std::string printed;                          ///< String for printed data
  uint32_t startbit = ECMD_UNSET;               ///< Startbit to insert data
  uint32_t numbits = 0;                         ///< Number of bits to insert data

  /************************************************************************/
  /* Parse Local FLAGS here!                                              */
  /************************************************************************/
  char* formatPtr = ecmdParseOptionWithArgs(&argc, &argv, "-i");
  if (formatPtr != NULL) {
    inputformat = formatPtr;
  }

  formatPtr = ecmdParseOptionWithArgs(&argc, &argv, "-b");
  if (formatPtr != NULL) {
    dataModifier = formatPtr;
  }

  /************************************************************************/
  /* Parse Common Cmdline Args,                                           */
  /************************************************************************/
  rc = ecmdCommandArgs(&argc, &argv);
  if (rc) return rc;

  /* Global args have been parsed, we can read if -coe was given */
  bool coeMode = ecmdGetGlobalVar(ECMD_GLOBALVAR_COEMODE); ///< Are we in continue on error mode

  /************************************************************************/
  /* Parse Local ARGS here!                                               */
  /************************************************************************/

  if (argc < 3) {  //chip + address + some data
    ecmdOutputError("putcfam - Too few arguments specified; you need at least a chip, an address, and some data.\n");
    ecmdOutputError("putcfam - Type 'putcfam -h' for usage.\n");
    return ECMD_INVALID_ARGS;
  }

  //Setup the target that will be used to query the system config
  std::string chipType, chipUnitType;
  rc = ecmdParseChipField(argv[0], chipType, chipUnitType, true /* supports wildcard usage */);
  if (rc) { 
    ecmdOutputError("putcfam - Wildcard character detected however it is not being used correctly.\n");
    return rc;
  }

  if (chipUnitType != "") {
    ecmdOutputError("putcfam - chipUnit specified on the command line, this function doesn't support chipUnits.\n");
    return ECMD_INVALID_ARGS;
  }

  if (chipType == "x") {
    target.chipTypeState = ECMD_TARGET_FIELD_WILDCARD;
  } else {
    target.chipType = chipType;
    target.chipTypeState = ECMD_TARGET_FIELD_VALID;
  }

  target.cageState = target.nodeState = target.slotState = target.posState = ECMD_TARGET_FIELD_WILDCARD;
  target.chipUnitTypeState = target.chipUnitNumState = target.threadState = ECMD_TARGET_FIELD_UNUSED;

  if (!ecmdIsAllHex(argv[1])) {
    ecmdOutputError("putcfam - Non-hex characters detected in address field\n");
    return ECMD_INVALID_ARGS;
  } else if (strlen(argv[1]) > 6) {
    ecmdOutputError("putcfam - Cfam addresses must be <= 24 bits in length\n");
    return ECMD_INVALID_ARGS;
  }
  address = ecmdGenB32FromHexRight(&address, argv[1]);

  /* Did they specify a start/numbits */
  if (argc > 3) {
    if (argc != 5) {
      ecmdOutputError("putcfam - Too many arguments specified; you probably added an unsupported option.\n");

      ecmdOutputError("putcfam - Type 'putcfam -h' for usage.\n");
      return ECMD_INVALID_ARGS;
    }

    if (!ecmdIsAllDecimal(argv[2])) {
      ecmdOutputError("putcfam - Non-decimal characters detected in startbit field\n");
      return ECMD_INVALID_ARGS;
    }
    startbit = (uint32_t)atoi(argv[2]);
    if (!ecmdIsAllDecimal(argv[3])) {
      ecmdOutputError("putcfam - Non-decimal characters detected in numbits field\n");
      return ECMD_INVALID_ARGS;
    }
    numbits = (uint32_t)atoi(argv[3]);


    /* Bounds check */
    if ((startbit + numbits) > ECMD_MAX_DATA_BITS) {
      char errbuf[100];
      sprintf(errbuf,"putcfam - Too much data requested > %d bits\n", ECMD_MAX_DATA_BITS);
      ecmdOutputError(errbuf);
      return ECMD_DATA_BOUNDS_OVERFLOW;
    } else if (numbits == 0) {
      ecmdOutputError("putcfam - Number of bits == 0, operation not performed\n");
      return ECMD_INVALID_ARGS;
    }

    rc = ecmdReadDataFormatted(buffer, argv[4], inputformat, (int)numbits);
    if (rc) {
      ecmdOutputError("putcfam - Problems occurred parsing input data, must be an invalid format\n");
      return rc;
    }


  } else {

    rc = ecmdReadDataFormatted(buffer, argv[2], inputformat);
    if (rc) {
      ecmdOutputError("putcfam - Problems occurred parsing input data, must be an invalid format\n");
      return rc;
    }
  }

  /************************************************************************/
  /* Kickoff Looping Stuff                                                */
  /************************************************************************/

  rc = ecmdLooperInit(target, ECMD_SELECTED_TARGETS_LOOP, looperData);
  if (rc) return rc;

  while (ecmdLooperNext(target, looperData) && (!coeRc || coeMode)) {

    /* Do we need to perform a read/modify/write op ? */
    if ((dataModifier != "insert") || (startbit != ECMD_UNSET)) {


      rc = getCfamRegister(target, address, fetchBuffer);

      if (rc) {
        printed = "putcfam - Error occured performing getcfam on ";
        printed += ecmdWriteTarget(target);
        printed += "\n";
        ecmdOutputError( printed.c_str() );
        coeRc = rc;
        continue;
      }
      else {
        validPosFound = true;
      }

      rc = ecmdApplyDataModifier(fetchBuffer, buffer, (startbit == ECMD_UNSET ? 0 : startbit), dataModifier);
      if (rc) break;

      rc = putCfamRegister(target, address, fetchBuffer);
      if (rc) {
        printed = "putcfam - Error occured performing putcfam on ";
        printed += ecmdWriteTarget(target);
        printed += "\n";
        ecmdOutputError( printed.c_str() );
        coeRc = rc;
        continue;
      }

    }
    else {

      rc = putCfamRegister(target, address, buffer);
      if (rc) {
        printed = "putcfam - Error occured performing putcfam on ";
        printed += ecmdWriteTarget(target);
        printed += "\n";
        ecmdOutputError( printed.c_str() );
        coeRc = rc;
        continue;
      }
      else {
        validPosFound = true;
      }

    }

    if (!ecmdGetGlobalVar(ECMD_GLOBALVAR_QUIETMODE)) {
      printed = ecmdWriteTarget(target) + "\n";
      ecmdOutput(printed.c_str());
    }
  }
  // coeRc will be the return code from in the loop, coe mode or not.
  if (coeRc) return coeRc;

  // This is an error common across all UI functions
  if (!validPosFound) {
    ecmdOutputError("putcfam - Unable to find a valid chip to execute command on\n");
    return ECMD_TARGET_NOT_CONFIGURED;
  }

  return rc;
}
#endif // ECMD_REMOVE_FSI_FUNCTIONS

#ifndef ECMD_REMOVE_SP_FUNCTIONS
uint32_t ecmdMakeSPSystemCallUser(int argc, char * argv[]) {
  uint32_t rc = ECMD_SUCCESS, coeRc = ECMD_SUCCESS;

  ecmdChipTarget target;        ///< Current target
  bool validPosFound = false;   ///< Did we find something to actually execute on ?
  std::string printed;          ///< Print Buffer
  std::string command;          ///< Print Buffer
  std::string standardop = "";       ///< Standard out captured by running command
  ecmdLooperData looperData;    ///< Store internal Looper data
  bool raw = false;             ///< Only output data from command, force quiet mode

  raw = ecmdParseOption(&argc, &argv, "-raw");

  /************************************************************************/
  /* Parse Common Cmdline Args                                            */
  /************************************************************************/

  rc = ecmdCommandArgs(&argc, &argv);
  if (rc) return rc;

  /* Global args have been parsed, we can read if -coe was given */
  bool coeMode = ecmdGetGlobalVar(ECMD_GLOBALVAR_COEMODE); ///< Are we in continue on error mode

  if (raw && !ecmdGetGlobalVar(ECMD_GLOBALVAR_QUIETMODE)) {
      ecmdSetGlobalVar(ECMD_GLOBALVAR_QUIETMODE, 1);
  }

  //Pull out the system call
  if(argc == 0) {
      ecmdOutputError("makespsystemcall - Command to run on the SE/SP not specified.\n");
      ecmdOutputError("makespsystemcall - Type 'makespsystemcall -h' for usage.\n");
      return ECMD_INVALID_ARGS;
  }
  else {
      command = "";
      for(int i=0; i<argc; i++) {
        command += argv[i];
        command += " ";
      }
  }

  target.cageState = target.nodeState = ECMD_TARGET_FIELD_WILDCARD;
  target.slotState = target.chipTypeState = target.posState = target.chipUnitTypeState = target.chipUnitNumState = target.threadState = ECMD_TARGET_FIELD_UNUSED;

  
  rc = ecmdLooperInit(target, ECMD_SELECTED_TARGETS_LOOP, looperData);
  if (rc) return rc;

  while (ecmdLooperNext(target, looperData) && (!coeRc || coeMode)) {

    /* Actually go fetch the data */
    rc = makeSPSystemCall(target, command, standardop);
    if (rc && !raw) {
      printed = "makespsystemcall - Error occured performing makeSPSystemCall on ";
      printed += ecmdWriteTarget(target) + "\n";
      ecmdOutputError( printed.c_str() );
      if (standardop.length() != 0) {
       printed = "makespsystemcall - Output from executing the command '" + command + "':\n\n";
       ecmdOutput( printed.c_str() );
       ecmdOutput( standardop.c_str() );
      }
      coeRc = rc;
      continue;
    }
    else {
      validPosFound = true;
    }
    //Print Output
    if (!raw) {
     printed = "makespsystemcall - Output from executing the command '" + command + "':\n\n";
     ecmdOutput( printed.c_str() );
    }
    
    if(standardop.length() != 0) {
     ecmdOutput( standardop.c_str() );
    }
    else if (!raw) {
     ecmdOutput( "No Output received from makeSPSystemCall\n");
    }
  }
  // coeRc will be the return code from in the loop, coe mode or not.
  if (coeRc) return coeRc;

  // This is an error common across all UI functions
  if (!validPosFound) {
    ecmdOutputError("makespsystemcall - Unable to find a valid chip to execute command on\n");
    return ECMD_TARGET_NOT_CONFIGURED;
  }

  return rc;
}
#endif // ECMD_REMOVE_SP_FUNCTIONS

uint32_t ecmdDeconfigUser(int argc, char * argv[]) {
  uint32_t rc = ECMD_SUCCESS, coeRc = ECMD_SUCCESS;

  ecmdChipTarget target;        ///< Current target
  ecmdLooperData looperData;    ///< Store internal Looper data
  std::string printed;          ///< Print Buffer
  int CAGE = 1, NODE = 2, SLOT = 3, POS = 4, CHIPUNIT = 5;
  int depth = 0;                 ///< depth found from Command line parms

  /************************************************************************/
  /* Parse Common Cmdline Args                                            */
  /************************************************************************/
  if (ecmdParseOption(&argc, &argv, "-dk"))             depth = CAGE;
  else if (ecmdParseOption(&argc, &argv, "-dn"))        depth = NODE;
  else if (ecmdParseOption(&argc, &argv, "-ds"))        depth = SLOT;
  else if (ecmdParseOption(&argc, &argv, "-dp"))        depth = POS;
  else if (ecmdParseOption(&argc, &argv, "-dc"))        depth = CHIPUNIT;

  /************************************************************************/
  /* Parse Common Cmdline Args                                            */
  /************************************************************************/
  rc = ecmdCommandArgs(&argc, &argv);
  if (rc) return rc;

  /* Global args have been parsed, we can read if -coe was given */
  bool coeMode = ecmdGetGlobalVar(ECMD_GLOBALVAR_COEMODE); ///< Are we in continue on error mode

  //Setup the target that will be used to query the system config
  if (argc > 1) {
    ecmdOutputError("deconfig - Too many arguments specified; you probably added an unsupported option.\n");
    ecmdOutputError("deconfig - Type 'deconfig -h' for usage.\n");
    return ECMD_INVALID_ARGS;
  } else if (argc == 1) {
    std::string chipType, chipUnitType;
    rc = ecmdParseChipField(argv[0], chipType, chipUnitType, true /* supports wildcard usage */);
    if (rc) { 
      ecmdOutputError("deconfig - Wildcard character detected however it is not being used correctly.\n");
      return rc;
    }

    /* Error check */
    if (depth) {
      if (chipUnitType == "" && depth < POS) {
        ecmdOutputError("deconfig - Invalid Depth parm specified when a chip was specified.  Try with -dp.\n");
        return ECMD_INVALID_ARGS;
      }

      if (chipUnitType != "" && depth < CHIPUNIT) {
        ecmdOutputError("deconfig - Invalid Depth parm specified when a chipUnit was specified.  Try with -dc.\n");
        return ECMD_INVALID_ARGS;
      }
    } else { /* No depth, set on for the code below */
      if (chipUnitType == "") {
        depth = POS;
      } else {
        depth = CHIPUNIT;
      }
    }

    if (chipType == "x") {
      target.chipTypeState = ECMD_TARGET_FIELD_WILDCARD;
    } else {
      target.chipType = chipType;
      target.chipTypeState = ECMD_TARGET_FIELD_VALID;
    }
    target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;

    if (chipUnitType != "") {
      target.chipUnitType = chipUnitType;
      target.chipUnitTypeState = ECMD_TARGET_FIELD_VALID;
    }
  } else {
    if (depth == 0) {
      depth = CAGE;
    }
    target.chipTypeState = ECMD_TARGET_FIELD_UNUSED;
    target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;
  }

  /* Now set our states based on depth */
  target.cageState = target.nodeState = target.slotState = target.posState = target.chipUnitNumState = ECMD_TARGET_FIELD_WILDCARD;
  target.threadState = ECMD_TARGET_FIELD_UNUSED;
  if (depth == POS) {
    target.chipUnitNumState = ECMD_TARGET_FIELD_UNUSED;
  } else if (depth == SLOT) {
    target.chipTypeState = ECMD_TARGET_FIELD_UNUSED;
  } else if (depth == NODE) {
    target.slotState = ECMD_TARGET_FIELD_UNUSED;
  } else if (depth == CAGE) {
    target.nodeState = ECMD_TARGET_FIELD_UNUSED;
  }

  /************************************************************************/
  /* Kickoff Looping Stuff                                                */
  /************************************************************************/
  rc = ecmdConfigLooperInit(target, ECMD_SELECTED_TARGETS_LOOP, looperData);
  if (rc) return rc;

  while (ecmdConfigLooperNext(target, looperData) && (!coeRc || coeMode)) {

    rc = ecmdDeconfigureTarget(target);
    if (rc == ECMD_TARGET_NOT_CONFIGURED) {
      printed = "deconfig - Error occured performing ecmdDeconfigureTarget on ";
      printed += ecmdWriteTarget(target) + ". Target is not available in the system.\n";
      ecmdOutputError( printed.c_str() );
      coeRc = rc;
      continue;
    }
    else if (rc) {
      printed = "deconfig - Error occured performing ecmdDeconfigureTarget on ";
      printed += ecmdWriteTarget(target) + "\n";
      ecmdOutputError( printed.c_str() );
      coeRc = rc;
      return rc;
    }
    if (!ecmdGetGlobalVar(ECMD_GLOBALVAR_QUIETMODE)) {
      printed = ecmdWriteTarget(target) + "deconfigured.\n";
      ecmdOutput( printed.c_str() );
    }
  }

  return rc;
}

uint32_t ecmdReconfigUser(int argc, char * argv[]) {
  uint32_t rc = ECMD_SUCCESS, coeRc = ECMD_SUCCESS;

  ecmdChipTarget target;        ///< Current target
  ecmdLooperData looperData;    ///< Store internal Looper data
  std::string printed;          ///< Print Buffer
  int CAGE = 1, NODE = 2, SLOT = 3, POS = 4, CHIPUNIT = 5;
  int depth = 0;                 ///< depth found from Command line parms
  bool validPosFound = false;   ///< Did we find something to actually execute on ?

  /************************************************************************/
  /* Parse Common Cmdline Args                                            */
  /************************************************************************/
  if (ecmdParseOption(&argc, &argv, "-dk"))             depth = CAGE;
  else if (ecmdParseOption(&argc, &argv, "-dn"))        depth = NODE;
  else if (ecmdParseOption(&argc, &argv, "-ds"))        depth = SLOT;
  else if (ecmdParseOption(&argc, &argv, "-dp"))        depth = POS;
  else if (ecmdParseOption(&argc, &argv, "-dc"))        depth = CHIPUNIT;

  /************************************************************************/
  /* Parse Common Cmdline Args                                            */
  /************************************************************************/
  rc = ecmdCommandArgs(&argc, &argv);
  if (rc) return rc;

  /* Global args have been parsed, we can read if -coe was given */
  bool coeMode = ecmdGetGlobalVar(ECMD_GLOBALVAR_COEMODE); ///< Are we in continue on error mode

  //Setup the target that will be used to query the system config
  if (argc > 1) {
    ecmdOutputError("reconfig - Too many arguments specified; you probably added an unsupported option.\n");
    ecmdOutputError("reconfig - Type 'reconfig -h' for usage.\n");
    return ECMD_INVALID_ARGS;
  } else if (argc == 1) {
    std::string chipType, chipUnitType;
    rc = ecmdParseChipField(argv[0], chipType, chipUnitType, true /* supports wildcard usage */);
    if (rc) { 
      ecmdOutputError("reconfig - Wildcard character detected however it is not being used correctly.\n");
      return rc;
    }

    /* Error check */
    if (depth) {
      if (chipUnitType == "" && depth < POS) {
        ecmdOutputError("reconfig - Invalid Depth parm specified when a chip was specified.  Try with -dp.\n");
        return ECMD_INVALID_ARGS;
      }

      if (chipUnitType != "" && depth < CHIPUNIT) {
        ecmdOutputError("reconfig - Invalid Depth parm specified when a chipUnit was specified.  Try with -dc.\n");
        return ECMD_INVALID_ARGS;
      }
    } else { /* No depth, set on for the code below */
      if (chipUnitType == "") {
        depth = POS;
      } else {
        depth = CHIPUNIT;
      }
    }
    
    if (chipType == "x") {
      target.chipTypeState = ECMD_TARGET_FIELD_WILDCARD;
    } else {
      target.chipType = chipType;
      target.chipTypeState = ECMD_TARGET_FIELD_VALID;
    }

    target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;
    if (chipUnitType != "") {
      target.chipUnitType = chipUnitType;
      target.chipUnitTypeState = ECMD_TARGET_FIELD_VALID;
    }
  } else {
    if (depth == 0) {
      depth = CAGE;
    }
    target.chipTypeState = ECMD_TARGET_FIELD_UNUSED;
    target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;
  }

  /* Now set our states based on depth */
  target.cageState = target.nodeState = target.slotState = target.posState = target.chipUnitNumState = ECMD_TARGET_FIELD_WILDCARD;
  target.threadState = ECMD_TARGET_FIELD_UNUSED;
  if (depth == POS) {
    target.chipUnitNumState = ECMD_TARGET_FIELD_UNUSED;
  } else if (depth == SLOT) {
    target.chipTypeState = ECMD_TARGET_FIELD_UNUSED;
  } else if (depth == NODE) {
    target.slotState = ECMD_TARGET_FIELD_UNUSED;
  } else if (depth == CAGE) {
    target.nodeState = ECMD_TARGET_FIELD_UNUSED;
  }
  
  /************************************************************************/
  /* Kickoff Looping Stuff                                                */
  /************************************************************************/
  rc = ecmdExistLooperInit(target, ECMD_SELECTED_TARGETS_LOOP, looperData);
  if (rc) return rc;

  while (ecmdExistLooperNext(target, looperData) && (!coeRc || coeMode)) {

    rc = ecmdConfigureTarget(target);
    if (rc == ECMD_TARGET_NOT_CONFIGURED) {
      printed = "reconfig - Error occured performing ecmdConfigureTarget on ";
      printed += ecmdWriteTarget(target) + ". Target is not available in the system.\n";
      ecmdOutputError( printed.c_str() );
      coeRc = rc;
      continue;
    }
    else if (rc) {
      printed = "reconfig - Error occured performing ecmdConfigureTarget on ";
      printed += ecmdWriteTarget(target) + "\n";
      ecmdOutputError( printed.c_str() );
      coeRc = rc;
      return rc;
    }
    else {
      validPosFound = true;
    }

    if (!ecmdGetGlobalVar(ECMD_GLOBALVAR_QUIETMODE)) {
      printed = ecmdWriteTarget(target) + "configured.\n";
      ecmdOutput( printed.c_str() );
    }
  }

  // This is an error common across all UI functions
  if (!validPosFound) {
    ecmdOutputError("reconfig - Unable to find a valid chip to execute command on\n");
    return ECMD_TARGET_NOT_CONFIGURED;
  }

  return rc;
}

#ifndef ECMD_REMOVE_FSI_FUNCTIONS
uint32_t ecmdGetGpRegisterUser(int argc, char* argv[]) {
  uint32_t rc = ECMD_SUCCESS, coeRc = ECMD_SUCCESS;

  bool expectFlag = false;
  bool maskFlag = false;
  char* expectPtr = NULL;                       ///< Pointer to expected data in arg list
  char* maskPtr = NULL;                         ///< Pointer to mask data in arg list
  ecmdDataBuffer expected;                      ///< Buffer to store expected data
  ecmdDataBuffer mask;                          ///< Buffer for mask of expected data
  std::string outputformat = "x";               ///< Output Format to display
  std::string inputformat = "x";                ///< Input format of data
  ecmdChipTarget target;                        ///< Current target being operated on
  ecmdDataBuffer buffer;                        ///< Buffer to hold gp register data
  bool validPosFound = false;                   ///< Did the looper find anything?
  ecmdLooperData looperData;            ///< Store internal Looper data
  std::string printed;                          ///< Output data

  /************************************************************************/
  /* Parse Local FLAGS here!                                              */
  /************************************************************************/
  //expect and mask flags check
  if ((expectPtr = ecmdParseOptionWithArgs(&argc, &argv, "-exp")) != NULL) {
    expectFlag = true;

    if ((maskPtr = ecmdParseOptionWithArgs(&argc, &argv, "-mask")) != NULL) {
      maskFlag = true;
    }
  }

  /* get format flag, if it's there */
  char * formatPtr = ecmdParseOptionWithArgs(&argc, &argv, "-o");
  if (formatPtr != NULL) {
    outputformat = formatPtr;
  }
  formatPtr = ecmdParseOptionWithArgs(&argc, &argv, "-i");
  if (formatPtr != NULL) {
    inputformat = formatPtr;
  }

  /************************************************************************/
  /* Parse Common Cmdline Args                                            */
  /************************************************************************/
  rc = ecmdCommandArgs(&argc, &argv);
  if (rc) return rc;

  /* Global args have been parsed, we can read if -coe was given */
  bool coeMode = ecmdGetGlobalVar(ECMD_GLOBALVAR_COEMODE); ///< Are we in continue on error mode

  /************************************************************************/
  /* Parse Local ARGS here!                                               */
  /************************************************************************/
  if (argc < 2) {  //chip + address
    ecmdOutputError("getgpreg - Too few arguments specified; you need at least a chip and a gpregister.\n");
    ecmdOutputError("getgpreg - Type 'getgpreg -h' for usage.\n");
    return ECMD_INVALID_ARGS;
  }

  //Setup the target that will be used to query the system config
  std::string chipType, chipUnitType;
  rc = ecmdParseChipField(argv[0], chipType, chipUnitType, true /* supports wildcard usage */);
  if (rc) { 
    ecmdOutputError("getgpreg - Wildcard character detected however it is not being used correctly.\n");
    return rc;
  }

  if (chipUnitType != "") {
    ecmdOutputError("getgpreg - chipUnit specified on the command line, this function doesn't support chipUnits.\n");
    return ECMD_INVALID_ARGS;
  }

  if (chipType == "x") {
    target.chipTypeState = ECMD_TARGET_FIELD_WILDCARD;
  } else {
    target.chipType = chipType;
    target.chipTypeState = ECMD_TARGET_FIELD_VALID;
  }

  target.cageState = target.nodeState = target.slotState = target.posState = ECMD_TARGET_FIELD_WILDCARD;
  target.chipUnitTypeState = target.chipUnitNumState = target.threadState = ECMD_TARGET_FIELD_UNUSED;

  //get address to fetch
  if (!ecmdIsAllDecimal(argv[1])) {
    ecmdOutputError("getgpreg - Non-decimal characters detected in reg num field\n");
    return ECMD_INVALID_ARGS;
  }
  uint32_t gpRegister = (uint32_t)atoi(argv[1]);


  if (expectFlag) {

    rc = ecmdReadDataFormatted(expected, expectPtr, inputformat);
    if (rc) {
      ecmdOutputError("getgpreg - Problems occurred parsing expected data, must be an invalid format\n");
      return rc;
    }

    if (maskFlag) {
      rc = ecmdReadDataFormatted(mask, maskPtr, inputformat);
      if (rc) {
        ecmdOutputError("getgpreg - Problems occurred parsing mask data, must be an invalid format\n");
        return rc;
      }

    }


  }
  if (argc > 2) { 
    ecmdOutputError("getgpreg - Too many arguments specified; you probably added an option that wasn't recognized.\n");
    ecmdOutputError("getgpreg - Type 'getgpreg -h' for usage.\n");
    return ECMD_INVALID_ARGS;
  }

  /************************************************************************/
  /* Kickoff Looping Stuff                                                */
  /************************************************************************/

  rc = ecmdLooperInit(target, ECMD_SELECTED_TARGETS_LOOP, looperData);
  if (rc) return rc;


  while (ecmdLooperNext(target, looperData) && (!coeRc || coeMode)) {

    rc = getGpRegister(target, gpRegister, buffer);
    if (rc) {
        printed = "getgpreg - Error occured performing getgpreg on ";
        printed += ecmdWriteTarget(target);
        printed += "\n";
        ecmdOutputError( printed.c_str() );
        coeRc = rc;
        continue;
    }
    else {
      validPosFound = true;
    }

    if (expectFlag) {

      if (maskFlag) {
        buffer.setAnd(mask, 0, buffer.getBitLength());
      }

      uint32_t mismatch = ECMD_UNSET;
      if (!ecmdCheckExpected(buffer, expected, mismatch)) {

        //@ make this stuff sprintf'd
        char outstr[75];
        printed = ecmdWriteTarget(target);
        sprintf(outstr, "\ngetgpreg - Data miscompare occured at address: %d\n", gpRegister);
        printed += outstr;
        ecmdOutputError( printed.c_str() );

        printed = "getgpreg - Actual";
        if (maskFlag) {
          printed += " (with mask): ";
        }
        else {
          printed += "            : ";
        }

        printed += ecmdWriteDataFormatted(buffer, outputformat);
        ecmdOutputError( printed.c_str() );

        printed = "getgpreg - Expected          : ";
        printed += ecmdWriteDataFormatted(expected, outputformat);
        ecmdOutputError( printed.c_str() );
        coeRc = ECMD_EXPECT_FAILURE;
        continue;
      }

    }
    else {
      printed = ecmdWriteTarget(target);
      printed += ecmdWriteDataFormatted(buffer, outputformat);
      ecmdOutput( printed.c_str() );
    }
  }
  // coeRc will be the return code from in the loop, coe mode or not.
  if (coeRc) return coeRc;

  // This is an error common across all UI functions
  if (!validPosFound) {
    ecmdOutputError("getgpreg - Unable to find a valid chip to execute command on\n");
    return ECMD_TARGET_NOT_CONFIGURED;
  }

  return rc;
}

uint32_t ecmdPutGpRegisterUser(int argc, char* argv[]) {

  uint32_t rc = ECMD_SUCCESS, coeRc = ECMD_SUCCESS;
  std::string inputformat = "x";                ///< Default input format
  std::string dataModifier = "insert";          ///< Default data Modifier (And/Or/insert)
  ecmdDataBuffer fetchBuffer;                   ///< Buffer to store read/modify/write data
  ecmdLooperData looperData;                    ///< Store internal Looper data
  ecmdChipTarget target;                        ///< Chip target being operated on
  ecmdDataBuffer buffer;                        ///< Container to store write data
  bool validPosFound = false;                   ///< Did the config looper actually find a chip ?
  std::string printed;                          ///< String for printed data
  uint32_t startbit = ECMD_UNSET;               ///< Startbit to insert data
  uint32_t numbits = 0;                         ///< Number of bits to insert data

  /************************************************************************/
  /* Parse Local FLAGS here!                                              */
  /************************************************************************/
  char* formatPtr = ecmdParseOptionWithArgs(&argc, &argv, "-i");
  if (formatPtr != NULL) {
    inputformat = formatPtr;
  }

  formatPtr = ecmdParseOptionWithArgs(&argc, &argv, "-b");
  if (formatPtr != NULL) {
    dataModifier = formatPtr;
  }

  /************************************************************************/
  /* Parse Common Cmdline Args,                                           */
  /************************************************************************/
  rc = ecmdCommandArgs(&argc, &argv);
  if (rc) return rc;

  /* Global args have been parsed, we can read if -coe was given */
  bool coeMode = ecmdGetGlobalVar(ECMD_GLOBALVAR_COEMODE); ///< Are we in continue on error mode

  /************************************************************************/
  /* Parse Local ARGS here!                                               */
  /************************************************************************/

  if (argc < 3) {  //chip + gpRegister + some data
    ecmdOutputError("putgpreg - Too few arguments specified; you need at least a chip, reg num, and some data.\n");
    ecmdOutputError("putgpreg - Type 'putgpreg -h' for usage.\n");
    return ECMD_INVALID_ARGS;
  }

  //Setup the target that will be used to query the system config
  std::string chipType, chipUnitType;
  rc = ecmdParseChipField(argv[0], chipType, chipUnitType, true /* supports wildcard usage */);
  if (rc) { 
    ecmdOutputError("putgpreg - Wildcard character detected however it is not being used correctly.\n");
    return rc;
  }

  if (chipUnitType != "") {
    ecmdOutputError("putgpreg - chipUnit specified on the command line, this function doesn't support chipUnits.\n");
    return ECMD_INVALID_ARGS;
  }
  if (chipType == "x") {
    target.chipTypeState = ECMD_TARGET_FIELD_WILDCARD;
  } else {
    target.chipType = chipType;
    target.chipTypeState = ECMD_TARGET_FIELD_VALID;
  }




  target.cageState = target.nodeState = target.slotState = target.posState = ECMD_TARGET_FIELD_WILDCARD;
  target.chipUnitTypeState = target.chipUnitNumState = target.threadState = ECMD_TARGET_FIELD_UNUSED;

  if (!ecmdIsAllDecimal(argv[1])) {
    ecmdOutputError("putgpreg - Non-decimal characters detected in reg num field\n");
    return ECMD_INVALID_ARGS;
  }
  uint32_t gpRegister = (uint32_t)atoi(argv[1]);

  /* Did they specify a start/numbits */
  if (argc > 3) {
    if (argc != 5) {
      ecmdOutputError("putgpreg - Too many arguments specified; you probably added an unsupported option.\n");

      ecmdOutputError("putgpreg - Type 'putgpreg -h' for usage.\n");
      return ECMD_INVALID_ARGS;
    }

    if (!ecmdIsAllDecimal(argv[2])) {
      ecmdOutputError("putgpreg - Non-decimal characters detected in startbit field\n");
      return ECMD_INVALID_ARGS;
    }
    startbit = (uint32_t)atoi(argv[2]);
    if (!ecmdIsAllDecimal(argv[3])) {
      ecmdOutputError("putgpreg - Non-decimal characters detected in numbits field\n");
      return ECMD_INVALID_ARGS;
    }
    numbits = (uint32_t)atoi(argv[3]);


    /* Bounds check */
    if ((startbit + numbits) > ECMD_MAX_DATA_BITS) {
      char errbuf[100];
      sprintf(errbuf,"putgpreg - Too much data requested > %d bits\n", ECMD_MAX_DATA_BITS);
      ecmdOutputError(errbuf);
      return ECMD_DATA_BOUNDS_OVERFLOW;
    } else if (numbits == 0) {
      ecmdOutputError("putgpreg - Number of bits == 0, operation not performed\n");
      return ECMD_INVALID_ARGS;
    }

    rc = ecmdReadDataFormatted(buffer, argv[4], inputformat, (int)numbits);
    if (rc) {
      ecmdOutputError("putgpreg - Problems occurred parsing input data, must be an invalid format\n");
      return rc;
    }


  } else {

    rc = ecmdReadDataFormatted(buffer, argv[2], inputformat);
    if (rc) {
      ecmdOutputError("putgpreg - Problems occurred parsing input data, must be an invalid format\n");
      return rc;
    }
  }

  /************************************************************************/
  /* Kickoff Looping Stuff                                                */
  /************************************************************************/

  rc = ecmdLooperInit(target, ECMD_SELECTED_TARGETS_LOOP, looperData);
  if (rc) return rc;

  while (ecmdLooperNext(target, looperData) && (!coeRc || coeMode)) {

    /* Do we need to perform a read/modify/write op ? */
    if ((dataModifier != "insert") || (startbit != ECMD_UNSET)) {

      rc = getGpRegister(target, gpRegister, fetchBuffer);
      if (rc) {
        printed = "putgpreg - Error occured performing putgpreg on ";
        printed += ecmdWriteTarget(target);
        printed += "\n";
        ecmdOutputError( printed.c_str() );
        coeRc = rc;
        continue;
      }
      else {
        validPosFound = true;
      }

      rc = ecmdApplyDataModifier(fetchBuffer, buffer, (startbit == ECMD_UNSET ? 0 : startbit), dataModifier);
      if (rc) break;

      rc = putGpRegister(target, gpRegister, fetchBuffer);
      if (rc) {
        printed = "putgpreg - Error occured performing putgpreg on ";
        printed += ecmdWriteTarget(target);
        printed += "\n";
        ecmdOutputError( printed.c_str() );
        coeRc = rc;
        continue;
      }

    }
    else {

      rc = putGpRegister(target, gpRegister, buffer);
      if (rc) {
        printed = "putgpreg - Error occured performing putgpreg on ";
        printed += ecmdWriteTarget(target);
        printed += "\n";
        ecmdOutputError( printed.c_str() );
        coeRc = rc;
        continue;
      }
      else {
        validPosFound = true;
      }

    }

    if (!ecmdGetGlobalVar(ECMD_GLOBALVAR_QUIETMODE)) {
      printed = ecmdWriteTarget(target) + "\n";
      ecmdOutput(printed.c_str());
    }
  }
  // coeRc will be the return code from in the loop, coe mode or not.
  if (coeRc) return coeRc;

  // This is an error common across all UI functions
  if (!validPosFound) {
    ecmdOutputError("putgpreg - Unable to find a valid chip to execute command on\n");
    return ECMD_TARGET_NOT_CONFIGURED;
  }

  return rc;
}
#endif // ECMD_REMOVE_FSI_FUNCTIONS

uint32_t ecmdEchoUser(int argc, char * argv[]) {

  uint32_t rc = ECMD_SUCCESS;
  std::string message;

  bool warning = false;
  bool error = false;

  warning = ecmdParseOption(&argc, &argv, "-warning");
  error = ecmdParseOption(&argc, &argv, "-error");

  /************************************************************************/
  /* Parse Common Cmdline Args                                            */
  /************************************************************************/
  rc = ecmdCommandArgs(&argc, &argv);
  if (rc) return rc;

  if (argc < 1) {
    ecmdOutputError("ecmdecho - At least one argument (a message to print) is required for ecmdecho.\n");
    return ECMD_INVALID_ARGS;
  }
  for (int idx = 0; idx < argc; idx ++) {
    message += argv[idx];
    message += " ";
  }
  message += "\n";

  if (error) {
    ecmdOutputError(message.c_str());
  } else if (warning) {
    ecmdOutputWarning(message.c_str());
  } else {
    ecmdOutput(message.c_str());
  }

  return rc;

}

uint32_t ecmdDelayUser(int argc, char * argv[]) {

  uint32_t rc = ECMD_SUCCESS;
  uint32_t msDelay = 0;      //holds ms delay
  uint32_t simCycles = 0;    //holds the sim delay cycles
  uint32_t delayType = 0;       //temp holder for sim cycles
  std::string temp   = "";     
  uint32_t delayValue = 0;
  size_t strpos;

  if (argc != 1) {
     ecmdOutputError("ecmddelay - only one Delay type supported.\n");
     ecmdOutputError("ecmddelay - Type 'ecmddelay -h' for usage.\n");
     return ECMD_INVALID_ARGS;
  }

  temp = argv[0];
  transform(temp.begin(), temp.end(), temp.begin(), (int(*)(int)) tolower);
  if ((strpos = temp.find("cycles")) != std::string::npos)  {
     delayType = 1;
  } else if ((strpos = temp.find("ms")) != std::string::npos)  {
     delayType = 2;
  } else if ((strpos = temp.find("s")) != std::string::npos)  {
     delayType = 3;
  }else{
     ecmdOutputError("ecmddelay - Delay type is not valid.\n");
     ecmdOutputError("ecmddelay - Type 'ecmddelay -h' for usage.\n");
     return ECMD_INVALID_ARGS;
  }
  // strpos points to cycles/ms/s after if/else
  temp.erase(strpos, temp.length()-strpos);

  if (!ecmdIsAllDecimal(temp.c_str())) {
    ecmdOutputError("ecmddelay - Non-Decimal characters detected in Delay field\n");
    return ECMD_INVALID_ARGS;
  }

  delayValue= (uint32_t)atoi(temp.c_str());
  if(delayType == 1)
    simCycles = delayValue;
  else if(delayType == 2)
    msDelay = delayValue;
  else if(delayType == 3)
    msDelay = delayValue*1000;

  rc = ecmdDelay(simCycles, msDelay);
  if (rc != ECMD_SUCCESS)
     ecmdOutputError("ecmddelay - Error returned from ecmdDelay()\n");
 
  return rc;

}

#ifndef ECMD_REMOVE_UNITID_FUNCTIONS
uint32_t ecmdUnitIdUser(int argc, char* argv[]) {
    
  uint32_t rc = ECMD_SUCCESS;
  std::string output;
  char buf[1000];
  ecmdChipTarget target;

  if (argc < 1) {
    ecmdOutputError("unitid - Too few arguments specified,Type 'unitid -h' for help.\n");
    return ECMD_INVALID_ARGS;
  }

  /************************************************************************/
  /* Parse Common Cmdline Args                                            */
  /************************************************************************/

  //Parsing the CmdLine for uid2tgt
  if (!strcmp(argv[0], "uid2tgt")){

    if(argv[0] != NULL) {
        rc = ecmdCommandArgs(&argc, &argv);
        if (rc) return rc;
    }

    if (argc < 2) {
        ecmdOutputError("unitid - Too few arguments specified for 'uid2tgt'. You need a unitid in hex or string format\n");
        ecmdOutputError("unitid - Type 'unitid -h' for usage.\n");
        return ECMD_INVALID_ARGS;
    }

    if (argc > 2) {
        ecmdOutputError("unitid - Too many arguments specified for 'uid2tgt'.  You probably added an unsupported option\n");
        ecmdOutputError("unitid - Type 'unitid -h' for usage.\n");
        return ECMD_INVALID_ARGS;
    }


  //define the target list,which will be obtained when the unitid will get processed

    std::list<ecmdChipTarget> target_list;
    std::list<ecmdChipTarget>::iterator target_list_iterator;

    rc = ecmdUnitIdStringToTarget(argv[1], target_list);
    if (rc){
        sprintf(buf,"ecmdUnitIdStringToTarget Failed with rc = 0x%x and unitId = %s\n",rc,argv[1]);
        output = (std::string)buf;
        ecmdOutputError(output.c_str());
        return rc;
    }
    if(target_list.size()==1){

        target_list_iterator = target_list.begin();
        sprintf(buf,"For Inputs Id = %s, Target is '%s' \n", argv[1],(ecmdWriteTarget(*target_list_iterator)).c_str());

        output = (std::string)buf;
        ecmdOutput(output.c_str());
    }else{

        output="This is a Group Id Input ..... Multiple Targets will Return For Input Id: ";
        output+=argv[1];
        output+="\n";
        ecmdOutput(output.c_str());
        uint32_t output_count = 0;
        for(target_list_iterator = target_list.begin();target_list_iterator != target_list.end();target_list_iterator++){
            sprintf(buf,"%s",(ecmdWriteTarget(*target_list_iterator)).c_str());
            output = (std::string)buf;

            // 2 tgts printed per line 
            if ((output_count+1)%2) output += ",\t";
            else  output += ",\n";
            output_count++;

            ecmdOutput(output.c_str());
        }
    }
  //Parsing the CmdLine for tgt2uid 
  }else if(!strcmp(argv[0], "tgt2uid")){

  /*This code is based on 'ecmdquery configd' such that we loop through all the
    possible tgts that the user asked for and determine the unitid.*/

    std::string l_chipType, l_chipUnitType;

    std::string cage;
    std::string node;
    std::string slot;
    std::string pos;
    std::string core;
    std::string thread;

    uint8_t cageType;
    uint8_t nodeType;
    uint8_t slotType;
    uint8_t posType;
    uint8_t coreType;
    uint8_t threadType;

    /************************************************************************/
    /* Parse Common Cmdline Args                                            */
    /************************************************************************/

    //Set all the states Unused to begin with. Then set them to valid based on the args
    target.cageState     = ECMD_TARGET_FIELD_UNUSED;
    target.nodeState     = ECMD_TARGET_FIELD_UNUSED;
    target.slotState     = ECMD_TARGET_FIELD_UNUSED;
    target.chipTypeState = ECMD_TARGET_FIELD_UNUSED;
    target.posState      = ECMD_TARGET_FIELD_UNUSED;
    target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;
    target.chipUnitNumState  = ECMD_TARGET_FIELD_UNUSED;
    target.threadState   = ECMD_TARGET_FIELD_UNUSED;

    rc = ecmdParseTargetFields(&argc, &argv, "cage", target, cageType, cage);
    if(rc) return rc;
    rc = ecmdParseTargetFields(&argc, &argv, "node", target, nodeType, node);
    if(rc) return rc;
    rc = ecmdParseTargetFields(&argc, &argv, "slot", target, slotType, slot);
    if(rc) return rc;
    rc = ecmdParseTargetFields(&argc, &argv, "pos", target, posType, pos);
    if(rc) return rc;
    rc = ecmdParseTargetFields(&argc, &argv, "core", target, coreType, core);
    if(rc) return rc;
    rc = ecmdParseTargetFields(&argc, &argv, "thread", target, threadType, thread);
    if(rc) return rc;


    //checking the depth of the unitid
    if (argc < 2) {
        ecmdOutputError("unitid - Too few arguments specified for 'tgt2uid'. You need at least a depth'.\n");
        ecmdOutputError("unitid - Type 'unitId -h' for usage.\n");
        return ECMD_INVALID_ARGS;
    }
    else {

      if(!strcmp(argv[1], "-dk")) rc = ecmdSetTargetDepth(target, ECMD_DEPTH_CAGE);
      else if(!strcmp(argv[1], "-dn")) rc = ecmdSetTargetDepth(target, ECMD_DEPTH_NODE);
      else if(!strcmp(argv[1], "-ds")) rc = ecmdSetTargetDepth(target, ECMD_DEPTH_SLOT);
      else if(!strcmp(argv[1], "-dp")) rc = ecmdSetTargetDepth(target, ECMD_DEPTH_CHIP);
      else if(!strcmp(argv[1], "-dc")) rc = ecmdSetTargetDepth(target, ECMD_DEPTH_CHIPUNIT);
      else if(!strcmp(argv[1], "-dt")) rc = ecmdSetTargetDepth(target, ECMD_DEPTH_THREAD);
      else {
        // unknow/unsupported depth input
        ecmdOutputError("unitid - Unknown Entry for depth. You need one of these: -dk,-dn,-ds,-dp,-dc,-dt.\n");
        ecmdOutputError("unitid - Type 'unitId -h' for usage.\n");
        return ECMD_INVALID_ARGS;
      }

      if (rc != ECMD_SUCCESS) {
      // Fail setting depth
        ecmdOutputError("unitid - Fail Setting Target Depth\n");
        return ECMD_INVALID_ARGS;
      }
    }

    // if depth is pos, chipUnit, or Thread, we need ChipType
    if ( (!strcmp(argv[1], "-dp")) || (!strcmp(argv[1], "-dc")) || (!strcmp(argv[1], "-dt"))) {

      if (argc < 3) {
        ecmdOutputError("unitid - Too few arguments specified for 'tgt2uid'. For this depth you need at least a chipType.\n");
        ecmdOutputError("unitid - Type 'unitId -h' for usage.\n");
        return ECMD_INVALID_ARGS;
      }
      else {
        rc = ecmdParseChipField(argv[2], l_chipType, l_chipUnitType);
    
        if (rc != ECMD_SUCCESS) {
          // Fail getting chipType, chipunitType
          ecmdOutputError("unitid - Fail Getting chipType, chipUnitType\n");
          return ECMD_INVALID_ARGS;
        }

        target.chipType=l_chipType; // we know we're at least at chip depth
    
        if ( ( (!strcmp(argv[1], "-dc")) || (!strcmp(argv[1], "-dt"))) &&
             ( l_chipUnitType.size() != 0)    ) {
          // at chipUnit or Thread depth we set chipUnitType if user gave usone
          target.chipUnitType=l_chipUnitType; // we know we're at least at chip depth
        }
        else {
          // disable chipUnitType
          target.chipUnitTypeState=ECMD_TARGET_FIELD_UNUSED;
        }
    
      }
    
    }
    
    rc = ecmdTargetToUnitId(target);
    if (rc) { 
      return rc;

    }
    else {           
      sprintf(buf,"For target = %s, unitId is '0x%X'\n",(ecmdWriteTarget(target)).c_str(),target.unitId);
      ecmdOutput(buf);
    }

  } else if(!strcmp(argv[0], "getversion")) {

      if (argc >1 ) {
        ecmdOutputError("unitid - Too many arguments specified for 'getversion'.You  only need 'getversion'\n");
        ecmdOutputError("unitid - Type 'unitId -h' for usage.\n");
        return ECMD_INVALID_ARGS;
      }
      else {
        uint32_t o_uidVersion=0;
        rc = ecmdGetUnitIdVersion(o_uidVersion);
        if (rc != ECMD_SUCCESS) {
          char errBuf[128];
          sprintf(errBuf,"unitid getversion-Failed,rc: 0x%x\n",rc);
          ecmdOutputError(errBuf);
          return rc;
        }
        else{
          char resBuf[128];
          sprintf(resBuf,"unitIdVersion = 0x%x\n",o_uidVersion);
          ecmdOutput(resBuf);

        }
     }

  } else {
    ecmdOutputError("unitid- Invalid arguments-Type 'unitid -h' for correct arguments\n");
    return ECMD_INVALID_ARGS;
  }
  
  return rc; 
}
#endif // ECMD_REMOVE_UNITID_FUNCTIONS

#ifndef ECMD_REMOVE_SENSOR_FUNCTIONS
uint32_t ecmdGetSensorUser(int argc, char* argv[]) 
{

  uint32_t rc = ECMD_SUCCESS, coeRc = ECMD_SUCCESS;
  
  std::string printed;                  //< Output data
  ecmdChipTarget target;
  ecmdLooperData looperdata;            //< Store internal Looper data
  bool validPosFound = false;           //< Did we find a valid chip in the looper
  std::string outputformat = "d";       //< Display format for output
  bool headerPrinted = false;           //< Print the header if it is not already printed
  /************************************************************************/
  /* Parse Common Cmdline Args                                            */
  /************************************************************************/
  rc = ecmdCommandArgs(&argc, &argv);
  if (rc) return rc;

  /* Global args have been parsed, we can read if -coe was given */
  bool coeMode = ecmdGetGlobalVar(ECMD_GLOBALVAR_COEMODE); ///< Are we in continue on error mode

  /************************************************************************/
  /* Parse Local ARGS here!                                               */
  /************************************************************************/
  if (argc != 5) {
    ecmdOutputError("getsensor - Too few arguments, Type 'getsensor -h' for usage.\n");
    return ECMD_INVALID_ARGS;
  }

  //Read the sensorType and its unit
  std::string sensorType = argv[1];
  std::string unit = argv[4];

  transform(sensorType.begin(), sensorType.end(), sensorType.begin(), (int(*)(int)) tolower);
  transform(unit.begin(), unit.end(), unit.begin(), (int(*)(int)) tolower);

  //Read sensorId
  std::string sensorId = argv[2];
  transform(sensorId.begin(), sensorId.end(), sensorId.begin(), (int(*)(int)) tolower);

  //Read readMode string
  ecmdSensorReadMode_t readMode =  ECMD_SENSOR_READ_MODE_DEFAULT;
  std::string modeStr = argv[3];
  transform(modeStr.begin(), modeStr.end(), modeStr.begin(), (int(*)(int)) tolower);

  if (modeStr == "1ms"){
    readMode = ECMD_SENSOR_READ_MODE_1ms;
  }else if(modeStr == "8ms"){
    readMode = ECMD_SENSOR_READ_MODE_8ms;
  }else if(modeStr == "32ms"){
    readMode = ECMD_SENSOR_READ_MODE_32ms;
  }else if(modeStr == "1s"){
    readMode = ECMD_SENSOR_READ_MODE_1s;
  }else if(modeStr == "8s"){
    readMode = ECMD_SENSOR_READ_MODE_8s;
  }else if(modeStr == "default"){              //allow default 
    readMode = ECMD_SENSOR_READ_MODE_DEFAULT;
  }else if(modeStr == "def"){
    readMode = ECMD_SENSOR_READ_MODE_DEFAULT; //allow def
  }else if(modeStr == "min"){
    readMode = ECMD_SENSOR_READ_MODE_MIN; //allow def
  }else if(modeStr == "max"){
    readMode = ECMD_SENSOR_READ_MODE_MAX; //allow def
  }else {
    ecmdOutputError("getsensor - Invalid sensor readMode. Type 'getsensor -h' for usage.\n");
    return ECMD_INVALID_ARGS;
  }

  //setup target
  std::string chipType, chipUnitType;
  rc = ecmdParseChipField(argv[0],chipType,chipUnitType);
  if (rc) { 
    ecmdOutputError("getsensor - Wildcard character detected however it is not supported by this command.\n");
    return rc;
  }

  target.chipType = chipType;
  if (target.chipType == "nochip") {
    target.chipTypeState = ECMD_TARGET_FIELD_UNUSED;
    target.posState = ECMD_TARGET_FIELD_UNUSED;
  } else {
  target.chipTypeState = ECMD_TARGET_FIELD_VALID;
    target.posState = ECMD_TARGET_FIELD_WILDCARD;
  }
  target.cageState = target.nodeState = target.slotState =  ECMD_TARGET_FIELD_WILDCARD;

  //if chipUnitType was passed, set a chipUnit target
  if(chipUnitType != "")
  {
    target.chipUnitType = chipUnitType;
    target.chipUnitTypeState =  ECMD_TARGET_FIELD_VALID;
    target.chipUnitNumState  =  ECMD_TARGET_FIELD_WILDCARD;
    target.threadState = ECMD_TARGET_FIELD_UNUSED;
  }
  else
  {
    target.chipUnitTypeState = target.chipUnitNumState = target.threadState = ECMD_TARGET_FIELD_UNUSED;
  }
  uint32_t o_data = 0; //the output return value
  ecmdDTSData o_dtsReadings; //the output return value in case of dts

  /************************************************************************/
  /* Kickoff Looping Stuff                                                */
  /************************************************************************/
  rc = ecmdLooperInit(target, ECMD_SELECTED_TARGETS_LOOP, looperdata);
  if (rc) return rc;

  while (ecmdLooperNext(target, looperdata) && (!coeRc || coeMode)) {

    if (sensorType == "thermal") 
    {
      ecmdThermalUnit_t sensorUnit; //thermal unit
      if(unit == "dc") 
      {
        //deci degree Celcius
        sensorUnit = ECMD_THERMAL_UNIT_dC;
      }
      else if(unit == "c") 
      {
        //degree celcius
        sensorUnit = ECMD_THERMAL_UNIT_C;
      }
      else 
      {
        ecmdOutputError("getsensor - Invalid unit for thermal sensor. Type 'getsensor -h' for usage.\n");
        return ECMD_INVALID_ARGS;
      }
      //get thermal sensor
      rc =  ecmdGetThermalSensor(target,sensorId.c_str(),o_data,sensorUnit, readMode);
    }

    else if (sensorType == "power")
    {
      ecmdPowerUnit_t sensorUnit;
      if(unit == "uw") 
      {
        //microWatt
        sensorUnit = ECMD_POWER_UNIT_uW;
      }
      else if(unit == "mw")
      {
        //milliWatt
        sensorUnit = ECMD_POWER_UNIT_mW;
      }
      else if(unit == "dw") 
      {
        //deciWatt
        sensorUnit = ECMD_POWER_UNIT_dW;
      }
      else if(unit == "w")
      {
        //Watt
        sensorUnit = ECMD_POWER_UNIT_W;
      }
      else
      {
        ecmdOutputError("getsensor - Invalid unit for power sensor. Type 'getsensor -h' for usage.\n");
        return ECMD_INVALID_ARGS;
      }
      //get power sensor
      rc =  ecmdGetPowerSensor(target,sensorId.c_str(),o_data,sensorUnit, readMode);
    }

    else if (sensorType == "voltage") 
    {
      ecmdVoltageUnit_t sensorUnit;
      if(unit == "mv")
      {
        //milliVolt
        sensorUnit = ECMD_VOLTAGE_UNIT_mV;
      }
      else if(unit == "v")
      {
        //Volt
        sensorUnit = ECMD_VOLTAGE_UNIT_V;
      }
      else
      {
        ecmdOutputError("getsensor - Invalid unit for voltage sensor. Type 'getsensor -h' for usage.\n");
        return ECMD_INVALID_ARGS;
      }
      //get voltage sensor
      rc =  ecmdGetVoltageSensor(target,sensorId.c_str(),o_data,sensorUnit, readMode);
    }

    else if (sensorType == "current") 
    {
      ecmdCurrentUnit_t sensorUnit;
      if(unit == "ma") 
      {
        //milliAmpere
        sensorUnit = ECMD_CURRENT_UNIT_mA; 
      }
      else if(unit == "a") 
      {
        //Ampere
        sensorUnit = ECMD_CURRENT_UNIT_A;
      }
      else { 
        ecmdOutputError("getsensor - Invalid unit for current sensor. Type 'getsensor -h' for usage.\n");
        return ECMD_INVALID_ARGS;
      }
      //get current sensor
      rc =  ecmdGetCurrentSensor(target,sensorId.c_str(),o_data,sensorUnit, readMode);
    }

    else if (sensorType == "humidity") 
    {
      ecmdHumidityUnit_t sensorUnit;
      if(unit == "gm3")
      {
        //gram per cubic meter
        sensorUnit = ECMD_HUMIDITY_UNIT_G_M3;
      }
      else if(unit == "p")
      {
        //relative percentage
        sensorUnit = ECMD_HUMIDITY_UNIT_REL_PERCENTAGE;
      }
      else if(unit == "dp")
      {
        //deci relative percentage
        sensorUnit = ECMD_HUMIDITY_UNIT_dREL_PERCENTAGE;
      }
      else { 
        ecmdOutputError("getsensor - Invalid unit for humidity sensor. Type 'getsensor -h' for usage.\n");
        return ECMD_INVALID_ARGS;
      }
      //get humidity sensor
      rc =  ecmdGetHumiditySensor(target,sensorId.c_str(),o_data,sensorUnit, readMode);
    }

    else if (sensorType == "airdensity") 
    {
      ecmdAirDensityUnit_t sensorUnit;
      if(unit == "kgm3")
      {
        //kilogram per cubic meter
        sensorUnit = ECMD_AIRDENSITY_UNIT_KG_M3;
      }
      else if(unit == "hpa")
      {
        //hekto pascal
        sensorUnit = ECMD_AIRDENSITY_UNIT_H_PA;
      }
      else if(unit == "dhpa")
      {
        //deci hekto pascal
        sensorUnit = ECMD_AIRDENSITY_UNIT_dH_PA;
      }
      else 
      { 
        ecmdOutputError("getsensor - Invalid unit for airdensity sensor. Type 'getsensor -h' for usage.\n");
        return ECMD_INVALID_ARGS;
      }
      rc =  ecmdGetAirDensitySensor(target,sensorId.c_str(),o_data,sensorUnit, readMode);
    }

    else if (sensorType == "utilization") 
    {
      ecmdUtilizationUnit_t sensorUnit;
      if(unit == "dp")
      {
        //deci Percentage
        sensorUnit = ECMD_UTILIZATION_UNIT_dP;
      }
      else if(unit == "p")
      {
        //Percentage
        sensorUnit = ECMD_UTILIZATION_UNIT_P;
      }
      else 
      { 
        ecmdOutputError("getsensor - Invalid unit for utilization sensor. Type 'getsensor -h' for usage.\n");
        return ECMD_INVALID_ARGS;
      }
      //get utilization sensor
      rc =  ecmdGetUtilizationSensor(target,sensorId.c_str(),o_data,sensorUnit, readMode);
    }

    else if (sensorType == "clockspeed") 
    {
      ecmdClockSpeedType_t sensorUnit;
      if(unit == "mhz") 
      {
        //megahertz
        sensorUnit = ECMD_CLOCK_FREQUENCY_MHZ_SPEC;
      }
      else if(unit == "khz")
      {
        //kilhertz
        sensorUnit = ECMD_CLOCK_FREQUENCY_KHZ_SPEC;
      }
      else if(unit == "ps")
      {
        //picosecond cycle time
        sensorUnit = ECMD_CLOCK_CYCLETIME_PS_SPEC;
      }
      else 
      {
        ecmdOutputError("getsensor - Invalid unit for clockspeed sensor. Type 'getsensor -h' for usage.\n");
        return ECMD_INVALID_ARGS;
      }
      //get clockspeed sensor
      rc =  ecmdGetClockSpeedSensor(target, sensorId.c_str(), o_data, sensorUnit, readMode);
    }
    else if (sensorType == "membw")
    {
      ecmdBandwidthUnit_t sensorUnit;
      if(unit == "mrw")
      {
        //Mega Read Write
        sensorUnit = ECMD_BW_UNIT_MRW;
      }
      else
      {
        ecmdOutputError("getsensor - Invalid unit for bandwidth sensor. Type 'getsensor -h' for usage.\n");
        return ECMD_INVALID_ARGS;
      }
      //get memory bandwidth sensor
      rc =  ecmdGetBandwidthSensor(target,sensorId.c_str(),o_data,sensorUnit, readMode);
    }
    else if (sensorType == "dts")
    {
      ecmdThermalUnit_t sensorUnit; //thermal unit
      if(unit == "dc") 
      {
        //deci degree Celcius
        sensorUnit = ECMD_THERMAL_UNIT_dC;
      }
      else if(unit == "c") 
      {
        //degree celcius
        sensorUnit = ECMD_THERMAL_UNIT_C;
      }
      else 
      {
        ecmdOutputError("getsensor - Invalid unit for dts sensor. Type 'getsensor -h' for usage.\n");
        return ECMD_INVALID_ARGS;
      }
      rc =  ecmdReadDigitalThermalSensor(target,sensorId.c_str(),o_dtsReadings,sensorUnit,readMode);
    }
    else
    {
      ecmdOutputError("getsensor - Invalid sensor type. Type 'getsensor -h' for usage.\n");
      return ECMD_INVALID_ARGS;
    }

    validPosFound = true;//setting true as we did find a target to make sensor call
    
    if (rc) {
      printed = "getsensor - Error occured performing getsensor " + sensorType + " " + sensorId + " on ";
      printed += ecmdWriteTarget(target) + "\n";
      ecmdOutputError(printed.c_str());
      coeRc = rc;
    }
    else {
      printed = ecmdWriteTarget(target);
      if ( sensorType == "dts" ) {
        char buffer[200];
        char temperature[30];
        std::string tripLevelStr;
        if ( headerPrinted == false ) {
          //print the header now
          std::string header1(printed.length(),' ');
          std::string header2(printed.length(),' ');
          for ( int i = 0; i < o_dtsReadings.num_of_sensors; i++ ) {
            header1 += "====================";
            header2 += " Name  Temp (trip)  ";
          }
          header1 += "\n";
          header2 += "\n";
          ecmdOutput(header1.c_str());
          ecmdOutput(header2.c_str());
          ecmdOutput(header1.c_str());
          headerPrinted = true; //setting it to true as the header is printed now
        }
        for (int i = 0; i < o_dtsReadings.num_of_sensors; i++) {
          if ( o_dtsReadings.validity_of_sensor_reading[i] ) {
            sprintf(temperature,UINT64_DEC_FORMAT,o_dtsReadings.temp_of_sensors[i]);
            switch (  o_dtsReadings.tripLevel_of_reading[i] ) {
              case 0 : tripLevelStr = "(no trip)"; break;
              case 1 : tripLevelStr = "(warning)"; break;
              case 2 : tripLevelStr = "(critical)"; break;
              case 3 : tripLevelStr = "(fatal)"; break;
              default: tripLevelStr = "(unknown)"; break;
            }
          } else {
            //if we come here, then we need to ignore the temperature
            sprintf(temperature,"%4s","   ");
            tripLevelStr = "invalid";
          } //end if
          sprintf(buffer,"%3s: %4s %-10s",o_dtsReadings.name_of_sensors[i].c_str(), temperature, tripLevelStr.c_str());
          printed += buffer;
        } //for loop 
        printed += "\n";
      } else {
        ecmdDataBuffer buf(32);
        buf.setWord(0,o_data);
        printed += ecmdWriteDataFormatted(buf, outputformat);
      }
      ecmdOutput(printed.c_str());
    }
  } //end - configLooper 

  // coeRc will be the return code from in the loop, coe mode or not.
  if (coeRc) return coeRc;

  // This is an error common across all UI functions
  if (!validPosFound) {
    ecmdOutputError("getsensor - Unable to find a valid chip to execute command on\n");
    return ECMD_TARGET_NOT_CONFIGURED;
  }

  return rc;
}
#endif // ECMD_REMOVE_SENSOR_FUNCTIONS



//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
uint32_t ecmdSyncPluginStateUser(int argc, char * argv[]) {
  uint32_t rc = ECMD_SUCCESS, coeRc = ECMD_SUCCESS;

  ecmdChipTarget target;        ///< Current target
  ecmdLooperData looperData;    ///< Store internal Looper data
  std::string printed;          ///< Print Buffer
  int CAGE = 1, NODE = 2, SLOT = 3, POS = 4, CHIPUNIT = 5, SYSTEM = 0;
  int depth = 0;                 ///< depth found from Command line parms
  bool validPosFound = false;   ///< Did we find something to actually execute on ?

  /************************************************************************/
  /* Parse Common Cmdline Args                                            */
  /************************************************************************/
  if (ecmdParseOption(&argc, &argv, "-dk"))             depth = CAGE;
  else if (ecmdParseOption(&argc, &argv, "-dn"))        depth = NODE;
  else if (ecmdParseOption(&argc, &argv, "-ds"))        depth = SLOT;
  else if (ecmdParseOption(&argc, &argv, "-dp"))        depth = POS;
  else if (ecmdParseOption(&argc, &argv, "-dc"))        depth = CHIPUNIT;

  /************************************************************************/
  /* Parse Common Cmdline Args                                            */
  /************************************************************************/
  rc = ecmdCommandArgs(&argc, &argv);
  if (rc) return rc;

  /* Global args have been parsed, we can read if -coe was given */
  bool coeMode = ecmdGetGlobalVar(ECMD_GLOBALVAR_COEMODE); ///< Are we in continue on error mode

  //Setup the target that will be used to query the system config
  if (argc > 1) {
    ecmdOutputError("syncpluginstate - Too many arguments specified; you probably added an unsupported option.\n");
    ecmdOutputError("syncpluginstate - Type 'syncpluginstate -h' for usage.\n");
    return ECMD_INVALID_ARGS;
  } else if (argc == 1) {
    std::string chipType, chipUnitType;
    rc = ecmdParseChipField(argv[0], chipType, chipUnitType); if (rc) return rc;

    /* Error check */
    if (depth) {
      if (chipUnitType == "" && depth < POS) {
        ecmdOutputError("syncpluginstate - Invalid Depth parm specified when a chip was specified.  Try with -dp.\n");
        return ECMD_INVALID_ARGS;
      }

      if (chipUnitType != "" && depth < CHIPUNIT) {
        ecmdOutputError("syncpluginstate - Invalid Depth parm specified when a chipUnit was specified.  Try with -dc.\n");
        return ECMD_INVALID_ARGS;
      }
    } else { /* No depth, set on for the code below */
      if (chipUnitType == "") {
        depth = POS;
      } else {
        depth = CHIPUNIT;
      }
    }
    target.chipType = chipType;
    target.chipTypeState = ECMD_TARGET_FIELD_VALID;
    target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;
    if (chipUnitType != "") {
      target.chipUnitType = chipUnitType;
      target.chipUnitTypeState = ECMD_TARGET_FIELD_VALID;
    }
  } else {
    if (depth == 0) {
      depth = CAGE;
    }
    if (argc==0) {
      depth=SYSTEM;
    }
    target.chipTypeState = ECMD_TARGET_FIELD_UNUSED;
    target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;
  }

  /* Now set our states based on depth */
  target.cageState = target.nodeState = target.slotState = target.posState = target.chipUnitNumState = ECMD_TARGET_FIELD_WILDCARD;
  target.threadState = ECMD_TARGET_FIELD_UNUSED;
  if (depth == POS) {
    target.chipUnitNumState = ECMD_TARGET_FIELD_UNUSED;
  } else if (depth == SLOT) {
    target.chipTypeState = ECMD_TARGET_FIELD_UNUSED;
  } else if (depth == NODE) {
    target.slotState = ECMD_TARGET_FIELD_UNUSED;
  } else if (depth == CAGE) {
    target.nodeState = ECMD_TARGET_FIELD_UNUSED;
  } else if (depth == SYSTEM) {
    target.cageState = ECMD_TARGET_FIELD_UNUSED;
  }
  if (depth == SYSTEM) { // no looping required .. target the whole system
    rc = syncPluginState(target);
    if (rc == ECMD_TARGET_NOT_CONFIGURED) {
      printed = "syncpluginstate - Error occured performing ecmdsyncpluginstate on system target \n";
      ecmdOutputError( printed.c_str() );
      coeRc = rc;
    }
    else if (rc) {
      printed = "syncpluginstate - Error occured performing ecmdSyncPluginState on system \n ";
      ecmdOutputError( printed.c_str() );
      coeRc = rc;
    }else {
      validPosFound = true;
    }
  }
  else
  {
    /************************************************************************/
    /* Kickoff Looping Stuff                                                */
    /************************************************************************/
    rc = ecmdExistLooperInit(target, ECMD_SELECTED_TARGETS_LOOP, looperData);
    if (rc) return rc;

    while (ecmdExistLooperNext(target, looperData) && (!coeRc || coeMode)) {

      rc = syncPluginState(target);
      if (rc == ECMD_TARGET_NOT_CONFIGURED) {
        printed = "syncpluginstate - Error occured performing ecmdConfigureTarget on ";
        printed += ecmdWriteTarget(target) + ". Target is not available in the system.\n";
        ecmdOutputError( printed.c_str() );
        coeRc = rc;
        continue;
      }
      else if (rc) {
        printed = "syncpluginstateg - Error occured performing ecmdConfigureTarget on ";
        printed += ecmdWriteTarget(target) + "\n";
        ecmdOutputError( printed.c_str() );
        coeRc = rc;
        return rc;
      }
      else {
        validPosFound = true;
      }

      printed = ecmdWriteTarget(target) + "synchronized.\n";
      ecmdOutput( printed.c_str() );
    }

  }
  // This is an error common across all UI functions
  if (!validPosFound) {
    ecmdOutputError("syncpluginstate - Unable to execute command \n");
    return ECMD_TARGET_NOT_CONFIGURED;
  }

  return rc;
}

#ifndef ECMD_REMOVE_INIT_FUNCTIONS
//Synchronizes the ipl mode between fsp and
// hostboot b/w normal and debug modes
uint32_t ecmdSyncIplModeUser(int , char * []) 
{
  uint32_t rc = ECMD_SUCCESS;
  std::string printed;          //Print Buffer
  int l_unused = 0;

  rc = syncIplMode(l_unused);
  if(rc)
  {
    printed = "synciplmode - Error occured performing ipl mode synchronization\n";
    ecmdOutputError( printed.c_str() );
  }
  return rc;
}
#endif // ECMD_REMOVE_INIT_FUNCTIONS

#ifndef ECMD_REMOVE_FSI_FUNCTIONS
uint32_t ecmdGetEcidUser(int argc, char* argv[])
{
    uint32_t rc = ECMD_SUCCESS, coeRc = ECMD_SUCCESS;

    std::string outputformat = "ecid";            ///< Output Format to display
    ecmdChipTarget target;                        ///< Current target being operated on
    ecmdDataBuffer buffer;                        ///< Buffer to hold Ecid data
    bool validPosFound = false;                   ///< Did the looper find anything?
    ecmdLooperData looperData;                    ///< Store internal Looper data
    std::string printed;                          ///< Output data
    bool verbose = false;                         ///< Prints extra decoded ECID values
    std::vector<std::string> additionalInfo;      ///< Contains the verbose data

    /************************************************************************/
    /* Parse Local FLAGS here!                                              */
    /************************************************************************/
    /* check for the verbose flag */
    if (ecmdParseOption(&argc, &argv, "-verbose"))
    {
	verbose = true;
	// Output hex format by default when verbose is chosen, unless over-ridden below.
	outputformat = "x";
    }
    /* get format flag, if it's there */
    char * formatPtr = ecmdParseOptionWithArgs(&argc, &argv, "-o");
    if (formatPtr != NULL)
    {
        outputformat = formatPtr;
    }

    

    /************************************************************************/
    /* Parse Common Cmdline Args                                            */
    /************************************************************************/
    rc = ecmdCommandArgs(&argc, &argv);
    if (rc) return rc;

    /* Global args have been parsed, we can read if -coe was given */
    bool coeMode = ecmdGetGlobalVar(ECMD_GLOBALVAR_COEMODE); ///< Are we in continue on error mode

    /************************************************************************/
    /* Parse Local ARGS here!                                               */
    /************************************************************************/
    if (argc < 1)  //chip
    {
        ecmdOutputError("getecid - Too few arguments specified; you need at least a chip.\n");
        ecmdOutputError("getecid - Type 'getecid -h' for usage.\n");
        return ECMD_INVALID_ARGS;
    }

    //Setup the target that will be used to query the system config
    std::string chipType, chipUnitType;
    rc = ecmdParseChipField(argv[0], chipType, chipUnitType, true /* supports wildcard usage */);
    if (rc)
    { 
        ecmdOutputError("getecid - Wildcard character detected however it is not being used correctly.\n");
        return rc;
    }

    if (chipUnitType != "")
    {
        ecmdOutputError("getecid - chipUnit specified on the command line, this function doesn't support chipUnits.\n");
        return ECMD_INVALID_ARGS;
    }

    if (chipType == "x")
    {
        target.chipTypeState = ECMD_TARGET_FIELD_WILDCARD;
    }
    else
    {
        target.chipType = chipType;
        target.chipTypeState = ECMD_TARGET_FIELD_VALID;
    }

    target.cageState = target.nodeState = target.slotState = target.posState = ECMD_TARGET_FIELD_WILDCARD;
    target.chipUnitTypeState = target.chipUnitNumState = target.threadState = ECMD_TARGET_FIELD_UNUSED;

    if (argc > 2)
    { 
        ecmdOutputError("getecid - Too many arguments specified; you probably added an option that wasn't recognized.\n");
        ecmdOutputError("getecid - Type 'getecid -h' for usage.\n");
        return ECMD_INVALID_ARGS;
    }

    /************************************************************************/
    /* Kickoff Looping Stuff                                                */
    /************************************************************************/

    rc = ecmdLooperInit(target, ECMD_SELECTED_TARGETS_LOOP, looperData);
    if (rc) return rc;

    while (ecmdLooperNext(target, looperData) && (!coeRc || coeMode)) {

      if (verbose)
      {
	rc = getEcidVerbose(target, buffer, additionalInfo);
      }
      else
      {
        rc = getEcid(target, buffer);
      }
      if (rc) 
      {
	printed = "getecid - Error occured performing getecid on ";
	printed += ecmdWriteTarget(target);
	printed += "\n";
	ecmdOutputError( printed.c_str() );
	coeRc = rc;
	continue;
      }
      else
      {
	validPosFound = true;
      }

      printed = ecmdWriteTarget(target);
      if (outputformat == "ecid")
      {
	// output ecid format            
	uint8_t        Xloc;
	uint8_t        Yloc;           
	char           ecidString[100];
	std::string    wafer;
	
	// --- the fuseString (proc)---
	// bits   0:3  are the version, which should be all zeros to start with
	// bits   4:63 are the wafer id ( ten 6 bit fields each containing a code)
	// bits  64:71 are the chip x location (7:0)
	// bits  72:79 are the chip y location (7:0)
	// bits  80:103 are used in a different chip location algorithm
	// bits 104:111 are the ECC over the whole 112 bits
	
	rc = formatEcidString( buffer, wafer );
	if (rc) return rc;
	
	buffer.extract(&Xloc, 64, 8);
	buffer.extract(&Yloc, 72, 8);
	
	
	sprintf(ecidString, "%s_%02d_%02d\n", wafer.c_str(), Xloc, Yloc);
	printed += ecidString;
      }
      else
      {
	printed += ecmdWriteDataFormatted(buffer, outputformat);
      }

      if (verbose)
      {	 
	// Display the wafer ID
	std::string    wafer;
	char waferOut[30];
	rc = formatEcidString( buffer, wafer );
	if (rc) return rc;
	sprintf(waferOut, "     Wafer ID: %s\n", wafer.c_str());
	printed += waferOut;

	// Go through additionalInfo
	for (uint32_t i = 0; i < additionalInfo.size(); ++i)
	{
	  printed += additionalInfo[i];
	}
	// Now clear out the additionalInfo so it's clean for the next target
	additionalInfo.clear();
	  
      }
	
      ecmdOutput( printed.c_str() );

    }
    // coeRc will be the return code from in the loop, coe mode or not.
    if (coeRc) return coeRc;

    // This is an error common across all UI functions
    if (!validPosFound)
    {
        ecmdOutputError("getecid - Unable to find a valid chip to execute command on\n");
        return ECMD_TARGET_NOT_CONFIGURED;
    }

    return rc;
}
#endif // ECMD_REMOVE_FSI_FUNCTIONS

#ifndef ECMD_REMOVE_MPIPL_FUNCTIONS
uint32_t ecmdMpiplClearCheckstopUser(int argc, char* argv[])
{
    uint32_t rc = ECMD_SUCCESS, coeRc = ECMD_SUCCESS;

    ecmdChipTarget target;                        ///< Current target being operated on
    bool validPosFound = false;                   ///< Did the looper find anything?
    ecmdLooperData looperData;                    ///< Store internal Looper data
    std::string printed;                          ///< Output data
    bool l_bucketModeEnabled = false;		  ///< Is bucket mode enabled or not?
    std::vector<ecmdChipTarget> l_targets;

    /************************************************************************/
    /* Parse Common Cmdline Args                                            */
    /************************************************************************/
    rc = ecmdCommandArgs(&argc, &argv);
    if (rc) return rc;

    /* Global args have been parsed, we can read if -coe was given */
    bool coeMode = ecmdGetGlobalVar(ECMD_GLOBALVAR_COEMODE); ///< Are we in continue on error mode

    /************************************************************************/
    /* parse local args here!                                               */
    /************************************************************************/
    if (argc < 1)  //chip
    {
        ecmdOutputError("mpiplclearcheckstop - Too few arguments specified; you need at least a chip.\n");
        ecmdOutputError("mpiplclearcheckstop - Type 'mpiplclearcheckstop -h' for usage.\n");
        return ECMD_INVALID_ARGS;
    }

    if (ecmdParseOption(&argc, &argv, "-bucket")) {
	l_bucketModeEnabled = true;
    }

    //Setup the target that will be used to query the system config
    std::string chipType, chipUnitType;
    rc = ecmdParseChipField(argv[0], chipType, chipUnitType, true /* supports wildcard usage */);
    if (rc)
    { 
        ecmdOutputError("mpiplclearcheckstop - Wildcard character detected however it is not being used correctly.\n");
        return rc;
    }

    if (chipUnitType != "")
    {
        ecmdOutputError("mpiplclearcheckstop - chipUnit specified on the command line, this function doesn't support chipUnits.\n");
        return ECMD_INVALID_ARGS;
    }

    if (chipType == "x")
    {
        target.chipTypeState = ECMD_TARGET_FIELD_WILDCARD;
    }
    else
    {
        target.chipType = chipType;
        target.chipTypeState = ECMD_TARGET_FIELD_VALID;
    }

    target.cageState = target.nodeState = target.slotState = target.posState = ECMD_TARGET_FIELD_WILDCARD;
    target.chipUnitTypeState = target.chipUnitNumState = target.threadState = ECMD_TARGET_FIELD_UNUSED;

    if (argc > 2)
    { 
        ecmdOutputError("mpiplclearcheckstop - Too many arguments specified; you probably added an option that wasn't recognized.\n");
        ecmdOutputError("mpiplclearcheckstop - Type 'mpiplclearcheckstop -h' for usage.\n");
        return ECMD_INVALID_ARGS;
    }

    /************************************************************************/
    /* If bucket mode is enabled then build a vector of ecmdChipTargts, where*/
    /* each target is of the same chip type.                                */
    /************************************************************************/
    if (l_bucketModeEnabled)
    {
	rc = ecmdLooperInit(target, ECMD_ALL_TARGETS_LOOP, looperData);
	if (rc) return rc;

	while (ecmdLooperNext(target, looperData)) {
	    l_targets.push_back(target);
	}

	rc = mpiplClearCheckstop(l_targets);
	if (rc) {
	    printed = "mpiplclearcheckstop - Error occured performing mpiplclearcheckstop ";
	    printed += "\n";
	    ecmdOutputError( printed.c_str() );
	    coeRc = rc;

	}
	else
	{
	    validPosFound = true;
	}
    }
    /************************************************************************/
    /* If bucket mode is disabled it's business as usual - loop on what the  */
    /* user said to loop on. The vector will always be 1 entry large in this*/
    /* case.                                                                */
    /************************************************************************/
    else 
    {
	rc = ecmdLooperInit(target, ECMD_SELECTED_TARGETS_LOOP, looperData);
	if (rc) return rc;

	while (ecmdLooperNext(target, looperData) && (!coeRc || coeMode)) {
	    l_targets.clear();
	    l_targets.push_back(target);
	    rc = mpiplClearCheckstop(l_targets);
	    if (rc) {
		printed = "mpiplclearcheckstop - Error occured performing mpiplclearcheckstop on ";
		printed += "\n";
		ecmdOutputError( printed.c_str() );
		coeRc = rc;
		continue;
	    }
	    else
	    {
		validPosFound = true;
	    }
	}
    }
    // coeRc will be the return code from in the loop, coe mode or not.
    if (coeRc) return coeRc;

    // This is an error common across all UI functions
    if (!validPosFound)
    {
	ecmdOutputError("mpiplclearcheckstop - Unable to find a valid chip to execute command on\n");
	return ECMD_TARGET_NOT_CONFIGURED;
    }

    return rc;
}

uint32_t ecmdMpiplForceWinkleUser(int argc, char* argv[])
{
    uint32_t rc = ECMD_SUCCESS, coeRc = ECMD_SUCCESS;

    ecmdChipTarget target;                        ///< Current target being operated on
    bool validPosFound = false;                   ///< Did the looper find anything?
    ecmdLooperData looperData;                    ///< Store internal Looper data
    std::string printed;                          ///< Output data

    /************************************************************************/
    /* Parse Common Cmdline Args                                            */
    /************************************************************************/
    rc = ecmdCommandArgs(&argc, &argv);
    if (rc) return rc;

    /* Global args have been parsed, we can read if -coe was given */
    bool coeMode = ecmdGetGlobalVar(ECMD_GLOBALVAR_COEMODE); ///< Are we in continue on error mode

    /************************************************************************/
    /* Parse Local ARGS here!                                               */
    /************************************************************************/
    if (argc < 1)  //chip
    {
        ecmdOutputError("mpiplforcewinkle - Too few arguments specified; you need at least a chip.\n");
        ecmdOutputError("mpiplforcewinkle - Type 'mpiplforcewinkle -h' for usage.\n");
        return ECMD_INVALID_ARGS;
    }

    //Setup the target that will be used to query the system config
    std::string chipType, chipUnitType;
    rc = ecmdParseChipField(argv[0], chipType, chipUnitType, true /* supports wildcard usage */);
    if (rc)
    { 
        ecmdOutputError("mpiplforcewinkle - Wildcard character detected however it is not being used correctly.\n");
        return rc;
    }

    if (chipUnitType != "")
    {
        ecmdOutputError("mpiplforcewinkle - chipUnit specified on the command line, this function doesn't support chipUnits.\n");
        return ECMD_INVALID_ARGS;
    }

    if (chipType == "x")
    {
        target.chipTypeState = ECMD_TARGET_FIELD_WILDCARD;
    }
    else
    {
        target.chipType = chipType;
        target.chipTypeState = ECMD_TARGET_FIELD_VALID;
    }

    target.cageState = target.nodeState = target.slotState = target.posState = ECMD_TARGET_FIELD_WILDCARD;
    target.chipUnitTypeState = target.chipUnitNumState = target.threadState = ECMD_TARGET_FIELD_UNUSED;

    if (argc > 2)
    { 
        ecmdOutputError("mpiplforcewinkle - Too many arguments specified; you probably added an option that wasn't recognized.\n");
        ecmdOutputError("mpiplforcewinkle - Type 'mpiplforcewinkle -h' for usage.\n");
        return ECMD_INVALID_ARGS;
    }

    /************************************************************************/
    /* Kickoff Looping Stuff                                                */
    /************************************************************************/

    rc = ecmdLooperInit(target, ECMD_SELECTED_TARGETS_LOOP, looperData);
    if (rc) return rc;

    while (ecmdLooperNext(target, looperData) && (!coeRc || coeMode)) {

        rc = mpiplForceWinkle(target);
        if (rc) {
            printed = "mpiplforcewinkle - Error occured performing mpiplforcewinkle on ";
            printed += ecmdWriteTarget(target);
            printed += "\n";
            ecmdOutputError( printed.c_str() );
            coeRc = rc;
            continue;
        }
        else
        {
            validPosFound = true;
        }
    }
    // coeRc will be the return code from in the loop, coe mode or not.
    if (coeRc) return coeRc;

    // This is an error common across all UI functions
    if (!validPosFound)
    {
        ecmdOutputError("mpiplforcewinkle - Unable to find a valid chip to execute command on\n");
        return ECMD_TARGET_NOT_CONFIGURED;
    }

    return rc;
}
#endif // ECMD_REMOVE_MPIPL_FUNCTIONS

uint32_t formatEcidString( ecmdDataBuffer & i_ecidBuffer, std::string  & o_waferString)
{
  uint32_t rc = ECMD_SUCCESS;

  ecmdDataBuffer waferID;
  std::string waferIdText;
  uint32_t ecidPos = 0;

  // Using the first 8 positions of the 10 character field, the last two are programmed to "blanks" (all ls)
  // On samsung wafer, word 3 is going to be blank (all 1's) and we use that to determine the format
  uint16_t word3 = i_ecidBuffer.getHalfWord(3);

  // P10 and later chips using samsung wafer
  if ((word3 & 0x0fff) == 0x0fff)
  {
    waferID.setBitLength(48);
    // Using the first 8 positions of the 10 character field, the last two are programmed to "blanks" (all ls)
    ecidPos = 8;
    i_ecidBuffer.extract(waferID, 4, 48);
  } else {
    waferID.setBitLength(60);
    ecidPos = 10;
    i_ecidBuffer.extract(waferID, 4, 60);
  }

  for (uint32_t offset = 0; offset < ecidPos; offset++)
  {
    uint8_t code = 0;
    waferID.extractToRight(&code, offset * 6, 6);
    if (code < 10)
    {
      code += '0';
    }
    else if ((code >= 10) && (code < 36))
    {
      code += ('A' - 10);
    }
    else if (code == '\x3D')
    {
      code = '-';
    }
    else if (code == '\x3E')
    {
      code = '.';
    }
    else if (code == '\x3F')
    {
      code = ' ';
    }
    else
    {
      // unknown code
    }
    waferIdText += code;
  }

  // P10 and later chips using samsung wafer has bits 52...63 set to all 1's
  if ((word3 & 0x0fff) == 0x0fff)
  {
    // getCheckSum is not called in p10 wrapper, so we will skip it
    o_waferString = waferIdText;
  } else {

    waferIdText[10] = 0;
    o_waferString = waferIdText;

    // getCheckSum()
    std::string rtn=o_waferString + "A0";
    int sum=0;
    for (uint32_t i = 0; i < rtn.size(); i++)
      {
        sum = ((sum * 8) + (rtn[i] - 32)) % 59;
      }
    if (sum != 0 && rtn.size() >= 12) {
      int adjust = 59 - sum;
      rtn[11] += adjust & 7;
      adjust >>= 3;
      rtn[10] += adjust & 7;
    }
    o_waferString = rtn;
  } 

  return rc;
  
}

uint32_t ecmdChipCleanupUser(int argc, char * argv[]) {
  uint32_t rc = ECMD_SUCCESS, coeRc = ECMD_SUCCESS;

  ecmdChipTarget target;        ///< Current target
  bool validPosFound = false;   ///< Did we find something to actually execute on ?
  std::string printed;          ///< Print Buffer
  ecmdLooperData looperdata;            ///< Store internal Looper data
  uint32_t mode = ECMD_CHIP_CLEANUP_MEMDA;
  
  /************************************************************************/
  /* Parse Local FLAGS here!                                              */
  /************************************************************************/
  if (ecmdParseOption(&argc, &argv, "-pm")) {
    mode = ECMD_CHIP_CLEANUP_PM;
  }

  /************************************************************************/
  /* Parse Common Cmdline Args                                            */
  /************************************************************************/
  rc = ecmdCommandArgs(&argc, &argv);
  if (rc) return rc;

  /* Global args have been parsed, we can read if -coe was given */
  bool coeMode = ecmdGetGlobalVar(ECMD_GLOBALVAR_COEMODE); ///< Are we in continue on error mode

  /************************************************************************/
  /* Parse Local ARGS here!                                               */
  /************************************************************************/
  if (argc > 1) {  //chip
    ecmdOutputError("ecmdchipcleanup - Too many arguments specified; specify at most one chip.\n");
    ecmdOutputError("ecmdchipcleanup - Type 'ecmdchipcleanup -h' for usage.\n");
    return ECMD_INVALID_ARGS;
  }

  // use PU
  if (argc == 0) {
    target.chipType = ECMD_CHIPT_PROCESSOR;
    target.chipTypeState = ECMD_TARGET_FIELD_VALID;
    target.cageState = target.nodeState = target.slotState = target.posState = ECMD_TARGET_FIELD_WILDCARD;
    target.chipUnitTypeState = target.chipUnitNumState = target.threadState = ECMD_TARGET_FIELD_UNUSED;
  } else {
    //Setup the target
    std::string chipType, chipUnitType;
    rc = ecmdParseChipField(argv[0], chipType, chipUnitType, true /* supports wildcard usage */);
    if (rc) {
        ecmdOutputError("ecmdchipcleanup - Wildcard character detected however it is not being used correctly.\n");
        return rc;
    }

    target.cageState = target.nodeState = target.slotState = target.posState = ECMD_TARGET_FIELD_WILDCARD;
    target.chipUnitTypeState = target.chipUnitNumState = target.threadState = ECMD_TARGET_FIELD_UNUSED;
    
    if (chipType == "x") {
      target.chipTypeState = ECMD_TARGET_FIELD_WILDCARD;
    } else {
      target.chipType = chipType;
      target.chipTypeState = ECMD_TARGET_FIELD_VALID;
    }

    if (chipUnitType != "") {
      target.chipUnitType = chipUnitType;
      target.chipUnitTypeState = ECMD_TARGET_FIELD_VALID;
      target.chipUnitNumState = ECMD_TARGET_FIELD_WILDCARD;
    } 
  }

  rc = ecmdLooperInit(target, ECMD_SELECTED_TARGETS_LOOP, looperdata);
  if (rc) return rc;

  while (ecmdLooperNext(target, looperdata) && (!coeRc || coeMode)) {
  
    /* Go cleanup the chips */
    rc = ecmdChipCleanup(target, mode);
    if (rc) {
      printed = "ecmdchipcleanup - Error occured performing ecmdChipCleanup on ";
      printed += ecmdWriteTarget(target) + "\n";
      ecmdOutputError( printed.c_str() );
      coeRc = rc;
      continue;
    }
    else {
      validPosFound = true;     
    }
  }

  // coeRc will be the return code from in the loop, coe mode or not.
  if (coeRc) return coeRc;

  // This is an error common across all UI functions
  if (!validPosFound) {
    printed = "ecmdchipcleanup - Unable to find a valid chip to execute command on\n";
    //this is an error common across all UI functions
    ecmdOutputError(printed.c_str());
    return ECMD_TARGET_NOT_CONFIGURED;
  }

  return rc;
}
