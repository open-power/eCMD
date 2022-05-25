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

/**
 * @file fapi2ClientCapi.H
 * @brief Hardware Procedure Framework (fapi2) eCMD Extension

 * Primary Extension Owner   : Kahn Evans
 * Secondary Extension Owner : Steven Janssen

 @section fapi2 FAPI2 (Hardware Procedure Framework) Extension
 This extension provides HWPF fapi2 only interfaces.  
 <ul>
 <li> eCMD specific headers</li>
 <p><b>NOTE: These are needed in eCMD procedures ONLY not actual fapi procedures that will be consumed by FW</b></p>
 <ul>
 <li> fapi2ClientCapi.H</li>
 <li> fapi2SharedUtils.H</li>
 </ul>
 
 <li>Hardware Procedure Framework (HWPF) specific headers</li>
 <p><b>NOTE: fapi2.H includes all the headers below.  You should include the fapi2.H header in a fapi2 Hardware Procedure (HWP)</b></p>
 <ul>
 <li> fapi2.H </li>
 <ul>
 <li>target.H </li> 
 <li>return_code.H</li>
 <li>buffer.H</li>
 <li>hw_access.H</li>
 <li>utils.H</li>
 <li>plat_trace.H</li>
 <li>error_scope.H</li>
 <li>set_sbe_error.H</li>
 <li>fapi2_hwp_executor.H</li>
 <li>fapi2_attribute_service.H</li>
 <li>fapi2_chip_ec_feature.H</li>
 <li>mvpd_access.H</li>
 <li>mbvpdAccess.H</li>
 </ul>
 </ul>
 <li> Library : libfapi2.so</li>
 </ul>

*/

#ifndef fapi2ClientCapi_H 
#define fapi2ClientCapi_H

#include <stdint.h>
#include <list>
#include <fapi2Structs.H>  
#include <attribute_ids.H>

#include <ecmdStructs.H>  


//------------------------------------------------------------------------------
// HW Communication Functions
//------------------------------------------------------------------------------

extern "C" {

#if !defined(ECMD_PYAPI)
/** @name Load/Unload Functions (NOTE: Available only in eCMD HWPF FAPI extention implementation, not FW)*/
//@{
/**
 @brief Initialize the FAPI extention - This call is required by eCMD programs
 @retval FAPI_SUCCESS if successful
 @retval nonzero if unsuccessful
*/
 uint32_t fapi2InitExtension();  
//@}
#endif

/** @name Misc Functions (NOTE: Available only in eCMD HWPF FAPI extention implementation, not FW)*/
//@{
#if !defined(ECMD_PYAPI)
/**
 @brief Query the location of a specific HWP
 @param i_fileType Enum that specifies which type of file you are looking for (see fapiStructs.H)
 @param i_fileName Name of file you are looking for 
 @param o_fileLocation Return string with full path and filename to location 
 @param i_version String to specify a version (e.g., 1.1, 710.4, default).  "default" is default
 @retval ECMD_SUCCESS if successful
 @retval ECMD_UNKNOWN_FILE if unable to find requested file
 @retval nonzero if unsuccessful

*/
uint32_t fapi2QueryFileLocation(fapi2::FileType_t i_fileType, std::string & i_fileName, std::string & o_fileLocation, std::string i_version = "default");

/**
 @brief Retrieve the value of an Attribute Override Setting from attribute file or cache.
 @param i_target Struct that specifies the target to operate on
 @param i_id ID of attribute to query
 @param o_data Structure containing the valid data
 @retval ECMD_INVALID_CONFIG_NAME Name specified is not found
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful

 <b>
 NOTE: </b>
 <b>
   <ul>
   <li>Hardware procedures (HWP) should not call this API directly as it's only supported by eCMD, not FW. </li>
  </ul>
</b>

*/
uint32_t fapi2GetAttributeOverride(const ecmdChipTarget & i_target, const uint32_t i_id, fapi2::AttributeData & o_data);
#endif



/**
 @brief Retrieve the value of an Attribute Setting
 @param i_target Struct that specifies the target to operate on
 @param i_id ID of attribute to query
 @param o_data Structure containing the valid data
 @retval ECMD_INVALID_CONFIG_NAME Name specified is not valid
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful

 <b>
 NOTE: </b>
 <b>
   <ul>
   <li>Hardware procedures (HWP) should not call this API directly as it's only supported by eCMD, not FW. </li>
   <li>HWPs should call the FAPI_ATTR_GET macro to read an attribute (see fapiAttributeService.H)</li>
  </ul>
</b>

*/
uint32_t fapi2GetAttribute(const ecmdChipTarget & i_target, const uint32_t i_id, fapi2::AttributeData & o_data);


/**
 @brief Set the value of an Attribute 
 @param i_target Struct that specifies the target to operate on
 @param i_id ID of attribute to query
 @param i_data Structure containing the valid data
 @retval ECMD_INVALID_CONFIG_NAME Name specified is not valid
 @retval ECMD_TARGET_NOT_CONFIGURED if target is not available in the system
 @retval ECMD_SUCCESS if successful

 <b>
 NOTE </b>
 <b>
   <ul>
   <li>Hardware procedures (HWP) should not call this API directly as it's only supported by eCMD, not FW. </li>
   <li>HWPs should call the FAPI_ATTR_SET macro to set an attribute (see fapiAttributeService.H) </li>
   </ul>
 </b>
*/
uint32_t fapi2SetAttribute(const ecmdChipTarget & i_target, const uint32_t i_id, fapi2::AttributeData & i_data);

/**
 @brief Translate an AttributeData object to a string
 @param i_attrId ID of attribute that goes with data
 @param i_attrData Data that we want printed out
 @param o_attrDataString Data transformed into a string
 @param i_fullData If True full array data is printed. If false only size of array printed.
 @param i_format Requested format of data: enum, x, d, b, file
 @retval ECMD_SUCCESS if successful
 @retval ECMD_INVALID_ARGS if unknown format
*/
uint32_t fapi2AttributeDataToString(fapi2::AttributeId i_attrId, const fapi2::AttributeData & i_attrData, std::string & o_attrDataString, bool i_fullData, const char * i_format);

/**
 @brief Translate a string to an attribute id if one exists
 @param i_attrString String representation of the attribute id
 @param o_attrId ID of attribute that matches string
 @retval ECMD_SUCCESS if successful
*/
uint32_t fapi2AttributeStringToId(std::string i_attrString, fapi2::AttributeId & o_attrId);

/**
 @brief Retrieve the information about an Attribute
 @param i_attrId ID of attribute to query
 @param o_attrType Mask value for FAPI_ATTRIBUTE_TYPE_* used with fapi2::AttributeData
 @param o_numOfEntries Number of values this attribute stores
 @param o_numOfBytes Size in bytes required to store this attribute
 @param o_attrEnum True if attribute supports enumeration values
 @retval ECMD_SUCCESS if successful
*/
uint32_t fapi2GetAttrInfo(fapi2::AttributeId i_attrId, uint32_t & o_attrType, uint32_t & o_numOfEntries, uint32_t & o_numOfBytes, bool & o_attrEnum);

#if !defined(ECMD_PYAPI)
/**
 @brief Retrieve a list of attributes based upon their TargetType and AttributeSource
 @param i_targetTypes An ored value of requested TargetType
 @param i_attributeSources An ored value of requested AttributeSource
 @param o_attributeIds A list of AttributeId based upon the requested types
 @retval ECMD_SUCCESS if successful
*/
uint32_t fapi2GetAttrIdsByType(fapi2::TargetType i_targetTypes, uint32_t i_attributeSources, std::list<fapi2::AttributeId> & o_attributeIds);
#endif

//@}

/**
 @brief Given an AttributeData struct representing an attribute enum, returns the same struct, but with the appropriate faInt/faUint data member set, representing the enum value if found
 @param io_attrData An AttributeData struct, representing an attribute enum (see NOTE below for the required data members the caller must set)
 @retval ECMD_SUCCESS if successful
 @retval ECMD_FAPI2_UNKNOWN_TYPE_ERROR or another nonzero value if unsuccessful 

 NOTE: AttributeData.faString must be set to a char* value, representing the enum string you want the numeric value of<br>
 NOTE: AttributeData.faString can either be just the enum name (e.g. CACHE) or a string in the format ENUM_<attribute_name>_<enum_name> (e.g. ENUM_ATTR_CONTAINED_IPL_TYPE_CACHE)<br>
 NOTE: AttributeData.faName must be set to a std::string value, representing the name of the attribute the enum string is for<br>
 NOTE: AttributeData.faValidMask must be set to a uint32_t value, representing the type of the attribute the enum string is for<br>
 NOTE: For example, if AttributeData.faString = CACHE, AttributeData.faName = ATTR_CONTAINED_IPL_TYPE and AttributeData.faValidMask = FAPI_ATTRIBUTE_TYPE_UINT8,<br>
 NOTE: if successful, AttributeData.faUint8 would be set to 0x1, representing the int value of the enum CACHE<br>
 NOTE: If AttributeData.faValidMask = FAPI_ATTRIBUTE_TYPE_INT32, AttributeData.faInt32 would represent the enum value instead. Similar logic applies for all other types<br>
*/
uint32_t fapi2AttrEnumStrToEnumVal(fapi2::AttributeData & io_attrData);

#if !defined(ECMD_PYAPI)
/**
 @brief Output a message related to an error
 @param i_message String to output

*/
void fapi2OutputError(const char* i_message);

/**
 @brief Output a message labeled as info
 @param i_message String to output

*/
void fapi2OutputInfo(const char* i_message);

/**
 @brief Output a message labeled as important
 @param i_message String to output

*/
void fapi2OutputImportant(const char* i_message);

/**
 @brief Output a message labeled as debug
 @param i_message String to output

*/
void fapi2OutputDebug(const char* i_message);

/**
 @brief Output a message in the scan trace - matches fw's scan trace
 @param i_message String to output

*/
void fapi2OutputScanTrace(const char* i_message);

/**
 @brief Output a message labeled as manufacturing
 @param i_message String to output

*/
void fapi2OutputManufacturing(const char* i_message);

/**
 @brief Dynamically update the format flags to be compatible with 64-bit architectures.
 @param i_message Original string to check
 @param o_message String with modified format strings, if they were found
 @param i_num Maximum number of characters to be copied from i_message
 @retval value of o_message

*/
char* fapi2FixOutputFormat(char* o_message, const char* i_message, size_t i_num);

bool fapi2IsOutputInfoEnabled();
bool fapi2IsOutputDebugEnabled();
bool fapi2IsOutputManufacturingEnabled();

uint32_t fapi2GetTargetType(const ecmdChipTarget & i_target, fapi2::TargetType & o_targetType);

bool fapi2IsOutputLabEnabled();
void fapi2OutputLab(const char* i_message);

/**
 @brief Allows supporting new fuction without adding new interfaces
 @param io_args List of arguments, starting with char * array of function name,
   then uint32_t * with version
 @retval ECMD_FUNCTION_NOT_SUPPORTED if function name/version is not supported
 @retval ECMD_SUCCESS if function is found and executed
*/
uint32_t fapi2GeneralApi(std::list<void *> & io_args);
#endif

} //extern c

#endif
