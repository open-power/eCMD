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


//--------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------
#include <fstream>
#include <inttypes.h>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>

#include <ecmdUtils.H>
#include <ecmdSharedUtils.H>
#include <ecmdStructs.H>
#include <ecmdClientCapi.H>
#include <ecmdReturnCodes.H>

#if !(defined (ECMD_REMOVE_SEDC_SUPPORT) || defined (ECMD_REMOVE_SCOM_FUNCTIONS))
# include <sedcScomdefParser.H>
# include <sedcScomdefClasses.H>
#endif

//----------------------------------------------------------------------
//  Constants
//----------------------------------------------------------------------



typedef enum {
  ECMD_FORMAT_NONE,
  ECMD_FORMAT_X,
  ECMD_FORMAT_XR,
  ECMD_FORMAT_XW,
  ECMD_FORMAT_XRW,
  ECMD_FORMAT_A,
  ECMD_FORMAT_B,
  ECMD_FORMAT_BN,
  ECMD_FORMAT_BW,
  ECMD_FORMAT_BX,
  ECMD_FORMAT_BXN,
  ECMD_FORMAT_BXW,
  ECMD_FORMAT_MEM,
  ECMD_FORMAT_MEMA,
  ECMD_FORMAT_MEMD,
  ECMD_FORMAT_MEME,
  ECMD_FORMAT_D
} ecmdFormatState_t;

#ifndef ECMD_STRIP_DEBUG
char frontFPPTxt[40]; //@01c Bumped from 35 to 40 (max str len = 37)
extern int  fppCallCount;
extern int ecmdClientDebug;
extern bool ecmdDebugOutput;
#endif


//--------------------------------------------------------------------
// Macros
//--------------------------------------------------------------------


#ifndef ECMD_STRIP_DEBUG
std::string printEcmdChipTargetState_t(ecmdChipTargetState_t state);
/**
 @brief Print ecmdDataBuffer's for the EFPP
 @param i_data Databuffer to print
 @param i_tabStop Any additional spacing that should be done in display (ie "  \t")
*/
void printEcmdDataBuffer(std::string variableType, std::string variableName, const ecmdDataBuffer & i_data, std::string i_tabStop);

void debugFunctionOuput(const char* outbuf);
#endif

#if !(defined (ECMD_REMOVE_SEDC_SUPPORT) || defined (ECMD_REMOVE_SCOM_FUNCTIONS))
/**
 @brief Read in the scomdef file and find the start of the specified entry.
 @retval ECMD_SUCCESS if scom lookup and display was successful, non-zero otherwise
 @param i_address Scom Address which we are looking for 
 @param io_scomdefFile File stream for the data, which is set to the start of the entry upon exit

 This function is used by ecmdDisplayScomData and not intended to be called by the regular user
 */
uint32_t readScomDefFile(uint64_t i_address, std::ifstream &io_scomdefFile);
#endif

uint32_t ecmdReadDataFormatted(ecmdDataBuffer & o_data, const char * i_dataStr, std::string i_format, int i_expectedLength) {
  uint32_t rc = ECMD_SUCCESS;

  uint32_t bitlength;
  ecmdCompressionMode_t compressMode = ECMD_COMP_UNKNOWN;

  /* Look to see if the user wants his data compressed after it was read in */
  size_t pos = i_format.find("c");

  if (pos != std::string::npos) {
    // They gave the compress flag, make sure it's a valid option then setup for it
    if (i_format[(pos+1)] == 'p') {
      compressMode = ECMD_COMP_PRD;
    } else if (i_format[(pos+1)] == 'z') {
      compressMode = ECMD_COMP_ZLIB;
    } else {
      ecmdOutputError(( "Input Data contained an invalid compression mode : '" + i_format + "'!\n").c_str());
      return ECMD_INVALID_ARGS;
    }

    // Now cleanup the string so the code below works
    i_format.erase(pos,2);
  }

  if (i_format == "x" || i_format == "xl") {
    rc = o_data.insertFromHexLeftAndResize(i_dataStr, 0, i_expectedLength);
  }
  else if (i_format == "xr") {
    rc = o_data.insertFromHexRightAndResize(i_dataStr, 0, i_expectedLength);
  }     
  else if (i_format == "b") {
    rc = o_data.insertFromBinAndResize(i_dataStr, 0, i_expectedLength);
  }
#ifndef REMOVE_SIM
  else if (i_format == "bX") {
    bitlength = strlen(i_dataStr);
    if (i_expectedLength != 0) bitlength = i_expectedLength;
    o_data.enableXstateBuffer();
    o_data.setBitLength(bitlength);
    o_data.setXstate(0,i_dataStr);
  }
#endif
  else if (i_format == "a") {
    bitlength = strlen(i_dataStr)*8;
    if (i_expectedLength != 0) bitlength = i_expectedLength;
    o_data.setBitLength(bitlength);
    rc = o_data.insertFromAscii(i_dataStr, 0);
  }
  else if (i_format == "d") {
    if(strlen(i_dataStr) > 10) {
     ecmdOutputError( "Integer overflow. Decimal number should be less that 4G.\n" );
     rc = ECMD_INVALID_ARGS;
    }
    else if(strlen(i_dataStr) == 10) {
      //Highest number for 32 bits is 4294967295
      if( strcmp(i_dataStr, "4294967295") > 0 ) {
        ecmdOutputError( "Integer overflow. Decimal number should be less that 4G.\n" );
        rc = ECMD_INVALID_ARGS;
      }
    }
    if(!rc) {
     uint32_t decdata = decToUInt32(i_dataStr);
     bitlength = 32;
     if (i_expectedLength != 0) bitlength = i_expectedLength;
     /* Put it in the whole word and then shift, as decimal data is assumed to be right aligned */
     o_data.setBitLength(bitlength);
     rc = o_data.insertFromRight(decdata, 0, bitlength);
    }
  }   
  else {
    ecmdOutputError( ("Did not recognize input format string " + i_format + "\n").c_str() );
    rc = ECMD_INVALID_ARGS;
  }
  if (rc == ECMD_DBUF_INVALID_DATA_FORMAT) {
    ecmdOutputError(( "Input Data contained some invalid characters for format specified : '" + i_format + "'!\n").c_str());
  }

  // All done reading in the data, compress it if the use said to earlier
  if (compressMode != ECMD_COMP_UNKNOWN) {
    o_data.compressBuffer(compressMode);
  }

  return rc;
}

uint32_t decToUInt32(const char *decstr) {
    uint32_t decdata;
    
    decdata=0;
    for(int i=0; decstr[i] >= '0' && decstr[i] <= '9';++i) {
     decdata = 10 * decdata + (decstr[i] - '0');
    }
    return decdata;
}

std::string ecmdWriteDataFormatted(const ecmdDataBuffer & i_data, std::string i_format, uint64_t i_address, ecmdEndianMode_t i_endianMode) {
  std::string printed;
  int formTagLen = i_format.length();
  ecmdFormatState_t curState = ECMD_FORMAT_NONE;
  int numCols = 0;
  bool good = true;
  bool compression = false;
  const ecmdDataBuffer * l_pData = & i_data;
  ecmdDataBuffer l_uncompressedData;

  // We have to check for the memory types before we fall into the looping below
  if (i_format.substr(0,3) == "mem") {
    if (i_format[3] == 'a')
      curState = ECMD_FORMAT_MEMA;
    else if (i_format[3] == 'd')
      curState = ECMD_FORMAT_MEMD;
    else if (i_format[3] == 'e')
      curState = ECMD_FORMAT_MEME;
    else
      curState = ECMD_FORMAT_MEM;
  } else { // No mem
    for (int i = 0; i < formTagLen; i++) {

      if (i_format[i] == 'x') {
        if (curState != ECMD_FORMAT_NONE) {
          good = false;
          break;
        }

        curState = ECMD_FORMAT_X;
      }
      else if (i_format[i] == 'l') {
        if (curState != ECMD_FORMAT_X) {
          good = false;
          break;
        }

      }
      else if (i_format[i] == 'r') {
        if (curState != ECMD_FORMAT_X) {
          good = false;
          break;
        }

        curState = ECMD_FORMAT_XR;
      }
      else if (i_format[i] == 'b') {
        if (curState != ECMD_FORMAT_NONE) {
          good = false;
          break;
        }

        curState = ECMD_FORMAT_B;
      }
      else if (i_format[i] == 'a') {
        if (curState != ECMD_FORMAT_NONE) {
          good = false;
          break;
        }

        curState = ECMD_FORMAT_A;
      }
      else if (i_format[i] == 'n') {
        if (curState == ECMD_FORMAT_B) {
          curState = ECMD_FORMAT_BN;
        } else if (curState == ECMD_FORMAT_BX) {
          curState = ECMD_FORMAT_BXN;
        } else {
          good = false;
          break;
        }

      }
      else if (i_format[i] == 'w') {
        if (curState == ECMD_FORMAT_X) {
          curState = ECMD_FORMAT_XW;
        }
        else if (curState == ECMD_FORMAT_XR) {
          curState = ECMD_FORMAT_XRW;
        }
        else if (curState == ECMD_FORMAT_B) {
          curState = ECMD_FORMAT_BW;
        }
        else if (curState == ECMD_FORMAT_BX) {
          curState = ECMD_FORMAT_BXW;
        }
        else {
          good = false;
          break;
        }

      }
      else if (i_format[i] == 'X') {
        if (curState != ECMD_FORMAT_B) {
          good = false;
          break;
        }

        curState = ECMD_FORMAT_BX;
      }
      else if (i_format[i] == 'd') {
        if (curState != ECMD_FORMAT_NONE) {
          good = false;
          break;
        }

        curState = ECMD_FORMAT_D;
      }
      else if (i_format[i] == 'c') {
        compression = true;
      }
      else if (isdigit(i_format[i])) {
        numCols *= 10;
        numCols += atoi(&i_format[i]);
        break;
      }
      else {
        good = false;
        break;
      }

    }
  }

  if (curState == ECMD_FORMAT_NONE) {
    good = false;
  }

  if (!good) {
    printed = "Unrecognized format string: ";
    printed += i_format + "\n";
    ecmdOutputError(printed.c_str());
    printed = "";
    return printed;
  }

  /* If compression was enabled on the buffer, and the user asked to view it, uncompress the data */
  if (compression) {
    if (l_pData->isBufferCompressed()) {
      // copy the data before decompressing
      l_uncompressedData = *l_pData;
      uint32_t l_rc = l_uncompressedData.uncompressBuffer();
      if (l_rc) {
        printed = "Compressed data failed to uncompress!\n";
        ecmdOutputError(printed.c_str());
        printed = "";
        return printed;
      }
      l_pData = & l_uncompressedData;
    } else {
      printed = "Compressed data option was given on the cmdline, but data wasn't compressed!\n";
      ecmdOutputError(printed.c_str());
      printed = "";
      return printed;
    }
  }

  if (curState == ECMD_FORMAT_X) {
    printed = "0x" + l_pData->genHexLeftStr();
    printed += "\n";
  }
  else if (curState == ECMD_FORMAT_XR) {
    printed = "0xr" + l_pData->genHexRightStr();
    printed += "\n";
  }
  else if (curState == ECMD_FORMAT_B) {
    printed = "0b" + l_pData->genBinStr();
    printed += "\n";
  }
  else if (curState == ECMD_FORMAT_A) {
    if (l_pData->getByteLength()>0) {
      printed =  l_pData->genAsciiStr(0, l_pData->getBitLength()).substr(0,l_pData->getByteLength());
      printed += "\n";
    }
  }
  else if (curState == ECMD_FORMAT_D) {
    if (l_pData->getBitLength() > 32) {
      ecmdOutputError("ecmdWriteDataFormatted - Unable to generate decimal numbers for buffers bigger than 32 bits\n");
      printed = "";
      return printed;
    }
    /* Short enough, let generate this number */
    char tempstr[100];
    uint32_t decimalData = l_pData->getWord(0);
    /* Right align it */
    decimalData >>= (32 - l_pData->getBitLength());
    /* Print it */
    sprintf(tempstr, "%d\n", decimalData);
    printed = tempstr;
  }
#ifndef REMOVE_SIM
  else if (curState == ECMD_FORMAT_BX) {
    if (!l_pData->isXstateEnabled()) {
      ecmdOutputError("ecmdWriteDataFormatted - Write of X-state data required but Xstate buffer not enabled\n");
      printed = "";
      return printed;
    }
    printed = "0b" + l_pData->genXstateStr();
    printed += "\n";
  }
#endif
  else if (curState == ECMD_FORMAT_MEM || curState == ECMD_FORMAT_MEMA || curState == ECMD_FORMAT_MEME) {
    uint64_t myAddr = i_address;
    uint32_t wordsDone = 0;
    std::string lastBytes;
    char tempstr[400];
    uint32_t wordsDonePrev;
    int numLastBytes=0;
    int i=0;
    
    //if not exact word multiple
    if ( (l_pData->getWordLength()*4) > (l_pData->getByteLength()) ) {
      numLastBytes = 4 - ((l_pData->getWordLength()*4) - (l_pData->getByteLength()));
    }
    // Loop through the complete 4 word blocks
    while ((l_pData->getWordLength() - wordsDone) > 3) {
      wordsDonePrev = wordsDone;
      //last word
      if ( (l_pData->getWordLength() == (wordsDone+4)) && (numLastBytes != 0)) {
        lastBytes = l_pData->genHexLeftStr(((wordsDone+3)*32), (numLastBytes*8));
        sprintf(tempstr,UINT64_HEX16_FORMAT ": %08X %08X %08X %s", myAddr, l_pData->getWord(wordsDone), l_pData->getWord(wordsDone+1), l_pData->getWord(wordsDone+2), lastBytes.c_str());
        i=0;
        while (i < (4-numLastBytes)) {
          strcat(tempstr, "  "); i++;
        }
      }
      else {
        sprintf(tempstr,UINT64_HEX16_FORMAT ": %08X %08X %08X %08X", myAddr, l_pData->getWord(wordsDone), l_pData->getWord(wordsDone+1), l_pData->getWord(wordsDone+2), l_pData->getWord(wordsDone+3));
      }
      printed += tempstr;
      // Text printing additions
      if (curState == ECMD_FORMAT_MEMA || curState == ECMD_FORMAT_MEME) {
        // ASCII
        if (curState == ECMD_FORMAT_MEMA) {
          if ( (l_pData->getWordLength() == (wordsDone+4)) && (numLastBytes != 0)) {
            sprintf(tempstr,"   [%s]",l_pData->genAsciiPrintStr(wordsDonePrev*32, (128-(8*(4-numLastBytes)))).c_str());
          }
          else {
            sprintf(tempstr,"   [%s]",l_pData->genAsciiPrintStr(wordsDonePrev*32, 128).c_str());
          }
        }
        // EBCDIC
        if (curState == ECMD_FORMAT_MEME) {
          if ( (l_pData->getWordLength() == (wordsDone+4)) && (numLastBytes != 0)) {
            sprintf(tempstr,"   [%s]",ecmdGenEbcdic(*l_pData,wordsDonePrev*32, (128-(8*(4-numLastBytes)))).c_str());
          }
          else {
            sprintf(tempstr,"   [%s]",ecmdGenEbcdic(*l_pData,wordsDonePrev*32, 128).c_str());
          }
        }
        printed += tempstr;
      }
      printed += "\n";
      myAddr += 16;
      wordsDone += 4;
    }
    // Done with all the 4 word blocks, see if we've got some straglers left
    if ((l_pData->getWordLength() - wordsDone) != 0) {
      wordsDonePrev = wordsDone;
      // Print the address
      sprintf(tempstr,UINT64_HEX16_FORMAT ":", myAddr);
      printed += tempstr;
      // Now throw on the words
      while ((uint32_t) wordsDone < l_pData->getWordLength()) {
        //last word
        if ( (l_pData->getWordLength() == (wordsDone+1)) && (numLastBytes != 0)) {
          lastBytes = l_pData->genHexLeftStr((wordsDone*32), (numLastBytes*8));
          sprintf(tempstr," %s", lastBytes.c_str());
          i=0;
          while (i < (4-numLastBytes)) {
            strcat(tempstr, "  "); i++;
          }
          wordsDone++;
        }
        else {
          sprintf(tempstr," %08X",l_pData->getWord(wordsDone++));
        }
        printed += tempstr;
      }
      // Text printing additions
      if (curState == ECMD_FORMAT_MEMA || curState == ECMD_FORMAT_MEME) {
        // Insert spaces from the end of the last incomplete line for alignment
        for (uint32_t y = 0; y < (4 - (wordsDone - wordsDonePrev)); y++) {
          printed.insert(printed.length(),"         ");
        }
        // ASCII
        if (curState == ECMD_FORMAT_MEMA) {
          if (numLastBytes) {
            sprintf(tempstr,"   [%s]",l_pData->genAsciiPrintStr(wordsDonePrev*32, (((wordsDone - wordsDonePrev)*32) - ((4-numLastBytes)*8))).c_str());
          }
          else {
            sprintf(tempstr,"   [%s]",l_pData->genAsciiPrintStr(wordsDonePrev*32, (wordsDone - wordsDonePrev)*32).c_str());
          }
        }
        // EBCDIC
        if (curState == ECMD_FORMAT_MEME) {
          if (numLastBytes) {
            sprintf(tempstr,"   [%s]",ecmdGenEbcdic(*l_pData,wordsDonePrev*32, (((wordsDone - wordsDonePrev)*32) - ((4-numLastBytes)*8))).c_str());
          }
          else {
            sprintf(tempstr,"   [%s]",ecmdGenEbcdic(*l_pData,wordsDonePrev*32, (wordsDone - wordsDonePrev)*32).c_str());
          }
        }
        printed += tempstr;
      }
      printed += "\n";
    }
  }
  else if (curState == ECMD_FORMAT_MEMD) {
    uint64_t myAddr = i_address;
    /*    int wordsDone = 0; */
    char tempstr[400];
    int y=0;


    //if not exact word multiple-To be used for printing in the byte format
    /*int numLastBytes=0;
     std::string lastBytes;
     if ( (l_pData->getWordLength()*4) > (l_pData->getByteLength()) ) {
     numLastBytes = 4 - ((l_pData->getWordLength()*4) - (l_pData->getByteLength()));
     }*/

    // Print out the data
    for (uint32_t x = 0; x < (l_pData->getWordLength() / 2); x++) {
      //To be used for printing in the byte format
      /*if ( ((l_pData->getWordLength()) == y+2) && (numLastBytes != 0)) {
       lastBytes = l_pData->genHexLeftStr(((y+1)*32), (numLastBytes*8));
       sprintf(tempstr,"D %016X %08X%s\n", myAddr, l_pData->getWord(y), lastBytes.c_str());
       }
       else {
       sprintf(tempstr,"D %016X %08X%08X\n", myAddr, l_pData->getWord(y), l_pData->getWord(y+1));
       }
       y += 2;
       */
      sprintf(tempstr,"D " UINT64_HEX16_FORMAT " %08X%08X\n", myAddr, l_pData->getWord(y), l_pData->getWord(y+1));
      y += 2;
      printed += tempstr;
      myAddr += 8;
    }

    if (l_pData->getWordLength() % 2) {
      //To be used for printing in the byte format
      /*if ( numLastBytes != 0) {
       lastBytes = l_pData->genHexLeftStr(((l_pData->getWordLength() - 1)*32), (numLastBytes*8));
       sprintf(tempstr,"D %016X %s\n", myAddr, lastBytes.c_str());
       }
       else {
       sprintf(tempstr,"D %016X %08X00000000\n", myAddr, l_pData->getWord((l_pData->getWordLength() - 1)));
       }*/
      sprintf(tempstr,"D " UINT64_HEX16_FORMAT " %08X00000000\n", myAddr, l_pData->getWord((l_pData->getWordLength() - 1)));
      printed += tempstr;
    }
  }
  else {

    char * outstr = new char[40];
    int curCol = 0;
    int colCount = 0;
    int blockSize = 32;
    if ((curState == ECMD_FORMAT_BN) || (curState == ECMD_FORMAT_BXN))  blockSize = 4;
    int curOffset = 0;
    int numBlocks = l_pData->getBitLength() % blockSize ? l_pData->getBitLength() / blockSize + 1: l_pData->getBitLength() / blockSize;
    int dataBitLength = l_pData->getBitLength();
    ecmdDataBuffer rightData;

    /* defect 16358, for -oxrw8 data we need to right shift the whole buffer and then display */
    if (curState == ECMD_FORMAT_XRW) {
      if (l_pData->getBitLength() % 4) {
        rightData = *l_pData;
        rightData.shiftRightAndResize(4 - (l_pData->getBitLength() % 4));
        dataBitLength += 4 - (l_pData->getBitLength() % 4);
      } else {
        /* There are no odd bits, so to save us time just treat as left aligned */
        curState = ECMD_FORMAT_XW;
      }
    }

    if (numCols) {

      if (curState == ECMD_FORMAT_BN || curState == ECMD_FORMAT_BW || curState == ECMD_FORMAT_B || curState == ECMD_FORMAT_BXN || curState == ECMD_FORMAT_BXW) {
        printed += "\n";
        printed += ecmdBitsHeader(5, blockSize, numCols, dataBitLength, i_endianMode);
      } 

      printed += "\n000: ";
    }

    for (int i = 0; i < numBlocks; i++) {

      if (curState == ECMD_FORMAT_XW) {
        printed += l_pData->genHexLeftStr(curOffset, blockSize < (dataBitLength - curOffset) ? blockSize : (dataBitLength - curOffset));
      }
      else if (curState == ECMD_FORMAT_XRW) {
        printed += rightData.genHexRightStr(curOffset, blockSize < (dataBitLength - curOffset) ? blockSize : (dataBitLength - curOffset));
      }
#ifndef REMOVE_SIM
      else if ((curState == ECMD_FORMAT_BX) || (curState == ECMD_FORMAT_BXN) || (curState == ECMD_FORMAT_BXW))  {
        printed +=  l_pData->genXstateStr(curOffset, blockSize < (dataBitLength - curOffset) ? blockSize : (dataBitLength - curOffset));
      }
#endif
      else {
        printed += l_pData->genBinStr(curOffset, blockSize < (dataBitLength - curOffset) ? blockSize : (dataBitLength - curOffset));
      }

      colCount++;
      if (numCols && colCount == numCols && (i != numBlocks-1) ) {
        curCol += numCols;

        sprintf(outstr, "\n%3.3d: ", curCol);

        printed += outstr;
        colCount = 0;
      }
      else {
        printed += " ";
      }

      curOffset += blockSize;
    }

    delete [] outstr;
    outstr = NULL;
    printed += "\n";
  }

  // Tack this onto the end of anything returned
  //printed += "\n";
    
  return printed;

}

std::string ecmdBitsHeader(int initCharOffset, int blockSize, int numCols, int i_maxBitWidth, ecmdEndianMode_t i_endianMode) {

  std::string topLine;
  std::string bottomLine;

  int bitsToPrint = blockSize * numCols < i_maxBitWidth ? blockSize * numCols : i_maxBitWidth;
  int numSpaces = numCols - 1;
  char curNum[2];
  int blockCount = 0;
  int lineCount = 0;
  int topLineTrack = -1;

  for (int i = 0; i < bitsToPrint + numSpaces; i++) {

    if (blockCount == blockSize) {
      topLine += " ";
      bottomLine += " ";
      blockCount = 0;
    }
    else {

      int topLineCount = lineCount / 10;
      if (topLineCount != topLineTrack) {
        snprintf(curNum, 2, "%d", (uint32_t)(topLineCount % 100));
        topLine += curNum;
        topLineTrack = topLineCount;
      }
      else {
        topLine += " ";
      }

      snprintf(curNum, 2, "%d", (uint32_t)(lineCount % 10));
      bottomLine += curNum;

      lineCount++;
      blockCount++;

      /* Let's see if we are done with data */
      if (lineCount >= i_maxBitWidth) break;
    }

  }

  /* Now create the final output */
  std::string totalLine;

  if (i_endianMode == ECMD_LITTLE_ENDIAN) {
    /* In little endian we need to flip the header information to display properly */
    reverse(topLine.begin(), topLine.end());
    reverse(bottomLine.begin(), bottomLine.end());
  }

  /* Pad the front with spacing */
  topLine.insert(0, initCharOffset, ' ');
  bottomLine.insert(0, initCharOffset, ' ');
#if 0
  for (int x = 0; x < initCharOffset; x++) {
    topLine = " " + topLine;
    bottomLine = " " + bottomLine;
  }
#endif

  totalLine = topLine + "\n" + bottomLine;

  return totalLine;
}

uint32_t ecmdDisplayDllInfo() {

  uint32_t rc = ECMD_SUCCESS;
  std::string printed;

  /* Let's display the dllInfo to the user */
  ecmdDllInfo info;
  rc = ecmdQueryDllInfo(info);
  if (rc) {
    ecmdOutputError("ecmdDisplayDllInfo - Problems occurred trying to get Dll Info\n");
    return rc;
  }
  ecmdOutput("================================================\n");
  printed = "Dll Type         : " + info.dllType + "\n";
  ecmdOutput(printed.c_str());

  printed = "Dll Product      : " + info.dllProduct + "\n";
  ecmdOutput(printed.c_str());

  printed = "Dll Product Type : ";
  printed += info.dllProductType + "\n";
  ecmdOutput(printed.c_str());

  printed = "Dll Environment  : ";
  if (info.dllEnv == ECMD_DLL_ENV_HW)
    printed += "Hardware\n";
  else
    printed += "Simulation\n";
  ecmdOutput(printed.c_str());

  printed = "Dll Build Date   : "; printed += info.dllBuildDate; printed += "\n"; ecmdOutput(printed.c_str());
  printed = "Dll Capi Version : "; printed += info.dllCapiVersion; printed += "\n"; ecmdOutput(printed.c_str());
  printed = "Dll Build Info   : "; printed += info.dllBuildInfo; printed += "\n"; ecmdOutput(printed.c_str());
  ecmdOutput("================================================\n");


  return rc;
}

#if !(defined (ECMD_REMOVE_SEDC_SUPPORT) || defined (ECMD_REMOVE_SCOM_FUNCTIONS))
uint32_t ecmdDisplayScomData(const ecmdChipTarget & i_target, const ecmdScomData & i_scomData, const ecmdDataBuffer & i_data, const char* i_format, std::string *o_strData) {
  uint32_t rc = ECMD_SUCCESS;
  std::list<ecmdFileLocation> l_fileLocs;       ///< List of scomdefs - hashFile isn't used for scomdefs
  std::string scomdefFileStr;                   ///< Full Path to the Scomdef file
  sedcScomdefEntry scomEntry;                ///< Returns a class containing the scomdef entry read from the file
  unsigned int runtimeFlags=0;                    ///< Directives on how to parse
  bool verboseFlag = false;
  bool verboseBitsSetFlag = false;              ///< Print Bit description only if bit/s are set
  bool verboseBitsClearFlag = false;            ///< Print Bit description only if No bits are set
  std::string printed;                          ///< Output data
  std::vector<std::string> errMsgs;             ///< Any error messages to go with a array that was marked invalid
  std::string l_version = "default";

  if ((std::string)i_format == "-v") {
    verboseFlag = true;
  }
  if ((std::string)i_format == "-vs0") {
    verboseBitsClearFlag = true;
  }
  if ((std::string)i_format == "-vs1") {
    verboseBitsSetFlag = true;
  }
  rc = ecmdQueryFileLocation(i_target, ECMD_FILE_SCOMDATA, l_fileLocs, l_version);
  if (rc) {
    printed = "ecmdDisplayScomData - Error occured locating scomdef file \nSkipping -v parsing\n";
    ecmdOutputWarning(printed.c_str());
    return rc;
  }

  std::ifstream scomdefFile;
  for (std::list<ecmdFileLocation>::const_iterator l_fileLoc = l_fileLocs.begin(); l_fileLoc != l_fileLocs.end(); l_fileLoc++)
  {
      rc = ECMD_SUCCESS;
      scomdefFile.open(l_fileLoc->textFile.c_str());
      if(scomdefFile.fail()) {
          printed = "ecmdDisplayScomData - Error occured opening scomdef file: " + l_fileLoc->textFile + "\nSkipping -v parsing\n";
          ecmdOutputWarning(printed.c_str());
          rc = ECMD_UNABLE_TO_OPEN_SCOMDEF;
          return rc;
      }
      rc = readScomDefFile(i_scomData.address, scomdefFile);
      if ((rc == ECMD_SCOMADDRESS_NOT_FOUND) && (i_target.chipUnitType != "") && (i_target.chipUnitTypeState ==  ECMD_TARGET_FIELD_VALID)) {
          uint64_t modifiedScomAddress = 0;
          rc = ecmdCreateChipUnitScomAddress(i_target, i_scomData.address, modifiedScomAddress);
          if (rc) {
              ecmdOutputWarning("Unable to convert the address into right chip unit address\n");
              return rc;
          }
          scomdefFile.clear();
          scomdefFile.seekg(0, std::ios::beg);
          rc = readScomDefFile(modifiedScomAddress, scomdefFile);
      }
      if (rc == ECMD_SCOMADDRESS_NOT_FOUND) {
          scomdefFile.close();
      }
      // Found the address, don't need to look in any other scomdefs
      else
      {
          break;
      }
  }
  // One more check outside of the loop to make sure we found something
  if (rc == ECMD_SCOMADDRESS_NOT_FOUND) {
      ecmdOutputWarning("Unable to find Scom Address in the Scomdef file\n");
      ecmdOutputWarning("ecmdDisplayScomData - Scom Address not found. Skipping -v parsing\n");
      return rc;
  }

  sedcScomdefParser(scomEntry, scomdefFile, errMsgs, runtimeFlags);

  std::list< std::string >::iterator descIt;
  std::list<sedcScomdefDefLine>::iterator definIt;
  std::list< std::string >::iterator bitDetIt;
  char bitDesc[400], bitDesc2[500];

  sprintf(bitDesc,"Name       : %20s%s\nDesc       : %20s", " ",scomEntry.name.c_str()," ");  
  ecmdOutput(bitDesc);
  if (o_strData != NULL) {
    *o_strData += bitDesc;
  }

  for (descIt = scomEntry.description.begin(); descIt != scomEntry.description.end(); descIt++) {
    ecmdOutput(descIt->c_str());
    if (o_strData != NULL) {
      *o_strData += *descIt;
    }
  }

  printed = "\n";
  ecmdOutput(printed.c_str());
  if (o_strData != NULL) {
    *o_strData += printed;
  }

  // We need these variable below to abstract some stuff for LE support
  uint32_t beStart, length;

  //Print Bits description
  for (definIt = scomEntry.definition.begin(); definIt != scomEntry.definition.end(); definIt++) {
    // Set our pivots for LE converstion in the BE buffer
    if (i_scomData.endianMode == ECMD_LITTLE_ENDIAN) {
      // minus 1 because you don't want the length, you want the last bit position and then flip
      beStart = (i_scomData.length - 1) - definIt->lhsNum;
      /* If we have rhs, the length is negative so flip it */
      if (definIt->rhsNum != -1) {
        length = definIt->length * -1;
        // We also have to add two to the length because it's messed up by the EDC parser which doesn't do little endian
        // example
        // 31-4 = 27 + 1 = 28, to get the real length.  EDC does this on little endian data
        // 4-31 = -27 + 1 = -26
        // The mult * -1 above gave us 26, but we are still missing 2.
        length += 2;
      } else {
        length = definIt->length;
      }
    } else { // Big Endian
      beStart = definIt->lhsNum;
      length = definIt->length;
    }
    if (verboseFlag || (verboseBitsSetFlag && i_data.getNumBitsSet(beStart, length)) ||
        (verboseBitsClearFlag && (!i_data.getNumBitsSet(beStart, length)))) {

      if(definIt->rhsNum == -1) {
        sprintf(bitDesc, "Bit(%d)", definIt->lhsNum);
      }
      else {
        sprintf(bitDesc, "Bit(%d:%d)", definIt->lhsNum,definIt->rhsNum);
      }
      sprintf(bitDesc2, "%-10s : ",bitDesc);
      ecmdOutput(bitDesc2);
      if (o_strData != NULL) {
        *o_strData += bitDesc2;
      }

      if (length <= 8) {
        std::string binstr = i_data.genBinStr(beStart, length);
        sprintf(bitDesc, "0b%-16s  %s\n",binstr.c_str(),definIt->dialName.c_str());
      }
      else {
        std::string hexLeftStr = i_data.genHexLeftStr(beStart, length);
        sprintf(bitDesc, "0x%-16s  %s\n",hexLeftStr.c_str(),definIt->dialName.c_str());
      }
      ecmdOutput(bitDesc);
      if (o_strData != NULL) {
        *o_strData += bitDesc;
      }
      std::string bitDescStr;
      for (bitDetIt = definIt->detail.begin(); bitDetIt != definIt->detail.end(); bitDetIt++) {
        sprintf(bitDesc, "%32s ", " ");
        //Would print the entire string no matter how long it is
        bitDescStr = (std::string)bitDesc + *bitDetIt;
        ecmdOutput(bitDescStr.c_str());
        if (o_strData != NULL) {
          *o_strData += bitDescStr;
        }
        bitDescStr = "\n";// Doing the newline separately cos there maybe control characters at the end of Desc
        ecmdOutput(bitDescStr.c_str());
        if (o_strData != NULL) {
          *o_strData += bitDescStr;
        }       

      }//end for
    }//end if 
  }// end for
  return rc;
}

uint32_t readScomDefFile(uint64_t address, std::ifstream &scomdefFile) {
  uint32_t rc = ECMD_SUCCESS;
  std::string scomdefFileStr;                      ///< Full path to scomdef file
  std::string printed;
  
  
  std::string curLine;
  uint32_t beginPtr=0;        //fix beam error
  uint32_t beginLen=0;        //fix beam error
  uint64_t addrFromFile;

  bool done = false; 
  std::vector<std::string> curArgs(4);
  
  while (getline(scomdefFile, curLine) && !done) {
    //Remove leading whitespace
    size_t curStart = curLine.find_first_not_of(" \t", 0);
    if (curStart != std::string::npos) {
      curLine = curLine.substr(curStart,curLine.length());
    }
    if((curLine[0] == 'B') && (curLine.find("BEGIN Scom") != std::string::npos)) {
      beginPtr = scomdefFile.tellg();
      beginLen = curLine.length();
    }
    if((curLine[0] == 'A') && (curLine.substr(0, 10) == "Address = ")) {
      ecmdParseTokens(curLine, " \t\n={},", curArgs);
      //Don't just check for the first address for a match.  This could be a chipunit address that has multiple addresses on this line
      for (std::vector<std::string>::iterator it = curArgs.begin(); it != curArgs.end(); ++it)
      {
	//sscanf(curArgs[1].c_str(),"%llX",&addrFromFile);
	sscanf(it->c_str(),UINT64_HEX_FORMAT,&addrFromFile);
	if ((curArgs.size() >= 2) && addrFromFile == address) {
	  done = true;
	  break;
	}
      }     
    }
  }
  if (done) {
    scomdefFile.seekg(beginPtr-beginLen-1);
  }
  else {
    rc = ECMD_SCOMADDRESS_NOT_FOUND;
  }
  return rc;
}
#endif

#ifndef ECMD_STRIP_DEBUG
/**********************************************************************************/
/**********************************************************************************/
/*                                                                                */
/* efppInOut = an enum that tells me if we are working in a before or after mode  */
/*             ECMD_FPP_FUNCTIONIN is intended to be the first thing done at the  */
/*             top of a function.                                                 */
/*             ECMD_FPP_FUNCTIONOUT is the last thing done.                       */
/*                                                                                */
/* fprototypeStr = a cut and paste string of the actual function prototype        */
/*                                                                                */
/* other parameters must be handled on a case by case basis based on the parsed   */
/*                  value within fprototypeStr.                                   */
/*                                                                                */
/*                                                                                */
/**********************************************************************************/
/* if you can think of a better way to detect and handle parameter types let me   */
/* know.  I think this method allows us to handle structures in a convinient way. */
/*                                                                                */
/* FLEXIBILITY and EXPANDABILITY                                                  */
/*                                                                                */
/**********************************************************************************/
/**********************************************************************************/
void ecmdFunctionParmPrinter(int tCount, efppInOut_t inOut, const char * fprototypeStr, std::vector < void * > args) {
  /* declare variables */

  /* If it's less than 8, nothing to do */
  if (ecmdClientDebug < 8) return;

  int looper, looper2;
  std::vector<std::string> tokens;
  std::vector<std::string> parmTokens;
  std::vector<std::string> parmEntryTokens;
  std::vector<std::string> fReturnType;

  char variableType[100]; /* this will be something like "char*" or "BIT32" */
  std::vector<std::string> variableName; /* this will be something like "fprototypeStr" no * or & in this */
  std::string printed;

  int mysize;
  int look4rc,outputRC,dataLooper;

  char tempIntStr[400];


  look4rc = outputRC = 0;
  /* validate the type of call we are doing, return if invalid */
  if(inOut == ECMD_FPP_FUNCTIONIN) {
    look4rc =0;
    sprintf(frontFPPTxt,"ECMD DEBUG (ecmdFPP) : ENTER(%03d) : ",tCount);
  } else if (inOut == ECMD_FPP_FUNCTIONOUT) {
    look4rc =1;
    sprintf(frontFPPTxt,"ECMD DEBUG (ecmdFPP) : EXIT (%03d) : ",tCount);
  } else {
    printed = "ECMD DEBUG (ecmdFPP) : ERROR::ecmdFunctionParmPrinter  Invalid Enum type on function call.\n";
    debugFunctionOuput(printed.c_str());
    return;
  }


  /* print the original function prototype */
  printed = frontFPPTxt;
  printed += "Function prototype: ";
  printed += fprototypeStr;
  printed += "\n";
  debugFunctionOuput(printed.c_str());




  /* parse the parameters */
  ecmdParseTokens(fprototypeStr, "()", tokens); /* this chops off the leading junk */
  /* example: */
  /* tokens[0] = "void ecmdFunctionParmPrinter"             */
  /* tokens[1] = "enum efppInOut, char *fprototypeStr, ..." */
  /* tokens[2] = " {"                                       */
  /* tokens.size() = 3                                      */

  ecmdParseTokens(tokens[0].c_str(), " ", fReturnType); /* this tokenizes the function name and return type */
  /* example: */
  /* fReturnType[0] = "void"             */
  /* fReturnType[1] = "ecmdFunctionParmPrinter" */
  /* fReturnType.size() = 2                                      */

  if(look4rc) {
    if((!strcmp(fReturnType[0].c_str(),"void")) && (look4rc)) {
      /* it's a void return so don't do return code printing. */
      outputRC =0;
    } else {
      outputRC =1;
    }
  }  

  if(tokens.size() >1) {
    ecmdParseTokens(tokens[1].c_str(), ",", parmTokens); /* this tokenizes the meat and potatoes */
  }
  /* example: */
  /* parmTokens[0] = "enum efppInOut"       */
  /* parmTokens[1] = " char *fprototypeStr" */
  /* parmTokens[2] = " ..."                 */
  /* parmTokens.size90 = 3                    */

  /* exit if we are on a functionin and debug level 8 */
  if((ecmdClientDebug == 8) && (inOut == ECMD_FPP_FUNCTIONIN)) {
    return;
  }

  /* exit if we are on a functionOut, debug level 8 and no return code */
  if((ecmdClientDebug == 8) && (inOut == ECMD_FPP_FUNCTIONOUT) && (outputRC ==0)) {
    return;
  }


  /* exit if we are on a functionOut, debug level 8 and no return code */
  if((ecmdClientDebug == 8) && (inOut == ECMD_FPP_FUNCTIONOUT) && (outputRC ==1)) {

    /* go check return code to see if it's zero or not. */
    if((!strcmp(fReturnType[0].c_str(),"uint32_t")) && (look4rc)) {
      uint32_t* dummy = (uint32_t*)(args[parmTokens.size()]);
      if (*dummy ==0) {

        /* print the splat line to show it's the end of the exit */
        printed = frontFPPTxt;
        printed += "\t ***************************************\n";
        debugFunctionOuput(printed.c_str());

        return;  /* normal return code so just exit */
      }
    } else {
      /* if return type is something other than uint32_t then we should probably return since it would not */
      /* match the intended meening of the debug8 exit plan */
      /* print the splat line to show it's the end of the exit */
      printed = frontFPPTxt;
      printed += "\t ***************************************\n";
      debugFunctionOuput(printed.c_str());

      return;
    }
  }



  /* remember the leading and trailing spaces, they could mess up a compare */

  /* we need to handle each one on it's own, and make sure the types match the string provided */

  printed = frontFPPTxt;
  printed += "\t ***************************************\n";
  debugFunctionOuput(printed.c_str());

  mysize = parmTokens.size();

  for(looper =0; (uint32_t) looper < parmTokens.size() + outputRC; looper++) {

    if(((uint32_t)looper == parmTokens.size()) && (outputRC ==1)) {
      /* we are on the last parameter we need to say the parm is a return code. */

      strcpy(variableType,fReturnType[0].c_str());
      variableName.clear();
      variableName.push_back("RETURN CODE");

    } else {

      ecmdParseTokens(parmTokens[looper].c_str(), " ", parmEntryTokens);
      /* example: */
      /* parmEntryTokens[0] = "enum"      */
      /* parmEntryTokens[1] = "efppInOut" */
      /* parmEntryTokens.size() = 2       */

      /* rules of engagement: */
      /* must have at least 2 entries, */
      /* last entry is variable name */
      /* last entry could have leading * or & character so strip it and cat it on to the previous string for consistancy */

      mysize = parmEntryTokens.size();

      if(mysize < 2) {
        printed = frontFPPTxt;
        printed += "ERROR::ecmdFunctionParmPrinter  We have an invalid parameter for parm entry # ";
        sprintf(tempIntStr,"%d",looper);
        printed += tempIntStr;
        printed += "\n";
        debugFunctionOuput(printed.c_str());

        continue;
      }

      strcpy(variableType,"");

      for(looper2=0; looper2 < (mysize -1); looper2++) {
        /* used -1 because we don't want the variable name yet */
        if(looper2 !=0) strcat(variableType," ");
        strcat(variableType,parmEntryTokens[looper2].c_str());
      }

      if(parmEntryTokens[mysize-1].c_str()[0] == '*') {
        strcat(variableType,"*");
        /* handle the "char **name" oddities too */
        if(parmEntryTokens[mysize-1].c_str()[1] == '*') strcat(variableType,"*");
      } else if(parmEntryTokens[mysize-1].c_str()[0] == '&') {
        strcat(variableType,"&");
      }

      ecmdParseTokens(parmEntryTokens[mysize-1].c_str(), "*&", variableName);
    }


    /****************************************/
    /* at this point variableType is somehting like : "enum" or "char*" or "const char*" */
    /* variableName[0] is the variable name */

    /* we need to strcmp on variableType and then declare an object of that type. it will go away */
    /* when it falls out of scope */

    if((strlen(variableType) == 0 )){
      printed = frontFPPTxt;
      printed += "ERROR::ecmdFunctionParmPrinter  variableType parsing messed up big time for parm number ";
      sprintf(tempIntStr,"%d",looper);
      printed += tempIntStr;
      printed += "\n";
      debugFunctionOuput(printed.c_str());
      continue;
    }


    if(!strcmp(variableType,"char")) {
      /* char */
      char * dummy = (char *)(args[looper]);

      printed = frontFPPTxt;
      printed += "\t type : ";
      printed += variableType;
      printed += " : variable name : ";
      printed += variableName[0];
      printed += " = ";
      sprintf(tempIntStr,"%c",*dummy);
      printed += tempIntStr;
      printed += "\n";
      debugFunctionOuput(printed.c_str());

    } else if((!strcmp(variableType,"const char *")) ||
              (!strcmp(variableType,"char *")) ||
              (!strcmp(variableType,"const char*"))   ){
      /* const char * */

      char ** dummy = (char **)(args[looper]);

      printed = frontFPPTxt;
      printed += "\t type : ";
      printed += variableType;
      printed += " : variable name : ";
      printed += variableName[0];
      printed += "\n";
      debugFunctionOuput(printed.c_str());

      printed = frontFPPTxt;
      printed += "\t \t value : ";
      if (*dummy == NULL) printed += "NULL";
      else printed += *dummy;
      printed += "\n";
      printed += frontFPPTxt;
      printed += "\t ***************************************\n";
      debugFunctionOuput(printed.c_str());

    } else if((!strcmp(variableType,"std::string"))       ||
              (!strcmp(variableType,"const std::string")) ||
              (!strcmp(variableType,"const std::string &")) ||
              (!strcmp(variableType,"std::string&"))      ||
              (!strcmp(variableType,"std::string &"))       ){
      /* std::string */

      std::string* dummy = (std::string*)(args[looper]);
      printed = frontFPPTxt;
      printed += "\t type : ";
      printed += variableType;
      printed += " : variable name : ";
      printed += variableName[0];
      printed += " = ";
      printed += dummy->c_str();
      printed += "\n";
      printed += frontFPPTxt;
      printed += "\t ***************************************\n";
      debugFunctionOuput(printed.c_str());

    } else if((!strcmp(variableType,"std::list< std::string >")) ||
              (!strcmp(variableType,"std::list<std::string>"))     ) {
      /* std::list<std::string> */

      std::list<std::string>* dummy = (std::list<std::string>*)(args[looper]);

      sprintf(tempIntStr, "%s\t type : %s : variable name : %s\n",frontFPPTxt, variableType, variableName[0].c_str()); debugFunctionOuput(tempIntStr);
      dataLooper = 0;
      for (std::list<std::string>::iterator entit = dummy->begin(); entit != dummy->end(); entit ++) {
        sprintf(tempIntStr,"%s\t \t entry : %d\n",frontFPPTxt, dataLooper++); debugFunctionOuput(tempIntStr);
        sprintf(tempIntStr,"%s\t \t \t value : std::string  = %s\n",frontFPPTxt, entit->c_str()); debugFunctionOuput(tempIntStr);
      }
      printed = "\n";
      printed += frontFPPTxt;
      printed += "\t ***************************************\n";
      debugFunctionOuput(printed.c_str());
    } else if((!strcmp(variableType,"std::list<ecmdArrayEntry> &")) ||
              (!strcmp(variableType,"std::list<ecmdNameEntry> &"))  ||
              (!strcmp(variableType,"std::list<ecmdIndexEntry> &"))  ||
              (!strcmp(variableType,"std::list <ecmdNameVectorEntry> &"))   ){
      /* std::list<something> & */
      printed = frontFPPTxt;
      printed += "\t type : ";
      printed += variableType;
      printed += " : variable name : ";
      printed += variableName[0];
      printed += " = ";
      printed += "placeholder for std::list dump\n";
      printed += frontFPPTxt;
      printed += "\t ***************************************\n";
      debugFunctionOuput(printed.c_str());



    } else if (!strcmp(variableType,"std::list<ecmdLatchEntry> &")) {
      std::list<ecmdLatchEntry>* dummy = (std::list<ecmdLatchEntry>*)(args[looper]);

      sprintf(tempIntStr, "%s\t type : %s : variable name : %s\n",frontFPPTxt, variableType, variableName[0].c_str()); debugFunctionOuput(tempIntStr);
      dataLooper = 0;
      for (std::list<ecmdLatchEntry>::iterator entit = dummy->begin(); entit != dummy->end(); entit ++) {
        sprintf(tempIntStr,"%s\t \t entry : %d\n",frontFPPTxt, dataLooper++); debugFunctionOuput(tempIntStr);
        sprintf(tempIntStr,"%s\t \t \t value : std::string    latchName  = %s\n",frontFPPTxt, entit->latchName.c_str()); debugFunctionOuput(tempIntStr);
        sprintf(tempIntStr,"%s\t \t \t value : std::string    ringName   = %s\n",frontFPPTxt, entit->ringName.c_str()); debugFunctionOuput(tempIntStr);
        printEcmdDataBuffer("ecmdDataBuffer", "buffer", entit->buffer, "\t\t ");
        sprintf(tempIntStr,"%s\t \t \t value : int        latchStartBit  = %d\n",frontFPPTxt, entit->latchStartBit); debugFunctionOuput(tempIntStr);
        sprintf(tempIntStr,"%s\t \t \t value : int          latchEndBit  = %d\n",frontFPPTxt, entit->latchEndBit); debugFunctionOuput(tempIntStr);
        sprintf(tempIntStr,"%s\t \t \t value : uint32_t              rc  = %X\n",frontFPPTxt, entit->rc); debugFunctionOuput(tempIntStr);
      }

      printed = "\n";
      printed += frontFPPTxt;
      printed += "\t ***************************************\n";
      debugFunctionOuput(printed.c_str());

    } else if (!strcmp(variableType,"std::list<ecmdLatchData> &")) {
      std::list<ecmdLatchData>* dummy = (std::list<ecmdLatchData>*)(args[looper]);

      sprintf(tempIntStr, "%s\t type : %s : variable name : %s\n",frontFPPTxt, variableType, variableName[0].c_str()); debugFunctionOuput(tempIntStr);
      dataLooper = 0;
      for (std::list<ecmdLatchData>::iterator entit = dummy->begin(); entit != dummy->end(); entit ++) {
        sprintf(tempIntStr,"%s\t \t entry : %d\n",frontFPPTxt, dataLooper++); debugFunctionOuput(tempIntStr);

        sprintf(tempIntStr,"%s\t \t \t value : std::string       latchName  = %s\n",frontFPPTxt, entit->latchName.c_str()); debugFunctionOuput(tempIntStr);

        sprintf(tempIntStr,"%s\t \t \t value : std::string        ringName  = %s\n",frontFPPTxt, entit->ringName.c_str()); debugFunctionOuput(tempIntStr);

        sprintf(tempIntStr,"%s\t \t \t value : int               bitLength  = %d\n",frontFPPTxt, entit->bitLength); debugFunctionOuput(tempIntStr);

        sprintf(tempIntStr,"%s\t \t \t value : BOOL      isChipUnitRelated  = ",frontFPPTxt);
        if(entit->isChipUnitRelated) {
          strcat(tempIntStr,"TRUE\n");
        } else {
          strcat(tempIntStr,"FALSE\n");
        }
        debugFunctionOuput(tempIntStr);

        sprintf(tempIntStr,"%s\t \t \t value : std::string relatedChipUnit  = %s\n",frontFPPTxt, entit->relatedChipUnit.c_str()); debugFunctionOuput(tempIntStr);

        sprintf(tempIntStr,"%s\t \t \t value : std::string     clockDomain  = %s\n",frontFPPTxt, entit->clockDomain.c_str()); debugFunctionOuput(tempIntStr);

        sprintf(tempIntStr,"%s\t \t \t value : ecmdClockState_t clockState  = ",frontFPPTxt); 
        if(entit->clockState == ECMD_CLOCKSTATE_UNKNOWN) {
          strcat(tempIntStr,"ECMD_CLOCKSTATE_UNKNOWN\n");
        } else if(entit->clockState == ECMD_CLOCKSTATE_ON) {
          strcat(tempIntStr,"ECMD_CLOCKSTATE_ON\n");
        } else if(entit->clockState == ECMD_CLOCKSTATE_OFF) {
          strcat(tempIntStr,"ECMD_CLOCKSTATE_OFF\n");
        } else if(entit->clockState == ECMD_CLOCKSTATE_NA) {
          strcat(tempIntStr,"ECMD_CLOCKSTATE_NA\n");
        } else {
          strcat(tempIntStr,"ERROR Unkown ecmdClockState_t value\n");
        }
        debugFunctionOuput(tempIntStr);

      }

      printed = "\n";
      printed += frontFPPTxt;
      printed += "\t ***************************************\n";
      debugFunctionOuput(printed.c_str());

    } else if (!strcmp(variableType,"std::list<ecmdSpyData> &")) {
      std::list<ecmdSpyData>* dummy = (std::list<ecmdSpyData>*)(args[looper]);

      sprintf(tempIntStr, "%s\t type : %s : variable name : %s\n",frontFPPTxt, variableType, variableName[0].c_str()); debugFunctionOuput(tempIntStr);
      dataLooper = 0;
      for (std::list<ecmdSpyData>::iterator entit = dummy->begin(); entit != dummy->end(); entit ++) {
        sprintf(tempIntStr,"%s\t \t entry : %d\n",frontFPPTxt, dataLooper++); debugFunctionOuput(tempIntStr);
        sprintf(tempIntStr,"%s\t \t \t value : std::string     spyName  = %s\n",frontFPPTxt, entit->spyName.c_str()); debugFunctionOuput(tempIntStr);
        sprintf(tempIntStr,"%s\t \t \t value : int           bitLength  = %d\n",frontFPPTxt, entit->bitLength); debugFunctionOuput(tempIntStr);

        sprintf(tempIntStr,"%s\t \t \t value : ecmdSpyType_t   spyType  = ",frontFPPTxt); 
        if(entit->spyType == ECMD_SPYTYPE_ALIAS) {
          strcat(tempIntStr,"ECMD_SPYTYPE_ALIAS\n");
        } else if(entit->spyType == ECMD_SPYTYPE_IDIAL) {
          strcat(tempIntStr,"ECMD_SPYTYPE_IDIAL\n");
        } else if(entit->spyType == ECMD_SPYTYPE_EDIAL) {
          strcat(tempIntStr,"ECMD_SPYTYPE_EDIAL\n");
        } else if(entit->spyType == ECMD_SPYTYPE_ECCGROUP) {
          strcat(tempIntStr,"ECMD_SPYTYPE_ECCGROUP\n");
        } else {
          strcat(tempIntStr,"ERROR Unkown ecmdSpyType_t value\n");
        }
        debugFunctionOuput(tempIntStr);

        sprintf(tempIntStr,"%s\t \t \t value : BOOL       isEccChecked  = ",frontFPPTxt);
        if(entit->isEccChecked) {
          strcat(tempIntStr,"TRUE\n");
        } else {
          strcat(tempIntStr,"FALSE\n");
        }
        debugFunctionOuput(tempIntStr);

        sprintf(tempIntStr,"%s\t \t \t value : BOOL       isEnumerated  = ",frontFPPTxt);
        if(entit->isEnumerated) {
          strcat(tempIntStr,"TRUE\n");
        } else {
          strcat(tempIntStr,"FALSE\n");
        }
        debugFunctionOuput(tempIntStr);

        sprintf(tempIntStr,"%s\t \t \t value : BOOL      isChipUnitRelated  = ",frontFPPTxt);
        if(entit->isChipUnitRelated) {
          strcat(tempIntStr,"TRUE\n");
        } else {
          strcat(tempIntStr,"FALSE\n");
        }
        debugFunctionOuput(tempIntStr);

        sprintf(tempIntStr,"%s\t \t \t value : std::string relatedChipUnit  = %s\n",frontFPPTxt, entit->relatedChipUnit.c_str()); debugFunctionOuput(tempIntStr);

        sprintf(tempIntStr,"%s\t \t \t value : std::string clockDomain  = %s\n",frontFPPTxt, entit->clockDomain.c_str()); debugFunctionOuput(tempIntStr);

        sprintf(tempIntStr,"%s\t \t \t value : ecmdClockState_t clockState  = ",frontFPPTxt); 
        if(entit->clockState == ECMD_CLOCKSTATE_UNKNOWN) {
          strcat(tempIntStr,"ECMD_CLOCKSTATE_UNKNOWN\n");
        } else if(entit->clockState == ECMD_CLOCKSTATE_ON) {
          strcat(tempIntStr,"ECMD_CLOCKSTATE_ON\n");
        } else if(entit->clockState == ECMD_CLOCKSTATE_OFF) {
          strcat(tempIntStr,"ECMD_CLOCKSTATE_OFF\n");
        } else if(entit->clockState == ECMD_CLOCKSTATE_NA) {
          strcat(tempIntStr,"ECMD_CLOCKSTATE_NA\n");
        } else {
          strcat(tempIntStr,"ERROR Unkown ecmdClockState_t value\n");
        }
        debugFunctionOuput(tempIntStr);

        std::list<std::string >::iterator enumIt;
        sprintf(tempIntStr,"%s\t \t \t value : std::list<std::string> enums = \n",frontFPPTxt); debugFunctionOuput(tempIntStr);
        int enumLooper=0;
        for (enumIt = entit->enums.begin(); enumIt != entit->enums.end(); enumIt++) {
          sprintf(tempIntStr,"%s\t \t \t \t entry : %d\t = %s\n",frontFPPTxt, enumLooper++, enumIt->c_str());
          debugFunctionOuput(tempIntStr);
        }

        std::list<std::string >::iterator epCheckersIt;
        sprintf(tempIntStr,"%s\t \t \t value : std::list<std::string> epCheckers = \n",frontFPPTxt);
        debugFunctionOuput(tempIntStr);
        int epcLooper=0;
        for (epCheckersIt = entit->epCheckers.begin(); epCheckersIt != entit->epCheckers.end(); epCheckersIt++) {
          sprintf(tempIntStr,"%s\t \t \t \t entry : %d\t = %s\n",frontFPPTxt, epcLooper++, epCheckersIt->c_str());
          debugFunctionOuput(tempIntStr);
        }

      }

      printed = "\n";
      printed += frontFPPTxt;
      printed += "\t ***************************************\n";
      debugFunctionOuput(printed.c_str());

    } else if (!strcmp(variableType,"std::list<ecmdScomData> &")) {
      std::list<ecmdScomData>* dummy = (std::list<ecmdScomData>*)(args[looper]);

      sprintf(tempIntStr, "%s\t type : %s : variable name : %s\n",frontFPPTxt, variableType, variableName[0].c_str()); debugFunctionOuput(tempIntStr);
      dataLooper = 0;
      for (std::list<ecmdScomData>::iterator entit = dummy->begin(); entit != dummy->end(); entit ++) {
        sprintf(tempIntStr,"%s\t \t entry : %d\n",frontFPPTxt, dataLooper++); debugFunctionOuput(tempIntStr);
        sprintf(tempIntStr,"%s\t \t \t value : uint64_t           address  = 0x" UINT64_HEX_FORMAT "\n",frontFPPTxt, entit->address);
        debugFunctionOuput(tempIntStr);

        sprintf(tempIntStr,"%s\t \t \t value : BOOL      isChipUnitRelated  = ",frontFPPTxt);
        if(entit->isChipUnitRelated) {
          strcat(tempIntStr,"TRUE\n");
        } else {
          strcat(tempIntStr,"FALSE\n");
        }
        debugFunctionOuput(tempIntStr);

        char *pstr = tempIntStr + sprintf(tempIntStr, "%s\t \t \t value : std::string relatedChipUnit  =", frontFPPTxt);
        std::list<std::string>::iterator relatedChipUnitIter;
        for (relatedChipUnitIter = entit->relatedChipUnit.begin(); relatedChipUnitIter != entit->relatedChipUnit.end(); relatedChipUnitIter++) {
          pstr += sprintf(pstr, "%s %s", tempIntStr, relatedChipUnitIter->c_str());
        }
        pstr += sprintf(pstr, "%s\n", tempIntStr);
        debugFunctionOuput(tempIntStr);

        sprintf(tempIntStr,"%s\t \t \t value : std::string    clockDomain  = %s\n",frontFPPTxt, entit->clockDomain.c_str());
        debugFunctionOuput(tempIntStr);

        sprintf(tempIntStr,"%s\t \t \t value : ecmdClockState_t clockState = ",frontFPPTxt); 
        if(entit->clockState == ECMD_CLOCKSTATE_UNKNOWN) {
          strcat(tempIntStr,"ECMD_CLOCKSTATE_UNKNOWN\n");
        } else if(entit->clockState == ECMD_CLOCKSTATE_ON) {
          strcat(tempIntStr,"ECMD_CLOCKSTATE_ON\n");
        } else if(entit->clockState == ECMD_CLOCKSTATE_OFF) {
          strcat(tempIntStr,"ECMD_CLOCKSTATE_OFF\n");
        } else if(entit->clockState == ECMD_CLOCKSTATE_NA) {
          strcat(tempIntStr,"ECMD_CLOCKSTATE_NA\n");
        } else {
          strcat(tempIntStr,"ERROR Unkown ecmdClockState_t value\n");
        }
        debugFunctionOuput(tempIntStr);
      }

      printed = "\n";
      printed += frontFPPTxt;
      printed += "\t ***************************************\n";
      debugFunctionOuput(printed.c_str());

    } else if (!strcmp(variableType,"std::list<ecmdRingData> &")) {
      std::list<ecmdRingData>* dummy = (std::list<ecmdRingData>*)(args[looper]);

      sprintf(tempIntStr, "%s\t type : %s : variable name : %s\n",frontFPPTxt, variableType, variableName[0].c_str()); debugFunctionOuput(tempIntStr);
      dataLooper = 0;
      for (std::list<ecmdRingData>::iterator entit = dummy->begin(); entit != dummy->end(); entit ++) {
        sprintf(tempIntStr,"%s\t \t entry : %d\n",frontFPPTxt, dataLooper++); debugFunctionOuput(tempIntStr);

        std::list<std::string >::iterator ringNamesIt;
        sprintf(tempIntStr,"%s\t \t \t value : std::list<std::string> ringNames = \n",frontFPPTxt); debugFunctionOuput(tempIntStr);
        int ringNamesLooper=0;
        for (ringNamesIt = entit->ringNames.begin(); ringNamesIt != entit->ringNames.end(); ringNamesIt++) {
          sprintf(tempIntStr,"%s\t \t \t \t entry : %d\t = %s\n",frontFPPTxt, ringNamesLooper++, ringNamesIt->c_str());
          debugFunctionOuput(tempIntStr);
        }

        sprintf(tempIntStr,"%s\t \t \t value : uint32_t            address  = 0x%X\n",frontFPPTxt, entit->address);
        debugFunctionOuput(tempIntStr);

        sprintf(tempIntStr,"%s\t \t \t value : int               bitLength  = %d\n",frontFPPTxt, entit->bitLength); debugFunctionOuput(tempIntStr);

        sprintf(tempIntStr,"%s\t \t \t value : BOOL       hasInversionMask  = ",frontFPPTxt);
        if(entit->hasInversionMask) {
          strcat(tempIntStr,"TRUE\n");
        } else {
          strcat(tempIntStr,"FALSE\n");
        }
        debugFunctionOuput(tempIntStr);

        sprintf(tempIntStr,"%s\t \t \t value : BOOL  supportsBroadsideLoad  = ",frontFPPTxt);
        if(entit->supportsBroadsideLoad) {
          strcat(tempIntStr,"TRUE\n");
        } else {
          strcat(tempIntStr,"FALSE\n");
        }
        debugFunctionOuput(tempIntStr);


        sprintf(tempIntStr,"%s\t \t \t value : BOOL            isCheckable  = ",frontFPPTxt);
        if(entit->isCheckable) {
          strcat(tempIntStr,"TRUE\n");
        } else {
          strcat(tempIntStr,"FALSE\n");
        }
        debugFunctionOuput(tempIntStr);

        sprintf(tempIntStr,"%s\t \t \t value : BOOL      isChipUnitRelated  = ",frontFPPTxt);
        if(entit->isChipUnitRelated) {
          strcat(tempIntStr,"TRUE\n");
        } else {
          strcat(tempIntStr,"FALSE\n");
        }
        debugFunctionOuput(tempIntStr);

        sprintf(tempIntStr,"%s\t \t \t value : std::string relatedChipUnit  = %s\n",frontFPPTxt, entit->relatedChipUnit.c_str()); debugFunctionOuput(tempIntStr);

        sprintf(tempIntStr,"%s\t \t \t value : std::string     clockDomain  = %s\n",frontFPPTxt, entit->clockDomain.c_str()); debugFunctionOuput(tempIntStr);

        sprintf(tempIntStr,"%s\t \t \t value : ecmdClockState_t clockState  = ",frontFPPTxt); 
        if(entit->clockState == ECMD_CLOCKSTATE_UNKNOWN) {
          strcat(tempIntStr,"ECMD_CLOCKSTATE_UNKNOWN\n");
        } else if(entit->clockState == ECMD_CLOCKSTATE_ON) {
          strcat(tempIntStr,"ECMD_CLOCKSTATE_ON\n");
        } else if(entit->clockState == ECMD_CLOCKSTATE_OFF) {
          strcat(tempIntStr,"ECMD_CLOCKSTATE_OFF\n");
        } else if(entit->clockState == ECMD_CLOCKSTATE_NA) {
          strcat(tempIntStr,"ECMD_CLOCKSTATE_NA\n");
        } else {
          strcat(tempIntStr,"ERROR Unkown ecmdClockState_t value\n");
        }
        debugFunctionOuput(tempIntStr);

      }

      printed = "\n";
      printed += frontFPPTxt;
      printed += "\t ***************************************\n";
      debugFunctionOuput(printed.c_str());




    } else if(!strcmp(variableType,"std::vector <ecmdDataBuffer> &")) {
      /* std::vector <ecmdDataBuffer> & */
      printed = frontFPPTxt;
      printed += "\t type : ";
      printed += variableType;
      printed += " : variable name : ";
      printed += variableName[0];
      printed += " = ";
      printed += "placeholder for std::vector dump\n";
      printed += frontFPPTxt;
      printed += "\t ***************************************\n";
      debugFunctionOuput(printed.c_str());

    } else if(!strcmp(variableType,"uint32_t") ||
              !strcmp(variableType,"uint32_t &") ||
              !strcmp(variableType,"uint32_t&") ) {
      /* uint32_t */

      uint32_t* dummy = (uint32_t*)(args[looper]);
      printed = frontFPPTxt;
      printed += "\t type : ";
      printed += variableType;
      printed += " : variable name : ";
      printed += variableName[0];
      printed += " : ";
      if(dummy == NULL) {
        sprintf(tempIntStr,"d=0 0x0");
      } else {
        sprintf(tempIntStr,"d=%u 0x%.08X",*dummy,*dummy);
      }
      printed += tempIntStr;
      printed += "\n";
      printed += frontFPPTxt;
      printed += "\t ***************************************\n";

      if((inOut == ECMD_FPP_FUNCTIONOUT) &&
         (ecmdClientDebug == 8)          &&
         ((dummy != NULL) && (*dummy == 0))                     ) {
        return;
      }

      debugFunctionOuput(printed.c_str());

    } else if((!strcmp(variableType,"int")) ||
             (!strcmp(variableType,"int&")) ||
             (!strcmp(variableType,"int &"))) {
      /* int */

      int* dummy = (int*)(args[looper]);
      printed = frontFPPTxt;
      printed += "\t type : ";
      printed += variableType;
      printed += " : variable name : ";
      printed += variableName[0];
      printed += " : ";
      if(dummy == NULL) {
        sprintf(tempIntStr,"d=0 0x0");
      } else {
        sprintf(tempIntStr,"d=%d 0x%.08X",*dummy,*dummy);
      }
      printed += tempIntStr;
      printed += "\n";
      printed += frontFPPTxt;
      printed += "\t ***************************************\n";

      if((inOut == ECMD_FPP_FUNCTIONOUT) &&
         (ecmdClientDebug == 8)          &&
         ((dummy != NULL) && (*dummy == 0))                     ) {
        return;
      }

      debugFunctionOuput(printed.c_str());

    } else if((!strcmp(variableType,"uint64_t")) ||
             (!strcmp(variableType,"uint64_t&")) ||
             (!strcmp(variableType,"uint64_t &"))) {
      /* uint64_t */
      uint64_t* dummy = (uint64_t*)(args[looper]);

      printed = frontFPPTxt;
      printed += "\t type : ";
      printed += variableType;
      printed += " : variable name : ";
      printed += variableName[0];
      printed += " : ";
      if(dummy == NULL) {
        sprintf(tempIntStr,"d=0 0x0");
      } else {
        sprintf(tempIntStr,"0x" UINT64_HEX16_FORMAT,*dummy);
      }
      printed += tempIntStr;
      printed += "\n";
      printed += frontFPPTxt;
      printed += "\t ***************************************\n";
      debugFunctionOuput(printed.c_str());

    } else if((!strcmp(variableType,"ecmdDllType_t"))           ||
              (!strcmp(variableType,"ecmdDllProduct_t"))        ||
              (!strcmp(variableType,"ecmdDllEnv_t"))            ||
              (!strcmp(variableType,"ecmdChipTargetState_t"))   ||
              (!strcmp(variableType,"ecmdChipInterfaceType_t")) ||
              (!strcmp(variableType,"ecmdQueryDetail_t"))       ||
              (!strcmp(variableType,"ecmdClockState_t"))        ||
              (!strcmp(variableType,"ecmdSpyType_t"))           ||
              (!strcmp(variableType,"ecmdFileType_t"))          ||
              (!strcmp(variableType,"ecmdLoopType_t"))    ||
              (!strcmp(variableType,"ecmdGlobalVarType_t"))     ||
              (!strcmp(variableType,"ecmdTraceType_t"))         ||
              (!strcmp(variableType,"ecmdLatchMode_t"))         ||
              (!strcmp(variableType,"ecmdLoopMode_t"))    ||
              (!strcmp(variableType,"ecmdTargetDisplayMode_t")) ||
              (!strcmp(variableType,"ecmdTargetDisplayMode_t &")) ||
              (!strcmp(variableType,"efppInOut_t"))               ){
      /* enums */
      int* dummy = (int*)(args[looper]);

      printed = frontFPPTxt;
      printed += "\t type : ";
      printed += variableType;
      printed += " : variable name : ";
      printed += variableName[0];
      printed += " = ";
      sprintf(tempIntStr,"%u",*dummy);
      printed += tempIntStr;

      printed += "\n";
      printed += frontFPPTxt;
      printed += "\t ***************************************\n";
      debugFunctionOuput(printed.c_str());

    } else if(!strcmp(variableType,"int *")) {
      /* int * */
      int *dummy = (int*)(args[looper]);

      printed = frontFPPTxt;
      printed += "\t type : ";
      printed += variableType;
      printed += " : variable name : ";
      printed += variableName[0];
      printed += " = ";
      sprintf(tempIntStr,"%d",*dummy);
      printed += tempIntStr;

      printed += "\n";
      printed += frontFPPTxt;
      printed += "\t ***************************************\n";
      debugFunctionOuput(printed.c_str());

    } else if(!strcmp(variableType,"bool")) {
      /* bool */
      bool* dummy = (bool*)(args[looper]);

      printed = frontFPPTxt;
      printed += "\t type : ";
      printed += variableType;
      printed += " : variable name : ";
      printed += variableName[0];
      printed += " = ";

      if(*dummy) {
        printed += "TRUE";

      } else {
        printed += "FALSE";
      }

      printed += "\n";
      printed += frontFPPTxt;
      printed += "\t ***************************************\n";
      debugFunctionOuput(printed.c_str());

    } else if(!strcmp(variableType,"char**")) {
      /* char** */
      char** dummy = (char**)(args[looper]);

      printed = frontFPPTxt;
      printed += "\t type : ";
      printed += variableType;
      printed += " : variable name : ";
      printed += variableName[0];
      printed += " = ";
      printed += *dummy;
      printed += "\n";
      printed += frontFPPTxt;
      printed += "\t ***************************************\n";
      debugFunctionOuput(printed.c_str());


    } else if((!strcmp(variableType,"ecmdGroupData &"))  ||
              (!strcmp(variableType,"ecmdArrayData &"))  ||
              (!strcmp(variableType,"ecmdArrayEntry &"))  ||
              (!strcmp(variableType,"ecmdNameEntry &"))  ||
              (!strcmp(variableType,"ecmdIndexEntry &"))  ||
              (!strcmp(variableType,"ecmdSpyData &"))  ||

              (!strcmp(variableType,"ecmdProcRegisterInfo &"))  ||
              (!strcmp(variableType,"ecmdLooperData &"))  ||
              (!strcmp(variableType,"ecmdLooperData&"))   ||
              (!strcmp(variableType,"ecmdThreadData &"))  ||
              (!strcmp(variableType,"ecmdCoreData &"))  ||
              (!strcmp(variableType,"ecmdSlotData &"))  ||
              (!strcmp(variableType,"ecmdNodeData &"))  ||
              (!strcmp(variableType,"ecmdCageData &"))  ||
              (!strcmp(variableType,"ecmdRingData &"))  ||
              (!strcmp(variableType,"ecmdCageData &"))
              ) {
      /* default structures not coded yet */
      printed = frontFPPTxt;
      printed += "\t type : ";
      printed += variableType;
      printed += " : variable name : ";
      printed += variableName[0];
      printed += "\n";
      printed += frontFPPTxt;
      printed += "\t \t value = ";
      printed += "placeholder for structure dump\n";
      printed += frontFPPTxt;
      printed += "\t ***************************************\n";
      debugFunctionOuput(printed.c_str());

    } else if((!strcmp(variableType,"ecmdQueryData &")) ||
              (!strcmp(variableType,"ecmdQueryData&"))    ) {

      ecmdQueryData* dummy = (ecmdQueryData*)(args[looper]);


      printed = frontFPPTxt;
      printed += "\t type : ";
      printed += variableType;
      printed += " : variable name : ";
      printed += variableName[0];
      printed += "\n";
      debugFunctionOuput(printed.c_str());

      std::list<ecmdCageData>::iterator ecmdCurCage;
      std::list<ecmdNodeData>::iterator ecmdCurNode;
      std::list<ecmdSlotData>::iterator ecmdCurSlot;
      std::list<ecmdChipData>::iterator ecmdCurChip;
      std::list<ecmdChipUnitData>::iterator ecmdCurChipUnit;
      std::list<ecmdThreadData>::iterator ecmdCurThread;
      char buf[100];
      if (dummy->cageData.empty()) {
        printed = frontFPPTxt;
        printed += "\t \t value = EMPTY\n"; debugFunctionOuput(printed.c_str());
      } else {

        for (ecmdCurCage = dummy->cageData.begin(); ecmdCurCage != dummy->cageData.end(); ecmdCurCage ++) {
          sprintf(buf,"%s\t \t k%d\n",frontFPPTxt, ecmdCurCage->cageId); debugFunctionOuput(buf);
          for (ecmdCurNode = ecmdCurCage->nodeData.begin(); ecmdCurNode != ecmdCurCage->nodeData.end(); ecmdCurNode ++) {
            sprintf(buf,"%s\t \t   n%d\n",frontFPPTxt, ecmdCurNode->nodeId); debugFunctionOuput(buf);

            for (ecmdCurSlot = ecmdCurNode->slotData.begin(); ecmdCurSlot != ecmdCurNode->slotData.end(); ecmdCurSlot ++) {
              sprintf(buf,"%s\t \t     s%d\n",frontFPPTxt, ecmdCurSlot->slotId); debugFunctionOuput(buf);

              for (ecmdCurChip = ecmdCurSlot->chipData.begin(); ecmdCurChip != ecmdCurSlot->chipData.end(); ecmdCurChip ++) {
                sprintf(buf,"%s\t \t       %s:p%d\n",frontFPPTxt, ecmdCurChip->chipType.c_str(), ecmdCurChip->pos); debugFunctionOuput(buf);

                for (ecmdCurChipUnit = ecmdCurChip->chipUnitData.begin(); ecmdCurChipUnit != ecmdCurChip->chipUnitData.end(); ecmdCurChipUnit ++) {
                  sprintf(buf,"%s\t \t         %s:c%d\n",frontFPPTxt, ecmdCurChipUnit->chipUnitType.c_str(), ecmdCurChipUnit->chipUnitNum); debugFunctionOuput(buf);

                  for (ecmdCurThread = ecmdCurChipUnit->threadData.begin(); ecmdCurThread != ecmdCurChipUnit->threadData.end(); ecmdCurThread ++) {
                    sprintf(buf,"%s\t \t           t%d\n",frontFPPTxt, ecmdCurThread->threadId); debugFunctionOuput(buf);
                  } /* curThreadIter */

                } /* curCoreIter */

              } /* curChipIter */

            } /* curSlotIter */

          } /* curNodeIter */

        } /* curCageIter */
      }
      printed = frontFPPTxt;
      printed += "\t ***************************************\n";
      debugFunctionOuput(printed.c_str());

    } else if((!strcmp(variableType,"ecmdDataBuffer &")) ||
              (!strcmp(variableType,"ecmdDataBuffer&"))  ||
              (!strcmp(variableType,"ecmdDataBuffer"))   ) {
      /***
       private:  //data
       int iv_Capacity;              ///< Actual buffer capacity - always >= iv_NumWords
       int iv_NumWords;              ///< Specified buffer size rounded to next word
       int iv_NumBits;               ///< Specified buffer size in bits
       uint32_t * iv_Data;           ///< Pointer to buffer inside iv_RealData
       uint32_t * iv_RealData;       ///< Real buffer - with header and tail
       ***/
      ecmdDataBuffer* dummy = (ecmdDataBuffer*)(args[looper]);

      printEcmdDataBuffer(variableType, variableName[0], *dummy, "");

    } else if((!strcmp(variableType,"ecmdChipTarget &")) ||
              (!strcmp(variableType,"ecmdChipTarget"))   ||
              (!strcmp(variableType,"ecmdChipTarget&"))    ) {

      /***
       struct ecmdChipTarget {

       uint32_t    cage;
       uint32_t    node;
       uint32_t    slot;
       std::string chipType;
       uint32_t    pos;
       std::string chipUnitType;
       uint8_t     chipUnitNum;
       uint8_t     thread;

       ecmdChipTargetState_t cageState;
       ecmdChipTargetState_t nodeState;
       ecmdChipTargetState_t slotState;
       ecmdChipTargetState_t chipTypeState;
       ecmdChipTargetState_t posState;
       ecmdChipTargetState_t chipUnitTypeState;
       ecmdChipTargetState_t chipUnitNumState;
       ecmdChipTargetState_t threadState;

       uint32_t unitId;
       ecmdChipTargetState_t unitIdState;
       };
       ***/

      ecmdChipTarget* dummy = (ecmdChipTarget*)(args[looper]);

      printed = frontFPPTxt;
      printed += "\t type : ";
      printed += variableType;
      printed += " : variable name : ";
      printed += variableName[0];
      printed += "\n";
      debugFunctionOuput(printed.c_str());

      printed = frontFPPTxt;
      printed += "\t \t value : uint32_t    cage         = ";
      sprintf(tempIntStr,"%u",dummy->cage);
      printed += tempIntStr;
      printed += "\tState = ";
      printed += printEcmdChipTargetState_t(dummy->cageState);
      debugFunctionOuput(printed.c_str());

      printed = frontFPPTxt;
      printed += "\t \t value : uint32_t    node         = ";
      if (dummy->node == ECMD_TARGETDEPTH_NA)
        printed += "NA";
      else {
        sprintf(tempIntStr,"%u",dummy->node);
        printed += tempIntStr;
      }
      printed += "\tState = ";
      printed += printEcmdChipTargetState_t(dummy->nodeState);
      debugFunctionOuput(printed.c_str());

      printed = frontFPPTxt;
      printed += "\t \t value : uint32_t    slot         = ";
      if (dummy->slot == ECMD_TARGETDEPTH_NA)
        printed += "NA";
      else {
        sprintf(tempIntStr,"%u",dummy->slot);
        printed += tempIntStr;
      }
      printed += "\tState = ";
      printed += printEcmdChipTargetState_t(dummy->slotState);
      debugFunctionOuput(printed.c_str());

      printed = frontFPPTxt;
      printed += "\t \t value : std::string chipType     = ";
      printed += dummy->chipType.c_str();
      printed += "\tState = ";
      printed += printEcmdChipTargetState_t(dummy->chipTypeState);
      debugFunctionOuput(printed.c_str());

      printed = frontFPPTxt;
      printed += "\t \t value : uint32_t    pos          = ";
      sprintf(tempIntStr,"%u",dummy->pos);
      printed += tempIntStr;
      printed += "\tState = ";
      printed += printEcmdChipTargetState_t(dummy->posState);
      debugFunctionOuput(printed.c_str());

      printed = frontFPPTxt;
      printed += "\t \t value : std::string chipUnitType = ";
      printed += dummy->chipUnitType.c_str();
      printed += "\tState = ";
      printed += printEcmdChipTargetState_t(dummy->chipUnitTypeState);
      debugFunctionOuput(printed.c_str());

      printed = frontFPPTxt;
      printed += "\t \t value : uint32_t    chipUnitNum  = ";
      sprintf(tempIntStr,"%u",dummy->chipUnitNum);
      printed += tempIntStr;
      printed += "\tState = ";
      printed += printEcmdChipTargetState_t(dummy->chipUnitNumState);
      debugFunctionOuput(printed.c_str());

      printed = frontFPPTxt;
      printed += "\t \t value : uint32_t    thread       = ";
      sprintf(tempIntStr,"%d",dummy->thread);
      printed += tempIntStr;
      printed += "\tState = ";
      printed += printEcmdChipTargetState_t(dummy->threadState);
      debugFunctionOuput(printed.c_str());

      printed = frontFPPTxt;
      printed += "\t \t value : uint32_t    unitId       = ";
      sprintf(tempIntStr,"0x%.08X",dummy->unitId);
      printed += tempIntStr;
      printed += "\tState = ";
      printed += printEcmdChipTargetState_t(dummy->unitIdState);
      debugFunctionOuput(printed.c_str());


      printed = frontFPPTxt;
      printed += "\t ***************************************\n";
      debugFunctionOuput(printed.c_str());

    } else if((!strcmp(variableType,"ecmdDllInfo &")) ||
              (!strcmp(variableType,"ecmdDllInfo&"))    ) {

      /***
       struct ecmdDllInfo {
       ecmdDllType_t         dllType;        ///< Dll instance type running
       ecmdDllProduct_t      dllProduct;     ///< Dll product supported
       ecmdDllEnv_t          dllEnv;         ///< Dll environment (Simulation vs Hardware)
       std::string           dllBuildDate;   ///< Date the Dll was built
       std::string           dllCapiVersion; ///< should be set to ECMD_CAPI_VERSION
       std::string           dllBuildInfo;   ///< Any additional info the Dll/Plugin would like to pass
       };
       ***/

      ecmdDllInfo* dummy = (ecmdDllInfo*)(args[looper]);

      printed = frontFPPTxt;
      printed += "\t type : ";
      printed += variableType;
      printed += " : variable name : ";
      printed += variableName[0];
      printed += "\n";
      printed += frontFPPTxt;
      printed += "\t \t value : ecmdDllType_t         dllType = " + dummy->dllType + "\n";
      debugFunctionOuput(printed.c_str());

      printed = frontFPPTxt;
      printed += "\t \t value : ecmdDllProduct_t      dllProduct = " + dummy->dllProduct + "\n";
      debugFunctionOuput(printed.c_str());

      printed = frontFPPTxt;
      printed += "\t \t value : ecmdDllEnv_t          dllEnv = ";
      if (dummy->dllEnv == ECMD_DLL_ENV_HW)
        printed += "Hardware\n";
      else
        printed += "Simulation\n";
      debugFunctionOuput(printed.c_str());

      printed = frontFPPTxt;
      printed += "\t \t value : std::string           dllBuildDate = ";
      //      printed += "strings not working yet";
      printed += dummy->dllBuildDate.c_str();
      printed += "\n";
      debugFunctionOuput(printed.c_str());

      printed = frontFPPTxt;
      printed += "\t \t value : std::string           dllCapiVersion = ";
      //      printed += "strings not working yet";
      printed += dummy->dllCapiVersion.c_str();
      printed += "\n";
      debugFunctionOuput(printed.c_str());

      printed = frontFPPTxt;
      printed += "\t \t value : std::string           dllBuildInfo = ";
      printed += dummy->dllBuildInfo.c_str();
      //      printed += "strings not working yet";

      printed += "\n";
      debugFunctionOuput(printed.c_str());

      printed = frontFPPTxt;
      printed += "\t ***************************************\n";
      debugFunctionOuput(printed.c_str());
      /*#endif*/

    } else {
      printed = "WARNING::ecmdFunctionParmPrinter  Unknown variableType = ";
      printed += variableType;
      printed += " \n";
      printed += frontFPPTxt;
      printed += "\t ***************************************\n";
      debugFunctionOuput(printed.c_str());
    }    

  }
  printed = frontFPPTxt;
  printed += "*********************************************************************************\n";
  debugFunctionOuput(printed.c_str());
  printed = frontFPPTxt;
  printed += "*********************************************************************************\n";
  debugFunctionOuput(printed.c_str());

}
#endif

#ifndef ECMD_STRIP_DEBUG
std::string printEcmdChipTargetState_t(ecmdChipTargetState_t i_state) {

  if (i_state == ECMD_TARGET_UNKNOWN_STATE ) {
    return "ECMD_TARGET_UNKNOWN_STATE\n";
  } else if (i_state == ECMD_TARGET_FIELD_VALID ) {
    return "ECMD_TARGET_FIELD_VALID\n";
  } else if (i_state == ECMD_TARGET_FIELD_UNUSED ) {
    return "ECMD_TARGET_FIELD_UNUSED\n";
  } else if (i_state == ECMD_TARGET_FIELD_WILDCARD ) {
    return "ECMD_TARGET_FIELD_WILDCARD\n";
  } else if (i_state == ECMD_TARGET_THREAD_ALIVE ) {
    return "ECMD_TARGET_THREAD_ALIVE\n";
  } else {
    return "ERROR INVALID TYPE\n";
  }

}
#endif

#ifndef ECMD_STRIP_DEBUG
void printEcmdDataBuffer(std::string variableType, std::string variableName, const ecmdDataBuffer & i_data, std::string tabStop) {
  std::string printed;
  char tempIntStr[180];
  int dataLooper;

  printed = frontFPPTxt;
  printed += "\t "+tabStop+"type : ";
  printed += variableType;
  printed += " : variable name : ";
  printed += variableName;
  printed += "\n";
  debugFunctionOuput(printed.c_str());

  printed = frontFPPTxt;
  printed += "\t \t "+tabStop+"value : int    iv_Capacity = ";
  sprintf(tempIntStr,"%d",i_data.getCapacity());
  printed += tempIntStr;
  printed += "\n";
  debugFunctionOuput(printed.c_str());

  printed = frontFPPTxt;
  printed += "\t \t "+tabStop+"value : int    iv_NumWords = ";
  sprintf(tempIntStr,"%d",i_data.getWordLength());
  printed += tempIntStr;
  printed += "\n";
  debugFunctionOuput(printed.c_str());

  printed = frontFPPTxt;
  printed += "\t \t "+tabStop+"value : int    iv_NumBits  = ";
  sprintf(tempIntStr,"%d",i_data.getBitLength());
  printed += tempIntStr;
  printed += "\n";
  debugFunctionOuput(printed.c_str());

  if(i_data.getBitLength() == 0) {
    printed = frontFPPTxt;
    printed += "\t \t "+tabStop+"value : uint32_t[] iv_Data =\n";
    printed += frontFPPTxt;
    printed += "\t "+tabStop+"***************************************\n";
    debugFunctionOuput(printed.c_str());
    return;
  }

#ifndef REMOVE_SIM
  if (i_data.isXstateEnabled() && i_data.hasXstate()) {
    if (ecmdClientDebug == 9) {
      printed = frontFPPTxt;
      printed += "\t \t "+tabStop+"value : XSTATE iv_DataStr  = ";
      printed += i_data.genXstateStr(0, 64 < i_data.getBitLength() ? 64 : i_data.getBitLength());
      printed += "\n";
      debugFunctionOuput(printed.c_str());
    } else {
      printed = frontFPPTxt;
      printed += "\t \t "+tabStop+"value : XSTATE iv_DataStr  = ";
      printed += i_data.genXstateStr();
      printed += "\n";
      debugFunctionOuput(printed.c_str());
    }
  } else {
#endif /* REMOVE_SIM */
    if (ecmdClientDebug == 9) {
      printed = frontFPPTxt;
      printed += "\t \t "+tabStop+"value : uint32_t[] iv_Data = ";
      printed += i_data.genHexLeftStr(0, 64 < i_data.getBitLength() ? 64 : i_data.getBitLength());
      printed += "\n";
      debugFunctionOuput(printed.c_str());
    } else {

      char tbuf[10];
      printed = frontFPPTxt;
      printed += "\t \t "+tabStop+"value : uint32_t[] iv_Data = ";

      if (i_data.getWordLength() > 4) {
        printed += "\n"; debugFunctionOuput(printed.c_str());
        printed = frontFPPTxt;
        printed += "\t \t \t\t"+tabStop+"";
      }

      for (dataLooper = 0; (uint32_t) dataLooper < i_data.getWordLength(); dataLooper ++) {
        if (!(dataLooper % 4) && (dataLooper != 0)) {
          printed += "\n";
          debugFunctionOuput(printed.c_str());
          printed = frontFPPTxt;
          printed += "\t \t \t\t"+tabStop+"";
        }
        sprintf(tbuf,"%.08X ",i_data.getWord(dataLooper));
        printed += tbuf;
      }
      printed += "\n";
      debugFunctionOuput(printed.c_str());

    }
#ifndef REMOVE_SIM
  }
#endif
  printed = frontFPPTxt;
  printed += "\t "+tabStop+"***************************************\n";
  debugFunctionOuput(printed.c_str());

  return ;
}
#endif /* strip_debug */

#ifndef ECMD_STRIP_DEBUG
void ecmdFunctionTimer(int32_t &i_myTcount, etmrInOut_t i_timerState, const char * i_funcName) {

  if (ecmdClientDebug < 5) return;

  uint32_t msTime;

  timeval curTv, listTv;
  static std::list<timeval> timeList;
  static timeval startTv;
  static timeval outsideTv;

  static float outsideTime = 0;
  static float dllTime = 0;
  char outstr[200];

  /* Update the current timer */
  gettimeofday(&curTv, NULL);

  if (i_timerState == ECMD_TMR_FUNCTIONIN) {
    if (timeList.empty()) {
      /* Calc time elapsed in outside DLL funtions */
      msTime = (curTv.tv_sec - outsideTv.tv_sec) * 1000;
      msTime += ((curTv.tv_usec - outsideTv.tv_usec) / 1000);
      outsideTime += (((float)msTime)/1000);
      sprintf(outstr,"ECMD DEBUG (ecmdTMR) : ENTER(%03d) : %dms before %s\n", i_myTcount, msTime, i_funcName);
      debugFunctionOuput(outstr);
    } else {
      sprintf(outstr,"ECMD DEBUG (ecmdTMR) : ENTER(%03d) : nested call to %s\n", i_myTcount, i_funcName);
      debugFunctionOuput(outstr);
    }
    /* Save our time so we can calc time spent in DLL */
    timeList.push_back(curTv);
  } else if (i_timerState == ECMD_TMR_FUNCTIONOUT) {
    /* Get what we have on the list */
    listTv = timeList.back();
    timeList.pop_back();
    /* Calc time spent */
    msTime = (curTv.tv_sec - listTv.tv_sec) * 1000;
    msTime += ((curTv.tv_usec - listTv.tv_usec) / 1000);
    sprintf(outstr,"ECMD DEBUG (ecmdTMR) : EXIT (%03d) : %dms spent in %s\n", i_myTcount, msTime, i_funcName);
    debugFunctionOuput(outstr);
    /* If the list is empty, save the time spent in the DLL and set the outsideTV */
    if (timeList.empty()) {
      dllTime += (((float)msTime)/1000);
      outsideTv = curTv;
    }
  } else if (i_timerState == ECMD_TMR_LOADDLL) {
    startTv = curTv;
    outsideTv = curTv;
    sprintf(outstr,"ECMD DEBUG (ecmdTMR) : START(%03d) : Starting timing code in ecmdLoadDll\n", i_myTcount);
    debugFunctionOuput(outstr);
  } else if (i_timerState == ECMD_TMR_UNLOADDLL) {
    msTime = (curTv.tv_sec - startTv.tv_sec) * 1000;
    msTime += ((curTv.tv_usec - startTv.tv_usec) / 1000);
    float totalTime = (((float)msTime)/1000);
    float dllPercent = (dllTime/totalTime)*100;
    float outsidePercent = (outsideTime/totalTime)*100;
    float lostTime = totalTime - (dllTime + outsideTime);
    float lostPercent = 100 - (dllPercent + outsidePercent);
    sprintf(outstr,"ECMD DEBUG (ecmdTMR) : FINAL(%03d) : ***************************************\n", i_myTcount);
    debugFunctionOuput(outstr);
    sprintf(outstr,"ECMD DEBUG (ecmdTMR) : FINAL(%03d) : Total time DLL Loaded:  %3.3fs\n", i_myTcount, totalTime);
    debugFunctionOuput(outstr);
    sprintf(outstr,"ECMD DEBUG (ecmdTMR) : FINAL(%03d) : Time spent in DLL:      %3.3fs (%2.2f%%)\n", i_myTcount, dllTime, dllPercent);
    debugFunctionOuput(outstr);
    sprintf(outstr,"ECMD DEBUG (ecmdTMR) : FINAL(%03d) : Time spent outside DLL: %3.3fs (%2.2f%%)\n", i_myTcount, outsideTime, outsidePercent);
    debugFunctionOuput(outstr);
    sprintf(outstr,"ECMD DEBUG (ecmdTMR) : FINAL(%03d) : Time unaccounted for:   %3.3fs (%2.2f%%)\n", i_myTcount, lostTime, lostPercent);
    debugFunctionOuput(outstr);
  }
};
#endif /* strip_debug */

#ifndef ECMD_STRIP_DEBUG
void debugFunctionOuput(const char* outbuf) {
  if (ecmdDebugOutput) {
    ecmdOutput(outbuf);
  } else {
    printf("%s", outbuf);
  }
}
#endif /* strip_debug */

/**
 @brief Registers an extensions initstate pointer defect #18081
 @param i_initState Pointer to initState static so it can be reset later
 */
static std::list<bool*> g_initPtrs;
void ecmdRegisterExtensionInitState(bool* i_initState) {
  if (find(g_initPtrs.begin(), g_initPtrs.end(), i_initState) == g_initPtrs.end()) {
    g_initPtrs.push_back(i_initState);
  }
}

/**
 @brief Reset Extension initstate pointer to uninitialized
*/
void ecmdResetExtensionInitState() {
  for (std::list<bool*>::iterator ptrit = g_initPtrs.begin(); ptrit != g_initPtrs.end(); ptrit ++) {
      *(*ptrit) = false;
      // Now erase the pointer from the list to avoid seg faults when unloading the dll more than once without loading again.
      // There could be cases where the initialized extensions have gone out of scope and no longer have a 
      // valid bool in the list.  If accessed in this situation, it causes a seg fault.
      ptrit = g_initPtrs.erase(ptrit);
      --ptrit;
  }
}
