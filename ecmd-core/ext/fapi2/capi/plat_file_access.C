//IBM_PROLOG_BEGIN_TAG
/* 
 * Copyright 2018 IBM International Business Machines Corp.
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
#include <plat_file_access.H> 

#include <stdio.h>

static const char ECMD_DLL_NOT_LOADED_ERROR[] = ": eCMD Function called before DLL has been loaded\n";
static const char ECMD_UNABLE_TO_FIND_FUNCTION_ERROR[] = ": Unable to find function, must be an invalid DLL - program aborting\n";

#ifndef ECMD_STATIC_FUNCTIONS
#include <fapi2ClientEnums.H>
#include <dlfcn.h>
extern void * dlHandle;
extern void * fapi2DllFnTable[];
#else
#include <fapi2DllCapi.H>
#endif

extern bool fapi2Initialized;
#ifndef ECMD_STRIP_DEBUG
extern int ecmdClientDebug;
extern int fppCallCount;
extern bool ecmdDebugOutput;
#endif

namespace fapi2plat
{

/***********  Old versions  **************/
// These can possibly be removed in next release
fapi2::ReturnCode loadEmbeddedFile(const ecmdChipTarget& i_target,
                         const char* i_path,
                         void*& o_data,
                         size_t& o_size)
{
    fapi2::ReturnCode rc(fapi2::FAPI2_RC_SUCCESS);
    const void* l_data = NULL;
    // Call new version
    rc = loadEmbeddedFile(i_target, i_path, l_data, o_size, 0);
    o_data = const_cast<void*>(l_data);
    return rc;
}

fapi2::ReturnCode freeEmbeddedFile(void* i_data)
{
    const void* l_data = i_data;
    return freeEmbeddedFile(l_data);
}


/***********  Current versions  **************/
fapi2::ReturnCode loadEmbeddedFile(const ecmdChipTarget& i_target,
                         const char* i_path,
                         const void*& o_data,
                         size_t& o_size,
                         uint32_t i_flags)
{
    const char * dllFunctionName = "dllFapi2LoadEmbeddedFile";
    fapi2::ReturnCode rc(fapi2::FAPI2_RC_SUCCESS);
    uint32_t l_ecmdRc;

#ifndef ECMD_STATIC_FUNCTIONS
    if (dlHandle == NULL)
    {
        fprintf(stderr,"%s%s",dllFunctionName,ECMD_DLL_NOT_LOADED_ERROR);
        exit(ECMD_DLL_INVALID);
    }
#endif

    if (!fapi2Initialized)
    {
        fprintf(stderr,"%s: eCMD Extension not initialized before function called\n", dllFunctionName);
        fprintf(stderr,"%s: OR eCMD fapi Extension not supported by plugin\n", dllFunctionName);
        exit(ECMD_DLL_INVALID);
    }

#ifndef ECMD_STRIP_DEBUG
    int myTcount;
    std::vector< void * > args;
    if (ecmdClientDebug != 0)
    {
        args.push_back((void*) &i_target);
        args.push_back((void*) &i_path);
        args.push_back((void*) &o_data);
        args.push_back((void*) &o_size);
        args.push_back((void*) &i_flags);
        fppCallCount++;
        myTcount = fppCallCount;
        ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"uint32_t loadEmbeddedFile(const ecmdChipTarget & i_target, const char* i_path, const void*& o_data, size_t& o_size, uint32_t i_flags)",args);
        ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"loadEmbeddedFile");
    }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
    l_ecmdRc = dllFapi2LoadEmbeddedFile(i_target, i_path, o_data, o_size, i_flags);
#else
    if (fapi2DllFnTable[ECMD_FAPI2LOADEMBEDDEDFILE] == NULL)
    {
        fapi2DllFnTable[ECMD_FAPI2LOADEMBEDDEDFILE] = (void*)dlsym(dlHandle, dllFunctionName);
        if (fapi2DllFnTable[ECMD_FAPI2LOADEMBEDDEDFILE] == NULL)
        {
            fprintf(stderr,"%s%s",dllFunctionName,ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
            ecmdDisplayDllInfo();
            exit(ECMD_DLL_INVALID);
        }
    }

    uint32_t (*Function)(const ecmdChipTarget&, const char*, const void*&, size_t&, uint32_t) = 
        (uint32_t(*)(const ecmdChipTarget&, const char*, const void*&, size_t&, uint32_t))fapi2DllFnTable[ECMD_FAPI2LOADEMBEDDEDFILE];
    l_ecmdRc = (*Function)(i_target, i_path, o_data, o_size, i_flags);
#endif
    if (l_ecmdRc)
    {
        rc = (fapi2::ReturnCodes) l_ecmdRc; 
    }

#ifndef ECMD_STRIP_DEBUG
    if (ecmdClientDebug != 0)
    {
        args.push_back((void*) &l_ecmdRc);
        ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"loadEmbeddedFile");
        ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t loadEmbeddedFile(const ecmdChipTarget & i_target, const char* i_path, const void*& o_data, size_t& o_size, uint32_t i_flags)",args);
    }
#endif

    return rc;
}

fapi2::ReturnCode freeEmbeddedFile(const void* i_data)
{
    const char * dllFunctionName = "dllFapi2FreeEmbeddedFile";
    fapi2::ReturnCode rc(fapi2::FAPI2_RC_SUCCESS);
    uint32_t l_ecmdRc;

#ifndef ECMD_STATIC_FUNCTIONS
    if (dlHandle == NULL)
    {
        fprintf(stderr,"%s%s",dllFunctionName,ECMD_DLL_NOT_LOADED_ERROR);
        exit(ECMD_DLL_INVALID);
    }
#endif

    if (!fapi2Initialized)
    {
        fprintf(stderr,"%s: eCMD Extension not initialized before function called\n", dllFunctionName);
        fprintf(stderr,"%s: OR eCMD fapi Extension not supported by plugin\n", dllFunctionName);
        exit(ECMD_DLL_INVALID);
    }

#ifndef ECMD_STRIP_DEBUG
    int myTcount;
    std::vector< void * > args;
    if (ecmdClientDebug != 0)
    {
        args.push_back((void*) &i_data);
        fppCallCount++;
        myTcount = fppCallCount;
        ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"uint32_t freeEmbeddedFile(const void* i_data)",args);
        ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"freeEmbeddedFile");
    }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
    l_ecmdRc = dllFapi2FreeEmbeddedFile(i_data);
#else
    if (fapi2DllFnTable[ECMD_FAPI2FREEEMBEDDEDFILE] == NULL)
    {
        fapi2DllFnTable[ECMD_FAPI2FREEEMBEDDEDFILE] = (void*)dlsym(dlHandle, dllFunctionName);
        if (fapi2DllFnTable[ECMD_FAPI2FREEEMBEDDEDFILE] == NULL)
        {
            fprintf(stderr,"%s%s",dllFunctionName,ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
            ecmdDisplayDllInfo();
            exit(ECMD_DLL_INVALID);
        }
    }

    uint32_t (*Function)(const void*) = 
        (uint32_t(*)(const void*))fapi2DllFnTable[ECMD_FAPI2FREEEMBEDDEDFILE];
    l_ecmdRc = (*Function)(i_data);
#endif
    if (l_ecmdRc)
    {
        rc = (fapi2::ReturnCodes) l_ecmdRc; 
    }

#ifndef ECMD_STRIP_DEBUG
    if (ecmdClientDebug != 0)
    {
        args.push_back((void*) &l_ecmdRc);
        ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"freeEmbeddedFile");
        ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t freeEmbeddedFile(void* i_data)",args);
    }
#endif

    return rc;

}

};

