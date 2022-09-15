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
#include <plat_hw_access.H> 

#include <stdio.h>

#include <ecmdClientCapi.H>
#include <ecmdSharedUtils.H>
#include <ecmdClientEnums.H>
#include <fapi2ClientCapi.H>

#include <mvpd_access.H> 
#include <fapi2SharedUtils.H> 
#include <variable_buffer_utils.H>

#ifndef ECMD_STATIC_FUNCTIONS
#include <fapi2ClientEnums.H>
#include <target.H>
#endif

#include <fapi2DllCapi.H>
#include <ecmdDllCapi.H>

#include <ecmdUtils.H>

static const char ECMD_DLL_NOT_LOADED_ERROR[] = ": eCMD Function called before DLL has been loaded\n";
static const char ECMD_UNABLE_TO_FIND_FUNCTION_ERROR[] = ": Unable to find function, must be an invalid DLL - program aborting\n";

#ifndef ECMD_STATIC_FUNCTIONS

#include <dlfcn.h>

/* This is from ecmdClientCapiFunc.C */
extern void * dlHandle;
extern void * DllFnTable[];
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

// Creating dummy RingID enum for legacy code
enum RingID
{
    DUMMY_VALUE = 0
};

namespace fapi2plat
{
    fapi2::ReturnCode getScom(const ecmdChipTarget& i_target,
                              const uint64_t i_address,
                              fapi2::buffer<uint64_t>& o_data)
    {
        fapi2::ReturnCode rc(fapi2::FAPI2_RC_SUCCESS);
        uint32_t l_ecmdRc;

        ecmdDataBuffer l_ecmd_buffer;

#ifndef ECMD_STATIC_FUNCTIONS
        if (dlHandle == NULL)
        {
            fprintf(stderr,"dllGetScom%s",ECMD_DLL_NOT_LOADED_ERROR);
            exit(ECMD_DLL_INVALID);
        }
#endif

#ifndef ECMD_STRIP_DEBUG
        int myTcount;
        std::vector< void * > args;
        if (ecmdClientDebug != 0)
        {
            args.push_back((void*) &i_target);
            args.push_back((void*) &i_address);
            args.push_back((void*) &l_ecmd_buffer);
            fppCallCount++;
            myTcount = fppCallCount;
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"uint32_t getScom(const ecmdChipTarget & i_target, uint64_t i_address, ecmdDataBuffer & o_data)",args);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"getScom");
        }
#endif

        ecmdChipTarget cacheTarget;
        cacheTarget = i_target;
        ecmdSetTargetDepth(cacheTarget, ECMD_DEPTH_CHIP);
        if (ecmdIsRingCacheEnabled(cacheTarget))
        {
            rc = (fapi2::ReturnCodes) ECMD_RING_CACHE_ENABLED;
            return rc;
        }
#ifdef ECMD_STATIC_FUNCTIONS
        l_ecmdRc = dllGetScom(i_target, i_address, l_ecmd_buffer); 
#else
        if (DllFnTable[ECMD_GETSCOM] == NULL)
        {
            DllFnTable[ECMD_GETSCOM] = (void*)dlsym(dlHandle, "dllGetScom");
            if (DllFnTable[ECMD_GETSCOM] == NULL)
            {
                fprintf(stderr,"dllGetScom%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
                ecmdDisplayDllInfo();
                exit(ECMD_DLL_INVALID);
            }
        }

        uint32_t (*Function)(const ecmdChipTarget &,  uint64_t,  ecmdDataBuffer &) = 
            (uint32_t(*)(const ecmdChipTarget &,  uint64_t,  ecmdDataBuffer &))DllFnTable[ECMD_GETSCOM];
        l_ecmdRc = (*Function)(i_target, i_address, l_ecmd_buffer);
#endif
        if (l_ecmdRc)
        {
            rc = (fapi2::ReturnCodes) l_ecmdRc; 
        }

#ifndef ECMD_STRIP_DEBUG
        if (ecmdClientDebug != 0)
        {
            args.push_back((void*) &l_ecmdRc);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"getScom");
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t getScom(const ecmdChipTarget & i_target, uint64_t i_address, ecmdDataBuffer & o_data)",args);
        }
#endif

        if (l_ecmdRc && !ecmdGetGlobalVar(ECMD_GLOBALVAR_QUIETERRORMODE))
        {
            std::string errorString;
            errorString = ecmdGetErrorMsg(l_ecmdRc, false, ecmdGetGlobalVar(ECMD_GLOBALVAR_CMDLINEMODE), false);
            if (errorString.size())
                ecmdOutput(errorString.c_str());
        }

        o_data = l_ecmd_buffer.getDoubleWord(0);

        return rc;
    }

    fapi2::ReturnCode putScom(const ecmdChipTarget & i_target, const uint64_t i_address, const fapi2::buffer<uint64_t> i_data) 
    {
        fapi2::ReturnCode rc;
        uint32_t l_ecmdRc;

        ecmdDataBuffer l_ecmd_buffer(64);
        l_ecmd_buffer.setDoubleWord(0, i_data);

#ifndef ECMD_STATIC_FUNCTIONS
        if (dlHandle == NULL)
        {
            fprintf(stderr,"dllPutScom%s",ECMD_DLL_NOT_LOADED_ERROR);
            exit(ECMD_DLL_INVALID);
        }
#endif

#ifndef ECMD_STRIP_DEBUG
        int myTcount;
        std::vector< void * > args;
        if (ecmdClientDebug != 0)
        {
            args.push_back((void*) &i_target);
            args.push_back((void*) &i_address);
            args.push_back((void*) &l_ecmd_buffer);
            fppCallCount++;
            myTcount = fppCallCount;
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"uint32_t putScom(const ecmdChipTarget & i_target, uint64_t i_address, const ecmdDataBuffer & i_data)",args);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"putScom");
        }
#endif

        ecmdChipTarget cacheTarget;
        cacheTarget = i_target;
        ecmdSetTargetDepth(cacheTarget, ECMD_DEPTH_CHIP);
        if (ecmdIsRingCacheEnabled(cacheTarget))
        {
            rc = (fapi2::ReturnCodes) ECMD_RING_CACHE_ENABLED;
            return rc;
        }
#ifdef ECMD_STATIC_FUNCTIONS
        l_ecmdRc = dllPutScom(i_target, i_address, l_ecmd_buffer); 
#else
        if (DllFnTable[ECMD_PUTSCOM] == NULL)
        {
            DllFnTable[ECMD_PUTSCOM] = (void*)dlsym(dlHandle, "dllPutScom");
            if (DllFnTable[ECMD_PUTSCOM] == NULL)
            {
                fprintf(stderr,"dllPutScom%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
                ecmdDisplayDllInfo();
                exit(ECMD_DLL_INVALID);
            }
        }

        uint32_t (*Function)(const ecmdChipTarget &,  uint64_t,  const ecmdDataBuffer &) = 
            (uint32_t(*)(const ecmdChipTarget &,  uint64_t,  const ecmdDataBuffer &))DllFnTable[ECMD_PUTSCOM];
        l_ecmdRc = (*Function)(i_target, i_address, l_ecmd_buffer);
#endif
        if (l_ecmdRc)
        {
            rc = (fapi2::ReturnCodes) l_ecmdRc;
        }

#ifndef ECMD_STRIP_DEBUG
        if (ecmdClientDebug != 0)
        {
            args.push_back((void*) &l_ecmdRc);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"putScom");
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t putScom(const ecmdChipTarget & i_target, uint64_t i_address, const ecmdDataBuffer & i_data)",args);
        }
#endif

        if (l_ecmdRc && !ecmdGetGlobalVar(ECMD_GLOBALVAR_QUIETERRORMODE))
        {
            std::string errorString;
            errorString = ecmdGetErrorMsg(l_ecmdRc, false, ecmdGetGlobalVar(ECMD_GLOBALVAR_CMDLINEMODE), false);
            if (errorString.size())
                ecmdOutput(errorString.c_str());
        }

        return rc;
    }

    fapi2::ReturnCode putScomUnderMask(const ecmdChipTarget & i_target, const uint64_t i_address, const fapi2::buffer<uint64_t> i_data, const fapi2::buffer<uint64_t> i_mask) 
    {
        fapi2::ReturnCode rc;
        uint32_t l_ecmdRc;
 
        ecmdDataBuffer l_ecmd_buffer(64), l_ecmd_buffer_mask(64);
        l_ecmd_buffer.setDoubleWord(0, i_data);
        l_ecmd_buffer_mask.setDoubleWord(0, i_mask);

#ifndef ECMD_STATIC_FUNCTIONS
        if (dlHandle == NULL)
        {
            fprintf(stderr,"dllPutScomUnderMask%s",ECMD_DLL_NOT_LOADED_ERROR);
            exit(ECMD_DLL_INVALID);
        }
#endif

        if (!fapi2Initialized)
        {
            fprintf(stderr,"dllPutScomUnderMask: eCMD Extension not initialized before function called\n");
            fprintf(stderr,"dllPutScomUnderMask: OR eCMD fapi Extension not supported by plugin\n");
            exit(ECMD_DLL_INVALID);
        }

#ifndef ECMD_STRIP_DEBUG
        int myTcount;
        std::vector< void * > args;
        if (ecmdClientDebug != 0)
        {
            args.push_back((void*) &i_target);
            args.push_back((void*) &i_address);
            args.push_back((void*) &l_ecmd_buffer);
            args.push_back((void*) &l_ecmd_buffer_mask);
            fppCallCount++;
            myTcount = fppCallCount;
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"uint32_t putScomUnderMask(const ecmdChipTarget & i_target, uint64_t i_address, const ecmdDataBuffer & i_data, const ecmdDataBuffer & i_mask)",args);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"putScomUnderMask");
        }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
        l_ecmdRc = dllPutScomUnderMask(i_target, i_address, l_ecmd_buffer, l_ecmd_buffer_mask);
#else
        if (DllFnTable[ECMD_PUTSCOMUNDERMASK] == NULL)
        {
            DllFnTable[ECMD_PUTSCOMUNDERMASK] = (void*)dlsym(dlHandle, "dllPutScomUnderMask");
            if (DllFnTable[ECMD_PUTSCOMUNDERMASK] == NULL)
            {
                fprintf(stderr,"dllPutScomUnderMask%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
                ecmdDisplayDllInfo();
                exit(ECMD_DLL_INVALID);
            }
        }

        uint32_t (*Function)(const ecmdChipTarget&, uint64_t, const ecmdDataBuffer &,  const ecmdDataBuffer &) = 
            (uint32_t(*)(const ecmdChipTarget&, uint64_t, const ecmdDataBuffer &,  const ecmdDataBuffer &))DllFnTable[ECMD_PUTSCOMUNDERMASK];
        l_ecmdRc = (*Function)(i_target, i_address, l_ecmd_buffer, l_ecmd_buffer_mask);
#endif
        if (l_ecmdRc)
        {
            rc = (fapi2::ReturnCodes) l_ecmdRc;
        }

#ifndef ECMD_STRIP_DEBUG
        if (ecmdClientDebug != 0)
        {
            args.push_back((void*) &l_ecmdRc);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"putScomUnderMask");
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t putScomUnderMask(const ecmdChipTarget & i_target, uint64_t i_address,  const ecmdDataBuffer & i_data, const ecmdDataBuffer & i_mask)",args);
        }
#endif

        return rc;
    }

    fapi2::ReturnCode getCfamRegister(const ecmdChipTarget& i_target,
                                      const uint32_t i_address,
                                      fapi2::buffer<uint32_t>& o_data)
    {
        fapi2::ReturnCode rc(fapi2::FAPI2_RC_SUCCESS);
        uint32_t l_ecmdRc;

        ecmdDataBuffer l_ecmd_buffer;

#ifndef ECMD_STATIC_FUNCTIONS
        if (dlHandle == NULL)
        {
            fprintf(stderr,"dllGetCfamRegister%s",ECMD_DLL_NOT_LOADED_ERROR);
            exit(ECMD_DLL_INVALID);
        }
#endif

#ifndef ECMD_STRIP_DEBUG
        int myTcount;
        std::vector< void * > args;
        if (ecmdClientDebug != 0)
        {
            args.push_back((void*) &i_target);
            args.push_back((void*) &i_address);
            args.push_back((void*) &l_ecmd_buffer);
            fppCallCount++;
            myTcount = fppCallCount;
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"uint32_t getCfamRegister(const ecmdChipTarget & i_target, uint32_t i_address, ecmdDataBuffer & o_data)",args);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"getCfamRegister");
        }
#endif

        ecmdChipTarget cacheTarget;
        cacheTarget = i_target;
        ecmdSetTargetDepth(cacheTarget, ECMD_DEPTH_CHIP);
        if (ecmdIsRingCacheEnabled(cacheTarget))
        {
            rc = (fapi2::ReturnCodes) ECMD_RING_CACHE_ENABLED;
            return rc;
        }
#ifdef ECMD_STATIC_FUNCTIONS
        l_ecmdRc = dllGetCfamRegister(i_target, i_address, l_ecmd_buffer); 
#else
        if (DllFnTable[ECMD_GETCFAMREGISTER] == NULL)
        {
            DllFnTable[ECMD_GETCFAMREGISTER] = (void*)dlsym(dlHandle, "dllGetCfamRegister");
            if (DllFnTable[ECMD_GETCFAMREGISTER] == NULL)
            {
                fprintf(stderr,"dllGetCfamRegister%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
                ecmdDisplayDllInfo();
                exit(ECMD_DLL_INVALID);
            }
        }

        uint32_t (*Function)(const ecmdChipTarget &,  uint32_t,  ecmdDataBuffer &) = 
            (uint32_t(*)(const ecmdChipTarget &,  uint32_t,  ecmdDataBuffer &))DllFnTable[ECMD_GETCFAMREGISTER];
        l_ecmdRc = (*Function)(i_target, i_address, l_ecmd_buffer);
#endif
        if (l_ecmdRc)
        {
            rc = (fapi2::ReturnCodes) l_ecmdRc; 
        }

#ifndef ECMD_STRIP_DEBUG
        if (ecmdClientDebug != 0)
        {
            args.push_back((void*) &l_ecmdRc);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"getCfamRegister");
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t getCfamRegister(const ecmdChipTarget & i_target, uint32_t i_address, ecmdDataBuffer & o_data)",args);
        }
#endif

        if (l_ecmdRc && !ecmdGetGlobalVar(ECMD_GLOBALVAR_QUIETERRORMODE))
        {
            std::string errorString;
            errorString = ecmdGetErrorMsg(l_ecmdRc, false, ecmdGetGlobalVar(ECMD_GLOBALVAR_CMDLINEMODE), false);
            if (errorString.size())
                ecmdOutput(errorString.c_str());
        }

        o_data = l_ecmd_buffer.getWord(0);

        return rc;
    }

    fapi2::ReturnCode putCfamRegister(const ecmdChipTarget& i_target,
                                      const uint32_t i_address,
                                      const fapi2::buffer<uint32_t> i_data)
    {
        fapi2::ReturnCode rc(fapi2::FAPI2_RC_SUCCESS);
        uint32_t l_ecmdRc;

        ecmdDataBuffer l_ecmd_buffer(32);
        l_ecmd_buffer.setWord(0, i_data);

#ifndef ECMD_STATIC_FUNCTIONS
        if (dlHandle == NULL)
        {
            fprintf(stderr,"dllPutCfamRegister%s",ECMD_DLL_NOT_LOADED_ERROR);
            exit(ECMD_DLL_INVALID);
        }
#endif

#ifndef ECMD_STRIP_DEBUG
        int myTcount;
        std::vector< void * > args;
        if (ecmdClientDebug != 0)
        {
            args.push_back((void*) &i_target);
            args.push_back((void*) &i_address);
            args.push_back((void*) &l_ecmd_buffer);
            fppCallCount++;
            myTcount = fppCallCount;
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"uint32_t putCfamRegister(const ecmdChipTarget & i_target, uint32_t i_address, const ecmdDataBuffer & i_data)",args);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"putCfamRegister");
        }
#endif

        ecmdChipTarget cacheTarget;
        cacheTarget = i_target;
        ecmdSetTargetDepth(cacheTarget, ECMD_DEPTH_CHIP);
        if (ecmdIsRingCacheEnabled(cacheTarget))
        {
            rc = (fapi2::ReturnCodes) ECMD_RING_CACHE_ENABLED;
            return rc;
        }
#ifdef ECMD_STATIC_FUNCTIONS
        l_ecmdRc = dllPutCfamRegister(i_target, i_address, l_ecmd_buffer); 
#else
        if (DllFnTable[ECMD_PUTCFAMREGISTER] == NULL)
        {
            DllFnTable[ECMD_PUTCFAMREGISTER] = (void*)dlsym(dlHandle, "dllPutCfamRegister");
            if (DllFnTable[ECMD_PUTCFAMREGISTER] == NULL)
            {
                fprintf(stderr,"dllPutCfamRegister%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
                ecmdDisplayDllInfo();
                exit(ECMD_DLL_INVALID);
            }
        }

        uint32_t (*Function)(const ecmdChipTarget &,  uint32_t,  const ecmdDataBuffer &) = 
            (uint32_t(*)(const ecmdChipTarget &,  uint32_t,  const ecmdDataBuffer &))DllFnTable[ECMD_PUTCFAMREGISTER];
        l_ecmdRc = (*Function)(i_target, i_address, l_ecmd_buffer);
#endif
        if (l_ecmdRc)
        {
            rc = (fapi2::ReturnCodes) l_ecmdRc; 
        }

#ifndef ECMD_STRIP_DEBUG
        if (ecmdClientDebug != 0)
        {
            args.push_back((void*) &l_ecmdRc);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"putCfamRegister");
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t putCfamRegister(const ecmdChipTarget & i_target, uint32_t i_address, const ecmdDataBuffer & i_data)",args);
        }
#endif

        if (l_ecmdRc && !ecmdGetGlobalVar(ECMD_GLOBALVAR_QUIETERRORMODE))
        {
            std::string errorString;
            errorString = ecmdGetErrorMsg(l_ecmdRc, false, ecmdGetGlobalVar(ECMD_GLOBALVAR_CMDLINEMODE), false);
            if (errorString.size())
                ecmdOutput(errorString.c_str());
        }

        return rc;
    }

    fapi2::ReturnCode getRing(const ecmdChipTarget & i_target, const scanRingId_t i_address, fapi2::variable_buffer & o_data, const uint32_t i_ringMode)
    {
        fapi2::ReturnCode rc(fapi2::FAPI2_RC_SUCCESS);
        uint32_t l_ecmdRc;
        ecmdDataBufferBase l_ecmd_buffer;

#ifndef ECMD_STATIC_FUNCTIONS
        if (dlHandle == NULL)
        {
            fprintf(stderr,"dllFapi2GetRing%s",ECMD_DLL_NOT_LOADED_ERROR);
            exit(ECMD_DLL_INVALID);
        }
#endif

        if (!fapi2Initialized)
        {
            fprintf(stderr,"dllFapi2GetRing: eCMD Extension not initialized before function called\n");
            fprintf(stderr,"dllFapi2GetRing: OR eCMD fapi Extension not supported by plugin\n");
            exit(ECMD_DLL_INVALID);
        }

#ifndef ECMD_STRIP_DEBUG
        int myTcount;
        std::vector< void * > args;
        if (ecmdClientDebug != 0)
        {
            args.push_back((void*) &i_target);
            args.push_back((void*) &i_address);
            args.push_back((void*) &l_ecmd_buffer);
            args.push_back((void*) &i_ringMode);
            fppCallCount++;
            myTcount = fppCallCount;
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"uint32_t getRing(const ecmdChipTarget & i_target, const scanRingId_t i_address, ecmdDataBufferBase & o_data, const uint32_t i_ringMode)",args);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"getRing");
        }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
        l_ecmdRc = dllFapi2GetRing(i_target, i_address, l_ecmd_buffer, i_ringMode);
#else
        if (fapi2DllFnTable[ECMD_FAPI2GETRING] == NULL)
        {
            fapi2DllFnTable[ECMD_FAPI2GETRING] = (void*)dlsym(dlHandle, "dllFapi2GetRing");
            if (fapi2DllFnTable[ECMD_FAPI2GETRING] == NULL)
            {
                fprintf(stderr,"dllFapi2GetRing%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
                ecmdDisplayDllInfo();
                exit(ECMD_DLL_INVALID);
            }
        }

        uint32_t (*Function)(const ecmdChipTarget&,  const scanRingId_t,  ecmdDataBufferBase &, const uint32_t ) = 
            (uint32_t(*)(const ecmdChipTarget&,  const scanRingId_t,  ecmdDataBufferBase &, const uint32_t))fapi2DllFnTable[ECMD_FAPI2GETRING];
        l_ecmdRc = (*Function)(i_target, i_address, l_ecmd_buffer, i_ringMode);
#endif
        if (l_ecmdRc)
        {
            rc = (fapi2::ReturnCodes) l_ecmdRc; 
        }

#ifndef ECMD_STRIP_DEBUG
        if (ecmdClientDebug != 0)
        {
            args.push_back((void*) &l_ecmdRc);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"getRing");
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t getRing(const ecmdChipTarget & i_target, const scanRingId_t i_address, ecmdDataBufferBase & o_data)",args);
        }
#endif

        l_ecmdRc = fapi2::bufferCopy(o_data, l_ecmd_buffer);
        if (l_ecmdRc)
        {
            return (fapi2::ReturnCodes) l_ecmdRc; 
        }

        return rc;
    }

    fapi2::ReturnCode putRing(const ecmdChipTarget & i_target, const RingId_t i_ringID, const fapi2::RingMode i_ringMode)
    {
        fapi2::ReturnCode rc(fapi2::FAPI2_RC_SUCCESS);
        uint32_t l_ecmdRc;

#ifndef ECMD_STATIC_FUNCTIONS
        if (dlHandle == NULL)
        {
            fprintf(stderr,"dllFapi2PutRingByID%s",ECMD_DLL_NOT_LOADED_ERROR);
            exit(ECMD_DLL_INVALID);
        }
#endif

        if (!fapi2Initialized)
        {
            fprintf(stderr,"dllFapi2PutRingByID: eCMD Extension not initialized before function called\n");
            fprintf(stderr,"dllFapi2PutRingByID: OR eCMD fapi Extension not supported by plugin\n");
            exit(ECMD_DLL_INVALID);
        }

#ifndef ECMD_STRIP_DEBUG
        int myTcount;
        std::vector< void * > args;
        if (ecmdClientDebug != 0)
        {
            args.push_back((void*) &i_target);
            args.push_back((void*) &i_ringID);
            args.push_back((void*) &i_ringMode);
            fppCallCount++;
            myTcount = fppCallCount;
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"uint32_t putRing(const ecmdChipTarget & i_target, const uint32_t i_ringID, const uint32_t i_ringMode)",args);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"putRing");
        }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
        l_ecmdRc = dllFapi2PutRingByID(i_target, i_ringID, i_ringMode);
#else
        if (fapi2DllFnTable[ECMD_FAPI2PUTRINGBYID] == NULL)
        {
            fapi2DllFnTable[ECMD_FAPI2PUTRINGBYID] = (void*)dlsym(dlHandle, "dllFapi2PutRingByID");
            if (fapi2DllFnTable[ECMD_FAPI2PUTRINGBYID] == NULL)
            {
                fprintf(stderr,"dllFapi2PutRingByID%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
                ecmdDisplayDllInfo();
                exit(ECMD_DLL_INVALID);
            }
        }

        uint32_t (*Function)(const ecmdChipTarget&, const uint32_t, const uint32_t) = 
            (uint32_t(*)(const ecmdChipTarget&, const uint32_t, const uint32_t))fapi2DllFnTable[ECMD_FAPI2PUTRINGBYID];
        l_ecmdRc = (*Function)(i_target, i_ringID, i_ringMode);
#endif
        if (l_ecmdRc)
        {
            rc = (fapi2::ReturnCodes) l_ecmdRc; 
        }

#ifndef ECMD_STRIP_DEBUG
        if (ecmdClientDebug != 0)
        {
            args.push_back((void*) &l_ecmdRc);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"putRing");
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t putRing(const ecmdChipTarget & i_target, const uint32_t i_ringID, const uint32_t i_ringMode)",args);
        }
#endif

        return rc;
    }

    fapi2::ReturnCode putRing(const ecmdChipTarget & i_target, const char* i_ringID, const fapi2::RingMode i_ringMode)
    {
        fapi2::ReturnCode rc(fapi2::FAPI2_RC_SUCCESS);
        uint32_t l_ecmdRc;

#ifndef ECMD_STATIC_FUNCTIONS
        if (dlHandle == NULL)
        {
            fprintf(stderr,"dllFapi2PutRingByName%s",ECMD_DLL_NOT_LOADED_ERROR);
            exit(ECMD_DLL_INVALID);
        }
#endif

        if (!fapi2Initialized)
        {
            fprintf(stderr,"dllFapi2PutRingByName: eCMD Extension not initialized before function called\n");
            fprintf(stderr,"dllFapi2PutRingByName: OR eCMD fapi Extension not supported by plugin\n");
            exit(ECMD_DLL_INVALID);
        }

#ifndef ECMD_STRIP_DEBUG
        int myTcount;
        std::vector< void * > args;
        if (ecmdClientDebug != 0)
        {
            args.push_back((void*) &i_target);
            args.push_back((void*) &i_ringID);
            args.push_back((void*) &i_ringMode);
            fppCallCount++;
            myTcount = fppCallCount;
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"uint32_t putRing(const ecmdChipTarget & i_target, const char* i_ringID, const uint32_t i_ringMode)",args);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"putRing");
        }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
        l_ecmdRc = dllFapi2PutRingByName(i_target, i_ringID, i_ringMode);
#else
        if (fapi2DllFnTable[ECMD_FAPI2PUTRINGBYNAME] == NULL)
        {
            fapi2DllFnTable[ECMD_FAPI2PUTRINGBYNAME] = (void*)dlsym(dlHandle, "dllFapi2PutRingByName");
            if (fapi2DllFnTable[ECMD_FAPI2PUTRINGBYNAME] == NULL)
            {
                fprintf(stderr,"dllFapi2PutRingByName%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
                ecmdDisplayDllInfo();
                exit(ECMD_DLL_INVALID);
            }
        }

        uint32_t (*Function)(const ecmdChipTarget&, const char*, const uint32_t) = 
            (uint32_t(*)(const ecmdChipTarget&, const char*, const uint32_t))fapi2DllFnTable[ECMD_FAPI2PUTRINGBYNAME];
        l_ecmdRc = (*Function)(i_target, i_ringID, i_ringMode);
#endif
        if (l_ecmdRc)
        {
            rc = (fapi2::ReturnCodes) l_ecmdRc; 
        }

#ifndef ECMD_STRIP_DEBUG
        if (ecmdClientDebug != 0)
        {
            args.push_back((void*) &l_ecmdRc);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"putRing");
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t putRing(const ecmdChipTarget & i_target, const char* i_ringID, const uint32_t i_ringMode)",args);
        }
#endif

        return rc;
    }

    fapi2::ReturnCode plat_putRingImpl(const ecmdChipTarget & i_target, const void* i_scanImage, const uint32_t i_modifiedRingAddress, const fapi2::RingMode i_ringMode)
    {
        fapi2::ReturnCode rc(fapi2::FAPI2_RC_SUCCESS);
        uint32_t l_ecmdRc;

#ifndef ECMD_STATIC_FUNCTIONS
        if (dlHandle == NULL)
        {
            fprintf(stderr,"dllFapi2PutRingImage%s",ECMD_DLL_NOT_LOADED_ERROR);
            exit(ECMD_DLL_INVALID);
        }
#endif

        if (!fapi2Initialized)
        {
            fprintf(stderr,"dllFapi2PlatPutRingImage: eCMD Extension not initialized before function called\n");
            fprintf(stderr,"dllFapi2PlatPutRingImage: OR eCMD fapi Extension not supported by plugin\n");
            exit(ECMD_DLL_INVALID);
        }

#ifndef ECMD_STRIP_DEBUG
        int myTcount;
        std::vector< void * > args;
        if (ecmdClientDebug != 0)
        {
            args.push_back((void*) &i_target);
            args.push_back((void*) &i_scanImage);
            args.push_back((void*) &i_modifiedRingAddress);
            args.push_back((void*) &i_ringMode);
            fppCallCount++;
            myTcount = fppCallCount;
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"uint32_t plat_putRingImpl(const ecmdChipTarget & i_target, const void* i_scanImage, const uint32_t i_modifiedRingAddress, const uint32_t i_ringMode)",args);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"plat_putRingImpl");
        }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
        l_ecmdRc = dllFapi2PutRingImage(i_target, i_scanImage, i_modifiedRingAddress, i_ringMode);
#else
        if (fapi2DllFnTable[ECMD_FAPI2PUTRINGIMAGE] == NULL)
        {
            fapi2DllFnTable[ECMD_FAPI2PUTRINGIMAGE] = (void*)dlsym(dlHandle, "dllFapi2PutRingImage");
            if (fapi2DllFnTable[ECMD_FAPI2PUTRINGIMAGE] == NULL)
            {
                fprintf(stderr,"dllFapi2PutRingImage%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
                ecmdDisplayDllInfo();
                exit(ECMD_DLL_INVALID);
            }
        }

        uint32_t (*Function)(const ecmdChipTarget&, const void*, const uint32_t, const uint32_t) = 
            (uint32_t(*)(const ecmdChipTarget&, const void*, const uint32_t, const uint32_t))fapi2DllFnTable[ECMD_FAPI2PUTRINGIMAGE];
        l_ecmdRc = (*Function)(i_target, i_scanImage, i_modifiedRingAddress, i_ringMode);
#endif
        if (l_ecmdRc)
        {
            rc = (fapi2::ReturnCodes) l_ecmdRc; 
        }

#ifndef ECMD_STRIP_DEBUG
        if (ecmdClientDebug != 0)
        {
            args.push_back((void*) &l_ecmdRc);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"plat_putRingImpl");
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t plat_putRingImpl(const ecmdChipTarget & i_target, const void* i_scanImage, const uint32_t i_modifiedRingAddress, const uint32_t i_ringMode)",args);
        }
#endif

        return rc;
    }

    fapi2::ReturnCode putRing(const ecmdChipTarget & i_target, const scanRingId_t i_address, const fapi2::variable_buffer & i_data, const uint32_t i_ringMode)
    {
        fapi2::ReturnCode rc(fapi2::FAPI2_RC_SUCCESS);
        uint32_t l_ecmdRc;
        ecmdDataBufferBase l_ecmd_buffer;

        l_ecmdRc = fapi2::bufferCopy(l_ecmd_buffer, i_data);
        if (l_ecmdRc)
        {
            return (fapi2::ReturnCodes) l_ecmdRc; 
        }

#ifndef ECMD_STATIC_FUNCTIONS
        if (dlHandle == NULL)
        {
            fprintf(stderr,"dllFapi2PutRing%s",ECMD_DLL_NOT_LOADED_ERROR);
            exit(ECMD_DLL_INVALID);
        }
#endif

        if (!fapi2Initialized)
        {
            fprintf(stderr,"dllFapi2PutRing: eCMD Extension not initialized before function called\n");
            fprintf(stderr,"dllFapi2PutRing: OR eCMD fapi Extension not supported by plugin\n");
            exit(ECMD_DLL_INVALID);
        }

#ifndef ECMD_STRIP_DEBUG
        int myTcount;
        std::vector< void * > args;
        if (ecmdClientDebug != 0)
        {
            args.push_back((void*) &i_target);
            args.push_back((void*) &i_address);
            args.push_back((void*) &l_ecmd_buffer);
            args.push_back((void*) &i_ringMode);
            fppCallCount++;
            myTcount = fppCallCount;
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"uint32_t putRing(const ecmdChipTarget & i_target, const scanRingId_t i_address, const ecmdDataBufferBase & i_data, const uint32_t i_ringMode)",args);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"putRing");
        }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
        l_ecmdRc = dllFapi2PutRing(i_target, i_address, l_ecmd_buffer, i_ringMode);
#else
        if (fapi2DllFnTable[ECMD_FAPI2PUTRING] == NULL)
        {
            fapi2DllFnTable[ECMD_FAPI2PUTRING] = (void*)dlsym(dlHandle, "dllFapi2PutRing");
            if (fapi2DllFnTable[ECMD_FAPI2PUTRING] == NULL)
            {
                fprintf(stderr,"dllFapi2PutRing%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
                ecmdDisplayDllInfo();
                exit(ECMD_DLL_INVALID);
            }
        }

        uint32_t (*Function)(const ecmdChipTarget&,  const scanRingId_t, const ecmdDataBufferBase &, const uint32_t i_ringMode) = 
            (uint32_t(*)(const ecmdChipTarget&,  const scanRingId_t, const ecmdDataBufferBase &, const uint32_t))fapi2DllFnTable[ECMD_FAPI2PUTRING];
        l_ecmdRc = (*Function)(i_target, i_address, l_ecmd_buffer, i_ringMode);
#endif
        if (l_ecmdRc)
        {
            rc = (fapi2::ReturnCodes) l_ecmdRc; 
        }

#ifndef ECMD_STRIP_DEBUG
        if (ecmdClientDebug != 0)
        {
            args.push_back((void*) &l_ecmdRc);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"putRing");
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t putRing(const ecmdChipTarget & i_target, const scanRingId_t i_address, const ecmdDataBufferBase & i_data, const uint32_t i_ringMode)",args);
        }
#endif

        return rc;
    }

    fapi2::ReturnCode getSpy(const ecmdChipTarget & i_target,
                             const char * const i_spyId,
                             fapi2::variable_buffer& o_data)
    {
        fapi2::ReturnCode rc(fapi2::FAPI2_RC_SUCCESS);
        uint32_t l_ecmdRc;

        ecmdDataBuffer l_ecmd_buffer;
        uint32_t l_mode = 0;

#ifndef ECMD_STATIC_FUNCTIONS
        if (dlHandle == NULL) 
        {
            fprintf(stderr,"dllGetSpy%s",ECMD_DLL_NOT_LOADED_ERROR);
            exit(ECMD_DLL_INVALID);
        }
#endif

#ifndef ECMD_STRIP_DEBUG
        int myTcount;
        std::vector< void * > args;
        if (ecmdClientDebug != 0)
        {
            args.push_back((void*) &i_target);
            args.push_back((void*) &i_spyId);
            args.push_back((void*) &l_ecmd_buffer);
            args.push_back((void*) &l_mode);
            fppCallCount++;
            myTcount = fppCallCount;
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"uint32_t getSpy(const ecmdChipTarget & i_target, const char * i_spyId, ecmdDataBuffer & l_ecmd_buffer, uint32_t l_mode)",args);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"getSpy");
        }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
        l_ecmdRc = dllGetSpy(i_target, i_spyId, l_ecmd_buffer, l_mode);
#else
        if (DllFnTable[ECMD_GETSPY] == NULL)
        {
            DllFnTable[ECMD_GETSPY] = (void*)dlsym(dlHandle, "dllGetSpy");
            if (DllFnTable[ECMD_GETSPY] == NULL)
            {
                fprintf(stderr,"dllGetSpy%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
                ecmdDisplayDllInfo();
                exit(ECMD_DLL_INVALID);
            }
        }

        uint32_t (*Function)(const ecmdChipTarget &,  const char * ,  ecmdDataBuffer &, uint32_t) = 
            (uint32_t(*)(const ecmdChipTarget &,  const char * ,  ecmdDataBuffer &, uint32_t))DllFnTable[ECMD_GETSPY];
        l_ecmdRc = (*Function)(i_target, i_spyId, l_ecmd_buffer, l_mode);
#endif
        if (l_ecmdRc)
        {
            rc = (fapi2::ReturnCodes) l_ecmdRc; 
        }

#ifndef ECMD_STRIP_DEBUG
        if (ecmdClientDebug != 0)
        {
            args.push_back((void*) &l_ecmdRc);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"getSpy");
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t getSpy(const ecmdChipTarget & i_target, const char * i_spyName, ecmdDataBuffer & l_ecmd_buffer, uint32_t l_mode)",args);
        }
#endif

        if (l_ecmdRc && !ecmdGetGlobalVar(ECMD_GLOBALVAR_QUIETERRORMODE))
        {
            std::string errorString;
            errorString = ecmdGetErrorMsg(l_ecmdRc, false, ecmdGetGlobalVar(ECMD_GLOBALVAR_CMDLINEMODE), false);
            if (errorString.size())
                ecmdOutput(errorString.c_str());
        }

        l_ecmdRc = fapi2::bufferCopy(o_data, l_ecmd_buffer);
        if (l_ecmdRc)
        {
            return (fapi2::ReturnCodes) l_ecmdRc; 
        }

        return rc;
    }

    fapi2::ReturnCode putSpy(const ecmdChipTarget & i_target,
                             const char * const i_spyId,
                             const fapi2::variable_buffer& i_data)
    {
        fapi2::ReturnCode rc(fapi2::FAPI2_RC_SUCCESS);
        uint32_t l_ecmdRc;
        uint32_t l_mode = ECMD_RING_MODE_SPARSE_ACCESS;

        ecmdDataBuffer l_ecmd_buffer;
        l_ecmdRc = fapi2::bufferCopy(l_ecmd_buffer, i_data);
        if (l_ecmdRc)
        {
            return (fapi2::ReturnCodes) l_ecmdRc; 
        }

#ifndef ECMD_STATIC_FUNCTIONS
        if (dlHandle == NULL) 
        {
	        fprintf(stderr,"dllPutSpy%s",ECMD_DLL_NOT_LOADED_ERROR);
	        exit(ECMD_DLL_INVALID);
        }
#endif

#ifndef ECMD_STRIP_DEBUG
        int myTcount;
        std::vector< void * > args;
        if (ecmdClientDebug != 0)
        {
            args.push_back((void*) &i_target);
            args.push_back((void*) &i_spyId);
            args.push_back((void*) &l_ecmd_buffer);
            args.push_back((void*) &l_mode);
            fppCallCount++;
            myTcount = fppCallCount;
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"uint32_t putSpy(const ecmdChipTarget & i_target, const char * i_spyId, const ecmdDataBuffer & l_ecmd_buffer, uint32_t l_mode)",args);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"putSpy");
        }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
        l_ecmdRc = dllPutSpy(i_target, i_spyId, l_ecmd_buffer, l_mode);
#else
        if (DllFnTable[ECMD_PUTSPY] == NULL)
        {
            DllFnTable[ECMD_PUTSPY] = (void*)dlsym(dlHandle, "dllPutSpy");
            if (DllFnTable[ECMD_PUTSPY] == NULL)
            {
                fprintf(stderr,"dllPutSpy%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
                ecmdDisplayDllInfo();
                exit(ECMD_DLL_INVALID);
            }
        }

        uint32_t (*Function)(const ecmdChipTarget &,  const char * ,  const ecmdDataBuffer &, uint32_t) = 
            (uint32_t(*)(const ecmdChipTarget &,  const char * ,  const ecmdDataBuffer &, uint32_t))DllFnTable[ECMD_PUTSPY];
        l_ecmdRc = (*Function)(i_target, i_spyId, l_ecmd_buffer, l_mode);
#endif
        if (l_ecmdRc)
        {
            rc = (fapi2::ReturnCodes) l_ecmdRc; 
        }

#ifndef ECMD_STRIP_DEBUG
        if (ecmdClientDebug != 0)
        {
            args.push_back((void*) &l_ecmdRc);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"putSpy");
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t putSpy(const ecmdChipTarget & i_target, const char * i_spyName, const ecmdDataBuffer & l_ecmd_buffer, uint32_t l_mode)",args);
        }
#endif

        if (l_ecmdRc && !ecmdGetGlobalVar(ECMD_GLOBALVAR_QUIETERRORMODE))
        {
            std::string errorString;
            errorString = ecmdGetErrorMsg(l_ecmdRc, false, ecmdGetGlobalVar(ECMD_GLOBALVAR_CMDLINEMODE), false);
            if (errorString.size())
                ecmdOutput(errorString.c_str());
        }

        return rc;
    }

    fapi2::ReturnCode getSpyImage(const ecmdChipTarget & i_target,
                                  const char * const i_spyId,
                                  fapi2::variable_buffer& o_data,
                                  const fapi2::variable_buffer& i_imageData)
    {
        fapi2::ReturnCode rc(fapi2::FAPI2_RC_SUCCESS);
        uint32_t l_ecmdRc;

        ecmdDataBuffer l_ecmd_buffer;
        ecmdDataBuffer l_ecmd_buffer_image_data;
        l_ecmdRc = fapi2::bufferCopy(l_ecmd_buffer_image_data, i_imageData);
        if (l_ecmdRc)
        {
            return (fapi2::ReturnCodes) l_ecmdRc; 
        }

#ifndef ECMD_STATIC_FUNCTIONS
        if (dlHandle == NULL) 
        {
	        fprintf(stderr,"dllGettSpy%s",ECMD_DLL_NOT_LOADED_ERROR);
	        exit(ECMD_DLL_INVALID);
        }
#endif

#ifndef ECMD_STRIP_DEBUG
        int myTcount;
        std::vector< void * > args;
        if (ecmdClientDebug != 0)
        {
            args.push_back((void*) &i_target);
            args.push_back((void*) &i_spyId);
            args.push_back((void*) &l_ecmd_buffer_image_data);
            args.push_back((void*) &l_ecmd_buffer);
            fppCallCount++;
            myTcount = fppCallCount;
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"uint32_t getSpyImage(const ecmdChipTarget & i_target, const char * i_spyId, const ecmdDataBuffer & l_ecmd_buffer_image_data, ecmdDataBuffer & l_ecmd_buffer)",args);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"getSpyImage");
        }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
        l_ecmdRc = dllGetSpyImage(i_target, i_spyId, l_ecmd_buffer_image_data, l_ecmd_buffer);
#else
        if (DllFnTable[ECMD_GETSPYIMAGE] == NULL)
        {
            DllFnTable[ECMD_GETSPYIMAGE] = (void*)dlsym(dlHandle, "dllGetSpyImage");
            if (DllFnTable[ECMD_GETSPYIMAGE] == NULL)
            {
                fprintf(stderr,"dllGetSpyImage%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
                ecmdDisplayDllInfo();
                exit(ECMD_DLL_INVALID);
            }
        }

        uint32_t (*Function)(const ecmdChipTarget &,  const char * ,  const ecmdDataBuffer &, ecmdDataBuffer &) = 
            (uint32_t(*)(const ecmdChipTarget &,  const char * ,  const ecmdDataBuffer &, ecmdDataBuffer &))DllFnTable[ECMD_GETSPYIMAGE];
        l_ecmdRc = (*Function)(i_target, i_spyId, l_ecmd_buffer_image_data, l_ecmd_buffer);
#endif
        if (l_ecmdRc)
        {
            rc = (fapi2::ReturnCodes) l_ecmdRc; 
        }

#ifndef ECMD_STRIP_DEBUG
        if (ecmdClientDebug != 0)
        {
            args.push_back((void*) &l_ecmdRc);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"getSpyImage");
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t getSpyImage(const ecmdChipTarget & i_target, const char * i_spyName, const ecmdDataBuffer & l_ecmd_buffer_image_data, ecmdDataBuffer & l_ecmd_buffer)",args);
        }
#endif

        if (l_ecmdRc && !ecmdGetGlobalVar(ECMD_GLOBALVAR_QUIETERRORMODE))
        {
            std::string errorString;
            errorString = ecmdGetErrorMsg(l_ecmdRc, false, ecmdGetGlobalVar(ECMD_GLOBALVAR_CMDLINEMODE), false);
            if (errorString.size())
                ecmdOutput(errorString.c_str());
        }

        l_ecmdRc = fapi2::bufferCopy(o_data, l_ecmd_buffer);
        if (l_ecmdRc)
        {
            return (fapi2::ReturnCodes) l_ecmdRc; 
        }

        return rc;
    }

    fapi2::ReturnCode putSpyImage(const ecmdChipTarget & i_target,
                                  const char* const i_spyId,
                                  const fapi2::variable_buffer& i_data,
                                  fapi2::variable_buffer& o_imageData)
    {
        fapi2::ReturnCode rc(fapi2::FAPI2_RC_SUCCESS);
        uint32_t l_ecmdRc;

        ecmdDataBuffer l_ecmd_buffer;
        l_ecmdRc = fapi2::bufferCopy(l_ecmd_buffer, i_data);
        if (l_ecmdRc)
        {
            return (fapi2::ReturnCodes) l_ecmdRc; 
        }

        ecmdDataBuffer l_ecmd_buffer_image_data;
        l_ecmdRc = fapi2::bufferCopy(l_ecmd_buffer_image_data, o_imageData);
        if (l_ecmdRc)
        {
            return (fapi2::ReturnCodes) l_ecmdRc; 
        }

#ifndef ECMD_STATIC_FUNCTIONS
        if (dlHandle == NULL) 
        {
	        fprintf(stderr,"dllPutSpy%s",ECMD_DLL_NOT_LOADED_ERROR);
	        exit(ECMD_DLL_INVALID);
        }
#endif

#ifndef ECMD_STRIP_DEBUG
        int myTcount;
        std::vector< void * > args;
        if (ecmdClientDebug != 0)
        {
            args.push_back((void*) &i_target);
            args.push_back((void*) &i_spyId);
            args.push_back((void*) &l_ecmd_buffer);
            args.push_back((void*) &l_ecmd_buffer_image_data);
            fppCallCount++;
            myTcount = fppCallCount;
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"uint32_t putSpyImage(const ecmdChipTarget & i_target, const char * i_spyId, const ecmdDataBuffer & l_ecmd_buffer, ecmdDataBuffer & l_ecmd_buffer_image_data)",args);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"putSpyImage");
        }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
        l_ecmdRc = dllPutSpyImage(i_target, i_spyId, l_ecmd_buffer, l_ecmd_buffer_image_data);
#else
        if (DllFnTable[ECMD_PUTSPYIMAGE] == NULL)
        {
            DllFnTable[ECMD_PUTSPYIMAGE] = (void*)dlsym(dlHandle, "dllPutSpyImage");
            if (DllFnTable[ECMD_PUTSPYIMAGE] == NULL)
            {
                fprintf(stderr,"dllPutSpyImage%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
                ecmdDisplayDllInfo();
                exit(ECMD_DLL_INVALID);
            }
        }

        uint32_t (*Function)(const ecmdChipTarget &,  const char * ,  const ecmdDataBuffer &, ecmdDataBuffer &) = 
            (uint32_t(*)(const ecmdChipTarget &,  const char * ,  const ecmdDataBuffer &, ecmdDataBuffer &))DllFnTable[ECMD_PUTSPYIMAGE];
        l_ecmdRc = (*Function)(i_target, i_spyId, l_ecmd_buffer, l_ecmd_buffer_image_data);
#endif
        if (l_ecmdRc)
        {
            rc = (fapi2::ReturnCodes) l_ecmdRc; 
        }

#ifndef ECMD_STRIP_DEBUG
        if (ecmdClientDebug != 0)
        {
            args.push_back((void*) &l_ecmdRc);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"putSpyImage");
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t putSpyImage(const ecmdChipTarget & i_target, const char * i_spyName, const ecmdDataBuffer & l_ecmd_buffer, ecmdDataBuffer & l_ecmd_data_buffer_image_data)",args);
        }
#endif

        if (l_ecmdRc && !ecmdGetGlobalVar(ECMD_GLOBALVAR_QUIETERRORMODE))
        {
            std::string errorString;
            errorString = ecmdGetErrorMsg(l_ecmdRc, false, ecmdGetGlobalVar(ECMD_GLOBALVAR_CMDLINEMODE), false);
            if (errorString.size())
                ecmdOutput(errorString.c_str());
        }

        l_ecmdRc = fapi2::bufferCopy(o_imageData, l_ecmd_buffer_image_data);
        if (l_ecmdRc)
        {
            return (fapi2::ReturnCodes) l_ecmdRc; 
        }

        return rc;
    }

} // namespace fapi2plat



namespace fapi2
{
    ReturnCode getMvpdField(const MvpdRecord i_record,
                            const MvpdKeyword i_keyword,
                            const Target<TARGET_TYPE_PROC_CHIP> &i_target,
                            uint8_t * const i_pBuffer,
                            uint32_t &io_fieldSize)
    {

        ReturnCode rc;
        uint32_t l_ecmdRc;
  
        ecmdChipTarget ecmdTarget;
        fapiTargetToEcmdTarget(i_target, ecmdTarget);

#ifndef ECMD_STATIC_FUNCTIONS
        if (dlHandle == NULL)
        {
            fprintf(stderr,"dllFapi2GetMvpdField%s",ECMD_DLL_NOT_LOADED_ERROR);
            exit(ECMD_DLL_INVALID);
        }
#endif

#ifndef ECMD_STRIP_DEBUG
        int myTcount;
        std::vector< void * > args;
        if (ecmdClientDebug != 0)
        {
            args.push_back((void*) &i_record);
            args.push_back((void*) &i_keyword);
            args.push_back((void*) &ecmdTarget);
            args.push_back((void*) &i_pBuffer);
            args.push_back((void*) &io_fieldSize);
            fppCallCount++;
            myTcount = fppCallCount;
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"uint32_t dllFapi2GetMvpdField(const MvpdRecord i_record, const MvpdKeyword i_keyword, const ecmdChipTarget & ecmdTarget, uint8_t * const i_pBuffer, uint32_t &io_fieldSize) )",args);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"dllFapi2GetMvpdField");
        }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
        l_ecmdRc = dllFapi2GetMvpdField(i_record, i_keyword, ecmdTarget, i_pBuffer, io_fieldSize);
#else
        if (fapi2DllFnTable[ECMD_FAPI2GETMVPDFIELD] == NULL)
        {
            fapi2DllFnTable[ECMD_FAPI2GETMVPDFIELD] = (void*)dlsym(dlHandle, "dllFapi2GetMvpdField");
            if (fapi2DllFnTable[ECMD_FAPI2GETMVPDFIELD] == NULL)
            {
                fprintf(stderr,"dllFapi2GetMvpdField%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
                ecmdDisplayDllInfo();
                exit(ECMD_DLL_INVALID);
            }
        }

        uint32_t (*Function)(const MvpdRecord, const MvpdKeyword, const ecmdChipTarget &, uint8_t *, uint32_t &) = 
            (uint32_t(*)(const MvpdRecord, const MvpdKeyword, const ecmdChipTarget &, uint8_t *, uint32_t &))fapi2DllFnTable[ECMD_FAPI2GETMVPDFIELD];
        l_ecmdRc = (*Function)(i_record, i_keyword, ecmdTarget, i_pBuffer, io_fieldSize);
#endif
        if (l_ecmdRc)
        {
            rc = (ReturnCodes) l_ecmdRc; 
        }

#ifndef ECMD_STRIP_DEBUG
        if (ecmdClientDebug != 0) {
            args.push_back((void*) &l_ecmdRc);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"dllFapi2GetMvpdField");
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t dllFapi2GetMvpdField(const MvpdRecord i_record, const MvpdKeyword i_keyword, const ecmdChipTarget & ecmdTarget, uint8_t * const i_pBuffer, uint32_t &io_fieldSize)",args);
        }
#endif

        return rc;
    }

    ReturnCode setMvpdField(const MvpdRecord i_record,
                            const MvpdKeyword i_keyword,
                            const Target<TARGET_TYPE_PROC_CHIP> &i_target,
                            const uint8_t * const i_pBuffer,
                            const uint32_t i_fieldSize)
    {

        ReturnCode rc;
        uint32_t l_ecmdRc;
  
        ecmdChipTarget ecmdTarget;
        fapiTargetToEcmdTarget(i_target, ecmdTarget);

#ifndef ECMD_STATIC_FUNCTIONS
        if (dlHandle == NULL)
        {
            fprintf(stderr,"dllFapi2SetMvpdField%s",ECMD_DLL_NOT_LOADED_ERROR);
            exit(ECMD_DLL_INVALID);
        }
#endif

#ifndef ECMD_STRIP_DEBUG
        int myTcount;
        std::vector< void * > args;
        if (ecmdClientDebug != 0) {
            args.push_back((void*) &i_record);
            args.push_back((void*) &i_keyword);
            args.push_back((void*) &ecmdTarget);
            args.push_back((void*) &i_pBuffer);
            args.push_back((void*) &i_fieldSize);
            fppCallCount++;
            myTcount = fppCallCount;
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONIN,"uint32_t dllFapi2SetMvpdField(const MvpdRecord i_record, const MvpdKeyword i_keyword, const ecmdChipTarget &ecmdTarget, const uint8_t * const i_pBuffer, const uint32_t i_fieldSize) )",args);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONIN,"dllFapi2SetMvpdField");
        }
#endif

#ifdef ECMD_STATIC_FUNCTIONS
        l_ecmdRc = dllFapi2SetMvpdField(i_record, i_keyword, ecmdTarget, i_pBuffer, i_fieldSize);
#else
        if (fapi2DllFnTable[ECMD_FAPI2SETMVPDFIELD] == NULL)
        {
            fapi2DllFnTable[ECMD_FAPI2SETMVPDFIELD] = (void*)dlsym(dlHandle, "dllFapi2SetMvpdField");
            if (fapi2DllFnTable[ECMD_FAPI2SETMVPDFIELD] == NULL)
            {
                fprintf(stderr,"dllFapi2SetMvpdField%s",ECMD_UNABLE_TO_FIND_FUNCTION_ERROR); 
                ecmdDisplayDllInfo();
                exit(ECMD_DLL_INVALID);
            }
        }

        uint32_t (*Function)(const MvpdRecord, const MvpdKeyword, const ecmdChipTarget &, const uint8_t *, const uint32_t ) = 
            (uint32_t(*)(const MvpdRecord, const MvpdKeyword, const ecmdChipTarget &, const uint8_t *, const uint32_t ))fapi2DllFnTable[ECMD_FAPI2SETMVPDFIELD];
        l_ecmdRc = (*Function)(i_record, i_keyword, ecmdTarget, i_pBuffer, i_fieldSize);
#endif
        if (l_ecmdRc)
        {
            rc = (ReturnCodes) l_ecmdRc; 
        }

#ifndef ECMD_STRIP_DEBUG
        if (ecmdClientDebug != 0)
        {
            args.push_back((void*) &l_ecmdRc);
            ecmdFunctionTimer(myTcount,ECMD_TMR_FUNCTIONOUT,"dllFapi2SetMvpdField");
            ecmdFunctionParmPrinter(myTcount,ECMD_FPP_FUNCTIONOUT,"uint32_t dllFapi2SetMvpdField(const MvpdRecord i_record, const MvpdKeyword i_keyword, const ecmdChipTarget &ecmdTarget, const uint8_t * const i_pBuffer, const uint32_t i_fieldSize)",args);
        }
#endif

        return rc;
    }

} // namespace fapi2
