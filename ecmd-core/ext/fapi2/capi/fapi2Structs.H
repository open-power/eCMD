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

#ifndef fapi2Structs_h
#define fapi2Structs_h

/**
 * @file fapi2Structs.H
 * @brief fapi eCMD Extension Structures

 * Primary Extension Owner   : Kahn Evans
 * Secondary Extension Owner : Steven Janssen
*/

//--------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------
#include <string>


//--------------------------------------------------------------------
//  Forward References                                                
//--------------------------------------------------------------------

#define ECMD_FAPI2_CAPI_VERSION "1.0"          ///< eCMD FAPI2 Extension version



namespace fapi2
{

/**
 * @brief Enumeration of fapi file types
 */
typedef enum {
    FAPI_FILE_UNKNOWN,       ///< Default for not initialized
    FAPI_FILE_HWP
} FileType_t;


enum AttributeSource
{
    FAPI_ATTRIBUTE_SOURCE_UNKNOWN = 0x00000000,
    FAPI_ATTRIBUTE_SOURCE_PLAT = 0x00000001,
    FAPI_ATTRIBUTE_SOURCE_HWP = 0x00000002,
};


#define FAPI_ATTRIBUTE_TYPE_UINT8       0x40000000
#define FAPI_ATTRIBUTE_TYPE_UINT32      0x20000000
#define FAPI_ATTRIBUTE_TYPE_UINT64      0x10000000
#define FAPI_ATTRIBUTE_TYPE_UINT8ARY    0x04000000
#define FAPI_ATTRIBUTE_TYPE_UINT32ARY   0x02000000
#define FAPI_ATTRIBUTE_TYPE_UINT64ARY   0x01000000
#define FAPI_ATTRIBUTE_TYPE_INT8        0x00400000
#define FAPI_ATTRIBUTE_TYPE_INT32       0x00200000
#define FAPI_ATTRIBUTE_TYPE_INT64       0x00100000
#define FAPI_ATTRIBUTE_TYPE_INT8ARY     0x00040000
#define FAPI_ATTRIBUTE_TYPE_INT32ARY    0x00020000
#define FAPI_ATTRIBUTE_TYPE_INT64ARY    0x00010000
#define FAPI_ATTRIBUTE_TYPE_UINT16      0x08000000
#define FAPI_ATTRIBUTE_TYPE_UINT16ARY   0x00800000
#define FAPI_ATTRIBUTE_TYPE_INT16       0x00080000
#define FAPI_ATTRIBUTE_TYPE_INT16ARY    0x00008000

#define FAPI_ATTRIBUTE_MODE_CONST       0x80000000
#define FAPI_ATTRIBUTE_MODE_HOSTBOOT    0x40000000
#define FAPI_ATTRIBUTE_MODE_SCRATCH     0x20000000
#define FAPI_ATTRIBUTE_MODE_DIMS        0x10000000
/**
 @brief Used by the get/set configuration functions to return the data
*/
struct AttributeData {
  // Constructor
  AttributeData();

  // Destructor 
  ~AttributeData();

  // Methods  
#if !defined(ECMD_PYAPI)
  void printStruct(void);
#endif

  // Members
  uint32_t              faValidMask;  ///< Indicates which of the fields in this structure are valid
  char *                faString;     ///< If the output data is a char *, will be populated
#if !defined(ECMD_PERLAPI) && !defined(ECMD_PYAPI)
  union {
#endif
    uint8_t             faUint8;      ///< If the output data is a uint8_t, will be populated
    int8_t              faInt8;       ///< If the output data is a int8_t, will be populated
#if !defined(ECMD_PERLAPI) && !defined(ECMD_PYAPI)
  };
  union {
#endif
    uint16_t            faUint16;      ///< If the output data is a uint16_t, will be populated
    int16_t             faInt16;       ///< If the output data is a int16_t, will be populated
#if !defined(ECMD_PERLAPI) && !defined(ECMD_PYAPI)
  };
  union {
#endif
    uint32_t            faUint32;     ///< If the output data is a uint32_t, will be populated
    int32_t             faInt32;      ///< If the output data is a int32_t, will be populated
#if !defined(ECMD_PERLAPI) && !defined(ECMD_PYAPI)
  };
  union {
#endif
    uint64_t            faUint64;     ///< If the output data is a uint64_t, will be populated
    int64_t             faInt64;      ///< If the output data is a int64_t, will be populated
#if !defined(ECMD_PERLAPI) && !defined(ECMD_PYAPI)
  };
  union {
#endif
    uint8_t *           faUint8ary;   ///< If the output data is a uint8_t array, will be populated
    int8_t *            faInt8ary;    ///< If the output data is a int8_t array, will be populated
#if !defined(ECMD_PERLAPI) && !defined(ECMD_PYAPI) 
  };
  union {
#endif
    uint16_t *          faUint16ary;  ///< If the output data is a uint16_t array, will be populated
    int16_t *           faInt16ary;   ///< If the output data is a int16_t array, will be populated
#if !defined(ECMD_PERLAPI) && !defined(ECMD_PYAPI)
  };
  union {
#endif
    uint32_t *          faUint32ary;  ///< If the output data is a uint32_t array, will be populated
    int32_t *           faInt32ary;   ///< If the output data is a int32_t array, will be populated
#if !defined(ECMD_PERLAPI) && !defined(ECMD_PYAPI)
  };
  union {
#endif
    uint64_t *          faUint64ary;  ///< If the output data is a uint64_t array, will be populated
    int64_t *           faInt64ary;   ///< If the output data is a int64_t array, will be populated
#if !defined(ECMD_PERLAPI) && !defined(ECMD_PYAPI)
  };
#endif
  std::string           faName;       ///< Name of attribute (use sparingly).    
  bool                  faEnumUsed;   ///< Indicates if an enum was used or not
  uint8_t               faNumOfArrayDim; //< Indicates how many dimensions the array is (4 max currently)
  uint8_t               faSizeOfArrayDimW; //< Indicates how large dimension W is
  uint8_t               faSizeOfArrayDimX; //< Indicates how large dimension X is
  uint8_t               faSizeOfArrayDimY; //< Indicates how large dimension Y is
  uint8_t               faSizeOfArrayDimZ; //< Indicates how large dimension Z is
  uint32_t              faMode; //< Indicates a specific mode this attribute is in
};

inline AttributeData::AttributeData():
// mask
faValidMask(0x0), 
// char * 
faString(NULL),   
// integers
faUint8(0x0),         faUint16(0x0),       faUint32(0x0),       faUint64(0x0),  
// arrays
faUint8ary(NULL),     faUint16ary(NULL),      faUint32ary(NULL),   faUint64ary(NULL),
// name of attribute in string form
faName("blah"),       
// enum 
faEnumUsed(false), 
// array information
faNumOfArrayDim(0),
faSizeOfArrayDimW(0), faSizeOfArrayDimX(0),
faSizeOfArrayDimY(0), faSizeOfArrayDimZ(0),
faMode(0x0)
{}

inline AttributeData::~AttributeData() {}

} //namespace
#endif 





