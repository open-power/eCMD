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

                               
// Get rid of some annoying lint errors that aren't real - cje
//lint -e613 Possible use of null pointer, lint doesn't understand we use iv_NumBits to check length and pointer validity
//lint -e668 Possible passing of a null pointer, same thing as above
//lint -e527 Lint doesn't understand that iv_UserOwned is changed in shareBuffer

//----------------------------------------------------------------------
//  Includes
//----------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <inttypes.h>
#include <netinet/in.h> /* for htonl */
#include <fstream>
#include <iostream>
#if defined (_AIX) && defined (_LP64)
#include "/usr/include/zlib.h"
#else
#include <zlib.h>
#endif

#include <ecmdDefines.H>
#include <ecmdDataBuffer.H>

//----------------------------------------------------------------------
//  Constants
//----------------------------------------------------------------------
#define EDB_RANDNUM 0x12345678
#define EDB_RETURN_CODE 0

// New Constants for improved performance
#define MIN(x,y)            (((x)<(y))?x:y)
#define UNIT_SZ             32

#define RETURN_ERROR(i_rc) if ((iv_RealData != NULL) && (iv_RealData[EDB_RETURN_CODE] == 0)) { iv_RealData[EDB_RETURN_CODE] = i_rc; } return i_rc;
#define SET_ERROR(i_rc) if ((iv_RealData != NULL) && (iv_RealData[EDB_RETURN_CODE] == 0)) { iv_RealData[EDB_RETURN_CODE] = i_rc; }

//----------------------------------------------------------------------
//  Forward declarations
//----------------------------------------------------------------------
void * ecmdBigEndianMemCopy(void * dest, const void *src, size_t count);

// new declaration here for performance improvement
// This function does NOT do input checks and does NOT handle xstate
inline uint32_t ecmdFastInsert(uint32_t *i_target, const uint32_t * i_data, uint32_t i_targetStart, uint32_t i_len, uint32_t i_sourceStart); 


//----------------------------------------------------------------------
//  Inlined Functions used to improve Performance
//----------------------------------------------------------------------
inline /* leave this inlined */
uint32_t fast_mask32(int32_t i_pos, int32_t i_len) {
  /* generates an arbitrary 32-bit mask using two
   operations, not too shabby */

  static const uint32_t l_mask32[] = {
    0x00000000,
    0x80000000, 0xC0000000, 0xE0000000, 0xF0000000,
    0xF8000000, 0xFC000000, 0xFE000000, 0xFF000000,
    0xFF800000, 0xFFC00000, 0xFFE00000, 0xFFF00000,
    0xFFF80000, 0xFFFC0000, 0xFFFE0000, 0xFFFF0000,
    0xFFFF8000, 0xFFFFC000, 0xFFFFE000, 0xFFFFF000,
    0xFFFFF800, 0xFFFFFC00, 0xFFFFFE00, 0xFFFFFF00,
    0xFFFFFF80, 0xFFFFFFC0, 0xFFFFFFE0, 0xFFFFFFF0,
    0xFFFFFFF8, 0xFFFFFFFC, 0xFFFFFFFE, 0xFFFFFFFF,
  };
  return l_mask32[i_len] >> i_pos;
}

inline /* leave this inlined */
uint32_t fast_set32(uint32_t i_trg, int32_t i_pos, int32_t i_len) {
  return fast_mask32(i_pos, i_len) | i_trg;
}

inline /* leave this inlined */
uint32_t fast_clear32(uint32_t i_trg, int32_t i_pos, int32_t i_len) {
  return fast_mask32(i_pos, i_len) & ~i_trg;
}

inline /* leave this inlined */
uint8_t fast_reverse8(uint8_t data) {
  static const uint8_t reverse8[] = {
    0x00,0x80,0x40,0xc0,0x20,0xa0,0x60,0xe0,
    0x10,0x90,0x50,0xd0,0x30,0xb0,0x70,0xf0,
    0x08,0x88,0x48,0xc8,0x28,0xa8,0x68,0xe8,
    0x18,0x98,0x58,0xd8,0x38,0xb8,0x78,0xf8,
    0x04,0x84,0x44,0xc4,0x24,0xa4,0x64,0xe4,
    0x14,0x94,0x54,0xd4,0x34,0xb4,0x74,0xf4,
    0x0c,0x8c,0x4c,0xcc,0x2c,0xac,0x6c,0xec,
    0x1c,0x9c,0x5c,0xdc,0x3c,0xbc,0x7c,0xfc,
    0x02,0x82,0x42,0xc2,0x22,0xa2,0x62,0xe2,
    0x12,0x92,0x52,0xd2,0x32,0xb2,0x72,0xf2,
    0x0a,0x8a,0x4a,0xca,0x2a,0xaa,0x6a,0xea,
    0x1a,0x9a,0x5a,0xda,0x3a,0xba,0x7a,0xfa,
    0x06,0x86,0x46,0xc6,0x26,0xa6,0x66,0xe6,
    0x16,0x96,0x56,0xd6,0x36,0xb6,0x76,0xf6,
    0x0e,0x8e,0x4e,0xce,0x2e,0xae,0x6e,0xee,
    0x1e,0x9e,0x5e,0xde,0x3e,0xbe,0x7e,0xfe,
    0x01,0x81,0x41,0xc1,0x21,0xa1,0x61,0xe1,
    0x11,0x91,0x51,0xd1,0x31,0xb1,0x71,0xf1,
    0x09,0x89,0x49,0xc9,0x29,0xa9,0x69,0xe9,
    0x19,0x99,0x59,0xd9,0x39,0xb9,0x79,0xf9,
    0x05,0x85,0x45,0xc5,0x25,0xa5,0x65,0xe5,
    0x15,0x95,0x55,0xd5,0x35,0xb5,0x75,0xf5,
    0x0d,0x8d,0x4d,0xcd,0x2d,0xad,0x6d,0xed,
    0x1d,0x9d,0x5d,0xdd,0x3d,0xbd,0x7d,0xfd,
    0x03,0x83,0x43,0xc3,0x23,0xa3,0x63,0xe3,
    0x13,0x93,0x53,0xd3,0x33,0xb3,0x73,0xf3,
    0x0b,0x8b,0x4b,0xcb,0x2b,0xab,0x6b,0xeb,
    0x1b,0x9b,0x5b,0xdb,0x3b,0xbb,0x7b,0xfb,
    0x07,0x87,0x47,0xc7,0x27,0xa7,0x67,0xe7,
    0x17,0x97,0x57,0xd7,0x37,0xb7,0x77,0xf7,
    0x0f,0x8f,0x4f,0xcf,0x2f,0xaf,0x6f,0xef,
    0x1f,0x9f,0x5f,0xdf,0x3f,0xbf,0x7f,0xff,
  };
  return reverse8[data];
}

inline /* leave this inlined */
uint32_t fast_reverse32(uint32_t data) {
  return fast_reverse8((data & 0xFF000000) >> 24) |
    fast_reverse8((data & 0x00FF0000) >> 16) << 8 |
    fast_reverse8((data & 0x0000FF00) >> 8) << 16 |
    fast_reverse8(data & 0x000000FF) << 24;
}

//---------------------------------------------------------------------
//  Constructors
//---------------------------------------------------------------------
ecmdDataBuffer::ecmdDataBuffer()  // Default constructor
: ecmdDataBufferBase()
{
}

ecmdDataBuffer::ecmdDataBuffer(uint32_t i_numBits)
: ecmdDataBufferBase(i_numBits)
{
}

ecmdDataBuffer::ecmdDataBuffer(const ecmdDataBuffer& i_other) 
: ecmdDataBufferBase(i_other)
{
  if (i_other.iv_NumBits != 0) {
#ifndef REMOVE_SIM
    if (i_other.isXstateEnabled()) {
      /* enable my xstate */
      enableXstateBuffer();
      strncpy(iv_DataStr, i_other.iv_DataStr, iv_NumBits);
    }
#endif
  }
  /* else do nothing.  already have an empty buffer */

}

//---------------------------------------------------------------------
//  Destructor
//---------------------------------------------------------------------
ecmdDataBuffer::~ecmdDataBuffer()
{
#ifndef REMOVE_SIM
  if (iv_DataStr != NULL) {
    delete[] iv_DataStr;
    iv_DataStr = NULL;
  }
#endif
}

//---------------------------------------------------------------------
//  Public Member Function Specifications
//---------------------------------------------------------------------
uint32_t ecmdDataBuffer::clear() {
  uint32_t rc = ECMD_DBUF_SUCCESS;

  rc = ecmdDataBufferBase::clear();
  if (rc) { return rc; }

  if(!iv_UserOwned)
  {
      if (isBufferOptimizable()) {
        return ECMD_DBUF_SUCCESS;
      }
  }

#ifndef REMOVE_SIM
  if (iv_DataStr != NULL) {
    delete[] iv_DataStr;
    iv_DataStr = NULL;
  }
  iv_XstateEnabled = false;
#endif

  return rc;
}

uint32_t ecmdDataBuffer::setCapacity(uint32_t i_newCapacity) {
  uint32_t rc = ECMD_DBUF_SUCCESS;

  if(!iv_UserOwned) {
    ETRAC0("**** ERROR (ecmdDataBuffer) : Attempt to modify non user owned buffer size.");
    RETURN_ERROR(ECMD_DBUF_NOT_OWNER);
  }

  /* for case where i_newCapacity is 0 (like in unflatten) use iv_LocalData for iv_RealData */
  /* This allows for iv_Data, the header, and the tail to be setup right */
  if (iv_Capacity == 0) { 
      /* We are using iv_LocalData, so point iv_RealData to the start of that */
      iv_RealData = iv_LocalData;
  }

  /* only resize to make the capacity bigger */
  if (iv_Capacity < i_newCapacity) {
    iv_Capacity = i_newCapacity;

    /* Now setup iv_RealData */
    if (iv_Capacity <= 2) {
      /* We are using iv_LocalData, so point iv_RealData to the start of that */
      iv_RealData = iv_LocalData;
    } else {
      /* If we are going from <= 64 to > 64, there was no malloc done so can't do delete */
      if ((iv_RealData != NULL) && (iv_RealData != iv_LocalData)) {
        delete[] iv_RealData;
      }
      iv_RealData = NULL;

      iv_RealData = new uint32_t[iv_Capacity + EDB_ADMIN_TOTAL_SIZE]; 
      if (iv_RealData == NULL) {
        ETRAC0("**** ERROR : ecmdDataBuffer::setCapacity : Unable to allocate memory for new databuffer");
        RETURN_ERROR(ECMD_DBUF_INIT_FAIL);
      }
    }


#ifndef REMOVE_SIM
    if (iv_XstateEnabled) {
      if (iv_DataStr != NULL) {
        delete[] iv_DataStr;
      }
      iv_DataStr = NULL;

      iv_DataStr = new char[(iv_Capacity*32)+42];
      if (iv_DataStr == NULL) {
        ETRAC0("**** ERROR : ecmdDataBuffer::setCapacity : Unable to allocate Xstate memory for new databuffer");
        RETURN_ERROR(ECMD_DBUF_INIT_FAIL);
      }
    }
#endif
  }

  /* Now setup iv_Data to point into the offset inside of iv_RealData */
  iv_Data = iv_RealData + EDB_ADMIN_HEADER_SIZE;

  /* We are all setup, now init everything to 0 */
  /* We want to do this regardless of if the buffer was resized. */
  /* This function is meant to be a destructive operation */
  /* Ok, now setup the header, and tail */
  iv_RealData[EDB_RETURN_CODE] = 0; ///< Reset error code
  iv_RealData[getWordLength() + EDB_ADMIN_HEADER_SIZE] = EDB_RANDNUM;

  rc = flushTo0();
  if (rc) return rc;

  return rc;
}

uint32_t ecmdDataBuffer::growBitLength(uint32_t i_newNumBits) {
  uint32_t rc = ECMD_DBUF_SUCCESS;
  uint32_t prevwordsize;
  uint32_t prevbitsize;

  if(!iv_UserOwned) {
    ETRAC0("**** ERROR (ecmdDataBuffer::growBitLength) : Attempt to modify non user owned buffer size.");
    RETURN_ERROR(ECMD_DBUF_NOT_OWNER);
  }

  /* Maybe we don't need to do anything */
  if (iv_NumBits == i_newNumBits) {
    return rc;
  } else if (i_newNumBits < iv_NumBits) {
    /* You can't grow smaller, use shrink */
    ETRAC0("**** ERROR (ecmdDataBuffer::growBitLength) : Attempted to grow to a smaller size than current buffer size.");
    RETURN_ERROR(ECMD_DBUF_INVALID_ARGS);
  }

  /* We need to verify we have room to do this shifting */
  /* Set our new length */
  prevwordsize = getWordLength();
  prevbitsize = iv_NumBits;
  iv_NumBits = i_newNumBits;
  if (getWordLength() > iv_Capacity) {
    /* UhOh we are out of room, have to resize */
    uint32_t * tempBuf = new uint32_t[prevwordsize];
    if (tempBuf == NULL) {
      ETRAC0("**** ERROR : ecmdDataBuffer::growBitLength : Unable to allocate temp buffer");
      RETURN_ERROR(ECMD_DBUF_INIT_FAIL);
    }
    memcpy(tempBuf, iv_Data, prevwordsize * 4);

#ifndef REMOVE_SIM
    char* temp = NULL;
    if (iv_XstateEnabled) {
      temp = new char[(iv_Capacity*32)+42];
      if (temp == NULL) {
        ETRAC0("**** ERROR : ecmdDataBuffer::growBitLength : Unable to allocate temp X-State buffer");
        RETURN_ERROR(ECMD_DBUF_INIT_FAIL);
      }
      strncpy(temp, iv_DataStr, prevbitsize);
    }
#endif
    /* Now resize with the new capacity */
    rc = setCapacity(getWordLength());
    if (rc) {
      if (tempBuf) {
        delete[] tempBuf;
      }
      return rc;
    }

    /* Restore the data */
    ecmdBigEndianMemCopy(iv_Data, tempBuf, (prevbitsize + 7) / 8);
    delete[] tempBuf;

#ifndef REMOVE_SIM
    if (iv_XstateEnabled) {
      strncpy(iv_DataStr, temp, prevbitsize); // copy back into iv_DataStr
      delete[] temp;
    }
#endif

    /* Clear any odd bits in the byte */
    for (uint32_t idx = prevbitsize; (idx < iv_NumBits) && (idx % 8); idx ++) {
      clearBit(idx);
    }

  } else if (prevwordsize < getWordLength()) {
    /* We didn't have to grow the buffer capacity, but we did move into a new word(s) so clear that data space out */
    for (uint32_t idx = prevwordsize; idx < getWordLength(); idx++) {
      memset(&iv_Data[idx], 0, 4);  // Clear the word

#ifndef REMOVE_SIM
      if (iv_XstateEnabled) {
        memset(&(iv_DataStr[(idx * 32)]), '0', 32); /* init to 0 */
      }
#endif
    }
  }    

  /* Only reset this stuff if things have changed */
  if (prevwordsize != getWordLength()) {
    iv_RealData[EDB_RETURN_CODE] = 0;  // error state
    iv_RealData[getWordLength() + EDB_ADMIN_HEADER_SIZE] = EDB_RANDNUM;
  }

  return rc;
}


uint32_t ecmdDataBuffer::setBit(uint32_t i_bit) {
  uint32_t rc = ECMD_DBUF_SUCCESS;

  rc = ecmdDataBufferBase::setBit(i_bit);
  if (rc) { return rc; }

#ifndef REMOVE_SIM
  if (iv_XstateEnabled) {
    iv_DataStr[i_bit] = '1';
  }
#endif

  return rc;
}

uint32_t ecmdDataBuffer::setWord(uint32_t i_wordOffset, uint32_t i_value) {
  uint32_t rc = ECMD_DBUF_SUCCESS;

  rc = ecmdDataBufferBase::setWord(i_wordOffset, i_value);
  if (rc) { return rc; }

#ifndef REMOVE_SIM
  if (iv_XstateEnabled) {
    uint32_t startBit = i_wordOffset * 32;
    uint32_t mask = 0x80000000;
    for (int i = 0; i < 32; i++) {
      if (i_value & mask) {
        iv_DataStr[startBit+i] = '1';
      }
      else {
        iv_DataStr[startBit+i] = '0';
      }

      mask >>= 1;
    }
  }
#endif

  return rc;
}

uint32_t ecmdDataBuffer::setByte(uint32_t i_byteOffset, uint8_t i_value) {
  uint32_t rc = ECMD_DBUF_SUCCESS;

  rc = ecmdDataBufferBase::setByte(i_byteOffset, i_value);
  if (rc) { return rc; }

#ifndef REMOVE_SIM
  if (iv_XstateEnabled) {
    uint32_t startBit = i_byteOffset * 8;
    uint8_t mask = 0x80;
    for (int i = 0; i < 8; i++) {
      if (i_value & mask) {
        iv_DataStr[startBit+i] = '1';
      }
      else {
        iv_DataStr[startBit+i] = '0';
      }

      mask >>= 1;
    }
  }
#endif

  return rc;
}

uint32_t ecmdDataBuffer::setHalfWord(uint32_t i_halfwordoffset, uint16_t i_value) {
  uint32_t rc = ECMD_DBUF_SUCCESS;

  rc = ecmdDataBufferBase::setHalfWord(i_halfwordoffset, i_value);
  if (rc) { return rc; }

#ifndef REMOVE_SIM
  if (iv_XstateEnabled) {
    uint32_t startBit = i_halfwordoffset * 16;
    uint16_t mask = 0x8000;
    for (int i = 0; i < 16; i++) {
      if (i_value & mask) {
        iv_DataStr[startBit+i] = '1';
      }
      else {
        iv_DataStr[startBit+i] = '0';
      }
      
      mask >>= 1;
    }
  }
#endif
  return rc;
}

uint32_t ecmdDataBuffer::setDoubleWord(uint32_t i_doublewordoffset, uint64_t i_value) {
  uint32_t rc = ECMD_DBUF_SUCCESS;

  rc = ecmdDataBufferBase::setDoubleWord(i_doublewordoffset, i_value);
  if (rc) { return rc; }

#ifndef REMOVE_SIM
  if (iv_XstateEnabled) {
    uint32_t startBit = i_doublewordoffset * 64;
    int bits = 64;
    if (!((i_doublewordoffset*2)+1 >= getWordLength()) ) {
      bits = 32;
    }
#ifdef _LP64
    uint64_t mask = 0x8000000000000000ul;
#else
    uint64_t mask = 0x8000000000000000ull;
#endif
    for (int i = 0; i < bits; i++) {
      if (i_value & mask) {
        iv_DataStr[startBit+i] = '1';
      }
      else {
        iv_DataStr[startBit+i] = '0';
      }

      mask >>= 1;
    }
  }
#endif
  return rc;
}

uint32_t ecmdDataBuffer::clearBit(uint32_t i_bit) {
  uint32_t rc = ECMD_DBUF_SUCCESS;

  rc = ecmdDataBufferBase::clearBit(i_bit);
  if (rc) { return rc; }

#ifndef REMOVE_SIM
  if (iv_XstateEnabled) {
    iv_DataStr[i_bit] = '0';
  }
#endif

  return rc;
}


uint32_t ecmdDataBuffer::flipBit(uint32_t i_bit) {
  uint32_t rc = ECMD_DBUF_SUCCESS;
  if (i_bit >= iv_NumBits) {
    ETRAC2("**** ERROR : ecmdDataBuffer::flipBit: bit %d >= NumBits (%d)", i_bit, iv_NumBits);
    RETURN_ERROR(ECMD_DBUF_BUFFER_OVERFLOW);
  }
#ifndef REMOVE_SIM
  if ((iv_XstateEnabled) && this->hasXstate(i_bit, 1)) {
    ETRAC1("**** ERROR : ecmdDataBuffer::flipBit: cannot flip non-binary data at bit %d", i_bit);
    RETURN_ERROR(ECMD_DBUF_XSTATE_ERROR);
  }
#endif

  rc = ecmdDataBufferBase::flipBit(i_bit);

  return rc;
}

bool   ecmdDataBuffer::isBitSet(uint32_t i_bit) const {
  if (i_bit >= iv_NumBits) {
    ETRAC2("**** ERROR : ecmdDataBuffer::isBitSet: i_bit %d >= NumBits (%d)", i_bit, iv_NumBits);
    SET_ERROR(ECMD_DBUF_BUFFER_OVERFLOW);
    return false;
  }

#ifndef REMOVE_SIM
  if (iv_XstateEnabled) {
    if (iv_DataStr[i_bit] != '1' && iv_DataStr[i_bit] != '0') {
      ETRAC1("**** ERROR : ecmdDataBuffer::isBitSet: non-binary character detected in data at bit %d", i_bit);
      SET_ERROR(ECMD_DBUF_XSTATE_ERROR);
      return false;
    }
  }
#endif

  return ecmdDataBufferBase::isBitSet(i_bit);
}

bool   ecmdDataBuffer::isBitClear(uint32_t i_bit) const {
  if (i_bit >= iv_NumBits) {
    ETRAC2("**** ERROR : ecmdDataBuffer::isBitClear: i_bit %d >= NumBits (%d)", i_bit, iv_NumBits);
    SET_ERROR(ECMD_DBUF_BUFFER_OVERFLOW);
    return false;
  }

#ifndef REMOVE_SIM
  if (iv_XstateEnabled) {
    if (iv_DataStr[i_bit] != '1' && iv_DataStr[i_bit] != '0') {
      ETRAC0( "**** ERROR : ecmdDataBuffer::isBitClear: non-binary character detected in data string");
      SET_ERROR(ECMD_DBUF_XSTATE_ERROR);
      return false;
    }
  }
#endif

  return ecmdDataBufferBase::isBitClear(i_bit);
}


uint32_t ecmdDataBuffer::flushTo0() {
  uint32_t rc = ECMD_DBUF_SUCCESS;
  if (getWordLength() > 0) {
    ecmdDataBufferBase::flushTo0();
#ifndef REMOVE_SIM
    if (iv_XstateEnabled)
      rc = this->fillDataStr('0');
#endif
  }
  return rc;
}

uint32_t ecmdDataBuffer::flushTo1() {
  uint32_t rc = ECMD_DBUF_SUCCESS;
  if (getWordLength() > 0) {
    ecmdDataBufferBase::flushTo1();
#ifndef REMOVE_SIM
    if (iv_XstateEnabled)
      rc = this->fillDataStr('1');
#endif
  }
  return rc;
}

uint32_t ecmdDataBuffer::reverse() {
  uint32_t rc = ECMD_DBUF_SUCCESS;

  ecmdDataBufferBase::reverse();

#ifndef REMOVE_SIM   
  if (iv_XstateEnabled) {
    /* We have xstates, re-generate the binary data */
    strncpy(&(iv_DataStr[0]), (this->genBinStr()).c_str(), iv_NumBits);
  }
#endif

  return rc;
}

uint32_t ecmdDataBuffer::applyInversionMask(const ecmdDataBuffer & i_invMaskBuffer, uint32_t i_invByteLen) {
  return applyInversionMask(i_invMaskBuffer.iv_Data, (i_invMaskBuffer.getByteLength() < i_invByteLen) ? i_invMaskBuffer.getByteLength() : i_invByteLen);
}


uint32_t ecmdDataBuffer::applyInversionMask(const uint32_t * i_invMask, uint32_t i_invByteLen) {

  ECMD_NULL_PTR_CHECK(i_invMask);

  uint32_t rc = ECMD_DBUF_SUCCESS;

  rc = ecmdDataBufferBase::applyInversionMask(i_invMask, i_invByteLen);
  if (rc) return rc;

#ifndef REMOVE_SIM   
  /* Do the smaller of data provided or size of buffer */
  uint32_t wordlen = (i_invByteLen / 4) + 1 < getWordLength() ? (i_invByteLen / 4) + 1 : getWordLength();

  if (iv_XstateEnabled) {
    uint32_t xbuf_size = (i_invByteLen * 8) < iv_NumBits ? (i_invByteLen * 8) : iv_NumBits;
    uint32_t curbit = 0;

    for (uint32_t word = 0; word < wordlen; word ++) {
      for (uint32_t bit = 0; bit < 32; bit ++) {
        
        if (curbit >= xbuf_size) break;

        if (i_invMask[word] & (0x80000000 >> bit)) {
          if (iv_DataStr[curbit] == '0') iv_DataStr[curbit] = '1';
          else if (iv_DataStr[curbit] == '1') iv_DataStr[curbit] = '0';
        }
        curbit ++;
      }
    }
  }
#endif
  return rc;
}

inline uint32_t ecmdFastInsert(uint32_t *i_target, const uint32_t * i_data, uint32_t i_targetStart, uint32_t i_len, uint32_t i_sourceStart) {

  uint32_t rc = ECMD_DBUF_SUCCESS;

  do {
    const uint32_t * p_src = i_data + i_sourceStart / UNIT_SZ;
    uint32_t * p_trg = i_target + i_targetStart / UNIT_SZ;

    // "slop" = unaligned bits 
    int32_t src_slop = i_sourceStart % UNIT_SZ;
    int32_t trg_slop = i_targetStart % UNIT_SZ;
    // "shift" = amount of shifting needed for target alignment 
    int32_t shift = trg_slop - src_slop;

    int32_t cnt = i_len;

    // "cnt" = largest number of bits to be moved each pass 
    cnt = MIN(cnt, UNIT_SZ);
    cnt = MIN(cnt, UNIT_SZ - src_slop);
    cnt = MIN(cnt, UNIT_SZ - trg_slop);

    // generate the source mask only once 
    uint32_t mask = fast_mask32(src_slop, cnt);
    // read the source bits only once 
    uint32_t src_bits = *p_src & mask;

    // ideally (i << -1) would yield (i >> 1), but it
    //   doesn't, so we need an extra branch here 
    if (shift < 0) {
      shift = -shift;
      src_bits <<= shift;
      mask <<= shift;
    } else {
      src_bits >>= shift;
      mask >>= shift;
    }

    // clear source '0' bits in the target 
    *p_trg &= ~mask;
    // set source '1' bits in the target  
    *p_trg |= src_bits;


    i_sourceStart += cnt;
    i_targetStart += cnt;

    i_len -= cnt;
  } while (0 < i_len);

  return rc;  
}


uint32_t ecmdDataBuffer::insert(const ecmdDataBuffer &i_bufferIn, uint32_t i_targetStart, uint32_t i_len, uint32_t i_sourceStart) {
    return insertBase(i_bufferIn, i_targetStart, i_len, i_sourceStart);
}

uint32_t ecmdDataBuffer::insertBase(const ecmdDataBufferBase &i_bufferIn, uint32_t i_targetStart, uint32_t i_len, uint32_t i_sourceStart) {
  uint32_t rc = ECMD_DBUF_SUCCESS;    

  rc = ecmdDataBufferBase::insertBase(i_bufferIn, i_targetStart, i_len, i_sourceStart);
  if (rc) return rc;

#ifndef REMOVE_SIM   
  // If the input buffer has xstates enabled, we need to enable ours 
  if (i_bufferIn.iv_XstateEnabled) {
    enableXstateBuffer();
    if (i_targetStart+i_len <= iv_NumBits) {
      strncpy(&(iv_DataStr[i_targetStart]), (static_cast<const ecmdDataBuffer &>(i_bufferIn).genXstateStr(i_sourceStart, i_len)).c_str(), i_len);
    }
  } else if (iv_XstateEnabled) {
    // We have xstates, but the incoming buffer didn't, just grab the binary data 
    if (i_targetStart+i_len <= iv_NumBits) {
      strncpy(&(iv_DataStr[i_targetStart]), (static_cast<const ecmdDataBuffer &>(i_bufferIn).genBinStr(i_sourceStart, i_len)).c_str(), i_len);
    }
  }      
#endif  

  return rc;      
}

uint32_t ecmdDataBuffer::insert(const uint32_t * i_data, uint32_t i_targetStart, uint32_t i_len, uint32_t i_sourceStart) {      

  ECMD_NULL_PTR_CHECK(i_data);

  uint32_t rc = ECMD_DBUF_SUCCESS;
    
  rc = ecmdDataBufferBase::insert(i_data, i_targetStart, i_len, i_sourceStart);
  if (rc) return rc;

#ifndef REMOVE_SIM   
  if (iv_XstateEnabled) {
    /* We have xstates, generate the binary data */
    if (i_targetStart+i_len <= iv_NumBits) {
      strncpy(&(iv_DataStr[i_targetStart]), (this->genBinStr(i_sourceStart, i_len)).c_str(), i_len);
    }
  }
#endif

  return rc;
}

uint32_t ecmdDataBuffer::extract(ecmdDataBuffer& o_bufferOut, uint32_t i_start, uint32_t i_len) const {
    return extractBase(o_bufferOut, i_start, i_len);
}

uint32_t ecmdDataBuffer::extractBase(ecmdDataBufferBase& o_bufferOut, uint32_t i_start, uint32_t i_len) const {
  uint32_t rc = ECMD_DBUF_SUCCESS;

  rc = ecmdDataBufferBase::extractBase(o_bufferOut, i_start, i_len);
  if (rc) return rc;   

#ifndef REMOVE_SIM
  /* We have xstates, force the output buffer to have them as well */
  if (iv_XstateEnabled) {
    static_cast<ecmdDataBuffer &>(o_bufferOut).enableXstateBuffer();
    if (i_start+i_len <= iv_NumBits) {
      strncpy(o_bufferOut.iv_DataStr, (genXstateStr(i_start, i_len)).c_str(), i_len);
      o_bufferOut.iv_DataStr[i_len] = '\0';
    }
    /* Bufferout has xstates but we don't we still need to apply the binary data to it */
  } else if (o_bufferOut.iv_XstateEnabled) {
    if (i_start+i_len <= iv_NumBits) {
      strncpy(o_bufferOut.iv_DataStr, (genBinStr(i_start, i_len)).c_str(), i_len);
      o_bufferOut.iv_DataStr[i_len] = '\0';
    }
  }      
#endif

  return rc;
}

uint32_t ecmdDataBuffer::extract(uint32_t *o_data, uint32_t i_start, uint32_t i_len) const {

  ECMD_NULL_PTR_CHECK(o_data);

  uint32_t rc = ECMD_DBUF_SUCCESS;

  rc = ecmdDataBufferBase::extract(o_data, i_start, i_len);
  if (rc) return rc;

#ifndef REMOVE_SIM
  if (iv_XstateEnabled) {
    /* If we are using this interface and find Xstate data we have a problem */
    if (hasXstate(i_start, i_len)) {
      ETRAC0("**** WARNING : ecmdDataBuffer::extract: Cannot extract when non-binary (X-State) character      present\n");
      RETURN_ERROR(ECMD_DBUF_XSTATE_ERROR);
    }
  }
#endif
  return rc;
}

uint32_t ecmdDataBuffer::extract(uint16_t *o_data, uint32_t i_start, uint32_t i_len) const {

  ECMD_NULL_PTR_CHECK(o_data);

  uint32_t rc = ECMD_DBUF_SUCCESS;

  rc = ecmdDataBufferBase::extract(o_data, i_start, i_len);
  if (rc) return rc;

#ifndef REMOVE_SIM
  if (iv_XstateEnabled) {
    /* If we are using this interface and find Xstate data we have a problem */
    if (hasXstate(i_start, i_len)) {
      ETRAC0("**** WARNING : ecmdDataBuffer::extract: Cannot extract when non-binary (X-State) character present\n");
      RETURN_ERROR(ECMD_DBUF_XSTATE_ERROR);
    }
  }
#endif

  return rc;
}

uint32_t ecmdDataBuffer::extract(uint8_t * o_data, uint32_t i_start, uint32_t i_bitLen) const {

  ECMD_NULL_PTR_CHECK(o_data);

  uint32_t rc = ECMD_DBUF_SUCCESS;

  rc = ecmdDataBufferBase::extract(o_data, i_start, i_bitLen);
  if (rc) return rc;

#ifndef REMOVE_SIM
  if (iv_XstateEnabled) {
    /* If we are using this interface and find Xstate data we have a problem */
    if (hasXstate(i_start, i_bitLen)) {
      ETRAC0("**** WARNING : ecmdDataBuffer::extract: Cannot extract when non-binary (X-State) character      present\n");
      RETURN_ERROR(ECMD_DBUF_XSTATE_ERROR);
    }
  }
#endif

  return rc;
}


uint32_t ecmdDataBuffer::setOr(const ecmdDataBuffer& i_bufferIn, uint32_t i_startBit, uint32_t i_len) {
#ifndef REMOVE_SIM
  if (i_bufferIn.iv_XstateEnabled) enableXstateBuffer();
#endif
  return ecmdDataBufferBase::setOr(i_bufferIn, i_startBit, i_len);
}

uint32_t ecmdDataBuffer::setXor(const ecmdDataBuffer& i_bufferIn, uint32_t i_startBit, uint32_t i_len) {
#ifndef REMOVE_SIM
  if (i_bufferIn.iv_XstateEnabled) enableXstateBuffer();
#endif
  return ecmdDataBufferBase::setXor(i_bufferIn, i_startBit, i_len);
}

uint32_t ecmdDataBuffer::merge(const ecmdDataBuffer& i_bufferIn) {
#ifndef REMOVE_SIM
  if (i_bufferIn.iv_XstateEnabled) enableXstateBuffer();
#endif
  return ecmdDataBufferBase::merge(i_bufferIn);
}

uint32_t ecmdDataBuffer::setAnd(const ecmdDataBuffer& i_bufferIn, uint32_t i_startBit, uint32_t i_len) {
#ifndef REMOVE_SIM
  if (i_bufferIn.iv_XstateEnabled) enableXstateBuffer();
#endif
  return ecmdDataBufferBase::setAnd(i_bufferIn, i_startBit, i_len);
}


std::string ecmdDataBuffer::genHexLeftStr(uint32_t i_start, uint32_t i_bitLen) const {
  uint32_t rc = ECMD_DBUF_SUCCESS;

  int tempNumWords = (i_bitLen + 31) / 32;
  std::string ret;
  char cPtr[10];

  if (i_start+i_bitLen > iv_NumBits) {
    ETRAC3("**** ERROR : ecmdDataBuffer::genHexLeftStr: i_start %d + i_len %d >= NumBits (%d)", i_start, i_bitLen, iv_NumBits);
    SET_ERROR(ECMD_DBUF_BUFFER_OVERFLOW);
    return ret;
  } else if (i_start >= iv_NumBits) {
    ETRAC2("**** ERROR : ecmdDataBuffer::genHexLeftStr: i_start %d >= iv_NumBits (%d)", i_start, iv_NumBits);
    return ret;
  } else if (i_bitLen > iv_NumBits) {
    ETRAC2("**** ERROR : ecmdDataBuffer::genHexLeftStr: i_bitLen %d > iv_NumBits (%d)", i_bitLen, iv_NumBits);
    return ret;
  }

  if ((i_start==0) && (i_bitLen==iv_NumBits)){
    for (int w = 0; w < tempNumWords; w++) {      
      sprintf(cPtr, "%.8X", iv_Data[w]);
      ret.append(cPtr);
    }
  } else {
    ecmdDataBuffer tmpBuffer(tempNumWords*32);
    tmpBuffer.flushTo0();

    rc = ecmdFastInsert(tmpBuffer.iv_Data,this->iv_Data,0,i_bitLen, i_start);
    if (rc) return ret;

    for (int w = 0; w < tempNumWords; w++) {
      sprintf(cPtr, "%.8X", tmpBuffer.iv_Data[w]);
      ret.append(cPtr);
    }
  }

  int overCount = (int)(32*tempNumWords - i_bitLen) / 4;

  if (overCount > 0) {      
    ret.erase(ret.length() - overCount, ret.length()-1);
  }

#ifndef REMOVE_SIM
  // If we are using this interface and find Xstate data we have a problem 
  if ((iv_XstateEnabled) && hasXstate(i_start, i_bitLen)) {
    ETRAC0("**** WARNING : ecmdDataBuffer::genHexLeftStr: Cannot extract when non-binary (X-State) character present");
    SET_ERROR(ECMD_DBUF_XSTATE_ERROR);
  }
#endif

  return ret; 
}


std::string ecmdDataBuffer::genHexRightStr(uint32_t i_start, uint32_t i_bitLen) const {

  /* Do gen a hex right string, we just shift the data right to nibble align and then do a genHexLeft - tricky eh */
  int shiftAmt = i_bitLen % 4 ? 4 - (i_bitLen % 4) : 0;
  std::string ret;
  uint32_t rc = ECMD_DBUF_SUCCESS;

  if (i_start+i_bitLen > iv_NumBits) {
    ETRAC3("**** ERROR : ecmdDataBuffer::genHexRightStr: i_start %d + i_len %d >= NumBits (%d)", i_start, i_bitLen, iv_NumBits);
    SET_ERROR(ECMD_DBUF_BUFFER_OVERFLOW);
    return ret;
  }
  // more input checks done as part of shiftRightAndResize and genHexLeftStr()

  ecmdDataBuffer temp;
  temp.setBitLength(i_bitLen);
  extract(temp, i_start, i_bitLen);

  if (shiftAmt) {
    rc = temp.shiftRightAndResize(shiftAmt);
    if (rc) {
      return ret;
    }
  }
  ret = temp.genHexLeftStr();

#ifndef REMOVE_SIM
  /* If we are using this interface and find Xstate data we have a problem */
  if ((iv_XstateEnabled) && hasXstate(i_start, i_bitLen)) {
    ETRAC0("**** WARNING : ecmdDataBuffer::genHexRightStr: Cannot extract when non-binary (X-State) character present");
    SET_ERROR(ECMD_DBUF_XSTATE_ERROR);
  }
#endif
    
  return ret;
}

std::string ecmdDataBuffer::genBinStr(uint32_t i_start, uint32_t i_bitLen) const {

  int tempNumWords = (i_bitLen + 31) / 32;
  std::string ret;
  char* data = new char[i_bitLen + 1];
  bool createdTemp = false;
  uint32_t* tempData = NULL;
  uint32_t rc = ECMD_DBUF_SUCCESS;

  if (i_start+i_bitLen > iv_NumBits) {
    ETRAC3("**** ERROR : ecmdDataBuffer::genBinStr: i_start %d + i_len %d >= NumBits (%d)", i_start, i_bitLen, iv_NumBits);
    SET_ERROR(ECMD_DBUF_BUFFER_OVERFLOW);
    if (data) // make sure we have the buffer before running delete on it.
    {
        delete[] data;
    }
    return ret;
  }
  // more input checks done as part of extract()

  /* Let's see if we can get a perf bonus */
  if ((i_start == 0) && (i_bitLen == iv_NumBits))
    tempData = iv_Data;
  else {
    tempData = new uint32_t[tempNumWords];
    /* extract iv_Data */
    rc = this->extract(&tempData[0], i_start, i_bitLen);
    if (rc) {
      if (data) // make sure we have the buffer before running delete on it.
      {
          delete[] data;
      }

      if (tempData) // make sure we have the buffer before running delete on it.
      {
          delete[] tempData;
      }
      return ret;
    }
    createdTemp = true;
  }

  uint32_t mask = 0x80000000;
  int curWord = 0;

  for (uint32_t w = 0; w < i_bitLen; w++) {
    if (tempData[curWord] & mask) {
      data[w] = '1';
    }
    else {
      data[w] = '0';
    }

    mask >>= 1;

    if (!mask) {
      curWord++;
      mask = 0x80000000;
    }

  }
  /* Terminate this puppy */
  data[i_bitLen] = '\0';

  if (createdTemp)
    delete[] tempData;

  ret = data;
  delete[] data;

#ifndef REMOVE_SIM
  /* If we are using this interface and find Xstate data we have a problem */
  if ((iv_XstateEnabled) && hasXstate(i_start, i_bitLen)) {
    ETRAC0("**** WARNING : ecmdDataBuffer::genBinStr: Cannot extract when non-binary (X-State) character present");
    SET_ERROR(ECMD_DBUF_XSTATE_ERROR);
  }
#endif

  return ret;
}

std::string ecmdDataBuffer::genAsciiStr(uint32_t i_start, uint32_t i_bitLen) const {
  std::string ret;
  uint8_t tempByte;

  if ((i_start + i_bitLen) > iv_NumBits) {
    ETRAC3("**** ERROR : ecmdDataBuffer::genAsciiStr: start %d + bitLen %d >= NumBits (%d)", i_start, i_bitLen, iv_NumBits);
    SET_ERROR(ECMD_DBUF_BUFFER_OVERFLOW);
    return ret;
  } else if (i_start >= iv_NumBits) {
    ETRAC2("**** ERROR : ecmdDataBuffer::genAsciiStr: bit %d >= NumBits (%d)", i_start, iv_NumBits);
    SET_ERROR(ECMD_DBUF_BUFFER_OVERFLOW);
    return ret;
  } else if (i_bitLen > iv_NumBits) {
    ETRAC2("**** ERROR : ecmdDataBuffer::genAsciiStr: bitLen %d > NumBits (%d)", i_bitLen, iv_NumBits);
    SET_ERROR(ECMD_DBUF_BUFFER_OVERFLOW);
    return ret;
  } else if (i_bitLen == 0) {
    ret = "";
    return ret;
  }

  // Create a temp data buffer, then just loop through it calling getByte
  // We create this temp buffer so that we can get data the user wanted aligned on a byte boundary
  // If the user specfied an i_start of 2, it's necessary to get everything aligned
  ecmdDataBuffer asciiBuffer(i_bitLen);
  asciiBuffer.insert(*this, 0, i_bitLen, i_start);

  int numBytes = asciiBuffer.getByteLength();
  for (int i = 0; i < numBytes; i++) { /* byte loop */
    tempByte = asciiBuffer.getByte(i);  /* grab a byte */     
    if (tempByte < 32 || tempByte > 126) { /* decimal 32 == space, 127 == DEL */
      ret += ".";  /* non-printing: use a . */
    } else {
      ret += tempByte; /* convert to ascii */
    }  
  }

#ifndef REMOVE_SIM
  /* If we are using this interface and find Xstate data we have a problem */
  if ((iv_XstateEnabled) && hasXstate(i_start, i_bitLen)) {
    ETRAC0("**** WARNING : ecmdDataBuffer::genAsciiStr: Cannot extract when non-binary (X-State) character present");
    SET_ERROR(ECMD_DBUF_XSTATE_ERROR);
  }
#endif

  return ret;
}
std::string ecmdDataBuffer::genAsciiPrintStr(uint32_t i_start, uint32_t i_bitlen) const {

  // Call genAsciiStr to get the string, this will take care of all our error checking
  std::string ret = genAsciiStr(i_start, i_bitlen);

  // Now search through the buffer, dealing with special characters
  size_t linePos = 0;
  while (linePos != std::string::npos) {

    // Handle %
    linePos = ret.find("%", linePos);
    if (linePos != std::string::npos) {
      ret.insert(linePos, "%");
      linePos += 2; // Skip past what we just inserted
    }

  }

  return ret;
}

#ifndef REMOVE_SIM
std::string ecmdDataBuffer::genXstateStr(uint32_t i_start, uint32_t i_bitLen) const {
  std::string ret;
  if (!iv_XstateEnabled) {
    ETRAC0("**** ERROR : ecmdDataBuffer::genXstateStr: Xstate operation called on buffer without xstate's enabled");
    SET_ERROR(ECMD_DBUF_XSTATE_NOT_ENABLED);
    return ret;
  }
    

  if (i_start+i_bitLen > iv_NumBits) {
    ETRAC3("**** ERROR : ecmdDataBuffer::genXstateStr: i_start %d + i_len %d >= NumBits (%d)", i_start, i_bitLen, iv_NumBits);
    SET_ERROR(ECMD_DBUF_BUFFER_OVERFLOW);
    return ret;
  }

  char * copyStr = new char[i_bitLen + 4];
  strncpy(copyStr, &iv_DataStr[i_start], i_bitLen);
  copyStr[i_bitLen] = '\0';

  ret = copyStr;

  delete[] copyStr;

  return ret;
}
#endif /* REMOVE_SIM */

std::string ecmdDataBuffer::genHexLeftStr() const { return this->genHexLeftStr(0, iv_NumBits); }
std::string ecmdDataBuffer::genHexRightStr() const { return this->genHexRightStr(0, iv_NumBits); }
std::string ecmdDataBuffer::genBinStr() const { return this->genBinStr(0, iv_NumBits); }
std::string ecmdDataBuffer::genAsciiStr() const { return this->genAsciiStr(0, iv_NumBits); }
#ifndef REMOVE_SIM
std::string ecmdDataBuffer::genXstateStr() const { return this->genXstateStr(0, iv_NumBits); }
#endif /* REMOVE_SIM */

uint32_t ecmdDataBuffer::insertFromHexLeftAndResize(const char * i_hexChars, uint32_t i_start, uint32_t i_length) {

  ECMD_NULL_PTR_CHECK(i_hexChars);

  if (i_length == 0) {
    i_length = strlen(i_hexChars) * 4;
  }

  /* If the user left the data identifier on the front, remove it */
  if (i_start == 0 && (strlen(i_hexChars)> 3)) {
    if (i_hexChars[0] == '0' && i_hexChars[1] == 'x') {
      if (i_hexChars[2] == 'l') {
        i_hexChars += 3; // Advance the pointer past the special characters
        i_length -= 12;    // And shrink the bit length
      } else {
        i_hexChars += 2; // Advance the pointer two characters to bypass it
        i_length -= 8;     // And shrink the bit length
      }
    }
  }

  uint32_t rc = setBitLength(i_length);
  if (rc) return rc;

  return insertFromHexLeft(i_hexChars, i_start, i_length);
}

uint32_t ecmdDataBuffer::insertFromHexLeft(const char * i_hexChars, uint32_t i_start, uint32_t i_length) {

  ECMD_NULL_PTR_CHECK(i_hexChars);

  uint32_t rc = ECMD_DBUF_SUCCESS;
  uint32_t i;

  uint32_t bitlength = (i_length == 0 ? strlen(i_hexChars) * 4 : i_length);

  /* If the user left the data identifier on the front, remove it */
  if (i_start == 0 && (strlen(i_hexChars)> 3)) {
    if (i_hexChars[0] == '0' && i_hexChars[1] == 'x') {
      if (i_hexChars[2] == 'l') {
        i_hexChars += 3; // Advance the pointer past the special characters
        bitlength -= 12;    // And shrink the bit length
      } else {
        i_hexChars += 2; // Advance the pointer two characters to bypass it
        bitlength -= 8;     // And shrink the bit length
      }
    }
  }

  if (bitlength == 0) {
    /* They don't want anything inserted */
    return rc;
  }

  /* Allocate all the space requested by the user */
  /* We init it all to zero so if the input data is less than length, it's padded zero */
  uint32_t wordLength = (bitlength + 31) / 32;

  uint32_t * number_ptr = new uint32_t[wordLength];
  for (i = 0; i < wordLength; i++) {
    number_ptr[i] = 0x0;
  }

  uint32_t tmpb32 = 0x0;
  char nextOne[2];
  nextOne[1] = '\0';

  /* Now calcualte the number of nibbles we need to looper for */
  /* We can't exceed the length of the input string, so loop for whatever is less */
  uint32_t nibbles;
  if ((strlen(i_hexChars) * 4) < bitlength) {
    nibbles = strlen(i_hexChars);
  } else {
    nibbles = (bitlength + 3) / 4;
  }

  for (i = 0; i < nibbles; i++) {
    if ((i & 0xFFF8) == i)
      number_ptr[i>>3] = 0x0;
    if (!isxdigit(i_hexChars[i])) {
      delete[] number_ptr;        //@01a delete buffer to prevent memory leak.
      RETURN_ERROR(ECMD_DBUF_INVALID_DATA_FORMAT);
    }
    nextOne[0] = i_hexChars[i];
    tmpb32 = strtoul(nextOne, NULL, 16);
    number_ptr[i>>3] |= (tmpb32 << (28 - (4 * (i & 0x07))));
  }

  rc = this->insert(number_ptr, i_start, bitlength);

  delete[] number_ptr;

  return rc;
}

uint32_t ecmdDataBuffer::insertFromHexRightAndResize(const char * i_hexChars, uint32_t i_start, uint32_t i_length) {

  ECMD_NULL_PTR_CHECK(i_hexChars);

  if (i_length == 0) {
    i_length = strlen(i_hexChars) * 4;
  }

  /* If the user left the data identifier on the front, remove it */
  if (i_start == 0 && (strlen(i_hexChars)> 3)) {
    if (i_hexChars[0] == '0' && i_hexChars[1] == 'x' && i_hexChars[2] == 'r') {
      i_hexChars += 3; // Advance the pointer past the special characters
      i_length -= 12;  // And shrink the bit length
    }
  }

  uint32_t rc = setBitLength(i_length);
  if (rc) return rc;

  return insertFromHexRight(i_hexChars, i_start, i_length);
}

uint32_t ecmdDataBuffer::insertFromHexRight(const char * i_hexChars, uint32_t i_start, uint32_t i_expectedLength) {

  ECMD_NULL_PTR_CHECK(i_hexChars);

  uint32_t rc = ECMD_DBUF_SUCCESS;
  ecmdDataBuffer insertBuffer;
  uint32_t bitlength = (i_expectedLength == 0 ? strlen(i_hexChars) * 4 : i_expectedLength);

  /* If the user left the data identifier on the front, remove it */
  if (i_start == 0 && (strlen(i_hexChars)> 3)) {
    if (i_hexChars[0] == '0' && i_hexChars[1] == 'x' && i_hexChars[2] == 'r') {
      i_hexChars += 3; // Advance the pointer past the special characters
      bitlength -= 12;  // And shrink the bit length
    }
  }

  if (bitlength == 0) {
    /* They don't want anything inserted */
    return rc;
  }

  /* Number of valid nibbles */
  uint32_t nibbles = (bitlength + 3) / 4;

  /* If they provided us more data then we expect we will have to offset into the data to start reading */
  uint32_t dataOverFlowOffset = strlen(i_hexChars) > nibbles ? strlen(i_hexChars) - nibbles : 0;

  /* First we will insert it as if it is left aligned , then we will just shift it over */

  /* First we set to the length of actual data */
  insertBuffer.setBitLength(strlen(i_hexChars) * 4);

  /* We need to offset into i_hexChars if they provided us more data then we expect */
  rc = insertBuffer.insertFromHexLeft(&(i_hexChars[dataOverFlowOffset]), 0, strlen(i_hexChars) * 4);
  if (rc) return rc;

  /* Now we have to shiftRight to align if they didn't provide as much data as was expected */
  /* So our data becomes right aligned in a buffer of size expectedlength */
  if (strlen(i_hexChars) < nibbles) {
    rc = insertBuffer.shiftRightAndResize((nibbles - strlen(i_hexChars)) * 4);
    if (rc) return rc;
  }

  /* Now we have left aligned data, we just shift to right the odd bits of the nibble to align to the right */
  if (bitlength % 4) {
    rc = insertBuffer.shiftLeftAndResize(4 - (bitlength % 4));
    if (rc) return rc;
  }

  /* Now we have our data insert into ourselves */
  rc = this->insert(insertBuffer, i_start, bitlength);

  return rc;
}

uint32_t ecmdDataBuffer::insertFromBinAndResize (const char * i_binChars, uint32_t i_start, uint32_t i_length) {

  ECMD_NULL_PTR_CHECK(i_binChars);

  uint32_t rc = ECMD_DBUF_SUCCESS;
  if (i_length == 0) {
    i_length = strlen(i_binChars);
  }

  rc = setBitLength(i_length);
  if (rc) return rc;

  /* If the user left the data identifier on the front, remove it */
  if (i_start == 0 && (strlen(i_binChars)> 2)) {
    if (i_binChars[0] == '0' && i_binChars[1] == 'b') {
      i_binChars += 2; // Advance the pointer past the special characters
      i_length -= 2;   // And shrink the bit length
    }
  }

  return insertFromBin(i_binChars, i_start, i_length);
}

uint32_t ecmdDataBuffer::insertFromBin (const char * i_binChars, uint32_t i_start, uint32_t i_length) {

  ECMD_NULL_PTR_CHECK(i_binChars);

  uint32_t rc = ECMD_DBUF_SUCCESS;

  // input checking done with clearBit() and setBit()
  if (i_length == 0) {
    i_length = strlen(i_binChars);
  }

  /* If the user left the data identifier on the front, remove it */
  if (i_start == 0 && (strlen(i_binChars)> 2)) {
    if (i_binChars[0] == '0' && i_binChars[1] == 'b') {
      i_binChars += 2; // Advance the pointer past the special characters
      i_length -= 2;   // And shrink the bit length
    }
  }

  uint32_t l_binLength = strlen(i_binChars); //Need the length of the string to compare against i_length for padding/dropping data.
  
  for (uint32_t i = 0; i < i_length; i++) {
    if (i_binChars[i] == '0') 
    {
      rc = this->clearBit(i_start+i); if (rc) return rc;
    }
    else if (i_binChars[i] == '1') 
    {
      rc = this->setBit(i_start+i); if (rc) return rc;
    }
    // Pad with 0s if user requested larger length than binary string
    else if (i >= l_binLength)
    {
      rc = this->clearBit(i_start+i); if (rc) return rc;
    }
    else 
    {
      RETURN_ERROR(ECMD_DBUF_INVALID_DATA_FORMAT);
    }
  }

  return rc;
}

uint32_t ecmdDataBuffer::insertFromAsciiAndResize(const char * i_asciiChars, uint32_t i_start) {

  ECMD_NULL_PTR_CHECK(i_asciiChars);

  uint32_t rc = setBitLength(strlen(i_asciiChars)*8);
  if (rc) return rc;

  return insertFromAscii(i_asciiChars, i_start);
}

uint32_t ecmdDataBuffer::insertFromAscii(const char * i_asciiChars, uint32_t i_start) {

  ECMD_NULL_PTR_CHECK(i_asciiChars);

  /* We can just call insert on this for the bitLength of the char buffer */
  uint32_t bitLength = strlen(i_asciiChars)*8;
  return insert((uint8_t *)i_asciiChars, i_start, bitLength, 0);
}

uint32_t ecmdDataBuffer::copy(ecmdDataBuffer &o_newCopy) const {
  uint32_t rc = ECMD_DBUF_SUCCESS;

  rc = o_newCopy.setBitLength(iv_NumBits);
  
  if (!rc && iv_NumBits != 0) {
    // iv_Data
    memcpy(o_newCopy.iv_Data, iv_Data, getWordLength() * 4);
    // Error state
    o_newCopy.iv_RealData[EDB_RETURN_CODE] = iv_RealData[EDB_RETURN_CODE];
#ifndef REMOVE_SIM
    if (iv_XstateEnabled) {
      /* enable the xstate in the copy */
      o_newCopy.enableXstateBuffer();
      strncpy(o_newCopy.iv_DataStr, iv_DataStr, iv_NumBits);
    } else if (o_newCopy.iv_XstateEnabled) {
      /* disable the xstate in the copy */
      o_newCopy.disableXstateBuffer();
    }
#endif
  }
  return rc;

}

/* Copy Operator */
ecmdDataBuffer& ecmdDataBuffer::operator=(const ecmdDataBuffer & i_master) {
  uint32_t rc = ECMD_DBUF_SUCCESS;

  rc = setBitLength(i_master.iv_NumBits);

  if (!rc && iv_NumBits != 0) {
    // iv_Data
    memcpy(iv_Data, i_master.iv_Data, getWordLength() * 4);
    // Error state
    iv_RealData[EDB_RETURN_CODE] = i_master.iv_RealData[EDB_RETURN_CODE];
#ifndef REMOVE_SIM
    if (i_master.iv_XstateEnabled) {
      enableXstateBuffer();
      strncpy(iv_DataStr, i_master.iv_DataStr, i_master.iv_NumBits);
    } else if (iv_XstateEnabled) {
      disableXstateBuffer();
    }
#endif
  }
  return *this;
}

ecmdDataBuffer& ecmdDataBuffer::operator=(const ecmdDataBufferBase & i_master) {
  uint32_t rc = ECMD_DBUF_SUCCESS;

  rc = setBitLength(i_master.iv_NumBits);

  if (!rc && iv_NumBits != 0) {
    // iv_Data
    memcpy(iv_Data, i_master.iv_Data, getWordLength() * 4);
    // Error state
    iv_RealData[EDB_RETURN_CODE] = i_master.iv_RealData[EDB_RETURN_CODE];
  }
  return *this;
}


uint32_t ecmdDataBuffer::memCopyIn(const uint32_t* i_buf, uint32_t i_bytes) { /* Does a memcpy from supplied buffer into ecmdDataBuffer */

  ECMD_NULL_PTR_CHECK(i_buf);

  uint32_t rc = ECMD_DBUF_SUCCESS;

  rc = ecmdDataBufferBase::memCopyIn(i_buf, i_bytes);
  if (rc) return rc;

#ifndef REMOVE_SIM
  uint32_t cbytes = i_bytes < getByteLength() ? i_bytes : getByteLength();

  if (iv_XstateEnabled) {
    uint32_t mask = 0x80000000;
    uint32_t curWord = 0;

    for (uint32_t w = 0; w < cbytes*8; w++) {
      if (iv_Data[curWord] & mask) {
        iv_DataStr[w] = '1';
      }
      else {
        iv_DataStr[w] = '0';
      }

      mask >>= 1;

      if (!mask) {
        curWord++;
        mask = 0x80000000;
      }
    }
  }

#endif
  return rc;
}


#ifndef REMOVE_SIM
uint32_t ecmdDataBuffer::enableXstateBuffer() {
  uint32_t rc = ECMD_DBUF_SUCCESS;
  if(!iv_UserOwned)
  {
    ETRAC0("**** ERROR (ecmdDataBuffer::enableXstateBuffer) : Attempt to modify non user owned buffer size.");
    RETURN_ERROR(ECMD_DBUF_NOT_OWNER);
  }

  /* If it is already enabled, we don't do it again */
  if (iv_XstateEnabled) return rc;

  if (iv_NumBits > 0) {
    /* Check for null here to satisfy lint, but should always be NULL coming into this */
    if (iv_DataStr == NULL) iv_DataStr = new char[(iv_Capacity*32)+42];
    if (iv_DataStr == NULL) {
      ETRAC0("**** ERROR : ecmdDataBuffer::enableXstateBuffer : Unable to allocate Xstate memory for new databuffer");
      RETURN_ERROR(ECMD_DBUF_INIT_FAIL);
    }
    iv_DataStr[iv_NumBits] = '\0';

    /* Copy the raw data into the xstate buffer */
    uint32_t mask = 0x80000000;
    uint32_t curWord = 0;
    uint32_t numBits = getBitLength();

    for (uint32_t w = 0; w < numBits; w++) {
      if (iv_Data[curWord] & mask) {
        iv_DataStr[w] = '1';
      }
      else {
        iv_DataStr[w] = '0';
      }

      mask >>= 1;

      if (!mask) {
        curWord++;
        mask = 0x80000000;
      }
    }
  }
  iv_XstateEnabled = true;
  return rc;
}
#endif /* REMOVE_SIM */

#ifndef REMOVE_SIM
uint32_t ecmdDataBuffer::disableXstateBuffer() {
  uint32_t rc = ECMD_DBUF_SUCCESS;

  if(!iv_UserOwned)
  {
    ETRAC0("**** ERROR (ecmdDataBuffer::disableXstateBuffer) : Attempt to modify non user owned buffer size.");
    RETURN_ERROR(ECMD_DBUF_NOT_OWNER);
  }
  if (iv_DataStr != NULL) {
    delete[] iv_DataStr;
    iv_DataStr = NULL;
  }
  iv_XstateEnabled = false;
  return rc;

 }
#endif /* REMOVE_SIM */

#ifndef REMOVE_SIM
bool ecmdDataBuffer::isXstateEnabled() const {
 return iv_XstateEnabled;
}
#endif /* REMOVE_SIM */



#ifndef REMOVE_SIM
uint32_t ecmdDataBuffer::flushToX(char i_value) {
  uint32_t rc = ECMD_DBUF_SUCCESS;

  if (getWordLength() > 0) {
    memset(iv_Data, 0, getWordLength() * 4); /* init to 0 */
    rc = this->fillDataStr(i_value);
  }

  return rc;
}
#endif /* REMOVE_SIM */

#ifndef REMOVE_SIM
bool ecmdDataBuffer::hasXstate() const {
  if (!iv_XstateEnabled) {
    ETRAC0("**** ERROR : ecmdDataBuffer::hasXstate: Xstate operation called on buffer without xstate's enabled");
    SET_ERROR(ECMD_DBUF_XSTATE_NOT_ENABLED);
    return false;
  }

  return (hasXstate(0,iv_NumBits));
}
#endif /* REMOVE_SIM */

#ifndef REMOVE_SIM
bool   ecmdDataBuffer::hasXstate(uint32_t i_start, uint32_t i_length) const {
  if (!iv_XstateEnabled) {
    ETRAC0("**** ERROR : ecmdDataBuffer::hasXstate: Xstate operation called on buffer without xstate's enabled");
    SET_ERROR(ECMD_DBUF_XSTATE_NOT_ENABLED);
    return false;
  }

  uint32_t stopBit = i_start + i_length;
  uint32_t minStop = iv_NumBits < stopBit ? iv_NumBits : stopBit; /* pick the smallest */

  for (uint32_t i = i_start; i < minStop; i++) {
    if (iv_DataStr[i] != '0' && iv_DataStr[i] != '1')
      return true;
  }
  return false;
}
#endif /* REMOVE_SIM */

#ifndef REMOVE_SIM
char ecmdDataBuffer::getXstate(uint32_t i_bit) const {
  if (!iv_XstateEnabled) {
    ETRAC0("**** ERROR : ecmdDataBuffer::getXstate: Xstate operation called on buffer without xstate's enabled");
    SET_ERROR(ECMD_DBUF_XSTATE_NOT_ENABLED);
    return '0';
  }

  if (i_bit >= iv_NumBits) {
    ETRAC2("**** ERROR : ecmdDataBuffer::getXstate: bit %d >= NumBits (%d)", i_bit, iv_NumBits);
    SET_ERROR(ECMD_DBUF_XSTATE_ERROR);
    return '0';
  }
  return iv_DataStr[i_bit];
}
#endif /* REMOVE_SIM */

#ifndef REMOVE_SIM
uint32_t ecmdDataBuffer::setXstate(uint32_t i_bit, char i_value) {
  uint32_t rc = ECMD_DBUF_SUCCESS;

  if (!iv_XstateEnabled) {
    ETRAC0("**** ERROR : ecmdDataBuffer::setXstate: Xstate operation called on buffer without xstate's enabled");
    RETURN_ERROR(ECMD_DBUF_XSTATE_NOT_ENABLED);
  }

  if (i_bit >= iv_NumBits) {
    ETRAC2("**** ERROR : ecmdDataBuffer::setXstate: bit %d >= NumBits (%d)", i_bit, iv_NumBits);
    RETURN_ERROR(ECMD_DBUF_BUFFER_OVERFLOW);
  }

  if (i_value == '0') rc = clearBit(i_bit);
  else if (i_value == '1') rc = setBit(i_bit);
  else if (!isxdigit(i_value)) {
    /* We call clearbit to write the raw bit to 0 */
    rc = clearBit(i_bit);
    iv_DataStr[i_bit] = i_value;
  } else {
    ETRAC1("**** ERROR : ecmdDataBuffer::setXstate: unrecognized Xstate character: %c", i_value);
    RETURN_ERROR(ECMD_DBUF_XSTATE_ERROR);
  }

  return rc;
}
#endif /* REMOVE_SIM */

#ifndef REMOVE_SIM
uint32_t ecmdDataBuffer::setXstate(uint32_t i_bit, char i_value, uint32_t i_length) {
  uint32_t rc = ECMD_DBUF_SUCCESS;

  if (!iv_XstateEnabled) {
    ETRAC0("**** ERROR : ecmdDataBuffer::setXstate: Xstate operation called on buffer without xstates enabled");
    RETURN_ERROR(ECMD_DBUF_XSTATE_NOT_ENABLED);
  }

  if (i_bit + i_length > iv_NumBits) {
    ETRAC3("**** ERROR : ecmdDataBuffer::setXstate: bit %d + len %d > NumBits (%d)", i_bit, i_length, iv_NumBits);
    RETURN_ERROR(ECMD_DBUF_BUFFER_OVERFLOW);
  }
  
  for (uint32_t idx = 0; idx < i_length; idx ++) {
    rc |= this->setXstate(i_bit + idx, i_value);
  }

  return rc;
}
#endif /* REMOVE_SIM */

#ifndef REMOVE_SIM
uint32_t ecmdDataBuffer::setXstate(uint32_t i_bitOffset, const char* i_datastr) {

  ECMD_NULL_PTR_CHECK(i_datastr);

  uint32_t rc = ECMD_DBUF_SUCCESS;

  if (!iv_XstateEnabled) {
    ETRAC0("**** ERROR : ecmdDataBuffer::setXstate: Xstate operation called on buffer without xstates enabled");
    RETURN_ERROR(ECMD_DBUF_XSTATE_NOT_ENABLED);
  }

  uint32_t len = (uint32_t)strlen(i_datastr);

  if (i_bitOffset+len > iv_NumBits) {
    ETRAC3("**** ERROR : ecmdDataBuffer::setXstate: i_bitOffset %d + len %d > NumBits (%d)", i_bitOffset, len, iv_NumBits);
    RETURN_ERROR(ECMD_DBUF_BUFFER_OVERFLOW);
  }

  for (uint32_t i = 0; i < len; i++) {
    if (i_datastr[i] == '0') rc = clearBit(i_bitOffset+i);
    else if (i_datastr[i] == '1') rc = setBit(i_bitOffset+i);
    else if (!isxdigit(i_datastr[i])) {
      rc = clearBit(i_bitOffset+i);
      iv_DataStr[i_bitOffset+i] = i_datastr[i];
    } 
    else {
      ETRAC1("**** ERROR : ecmdDataBuffer::setXstate: unrecognized Xstate character: %c", i_datastr[i]);
      RETURN_ERROR(ECMD_DBUF_XSTATE_ERROR);
    }
  }

  return rc;
}
#endif /* REMOVE_SIM */


#ifndef REMOVE_SIM
uint32_t ecmdDataBuffer::memCopyInXstate(const char * i_buf, uint32_t i_bits) { /* Does a memcpy from supplied buffer into ecmdDataBuffer */

  ECMD_NULL_PTR_CHECK(i_buf);

  uint32_t rc = ECMD_DBUF_SUCCESS;

  /* cbytes is equal to the bit length of data */
  uint32_t cbytes = i_bits < getBitLength() ? i_bits : getBitLength();
  uint32_t index;

  if (!iv_XstateEnabled) {
    ETRAC0("**** ERROR : ecmdDataBuffer::memCopyInXstate: Xstate operation called on buffer without xstates enabled");
    RETURN_ERROR(ECMD_DBUF_XSTATE_NOT_ENABLED);
  }
  uint32_t buf_len = strlen(i_buf);
  if (buf_len < i_bits) {
    ETRAC2("**** ERROR : ecmdDataBuffer::memCopyInXstate: supplied buffer(%x) shorter than i_bits(%x)", buf_len, i_bits);
    RETURN_ERROR(ECMD_DBUF_INVALID_DATA_FORMAT);
  }

  /* Put the data into the Xstate array */
  strncpy(iv_DataStr, i_buf, cbytes);
  iv_DataStr[cbytes] = '\0';

  /* Now slide it over to the raw buffer */
  for (uint32_t bit = 0; bit < cbytes; bit++) {
    index = bit/32;
    if (i_buf[bit] == '1') {
      iv_Data[index] |= 0x00000001 << (31 - (bit-(index * 32)));
    } else {
      iv_Data[index] &= (~0x00000001 << (31 - (bit-(index * 32))));
    }
  }

  return rc;
}
#endif /* REMOVE_SIM */

#ifndef REMOVE_SIM
uint32_t ecmdDataBuffer::memCopyOutXstate(char * o_buf, uint32_t i_bits) const { /* Does a memcpy from ecmdDataBuffer into supplied buffer */

  ECMD_NULL_PTR_CHECK(o_buf);

  uint32_t rc = ECMD_DBUF_SUCCESS;
  uint32_t cbytes = i_bits < getBitLength() ? i_bits : getBitLength();
  if (!iv_XstateEnabled) {
    ETRAC0("**** ERROR : ecmdDataBuffer::memCopyOutXstate: Xstate operation called on buffer without xstates enabled");
    RETURN_ERROR(ECMD_DBUF_XSTATE_NOT_ENABLED);
  }

  strncpy(o_buf, iv_DataStr, cbytes);
  o_buf[cbytes] = '\0';

  return rc;
}
#endif /* REMOVE_SIM */



int ecmdDataBuffer::operator == (const ecmdDataBuffer& i_other) const {

  /* Check the length */
  uint32_t maxBits = 32;
  uint32_t numBits = getBitLength();
  uint32_t numToFetch = numBits < maxBits ? numBits : maxBits;
  uint32_t myData, otherData;
  uint32_t wordCounter = 0;

  if (getBitLength() != i_other.getBitLength()) {
    return 0;
  }

  if (getBitLength() == 0) /* two empty buffers are equal */
    return 1;

  /* Now run through the data */
  while (numToFetch > 0) {

    myData = iv_Data[wordCounter];
    otherData = i_other.iv_Data[wordCounter];

    if (numToFetch == maxBits) {
      if (myData != otherData) 
        return 0;
    }
    else {
      uint32_t mask = 0x80000000;
      for (uint32_t i = 0; i < numToFetch; i++, mask >>= 1) {
        if ( (myData & mask) != (otherData & mask) ) {
          return 0;
        }
      }
    }

    numBits -= numToFetch;
    numToFetch = (numBits < maxBits) ? numBits : maxBits;
    wordCounter++;
  }

#ifndef REMOVE_SIM
  /* Check the X-state buffer */
  if (iv_XstateEnabled && i_other.iv_XstateEnabled) {
    if (strncmp(iv_DataStr, i_other.iv_DataStr, iv_NumBits)) {
      return 0;
    }
  }
#endif

  /* Must have matched */
  return 1;
}

int ecmdDataBuffer::operator != (const ecmdDataBuffer& i_other) const {
  return !(*this == i_other);
}

ecmdDataBuffer ecmdDataBuffer::operator & (const ecmdDataBuffer& i_other) const {

  ecmdDataBuffer newItem = *this;

  if (iv_NumBits != i_other.iv_NumBits) {
    ETRAC2("**** ERROR : ecmdDataBuffer::operater &: NumBits in (%d) do not match NumBits (%d)", i_other.iv_NumBits, iv_NumBits);
    SET_ERROR(ECMD_DBUF_BUFFER_OVERFLOW);
  } else {
    newItem.setAnd(i_other.iv_Data, 0, iv_NumBits);
  }

  return newItem;
}

ecmdDataBuffer ecmdDataBuffer::operator | (const ecmdDataBuffer& i_other) const {

  ecmdDataBuffer newItem = *this;

  newItem.setOr(i_other.iv_Data, 0, iv_NumBits);

  return newItem;
}


//---------------------------------------------------------------------
//  Private Member Function Specifications
//---------------------------------------------------------------------
#ifndef REMOVE_SIM
uint32_t ecmdDataBuffer::fillDataStr(char i_fillChar) {
  if (!iv_XstateEnabled) {
    ETRAC0("**** ERROR : ecmdDataBuffer::fillDataStr: Xstate operation called on buffer without xstates enabled");
    RETURN_ERROR(ECMD_DBUF_XSTATE_NOT_ENABLED);
  }
  if (getWordLength() > 0) {
    memset(iv_DataStr, i_fillChar, iv_NumBits);
    iv_DataStr[iv_NumBits] = '\0';  
  }
  return ECMD_DBUF_SUCCESS;
}
#endif


uint32_t ecmdDataBuffer::writeFileMultiple(const char * i_filename, ecmdFormatType_t i_format, ecmdWriteMode_t i_mode, uint32_t & o_dataNumber, const char * i_facName) {

  ECMD_NULL_PTR_CHECK(i_filename);
  
  uint32_t rc = ECMD_DBUF_SUCCESS;
  std::ofstream ops;
  std::ifstream ins;
  uint32_t totalFileSz, begOffset=0, tmphdrfield=0, szcount = 0, curDBOffset=0, tableSz=0;
  uint32_t numBytes = getByteLength();
  uint32_t numBits = getBitLength();
  bool firstDBWrite = false;
  char *offsetTableData=NULL, propertyStr[201];
  ecmdFormatType_t existingFmt;
  std::string asciidatastr,xstatestr;
  uint32_t *buffer;
  
 //Check if format asked for is the same as what was used b4
  #ifndef REMOVE_SIM
  if (iv_XstateEnabled) {
    if((hasXstate()) && (i_format != ECMD_SAVE_FORMAT_XSTATE)) {
      ETRAC0( "**** ERROR : ecmdDataBuffer: writeFileMultiple: Buffer has Xstate data but non-xstate save mode requested");
      return(ECMD_DBUF_XSTATE_ERROR);
    }
  }
  #else
  if (i_format == ECMD_SAVE_FORMAT_XSTATE) {
    ETRAC0( "**** ERROR : ecmdDataBuffer: writeFileMultiple: FORMAT_XSTATE not supported in this configuration");
    return(ECMD_DBUF_XSTATE_ERROR);
  }    
  #endif
  
  
 if (i_format == ECMD_SAVE_FORMAT_BINARY_DATA ) {
   if (i_mode != ECMD_WRITE_MODE) {
    ETRAC0("**** ERROR : File Format ECMD_SAVE_FORMAT_BINARY_DATA only supported in ECMD_WRITE_MODE");
    RETURN_ERROR(ECMD_DBUF_INVALID_ARGS);
   } else if (i_facName != NULL) {
    ETRAC0("**** ERROR : File Format ECMD_SAVE_FORMAT_BINARY_DATA does not support storing of string-property");
    RETURN_ERROR(ECMD_DBUF_INVALID_ARGS);
   }
 }

  if ((i_facName != NULL) && (strlen(i_facName) > 200)) {
    ETRAC0("**** ERROR : Input FacName string of length greater than 200 bytes not supported");
    RETURN_ERROR(ECMD_DBUF_INVALID_ARGS);
  } 
  if (i_facName != NULL) tmphdrfield = 0x80000000; // set the string property bit
  
  //Open file for Read if it exists
  ins.open(i_filename);
    
  if ((!ins.fail()) && (i_mode != ECMD_WRITE_MODE) && (i_format != ECMD_SAVE_FORMAT_BINARY_DATA)) {
    ins.seekg(0, std::ios::end);
    totalFileSz = ins.tellg();
    if (totalFileSz == 0) {
      firstDBWrite = true;
    }
    else {
      ins.seekg(totalFileSz-8);//goto the BEGIN of the offset table
      ins.read((char *)&begOffset,4); begOffset = htonl(begOffset);
      ins.read((char *)&existingFmt,4); existingFmt = (ecmdFormatType_t)htonl(existingFmt);
      if (existingFmt != i_format) {
        ETRAC0("**** ERROR : Format requested does not match up with the file Format.");
    RETURN_ERROR(ECMD_DBUF_INVALID_ARGS);
      }
      ins.seekg(begOffset);
      offsetTableData = new char[totalFileSz-begOffset];
      //Read the offset Table starting from BEGIN till before the END
      ins.read(offsetTableData, totalFileSz-begOffset-12);//Take off the 4byte END,4 Byte Beg Offset, 4byte format
      if (ins.fail()) {
       ETRAC1("**** ERROR : Read of the offset table failed on file : %s",i_filename);
       if (offsetTableData != NULL) delete[] offsetTableData;
       RETURN_ERROR(ECMD_DBUF_FILE_OPERATION_FAIL); 
      }
      tableSz = totalFileSz-begOffset-12;
    }
    ins.close();
  } else { firstDBWrite = true;}
  
  //Open file for read/write
  if (i_mode == ECMD_APPEND_MODE) {
   if (!firstDBWrite) {
    ops.open(i_filename,  std::ios_base::in|std::ios_base::out );
   }
   else {
    ops.open(i_filename,  std::ios_base::out );
   }
  } else if(i_mode == ECMD_WRITE_MODE) {
    ops.open(i_filename, std::ofstream::out | std::ofstream::trunc);
  }
  if (ops.fail()) {
    ETRAC1("**** ERROR : Unable to open file : %s for write",i_filename);
    if (offsetTableData != NULL) delete[] offsetTableData;
    RETURN_ERROR(ECMD_DBUF_FOPEN_FAIL);  
  }
  
  if (!firstDBWrite) {
    ops.seekp(begOffset);//Write the New Buffer after the last buffer, over the current Offset Table 
  }
  
  curDBOffset = ops.tellp(); curDBOffset = htonl(curDBOffset); //Get the pointer to the current data 
  
  //Write Header
  if (i_format != ECMD_SAVE_FORMAT_BINARY_DATA) {
    if (i_format == ECMD_SAVE_FORMAT_BINARY) {
      //BIN Hdr
      ops.write("START",8);
      numBits     = htonl(numBits);          ops.write((char *)&numBits,4);//Bit length field
      i_format    = (ecmdFormatType_t)htonl(i_format);   ops.write((char*)&i_format,4);//Format field
      tmphdrfield = htonl(tmphdrfield);          ops.write((char*)&tmphdrfield,4);//Leave 1 words extra room here
      if (i_facName != NULL) {
        memset(propertyStr, '\0', 201);
        strcpy(propertyStr, i_facName);
    ops.write(propertyStr, 200); // String associated with the data buffer
      }
    }
    else {
      //ASCII or XSTATE Hdr
      ops << "START";
      char tmpstr[9]; //@01c Bumped to 9 to account for NULL terminator
      sprintf(tmpstr,"%08X",numBits); ops.write(tmpstr,8);
      sprintf(tmpstr,"%08X",i_format); ops.write(tmpstr,8);
      sprintf(tmpstr,"%08X",tmphdrfield); ops.write(tmpstr,8);
      ops << "\n";
      if (i_facName != NULL) {
       ops << (i_facName); // String associated with the data buffer
       ops << "\n";
      }
    }
    if (ops.fail()) {
     ETRAC1("**** ERROR : Write of the header failed on file : %s",i_filename);
    if (offsetTableData != NULL) delete[] offsetTableData;
     RETURN_ERROR(ECMD_DBUF_FILE_OPERATION_FAIL); 
    }
  }
  //Write DataBuffer
  if ( i_format == ECMD_SAVE_FORMAT_BINARY) {
    //Write Data
    buffer = new uint32_t[getWordLength()];
    memCopyOut(buffer, numBytes);
    //Convert into Network Byte order-Big Endian before writing
    int len=0;
    for(uint32_t i=0; i< getWordLength(); i++) {
     buffer[i]=htonl(buffer[i]);
     len = 4;
     if ( i == (getWordLength()-1)) {
       len = numBytes - (i*4);
     }
     ops.write((char *)&buffer[i],len);
    }
    delete[] buffer; buffer = NULL;
    ops.write("END",3);//Key
    if (ops.fail()) {
     ETRAC1("**** ERROR : Write operation in format ECMD_SAVE_FORMAT_BINARY failed on file : %s",i_filename);
     if (offsetTableData != NULL) delete[] offsetTableData;
     RETURN_ERROR(ECMD_DBUF_FILE_OPERATION_FAIL); 
    }
  }  
  else if ( i_format == ECMD_SAVE_FORMAT_ASCII) {
   while ((uint32_t)szcount < numBits) {
     if((uint32_t)szcount+32 < numBits) {
      asciidatastr = genHexLeftStr(szcount, 32);
     } else {
      asciidatastr = genHexLeftStr(szcount, numBits-(szcount));
     }
     if (szcount !=0 ) {
      if((szcount % 256) == 0) {
    ops << "\n"; 
      }
      else if((szcount % 32) == 0) {
    ops << " ";
      }
     }
     ops << asciidatastr.c_str();
     szcount+= 32;
   }
   ops << "\nEND\n";
   if (ops.fail()) {
     ETRAC1("**** ERROR : Write operation in format ECMD_SAVE_FORMAT_ASCII failed on file : %s",i_filename);
     if (offsetTableData != NULL) delete[] offsetTableData;
     RETURN_ERROR(ECMD_DBUF_FILE_OPERATION_FAIL); 
   }
  } 
#ifndef REMOVE_SIM
  else if ( i_format == ECMD_SAVE_FORMAT_XSTATE) {
  if (!iv_XstateEnabled) {
    ETRAC0("**** ERROR : ecmdDataBuffer::getXstate: Xstate operation called on buffer without xstates enabled");
    if (offsetTableData != NULL) delete[] offsetTableData;
    RETURN_ERROR(ECMD_DBUF_XSTATE_NOT_ENABLED);
  }
   while ((uint32_t)szcount < numBits) {
     if((uint32_t)szcount+64 < numBits) {
      xstatestr = genXstateStr(szcount, 64) ;
     } else {
      xstatestr = genXstateStr(szcount, numBits-(szcount)) ;
     }
     ops << xstatestr.c_str();
     if ((uint32_t)szcount+64<numBits)
      ops << "\n";
     szcount+= 64;
   }
   ops << "\nEND\n";
   if (ops.fail()) {
     ETRAC1("**** ERROR : Write operation in format ECMD_SAVE_FORMAT_XSTATE failed on file : %s",i_filename);
     if (offsetTableData != NULL) delete[] offsetTableData;
     RETURN_ERROR(ECMD_DBUF_FILE_OPERATION_FAIL); 
   }
  }
#endif
  else if (i_format == ECMD_SAVE_FORMAT_BINARY_DATA) {
    buffer = new uint32_t[getWordLength()];
    memCopyOut(buffer, numBytes);
    int len=0;
    //Convert into Network Byte order-Big Endian before writing
    for(uint32_t i=0; i< getWordLength(); i++) {
     buffer[i]=htonl(buffer[i]);
     if (((i+1)*4) > numBytes) {
      len = numBytes - (i*4);
     } else {
       len = 4;
     }
     ops.write((char *)&buffer[i],len);
    }
    delete[] buffer; buffer = NULL;
    if (ops.fail()) {
     ETRAC1("**** ERROR : Write operation in format ECMD_SAVE_FORMAT_BINARY_DATA failed on file : %s",i_filename);
     if (offsetTableData != NULL) delete[] offsetTableData;
     RETURN_ERROR(ECMD_DBUF_FILE_OPERATION_FAIL); 
    }
    ops.close();
  } 
  
  //Write the Offset Table Back with the new offset
  if (i_format != ECMD_SAVE_FORMAT_BINARY_DATA) {
     begOffset = ops.tellp();  begOffset = htonl(begOffset);
     if (!firstDBWrite) {
      ops.write(offsetTableData, tableSz);
      //Get the size of Offset Table after subtracting 8Byte BEGIN
      o_dataNumber = (tableSz - 8)/4; //Each Databuffer offset is a uint32_t (4 bytes)
     }
     else {
      ops.write("BEGIN",8);
      o_dataNumber = 0;
     }
     ops.write((char *)&curDBOffset, sizeof(curDBOffset));
     ops.write("END",4);
     ops.write((char *)&begOffset, sizeof(begOffset));
     i_format = (ecmdFormatType_t)htonl(i_format);   ops.write((char*)&i_format,4);
     ops.close();
  }
  
  if (offsetTableData != NULL) delete[] offsetTableData;
  return(rc);
}

uint32_t ecmdDataBuffer::writeFile(const char * filename, ecmdFormatType_t format, const char * i_facName) {
  ecmdWriteMode_t mode = ECMD_WRITE_MODE;
  uint32_t dataNumber;
  return this->writeFileMultiple(filename, format, mode, dataNumber, i_facName) ;
}

uint32_t ecmdDataBuffer::writeFileStream(std::ostream & o_filestream) {
  uint32_t rc = ECMD_DBUF_SUCCESS;
  uint32_t *buffer;
  uint32_t numBytes = getByteLength();
  
  buffer = new uint32_t[getWordLength()];
  rc = memCopyOut(buffer, numBytes);
  if (rc){ 
    delete [] buffer;
    buffer = NULL;
    return rc;
  }
  
  int len=0;
  //Convert into Network Byte order-Big Endian before writing
  for(uint32_t i=0; i < getWordLength(); i++) {
   buffer[i] = htonl(buffer[i]);
   if (((i+1)*4) > numBytes) {
    len = numBytes - (i*4);
   } else {
     len = 4;
   }
   o_filestream.write((char *)&buffer[i],len);
   if (o_filestream.fail()) {
      ETRAC0("**** ERROR : Write operation failed.");
      delete[] buffer;
      RETURN_ERROR(ECMD_DBUF_FILE_OPERATION_FAIL); 
   }

  }
  delete[] buffer;
  return rc;
}

uint32_t ecmdDataBuffer::queryNumOfBuffers(const char * i_filename, ecmdFormatType_t i_format, uint32_t &o_num) {

  ECMD_NULL_PTR_CHECK(i_filename);

  uint32_t rc = ECMD_DBUF_SUCCESS;
  std::ifstream ins;
  uint32_t endOffset = 0, totalFileSz=0;
  bool endFound = false;
  char endKeyword[4];
  
  ins.open(i_filename);
    
  if (ins.fail()) {
    ETRAC1("**** ERROR : Unable to open file : %s for reading",i_filename);
    RETURN_ERROR(ECMD_DBUF_FOPEN_FAIL);  
  }
  
  ins.seekg(0, std::ios::end);
  totalFileSz = ins.tellg();
  if (totalFileSz == 0) {
    ETRAC1("**** ERROR : File : %s is empty",i_filename);
    RETURN_ERROR(ECMD_DBUF_INVALID_ARGS);
  } else {
    ins.seekg(0); // Goto the beginning of the file 
  }
  
  ecmdFormatType_t existingFmt;
  uint32_t begOffset=0;
  if (i_format == ECMD_SAVE_FORMAT_BINARY_DATA) {
    o_num = 1; // Can have only 1 databuffer with this format
    return rc; 
  }
  
  ins.seekg(totalFileSz-8); //get the Begin offset of the offset table
  ins.read((char *)&begOffset,4); begOffset = htonl(begOffset);
  ins.read((char *)&existingFmt,4); existingFmt = (ecmdFormatType_t)htonl(existingFmt);
  if (existingFmt != i_format) {
    ETRAC0("**** ERROR : Format requested does not match up with the file Format.");
    RETURN_ERROR(ECMD_DBUF_INVALID_ARGS);
  }
  //Find out the END offset
  ins.seekg(begOffset+8); //goto the beginning of the table
  while(!endFound) {
    ins.read(endKeyword,4);  
    if (strcmp(endKeyword, "END") == 0) {
      endOffset = ins.tellg(); endOffset -= 4;
      endFound = true;
    }
    if ((uint32_t)ins.tellg() >= totalFileSz) break;
  }
  if (!endFound) {
    ETRAC0("**** ERROR : END keyword not found. Invalid File Format.");
    RETURN_ERROR(ECMD_DBUF_FILE_FORMAT_MISMATCH);
  } else {
    o_num = (endOffset - (begOffset+8)) / 4;
  }
  
  return rc;
  
}

uint32_t ecmdDataBuffer::readFileMultiple(const char * i_filename, ecmdFormatType_t i_format, const char * i_prpty, uint32_t &o_dataNumber) {

  ECMD_NULL_PTR_CHECK(i_filename);

  uint32_t *dataOffsets;
  bool propertyMatch = false;
  std::string facNameFromFile; 
  std::string i_property; 
  std::ifstream ins;
  uint32_t property = 0;
  uint32_t endOffset = 0, totalFileSz=0;
  bool endFound = false;
  char key[6], hexstr[8], endKeyword[4], fac[201];
  ecmdFormatType_t format;

  if (i_prpty == NULL) {
    ETRAC0("**** ERROR : property field NULL.");
    RETURN_ERROR(ECMD_DBUF_INVALID_ARGS);
  } else { 
    i_property = i_prpty; 
  }

  if (i_format == ECMD_SAVE_FORMAT_BINARY_DATA) {
    ETRAC0("**** ERROR : File Format ECMD_SAVE_FORMAT_BINARY_DATA not supported when property value is used as input.");
    RETURN_ERROR(ECMD_DBUF_INVALID_ARGS);
  }

  ins.open(i_filename);

  if (ins.fail()) {
    ETRAC1("**** ERROR : Unable to open file : %s for reading",i_filename);
    RETURN_ERROR(ECMD_DBUF_FOPEN_FAIL);
  }

  ins.seekg(0, std::ios::end);
  totalFileSz = ins.tellg();
  if (totalFileSz == 0) {
    ETRAC1("**** ERROR : File : %s is empty",i_filename);
    RETURN_ERROR(ECMD_DBUF_INVALID_ARGS);
  } else {
    ins.seekg(0); // Goto the beginning of the file
  }

  //Read the DataBuffer offset table-Seek to the correct DataBuffer Hdr
    ecmdFormatType_t existingFmt;
    uint32_t begOffset=0 ;

    ins.seekg(totalFileSz-8);//get the Begin offset of the offset table
    ins.read((char *)&begOffset,4); begOffset = htonl(begOffset);
    ins.read((char *)&existingFmt,4); existingFmt = (ecmdFormatType_t)htonl(existingFmt);
    if (existingFmt != i_format) {
      ETRAC0("**** ERROR : Format requested does not match up with the file Format.");
      RETURN_ERROR(ECMD_DBUF_INVALID_ARGS);
    }

    //Find out the END offset
    ins.seekg(begOffset+8); //goto the beginning of the table
    while(!endFound) {
      ins.read(endKeyword,4);
      if (strcmp(endKeyword, "END") == 0) {
        endOffset = ins.tellg(); endOffset -= 4;
        endFound = true;
      }
      if ((uint32_t)ins.tellg() >= totalFileSz) break;
    }
    if (!endFound) {
      ETRAC0("**** ERROR : END keyword not found. Invalid File Format.");
      RETURN_ERROR(ECMD_DBUF_FILE_FORMAT_MISMATCH);
    }
    uint32_t numOfDataBuffers = (endOffset-(begOffset+8))/4;
    dataOffsets = new uint32_t[numOfDataBuffers];

    for (uint32_t i=0; i<numOfDataBuffers; i++) {
      ins.seekg(begOffset+8+(4*i));
      ins.read((char *)&dataOffsets[i],4);  dataOffsets[i] = htonl(dataOffsets[i]);
    }

    /* Look for the property value */
    for (uint32_t i=0; i<numOfDataBuffers; i++) {
      ins.seekg(dataOffsets[i]);
      if (i_format == ECMD_SAVE_FORMAT_BINARY) {
        ins.seekg(12,std::ios::cur);
        ins.read((char *)&format,4);    format = (ecmdFormatType_t)htonl(format);
        ins.read((char *)&property,4);    property = htonl(property);
        if (property == 0x80000000) {
         ins.read(fac, 200);
         fac[200] = '\0';
         facNameFromFile = fac;
         if (facNameFromFile.find(i_prpty, 0, facNameFromFile.length()) != std::string::npos){
           o_dataNumber = i;  
           propertyMatch = true;
           break;
         }
        }
      } else if( (i_format == ECMD_SAVE_FORMAT_ASCII) || (i_format == ECMD_SAVE_FORMAT_XSTATE)) {
        ins.width(6); ins >> key;
        ins.width(9);  ins >> hexstr;
        ins.width(9); ins >> hexstr;
        format = (ecmdFormatType_t) strtoul(hexstr, NULL, 16);
        ins.getline(hexstr, 9);
        if (strcmp(hexstr, "80000000") == 0) {
          ins.getline(fac, 200);
          facNameFromFile = fac;
          if (facNameFromFile.find(i_property.c_str(), 0, facNameFromFile.length()) != std::string::npos){
            o_dataNumber = i;
            propertyMatch = true;
            break;
          }
        }
      }
    }
    delete [] dataOffsets;
    if (!propertyMatch) {
       ETRAC1("**** ERROR : Match for property: %s not found",i_property.c_str());
       RETURN_ERROR(ECMD_DBUF_INVALID_ARGS);
    } else {
      std::string ofac;
      return this->readFileMultiple(i_filename, i_format, o_dataNumber, &ofac);
    }
}

uint32_t ecmdDataBuffer::readFileMultiple(const char * i_filename, ecmdFormatType_t i_format, uint32_t i_dataNumber, std::string *o_facName) {

  ECMD_NULL_PTR_CHECK(i_filename);

  uint32_t rc = ECMD_DBUF_SUCCESS;
  std::ifstream ins;
  uint32_t numBits = 0, numBytes = 0, hexbitlen = 0, property = 0, *buffer;
  uint32_t endOffset = 0, totalFileSz=0;
  bool endFound = false;
  char key[6], hexstr[8], endKeyword[4], fac[201];

#ifndef REMOVE_SIM
  uint32_t NumDwords = 0;
  char binstr[64];
#endif 

#ifdef REMOVE_SIM
  if (i_format == ECMD_SAVE_FORMAT_XSTATE) {
    ETRAC0( "**** ERROR : ecmdDataBuffer: readFileMultiple: FORMAT_XSTATE not supported in this configuration");
    return(ECMD_DBUF_XSTATE_ERROR);
  }    
#endif
  
  ins.open(i_filename);
    
  if (ins.fail()) {
    ETRAC1("**** ERROR : Unable to open file : %s for reading",i_filename);
    RETURN_ERROR(ECMD_DBUF_FOPEN_FAIL);  
  }
 
  ins.seekg(0, std::ios::end);
  totalFileSz = ins.tellg();
  if (totalFileSz == 0) {
    ETRAC1("**** ERROR : File : %s is empty",i_filename);
    RETURN_ERROR(ECMD_DBUF_INVALID_ARGS);
  } else {
    ins.seekg(0); // Goto the beginning of the file 
  }
  //Read the DataBuffer offset table-Seek to the right DataBuffer Hdr
  if (i_dataNumber != 0) {
    ecmdFormatType_t existingFmt;
    uint32_t begOffset=0, dataOffset;
    if (i_format == ECMD_SAVE_FORMAT_BINARY_DATA) {
      ETRAC0("**** ERROR : File Format ECMD_SAVE_FORMAT_BINARY_DATA not supported when file contains multiple DataBuffers.");
      RETURN_ERROR(ECMD_DBUF_INVALID_ARGS);
    }
    
    ins.seekg(totalFileSz-8);//get the Begin offset of the offset table
    ins.read((char *)&begOffset,4); begOffset = htonl(begOffset);
    ins.read((char *)&existingFmt,4); existingFmt = (ecmdFormatType_t)htonl(existingFmt);
    if (existingFmt != i_format) {
      ETRAC0("**** ERROR : Format requested does not match up with the file Format.");
      RETURN_ERROR(ECMD_DBUF_INVALID_ARGS);
    }
    //Find out the END offset
    ins.seekg(begOffset+8); //goto the beginning of the table
    while(!endFound) {
      ins.read(endKeyword,4);  
      if (strcmp(endKeyword, "END") == 0) {
        endOffset = ins.tellg(); endOffset -= 4;
        endFound = true;
      }
      if ((uint32_t)ins.tellg() >= totalFileSz) break;
    }
    if (!endFound) {
      ETRAC0("**** ERROR : END keyword not found. Invalid File Format.");
      RETURN_ERROR(ECMD_DBUF_FILE_FORMAT_MISMATCH);
    }
    if ((begOffset+8+(4*i_dataNumber)) >= endOffset) {
      ETRAC0("**** ERROR : Data Number requested exceeds the maximum Data Number in the File.");
      RETURN_ERROR(ECMD_DBUF_DATANUMBER_NOT_FOUND);
    }
    ins.seekg(begOffset+8+(4*i_dataNumber));//seek to the right databuffer header
    ins.read((char *)&dataOffset,4);  dataOffset = htonl(dataOffset);
    if (ins.fail()) {
     ETRAC1("**** ERROR : Read of the dataOffset failed on file : %s",i_filename);
     RETURN_ERROR(ECMD_DBUF_FILE_OPERATION_FAIL); 
    }
    ins.seekg(dataOffset);
    if (ins.eof()) {
     ETRAC0("**** ERROR : Data Offset is greater than the file size.");
     RETURN_ERROR(ECMD_DBUF_FILE_OPERATION_FAIL); 
    }
  }
 
  if ( i_format == ECMD_SAVE_FORMAT_BINARY) {
    // Read Hdr
    ins.read(key,5); key[5]='\0';
    if(strcmp(key,"START")!=0) {
      ETRAC0("**** ERROR : Keyword START not found.");
      RETURN_ERROR(ECMD_DBUF_FILE_FORMAT_MISMATCH); 
    }
    ins.seekg(3,std::ios::cur);
    ins.read((char *)&numBits,4);    numBits = htonl(numBits);
    ins.read((char *)&i_format,4);    i_format = (ecmdFormatType_t)htonl(i_format);
    if (i_format != ECMD_SAVE_FORMAT_BINARY ) {
      ETRAC0("**** ERROR : Format mismatch. Expected ECMD_SAVE_FORMAT_BINARY.");
      RETURN_ERROR(ECMD_DBUF_FILE_FORMAT_MISMATCH);  
    }
    ins.read((char *)&property,4);    property = htonl(property);
    if (property == 0x80000000) {
      ins.read(fac, 200);
      fac[200] = '\0'; 
      if (o_facName != NULL)
        *o_facName = fac;
    }
    this->setBitLength(numBits);
    numBytes=getByteLength();
    //Read Data
    buffer = new uint32_t[getWordLength()];
    ins.read((char *)buffer,numBytes);
    if (ins.fail()) {
      ETRAC1("**** ERROR : Read operation in format ECMD_SAVE_FORMAT_BINARY failed on file : %s",i_filename);
      delete[] buffer;
      RETURN_ERROR(ECMD_DBUF_FILE_OPERATION_FAIL); 
    }
    for(uint32_t i=0; i< getWordLength(); i++) {
     buffer[i]=htonl(buffer[i]);
    }
    rc = memCopyIn(buffer, numBytes); 
    delete[] buffer; buffer = NULL;
    if (rc) return rc;
  } else if ( i_format == ECMD_SAVE_FORMAT_BINARY_DATA) {
    ins.seekg(0, std::ios::end);
    numBytes = ins.tellg();
    numBits = numBytes * 8;
    this->setBitLength(numBits);
    ins.seekg(0, std::ios::beg);
    buffer = new uint32_t[getWordLength()];
    ins.read((char *)buffer,numBytes);
    if (ins.fail()) {
      ETRAC1("**** ERROR : Read operation in format ECMD_SAVE_FORMAT_BINARY_DATA failed on file : %s",i_filename);
      delete[] buffer;
      RETURN_ERROR(ECMD_DBUF_FILE_OPERATION_FAIL); 
    }
    for(uint32_t i=0; i< getWordLength(); i++) {
     buffer[i]=htonl(buffer[i]);
    }
    rc = memCopyIn(buffer, numBytes); 
    delete[] buffer; buffer = NULL;
    if (rc) return rc;
  } else if(i_format ==  ECMD_SAVE_FORMAT_ASCII) {
    ins.width(6); ins >> key; 
    if(strcmp(key,"START")!=0) {
      ETRAC0("**** ERROR : Keyword START not found.");
      RETURN_ERROR(ECMD_DBUF_FILE_FORMAT_MISMATCH); 
    }
    ins.width(9);  ins >> hexstr; 
    numBits = strtoul(hexstr, NULL, 16); 
    ins.width(9); ins >> hexstr;  
    i_format = (ecmdFormatType_t) strtoul(hexstr, NULL, 16);
    if (i_format != ECMD_SAVE_FORMAT_ASCII ) {
      ETRAC0("**** ERROR : Format mismatch. Expected ECMD_SAVE_FORMAT_ASCII.");
      RETURN_ERROR(ECMD_DBUF_FILE_FORMAT_MISMATCH);  
    }
    ins.getline(hexstr, 9); 
    if (strcmp(hexstr, "80000000") == 0) {
      ins.getline(fac, 200);
      if (o_facName != NULL)
        *o_facName = fac;
    } 
    this->setBitLength(numBits);
    for (uint32_t i = 0; i < getWordLength(); i++) {
      ins.width(9);  ins >> hexstr;
      if (((i*32)+32) > numBits) {
        hexstr[strlen(hexstr)] = '\0'; //strip newline char
    hexbitlen = numBits - (i*32);
      } else {
        hexbitlen = 32;
      }
      rc = insertFromHexLeft (hexstr, i*32, hexbitlen); if (rc) return rc;
      ins.seekg(1,std::ios::cur);//Space or Newline char
    }
#ifndef REMOVE_SIM
  } else if( i_format == ECMD_SAVE_FORMAT_XSTATE) {
      /// cje need to enable xstate
    ins.width(6); ins >> key; 
    if(strcmp(key,"START")!=0) {
      ETRAC0("**** ERROR : Keyword START not found.");
      RETURN_ERROR(ECMD_DBUF_FILE_FORMAT_MISMATCH);
    }
    ins.width(9);  ins >> hexstr;
    numBits = strtoul(hexstr, NULL, 16);
    ins.width(9); ins >> hexstr;
    i_format = (ecmdFormatType_t) strtoul(hexstr, NULL, 16);
    if (i_format != ECMD_SAVE_FORMAT_XSTATE ) {
      ETRAC0("**** ERROR : Format mismatch. Expected ECMD_SAVE_FORMAT_XSTATE.");
      RETURN_ERROR(ECMD_DBUF_FILE_FORMAT_MISMATCH);
    }
    ins.getline(hexstr, 9);
    if (strcmp(hexstr, "80000000") == 0) { // String property set
      ins.getline(fac, 200);
      if (o_facName != NULL)
        *o_facName = fac;
    }
    
    this->setBitLength(numBits);
    NumDwords = (iv_NumBits + 63) / 64;
    for (uint32_t i = 0; i < NumDwords; i++) {
      ins.width(65);  ins >> binstr;
      if ((i*64)+64 > numBits) 
        binstr[strlen(binstr)] = '\0'; //strip newline char
      rc = setXstate(i*64, binstr); if (rc) return rc;
      ins.seekg(1,std::ios::cur);// New line char
    }
#endif
  }
  ins.close();
  return(rc);
}

uint32_t ecmdDataBuffer::readFile(const char * i_filename, ecmdFormatType_t i_format, std::string *o_facName) {
  uint32_t dataNumber=0;
  return this->readFileMultiple(i_filename, i_format, dataNumber, o_facName);
}

uint32_t ecmdDataBuffer::readFileStream(std::istream & i_filestream, uint32_t i_bitlength) {
  uint32_t rc = ECMD_DBUF_SUCCESS;
  uint32_t numBytes = (i_bitlength + 7) / 8;
  uint32_t *buffer;
  
  this->setBitLength(i_bitlength);
  
  buffer = new uint32_t[getWordLength()];
  i_filestream.read((char *)buffer, numBytes);
  if (i_filestream.fail()) {
      ETRAC0("**** ERROR : Read operation failed.");
      delete[] buffer;
      RETURN_ERROR(ECMD_DBUF_FILE_OPERATION_FAIL); 
  }

  
  for (uint32_t i=0; i< getWordLength(); i++) {
   buffer[i] = htonl(buffer[i]);
  }
  rc = memCopyIn(buffer, numBytes);
  delete[] buffer;
  return rc;
}

uint32_t ecmdDataBuffer::shareBuffer(ecmdDataBuffer* i_sharingBuffer)
{
    ECMD_NULL_PTR_CHECK(i_sharingBuffer);

    uint32_t rc = ECMD_DBUF_SUCCESS;

    rc = ecmdDataBufferBase::shareBuffer(i_sharingBuffer);
    if (rc) return rc;
#ifndef REMOVE_SIM
    if (i_sharingBuffer->iv_DataStr != NULL) {
      delete[] i_sharingBuffer->iv_DataStr;
    }
    i_sharingBuffer->iv_DataStr = iv_DataStr;
    i_sharingBuffer->iv_XstateEnabled = iv_XstateEnabled;
#endif
    return(rc);
}

uint32_t* ecmdDataBufferImplementationHelper::getDataPtr( void* i_buffer ) {
  if (i_buffer == NULL) return NULL;
  ecmdDataBuffer* buff = (ecmdDataBuffer*)i_buffer;
  return buff->iv_Data;
};

const uint32_t * ecmdDataBufferImplementationHelper::getConstDataPtr( const ecmdDataBuffer * i_buffer ) {
  if (i_buffer == NULL) return NULL;
  return i_buffer->iv_Data;
};

#ifndef REMOVE_SIM
void ecmdDataBufferImplementationHelper::applyRawBufferToXstate( void* i_buffer ) {
  if (i_buffer == NULL) return;
  ecmdDataBuffer* buff = (ecmdDataBuffer*)i_buffer;
  if (!buff->iv_XstateEnabled) {
    return;
  }
  strcpy(buff->iv_DataStr,buff->genXstateStr().c_str());
  return;
}
#endif

/********************************************************************************
       These routines belong to derived class ecmdOptimizableDataBuffer
 ********************************************************************************/

ecmdOptimizableDataBuffer::ecmdOptimizableDataBuffer()
{
   iv_BufferOptimizable = true;     
}

ecmdOptimizableDataBuffer::ecmdOptimizableDataBuffer(uint32_t i_numBits)
 : ecmdDataBuffer(i_numBits) {
       iv_BufferOptimizable = true;     
}

ecmdOptimizableDataBuffer::~ecmdOptimizableDataBuffer(){}

uint32_t ecmdDataBuffer::shiftLeft(uint32_t i_shiftnum, uint32_t i_offset)
{
  return ecmdDataBufferBase::shiftLeft(i_shiftnum,i_offset);
}

uint32_t ecmdDataBuffer::memCopyOut(uint32_t * o_buf, uint32_t i_bytes) const
{
  ECMD_NULL_PTR_CHECK(o_buf);

  return ecmdDataBufferBase::memCopyOut(o_buf, i_bytes);
}

uint32_t ecmdDataBuffer::memCopyOut(uint8_t * o_buf, uint32_t i_bytes) const
{
  ECMD_NULL_PTR_CHECK(o_buf);

  return ecmdDataBufferBase::memCopyOut(o_buf, i_bytes);
}

uint32_t ecmdDataBuffer::memCopyOut(uint16_t * o_buf, uint32_t i_bytes) const
{
  ECMD_NULL_PTR_CHECK(o_buf);

  return ecmdDataBufferBase::memCopyOut(o_buf, i_bytes);
}

uint16_t ecmdDataBuffer::getHalfWord(uint32_t i_halfwordoffset) const
{
  return ecmdDataBufferBase::getHalfWord(i_halfwordoffset);
}
uint32_t ecmdDataBuffer::memCopyIn(const uint16_t * i_buf, uint32_t i_bytes)
{
  ECMD_NULL_PTR_CHECK(i_buf);

  return ecmdDataBufferBase::memCopyIn(i_buf, i_bytes);
}
uint32_t ecmdDataBuffer::memCopyIn(const uint8_t * i_buf, uint32_t i_bytes)
{
  ECMD_NULL_PTR_CHECK(i_buf);

  return ecmdDataBufferBase::memCopyIn(i_buf, i_bytes);
}
bool   ecmdDataBuffer::isBitClear(uint32_t i_bit, uint32_t i_len) const
{
  return ecmdDataBufferBase::isBitClear(i_bit, i_len);
}
uint32_t ecmdDataBuffer::shrinkBitLength(uint32_t i_newNumBits)
{
  return ecmdDataBufferBase::shrinkBitLength(i_newNumBits);
}
uint32_t ecmdDataBuffer::unflatten(const uint8_t * i_data, uint32_t i_len)
{
  ECMD_NULL_PTR_CHECK(i_data);

  return ecmdDataBufferBase::unflatten(i_data, i_len);
}
uint32_t ecmdDataBuffer::unflattenTryKeepCapacity(const uint8_t * i_data, uint32_t i_len)
{
  ECMD_NULL_PTR_CHECK(i_data);

  return ecmdDataBufferBase::unflattenTryKeepCapacity(i_data, i_len);
}
uint32_t  ecmdDataBuffer::setBitLength(uint32_t i_newNumBits)
{
  return ecmdDataBufferBase::setBitLength(i_newNumBits);
}
uint32_t ecmdDataBuffer::getNumBitsSet(uint32_t i_bit, uint32_t i_len) const
{
  return ecmdDataBufferBase::getNumBitsSet(i_bit, i_len);
}
uint32_t ecmdDataBuffer::flattenSize(void) const
{
  return ecmdDataBufferBase::flattenSize();
}
uint32_t   ecmdDataBuffer::getBitLength() const
{
  return ecmdDataBufferBase::getBitLength();
}
uint32_t   ecmdDataBuffer::getWordLength() const
{
  return ecmdDataBufferBase::getWordLength();
}
uint32_t ecmdDataBuffer::extractPreserve(ecmdDataBuffer & o_bufferOut, uint32_t i_start, uint32_t i_len, uint32_t i_targetStart) const
{
  return ecmdDataBufferBase::extractPreserve((o_bufferOut), i_start, i_len, i_targetStart);
}
uint32_t ecmdDataBuffer::extractPreserve(uint32_t * o_data, uint32_t i_start, uint32_t i_len, uint32_t i_targetStart) const
{
  ECMD_NULL_PTR_CHECK(o_data);

  return ecmdDataBufferBase::extractPreserve(o_data, i_start, i_len, i_targetStart);
}

uint32_t  ecmdDataBuffer::setWordLength(uint32_t i_newNumWords)
{
  return ecmdDataBufferBase::setWordLength(i_newNumWords);
}
uint32_t ecmdDataBuffer::flatten(uint8_t * o_data, uint32_t i_len) const
{
  ECMD_NULL_PTR_CHECK(o_data);

  return ecmdDataBufferBase::flatten(o_data, i_len);
}
uint32_t  ecmdDataBuffer::setByteLength(uint32_t i_newNumBytes)
{
  return ecmdDataBufferBase::setByteLength(i_newNumBytes);
}
uint32_t   ecmdDataBuffer::getByteLength() const
{
  return ecmdDataBufferBase::getByteLength();
}
uint32_t ecmdDataBuffer::getWord(uint32_t i_wordoffset) const
{
  return ecmdDataBufferBase::getWord(i_wordoffset);
}
uint8_t ecmdDataBuffer::getByte(uint32_t i_byteoffset) const
{
  return ecmdDataBufferBase::getByte(i_byteoffset);
}
uint32_t ecmdDataBuffer::insert(uint32_t i_data, uint32_t i_targetStart, uint32_t i_len, uint32_t i_sourceStart )
{
  return ecmdDataBufferBase::insert(i_data,i_targetStart,i_len,i_sourceStart);
}
uint64_t ecmdDataBuffer::getDoubleWord(uint32_t i_doublewordoffset) const
{
  return ecmdDataBufferBase::getDoubleWord(i_doublewordoffset);
}

uint32_t ecmdDataBuffer::setXor(uint32_t i_data, uint32_t i_startbit, uint32_t i_len)
{
  return ecmdDataBufferBase::setXor(i_data, i_startbit, i_len);
}
uint32_t ecmdDataBuffer::setXor(const uint32_t * i_data, uint32_t i_startbit, uint32_t i_len)
{
  ECMD_NULL_PTR_CHECK(i_data);
  
  return ecmdDataBufferBase::setXor(i_data, i_startbit, i_len);
}

bool ecmdDataBuffer::getBit(uint32_t i_bit) const
{
   return ecmdDataBufferBase::getBit(i_bit);
}

uint32_t ecmdDataBuffer::insert(uint8_t i_datain, uint32_t i_targetStart, uint32_t i_len, uint32_t i_sourceStart)
{
   return ecmdDataBufferBase::insert(i_datain, i_targetStart, i_len, i_sourceStart);
}

uint32_t ecmdDataBuffer::setAnd(uint32_t i_data, uint32_t i_startbit, uint32_t i_len)
{
   return ecmdDataBufferBase::setAnd(i_data, i_startbit, i_len);
}

uint32_t ecmdDataBuffer::shiftRight(uint32_t i_shiftnum, uint32_t i_offset )
{
   return ecmdDataBufferBase::shiftRight(i_shiftnum, i_offset);
}

uint32_t ecmdDataBuffer::insertFromRight(const uint32_t * i_data, uint32_t i_start, uint32_t i_len)
{
   ECMD_NULL_PTR_CHECK(i_data);

   return ecmdDataBufferBase::insertFromRight(i_data, i_start, i_len);
}

uint32_t ecmdDataBuffer::oddParity(uint32_t i_start, uint32_t i_stop) const
{
   return ecmdDataBufferBase::oddParity(i_start, i_stop);
}

uint32_t ecmdDataBuffer::concat(const ecmdDataBuffer & i_buf0, const ecmdDataBuffer & i_buf1)
{
  return ecmdDataBufferBase::concat(i_buf0, i_buf1);
}

uint32_t ecmdDataBuffer::concat(const std::vector<ecmdDataBuffer> & i_bufs) 
{
  uint32_t rc = ECMD_DBUF_SUCCESS;
  uint32_t totalSize = 0, offset = 0, x;
  /* Loop through and get the total bit size and set length */
  for (x = 0; x < i_bufs.size(); x++) {
    totalSize += i_bufs[x].iv_NumBits;
  }
  rc = this->setBitLength(totalSize); if (rc) return rc;

  /* Now that the size is set, loop through and insert */
  for (x = 0; x < i_bufs.size(); x++) {
    rc = this->insert(i_bufs[x], offset, i_bufs[x].iv_NumBits); if (rc) return rc;
    offset += i_bufs[x].iv_NumBits;
  }

  return rc;
}

uint32_t ecmdDataBuffer::extractToRight(uint32_t * o_data, uint32_t i_start, uint32_t i_len) const
{
   ECMD_NULL_PTR_CHECK(o_data);
  
   return ecmdDataBufferBase::extractToRight(o_data, i_start, i_len);
}

uint32_t ecmdDataBuffer::compressBuffer(ecmdCompressionMode_t i_mode)
{
  return ecmdDataBufferBase::compressBuffer(i_mode);
}

uint32_t ecmdDataBuffer::uncompressBuffer()
{
    return ecmdDataBufferBase::uncompressBuffer();
}


uint32_t ecmdDataBuffer::flipBit(uint32_t i_bit, uint32_t i_len)
{
  return ecmdDataBufferBase::flipBit(i_bit, i_len);
}

uint32_t ecmdDataBuffer::evenParity(uint32_t i_start, uint32_t i_stop) const
{
   return ecmdDataBufferBase::evenParity(i_start, i_stop);
}

uint32_t ecmdDataBuffer::shiftLeftAndResize(uint32_t i_shiftnum)
{
  return ecmdDataBufferBase::shiftLeftAndResize(i_shiftnum);
}

uint32_t ecmdDataBuffer::extractToRight(uint8_t * o_data, uint32_t i_start, uint32_t i_len) const
{ 
   ECMD_NULL_PTR_CHECK(o_data);

   return ecmdDataBufferBase::extractToRight(o_data, i_start, i_len);
}

bool   ecmdDataBuffer::isBitSet(uint32_t i_bit, uint32_t i_len) const
{
  return  ecmdDataBufferBase::isBitSet(i_bit, i_len);
}

uint32_t ecmdDataBuffer::setOr(uint32_t i_data, uint32_t i_startbit, uint32_t i_len)
{
   return  ecmdDataBufferBase::setOr(i_data, i_startbit, i_len);
}

void ecmdDataBuffer::queryErrorState(uint32_t & o_errorState)
{
   return  ecmdDataBufferBase::queryErrorState(o_errorState);
}

void ecmdDataBuffer::queryErrorState(uint32_t & o_errorState) const
{
   return  ecmdDataBufferBase::queryErrorState(o_errorState);
}

uint32_t ecmdDataBuffer::invert()
{
   return  ecmdDataBufferBase::invert();
}

uint32_t ecmdDataBuffer::setBit(uint32_t i_bit, uint32_t i_len)
{
   return  ecmdDataBufferBase::setBit(i_bit, i_len);
}

uint32_t ecmdDataBuffer::extractToRight(uint16_t * o_data, uint32_t i_start, uint32_t i_len) const
{
   ECMD_NULL_PTR_CHECK(o_data);

   return  ecmdDataBufferBase::extractToRight(o_data, i_start, i_len);
}

uint32_t ecmdDataBuffer::evenParity(uint32_t i_start, uint32_t i_stop, uint32_t i_insertpos)
{
    return  ecmdDataBufferBase::evenParity(i_start, i_stop, i_insertpos);     
}

uint32_t ecmdDataBuffer::writeBit(uint32_t i_bit, uint32_t i_value)
{
   return  ecmdDataBufferBase::writeBit(i_bit, i_value);
}

uint32_t ecmdDataBuffer::clearBit(uint32_t i_bit, uint32_t i_len)
{
   return  ecmdDataBufferBase::clearBit(i_bit, i_len);
}

uint32_t ecmdDataBuffer::insert(const uint8_t * i_data, uint32_t i_targetStart, uint32_t i_len, uint32_t i_sourceStart)
{
  ECMD_NULL_PTR_CHECK(i_data);

  return  ecmdDataBufferBase::insert(i_data, i_targetStart, i_len, i_sourceStart);
}

uint32_t ecmdDataBuffer::insertFromRight(uint32_t i_data, uint32_t i_start, uint32_t i_len)
{
  return ecmdDataBufferBase::insertFromRight(i_data, i_start, i_len);
}

uint32_t   ecmdDataBuffer::getCapacity() const
{
  return ecmdDataBufferBase::getCapacity();
}

uint32_t ecmdDataBuffer::rotateRight(uint32_t i_rotatenum)
{
  return ecmdDataBufferBase::rotateRight(i_rotatenum);
}

uint32_t ecmdDataBuffer::rotateLeft(uint32_t i_rotatenum)
{
  return ecmdDataBufferBase::rotateLeft(i_rotatenum);
}

uint32_t ecmdDataBuffer::extractPreserve(uint16_t * o_data, uint32_t i_start, uint32_t i_len, uint32_t i_targetStart) const
{
  ECMD_NULL_PTR_CHECK(o_data);

  return ecmdDataBufferBase::extractPreserve(o_data, i_start, i_len, i_targetStart);
}

uint32_t ecmdDataBuffer::extractPreserve(uint8_t * o_data, uint32_t i_start, uint32_t i_len, uint32_t i_targetStart ) const
{
  ECMD_NULL_PTR_CHECK(o_data)

  return ecmdDataBufferBase::extractPreserve(o_data, i_start, i_len, i_targetStart);
}

uint32_t  ecmdDataBuffer::setDoubleWordLength(uint32_t i_newNumDoubleWords)
{
  return ecmdDataBufferBase::setDoubleWordLength(i_newNumDoubleWords);
}

uint32_t ecmdDataBuffer::insertFromRight(const uint8_t * i_data, uint32_t i_start, uint32_t i_len)
{
  ECMD_NULL_PTR_CHECK(i_data)

  return ecmdDataBufferBase::insertFromRight(i_data, i_start, i_len);
}

uint32_t ecmdDataBuffer::insertFromRight(const uint8_t i_datain, uint32_t i_start, uint32_t i_len)
{
  return ecmdDataBufferBase::insertFromRight(i_datain, i_start, i_len);
}

uint32_t ecmdDataBuffer::insertFromRight(uint16_t i_datain, uint32_t i_start, uint32_t i_len)
{
  return ecmdDataBufferBase::insertFromRight(i_datain, i_start, i_len);
}

bool ecmdDataBuffer::isBufferCompressed()
{
  return ecmdDataBufferBase::isBufferCompressed();
}

uint32_t ecmdDataBuffer::extractToRight(ecmdDataBuffer & o_bufferOut, uint32_t i_start, uint32_t i_len) const
{
  return ecmdDataBufferBase::extractToRight(o_bufferOut, i_start, i_len);
}

uint32_t   ecmdDataBuffer::getDoubleWordLength() const
{
  return ecmdDataBufferBase::getDoubleWordLength();
}

uint32_t   ecmdDataBuffer::getHalfWordLength() const
{
  return ecmdDataBufferBase::getHalfWordLength();
}

uint32_t ecmdDataBuffer::insert(const uint16_t * i_datain, uint32_t i_targetStart, uint32_t i_len, uint32_t i_sourceStart )
{
  ECMD_NULL_PTR_CHECK(i_datain);

  return ecmdDataBufferBase::insert(i_datain, i_targetStart, i_len, i_sourceStart);
}

uint32_t ecmdDataBuffer::insert(uint16_t i_datain, uint32_t i_targetStart, uint32_t i_len, uint32_t i_sourceStart )
{
  return ecmdDataBufferBase::insert(i_datain, i_targetStart, i_len, i_sourceStart);
}

uint32_t ecmdDataBuffer::insertFromRight(const uint16_t * i_datain, uint32_t i_start, uint32_t i_len)
{
  ECMD_NULL_PTR_CHECK(i_datain);

  return ecmdDataBufferBase::insertFromRight(i_datain, i_start, i_len);
}

uint32_t ecmdDataBuffer::oddParity(uint32_t i_start, uint32_t i_stop, uint32_t i_insertpos)
{
  return ecmdDataBufferBase::oddParity(i_start, i_stop, i_insertpos);
}

uint32_t ecmdDataBuffer::setAnd(const uint32_t * i_data, uint32_t i_startbit, uint32_t i_len)
{
  ECMD_NULL_PTR_CHECK(i_data);

  return ecmdDataBufferBase::setAnd(i_data, i_startbit, i_len);
}

uint32_t  ecmdDataBuffer::setHalfWordLength(uint32_t i_newNumHalfWords)
{
  return ecmdDataBufferBase::setHalfWordLength(i_newNumHalfWords);
}

uint32_t ecmdDataBuffer::setOr(const uint32_t * i_data, uint32_t i_startbit, uint32_t i_len)
{
  ECMD_NULL_PTR_CHECK(i_data);

  return ecmdDataBufferBase::setOr(i_data, i_startbit, i_len);
}

uint32_t ecmdDataBuffer::shiftRightAndResize(uint32_t i_shiftnum, uint32_t i_offset )
{
  return ecmdDataBufferBase::shiftRightAndResize(i_shiftnum, i_offset);
}
