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
#include <fstream>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdexcept> 
#include <climits>

#include <ecmdClientCapi.H>
#include <fapi2ClientCapi.H>
#include <fapi2Interpreter.H>
#include <target_types.H>
#include <ecmdCommandUtils.H>
#include <ecmdReturnCodes.H>
#include <ecmdUtils.H>
#include <ecmdDataBuffer.H>
#include <ecmdSharedUtils.H>
#include <algorithm>
#include <map>

//----------------------------------------------------------------------
//  Helper Functions
//----------------------------------------------------------------------

template <typename T>
uint32_t convertStrToType(const std::string &i_attributeName, std::string &i_attributeDataStr, const std::string &i_format, T &o_value) {
  uint32_t rc = ECMD_SUCCESS;
  std::string printed;

  if ((i_format.compare("x") == 0) || (i_format.compare("xl") == 0) || (i_format.compare("xr") == 0) || (i_format.compare("b") == 0) || (i_format.compare("d") == 0))
  {
      printed = "convertStrToType - Inputted value: ";
      printed += i_attributeDataStr;

      bool l_invalidChar = false;
      std::string searchStr = i_attributeDataStr;
      if (i_format.compare("b") == 0) // if type == binary, all characters must be 1,0 or -
      {
          if (searchStr.substr(0,2).compare("0b") == 0)
          {
              searchStr.erase(0,2);
              i_attributeDataStr = searchStr;
          }
          else if (searchStr.substr(0,3).compare("-0b") == 0)
          {
              searchStr.erase(1,2);
              i_attributeDataStr = searchStr;
              searchStr = searchStr.substr(1);
          }
          else if (searchStr.substr(0,1).compare("-") == 0) searchStr = searchStr.substr(1);

          if (searchStr.find_first_not_of("01") != std::string::npos)
          {
              printed += ", cannot convert to binary type, invalid char detected.\n";
              l_invalidChar = true;
          }
      }
      else if (i_format.compare("d") == 0 ) // if type == decimal, all characters must be 1-9 or -
      {
          if (searchStr.substr(0,1).compare("-") == 0) searchStr = searchStr.substr(1);
          if (searchStr.find_first_not_of("0123456789") != std::string::npos)
          {
              printed += ", cannot convert to decimal type, invalid char detected.\n";
              l_invalidChar = true;
          }
      }
      else // if type == hex, all characters must be 1-9, A-F or -
      {
          if (searchStr.substr(0,2).compare("0x") == 0) searchStr = searchStr.substr(2);
          else if (searchStr.substr(0,3).compare("-0x") == 0) searchStr = searchStr.substr(3);
          else if (searchStr.substr(0,1).compare("-") == 0) searchStr = searchStr.substr(1);

          if (searchStr.find_first_not_of("0123456789ABCDEFabcdef") != std::string::npos)
          {
              printed += ", cannot convert to hex type, invalid character detected.\n";
              l_invalidChar = true;
          }
      }

      if (l_invalidChar)
      {
          ecmdOutputError(printed.c_str());
          return ECMD_FAPI2_DATA_CONVERSION_ERROR;
      }

      if (std::is_same<T,uint8_t>::value || std::is_same<T,uint16_t>::value || std::is_same<T,uint32_t>::value || std::is_same<T,uint64_t>::value)
      {
          unsigned long long l_convertedVal = 0;
          unsigned long long min = 0;
          unsigned long long max = 0;

          if (i_attributeDataStr.substr(0,1).compare("-") == 0)
          {
              printed += " is out of the range of the attribute type.\n";
              ecmdOutputError(printed.c_str());
              return ECMD_FAPI2_DATA_CONVERSION_ERROR;
          }

          try 
          {   
              if ((i_format.compare("x") == 0) || (i_format.compare("xl") == 0) || (i_format.compare("xr") == 0)) l_convertedVal = std::strtoul(i_attributeDataStr.c_str(), NULL, 16);
              else if (i_format.compare("b") == 0) l_convertedVal = std::strtoul(i_attributeDataStr.c_str(), NULL, 2);
              else l_convertedVal = std::strtoul(i_attributeDataStr.c_str(), NULL, 10);
          }
          catch (const std::out_of_range &err)
          {
              printed += " is out of the range of the attribute type.\n";
              ecmdOutputError(printed.c_str());
              return ECMD_FAPI2_DATA_CONVERSION_ERROR;
          }
          catch (const std::invalid_argument &err)
          {
              printed += " could not be converted to ";
              printed += i_format + " format with std::strtoul.\n";
              ecmdOutputError(printed.c_str());
              return ECMD_FAPI2_DATA_CONVERSION_ERROR;
          }

          // std::strtoul will return 0 if a conversion cannot be performed
          if (i_attributeDataStr.compare("0") != 0 && (l_convertedVal == 0))
          {
              printed += " could not be converted to ";
              printed += i_format + " format with std::strtoul.\n";
              ecmdOutputError(printed.c_str());
              return ECMD_FAPI2_DATA_CONVERSION_ERROR;
          }

          if (std::is_same<T,uint8_t>::value) max = UCHAR_MAX;
          else if (std::is_same<T,uint16_t>::value) max = USHRT_MAX;
          else if (std::is_same<T,uint32_t>::value) max = UINT_MAX;
          else if (std::is_same<T,uint64_t>::value) max = ULLONG_MAX;

          if ((l_convertedVal < min) || (l_convertedVal > max))
          {
              printed += " is out of the range of the attribute type.\n";
              ecmdOutputError(printed.c_str());
              return ECMD_FAPI2_DATA_CONVERSION_ERROR;
          }
          else
          {
              o_value = (T)l_convertedVal;
          }
      } 
      else
      {
          long long l_convertedVal = 0;
          long long min = 0;
          long long max = 0;

          try 
          {
              if ((i_format.compare("x") == 0) || (i_format.compare("xl") == 0) || (i_format.compare("xr") == 0)) l_convertedVal = std::stoll(i_attributeDataStr.c_str(), NULL, 16);
              else if (i_format.compare("b") == 0) l_convertedVal = std::stoll(i_attributeDataStr.c_str(), NULL, 2);
              else l_convertedVal = std::stoll(i_attributeDataStr.c_str(), NULL, 10);
          }
          catch (const std::out_of_range &err) 
          {
              printed += " is out of the range of the attribute type.\n";
              ecmdOutputError(printed.c_str());
              return ECMD_FAPI2_DATA_CONVERSION_ERROR;     
          }
          catch (const std::invalid_argument &err)
          {
              printed += " could not be converted to ";
              printed += i_format + " format with std::strtoul.\n";
              ecmdOutputError(printed.c_str());
              return ECMD_FAPI2_DATA_CONVERSION_ERROR;
          }

          // std::strtoul will return 0 if a conversion cannot be performed
          if (i_attributeDataStr.compare("0") != 0 && (l_convertedVal == 0))
          {
              printed += " could not be converted to ";
              printed += i_format + " format with std::strtoul.\n";
              ecmdOutputError(printed.c_str());
              return ECMD_FAPI2_DATA_CONVERSION_ERROR;
          }

          if (std::is_same<T,int8_t>::value)
          {
              min = SCHAR_MIN;
              max = SCHAR_MAX;
          }
          else if (std::is_same<T,int16_t>::value)
          {
              min = SHRT_MIN;
              max = SHRT_MAX;
          }
          else if (std::is_same<T,int32_t>::value)
          {
              min = INT_MIN;
              max = INT_MAX;
          }
          else if (std::is_same<T,int64_t>::value)
          {
              min = LLONG_MIN;
              max = LLONG_MAX;
          }

          if ((l_convertedVal < min) || (l_convertedVal > max))
          {
              printed += " is out of the range of the attribute type.\n";
              ecmdOutputError(printed.c_str());
              return ECMD_FAPI2_DATA_CONVERSION_ERROR;
          }
          else 
          {
              o_value = (T)l_convertedVal;
          }
      }
  }
  else if (i_format.compare("enum") == 0)
  {
      fapi2::AttributeData l_enumData;
      l_enumData.faString = const_cast<char*>(i_attributeDataStr.c_str());
      l_enumData.faName = i_attributeName; 

      if (std::is_same<T,uint8_t>::value) l_enumData.faValidMask = FAPI_ATTRIBUTE_TYPE_UINT8;
      else if (std::is_same<T,uint16_t>::value) l_enumData.faValidMask = FAPI_ATTRIBUTE_TYPE_UINT16;
      else if (std::is_same<T,uint32_t>::value) l_enumData.faValidMask = FAPI_ATTRIBUTE_TYPE_UINT32;
      else if (std::is_same<T,uint64_t>::value) l_enumData.faValidMask = FAPI_ATTRIBUTE_TYPE_UINT64;
      else if (std::is_same<T,int8_t>::value) l_enumData.faValidMask = FAPI_ATTRIBUTE_TYPE_INT8;
      else if (std::is_same<T,int16_t>::value) l_enumData.faValidMask = FAPI_ATTRIBUTE_TYPE_INT16;
      else if (std::is_same<T,int32_t>::value) l_enumData.faValidMask = FAPI_ATTRIBUTE_TYPE_INT32;
      else if (std::is_same<T,int64_t>::value) l_enumData.faValidMask = FAPI_ATTRIBUTE_TYPE_INT64;

      rc = fapi2AttrEnumStrToEnumVal(l_enumData);
      if (rc)
      { 
          printed = "convertStrToType - Error occured when trying to retrieve enum value from enum string \"";
          printed += i_attributeDataStr  + "\".\n";
          ecmdOutputError(printed.c_str());
          rc = ECMD_FAPI2_DATA_CONVERSION_ERROR;
      }
      else
      {
          if (std::is_same<T,uint8_t>::value) o_value = l_enumData.faUint8;
          else if (std::is_same<T,uint16_t>::value) o_value = l_enumData.faUint16;
          else if (std::is_same<T,uint32_t>::value) o_value = l_enumData.faUint32;
          else if (std::is_same<T,uint64_t>::value) o_value = l_enumData.faUint64;
          else if (std::is_same<T,int8_t>::value) o_value = l_enumData.faInt8;
          else if (std::is_same<T,int16_t>::value) o_value = l_enumData.faInt16;
          else if (std::is_same<T,int32_t>::value) o_value = l_enumData.faInt32;
          else if (std::is_same<T,int64_t>::value) o_value = l_enumData.faInt64;
      }
  }
  else
  {
    printed = "convertStrToType - Unsupported format specified with -i \"";
    printed += i_format + "\", ";
    printed += "see fapi2setattr -h for supported formats.\n";
    ecmdOutputError(printed.c_str());
    return ECMD_FAPI2_DATA_CONVERSION_ERROR;
  }

  return rc;
}

template <typename T>
uint32_t convertAttributeStrToArray(std::string &i_attributeName, std::string &i_attributeDataStr, uint32_t &i_expectedNumEntries, T &o_attributeDataArr, std::string &i_format) { 
  
  uint32_t rc = ECMD_SUCCESS;
  std::string printed;
  std::string numberStr;
  uint32_t numEntries = 0;
  int l_dataStrLength = i_attributeDataStr.length();
  int l_curIndex = 0;
   
  for (char const &c: i_attributeDataStr)
  {
      bool l_isLastChar = l_curIndex == (l_dataStrLength - 1);
      if ((',' == c) || l_isLastChar)
      {
          if (!numberStr.empty() || l_isLastChar)
          {
	      if (l_isLastChar) numberStr.push_back(c);
              rc = convertStrToType(i_attributeName, numberStr, i_format, o_attributeDataArr[numEntries]);
              if (rc) return rc;
              numEntries++;
              numberStr.clear();
          }             
      }
      else
      {
          numberStr.push_back(c);
      }
      l_curIndex++;
  }

  if (numEntries != i_expectedNumEntries)
  {
      if (numEntries > i_expectedNumEntries) printed = "convertAttributeStrToArray - Too many entries passed through, expected ";
      else printed = "convertAttributeStrToArray - Too few entries passed through, expected ";

      printed += std::to_string(i_expectedNumEntries) +  " entries, but got ";
      printed += std::to_string(numEntries) + " entries.\n";
      ecmdOutputError(printed.c_str());
      rc = ECMD_FAPI2_ARRAY_SIZE_MISMATCH_ERROR; // Too many entries have been passed through
      return rc; 
  }  

  return rc;
}


//----------------------------------------------------------------------
//  User Types
//----------------------------------------------------------------------
struct AttributeInfo {
    uint32_t attributeType;
    uint32_t numOfEntries;
    uint32_t numOfBytes;
    bool attributeEnum;
};

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

uint32_t fapi2GetAttributeUser(int argc, char * argv[]) {
  uint32_t rc = ECMD_SUCCESS, coeRc = ECMD_SUCCESS;

  ecmdChipTarget target;        ///< Current target
  std::string attributeName;    ///< Name of attribute to fetch
  bool validPosFound = false;   ///< Did we find something to actually execute on ?
  ecmdDataBuffer numData;       ///< Initialise data buffer with the numeric value
  std::string printed;          ///< Print Buffer
  ecmdLooperData looperData;    ///< Store internal Looper data
  bool l_from_hostboot = false; ///< Set flag indicating attribute should be requested from hostboot

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

  if (ecmdParseOption(&argc, &argv, "-hostboot")) {
      l_from_hostboot = true;
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
  if (argc < 1) {
    ecmdOutputError("fapi2getattr - Too few arguments specified; you need at least an attribute.\n");
    ecmdOutputError("fapi2getattr - Type 'fapi2getattr -h' for usage.\n");
    return ECMD_INVALID_ARGS;
  }


  //Setup the target that will be used to query the system config
  if (argc > 2) {
    ecmdOutputError("fapi2getattr - Too many arguments specified; you probably added an unsupported option.\n");
    ecmdOutputError("fapi2getattr - Type 'fapi2getattr -h' for usage.\n");
    return ECMD_INVALID_ARGS;
  } else if (argc == 2) {
    std::string chipType, chipUnitType;
    rc = ecmdParseChipField(argv[0], chipType, chipUnitType);
    if (rc) { 
      ecmdOutputError("fapi2getattr - Wildcard character detected however it is not supported by this command.\n");
      return rc;
    }

    /* Error check */
    if (depth) {
      if (chipUnitType == "" && depth < POS) {
        ecmdOutputError("fapi2getattr - Invalid Depth parm specified when a chip was specified.  Try with -dp.\n");
        return ECMD_INVALID_ARGS;
      }

      if (chipUnitType != "" && depth < CHIPUNIT) {
        ecmdOutputError("fapi2getattr - Invalid Depth parm specified when a chipUnit was specified.  Try with -dc.\n");
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
    attributeName = argv[1];
  } else {
    if (depth == 0) depth = CAGE;
    attributeName = argv[0];
  }

  /* Check that attribute name is valid */
  fapi2::AttributeId attributeId;
  rc = fapi2AttributeStringToId(attributeName, attributeId);
  if (rc) {
    printed = "fapi2getattr - Unknown attribute name (";
    printed += attributeName + ")\n";
    ecmdOutputError( printed.c_str() );
    return ECMD_INVALID_ARGS;
  }

  /* Determine attribute type and size */
  uint32_t attributeType = 0;
  uint32_t l_numOfEntries;
  uint32_t l_numOfBytes;
  bool attributeEnum = false;
  rc = fapi2GetAttrInfo(attributeId, attributeType, l_numOfEntries, l_numOfBytes, attributeEnum);
  if (rc) {
    printed = "fapi2getattr - Unknown attribute type (";
    printed += attributeName + ")\n";
    ecmdOutputError( printed.c_str() );
    return ECMD_INVALID_ARGS;
  }

  fapi2::AttributeData attributeData;
  attributeData.faValidMask = attributeType;
  attributeData.faEnumUsed = attributeEnum;

  if (l_from_hostboot)
  {
      attributeData.faMode = FAPI_ATTRIBUTE_MODE_HOSTBOOT;
  }

  if (attributeType == FAPI_ATTRIBUTE_TYPE_UINT8ARY)
  {
      attributeData.faUint8ary = new uint8_t[l_numOfEntries];
  }
  else if (attributeType == FAPI_ATTRIBUTE_TYPE_UINT16ARY)
  {
      attributeData.faUint16ary = new uint16_t[l_numOfEntries];
  }
  else if (attributeType == FAPI_ATTRIBUTE_TYPE_UINT32ARY)
  {
      attributeData.faUint32ary = new uint32_t[l_numOfEntries];
  }
  else if (attributeType == FAPI_ATTRIBUTE_TYPE_UINT64ARY)
  {
      attributeData.faUint64ary = new uint64_t[l_numOfEntries];
  }
  else if (attributeType == FAPI_ATTRIBUTE_TYPE_INT8ARY)
  {
      attributeData.faInt8ary = new int8_t[l_numOfEntries];
  }
  else if (attributeType == FAPI_ATTRIBUTE_TYPE_INT16ARY)
  {
      attributeData.faInt16ary = new int16_t[l_numOfEntries];
  }
  else if (attributeType == FAPI_ATTRIBUTE_TYPE_INT32ARY)
  {
      attributeData.faInt32ary = new int32_t[l_numOfEntries];
  }
  else if (attributeType == FAPI_ATTRIBUTE_TYPE_INT64ARY)
  {
      attributeData.faInt64ary = new int64_t[l_numOfEntries];
  }

  /* Now set our states based on depth */
  target.cageState = target.nodeState = target.slotState = target.posState = target.chipUnitNumState = ECMD_TARGET_FIELD_WILDCARD;
  target.threadState = ECMD_TARGET_FIELD_UNUSED;
  if (depth == POS) {
    target.chipUnitNumState = ECMD_TARGET_FIELD_UNUSED;
    target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;
  } else if (depth == SLOT) {
    target.posState = ECMD_TARGET_FIELD_UNUSED;
    target.chipTypeState = ECMD_TARGET_FIELD_UNUSED;
    target.chipUnitNumState = ECMD_TARGET_FIELD_UNUSED;
    target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;
  } else if (depth == NODE) {
    target.slotState = ECMD_TARGET_FIELD_UNUSED;
    target.posState = ECMD_TARGET_FIELD_UNUSED;
    target.chipTypeState = ECMD_TARGET_FIELD_UNUSED;
    target.chipUnitNumState = ECMD_TARGET_FIELD_UNUSED;
    target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;
  } else if (depth == CAGE) {
    target.nodeState = ECMD_TARGET_FIELD_UNUSED;
    target.slotState = ECMD_TARGET_FIELD_UNUSED;
    target.posState = ECMD_TARGET_FIELD_UNUSED;
    target.chipTypeState = ECMD_TARGET_FIELD_UNUSED;
    target.chipUnitNumState = ECMD_TARGET_FIELD_UNUSED;
    target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;
  }

  /************************************************************************/
  /* Kickoff Looping Stuff                                                */
  /************************************************************************/
  rc = ecmdLooperInit(target, ECMD_SELECTED_TARGETS_LOOP, looperData);
  if (rc)
  {
    if (attributeData.faUint8ary != NULL) delete [] attributeData.faUint8ary;
    else if (attributeData.faInt8ary != NULL) delete [] attributeData.faInt8ary;
    if (attributeData.faUint16ary != NULL) delete [] attributeData.faUint16ary;
    else if (attributeData.faInt16ary != NULL) delete [] attributeData.faInt16ary;
    if (attributeData.faUint32ary != NULL) delete [] attributeData.faUint32ary;
    else if (attributeData.faInt32ary != NULL) delete [] attributeData.faInt32ary;
    if (attributeData.faUint64ary != NULL) delete [] attributeData.faUint64ary;
    else if (attributeData.faInt64ary != NULL) delete [] attributeData.faInt64ary;
    return rc;
  }

  while (ecmdLooperNext(target, looperData) && (!coeRc || coeMode)) {

    /* Actually go fetch the data */
    rc = fapi2GetAttribute(target, attributeId, attributeData);
    if (rc) {
      printed = "fapi2getattr - Error occured performing fapi2GetAttribute on ";
      printed += ecmdWriteTarget(target) + "\n";
      ecmdOutputError( printed.c_str() );
      coeRc = rc;
      continue;
    }
    else {
      validPosFound = true;
    }

    printed = ecmdWriteTarget(target) + "\n";

    std::string attributeDataString;
    rc = fapi2AttributeDataToString(attributeId, attributeData, attributeDataString, true, format.c_str());
    if (rc) {
      printed = "fapi2getattr - Error occured performing fapi2AttributeDataToString for ";
      printed += attributeName + "\n";
      ecmdOutputError( printed.c_str() );
      coeRc = rc;
      continue;
    }
    printed += attributeName + " = " + attributeDataString + "\n";
    ecmdOutput(printed.c_str());

  }

  if (attributeData.faUint8ary != NULL) delete [] attributeData.faUint8ary;
  else if (attributeData.faInt8ary != NULL) delete [] attributeData.faInt8ary;
  if (attributeData.faUint16ary != NULL) delete [] attributeData.faUint16ary;
  else if (attributeData.faInt16ary != NULL) delete [] attributeData.faInt16ary;
  if (attributeData.faUint32ary != NULL) delete [] attributeData.faUint32ary;
  else if (attributeData.faInt32ary != NULL) delete [] attributeData.faInt32ary;
  if (attributeData.faUint64ary != NULL) delete [] attributeData.faUint64ary;
  else if (attributeData.faInt64ary != NULL) delete [] attributeData.faInt64ary;
  
  // coeRc will be the return code from in the loop, coe mode or not.
  if (coeRc) return coeRc;

  // This is an error common across all UI functions
  if (!validPosFound) {
    ecmdOutputError("fapi2getattr - Unable to find a valid chip to execute command on\n");
    return ECMD_TARGET_NOT_CONFIGURED;
  }

  return rc;
}

uint32_t fapi2SetAttributeUser(int argc, char * argv[]) {
  uint32_t rc = ECMD_SUCCESS, coeRc = ECMD_SUCCESS;

  ecmdChipTarget target;        ///< Current target
  std::string attributeName;    ///< Name of attribute to fetch
  std::string attributeStrValue;///< New value to set the attribute to (in String format)
  bool validPosFound = false;   ///< Did we find something to actually execute on ?
  ecmdDataBuffer numData;       ///< Initialise data buffer with the numeric value
  std::string printed;          ///< Print Buffer
  ecmdLooperData looperData;    ///< Store internal Looper data

  int CAGE = 1, NODE = 2, SLOT = 3, POS = 4, CHIPUNIT = 5;
  int depth = 0;                 ///< depth found from Command line parms

  /* get format flag, if it's there */
  std::string format;
  char * formatPtr = ecmdParseOptionWithArgs(&argc, &argv, "-i");
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
  if (argc < 2) {
    ecmdOutputError("fapi2setattr - Too few arguments specified; you need at least an attribute and a value to set it to.\n");
    ecmdOutputError("fapi2setattr - Type 'fapi2setattr -h' for usage.\n");
    return ECMD_INVALID_ARGS;
  }

  //Setup the target that will be used to query the system config
  if (argc > 3) {
    ecmdOutputError("fapi2setattr - Too many arguments specified; you probably added an unsupported option.\n");
    ecmdOutputError("fapi2setattr - Type 'fapi2setattr -h' for usage.\n");
    return ECMD_INVALID_ARGS;
  } else if (argc == 3) {
    std::string chipType, chipUnitType;
    rc = ecmdParseChipField(argv[0], chipType, chipUnitType);
    if (rc) { 
      ecmdOutputError("fapi2setattr - Wildcard character detected however it is not supported by this command.\n");
      return rc;
    }

    /* Error check */
    if (depth) {
      if (chipUnitType == "" && depth < POS) {
        ecmdOutputError("fapi2setattr - Invalid Depth parm specified when a chip was specified.  Try with -dp.\n");
        return ECMD_INVALID_ARGS;
      }

      if (chipUnitType != "" && depth < CHIPUNIT) {
        ecmdOutputError("fapi2setattr - Invalid Depth parm specified when a chipUnit was specified.  Try with -dc.\n");
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
    attributeName = argv[1];
    attributeStrValue = argv[2];
  } else {
    if (depth == 0) depth = CAGE;
    attributeName = argv[0];
    attributeStrValue = argv[1];
  }

  /* Check that attribute name is valid */
  fapi2::AttributeId attributeId;
  rc = fapi2AttributeStringToId(attributeName, attributeId);
  if (rc) {
    printed = "fapi2setattr - Unknown attribute name (";
    printed += attributeName + ")\n";
    ecmdOutputError( printed.c_str() );
    return ECMD_INVALID_ARGS;
  }

  /* Determine attribute type and size */
  uint32_t attributeType = 0;
  uint32_t l_numOfEntries;
  uint32_t l_numOfBytes;
  bool attributeEnum = false;
  rc = fapi2GetAttrInfo(attributeId, attributeType, l_numOfEntries, l_numOfBytes, attributeEnum);
  if (rc) {
    printed = "fapi2setattr - Unknown attribute type (";
    printed += attributeName + ")\n";
    ecmdOutputError( printed.c_str() );
    return ECMD_INVALID_ARGS;
  }
  fapi2::AttributeData attributeData;
  attributeData.faValidMask = attributeType;
  attributeData.faEnumUsed = attributeEnum;

  // formating error string for failure when calling convertStrToType()
  printed = "fapi2setattr - Error occured when trying to convert inputted value: "; 

  if (attributeType == FAPI_ATTRIBUTE_TYPE_UINT8)
  {
      rc = convertStrToType(attributeName, attributeStrValue, format, attributeData.faUint8);
      if (rc)
      {   
          printed += attributeStrValue + " to AttributeData.faUint8.\n";
          ecmdOutputError(printed.c_str());
          return rc;
      }
  }
  else if (attributeType == FAPI_ATTRIBUTE_TYPE_UINT16)
  {
      rc = convertStrToType(attributeName, attributeStrValue, format, attributeData.faUint16);
      if (rc)
      {
          printed += attributeStrValue + " to AttributeData.faUint16.\n";
          ecmdOutputError(printed.c_str());
          return rc;
      }
  }
  else if (attributeType == FAPI_ATTRIBUTE_TYPE_UINT32)
  {
      rc = convertStrToType(attributeName, attributeStrValue, format, attributeData.faUint32);
      if (rc)
      {
          printed += attributeStrValue + " to AttributeData.faUint32.\n";
          ecmdOutputError(printed.c_str());
          return rc;
      }
  }
  else if (attributeType == FAPI_ATTRIBUTE_TYPE_UINT64)
  {
      rc = convertStrToType(attributeName, attributeStrValue, format, attributeData.faUint64);
      if (rc)
      {
          printed += attributeStrValue + " to AttributeData.faUint64.\n";
          ecmdOutputError(printed.c_str());
          return rc;
      }
  }
  else if  (attributeType == FAPI_ATTRIBUTE_TYPE_UINT8ARY)
  {
      attributeData.faUint8ary = new uint8_t[l_numOfEntries];
      rc = convertAttributeStrToArray(attributeName, attributeStrValue, l_numOfEntries, attributeData.faUint8ary, format);
      if (rc) 
      {
          printed += attributeStrValue + " to AttributeData.faUint8ary.\n";
          ecmdOutputError(printed.c_str());
          delete [] attributeData.faUint8ary;
          return rc; 
      }
  }
  else if (attributeType == FAPI_ATTRIBUTE_TYPE_UINT16ARY)
  {
      attributeData.faUint16ary = new uint16_t[l_numOfEntries];
      rc = convertAttributeStrToArray(attributeName, attributeStrValue, l_numOfEntries, attributeData.faUint16ary, format);
      if (rc) 
      {
          printed += attributeStrValue + " to AttributeData.faUint16ary.\n";
          ecmdOutputError(printed.c_str());
          delete [] attributeData.faUint16ary;
          return rc; 
      }
  }
  else if (attributeType == FAPI_ATTRIBUTE_TYPE_UINT32ARY)
  {
      attributeData.faUint32ary = new uint32_t[l_numOfEntries];
      rc = convertAttributeStrToArray(attributeName, attributeStrValue, l_numOfEntries, attributeData.faUint32ary, format);
      if (rc) 
      {
          printed += attributeStrValue + " to AttributeData.faUint32ary.\n";
          ecmdOutputError(printed.c_str());
          delete [] attributeData.faUint32ary;
          return rc; 
      }
  }
  else if (attributeType == FAPI_ATTRIBUTE_TYPE_UINT64ARY)
  {
      attributeData.faUint64ary = new uint64_t[l_numOfEntries];
      rc = convertAttributeStrToArray(attributeName, attributeStrValue, l_numOfEntries, attributeData.faUint64ary, format);
      if (rc) 
      {
          printed += attributeStrValue + " to AttributeData.faUint64ary.\n";
          ecmdOutputError(printed.c_str());
          delete [] attributeData.faUint64ary;
          return rc; 
      }
  }
  else if (attributeType == FAPI_ATTRIBUTE_TYPE_INT8)
  {
      rc = convertStrToType(attributeName, attributeStrValue, format, attributeData.faInt8);
      if (rc)
      {
          printed += attributeStrValue + " to AttributeData.faInt8.\n";
          ecmdOutputError(printed.c_str());
          return rc;
      }
  }
  else if (attributeType == FAPI_ATTRIBUTE_TYPE_INT16)
  {
      rc = convertStrToType(attributeName, attributeStrValue, format, attributeData.faInt16);
      if (rc)
      {
          printed += attributeStrValue + " to AttributeData.faInt16.\n";
          ecmdOutputError(printed.c_str());
          return rc;
      }
  }
  else if (attributeType == FAPI_ATTRIBUTE_TYPE_INT32)
  {
      rc = convertStrToType(attributeName, attributeStrValue, format, attributeData.faInt32);
      if (rc)
      {
          printed += attributeStrValue + " to AttributeData.faInt32.\n";
          ecmdOutputError(printed.c_str());
          return rc;
      }
  }
  else if (attributeType == FAPI_ATTRIBUTE_TYPE_INT64)
  {
      rc = convertStrToType(attributeName, attributeStrValue, format, attributeData.faInt64);
      if (rc)
      {
          printed += attributeStrValue + " to AttributeData.faInt64.\n";
          ecmdOutputError(printed.c_str());
          return rc;
      }
  }
  else if (attributeType == FAPI_ATTRIBUTE_TYPE_INT8ARY)
  {
      attributeData.faInt8ary = new int8_t[l_numOfEntries];
      rc = convertAttributeStrToArray(attributeName, attributeStrValue, l_numOfEntries, attributeData.faInt8ary, format);
      if (rc) 
      {
          printed += attributeStrValue + " to AttributeData.faInt8ary.\n";
          ecmdOutputError(printed.c_str());
          delete [] attributeData.faInt8ary;
          return rc; 
      }
  }
  else if (attributeType == FAPI_ATTRIBUTE_TYPE_INT16ARY)
  {
      attributeData.faInt16ary = new int16_t[l_numOfEntries];
      rc = convertAttributeStrToArray(attributeName, attributeStrValue, l_numOfEntries, attributeData.faInt16ary, format);
      if (rc) 
      {
          printed += attributeStrValue + " to AttributeData.faInt16ary.\n";
          ecmdOutputError(printed.c_str());
          delete [] attributeData.faInt16ary;
          return rc; 
      }
  }
  else if (attributeType == FAPI_ATTRIBUTE_TYPE_INT32ARY)
  {
      attributeData.faInt32ary = new int32_t[l_numOfEntries];
      rc = convertAttributeStrToArray(attributeName, attributeStrValue, l_numOfEntries, attributeData.faInt32ary, format);
      if (rc) 
      {
          printed += attributeStrValue + " to AttributeData.faInt32ary.\n";
          ecmdOutputError(printed.c_str());
          delete [] attributeData.faInt32ary;          
          return rc; 
      }
  }
  else if (attributeType == FAPI_ATTRIBUTE_TYPE_INT64ARY)
  {
      attributeData.faInt64ary = new int64_t[l_numOfEntries];
      rc = convertAttributeStrToArray(attributeName, attributeStrValue, l_numOfEntries, attributeData.faInt64ary, format);
      if (rc) 
      {
          printed += attributeStrValue + " to AttributeData.faInt64ary.\n";
          ecmdOutputError(printed.c_str());
          delete [] attributeData.faInt64ary;
          return rc; 
      }
  }

  /* Now set our states based on depth */
  target.cageState = target.nodeState = target.slotState = target.posState = target.chipUnitNumState = ECMD_TARGET_FIELD_WILDCARD;
  target.threadState = ECMD_TARGET_FIELD_UNUSED;
  if (depth == POS) {
    target.chipUnitNumState = ECMD_TARGET_FIELD_UNUSED;
    target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;
  } else if (depth == SLOT) {
    target.posState = ECMD_TARGET_FIELD_UNUSED;
    target.chipTypeState = ECMD_TARGET_FIELD_UNUSED;
    target.chipUnitNumState = ECMD_TARGET_FIELD_UNUSED;
    target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;
  } else if (depth == NODE) {
    target.slotState = ECMD_TARGET_FIELD_UNUSED;
    target.posState = ECMD_TARGET_FIELD_UNUSED;
    target.chipTypeState = ECMD_TARGET_FIELD_UNUSED;
    target.chipUnitNumState = ECMD_TARGET_FIELD_UNUSED;
    target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;
  } else if (depth == CAGE) {
    target.nodeState = ECMD_TARGET_FIELD_UNUSED;
    target.slotState = ECMD_TARGET_FIELD_UNUSED;
    target.posState = ECMD_TARGET_FIELD_UNUSED;
    target.chipTypeState = ECMD_TARGET_FIELD_UNUSED;
    target.chipUnitNumState = ECMD_TARGET_FIELD_UNUSED;
    target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;
  }

  /************************************************************************/
  /* Kickoff Looping Stuff                                                */
  /************************************************************************/
  rc = ecmdLooperInit(target, ECMD_SELECTED_TARGETS_LOOP, looperData);
  if (rc)
  {
    if (attributeData.faUint8ary != NULL) delete [] attributeData.faUint8ary;
    else if (attributeData.faInt8ary != NULL) delete [] attributeData.faInt8ary;
    if (attributeData.faUint16ary != NULL) delete [] attributeData.faUint16ary;
    else if (attributeData.faInt16ary != NULL) delete [] attributeData.faInt16ary;
    if (attributeData.faUint32ary != NULL) delete [] attributeData.faUint32ary;
    else if (attributeData.faInt32ary != NULL) delete [] attributeData.faInt32ary;
    if (attributeData.faUint64ary != NULL) delete [] attributeData.faUint64ary;
    else if (attributeData.faInt64ary != NULL) delete [] attributeData.faInt64ary;
    return rc;
  }

  while (ecmdLooperNext(target, looperData) && (!coeRc || coeMode)) {

    /* Actually go fetch the data */
    rc = fapi2SetAttribute(target, attributeId, attributeData);
    if (rc) {
      printed = "fapi2setattr - Error occured performing fapi2SetAttribute on ";
      printed += ecmdWriteTarget(target) + "\n";
      ecmdOutputError( printed.c_str() );
      coeRc = rc;
      continue;
    }
    else {
      validPosFound = true;
    }

    printed = ecmdWriteTarget(target) + "\n";

    // this if statement is needed, as fapi2AttributeDataToString() does not accept xl or xr as a valid format
    if ((format.compare("xl") == 0) || (format.compare("xr") == 0)) format = "x";

    std::string attributeDataString;
    rc = fapi2AttributeDataToString(attributeId, attributeData, attributeDataString, true, format.c_str());
    if (rc) {
      printed = "fapi2setattr - Error occured performing fapi2AttributeDataToString for ";
      printed += attributeName + "\n";
      ecmdOutputError( printed.c_str() );
      coeRc = rc;
      continue;
    }
    printed += attributeName + " = " + attributeDataString + "\n";
    ecmdOutput(printed.c_str());

  }

  if (attributeData.faUint8ary != NULL) delete [] attributeData.faUint8ary;
  else if (attributeData.faInt8ary != NULL) delete [] attributeData.faInt8ary;
  if (attributeData.faUint16ary != NULL) delete [] attributeData.faUint16ary;
  else if (attributeData.faInt16ary != NULL) delete [] attributeData.faInt16ary;
  if (attributeData.faUint32ary != NULL) delete [] attributeData.faUint32ary;
  else if (attributeData.faInt32ary != NULL) delete [] attributeData.faInt32ary;
  if (attributeData.faUint64ary != NULL) delete [] attributeData.faUint64ary;
  else if (attributeData.faInt64ary != NULL) delete [] attributeData.faInt64ary;

  // coeRc will be the return code from in the loop, coe mode or not.
  if (coeRc) return coeRc;

  // This is an error common across all UI functions
  if (!validPosFound) {
    ecmdOutputError("fapi2setattr - Unable to find a valid chip to execute command on\n");
    return ECMD_TARGET_NOT_CONFIGURED;
  }

  return rc;
}

uint32_t fapi2DumpAttributeUser(int argc, char * argv[])
{
    // fapi2dumpattr [chip[.chipunit]] [all | hwp | plat] [-f filename]
    // all is default
    uint32_t rc = ECMD_SUCCESS;

    ecmdChipTarget target;        ///< Current target
    std::string attributeName;    ///< Name of attribute to fetch
    bool validPosFound = false;   ///< Did we find something to actually execute on ?
    ecmdDataBuffer numData;       ///< Initialise data buffer with the numeric value
    std::string printed;          ///< Print Buffer
    ecmdLooperData looperData;    ///< Store internal Looper data
    char * dumpfilename = NULL;

    /* get format flag, if it's there */
    std::string format;
    char * formatPtr = ecmdParseOptionWithArgs(&argc, &argv, "-o");
    if (formatPtr == NULL) {
        format = "file";
    } else if( strcmp(formatPtr, "x") == 0 ) {
        // filex needed here due to how formatting is needed to be done for dump mode
        format = "filex";
    } else {
        format = formatPtr;
    }

    int CAGE = 1, NODE = 2, SLOT = 3, POS = 4, CHIPUNIT = 5;
    int depth = 0;                 ///< depth found from Command line parms

    uint32_t attributeSource = fapi2::FAPI_ATTRIBUTE_SOURCE_PLAT | fapi2::FAPI_ATTRIBUTE_SOURCE_HWP;

    if (ecmdParseOption(&argc, &argv, "-dk"))             depth = CAGE;
    else if (ecmdParseOption(&argc, &argv, "-dn"))        depth = NODE;
    else if (ecmdParseOption(&argc, &argv, "-ds"))        depth = SLOT;
    else if (ecmdParseOption(&argc, &argv, "-dp"))        depth = POS;
    else if (ecmdParseOption(&argc, &argv, "-dc"))        depth = CHIPUNIT;

    target.cageState = target.nodeState = target.slotState = target.chipTypeState = target.posState = ECMD_TARGET_FIELD_WILDCARD;
    target.chipUnitTypeState = ECMD_TARGET_FIELD_WILDCARD;
    target.chipUnitNumState = ECMD_TARGET_FIELD_WILDCARD;
    target.threadState = ECMD_TARGET_FIELD_UNUSED;

    dumpfilename = ecmdParseOptionWithArgs(&argc, &argv, "-f");

    /************************************************************************/
    /* Parse Common Cmdline Args                                            */
    /************************************************************************/
    rc = ecmdCommandArgs(&argc, &argv);
    if (rc) return rc;

    /************************************************************************/
    /* Parse Local ARGS here!                                               */
    /************************************************************************/
    //Setup the target that will be used to query the system config
    if (argc > 3)
    {
        ecmdOutputError("fapi2dumpattr - Too many arguments specified; you probably added an unsupported option.\n");
        ecmdOutputError("fapi2dumpattr - Type 'fapi2dumpattr -h' for usage.\n");
        return ECMD_INVALID_ARGS;
    }
    for (int argIndex = 0; argIndex < argc; argIndex++)
    {
        // need to parse chip and/or all|plat|hwp
        if (strcmp("all", argv[argIndex]) == 0)
        {
            attributeSource = fapi2::FAPI_ATTRIBUTE_SOURCE_PLAT | fapi2::FAPI_ATTRIBUTE_SOURCE_HWP;
        }
        else if (strcmp("plat", argv[argIndex]) == 0)
        {
            attributeSource = fapi2::FAPI_ATTRIBUTE_SOURCE_PLAT;
        }
        else if(strcmp("hwp", argv[argIndex]) == 0)
        {
            attributeSource = fapi2::FAPI_ATTRIBUTE_SOURCE_HWP;
        }
        else
        {
            std::string chipType, chipUnitType;
            rc = ecmdParseChipField(argv[argIndex], chipType, chipUnitType);
            if (rc)
            { 
                ecmdOutputError("fapi2dumpattr - Wildcard character detected however it is not supported by this command.\n");
                return rc;
            }

            /* Error check */
            if (depth)
            {
                    if (chipUnitType == "" && depth < POS)
                    {
                        ecmdOutputError("fapi2dumpattr - Invalid Depth parm specified when a chip was specified.  Try with -dp.\n");
                        return ECMD_INVALID_ARGS;
                    }

                    if (chipUnitType != "" && depth < CHIPUNIT)
                    {
                        ecmdOutputError("fapi2dumpattr - Invalid Depth parm specified when a chipUnit was specified.  Try with -dc.\n");
                        return ECMD_INVALID_ARGS;
                    }
            }
            else
            { /* No depth, set on for the code below */
                if (chipUnitType == "") {
                    depth = POS;
                } else {
                    depth = CHIPUNIT;
                }
            }
            target.chipType = chipType;
            target.chipTypeState = ECMD_TARGET_FIELD_VALID;
            target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;
            if (chipUnitType != "")
            {
                target.chipUnitType = chipUnitType;
                target.chipUnitTypeState = ECMD_TARGET_FIELD_VALID;
            }
        }
    }

    /* Now set our states based on depth */
    if (depth == POS)
    {
        target.chipUnitNumState = ECMD_TARGET_FIELD_UNUSED;
        target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;
    }
    else if (depth == SLOT)
    {
        target.chipTypeState = ECMD_TARGET_FIELD_UNUSED;
    }
    else if (depth == NODE)
    {
        target.slotState = ECMD_TARGET_FIELD_UNUSED;
    }
    else if (depth == CAGE)
    {
        target.nodeState = ECMD_TARGET_FIELD_UNUSED;
    }

    ecmdQueryData queryData;
    rc = ecmdQueryConfigSelected(target, queryData, ECMD_SELECTED_TARGETS_LOOP_DEFALL);
    if (rc) return rc;

    std::list<ecmdChipTarget> targetList;
    for (std::list<ecmdCageData>::iterator ecmdCurCage = queryData.cageData.begin(); ecmdCurCage != queryData.cageData.end(); ecmdCurCage++)
    {
        if (depth == 0 || depth == CAGE)
        {
            // print cage
            target.cage = ecmdCurCage->cageId;
            target.cageState = ECMD_TARGET_FIELD_VALID;
            target.nodeState = ECMD_TARGET_FIELD_UNUSED;
            target.slotState = ECMD_TARGET_FIELD_UNUSED;
            target.chipType = "";
            target.chipTypeState = ECMD_TARGET_FIELD_UNUSED;
            target.chipUnitType = "";
            target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;
            target.threadState = ECMD_TARGET_FIELD_UNUSED;
            targetList.push_back(target);
        }
        for (std::list<ecmdNodeData>::iterator ecmdCurNode = ecmdCurCage->nodeData.begin(); ecmdCurNode != ecmdCurCage->nodeData.end(); ecmdCurNode++)
        { 
            for (std::list<ecmdSlotData>::iterator ecmdCurSlot = ecmdCurNode->slotData.begin(); ecmdCurSlot != ecmdCurNode->slotData.end(); ecmdCurSlot++)
            {
                for (std::list<ecmdChipData>::iterator ecmdCurChip = ecmdCurSlot->chipData.begin(); ecmdCurChip != ecmdCurSlot->chipData.end(); ecmdCurChip++)
                {
                    if (depth == 0 || depth == POS)
                    {
                        //print chip
                        target.cage = ecmdCurCage->cageId;
                        target.cageState = ECMD_TARGET_FIELD_VALID;
                        target.node = ecmdCurNode->nodeId;
                        target.nodeState = ECMD_TARGET_FIELD_VALID;
                        target.slot = ecmdCurSlot->slotId;
                        target.slotState = ECMD_TARGET_FIELD_VALID;
                        target.chipType = ecmdCurChip->chipType;
                        target.chipTypeState = ECMD_TARGET_FIELD_VALID;
                        target.pos = ecmdCurChip->pos;
                        target.posState = ECMD_TARGET_FIELD_VALID;
                        target.chipUnitType = "";
                        target.chipUnitTypeState = target.chipUnitNumState = ECMD_TARGET_FIELD_UNUSED;
                        target.threadState = ECMD_TARGET_FIELD_UNUSED;
                        targetList.push_back(target);
                    }
                    for (std::list<ecmdChipUnitData>::iterator ecmdCurChipUnit = ecmdCurChip->chipUnitData.begin(); ecmdCurChipUnit != ecmdCurChip->chipUnitData.end(); ecmdCurChipUnit++)
                    {
                        if (depth == 0 || depth == CHIPUNIT)
                        {
                            // print chip unit
                            target.cage = ecmdCurCage->cageId;
                            target.cageState = ECMD_TARGET_FIELD_VALID;
                            target.node = ecmdCurNode->nodeId;
                            target.nodeState = ECMD_TARGET_FIELD_VALID;
                            target.slot = ecmdCurSlot->slotId;
                            target.slotState = ECMD_TARGET_FIELD_VALID;
                            target.chipType = ecmdCurChip->chipType;
                            target.chipTypeState = ECMD_TARGET_FIELD_VALID;
                            target.pos = ecmdCurChip->pos;
                            target.posState = ECMD_TARGET_FIELD_VALID;
                            target.chipUnitType = ecmdCurChipUnit->chipUnitType;
                            target.chipUnitTypeState = ECMD_TARGET_FIELD_VALID;
                            target.chipUnitNum = ecmdCurChipUnit->chipUnitNum;
                            target.chipUnitNumState = ECMD_TARGET_FIELD_VALID;
                            target.threadState = ECMD_TARGET_FIELD_UNUSED;
                            targetList.push_back(target);
                        }
                    }
                }
            }
        }
    }

    // open file to write if needed
    std::ofstream dumpfile;
    if (dumpfilename != NULL)
    {
        errno = 0;
        dumpfile.open(dumpfilename);
        if (dumpfile.fail())
        {
            printed = "fapi2dumpattr - Unable to open file (";
            printed += dumpfilename;
            printed += ") to write. ";
            printed += strerror(errno);
            printed += ".\n";
            ecmdOutputError(printed.c_str());
            return ECMD_INVALID_ARGS;
        }
    }

    std::map<fapi2::TargetType, std::list<fapi2::AttributeId> > attributeIds;
    std::list<fapi2::AttributeId> systemAttributeIds;

    std::map<fapi2::AttributeId, AttributeInfo> attributeInfoMap;

    fapi2::TargetType targetType = fapi2::TARGET_TYPE_NONE;
    for (std::list<ecmdChipTarget>::iterator targetIter = targetList.begin(); targetIter != targetList.end(); targetIter++)
    {
        rc = fapi2GetTargetType(*targetIter, targetType);
        if (rc)
        {
            // ERROR
            continue;
        }

        if (attributeIds.count(targetType) == 0)
        {
            fapi2GetAttrIdsByType(targetType, attributeSource, attributeIds[targetType]);
        }

        if ((attributeIds.count(targetType) != 0) && (!attributeIds[targetType].empty()))
        {
            printed = "\ntarget = " + ecmdWriteTarget(*targetIter, ECMD_DISPLAY_TARGET_HYBRID) + "\n";
            if (dumpfilename != NULL)
                dumpfile << printed;
            else
                ecmdOutput(printed.c_str());
            for (std::list<fapi2::AttributeId>::iterator curAttr = attributeIds[targetType].begin(); curAttr != attributeIds[targetType].end(); curAttr++)
            {
                std::map<fapi2::AttributeId, AttributeInfo>::iterator attributeInfoIter = attributeInfoMap.find(*curAttr);
                if (attributeInfoIter == attributeInfoMap.end())
                {
                    AttributeInfo tempAttributeInfo = {0, 0, 0, false};
                    /* Determine attribute type and size */
                    rc = fapi2GetAttrInfo(*curAttr, tempAttributeInfo.attributeType, tempAttributeInfo.numOfEntries, tempAttributeInfo.numOfBytes, tempAttributeInfo.attributeEnum);
                    if (rc) {
                        printed = "fapi2dumpattr - Unknown attribute type\n";
                        ecmdOutputError( printed.c_str() );
                    }
                    attributeInfoMap[*curAttr] = tempAttributeInfo;
                    attributeInfoIter = attributeInfoMap.find(*curAttr);
                }

                fapi2::AttributeData attributeData;
                attributeData.faValidMask = attributeInfoIter->second.attributeType;
                attributeData.faEnumUsed = attributeInfoIter->second.attributeEnum;

                if (attributeInfoIter->second.attributeType == FAPI_ATTRIBUTE_TYPE_UINT8ARY)
                {
                    attributeData.faUint8ary = new uint8_t[attributeInfoIter->second.numOfEntries];
                }
                else if (attributeInfoIter->second.attributeType == FAPI_ATTRIBUTE_TYPE_UINT16ARY)
                {
                    attributeData.faUint16ary = new uint16_t[attributeInfoIter->second.numOfEntries];
                }
                else if (attributeInfoIter->second.attributeType == FAPI_ATTRIBUTE_TYPE_UINT32ARY)
                {
                    attributeData.faUint32ary = new uint32_t[attributeInfoIter->second.numOfEntries];
                }
                else if (attributeInfoIter->second.attributeType == FAPI_ATTRIBUTE_TYPE_UINT64ARY)
                {
                    attributeData.faUint64ary = new uint64_t[attributeInfoIter->second.numOfEntries];
                }
                else if (attributeInfoIter->second.attributeType == FAPI_ATTRIBUTE_TYPE_INT8ARY)
                {
                    attributeData.faInt8ary = new int8_t[attributeInfoIter->second.numOfEntries];
                }
                else if (attributeInfoIter->second.attributeType == FAPI_ATTRIBUTE_TYPE_INT16ARY)
                {
                    attributeData.faInt16ary = new int16_t[attributeInfoIter->second.numOfEntries];
                }
                else if (attributeInfoIter->second.attributeType == FAPI_ATTRIBUTE_TYPE_INT32ARY)
                {
                    attributeData.faInt32ary = new int32_t[attributeInfoIter->second.numOfEntries];
                }
                else if (attributeInfoIter->second.attributeType == FAPI_ATTRIBUTE_TYPE_INT64ARY)
                {
                    attributeData.faInt64ary = new int64_t[attributeInfoIter->second.numOfEntries];
                }

                rc = fapi2GetAttribute(*targetIter, *curAttr, attributeData);
                if (rc) 
                {
                    printed = "fapi2dumpattr - Error occured performing fapi2GetAttribute on ";
                    printed += ecmdWriteTarget(*targetIter) + "\n";
                    ecmdOutputError( printed.c_str() );
                    continue;
                }
                else
                {
                    validPosFound = true;
                }

                std::string attributeDataString;
                rc = fapi2AttributeDataToString(*curAttr, attributeData, attributeDataString, true, format.c_str());
                if (rc)
                {
                    printed = "fapi2dumpattr - Error occured performing fapi2AttributeDataToString for ";
                    printed += attributeName + "\n";
                    ecmdOutputError( printed.c_str() );
                    continue;
                }
                printed = attributeDataString;
                if (dumpfilename != NULL)
                    dumpfile << printed;
                else
                    ecmdOutput(printed.c_str());

                if (attributeData.faUint8ary != NULL) delete [] attributeData.faUint8ary;
                else if (attributeData.faInt8ary != NULL) delete [] attributeData.faInt8ary;
                if (attributeData.faUint16ary != NULL) delete [] attributeData.faUint16ary;
                else if (attributeData.faInt16ary != NULL) delete [] attributeData.faInt16ary;
                if (attributeData.faUint32ary != NULL) delete [] attributeData.faUint32ary;
                else if (attributeData.faInt32ary != NULL) delete [] attributeData.faInt32ary;
                if (attributeData.faUint64ary != NULL) delete [] attributeData.faUint64ary;
                else if (attributeData.faInt64ary != NULL) delete [] attributeData.faInt64ary;
            }
        }
    }
    if (dumpfilename != NULL)
        dumpfile.close();

    // This is an error common across all UI functions
    if (!validPosFound) {
        ecmdOutputError("fapi2dumpattr - Unable to find a valid chip to execute command on\n");
        return ECMD_TARGET_NOT_CONFIGURED;
    }

    return rc;
}
