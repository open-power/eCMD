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


//----------------------------------------------------------------------
//  Includes
//----------------------------------------------------------------------
#include <stdio.h>
#include <dlfcn.h>

#include <ecmdClientCapi.H>
#include <ecmdSharedUtils.H>
#include <ecmdReturnCodes.H>
#include <ecmdClientEnums.H>
#include <ecmdUtils.H>

#include <hw_access_def.H> 
#include <return_code.H>

#include <fapi2ClientCapi.H>
//#include <fapi2SystemConfig.H>
#include <fapi2SharedUtils.H>
#include <fapi2Structs.H>
#ifndef ECMD_STATIC_FUNCTIONS
#include <fapi2ClientEnums.H>
#else
#include <fapi2DllCapi.H>
#include <ecmdDllCapi.H>
#endif

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

//----------------------------------------------------------------------
//  Global Variables
//----------------------------------------------------------------------

static const char ECMD_DLL_NOT_LOADED_ERROR[] = ": eCMD Function called before DLL has been loaded\n";
static const char ECMD_UNABLE_TO_FIND_FUNCTION_ERROR[] = ": Unable to find function, must be an invalid DLL - program aborting\n";


#ifndef ECMD_STATIC_FUNCTIONS
/* This is from ecmdClientCapiFunc.C */
extern void * dlHandle;
/* These are from fapiClientCapiFunc.C */
extern void * fapi2DllFnTable[];
extern void * DllFnTable[];
#endif
extern bool fapi2Initialized;

using namespace fapi2;
#ifndef ECMD_STRIP_DEBUG
extern int ecmdClientDebug;
extern int fppCallCount;
extern bool ecmdDebugOutput;
#endif



//---------------------------------------------------------------------
// Member Function Specifications
//---------------------------------------------------------------------
uint32_t fapi2QueryFileLocation(fapi2::FileType_t i_fileType, std::string & i_fileName,  std::string & o_fileLocation, std::string i_version) {

  uint32_t rc;

#ifndef ECMD_STATIC_FUNCTIONS
  if (dlHandle == NULL) {
    fprintf(stderr,"dllFapi2QueryFileLocation%s",ECMD_DLL_NOT_LOADED_ERROR);
    exit(ECMD_DLL_INVALID);
  }
#endif

#ifndef ECMD_STRIP_DEBUG
  int myTcount;
  std::vector< void * > args;
  if (ecmdClientDebug != 0) {
     args.push_back((void*) &i_fileType);
     args.push_back((void*) &i_fileName);
     args.push_back((void*) &o_fileLocation);
     args.push_back((void*) &i_version);
     fppCallCount++;
     myTcount = fppCallCount;
     ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"uint32_t fapi2QueryFileLocation(fapi2::FileType_t i_fileType, std::string & i_fileName, std::string & o_fileLocation, std::string i_version)",args);
     ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"fapi2QueryFileLocation");
  }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
  rc = dllFapi2QueryFileLocation(i_fileType, i_fileName, o_fileLocation, i_version);
#else
  if (fapi2DllFnTable[ECMD_FAPI2QUERYFILELOCATION] == NULL) {
     fapi2DllFnTable[ECMD_FAPI2QUERYFILELOCATION] = (void*)dlsym(dlHandle, "dllFapi2QueryFileLocation");
     if (fapi2DllFnTable[ECMD_FAPI2QUERYFILELOCATION] == NULL) {
       fprintf(stderr,"dllFapi2QueryFileLocation%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
       ecmdDisplayDllInfo();
       exit(ECMD_DLL_INVALID);
     }
  }

  uint32_t (*Function)(fapi2::FileType_t,  std::string &, std::string &,  std::string) = 
      (uint32_t(*)(fapi2::FileType_t,  std::string &, std::string &,  std::string))fapi2DllFnTable[ECMD_FAPI2QUERYFILELOCATION];
  rc =    (*Function)(i_fileType, i_fileName, o_fileLocation, i_version);
#endif

#ifndef ECMD_STRIP_DEBUG
  if (ecmdClientDebug != 0) {
     args.push_back((void*) &rc);
     ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"fapi2QueryFileLocation");
     ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t fapi2QueryFileLocation(fapi2::FileType_t i_fileType, std::string & i_fileName, std::string & o_fileLocation, std::string i_version)",args);
   }
#endif

  if (rc && !ecmdGetGlobalVar(ECMD_GLOBALVAR_QUIETERRORMODE)) {
    std::string errorString;
    errorString = ecmdGetErrorMsg(rc, false, ecmdGetGlobalVar(ECMD_GLOBALVAR_CMDLINEMODE), false);
    if (errorString.size()) ecmdOutput(errorString.c_str());
  }

  return rc;
}

uint32_t fapi2GetAttributeOverride(const ecmdChipTarget & i_target, const uint32_t i_id, fapi2::AttributeData & o_data){

  uint32_t rc;

#ifndef ECMD_STATIC_FUNCTIONS
  if (dlHandle == NULL) {
    fprintf(stderr,"dllFapi2GetAttributeOverride%s",ECMD_DLL_NOT_LOADED_ERROR);
    exit(ECMD_DLL_INVALID);
  }
#endif

#ifndef ECMD_STRIP_DEBUG
  int myTcount;
  std::vector< void * > args;
  if (ecmdClientDebug != 0) {
     args.push_back((void*) &i_target);
     args.push_back((void*) &i_id);
     args.push_back((void*) &o_data);
     fppCallCount++;
     myTcount = fppCallCount;
     ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"uint32_t fapi2GetAttributeOverride(const ecmdChipTarget & i_target, const uint32_t i_id, fapi2::AttributeData & o_data)",args);
     ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"fapi2GetAttributeOverride");
  }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
  rc = dllFapi2GetAttributeOverride(i_target, i_id, o_data);
#else
  if (fapi2DllFnTable[ECMD_FAPI2GETATTRIBUTEOVERRIDE] == NULL) {
     fapi2DllFnTable[ECMD_FAPI2GETATTRIBUTEOVERRIDE] = (void*)dlsym(dlHandle, "dllFapi2GetAttributeOverride");
     if (fapi2DllFnTable[ECMD_FAPI2GETATTRIBUTEOVERRIDE] == NULL) {
       fprintf(stderr,"dllFapi2GetAttributeOverride%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
       ecmdDisplayDllInfo();
       exit(ECMD_DLL_INVALID);
     }
  }

  uint32_t (*Function)(const ecmdChipTarget &, const uint32_t,  fapi2::AttributeData &) = 
      (uint32_t(*)(const ecmdChipTarget &, const uint32_t,  fapi2::AttributeData &))fapi2DllFnTable[ECMD_FAPI2GETATTRIBUTEOVERRIDE];
  rc =    (*Function)(i_target, i_id, o_data);
#endif

#ifndef ECMD_STRIP_DEBUG
  if (ecmdClientDebug != 0) {
     args.push_back((void*) &rc);
     ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"fapi2GetAttributeOverride");
     ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t fapi2GetAttributeOverride(const ecmdChipTarget & i_target, const uint32_t i_id, fapi2::AttributeData & o_data)",args);
   }
#endif

  if (rc && !ecmdGetGlobalVar(ECMD_GLOBALVAR_QUIETERRORMODE)) {
    std::string errorString;
    errorString = ecmdGetErrorMsg(rc, false, ecmdGetGlobalVar(ECMD_GLOBALVAR_CMDLINEMODE), false);
    if (errorString.size()) ecmdOutput(errorString.c_str());
  }

  return rc;
}

uint32_t fapi2GetAttribute(const ecmdChipTarget & i_target, const uint32_t i_id, fapi2::AttributeData & o_data){

  uint32_t rc;

#ifndef ECMD_STATIC_FUNCTIONS
  if (dlHandle == NULL) {
    fprintf(stderr,"dllFapi2GetAttribute%s",ECMD_DLL_NOT_LOADED_ERROR);
    exit(ECMD_DLL_INVALID);
  }
#endif

#ifndef ECMD_STRIP_DEBUG
  int myTcount;
  std::vector< void * > args;
  if (ecmdClientDebug != 0) {
     args.push_back((void*) &i_target);
     args.push_back((void*) &i_id);
     args.push_back((void*) &o_data);
     fppCallCount++;
     myTcount = fppCallCount;
     ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"uint32_t fapi2GetAttribute(const ecmdChipTarget & i_target, const uint32_t i_id, fapi2::AttributeData & o_data)",args);
     ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"fapi2GetAttribute");
  }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
  rc = dllFapi2GetAttribute(i_target, i_id, o_data);
#else
  if (fapi2DllFnTable[ECMD_FAPI2GETATTRIBUTE] == NULL) {
     fapi2DllFnTable[ECMD_FAPI2GETATTRIBUTE] = (void*)dlsym(dlHandle, "dllFapi2GetAttribute");
     if (fapi2DllFnTable[ECMD_FAPI2GETATTRIBUTE] == NULL) {
       fprintf(stderr,"dllFapi2GetAttribute%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
       ecmdDisplayDllInfo();
       exit(ECMD_DLL_INVALID);
     }
  }

  uint32_t (*Function)(const ecmdChipTarget &, const uint32_t,  fapi2::AttributeData &) = 
      (uint32_t(*)(const ecmdChipTarget &, const uint32_t,  fapi2::AttributeData &))fapi2DllFnTable[ECMD_FAPI2GETATTRIBUTE];
  rc =    (*Function)(i_target, i_id, o_data);
#endif

#ifndef ECMD_STRIP_DEBUG
  if (ecmdClientDebug != 0) {
     args.push_back((void*) &rc);
     ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"fapi2GetAttribute");
     ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t fapi2GetAttribute(const ecmdChipTarget & i_target, const uint32_t i_id, fapi2::AttributeData & o_data)",args);
   }
#endif

  if (rc && !ecmdGetGlobalVar(ECMD_GLOBALVAR_QUIETERRORMODE)) {
    std::string errorString;
    errorString = ecmdGetErrorMsg(rc, false, ecmdGetGlobalVar(ECMD_GLOBALVAR_CMDLINEMODE), false);
    if (errorString.size()) ecmdOutput(errorString.c_str());
  }

  return rc;
}

uint32_t fapi2SetAttribute(const ecmdChipTarget & i_target, const uint32_t i_id, fapi2::AttributeData & i_data){

  uint32_t rc;

#ifndef ECMD_STATIC_FUNCTIONS
  if (dlHandle == NULL) {
    fprintf(stderr,"dllFapi2SetAttribute%s",ECMD_DLL_NOT_LOADED_ERROR);
    exit(ECMD_DLL_INVALID);
  }
#endif

#ifndef ECMD_STRIP_DEBUG
  int myTcount;
  std::vector< void * > args;
  if (ecmdClientDebug != 0) {
     args.push_back((void*) &i_target);
     args.push_back((void*) &i_id);
     args.push_back((void*) &i_data);
     fppCallCount++;
     myTcount = fppCallCount;
     ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"uint32_t ecmdGetConfigurationComplex(const ecmdChipTarget & i_target, const uint32_t i_id, ecmdConfigData & i_data)",args);
     ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"ecmdGetConfigurationComplex");
  }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
  rc = dllFapi2SetAttribute(i_target, i_id, i_data);
#else
  if (fapi2DllFnTable[ECMD_FAPI2SETATTRIBUTE] == NULL) {
     fapi2DllFnTable[ECMD_FAPI2SETATTRIBUTE] = (void*)dlsym(dlHandle, "dllFapi2SetAttribute");
     if (fapi2DllFnTable[ECMD_FAPI2SETATTRIBUTE] == NULL) {
       fprintf(stderr,"dllFapi2SetAttribute%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
       ecmdDisplayDllInfo();
       exit(ECMD_DLL_INVALID);
     }
  }

  uint32_t (*Function)(const ecmdChipTarget &, const uint32_t,  fapi2::AttributeData &) = 
      (uint32_t(*)(const ecmdChipTarget &, const uint32_t,  fapi2::AttributeData &))fapi2DllFnTable[ECMD_FAPI2SETATTRIBUTE];
  rc =    (*Function)(i_target, i_id, i_data);
#endif

#ifndef ECMD_STRIP_DEBUG
  if (ecmdClientDebug != 0) {
     args.push_back((void*) &rc);
     ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"fapi2SetAttribute");
     ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t fapi2SetAttribute(const ecmdChipTarget & i_target, const uint32_t i_id, fapi2::AttributeData & i_data)",args);
   }
#endif

  if (rc && !ecmdGetGlobalVar(ECMD_GLOBALVAR_QUIETERRORMODE)) {
    std::string errorString;
    errorString = ecmdGetErrorMsg(rc, false, ecmdGetGlobalVar(ECMD_GLOBALVAR_CMDLINEMODE), false);
    if (errorString.size()) ecmdOutput(errorString.c_str());
  }

  return rc;
}

uint32_t fapi2AttributeDataToString(fapi2::AttributeId i_attrId, const fapi2::AttributeData & i_attrData, std::string & o_attrDataString, bool i_fullData, const char * i_format){

  uint32_t rc;

#ifndef ECMD_STATIC_FUNCTIONS
  if (dlHandle == NULL) {
    fprintf(stderr,"dllFapi2AttributeDataToString%s",ECMD_DLL_NOT_LOADED_ERROR);
    exit(ECMD_DLL_INVALID);
  }
#endif

#ifndef ECMD_STRIP_DEBUG
  int myTcount;
  std::vector< void * > args;
  if (ecmdClientDebug != 0) {
     args.push_back((void*) &i_attrId);
     args.push_back((void*) &i_attrData);
     args.push_back((void*) &o_attrDataString);
     args.push_back((void*) &i_fullData);
     args.push_back((void*) &i_format);
     fppCallCount++;
     myTcount = fppCallCount;
     ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"uint32_t fapi2AttributeDataToString(fapi2::AttributeId i_attrId, const fapi2::AttributeData & i_attrData, std::string & o_attrDataString, bool i_fullData, const char * i_format)",args);
     ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"fapi2AttributeDataToString");
  }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
  rc = dllFapi2AttributeDataToString(i_attrId, i_attrData, o_attrDataString, i_fullData, i_format);
#else
  if (fapi2DllFnTable[ECMD_FAPI2ATTRIBUTEDATATOSTRING] == NULL) {
     fapi2DllFnTable[ECMD_FAPI2ATTRIBUTEDATATOSTRING] = (void*)dlsym(dlHandle, "dllFapi2AttributeDataToString");
     if (fapi2DllFnTable[ECMD_FAPI2ATTRIBUTEDATATOSTRING] == NULL) {
       fprintf(stderr,"dllFapi2AttributeDataToString%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
       ecmdDisplayDllInfo();
       exit(ECMD_DLL_INVALID);
     }
  }

  uint32_t (*Function)(fapi2::AttributeId, const fapi2::AttributeData &, std::string &, bool, const char *) = 
      (uint32_t(*)(fapi2::AttributeId, const fapi2::AttributeData &, std::string &, bool, const char *))fapi2DllFnTable[ECMD_FAPI2ATTRIBUTEDATATOSTRING];
  rc =    (*Function)(i_attrId, i_attrData, o_attrDataString, i_fullData, i_format);
#endif

#ifndef ECMD_STRIP_DEBUG
  if (ecmdClientDebug != 0) {
     args.push_back((void*) &rc);
     ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"fapi2AttributeDataToString");
     ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t fapi2AttributeDataToString(fapi2::AttributeId i_attrId, const fapi2::AttributeData & i_attrData, std::string & o_attrDataString, bool i_fullData, const char * i_format)",args);
   }
#endif

  if (rc && !ecmdGetGlobalVar(ECMD_GLOBALVAR_QUIETERRORMODE)) {
    std::string errorString;
    errorString = ecmdGetErrorMsg(rc, false, ecmdGetGlobalVar(ECMD_GLOBALVAR_CMDLINEMODE), false);
    if (errorString.size()) ecmdOutput(errorString.c_str());
  }

  return rc;
}

uint32_t fapi2AttributeStringToId(std::string i_attrString, fapi2::AttributeId & o_attrId){

  uint32_t rc;

#ifndef ECMD_STATIC_FUNCTIONS
  if (dlHandle == NULL) {
    fprintf(stderr,"dllFapi2AttributeStringToId%s",ECMD_DLL_NOT_LOADED_ERROR);
    exit(ECMD_DLL_INVALID);
  }
#endif

#ifndef ECMD_STRIP_DEBUG
  int myTcount;
  std::vector< void * > args;
  if (ecmdClientDebug != 0) {
     args.push_back((void*) &i_attrString);
     args.push_back((void*) &o_attrId);
     fppCallCount++;
     myTcount = fppCallCount;
     ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"uint32_t fapi2AttributeStringToId(std::string i_attrString, fapi2::AttributeId & o_attrId)",args);
     ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"fapi2AttributeStringToId");
  }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
  rc = dllFapi2AttributeStringToId(i_attrString, o_attrId);
#else
  if (fapi2DllFnTable[ECMD_FAPI2ATTRIBUTESTRINGTOID] == NULL) {
     fapi2DllFnTable[ECMD_FAPI2ATTRIBUTESTRINGTOID] = (void*)dlsym(dlHandle, "dllFapi2AttributeStringToId");
     if (fapi2DllFnTable[ECMD_FAPI2ATTRIBUTESTRINGTOID] == NULL) {
       fprintf(stderr,"dllFapi2AttributeStringToId%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
       ecmdDisplayDllInfo();
       exit(ECMD_DLL_INVALID);
     }
  }

  uint32_t (*Function)(std::string, fapi2::AttributeId &) = 
      (uint32_t(*)(std::string, fapi2::AttributeId &))fapi2DllFnTable[ECMD_FAPI2ATTRIBUTESTRINGTOID];
  rc =    (*Function)(i_attrString, o_attrId);
#endif

#ifndef ECMD_STRIP_DEBUG
  if (ecmdClientDebug != 0) {
     args.push_back((void*) &rc);
     ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"fapi2AttributeStringToId");
     ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t fapi2AttributeStringToId(std::string i_attrString, fapi2::AttributeId & o_attrId)",args);
   }
#endif

  if (rc && !ecmdGetGlobalVar(ECMD_GLOBALVAR_QUIETERRORMODE)) {
    std::string errorString;
    errorString = ecmdGetErrorMsg(rc, false, ecmdGetGlobalVar(ECMD_GLOBALVAR_CMDLINEMODE), false);
    if (errorString.size()) ecmdOutput(errorString.c_str());
  }

  return rc;
}

uint32_t fapi2GetAttrInfo(fapi2::AttributeId i_attrId, uint32_t & o_attrType, uint32_t & o_numOfEntries, uint32_t & o_numOfBytes, bool & o_attrEnum){

  uint32_t rc;

#ifndef ECMD_STATIC_FUNCTIONS
  if (dlHandle == NULL) {
    fprintf(stderr,"dllFapi2GetAttrInfo%s",ECMD_DLL_NOT_LOADED_ERROR);
    exit(ECMD_DLL_INVALID);
  }
#endif

#ifndef ECMD_STRIP_DEBUG
  int myTcount;
  std::vector< void * > args;
  if (ecmdClientDebug != 0) {
     args.push_back((void*) &i_attrId);
     args.push_back((void*) &o_attrType);
     args.push_back((void*) &o_numOfEntries);
     args.push_back((void*) &o_numOfBytes);
     args.push_back((void*) &o_attrEnum);
     fppCallCount++;
     myTcount = fppCallCount;
     ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"uint32_t fapi2GetAttrInfo(fapi2::AttributeId i_attrId, uint32_t & o_attrType, uint32_t & o_numOfEntries, uint32_t & o_numOfBytes, bool & o_attrEnum)",args);
     ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"fapi2GetAttrInfo");
  }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
  rc = dllFapi2GetAttrInfo(i_attrId, o_attrType, o_numOfEntries, o_numOfBytes, o_attrEnum);
#else
  if (fapi2DllFnTable[ECMD_FAPI2GETATTRINFO] == NULL) {
     fapi2DllFnTable[ECMD_FAPI2GETATTRINFO] = (void*)dlsym(dlHandle, "dllFapi2GetAttrInfo");
     if (fapi2DllFnTable[ECMD_FAPI2GETATTRINFO] == NULL) {
       fprintf(stderr,"dllFapi2GetAttrInfo%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
       ecmdDisplayDllInfo();
       exit(ECMD_DLL_INVALID);
     }
  }

  uint32_t (*Function)(fapi2::AttributeId, uint32_t &, uint32_t &, uint32_t &, bool &) = 
      (uint32_t(*)(fapi2::AttributeId, uint32_t &, uint32_t &, uint32_t &, bool &))fapi2DllFnTable[ECMD_FAPI2GETATTRINFO];
  rc =    (*Function)(i_attrId, o_attrType, o_numOfEntries, o_numOfBytes, o_attrEnum);
#endif

#ifndef ECMD_STRIP_DEBUG
  if (ecmdClientDebug != 0) {
     args.push_back((void*) &rc);
     ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"fapi2GetAttrInfo");
     ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t fapi2GetAttrInfo(fapi2::AttributeId i_attrId, uint32_t & o_attrType, uint32_t & o_numOfEntries, uint32_t & o_numOfBytes, bool & o_attrEnum)",args);
   }
#endif

  if (rc && !ecmdGetGlobalVar(ECMD_GLOBALVAR_QUIETERRORMODE)) {
    std::string errorString;
    errorString = ecmdGetErrorMsg(rc, false, ecmdGetGlobalVar(ECMD_GLOBALVAR_CMDLINEMODE), false);
    if (errorString.size()) ecmdOutput(errorString.c_str());
  }

  return rc;
}

uint32_t fapi2GetAttrIdsByType(fapi2::TargetType i_targetTypes, uint32_t i_attributeSources, std::list<fapi2::AttributeId> & o_attributeIds){

  uint32_t rc;

#ifndef ECMD_STATIC_FUNCTIONS
  if (dlHandle == NULL) {
    fprintf(stderr,"dllFapi2GetAttrIdsByType%s",ECMD_DLL_NOT_LOADED_ERROR);
    exit(ECMD_DLL_INVALID);
  }
#endif

#ifndef ECMD_STRIP_DEBUG
  int myTcount;
  std::vector< void * > args;
  if (ecmdClientDebug != 0) {
     args.push_back((void*) &i_targetTypes);
     args.push_back((void*) &i_attributeSources);
     args.push_back((void*) &o_attributeIds);
     fppCallCount++;
     myTcount = fppCallCount;
     ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"uint32_t fapi2GetAttrIdsByType(fapi2::TargetType i_targetTypes, uint32_t i_attributeSources, std::list<fapi2::AttributeId> & o_attributeIds)",args);
     ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"fapi2GetAttrIdsByType");
  }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
  rc = dllFapi2GetAttrIdsByType(i_targetTypes, i_attributeSources, o_attributeIds);
#else
  if (fapi2DllFnTable[ECMD_FAPI2GETATTRIDSBYTYPE] == NULL) {
     fapi2DllFnTable[ECMD_FAPI2GETATTRIDSBYTYPE] = (void*)dlsym(dlHandle, "dllFapi2GetAttrIdsByType");
     if (fapi2DllFnTable[ECMD_FAPI2GETATTRIDSBYTYPE] == NULL) {
       fprintf(stderr,"dllFapi2GetAttrIdsByType%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
       ecmdDisplayDllInfo();
       exit(ECMD_DLL_INVALID);
     }
  }

  uint32_t (*Function)(fapi2::TargetType, uint32_t, std::list<fapi2::AttributeId> &) = 
      (uint32_t(*)(fapi2::TargetType, uint32_t, std::list<fapi2::AttributeId> &))fapi2DllFnTable[ECMD_FAPI2GETATTRIDSBYTYPE];
  rc =    (*Function)(i_targetTypes, i_attributeSources, o_attributeIds);
#endif

#ifndef ECMD_STRIP_DEBUG
  if (ecmdClientDebug != 0) {
     args.push_back((void*) &rc);
     ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"fapi2GetAttrIdsByType");
     ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t fapi2GetAttrIdsByType(fapi2::TargetType i_TargetTypes, uint32_t i_attributeSources, std::list<fapi2::AttributeId> & o_attributeIds)",args);
   }
#endif

  if (rc && !ecmdGetGlobalVar(ECMD_GLOBALVAR_QUIETERRORMODE)) {
    std::string errorString;
    errorString = ecmdGetErrorMsg(rc, false, ecmdGetGlobalVar(ECMD_GLOBALVAR_CMDLINEMODE), false);
    if (errorString.size()) ecmdOutput(errorString.c_str());
  }

  return rc;
}

uint32_t fapi2AttrEnumStrToEnumVal(fapi2::AttributeData & io_attrData) {

  uint32_t rc;

#ifndef ECMD_STATIC_FUNCTIONS
  if (dlHandle == NULL) {
    fprintf(stderr,"dllFapi2AttrEnumStrToEnumVal%s",ECMD_DLL_NOT_LOADED_ERROR);
    exit(ECMD_DLL_INVALID);
  }
#endif

#ifndef ECMD_STRIP_DEBUG
  int myTcount;
  std::vector< void * > args;
  if (ecmdClientDebug != 0) {
     args.push_back((void*) &io_attrData);
     fppCallCount++;
     myTcount = fppCallCount;
     ecmdFunctionParmPrinter(myTcount, ECMD_FPP_FUNCTIONIN, "uint32_t fapi2AttrEnumStrToEnumVal(AttributeData & io_attrData)",args);
     ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"fapi2AttrEnumStrToEnumVal");
  }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
  rc = fapi2AttrEnumStrToEnumVal(io_attrData);
#else
  if (fapi2DllFnTable[ECMD_FAPI2ATTRENUMSTRTOENUMVAL] == NULL) {
     fapi2DllFnTable[ECMD_FAPI2ATTRENUMSTRTOENUMVAL] = (void*)dlsym(dlHandle, "dllFapi2AttrEnumStrToEnumVal");
     if (fapi2DllFnTable[ECMD_FAPI2ATTRENUMSTRTOENUMVAL] == NULL) {
       fprintf(stderr,"dllFapi2AttrEnumStrToEnumVal%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
       ecmdDisplayDllInfo();
       exit(ECMD_DLL_INVALID);
     }
  }

  uint32_t (*Function)(AttributeData &) = 
      (uint32_t(*)(AttributeData &))fapi2DllFnTable[ECMD_FAPI2ATTRENUMSTRTOENUMVAL];
  rc =    (*Function)(io_attrData);
#endif

#ifndef ECMD_STRIP_DEBUG
  if (ecmdClientDebug != 0) {
     args.push_back((void*) &rc);
     ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"fapi2AttrEnumStrToEnumVal");
     ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t fapi2AttrEnumStrToEnumVal(AttributeData & io_attrData)",args);
   }
#endif

  if (rc && !ecmdGetGlobalVar(ECMD_GLOBALVAR_QUIETERRORMODE)) {
    std::string errorString;
    errorString = ecmdGetErrorMsg(rc, false, ecmdGetGlobalVar(ECMD_GLOBALVAR_CMDLINEMODE), false);
    if (errorString.size()) ecmdOutput(errorString.c_str());
  }

  return rc;
}

void fapi2OutputError(const char* i_message) {

#ifndef ECMD_STATIC_FUNCTIONS
  if (dlHandle == NULL) {
    fprintf(stderr,"dllFapi2OutputError%s",ECMD_DLL_NOT_LOADED_ERROR);
    exit(ECMD_DLL_INVALID);
  }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
  dllFapi2OutputError(i_message);
#else
  if (fapi2DllFnTable[ECMD_FAPI2OUTPUTERROR] == NULL) {
     fapi2DllFnTable[ECMD_FAPI2OUTPUTERROR] = (void*)dlsym(dlHandle, "dllFapi2OutputError");
     if (fapi2DllFnTable[ECMD_FAPI2OUTPUTERROR] == NULL) {
       fprintf(stderr,"dllFapi2OutputError%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
       ecmdDisplayDllInfo();
       exit(ECMD_DLL_INVALID);
     }
  }

  void (*Function)(const char*) = 
      (void(*)(const char*))fapi2DllFnTable[ECMD_FAPI2OUTPUTERROR];
   (*Function)(i_message);
#endif

}

void fapi2OutputInfo(const char* i_message) {

#ifndef ECMD_STATIC_FUNCTIONS
  if (dlHandle == NULL) {
    fprintf(stderr,"dllFapi2OutputInfo%s",ECMD_DLL_NOT_LOADED_ERROR);
    exit(ECMD_DLL_INVALID);
  }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
  dllFapi2OutputInfo(i_message);
#else
  if (fapi2DllFnTable[ECMD_FAPI2OUTPUTINFO] == NULL) {
     fapi2DllFnTable[ECMD_FAPI2OUTPUTINFO] = (void*)dlsym(dlHandle, "dllFapi2OutputInfo");
     if (fapi2DllFnTable[ECMD_FAPI2OUTPUTINFO] == NULL) {
       fprintf(stderr,"dllFapi2OutputInfo%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
       ecmdDisplayDllInfo();
       exit(ECMD_DLL_INVALID);
     }
  }

  void (*Function)(const char*) = 
      (void(*)(const char*))fapi2DllFnTable[ECMD_FAPI2OUTPUTINFO];
   (*Function)(i_message);
#endif

}

void fapi2OutputImportant(const char* i_message) {

#ifndef ECMD_STATIC_FUNCTIONS
  if (dlHandle == NULL) {
    fprintf(stderr,"dllFapi2OutputImportant%s",ECMD_DLL_NOT_LOADED_ERROR);
    exit(ECMD_DLL_INVALID);
  }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
  dllFapi2OutputImportant(i_message);
#else
  if (fapi2DllFnTable[ECMD_FAPI2OUTPUTIMPORTANT] == NULL) {
     fapi2DllFnTable[ECMD_FAPI2OUTPUTIMPORTANT] = (void*)dlsym(dlHandle, "dllFapi2OutputImportant");
     if (fapi2DllFnTable[ECMD_FAPI2OUTPUTIMPORTANT] == NULL) {
       fprintf(stderr,"dllFapi2OutputImportant%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
       ecmdDisplayDllInfo();
       exit(ECMD_DLL_INVALID);
     }
  }

  void (*Function)(const char*) = 
      (void(*)(const char*))fapi2DllFnTable[ECMD_FAPI2OUTPUTIMPORTANT];
   (*Function)(i_message);
#endif

}

void fapi2OutputDebug(const char* i_message) {

#ifndef ECMD_STATIC_FUNCTIONS
  if (dlHandle == NULL) {
    fprintf(stderr,"dllFapi2OutputDebug%s",ECMD_DLL_NOT_LOADED_ERROR);
    exit(ECMD_DLL_INVALID);
  }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
  dllFapi2OutputDebug(i_message);
#else
  if (fapi2DllFnTable[ECMD_FAPI2OUTPUTDEBUG] == NULL) {
     fapi2DllFnTable[ECMD_FAPI2OUTPUTDEBUG] = (void*)dlsym(dlHandle, "dllFapi2OutputDebug");
     if (fapi2DllFnTable[ECMD_FAPI2OUTPUTDEBUG] == NULL) {
       fprintf(stderr,"dllFapi2OutputDebug%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
       ecmdDisplayDllInfo();
       exit(ECMD_DLL_INVALID);
     }
  }

  void (*Function)(const char*) = 
      (void(*)(const char*))fapi2DllFnTable[ECMD_FAPI2OUTPUTDEBUG];
   (*Function)(i_message);
#endif
}

void fapi2OutputManufacturing(const char* i_message) {

#ifndef ECMD_STATIC_FUNCTIONS
  if (dlHandle == NULL) {
    fprintf(stderr,"dllFapi2OutputManufacturing%s",ECMD_DLL_NOT_LOADED_ERROR);
    exit(ECMD_DLL_INVALID);
  }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
  dllFapi2OutputManufacturing(i_message);
#else
  if (fapi2DllFnTable[ECMD_FAPI2OUTPUTMANUFACTURING] == NULL) {
     fapi2DllFnTable[ECMD_FAPI2OUTPUTMANUFACTURING] = (void*)dlsym(dlHandle, "dllFapi2OutputManufacturing");
     if (fapi2DllFnTable[ECMD_FAPI2OUTPUTMANUFACTURING] == NULL) {
       fprintf(stderr,"dllFapi2OutputManufacturing%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
       ecmdDisplayDllInfo();
       exit(ECMD_DLL_INVALID);
     }
  }

  void (*Function)(const char*) = 
      (void(*)(const char*))fapi2DllFnTable[ECMD_FAPI2OUTPUTMANUFACTURING];
   (*Function)(i_message);
#endif
}

void fapi2OutputScanTrace(const char* i_message) {

#ifndef ECMD_STATIC_FUNCTIONS
  if (dlHandle == NULL) {
    fprintf(stderr,"dllFapi2OutputScanTrace%s",ECMD_DLL_NOT_LOADED_ERROR);
    exit(ECMD_DLL_INVALID);
  }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
  dllFapi2OutputScanTrace(i_message);
#else
  if (fapi2DllFnTable[ECMD_FAPI2OUTPUTSCANTRACE] == NULL) {
     fapi2DllFnTable[ECMD_FAPI2OUTPUTSCANTRACE] = (void*)dlsym(dlHandle, "dllFapi2OutputScanTrace");
     if (fapi2DllFnTable[ECMD_FAPI2OUTPUTSCANTRACE] == NULL) {
       fprintf(stderr,"dllFapi2OutputScanTrace%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
       ecmdDisplayDllInfo();
       exit(ECMD_DLL_INVALID);
     }
  }

  void (*Function)(const char*) = 
      (void(*)(const char*))fapi2DllFnTable[ECMD_FAPI2OUTPUTSCANTRACE];
   (*Function)(i_message);
#endif
}

char* fapi2FixOutputFormat(char* o_message, const char* i_message, size_t i_num){

  char * rc = NULL;
#ifndef ECMD_STATIC_FUNCTIONS
  if (dlHandle == NULL) {
    fprintf(stderr,"dllFapi2FixOutputFormat%s",ECMD_DLL_NOT_LOADED_ERROR);
    exit(ECMD_DLL_INVALID);
  }
#endif

#ifndef ECMD_STRIP_DEBUG
  int myTcount;
  std::vector< void * > args;
  if (ecmdClientDebug != 0) {
     args.push_back((void*) &o_message);
     args.push_back((void*) &i_message);
     args.push_back((void*) &i_num);
     fppCallCount++;
     myTcount = fppCallCount;
     ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"char* fapiFixOutputFormat(char* o_message, const char* i_message, size_t i_num)",args);
     ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"fapiFixOutputFormat");
  }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
  rc = dllFapi2FixOutputFormat(o_message, i_message, i_num);
#else
  if (fapi2DllFnTable[ECMD_FAPI2FIXOUTPUTFORMAT] == NULL) {
     fapi2DllFnTable[ECMD_FAPI2FIXOUTPUTFORMAT] = (void*)dlsym(dlHandle, "dllFapi2FixOutputFormat");
     if (fapi2DllFnTable[ECMD_FAPI2FIXOUTPUTFORMAT] == NULL) {
       fprintf(stderr,"dllFapi2FixOutputFormat%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
       ecmdDisplayDllInfo();
       exit(ECMD_DLL_INVALID);
     }
  }

  char* (*Function)(char*, const char*, size_t) = 
      (char*(*)(char*, const char*, size_t))fapi2DllFnTable[ECMD_FAPI2FIXOUTPUTFORMAT];
  rc = (*Function)(o_message, i_message, i_num);
#endif

#ifndef ECMD_STRIP_DEBUG
  if (ecmdClientDebug != 0) {
     args.push_back((void*) &rc);
     ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"char* fapiFixOutputFormat(char* o_message, const char* i_message, size_t i_num)",args);
     ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"fapiFixOutputFormat");
   }
#endif

  return rc;
}

bool fapi2IsOutputInfoEnabled(){

  bool rc;

#ifndef ECMD_STATIC_FUNCTIONS
  if (dlHandle == NULL) {
    fprintf(stderr,"dllFapi2IsOutputInfoEnabled%s",ECMD_DLL_NOT_LOADED_ERROR);
    exit(ECMD_DLL_INVALID);
  }
#endif

if (!fapi2Initialized) {
   fprintf(stderr,"dllFapi2IsOutputInfoEnabled: eCMD Extension not initialized before function called\n");
   fprintf(stderr,"dllFapi2IsOutputInfoEnabled: OR eCMD fapi2 Extension not supported by plugin\n");
   exit(ECMD_DLL_INVALID);
}

#ifndef ECMD_STRIP_DEBUG
  int myTcount;
  std::vector< void * > args;
  if (ecmdClientDebug != 0) {
     fppCallCount++;
     myTcount = fppCallCount;
     ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"",args);
     ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"fapi2IsOutputInfoEnabled");
  }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
  rc = dllFapi2IsOutputInfoEnabled();
#else
  if (fapi2DllFnTable[ECMD_FAPI2ISOUTPUTINFOENABLED] == NULL) {
     fapi2DllFnTable[ECMD_FAPI2ISOUTPUTINFOENABLED] = (void*)dlsym(dlHandle, "dllFapi2IsOutputInfoEnabled");
     if (fapi2DllFnTable[ECMD_FAPI2ISOUTPUTINFOENABLED] == NULL) {
       fprintf(stderr,"dllFapi2IsOutputInfoEnabled%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
       ecmdDisplayDllInfo();
       exit(ECMD_DLL_INVALID);
     }
  }

  bool (*Function)() = 
      (bool (*)())fapi2DllFnTable[ECMD_FAPI2ISOUTPUTINFOENABLED];
  rc =    (*Function)();
#endif

#ifndef ECMD_STRIP_DEBUG
  if (ecmdClientDebug != 0) {
     ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"fapi2IsOutputInfoEnabled");
     ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"", args);
   }
#endif

  if (rc && !ecmdGetGlobalVar(ECMD_GLOBALVAR_QUIETERRORMODE)) {
    std::string errorString;
    errorString = ecmdGetErrorMsg(rc, false, ecmdGetGlobalVar(ECMD_GLOBALVAR_CMDLINEMODE), false);
    if (errorString.size()) ecmdOutput(errorString.c_str());
  }

  return rc;
}

bool fapi2IsOutputDebugEnabled(){

  bool rc;

#ifndef ECMD_STATIC_FUNCTIONS
  if (dlHandle == NULL) {
    fprintf(stderr,"dllFapi2IsOutputDebugEnabled%s",ECMD_DLL_NOT_LOADED_ERROR);
    exit(ECMD_DLL_INVALID);
  }
#endif

if (!fapi2Initialized) {
   fprintf(stderr,"dllFapi2IsOutputDebugEnabled: eCMD Extension not initialized before function called\n");
   fprintf(stderr,"dllFapi2IsOutputDebugEnabled: OR eCMD fapi2 Extension not supported by plugin\n");
   exit(ECMD_DLL_INVALID);
}

#ifndef ECMD_STRIP_DEBUG
  int myTcount;
  std::vector< void * > args;
  if (ecmdClientDebug != 0) {
     fppCallCount++;
     myTcount = fppCallCount;
     ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"",args);
     ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"fapi2IsOutputDebugEnabled");
  }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
  rc = dllFapi2IsOutputDebugEnabled();
#else
  if (fapi2DllFnTable[ECMD_FAPI2ISOUTPUTDEBUGENABLED] == NULL) {
     fapi2DllFnTable[ECMD_FAPI2ISOUTPUTDEBUGENABLED] = (void*)dlsym(dlHandle, "dllFapi2IsOutputDebugEnabled");
     if (fapi2DllFnTable[ECMD_FAPI2ISOUTPUTDEBUGENABLED] == NULL) {
       fprintf(stderr,"dllFapi2IsOutputDebugEnabled%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
       ecmdDisplayDllInfo();
       exit(ECMD_DLL_INVALID);
     }
  }

  bool (*Function)() = 
      (bool (*)())fapi2DllFnTable[ECMD_FAPI2ISOUTPUTDEBUGENABLED];
  rc =    (*Function)();
#endif

#ifndef ECMD_STRIP_DEBUG
  if (ecmdClientDebug != 0) {
     ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"fapi2IsOutputDebugEnabled");
     ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"", args);
   }
#endif

  if (rc && !ecmdGetGlobalVar(ECMD_GLOBALVAR_QUIETERRORMODE)) {
    std::string errorString;
    errorString = ecmdGetErrorMsg(rc, false, ecmdGetGlobalVar(ECMD_GLOBALVAR_CMDLINEMODE), false);
    if (errorString.size()) ecmdOutput(errorString.c_str());
  }

  return rc;
}

bool fapi2IsOutputManufacturingEnabled(){

  bool rc;

#ifndef ECMD_STATIC_FUNCTIONS
  if (dlHandle == NULL) {
    fprintf(stderr,"dllFapi2IsOutputManufacturingEnabled%s",ECMD_DLL_NOT_LOADED_ERROR);
    exit(ECMD_DLL_INVALID);
  }
#endif

if (!fapi2Initialized) {
   fprintf(stderr,"dllFapi2IsOutputManufacturingEnabled: eCMD Extension not initialized before function called\n");
   fprintf(stderr,"dllFapi2IsOutputManufacturingEnabled: OR eCMD fapi2 Extension not supported by plugin\n");
   exit(ECMD_DLL_INVALID);
}

#ifndef ECMD_STRIP_DEBUG
  int myTcount;
  std::vector< void * > args;
  if (ecmdClientDebug != 0) {
     fppCallCount++;
     myTcount = fppCallCount;
     ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"",args);
     ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"fapi2IsOutputManufacturingEnabled");
  }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
  rc = dllFapi2IsOutputManufacturingEnabled();
#else
  if (fapi2DllFnTable[ECMD_FAPI2ISOUTPUTMANUFACTURINGENABLED] == NULL) {
     fapi2DllFnTable[ECMD_FAPI2ISOUTPUTMANUFACTURINGENABLED] = (void*)dlsym(dlHandle, "dllFapi2IsOutputManufacturingEnabled");
     if (fapi2DllFnTable[ECMD_FAPI2ISOUTPUTMANUFACTURINGENABLED] == NULL) {
       fprintf(stderr,"dllFapi2IsOutputManufacturingEnabled%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
       ecmdDisplayDllInfo();
       exit(ECMD_DLL_INVALID);
     }
  }

  bool (*Function)() = 
      (bool (*)())fapi2DllFnTable[ECMD_FAPI2ISOUTPUTMANUFACTURINGENABLED];
  rc =    (*Function)();
#endif

#ifndef ECMD_STRIP_DEBUG
  if (ecmdClientDebug != 0) {
     ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"fapi2IsOutputManufacturingEnabled");
     ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"", args);
   }
#endif

  if (rc && !ecmdGetGlobalVar(ECMD_GLOBALVAR_QUIETERRORMODE)) {
    std::string errorString;
    errorString = ecmdGetErrorMsg(rc, false, ecmdGetGlobalVar(ECMD_GLOBALVAR_CMDLINEMODE), false);
    if (errorString.size()) ecmdOutput(errorString.c_str());
  }

  return rc;
}

bool fapi2IsOutputLabEnabled(){

  bool rc;

#ifndef ECMD_STATIC_FUNCTIONS
  if (dlHandle == NULL) {
    fprintf(stderr,"dllFapi2IsOutputLabEnabled%s",ECMD_DLL_NOT_LOADED_ERROR);
    exit(ECMD_DLL_INVALID);
  }
#endif

if (!fapi2Initialized) {
   fprintf(stderr,"dllFapi2IsOutputLabEnabled: eCMD Extension not initialized before function called\n");
   fprintf(stderr,"dllFapi2IsOutputLabEnabled: OR eCMD fapi2 Extension not supported by plugin\n");
   exit(ECMD_DLL_INVALID);
}

#ifndef ECMD_STRIP_DEBUG
  int myTcount;
  std::vector< void * > args;
  if (ecmdClientDebug != 0) {
     fppCallCount++;
     myTcount = fppCallCount;
     ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"",args);
     ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"fapi2IsOutputManufacturingEnabled");
  }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
  rc = dllFapi2IsOutputLabEnabled();
#else
  if (fapi2DllFnTable[ECMD_FAPI2ISOUTPUTLABENABLED] == NULL) {
     fapi2DllFnTable[ECMD_FAPI2ISOUTPUTLABENABLED] = (void*)dlsym(dlHandle, "dllFapi2IsOutputLabEnabled");
     if (fapi2DllFnTable[ECMD_FAPI2ISOUTPUTLABENABLED] == NULL) {
       fprintf(stderr,"dllFapi2IsOutputLabEnabled%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
       ecmdDisplayDllInfo();
       exit(ECMD_DLL_INVALID);
     }
  }

  bool (*Function)() = 
      (bool (*)())fapi2DllFnTable[ECMD_FAPI2ISOUTPUTLABENABLED];
  rc =    (*Function)();
#endif

#ifndef ECMD_STRIP_DEBUG
  if (ecmdClientDebug != 0) {
     ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"fapi2IsOutputLabEnabled");
     ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"", args);
   }
#endif

  if (rc && !ecmdGetGlobalVar(ECMD_GLOBALVAR_QUIETERRORMODE)) {
    std::string errorString;
    errorString = ecmdGetErrorMsg(rc, false, ecmdGetGlobalVar(ECMD_GLOBALVAR_CMDLINEMODE), false);
    if (errorString.size()) ecmdOutput(errorString.c_str());
  }

  return rc;
}

void fapi2OutputLab(const char* i_message) {

#ifndef ECMD_STATIC_FUNCTIONS
  if (dlHandle == NULL) {
    fprintf(stderr,"dllFapi2OutputLab%s",ECMD_DLL_NOT_LOADED_ERROR);
    exit(ECMD_DLL_INVALID);
  }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
  dllFapi2OutputLab(i_message);
#else
  if (fapi2DllFnTable[ECMD_FAPI2OUTPUTLAB] == NULL) {
     fapi2DllFnTable[ECMD_FAPI2OUTPUTLAB] = (void*)dlsym(dlHandle, "dllFapi2OutputLab");
     if (fapi2DllFnTable[ECMD_FAPI2OUTPUTLAB] == NULL) {
       fprintf(stderr,"dllFapi2OutputLab%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
       ecmdDisplayDllInfo();
       exit(ECMD_DLL_INVALID);
     }
  }

  void (*Function)(const char*) = 
      (void(*)(const char*))fapi2DllFnTable[ECMD_FAPI2OUTPUTLAB];
   (*Function)(i_message);
#endif
}
