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
#define fapi2Interpreter_C
#include <string.h>

#include <fapi2Interpreter.H>
#include <ecmdReturnCodes.H>
#include <ecmdUtils.H>
#include <ecmdSharedUtils.H>
#include <ecmdCommandUtils.H>

#ifdef ECMD_FAPI2_EXTENSION_SUPPORT
#include <fapi2ClientCapi.H>
#endif



#undef fapi2Interpreter_C
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

//---------------------------------------------------------------------
// Member Function Specifications
//---------------------------------------------------------------------

uint32_t fapi2CommandInterpreter(int argc, char* argv[]) {

  uint32_t rc = ECMD_INT_UNKNOWN_COMMAND;


  if (argc >= 1) {

    /* Let's make sure it is ours */
    if (strncmp(argv[0],"fapi2",5)) {
      return ECMD_INT_UNKNOWN_COMMAND;
    }

    /* Let's handle the '-h' arg right here */
    if (ecmdParseOption(&argc, &argv, "-h")) {
      ecmdPrintHelp(argv[0]);
      return rc;
    }

    switch (argv[0][5]) {

        /************************/
        /* The D's              */
        /************************/
      case 'd':

        if (!strcmp(argv[0], "fapi2dumpattr")) {
          rc = fapi2DumpAttributeUser(argc - 1, argv + 1);
        } else {
          /* We don't understand this function, let's let the caller know */
          rc = ECMD_INT_UNKNOWN_COMMAND;
        }
        break;

        /************************/
        /* The G's              */
        /************************/
      case 'g':

        if (!strcmp(argv[0], "fapi2getattr")) {
          rc = fapi2GetAttributeUser(argc - 1, argv + 1);
        } else {
          /* We don't understand this function, let's let the caller know */
          rc = ECMD_INT_UNKNOWN_COMMAND;
        }
        break;
        
        /************************/
        /* The S's              */
        /************************/
      case 's':

        if (!strcmp(argv[0], "fapi2setattr")) {
          rc = fapi2SetAttributeUser(argc - 1, argv + 1);
        } else {
          /* We don't understand this function, let's let the caller know */
          rc = ECMD_INT_UNKNOWN_COMMAND;
        }
        break;

        /************************/
        /* The Unknown          */
        /************************/
      default:
        /* We don't understand this function, let's let the caller know */
        rc = ECMD_INT_UNKNOWN_COMMAND;
        break;
    }



  } /* End if (argc >= 1) */
  else {
    /* For now we will return invalid, for the future we may want a shell here */
    rc = ECMD_INT_UNKNOWN_COMMAND;

  }

  return rc;
}
