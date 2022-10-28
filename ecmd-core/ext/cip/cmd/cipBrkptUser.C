//IBM_PROLOG_BEGIN_TAG
/* 
 * Copyright 2006,2019 IBM International Business Machines Corp.
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
#include <algorithm> // for transform

#include <ecmdCommandUtils.H>
#include <ecmdReturnCodes.H>
#include <ecmdClientCapi.H>
#include <ecmdUtils.H>
#include <ecmdDataBuffer.H>
#include <ecmdSharedUtils.H>

#include <cipClientCapi.H>

//----------------------------------------------------------------------
//  User Types
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//  Macros
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//  Constants
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

#ifndef CIP_REMOVE_BREAKPOINT_FUNCTIONS
uint32_t cipBrkptUser(int argc, char* argv[])
{
  uint32_t rc = ECMD_SUCCESS;
  uint32_t coeRc = ECMD_SUCCESS;
  std::string printed;
  std::string temp;
  char buf[200];
  ecmdChipTarget target;                        ///< Current target being operated on
  ecmdChipTarget cuTarget;                      ///< Current target being operated on for the chipUnit
  ecmdLooperData looperdata;
  bool validPosFound = false;                   ///< Did the looper find anything?
  bool addrTypeSet= false;                      ///< do we need to check for address?
  bool breakOut = false;                        ///< used to control looper for get and clear
  // Breakpoint related defines
  std::list<cipBrkptTableEntry>  l_brkptTableEntries;  ///< buffer to hold Breakpoint table for all calls
  std::list<cipBrkptTableEntry>  lt_brkptTableEntries; ///< local buffer to hold Breakpoint table
  cipXlateVariables l_xlate;                           ///< User defined variables
  ecmdDataBuffer    l_brkptAddr_96(12*8);                 ///< Address to set Breakpoint
  std::list<cipBrkptTableEntry>::iterator bpt_itor;

  ecmdDataBuffer    l_virtAddrOut_96(12*8);
  uint32_t l_addressSize;
  uint32_t bpt_size;
  bool clearAll=false;                                 ///< variable to support clear

  //make sure we have the BP address cleared out
  l_brkptAddr_96.flushTo0();
  // set defaults to xlate structure
  // from meeting with Piranha team(Mark B)we set defaults accroding to Piranha
  // for now only partitionId can be overidden
  l_xlate.tagsActive = true;                           ///< tags active
  l_xlate.mode32bit = false;                           ///< 64 bit
  l_xlate.writeECC = false;                            ///< have ECC calculated
  l_xlate.manualXlateFlag = false;                     ///< Manual translation not needed  
  l_xlate.addrType = CIP_MAINSTORE_REAL_ADDR;          ///< Setting address to Real by default
  l_xlate.partitionId = 0;                             ///< default partition ID to 0

  if (ecmdParseOption(&argc, &argv, "all")) {
     clearAll=true; //foe clear option
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
     if (argc < 1) {  //operation + misc
       ecmdOutputError("cipbrkpt - Too few arguments specified; you need at least a operation type.\n");
       ecmdOutputError("cipbrkpt - Type 'cipbrkpt -h' for usage.\n");
       return ECMD_INVALID_ARGS;
     }

  // from here on we do specific interface type
  if ((!strcmp(argv[0], "set")) || (!strcmp(argv[0], "get"))|| (!strcmp(argv[0], "clear"))) {

     // now check to see what is the type of address we are looking at
     if( argc >=2 ){
        temp = argv[1];
        transform(temp.begin(), temp.end(), temp.begin(), (int(*)(int)) tolower);
          if(temp == "real"){
            l_xlate.addrType = CIP_MAINSTORE_REAL_ADDR;
            addrTypeSet=true;
          } else if (temp == "effective") {
            l_xlate.addrType = CIP_MAINSTORE_EFFECTIVE_ADDR;
            addrTypeSet=true;
          } else if (temp == "virtual"){
            l_xlate.addrType = CIP_MAINSTORE_VIRTUAL_ADDR;
            addrTypeSet=true;
          } else {
            // prompt user only for setting BP's
            if(!strcmp(argv[0], "set")){
              ecmdOutputError("cipbrkpt - Adress needs to be one of the address types(real/effective/virtual).\n");
              return ECMD_INVALID_ARGS;
            }
         }
     }

     //check for address if the type is set
     if(addrTypeSet){
     // check if the address supplied is valid hex
        if( argc >=3 ){
          if (!ecmdIsAllHex(argv[2])) {
            ecmdOutputError("cipbrkpt - Non-hex characters detected in address field\n");
            return ECMD_INVALID_ARGS;
          }
          //  we are restricting address to 96 bits
          if ( strlen(argv[2]) > 24 ) // strlen does NOT count NULL terminator
          {
             ecmdOutputError("cipbrkpt - Address field is too large (>24 chars). It is restricted to 96 bits (12bytes)\n");
             return ECMD_INVALID_ARGS;
          }
          l_addressSize = strlen(argv[2]) * 4; //total bits, 4= 1 nibble = 1 character
          rc = l_brkptAddr_96.insertFromHexRight(argv[2], (l_brkptAddr_96.getBitLength() - l_addressSize),l_addressSize );
        } else {
          // prompt user only for setting BP's
          if(!strcmp(argv[0], "set")){
            ecmdOutputError("cipbrkpt - Adress needs to be provided to set Breakpoint\n");
            return ECMD_INVALID_ARGS;
          }
        }
     }
     // we are going to set default for tagsActive, ecc, mode, xlate
     // current option is to override the partition Id, if the user does not provide any
     // we set the partition id to 0
     char* partionId= ecmdParseOptionWithArgs(&argc, &argv, "partitionId");

     if (partionId != NULL ){
        if (!ecmdIsAllDecimal(partionId)) {
           ecmdOutputError("cipbrkpt - Non-Decimal characters detected in partition field\n");
           return ECMD_INVALID_ARGS;
        }else{
           l_xlate.partitionId = (uint32_t)atoi(partionId);
        }
     }

     // for set operation we need to make sure the Address type and address are valid.
     uint32_t temp0,temp1,temp2;
     temp0=l_brkptAddr_96.getWord(0);
     temp1=l_brkptAddr_96.getWord(1);
     temp2=l_brkptAddr_96.getWord(2);
     if(!strcmp(argv[0], "set") && !addrTypeSet && (temp0 == 0) && (temp1 == 0) && (temp2== 0)){
         ecmdOutputError("cipbrkpt - Address type and Adress needs to be provided to set Breakpoint\n");
         return ECMD_INVALID_ARGS;
     }

     // setting up target
     // we set breakpoints at the thread level so setup the target accordingly.
     target.cageState = target.nodeState = target.slotState = target.posState = ECMD_TARGET_FIELD_WILDCARD;
     target.chipType = "pu"; // we know for sure this needs to be set on proc
     target.chipTypeState = ECMD_TARGET_FIELD_VALID;
     target.chipUnitType = "c"; // we use core
     target.chipUnitTypeState = ECMD_TARGET_FIELD_VALID;
     target.chipUnitNumState = ECMD_TARGET_FIELD_WILDCARD;
     // setting the thread state to wildcard to accept any thread value passed by user
     target.threadState = ECMD_TARGET_FIELD_WILDCARD;

     /************************************************************************/
     /* Kickoff Looping Stuff                                                */
     /************************************************************************/
     rc = ecmdLooperInit(target, ECMD_SELECTED_TARGETS_LOOP, looperdata);
     if (rc) return rc;

     // Implementing looper way.
     while (ecmdLooperNext(target, looperdata) && (!coeRc || coeMode)) {
       //now that we have a target make sure we have it configured at the thread level.
       if (!strcmp(argv[0], "set")){
          rc = cipSetSoftwareBreakpoint(target, l_brkptAddr_96, l_xlate, l_brkptTableEntries, l_virtAddrOut_96);
          if (rc != ECMD_SUCCESS)
          {
             // Failed setting breakpoint, build message and return
             sprintf(buf, "Unable to set breakpoint at address 0x%s, rc = 0x%x\n", l_brkptAddr_96.genHexLeftStr().c_str(), rc);
             coeRc=rc;
             ecmdOutputError(buf);
             break;
          }
        }else if (!strcmp(argv[0], "get")){

          rc = cipGetSoftwareBreakpoint (target, l_brkptAddr_96, l_xlate, l_brkptTableEntries, l_virtAddrOut_96);
          if (rc != ECMD_SUCCESS)
          {
             sprintf(buf, "cipGetSoftwareBreakpoint() failed with rc = 0x%x\n",rc);
             coeRc=rc;
             ecmdOutputError(buf);
             break;
          }
             // looper is common for all the operations. We will break if we are here
             // running a empty loop incase of wildcard will give us empty list.
             breakOut=true;
        }else if (!strcmp(argv[0], "clear")){
          // first get the list of Breakpoints      
          rc = cipGetSoftwareBreakpoint (target, l_brkptAddr_96, l_xlate, l_brkptTableEntries, l_virtAddrOut_96);
          if (rc != ECMD_SUCCESS)
          {
             sprintf(buf, "cipGetSoftwareBreakpoint() failed with rc = 0x%x\n",rc);
             coeRc=rc;
             ecmdOutputError(buf);
             break;
          } else {
             bpt_size = l_brkptTableEntries.size();
             if (bpt_size > 0) {
                bpt_itor = l_brkptTableEntries.begin();
                ecmdDataBuffer    lt_brkptAddr(12*8);
                bool bpCleared =false;
                for ( uint32_t i=0; i < bpt_size; i++, bpt_itor++){
                    lt_brkptAddr.insertFromHexRight(bpt_itor->Address.genHexRightStr().c_str(),16,80);
                    //compare both arrays and make sure we compare 80bits for match.
                    //first array for l_brkptAddr_96 has the 4 nibbles for padding so neglect it for comparision
                    if( ( (l_brkptAddr_96 == lt_brkptAddr) &&  // compare for 80bit address
                        (bpt_itor->Addr_Type == l_xlate.addrType) && // is the address type same?
                        (bpt_itor->Partition_Id == l_xlate.partitionId) )|| // same partition?
                         clearAll){

                         if(clearAll){
                        //for all option we need to make sure the address passed also has the proper xstate set
                        //Breakpoints will not be deleted if the xstate does not match
                        //copy over the xlate params
                           cipMainstoreAddrType_t  addr_type=CIP_MAINSTORE_REAL_ADDR;
                           l_xlate.tagsActive =  bpt_itor->TA;
                           l_xlate.mode32bit = bpt_itor->Addr_Mode;
                           switch( bpt_itor->Addr_Type ){
                              case 1: addr_type=CIP_MAINSTORE_REAL_ADDR; break;
                              case 2: addr_type=CIP_MAINSTORE_EFFECTIVE_ADDR;break;
                              case 3: addr_type=CIP_MAINSTORE_VIRTUAL_ADDR;break;
                           }
                           l_xlate.addrType = addr_type;
                           l_xlate.partitionId = bpt_itor->Partition_Id;
                        }//clearAll

                        rc = cipClearSoftwareBreakpoint(target, lt_brkptAddr, l_xlate, lt_brkptTableEntries, l_virtAddrOut_96);
                        if (rc!= ECMD_SUCCESS)
                        {
                            sprintf(buf, "cipClearSoftwareBreakpoint() failed with rc=0x%x\n", rc);
                            coeRc=rc;
                            ecmdOutputError(buf);
                            break; // breakout if any error seen during the clearing
                        }else{
                            // if we come here then we had a match and we were successful in clearing the BP
                            bpCleared = true;
                        }    
                    }
                }//for loop
                  //let the user know if the BP was cleared or not
                  if (!bpCleared){
                     sprintf(buf, "Breakpoint not cleared, either address/type/partitionId did not match\n");
                     ecmdOutputWarning(buf);
                  } 
             }
             // we want to display the BP table everytime so try a get for the following loop to print any BP's
             rc = cipGetSoftwareBreakpoint (target, l_brkptAddr_96, l_xlate, l_brkptTableEntries, l_virtAddrOut_96);
             if (rc != ECMD_SUCCESS)
             {
                sprintf(buf, "cipGetSoftwareBreakpoint() failed with rc = 0x%x\n",rc);
                coeRc=rc;
                ecmdOutputError(buf);
                break;
             }  
             // looper is common for all the operations. We will break if we are here
             // running a empty loop incase of wildcard will give us empty list.
             breakOut= true;
          }//else no get error
        }// clear Breakpoint

        //if clearall is called then we have no BP's to display, do not run the loop
        if(!clearAll){
        // we will attempt to print the BP table here. we will get the table returned to us in every call
             bpt_size = l_brkptTableEntries.size();
             // success! print out o_brkptTableEntries
             if (bpt_size > 0) {
                 // print target info for looping
                 printed=ecmdWriteTarget(target); printed+="\n"; ecmdOutput(printed.c_str());
                 // if we have entries to display then print out the common header
                 printed = "Entry Address              Instruction      Par_Id Installed TA Mode Addr_Type\n"; ecmdOutput(printed.c_str());
                 printed = "===== ==================== ================ ====== ========= == ==== =========\n"; ecmdOutput(printed.c_str());
                 bpt_itor = l_brkptTableEntries.begin();
                 std::string addr_type;
                 // now loop through the table to getall the entries
                 for ( uint32_t i=0; i < bpt_size; i++, bpt_itor++){
                     // convert to aprropriate string values
                     switch( bpt_itor->Addr_Type ){
                        case 0: addr_type="Unknown"; break;
                        case 1: addr_type="Real"; break;
                        case 2: addr_type="Effective";break;
                        case 3: addr_type="Virtual";break;
                     }
                     sprintf(buf,"%3d   %-20s %-16s   %-2d       %-3s    %-1s  %-2s  %-9s\n", (i+1), bpt_itor->Address.genHexLeftStr().c_str(),
                         bpt_itor->Original_Instruction.genHexLeftStr().c_str(), bpt_itor->Partition_Id,
                         ((bpt_itor->Installed == 1)?"Yes":"No"),((bpt_itor->TA == 1)?"A":"N"),
                         ((bpt_itor->Addr_Mode== 1)?"32":"64"),addr_type.c_str());
                     ecmdOutput(buf);
                }//for loop ends
             } else {
                ecmdOutput("No Breakpoints found.\n");
             }
        }
        validPosFound = true;//setting true as we did find a target to make sensor call
        if ( breakOut ) break;
  
     }
     // coeRc will be the return code from in the loop, coe mode or not.
     if (coeRc) return coeRc;

     // This is an error common across all UI functions
     if (!validPosFound) {
        ecmdOutputError("cipbrpkt - Unable to find a valid chip to execute command on\n");
        return ECMD_TARGET_NOT_CONFIGURED;
     }
  }else{
    /* Invalid operation*/
    ecmdOutputError("cipbrpkt - Invalid Operation Mode.\n");
    ecmdOutputError("cipbrpkt - Type 'cipbrpkt -h' for usage.\n");
    return ECMD_INVALID_ARGS;
  }
  return rc;
}
#endif
