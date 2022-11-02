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

/**
 @file ecmdVersion.C
 @brief Utility program to display just the eCMD Version in a string to use as a path for CTE
*/


//----------------------------------------------------------------------
//  Includes
//----------------------------------------------------------------------
#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string>
#include <inttypes.h>

#include <ecmdReturnCodes.H>

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
/**
 @brief This is a backdoor mechanism to display the version of the current eCMD dll. DO NOT USE WILL CAUSE YOUR PROGRAM TO DIE
 @param options Options to change display mode
*/
uint32_t ecmdCheckDllVersion(const char* options);

//----------------------------------------------------------------------
//  Global Variables
//----------------------------------------------------------------------

//---------------------------------------------------------------------
// Member Function Specifications
//---------------------------------------------------------------------

int main (int , char *argv[])
{
  uint32_t rc = 0;


  /* Fix for defect 18174, if we are running the eCMD dev release, we can't query the dll version we just have to return dev here to pick up all the proper supporting files, hopefully the dll will be happy with that, if not it will be found when the real dll is loaded */
#ifdef ECMD_DEV
  printf("dev");
#else 

  /* This is going to be real simple, we just call the dll and it does all the work */
  /* Pass in any args we may have, if argv[1] is null that is handled properly */
  rc = ecmdCheckDllVersion(argv[1]);
#endif

  return rc;
}


uint32_t ecmdCheckDllVersion(const char* options) {


  void * dlHandle = NULL;
  const char* dlError;
  uint32_t rc = ECMD_SUCCESS;
  std::string i_dllName;


  /* Only do this if it hasn't been done already */
  if (dlHandle != NULL) {
    fprintf(stderr,"ERROR: ecmdCheckDllVersion: This function cannot be called after ecmdLoadDll\n");
    fprintf(stderr,"ERROR: ecmdCheckDllVersion: Aborting execution!\n");
    abort();
  }

  /* --------------------- */
  /* load DLL              */
  /* --------------------- */
  /* Let's try to get it from the env var */
  char * tempptr = getenv("ECMD_DLL_FILE");  /* is there a ECMD_DLL_FILE environment variable? */
  if (tempptr != NULL) {
    i_dllName = tempptr;
  } else {
    fprintf(stderr,"ERROR: ecmdCheckDllVersion: Unable to find DLL to load, you must set ECMD_DLL_FILE\n");
    return ECMD_INVALID_DLL_FILENAME;
  }


  dlHandle = dlopen(i_dllName.c_str(), RTLD_LAZY);
  if (!dlHandle) {
    if ((dlError = dlerror()) != NULL) {
      fprintf(stderr,"ERROR: ecmdCheckDllVersion: Problems loading '%s' : %s\n", i_dllName.c_str(), dlError);
      return ECMD_DLL_LOAD_FAILURE;
    }
  }

  /* Now we need to call loadDll on the dll itself so it can initialize */

  uint32_t (*Function)(const char *) = 
    (uint32_t(*)(const char *))(void*)dlsym(dlHandle, "dllCheckDllVersion");
  if (!Function) {
    fprintf(stderr,"ecmdCheckDllVersion: Unable to find dllCheckDllVersion function, must be an invalid DLL\n");
    rc = ECMD_DLL_LOAD_FAILURE;
  } else {
    rc = (*Function)(options);
  }

  if (dlHandle) dlclose(dlHandle);
  /* The dllCheckDllVersion does an exit so we will never get here */
  return rc;
}
