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
 @file ecmdMain.C
 @brief Main Program entry point for ecmdDllClient Application
*/


//----------------------------------------------------------------------
//  Includes
//----------------------------------------------------------------------
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include <ecmdClientCapi.H>
#include <ecmdInterpreter.H>
#include <ecmdReturnCodes.H>
#include <ecmdCommandUtils.H>
#include <ecmdSharedUtils.H>

#define ERRORBUF_SIZE 200

int main (int argc, char *argv[])
{
  uint32_t rc = ECMD_SUCCESS;

  std::string cmdSave;
  std::string curCmd;
  bool isSystemCmd = false;
  char errorbuf[ERRORBUF_SIZE];
  for (int i = 0; i < argc; i++) {
    cmdSave += argv[i];
    cmdSave += " ";
  }
  cmdSave += "\n";
  std::string dllToLoadName;
  dllToLoadName.clear(); // get dllnName from environment variable
  rc = ecmdLoadDll(dllToLoadName);
  if (rc) {
    ecmdLoadDllRecovery(cmdSave, rc);
  }

  if (rc == ECMD_SUCCESS) {

    // By default, quiet error mode is on.  This means no errors are printing in the dll wrapper code
    // For the command line, turn off quiet error mode so those errors will be reported as the plugin call returns
    rc = ecmdSetGlobalVar(ECMD_GLOBALVAR_QUIETERRORMODE, 0);
    if (rc) return rc;

    // Also tell the plugin that we are running in cmdline mode - STGC01079967
    // Currently this is used to cleanup stored error messages in *Func.C so that -coe mode doesn't double report them
    rc = ecmdSetGlobalVar(ECMD_GLOBALVAR_CMDLINEMODE, 1);
    if (rc) return rc;

    /* Check to see if we are using stdin to pass in multiple commands */
    bool shellMode = ecmdParseOption(&argc, &argv, "-shell");
    bool stdinMode = ecmdParseOption(&argc, &argv, "-stdin");
    if (stdinMode || shellMode) {

      /* Grab any other args that may be there */
      rc = ecmdCommandArgs(&argc, &argv);
      if (rc) return rc;

      /* There shouldn't be any more args when doing a -stdin/-shell */
      if (argc > 1) {
        ecmdOutputError("ecmd - Invalid args passed to ecmd in -stdin/-shell mode\n");
        return ECMD_INVALID_ARGS;
      }

      /* Let's get things going */
      std::vector<std::string> commands;
      int   c_argc;
      char* c_argv[ECMD_ARG_LIMIT + 1];       ///< A limit of 20 tokens(args) per command
      char* buffer = NULL;
      size_t   bufflen = 0;
      size_t   commlen;
      bool shellAlive = true;
      bool pipeMode = false;
      std::string pipeCmd;
      bool redirMode = false;
      std::string redirFile;

      FILE *stream = NULL;

      if (shellMode) {
        //setupEcmds(); /* Removed, not used - JTA 10/15/09 */
        ecmdOutput(getEcmdPrompt().c_str()); fflush(0);
      }

      /* ecmdParseStdInCommands reads from stdin and returns a vector of strings */
      /*  each string contains one command (ie 'ecmdquery version')              */
      /* When Ctrl-D or EOF is reached this function will fail to break out of loop */

      while (shellAlive) {
        if (shellMode) {
          /* We're going to have to be smart about this */
          /* AIX: always do the old way (ecmdParseStdinCommands) */
          /* Linux: If it's a typescript (wm), do it the old way */
          /* Otherwise, use hans new code that runs system commands, etc.. JTA 10/15/09 */
#ifdef _AIX
          rc = ecmdParseStdinCommands(commands);
#else
          char * tempptr = getenv("TERM");
          if (!strcmp(tempptr, "wm")) {
            rc = ecmdParseStdinCommands(commands);
          } else {
            rc = ecmdParseShellCommands(commands);
          }
#endif
        } else {
          rc = ecmdParseStdinCommands(commands);
        }

        if (!rc) break;
        
        /*Here rc=1. But we treat rc as success when it is ECMD_SUCCESS(0) in the codes below.
          so reset rc to ECMD_SUCCESS to avoid break out of code due to its value = 1 */
        rc = ECMD_SUCCESS;
        
        /* Walk through individual commands from ecmdParseStdInCommands */
        for (std::vector< std::string >::iterator commandIter = commands.begin(); commandIter != commands.end(); commandIter++) {
          isSystemCmd = false;
          c_argc = 0;
          c_argv[0] = NULL;

          /* Check for a comment or empty line, if so delete it */
          if ((*commandIter)[0] == '#') {
            continue;
          } else if (commandIter->length() == 0) {
            continue;
          }
          // remove leading blanks
          if (commandIter->length() > 0)
          {
            for (unsigned int a=0; a<commandIter->length(); a++)
            {
              if (commandIter->at(0)==' ')
              {
                commandIter->erase(0,1);
              }
              else
                break;
            }
          }
          if (shellMode) {
            if ((*commandIter) == "quit" || (*commandIter) == "exit") {
              ecmdOutput("Leaving ecmd shell at users request... \n");
              shellAlive = false;
              break;
            }
            // ---- catch the cd command ----------
            if ((commandIter->substr(0,3)) == "cd ")
            {
              commandIter->erase(0,3);
              int cdrc = chdir(&(*commandIter->c_str()));
              if (cdrc)
              {
                ecmdOutput("eCMD bash: No such file or directory");
              }
              continue;
            }
            // ---- catch the pipe command ----------
            pipeCmd.clear();
            pipeMode=false;
            int pipeStart =0;
            pipeStart = commandIter->find_first_of('|',0);
            if (pipeStart != -1)
            {
              pipeCmd = commandIter->substr(pipeStart,commandIter->length()-pipeStart);           
              pipeCmd.erase(0,1);
              commandIter->erase(pipeStart,commandIter->length()-pipeStart);
              printf("ecmd snippet CMD :%s \n",commandIter->c_str());
              if((stream = freopen("ecmdpipe.txt", "w", stdout)) == NULL)
              {
                printf("freopen exception !! \n");
              }
              else
              {
                pipeMode = true;
              }

            }
            // ---- catch the redirect to file cmd ----------
            else if ( commandIter->find_first_of('>',0) != std::string::npos)
            {
              int redirStart =0;
              redirFile.clear();
              redirMode = false;

              redirStart = commandIter->find_first_of('>',0);
              redirFile = commandIter->substr(redirStart,commandIter->length()-redirStart);           
              redirFile.erase(0,1);
              // erase leading blanks in filename

              if (redirFile.length() > 0)
              {
                for (unsigned int a=0; a<redirFile.length(); a++)
                {
                  if (redirFile.at(0)== ' ')
                  {
                    redirFile.erase(0,1);
                  }
                  else
                    break;
                }
              }
              commandIter->erase(redirStart,commandIter->length()-redirStart);
              if((stream = freopen(redirFile.c_str(), "w", stdout)) == NULL)
              {
                printf("freopen redir exception !! \n");
              }
              else
              {
                redirMode = true;
              }

            }
          }

          /* Create a char buffer to hold the whole command, we will use this to create pointers to each token in the command (like argc,argv) */
          commlen = commandIter->length();
          if ( commlen > bufflen) {
            if (buffer != NULL) delete[] buffer;
            buffer = new char[commlen + 20];
            if (buffer == NULL)
            {
               bufflen = 0;
               break;
            }
            else
               bufflen = commlen + 19;
          }


          strcpy(buffer, commandIter->c_str());

          /* Now start carving this thing up */
          bool lookingForStart = true; /* Are we looking for the start of a word ? */
          for (size_t c = 0; c < commlen; c++) {
            if (lookingForStart) {
              if (buffer[c] != ' ' && buffer[c] != '\t') {
                c_argv[c_argc++] = &buffer[c];
                lookingForStart = false;
              }
            } else {
              /* Looking for the end */
              if (buffer[c] == ' ' || buffer[c] == '\t') {
                buffer[c] = '\0';
                lookingForStart = true;
              }
            }
            if (c_argc > ECMD_ARG_LIMIT) {
              sprintf(errorbuf,"ecmd - Found a command with greater then %d arguments, not supported\n",ECMD_ARG_LIMIT);
              ecmdOutputError(errorbuf);
              rc = ECMD_INVALID_ARGS;
              break;
            }
          }

          // ignore a line if it is only space or tabs - 
          // This prevents c_argv[o] being accessed below when still pointing to NULL
          if (c_argc == 0) continue;

          // Before Executing the cmd save it on the Dll side 
          ecmdSetCurrentCmdline(c_argc, c_argv);
          curCmd = "";
          for (int j=0 ; j < c_argc ; j++ )
          {
            curCmd += c_argv[j];
            curCmd += " ";
          }
          curCmd+="\n";

          /* We now want to call the command interpreter to handle what the user provided us */
          rc = ecmdCallInterpreters(c_argc, c_argv);
          if ((rc == ECMD_INT_UNKNOWN_COMMAND) || (rc == ECMD_UNKNOWN_HELP_FILE)) {
            if (strlen(c_argv[0]) < (ERRORBUF_SIZE - 50)) {
              //sprintf(errorbuf,"executing system cmd: %s  \n",curCmd.c_str());
              //ecmdOutputError(errorbuf);
              isSystemCmd = true;
              // to avoid a warning, +1 here or negate the return value when cast to void
              (void)(system(curCmd.c_str())+1);
            } else {
              sprintf(errorbuf,"ecmd -  Unknown Command specified \n");
              ecmdOutputError(errorbuf);
            }
          } else if (rc) {
            std::string parse = ecmdGetErrorMsg(ECMD_GET_ALL_REMAINING_ERRORS, false);
            if (parse.length() > 0) {
              /* Display the registered message right away BZ#160 */
              ecmdOutput(parse.c_str());
              ecmdOutput("\n");
            }

            // If we did get a return code, parse that and print it to the screen
            parse = ecmdParseReturnCode(rc);
            if (strlen(c_argv[0]) + parse.length() < (ERRORBUF_SIZE - 50))
              sprintf(errorbuf,"ecmd - '%s' returned with error code 0x%X (%s)\n", c_argv[0], rc, parse.c_str());
            else
              sprintf(errorbuf,"ecmd - Command returned with error code 0x%X (%s)\n", rc, parse.c_str());
            ecmdOutputError(errorbuf);
            break;
          }
          if (pipeMode)
          {

            //  get current tty ..
            // to avoid a warning, +1 here or negate the return value when cast to void
            (void)(system("tty > ttyfile.txt")+1);

            char str[20];
            (void)memset(str,0,sizeof(str));
            std::ifstream file_op("ttyfile.txt");
            file_op.getline(str,20);
            file_op.close();


            // .. and use it to return control to the current terminal)
            if((stream = freopen(str, "w", stream)) == NULL)
            {
              printf("tty : exception \n");
            }

            pipeCmd = pipeCmd + (std::string)" ecmdpipe.txt";
            // to avoid a warning, +1 here or negate the return value when cast to void
            (void)(system(pipeCmd.c_str())+1);
            // back to normal output

            printf("pipeMode: exited \n");
            pipeMode=false;

          }
          if (redirMode)
          {

            //  get current tty ..
            // to avoid a warning, +1 here or negate the return value when cast to void
            (void)(system("tty > ttyfile.txt")+1);

            char str[20];
            (void)memset(str,0,sizeof(str));
            std::ifstream file_op("ttyfile.txt");
            file_op.getline(str,20);
            file_op.close();


            // .. and use it to return control to the current terminal)
            if((stream = freopen(str, "w", stream)) == NULL)
            {
              printf("tty : exception \n");
            }

            redirMode=false;

          }

          if (!ecmdGetGlobalVar(ECMD_GLOBALVAR_QUIETMODE)) {
            if (isSystemCmd == false) {
              ecmdOutput((*commandIter).c_str());
              ecmdOutput("\n");
            }
          }
        } /* tokens loop */
        /* Removing the rc check to prevent the shell from exiting on failing command - JTA 02/16/09 */
        /* MAB D713111 07/08/09 - if in stdinMode (not shellMode) and rc, then break */
        if (stdinMode && rc) break;

        /* Print the prompt again */
        if (shellMode && shellAlive) {
          ecmdOutput(getEcmdPrompt().c_str()); fflush(0);
        }
      }
      if (buffer != NULL) delete[] buffer;

    } else {
      /* Standard command line command */

      if (argv[1] == NULL) {
          sprintf(errorbuf,"ecmd - Must specify a command to execute. Run 'ecmd -h' for command list.\n");
          ecmdOutputError(errorbuf);
          rc = ECMD_INT_UNKNOWN_COMMAND;
          std::string parse = ecmdParseReturnCode(rc);
          if (parse.length() < (ERRORBUF_SIZE - 50))
              sprintf(errorbuf,"ecmd - <Null argument> returned with error code 0x%X (%s)\n", rc, parse.c_str());
          ecmdOutputError(errorbuf);
      } else {

        // Before Executing the cmd save it on the Dll side 
        ecmdSetCurrentCmdline(argc-1, argv+1);

        /* We now want to call the command interpreter to handle what the user provided us */
        rc = ecmdCallInterpreters(argc - 1, argv + 1);
        if (rc == ECMD_INT_UNKNOWN_COMMAND) {
          if (strlen(argv[1]) < (ERRORBUF_SIZE - 50))
            sprintf(errorbuf,"ecmd - Unknown Command specified '%s'\n", argv[1]);
          else
            sprintf(errorbuf,"ecmd - Unknown Command specified \n");
          ecmdOutputError(errorbuf);
        } else if (rc) {

          // See if any errors are left, regardless of if we have a return code.  This will prevent error messages from being lost
          std::string parse = ecmdGetErrorMsg(ECMD_GET_ALL_REMAINING_ERRORS, false);
          /* Display the registered message right away BZ#160 */
          if (parse.length() > 0) {
            ecmdOutput(parse.c_str());
          }

          // If we did get a return code, parse that and print it to the screen
          parse = ecmdParseReturnCode(rc);
          if (strlen(argv[1]) + parse.length() < (ERRORBUF_SIZE - 50))
            sprintf(errorbuf,"ecmd - '%s' returned with error code 0x%X (%s)\n", argv[1], rc, parse.c_str());
          else
            sprintf(errorbuf,"ecmd - Command returned with error code 0x%X (%s)\n", rc, parse.c_str());
          ecmdOutputError(errorbuf);
        }
      }
    }

    /* Only print to the screen if quietmode isn't on */
    if (!ecmdGetGlobalVar(ECMD_GLOBALVAR_QUIETMODE)) {
      ecmdOutput(cmdSave.c_str());
    }

    // If you check the rc, don't use the "rc" variable, it will corrupt the return value
    ecmdUnloadDll();
  }

  return rc;
}
