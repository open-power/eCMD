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


#include <stdio.h>

#include <ecmdClientCapi.H>
#include <ecmdSharedUtils.H>
#include <fapi2ClientCapi.H>
#ifndef ECMD_STATIC_FUNCTIONS
#include <fapi2ClientEnums.H>
#include <target.H>
#endif

#include <ecmdUtils.H>

static const char ECMD_DLL_NOT_LOADED_ERROR[] = ": eCMD Function called before DLL has been loaded\n";
static const char ECMD_UNABLE_TO_FIND_FUNCTION_ERROR[] = ": Unable to find function, must be an invalid DLL - program aborting\n";

#ifndef ECMD_STATIC_FUNCTIONS

 #include <dlfcn.h>

/* This is from ecmdClientCapiFunc.C */
 extern void * dlHandle;
 extern void * fapi2DllFnTable[FAPI2_NUMFUNCTIONS];

#else

 #include <fapi2DllCapi.H>

#endif

extern bool fapi2Initialized; 

#ifndef ECMD_STRIP_DEBUG
extern int ecmdClientDebug;
extern int fppCallCount;
extern bool ecmdDebugOutput;
#endif

uint32_t fapi2GetAssociatedTargets( const ecmdChipTarget & i_target, const fapi2::TargetType i_associatedTargetType, std::list<const ecmdChipTarget *> & o_targets, const fapi2::TargetState i_state)
{
    uint32_t l_rc;

#ifndef ECMD_STATIC_FUNCTIONS
    if (dlHandle == NULL)
    {
        fprintf(stderr,"dllFapi2GetAssociatedTargets%s",ECMD_DLL_NOT_LOADED_ERROR);
        exit(ECMD_DLL_INVALID);
    }
#endif

    if (!fapi2Initialized)
    {
        fprintf(stderr,"dllFapi2GetAssociatedTargets: eCMD Extension not initialized before function called\n");
        fprintf(stderr,"dllFapi2GetAssociatedTargets: OR eCMD fapi2 Extension not supported by plugin\n");
        exit(ECMD_DLL_INVALID);
    }

#ifndef ECMD_STRIP_DEBUG
    int myTcount;
    std::vector< void * > args;
    if (ecmdClientDebug != 0)
    {
        args.push_back((void*) &i_target);
        args.push_back((void*) &i_associatedTargetType);
        args.push_back((void*) &o_targets);
        args.push_back((void*) &i_state);
        fppCallCount++;
        myTcount = fppCallCount;
        ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"uint32_t fapi2GetAssociatedTargets(const ecmdChipTarget & i_target, const fapi2::TargetType i_associatedTargetType, std::list<const ecmdChipTarget *> & o_targets, const fapi2::TargetState i_state)",args);
        ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"fapi2GetAssociatedTargets");
    }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
    l_rc = dllFapi2GetAssociatedTargets(i_target, i_associatedTargetType, o_targets, i_state);
#else
    if (fapi2DllFnTable[ECMD_FAPI2GETASSOCIATEDTARGETS] == NULL)
    {
        fapi2DllFnTable[ECMD_FAPI2GETASSOCIATEDTARGETS] = (void*)dlsym(dlHandle, "dllFapi2GetAssociatedTargets");
        if (fapi2DllFnTable[ECMD_FAPI2GETASSOCIATEDTARGETS] == NULL)
        {
            fprintf(stderr,"dllFapi2GetAssociatedTargets%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
            ecmdDisplayDllInfo();
            exit(ECMD_DLL_INVALID);
        }
    }

    uint32_t (*Function)(const ecmdChipTarget &, const fapi2::TargetType, std::list<const ecmdChipTarget *> &, const fapi2::TargetState) = 
        (uint32_t(*)(const ecmdChipTarget &, const fapi2::TargetType, std::list<const ecmdChipTarget *> &, const fapi2::TargetState ))fapi2DllFnTable[ECMD_FAPI2GETASSOCIATEDTARGETS];
    l_rc = (*Function)(i_target, i_associatedTargetType, o_targets, i_state);
#endif

#ifndef ECMD_STRIP_DEBUG
    if (ecmdClientDebug != 0)
    {
        args.push_back((void*) &l_rc);
        ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"fapi2GetAssociatedTargets");
        ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t fapi2GetAssociatedTargets(const ecmdChipTarget & i_target, const fapi2::TargetType i_associatedTargetType, std::list<const ecmdChipTarget *> & o_targets, const fapi2::TargetState i_state)",args);
    }
#endif

    return l_rc;
}

uint32_t fapi2GetFilteredTargets( const ecmdChipTarget & i_target, const fapi2::TargetType i_associatedTargetType, std::list<const ecmdChipTarget *> & o_targets, const fapi2::TargetFilter i_filter, const fapi2::TargetState i_state)
{
    uint32_t l_rc;

#ifndef ECMD_STATIC_FUNCTIONS
    if (dlHandle == NULL)
    {
        fprintf(stderr,"dllFapi2GetFilteredTargets%s",ECMD_DLL_NOT_LOADED_ERROR);
        exit(ECMD_DLL_INVALID);
    }
#endif

    if (!fapi2Initialized)
    {
        fprintf(stderr,"dllFapi2GetFilteredTargets: eCMD Extension not initialized before function called\n");
        fprintf(stderr,"dllFapi2GetFilteredTargets: OR eCMD fapi2 Extension not supported by plugin\n");
        exit(ECMD_DLL_INVALID);
    }

#ifndef ECMD_STRIP_DEBUG
    int myTcount;
    std::vector< void * > args;
    if (ecmdClientDebug != 0)
    {
        args.push_back((void*) &i_target);
        args.push_back((void*) &i_associatedTargetType);
        args.push_back((void*) &o_targets);
        args.push_back((void*) &i_state);
        fppCallCount++;
        myTcount = fppCallCount;
        ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"uint32_t fapi2GetFilteredTargets(const ecmdChipTarget & i_target, const fapi2::TargetType i_associatedTargetType, std::list<const ecmdChipTarget *> & o_targets, const fapi2::TargetFilter i_filter, const fapi2::TargetState i_state)",args);
        ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"fapi2GetFilteredTargets");
    }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
    l_rc = dllFapi2GetFilteredTargets(i_target, i_associatedTargetType, o_targets, i_filter, i_state);
#else
    if (fapi2DllFnTable[ECMD_FAPI2GETFILTEREDTARGETS] == NULL)
    {
        fapi2DllFnTable[ECMD_FAPI2GETFILTEREDTARGETS] = (void*)dlsym(dlHandle, "dllFapi2GetFilteredTargets");
        if (fapi2DllFnTable[ECMD_FAPI2GETFILTEREDTARGETS] == NULL)
        {
            fprintf(stderr,"dllFapi2GetFilteredTargets%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
            ecmdDisplayDllInfo();
            exit(ECMD_DLL_INVALID);
        }
    }

    uint32_t (*Function)(const ecmdChipTarget &, const fapi2::TargetType, std::list<const ecmdChipTarget *> &, const fapi2::TargetFilter, const fapi2::TargetState) = 
        (uint32_t(*)(const ecmdChipTarget &, const fapi2::TargetType, std::list<const ecmdChipTarget *> &, const fapi2::TargetFilter, const fapi2::TargetState ))fapi2DllFnTable[ECMD_FAPI2GETFILTEREDTARGETS];
    l_rc = (*Function)(i_target, i_associatedTargetType, o_targets, i_filter, i_state);
#endif

#ifndef ECMD_STRIP_DEBUG
    if (ecmdClientDebug != 0)
    {
        args.push_back((void*) &l_rc);
        ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"fapi2GetFilteredTargets");
        ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t fapi2GetFilteredTargets(const ecmdChipTarget & i_target, const fapi2::TargetType i_associatedTargetType, std::list<const ecmdChipTarget *> & o_targets, const fapi2::TargetFilter i_filter, const fapi2::TargetState i_state)",args);
    }
#endif

    return l_rc;
}

uint32_t fapi2GetTargetType( const ecmdChipTarget & i_target, fapi2::TargetType & o_targetType)
{
    uint32_t l_rc;

#ifndef ECMD_STATIC_FUNCTIONS
    if (dlHandle == NULL)
    {
        fprintf(stderr,"dllFapi2GetTargetType%s",ECMD_DLL_NOT_LOADED_ERROR);
        exit(ECMD_DLL_INVALID);
    }
#endif

    if (!fapi2Initialized)
    {
        fprintf(stderr,"dllFapi2GetTargetType: eCMD Extension not initialized before function called\n");
        fprintf(stderr,"dllFapi2GetTargetType: OR eCMD fapi2 Extension not supported by plugin\n");
        exit(ECMD_DLL_INVALID);
    }

#ifndef ECMD_STRIP_DEBUG
    int myTcount;
    std::vector< void * > args;
    if (ecmdClientDebug != 0)
    {
        args.push_back((void*) &i_target);
        args.push_back((void*) &o_targetType);
        fppCallCount++;
        myTcount = fppCallCount;
        ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"uint32_t fapi2GetTargetType(const ecmdChipTarget & i_target, fapi2::TargetType & o_targetType)",args);
        ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"fapi2GetTargetType");
    }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
    l_rc = dllFapi2GetTargetType(i_target, o_targetType);
#else
    if (fapi2DllFnTable[ECMD_FAPI2GETTARGETTYPE] == NULL)
    {
        fapi2DllFnTable[ECMD_FAPI2GETTARGETTYPE] = (void*)dlsym(dlHandle, "dllFapi2GetTargetType");
        if (fapi2DllFnTable[ECMD_FAPI2GETTARGETTYPE] == NULL)
        {
            fprintf(stderr,"dllFapi2GetTargetType%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
            ecmdDisplayDllInfo();
            exit(ECMD_DLL_INVALID);
        }
    }

    uint32_t (*Function)(const ecmdChipTarget &, fapi2::TargetType &) =
        (uint32_t(*)(const ecmdChipTarget &, fapi2::TargetType &))fapi2DllFnTable[ECMD_FAPI2GETTARGETTYPE];
    l_rc = (*Function)(i_target, o_targetType);
#endif

#ifndef ECMD_STRIP_DEBUG
    if (ecmdClientDebug != 0)
    {
        args.push_back((void*) &l_rc);
        ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"fapi2GetTargetType");
        ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t fapi2GetTargetType(const ecmdChipTarget & i_target, fapi2::TargetType & o_targetType)",args);
    }
#endif

    return l_rc;
}

uint32_t fapi2GeneralApi(std::list<void *> & io_args)
{
    uint32_t l_rc;

#ifndef ECMD_STATIC_FUNCTIONS
    if (dlHandle == NULL)
    {
        fprintf(stderr,"dllFapi2GeneralApi%s",ECMD_DLL_NOT_LOADED_ERROR);
        exit(ECMD_DLL_INVALID);
    }
#endif

    if (!fapi2Initialized)
    {
        fprintf(stderr,"dllFapi2GeneralApi: eCMD Extension not initialized before function called\n");
        fprintf(stderr,"dllFapi2GeneralApi: OR eCMD fapi2 Extension not supported by plugin\n");
        exit(ECMD_DLL_INVALID);
    }

#ifndef ECMD_STRIP_DEBUG
    int myTcount;
    std::vector< void * > args;
    if (ecmdClientDebug != 0)
    {
        args.push_back((void*) &io_args);
        fppCallCount++;
        myTcount = fppCallCount;
        ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"uint32_t fapi2GeneralApi(std::list<void *> & io_args)",args);
        ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"fapi2GeneralApi");
    }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
    l_rc = dllFapi2GeneralApi(io_args);
#else
    if (fapi2DllFnTable[ECMD_FAPI2GENERALAPI] == NULL)
    {
        fapi2DllFnTable[ECMD_FAPI2GENERALAPI] = (void*)dlsym(dlHandle, "dllFapi2GeneralApi");
        if (fapi2DllFnTable[ECMD_FAPI2GENERALAPI] == NULL)
        {
            fprintf(stderr,"dllFapi2GeneralApi%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
            ecmdDisplayDllInfo();
            exit(ECMD_DLL_INVALID);
        }
    }

    uint32_t (*Function)(std::list<void *> &) =
        (uint32_t(*)(std::list<void *> & io_args))fapi2DllFnTable[ECMD_FAPI2GENERALAPI];
    l_rc = (*Function)(io_args);
#endif

#ifndef ECMD_STRIP_DEBUG
    if (ecmdClientDebug != 0)
    {
        args.push_back((void*) &l_rc);
        ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"fapi2GeneralApi");
        ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t fapi2GeneralApi(std::list<void *> & io_args)",args);
    }
#endif

    return l_rc;
}
