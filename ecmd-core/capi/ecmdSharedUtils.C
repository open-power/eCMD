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


//lint -e571 The casts are necessary in ecmdHashString - JTA
//lint -e744 We don't want there to be a default in the ecmdHashString case - JTA

//----------------------------------------------------------------------
//  Includes
//----------------------------------------------------------------------
#include <fstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <map>

#include <ecmdSharedUtils.H>
#include <ecmdStructs.H>
#include <ecmdReturnCodes.H>

// For Calling Hashing Algorithm in MCP: static inline u32 jhash(const void *key, u32 length, u32 initval)
#ifdef ECMD_USE_MCP
#include <stdio.h>

// Needed for jhash() call
#include <sys/types.h>
typedef u_int32_t u32 ;
typedef u_int8_t u8 ;

#define void u8
#include <linux/jhash.h>
#undef void
#endif // ECMD_USE_MCP

// Determine endianness of the machine we are building for
// Since AIX and Linux do it different, we are creating our own defines
// _BIG_ENDIAN & _LITTLE_ENDIAN are the defines we are creating
#ifdef __linux__
#include <endian.h>
#  if __BYTE_ORDER == __LITTLE_ENDIAN
#    ifndef _LITTLE_ENDIAN
#      define _LITTLE_ENDIAN
#    endif
#  elif __BYTE_ORDER == __BIG_ENDIAN
#    ifndef _BIG_ENDIAN
#      define _BIG_ENDIAN
#    endif
#  endif
#endif
#ifdef _AIX
#include <sys/machine.h>
#  if BYTE_ORDER == LITTLE_ENDIAN
#    ifndef _LITTLE_ENDIAN
#      define _LITTLE_ENDIAN
#    endif
#  elif BYTE_ORDER == BIG_ENDIAN
#    ifndef _BIG_ENDIAN
#      define _BIG_ENDIAN
#    endif
#  endif
#endif

// Determine endianess for ecmdHashString64
#if (defined(_LITTLE_ENDIAN))
# define HASH_LITTLE_ENDIAN 1
# define HASH_BIG_ENDIAN 0
#elif (defined(_BIG_ENDIAN))
# define HASH_LITTLE_ENDIAN 0
# define HASH_BIG_ENDIAN 1
#else
# define HASH_LITTLE_ENDIAN 0
# define HASH_BIG_ENDIAN 0
#endif


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
 * @brief Iterates over argv, removing null pointers and decrementing argc
 * @retval None
 * @param io_argc Pointer to number of elements in io_argv array
 * @param io_argv Array of strings passed in from command line

 - Utility function for ecmdParseOption and ecmdParseOptionWithArgs
 */
void ecmdRemoveNullPointers (int * io_argc, char ** io_argv[]);


//----------------------------------------------------------------------
//  Global Variables
//----------------------------------------------------------------------

//---------------------------------------------------------------------
// Member Function Specifications
//---------------------------------------------------------------------

void ecmdRemoveNullPointers (int *argc, char **argv[]) {
  int counter=0;
  int counter2=0;

  for (counter=0;counter<(*argc+1);counter++) {
    for (counter2=counter;counter2<*argc;counter2++) {
      if ((*argv)[counter]==NULL) {
        (*argv)[counter]=(*argv)[counter2];
        (*argv)[counter2]=NULL;
      }
    }
  }

  for (counter=0;counter<(*argc);counter++) {
    if ((*argv)[counter]==NULL) {
      *argc=counter;
      return;
    }
  }
}


bool ecmdParseOption (int *argc, char **argv[], const char *option) {
  int counter = 0;
  bool foundit = false;

  for (counter = 0; counter < *argc ; counter++) {
    if (((*argv)[counter] != NULL) && (strcmp((*argv)[counter],option)==0)) {
      (*argv)[counter]=NULL;
      foundit = true;
      break;
    }
  }

  ecmdRemoveNullPointers(argc, argv);
  return foundit;
}

/* ----------------------------------------------------------------- */
/* Function will parse for an option and eliminate it from a list    */
/* while returning a pointer to the option that is used.             */
/* If no option is available, the function will return NULL          */
/* ----------------------------------------------------------------- */
char * ecmdParseOptionWithArgs(int *argc, char **argv[], const char *option) {
  int counter = 0;
  char *returnValue=NULL;

  for (counter = 0; counter < *argc ; counter++) {
    if (((*argv)[counter] != NULL) && (strncmp((*argv)[counter],option,strlen(option))==0)) {
      if (strlen((*argv)[counter])>strlen(option)) {
        returnValue = &((*argv)[counter][strlen(option)]);
        (*argv)[counter]=NULL;
      } else {
        if ((counter+1)<*argc) {
          returnValue = (*argv)[counter+1];
          (*argv)[counter]=NULL;
          (*argv)[counter+1]=NULL;
        } else {
          returnValue = NULL;
        }
      }
      /* We found it , let's stop looping , we don't want to pull other args out if they are here, this fixes BZ#6 */
      break;

    }
  }

  ecmdRemoveNullPointers(argc, argv);

  return returnValue;
}


std::string ecmdGenEbcdic(const ecmdDataBuffer &i_data, int start, int bitLen) {
  std::string ret;
  std::string tempChar;
  // Conversion table
  char EBCDICtoASCIITable [256] = {
    0x2E,0x01,0x02,0x03,0x2E,0x09,0x2E,0x7f,
    0x2E,0x2E,0x2E,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x10,0x11,0x12,0x13,0x2E,0x2E,0x08,0x2E,
    0x18,0x19,0x2E,0x2E,0x1c,0x1d,0x1e,0x1f,
    0x2E,0x2E,0x2E,0x2E,0x2E,0x0a,0x17,0x1b,
    0x2E,0x2E,0x2E,0x2E,0x2E,0x05,0x06,0x07,
    0x2E,0x2E,0x16,0x2E,0x2E,0x2E,0x2E,0x04,
    0x2E,0x2E,0x2E,0x2E,0x14,0x15,0x2E,0x1a,
    0x20,0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,
    0x2E,0x2E,0x2E,0x2e,0x3c,0x28,0x2b,0x7c,
    0x26,0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,
    0x2E,0x2E,0x21,0x24,0x2a,0x29,0x3b,0x5e,
    0x2d,0x2f,0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,
    0x2E,0x2E,0x2E,0x2c,0x25,0x5f,0x3e,0x3f,
    0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,
    0x2E,0x60,0x3a,0x23,0x40,0x27,0x3d,0x22,
    0x2E,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
    0x68,0x69,0x2E,0x7b,0x2E,0x2E,0x2E,0x2E,
    0x2E,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,
    0x71,0x72,0x2E,0x7d,0x2E,0x2E,0x2E,0x2E,
    0x2E,0x7e,0x73,0x74,0x75,0x76,0x77,0x78,
    0x79,0x7a,0x2E,0x2E,0x2E,0x5b,0x2E,0x2E,
    0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,
    0x2E,0x2E,0x2E,0x2E,0x2E,0x5d,0x2E,0x2E,
    0x7b,0x41,0x42,0x43,0x44,0x45,0x46,0x47,
    0x48,0x49,0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,
    0x7d,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,0x50,
    0x51,0x52,0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,
    0x5c,0x2E,0x53,0x54,0x55,0x56,0x57,0x58,
    0x59,0x5a,0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
    0x38,0x39,0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,
};

  /* Loop through the dataBuffer and convert to EBCDIC */
  int startByte = start/8;
  int endByte = (start+bitLen)/8;
  for (int x = startByte; x < endByte; x++) {
    tempChar = EBCDICtoASCIITable[i_data.getByte(x)];
    ret.insert(ret.length(),tempChar);
  }

  return ret;
}

#define mix(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}

uint32_t ecmdHashString32(const char *k, uint32_t c)
{
#ifdef ECMD_USE_MCP
// Call MCP's jhash() function directly 
return jhash((const u8*)k, strlen(k), c);
#endif // ECMD_USE_MCP

    uint32_t length;
    uint32_t a,b,len;

    /* Set up the internal state */
    len = strlen(k);
    length = len;

    a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */

    /*---------------------------------------- handle most of the key */
    while (len >= 12)
    {
        a += (k[0] +((uint32_t)k[1]<<8) +((uint32_t)k[2]<<16) +((uint32_t)k[3]<<24));
        b += (k[4] +((uint32_t)k[5]<<8) +((uint32_t)k[6]<<16) +((uint32_t)k[7]<<24));
        c += (k[8] +((uint32_t)k[9]<<8) +((uint32_t)k[10]<<16)+((uint32_t)k[11]<<24));
        mix(a,b,c);
        k += 12; len -= 12;
    }
    /*------------------------------------- handle the last 11 bytes */
    c += length;
    // Add /*fall through*/ to tell Beam to ignore and not throw an error. @01a
    switch(len)  /* all the case statements fall through */
    {
    case 11: c+=((uint32_t)k[10]<<24); /*fall through*/
    case 10: c+=((uint32_t)k[9]<<16);  /*fall through*/
    case 9 : c+=((uint32_t)k[8]<<8);
        /* the first byte of c is reserved for the length */ /*fall through*/
    case 8 : b+=((uint32_t)k[7]<<24);  /*fall through*/
    case 7 : b+=((uint32_t)k[6]<<16);  /*fall through*/
    case 6 : b+=((uint32_t)k[5]<<8);   /*fall through*/
    case 5 : b+=k[4];                  /*fall through*/
    case 4 : a+=((uint32_t)k[3]<<24);  /*fall through*/
    case 3 : a+=((uint32_t)k[2]<<16);  /*fall through*/
    case 2 : a+=((uint32_t)k[1]<<8);   /*fall through*/
    case 1 : a+=k[0];
        /* case 0: nothing left to add */
    }
    mix(a,b,c);

    return(c);
}
#undef mix

#define rot(x,k) (((x)<<(k)) | ((x)>>(32-(k))))
#define mix(a,b,c) \
{ \
  a -= c;  a ^= rot(c, 4);  c += b; \
  b -= a;  b ^= rot(a, 6);  a += c; \
  c -= b;  c ^= rot(b, 8);  b += a; \
  a -= c;  a ^= rot(c,16);  c += b; \
  b -= a;  b ^= rot(a,19);  a += c; \
  c -= b;  c ^= rot(b, 4);  b += a; \
}
#define final(a,b,c) \
{ \
  c ^= b; c -= rot(b,14); \
  a ^= c; a -= rot(c,11); \
  b ^= a; b -= rot(a,25); \
  c ^= b; c -= rot(b,16); \
  a ^= c; a -= rot(c,4);  \
  b ^= a; b -= rot(a,14); \
  c ^= b; c -= rot(b,24); \
}

uint64_t ecmdHashString64(const char *i_str, uint64_t i_initval)
{
  uint32_t a,b,c;                                          /* internal state */
  union { const void *ptr; size_t i; } u;     /* needed for Mac Powerbook G4 */

  size_t length = strlen(i_str);
  uint32_t pc = (i_initval >> 32);
  uint32_t pb = i_initval;
  /* Set up the internal state */
  a = b = c = 0xdeadbeef + ((uint32_t)length) + pc;
  c += pb;

  u.ptr = i_str;
  if (HASH_LITTLE_ENDIAN && ((u.i & 0x3) == 0)) {
    const uint32_t *k = (const uint32_t *)i_str;         /* read 32-bit chunks */

    /*------ all but last block: aligned reads and affect 32 bits of (a,b,c) */
    while (length > 12)
    {
      a += k[0];
      b += k[1];
      c += k[2];
      mix(a,b,c);
      length -= 12;
      k += 3;
    }

    /*----------------------------- handle the last (probably partial) block */
    /* 
     * "k[2]&0xffffff" actually reads beyond the end of the string, but
     * then masks off the part it's not allowed to read.  Because the
     * string is aligned, the masked-off tail is in the same word as the
     * rest of the string.  Every machine with memory protection I've seen
     * does it on word boundaries, so is OK with this.  But VALGRIND will
     * still catch it and complain.  The masking trick does make the hash
     * noticably faster for short strings (like English words).
     */
#ifndef VALGRIND

    switch(length)
    {
    case 12: c+=k[2]; b+=k[1]; a+=k[0]; break;
    case 11: c+=k[2]&0xffffff; b+=k[1]; a+=k[0]; break;
    case 10: c+=k[2]&0xffff; b+=k[1]; a+=k[0]; break;
    case 9 : c+=k[2]&0xff; b+=k[1]; a+=k[0]; break;
    case 8 : b+=k[1]; a+=k[0]; break;
    case 7 : b+=k[1]&0xffffff; a+=k[0]; break;
    case 6 : b+=k[1]&0xffff; a+=k[0]; break;
    case 5 : b+=k[1]&0xff; a+=k[0]; break;
    case 4 : a+=k[0]; break;
    case 3 : a+=k[0]&0xffffff; break;
    case 2 : a+=k[0]&0xffff; break;
    case 1 : a+=k[0]&0xff; break;
    case 0 : return c + (((uint64_t)b)<<32);  /* zero length strings require no mixing */
    }

#else /* make valgrind happy */

    const uint8_t *k8 = (const uint8_t *)k;
    switch(length)
    {
    case 12: c+=k[2]; b+=k[1]; a+=k[0]; break;
    case 11: c+=((uint32_t)k8[10])<<16;  /* fall through */
    case 10: c+=((uint32_t)k8[9])<<8;    /* fall through */
    case 9 : c+=k8[8];                   /* fall through */
    case 8 : b+=k[1]; a+=k[0]; break;
    case 7 : b+=((uint32_t)k8[6])<<16;   /* fall through */
    case 6 : b+=((uint32_t)k8[5])<<8;    /* fall through */
    case 5 : b+=k8[4];                   /* fall through */
    case 4 : a+=k[0]; break;
    case 3 : a+=((uint32_t)k8[2])<<16;   /* fall through */
    case 2 : a+=((uint32_t)k8[1])<<8;    /* fall through */
    case 1 : a+=k8[0]; break;
    case 0 : return c + (((uint64_t)b)<<32);  /* zero length strings require no mixing */
    }

#endif /* !valgrind */

  } else if (HASH_LITTLE_ENDIAN && ((u.i & 0x1) == 0)) {
    const uint16_t *k = (const uint16_t *)i_str;         /* read 16-bit chunks */
    const uint8_t  *k8;

    /*--------------- all but last block: aligned reads and different mixing */
    while (length > 12)
    {
      a += k[0] + (((uint32_t)k[1])<<16);
      b += k[2] + (((uint32_t)k[3])<<16);
      c += k[4] + (((uint32_t)k[5])<<16);
      mix(a,b,c);
      length -= 12;
      k += 6;
    }

    /*----------------------------- handle the last (probably partial) block */
    k8 = (const uint8_t *)k;
    switch(length)
    {
    case 12: c+=k[4]+(((uint32_t)k[5])<<16);
             b+=k[2]+(((uint32_t)k[3])<<16);
             a+=k[0]+(((uint32_t)k[1])<<16);
             break;
    case 11: c+=((uint32_t)k8[10])<<16;     /* fall through */
    case 10: c+=k[4];
             b+=k[2]+(((uint32_t)k[3])<<16);
             a+=k[0]+(((uint32_t)k[1])<<16);
             break;
    case 9 : c+=k8[8];                      /* fall through */
    case 8 : b+=k[2]+(((uint32_t)k[3])<<16);
             a+=k[0]+(((uint32_t)k[1])<<16);
             break;
    case 7 : b+=((uint32_t)k8[6])<<16;      /* fall through */
    case 6 : b+=k[2];
             a+=k[0]+(((uint32_t)k[1])<<16);
             break;
    case 5 : b+=k8[4];                      /* fall through */
    case 4 : a+=k[0]+(((uint32_t)k[1])<<16);
             break;
    case 3 : a+=((uint32_t)k8[2])<<16;      /* fall through */
    case 2 : a+=k[0];
             break;
    case 1 : a+=k8[0];
             break;
    case 0 : return c + (((uint64_t)b)<<32);  /* zero length strings require no mixing */
    }

  } else {                        /* need to read the key one byte at a time */
    const uint8_t *k = (const uint8_t *)i_str;

    /*--------------- all but the last block: affect some 32 bits of (a,b,c) */
    while (length > 12)
    {
      a += k[0];
      a += ((uint32_t)k[1])<<8;
      a += ((uint32_t)k[2])<<16;
      a += ((uint32_t)k[3])<<24;
      b += k[4];
      b += ((uint32_t)k[5])<<8;
      b += ((uint32_t)k[6])<<16;
      b += ((uint32_t)k[7])<<24;
      c += k[8];
      c += ((uint32_t)k[9])<<8;
      c += ((uint32_t)k[10])<<16;
      c += ((uint32_t)k[11])<<24;
      mix(a,b,c);
      length -= 12;
      k += 12;
    }

    /*-------------------------------- last block: affect all 32 bits of (c) */
    switch(length)                   /* all the case statements fall through */
    {
    case 12: c+=((uint32_t)k[11])<<24;
             // fall through
    case 11: c+=((uint32_t)k[10])<<16;
             // fall through
    case 10: c+=((uint32_t)k[9])<<8;
             // fall through
    case 9 : c+=k[8];
             // fall through
    case 8 : b+=((uint32_t)k[7])<<24;
             // fall through
    case 7 : b+=((uint32_t)k[6])<<16;
             // fall through
    case 6 : b+=((uint32_t)k[5])<<8;
             // fall through
    case 5 : b+=k[4];
             // fall through
    case 4 : a+=((uint32_t)k[3])<<24;
             // fall through
    case 3 : a+=((uint32_t)k[2])<<16;
             // fall through
    case 2 : a+=((uint32_t)k[1])<<8;
             // fall through
    case 1 : a+=k[0];
             break;
    case 0 : return c + (((uint64_t)b)<<32);  /* zero length strings require no mixing */
    }
  }

  final(a,b,c);

  return c + (((uint64_t)b)<<32);
}
#undef rot
#undef mix
#undef final

uint32_t ecmdHexToUInt32(const char* str)
{
	const char* start = str + 2;
	return strtoull(start, NULL, 16);
}

uint32_t ecmdSetTargetDepth(ecmdChipTarget & io_target, ecmdTargetDepth_t i_depth)
{
  uint32_t rc = ECMD_SUCCESS;

  // Set Target Depth based on i_depth input
  switch (i_depth)
  {
    case ECMD_DEPTH_CAGE:
      io_target.cageState = ECMD_TARGET_FIELD_VALID;
      io_target.nodeState = io_target.slotState = io_target.chipTypeState = io_target.posState = io_target.chipUnitTypeState = io_target.chipUnitNumState = io_target.threadState = ECMD_TARGET_FIELD_UNUSED;
      break;

    case ECMD_DEPTH_NODE:
      io_target.cageState = io_target.nodeState = ECMD_TARGET_FIELD_VALID;
      io_target.slotState = io_target.chipTypeState = io_target.posState = io_target.chipUnitTypeState = io_target.chipUnitNumState = io_target.threadState = ECMD_TARGET_FIELD_UNUSED;
      break;

    case ECMD_DEPTH_SLOT:
      io_target.cageState = io_target.nodeState = io_target.slotState = ECMD_TARGET_FIELD_VALID;
      io_target.chipTypeState = io_target.posState = io_target.chipUnitTypeState = io_target.chipUnitNumState = io_target.threadState = ECMD_TARGET_FIELD_UNUSED;
      break;

    case ECMD_DEPTH_CHIP:
      io_target.cageState = io_target.nodeState = io_target.slotState = io_target.chipTypeState = io_target.posState = ECMD_TARGET_FIELD_VALID;
      io_target.chipUnitTypeState = io_target.chipUnitNumState = io_target.threadState = ECMD_TARGET_FIELD_UNUSED;
      break;

    case ECMD_DEPTH_CORE:
      io_target.cageState = io_target.nodeState = io_target.slotState = io_target.chipTypeState = io_target.posState =  io_target.chipUnitNumState = ECMD_TARGET_FIELD_VALID;
      io_target.chipUnitTypeState = io_target.threadState = ECMD_TARGET_FIELD_UNUSED;
      break;

    case ECMD_DEPTH_CHIPUNIT:
      io_target.cageState = io_target.nodeState = io_target.slotState = io_target.chipTypeState = io_target.posState = io_target.chipUnitTypeState = io_target.chipUnitNumState = ECMD_TARGET_FIELD_VALID;
      io_target.threadState = ECMD_TARGET_FIELD_UNUSED;
      break;

    case ECMD_DEPTH_THREAD:
      io_target.cageState = io_target.nodeState = io_target.slotState = io_target.chipTypeState = io_target.posState = io_target.chipUnitTypeState = io_target.chipUnitNumState = io_target.threadState = ECMD_TARGET_FIELD_VALID;
      break;


    default: // Used an unknown ecmdTargetDepth_t enum value
      rc = ECMD_INVALID_ARGS;
      break;
  }

  return rc;
}



uint32_t ecmdReadTarget(std::string i_targetStr, ecmdChipTarget & o_target) {

  uint32_t rc = ECMD_SUCCESS;
  std::vector<std::string> tokens;
  bool allFound = false;
  bool naFound = false;
  //Commenting numFound out to resolve compiler errors.  Nothing is ever being checked against it.
  //bool numFound = false;
  int num = 0;  //fix beam error
  bool l_alt_format_used = false;

  /* Set all the target states to unused as a starting point */
  o_target.cageState = ECMD_TARGET_FIELD_UNUSED;
  o_target.nodeState = ECMD_TARGET_FIELD_UNUSED;
  o_target.slotState = ECMD_TARGET_FIELD_UNUSED;
  o_target.chipTypeState = ECMD_TARGET_FIELD_UNUSED;
  o_target.posState = ECMD_TARGET_FIELD_UNUSED;
  o_target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;
  o_target.chipUnitNumState = ECMD_TARGET_FIELD_UNUSED;
  o_target.threadState = ECMD_TARGET_FIELD_UNUSED;

  /* Tokenize my string on colon */
  ecmdParseTokens(i_targetStr, ":", tokens);

  // Two possible formats that are supported: 
  // The 2nd one is considered the alternate format (i.e. l_alt_format_used = true)
  // cage:node:slot:chiptype[.chipunittype]:position[:chipunitnum[:thread]
  // 0    1    2    3                       4         5            6
  // chiptyp[.chipunittype]:cage:node:slot:position[:chipunitnum[:thread]
  // 0                      1    2    3    4         5            6

  if ((tokens.size() >= 2) && (tokens[0].substr(0, 1) != "k") && (tokens[1].substr(0, 1) != "n"))
  {
      l_alt_format_used = true;
  }

  for (uint32_t x = 0; x < tokens.size(); x++) {
    allFound = false;
    naFound = false;
    //numFound = false;
    if (tokens[x].substr(1,tokens[x].length()) == "all") {
      allFound = true;
    } else if (tokens[x].substr(1,tokens[x].length()) == "-") {
      naFound = true;
    }
    else if (tokens[x].find_first_not_of("0123456789",1) == std::string::npos) {
      sscanf(tokens[x].substr(1,tokens[x].length()).c_str(), "%d", &num);
      //numFound = true;
    }

    switch (x) {
	case 0:
	    {
		if (l_alt_format_used)
		{
		    rc = ecmdParseChipField(tokens[x], o_target.chipType, o_target.chipUnitType); if (rc) return rc;
		    if (o_target.chipType == "chipall") {
			o_target.chipType = ""; /* Blank it for clarity */
			o_target.chipTypeState = ECMD_TARGET_FIELD_WILDCARD;
		    } else {
			o_target.chipTypeState = ECMD_TARGET_FIELD_VALID;
			if (o_target.chipUnitType != "") {
			    o_target.chipUnitTypeState = ECMD_TARGET_FIELD_VALID;
			} else {
			    o_target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;
			}
		    }
		}
		else
		{
		    if (tokens[x].substr(0, 1) == "k") {
			if (allFound) {
			    o_target.cageState = ECMD_TARGET_FIELD_WILDCARD;
			} else {
			    o_target.cage = num;
			    o_target.cageState = ECMD_TARGET_FIELD_VALID;
			}
		    } else {
			// ERROR
			return ECMD_INVALID_ARGS;
		    }

		}
	    }
        break;
      case 1:
        {
	    if (l_alt_format_used)
	    {
		if (tokens[x].substr(0, 1) == "k") {
		    if (allFound) {
			o_target.cageState = ECMD_TARGET_FIELD_WILDCARD;
		    } else {
			o_target.cage = num;
			o_target.cageState = ECMD_TARGET_FIELD_VALID;
		    }
		} else {
		    // ERROR
		    return ECMD_INVALID_ARGS;
		}


	    }
	    else
	    {
		if (tokens[x].substr(0, 1) == "n") {
		    if (allFound) {
			o_target.nodeState = ECMD_TARGET_FIELD_WILDCARD;
		    } else if (naFound) {
			o_target.nodeState = ECMD_TARGET_FIELD_WILDCARD;
		    } else {
			if (naFound) {
			    o_target.node = ECMD_TARGETDEPTH_NA;
			} else {
			    o_target.node = num;
			}
			o_target.nodeState = ECMD_TARGET_FIELD_VALID;
		    }
		} else {
		    // ERROR
		    return ECMD_INVALID_ARGS;
		}
	    }
        }
        break;
      case 2:
        {
	    if (l_alt_format_used)
	    {
		if (tokens[x].substr(0, 1) == "n") {
		    if (allFound) {
			o_target.nodeState = ECMD_TARGET_FIELD_WILDCARD;
		    } else if (naFound) {
			o_target.nodeState = ECMD_TARGET_FIELD_WILDCARD;
		    } else {
			if (naFound) {
			    o_target.node = ECMD_TARGETDEPTH_NA;
			} else {
			    o_target.node = num;
			}
			o_target.nodeState = ECMD_TARGET_FIELD_VALID;
		    }
		} else {
		    // ERROR
		    return ECMD_INVALID_ARGS;
		}
	    }
	    else
	    {
		if (tokens[x].substr(0, 1) == "s") {
		    if (allFound) {
			o_target.slotState = ECMD_TARGET_FIELD_WILDCARD;
		    } else {
			if (naFound) {
			    o_target.slot = ECMD_TARGETDEPTH_NA;
			} else {
			    o_target.slot = num;
			}
			o_target.slotState = ECMD_TARGET_FIELD_VALID;
		    }
		} else {
		    // ERROR
		    return ECMD_INVALID_ARGS;
		}

	    }
        }
        break;
      case 3:
        {
	    if (l_alt_format_used)
	    {
		if (tokens[x].substr(0, 1) == "s") {
		    if (allFound) {
			o_target.slotState = ECMD_TARGET_FIELD_WILDCARD;
		    } else {
			if (naFound) {
			    o_target.slot = ECMD_TARGETDEPTH_NA;
			} else {
			    o_target.slot = num;
			}
			o_target.slotState = ECMD_TARGET_FIELD_VALID;
		    }
		} else {
		    // ERROR
		    return ECMD_INVALID_ARGS;
		}

	    }
	    else
	    {
		rc = ecmdParseChipField(tokens[x], o_target.chipType, o_target.chipUnitType); if (rc) return rc;
		if (o_target.chipType == "chipall") {
		    o_target.chipType = ""; /* Blank it for clarity */
		    o_target.chipTypeState = ECMD_TARGET_FIELD_WILDCARD;
		} else {
		    o_target.chipTypeState = ECMD_TARGET_FIELD_VALID;
		    if (o_target.chipUnitType != "") {
			o_target.chipUnitTypeState = ECMD_TARGET_FIELD_VALID;
		    } else {
			o_target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;
		    }
		}
	    }
        }
        break;
      case 4:
        {
          if (tokens[x].substr(0, 1) == "p") {
            if (allFound) {
              o_target.posState = ECMD_TARGET_FIELD_WILDCARD;
            } else {
              o_target.pos = num;
              o_target.posState = ECMD_TARGET_FIELD_VALID;
            }
          } else {
            // ERROR
            return ECMD_INVALID_ARGS;
          }
        }
        break;
      case 5:
        {
          if (tokens[x].substr(0, 1) == "c") {
            if (allFound) {
              o_target.chipUnitNumState = ECMD_TARGET_FIELD_WILDCARD;
            } else {
              o_target.chipUnitNum = num;
              o_target.chipUnitNumState = ECMD_TARGET_FIELD_VALID;
            }
          } else {
            // ERROR
            return ECMD_INVALID_ARGS;
          }
        }
        break;
      case 6:
        {
          if (tokens[x].substr(0, 1) == "t") {
            if (allFound) {
              o_target.threadState = ECMD_TARGET_FIELD_WILDCARD;
            } else {
              o_target.thread = num;
              o_target.threadState = ECMD_TARGET_FIELD_VALID;
            }
          } else {
            // ERROR
            return ECMD_INVALID_ARGS;
          }
        }
        break;
      default:
        {
          // ERROR
          return ECMD_INVALID_ARGS;
        }
        break;
    }
  }

  return rc;
}

uint32_t ecmdParseChipField(std::string i_chipField, std::string &o_chipType, std::string &o_chipUnitType, bool i_supportsWildcard) {
  uint32_t rc = ECMD_SUCCESS;

  /* See if the chipUnit separator (the period) is found.  If it is, then break up the input field.
     if it is not, then just return the chipType 
  */
  size_t dotLinePos = i_chipField.find(".");
  size_t wildcardLinePos = i_chipField.find("x");

  if (dotLinePos == std::string::npos) {
    /* chipType was specified without a chipUnitType */
    o_chipType = i_chipField.substr(0, dotLinePos);
    o_chipUnitType = "";
    
    /* If the chipType is 1 char long let's make sure it's the eCMD support wildcard char 'x' */
    if (i_chipField.size() == 1){
      if (wildcardLinePos == std::string::npos){
        /* One char was used for the chipType but it wasn't the eCMD wildcard char */
        o_chipType = i_chipField;
        o_chipUnitType = "";
      } else {
        if (i_supportsWildcard){
          o_chipType = "x";
          o_chipUnitType = "";
        } else {
          /* Wildcard found but it's not supported */ 
          return ECMD_WILDCARD_CHAR_NOT_SUPPORTED;
        }
      }
    }
  } else {
    /* chipUnitType was specified */
    o_chipType = i_chipField.substr(0, dotLinePos);
    o_chipUnitType = i_chipField.substr((dotLinePos+1), i_chipField.length());

    /* Split <chipType>.<chipUnitType> into two separate strings */
    std::string localChipType = o_chipType;
    std::string localChipUnitType = o_chipUnitType;
    
    /* Check the chipType field for eCMD wildcard char */
    if (localChipType.size() == 1 && localChipType == "x"){
      if (i_supportsWildcard) {
        /* Wildcard found in chipType field */
        o_chipType = "x";
      } else {
        /* Wildcard found but it's not supported */ 
        return ECMD_WILDCARD_CHAR_NOT_SUPPORTED;
      }
    }

    /* Check the chipUnitType field for eCMD wildcard char */
    if (localChipUnitType.size() == 1 && localChipUnitType == "x"){
    #if 0 
      if (i_supportsWildcard) {
        /* Wildcard found in chipUnitType field */
        o_chipUnitType = "x";
      } else {
        /* Wildcard found but it's not supported */ 
     #endif 
        return ECMD_WILDCARD_MISUSE;
     // }
    }

  }

  return rc;
}

uint32_t ecmdReadDcard(const char *i_filename, std::list<ecmdMemoryEntry> &o_data, uint64_t i_addressOffset) {
  std::ifstream ins;
  std::string line;
  std::vector<std::string> splitArgs;
  int rc = 0;
  uint32_t databitlength=0, tagbitlength=0, bitlength=0;
  uint64_t curaddress, nextaddress=0;
  uint32_t tagsBitOffset =0, startOffset =0;
  bool isFirstTimeInLoop = true, setTag = true;
  std::list<ecmdMemoryEntry> memdata;        //ListElements corresponding to every line in file
  std::list<ecmdMemoryEntry>::iterator memdataIter;
  std::list<ecmdMemoryEntry>::iterator o_dataIter;
  ecmdMemoryEntry dcardEntry;
  
  ins.open(i_filename);
  
  if (ins.fail()) {
    return ECMD_DBUF_FOPEN_FAIL;  
  }
  
  //Loop to ealk thru the file and set the Data databuffers bitlengths
  while (!ins.eof()) {
    ecmdMemoryEntry dcardEntryForEveryLine;

    getline(ins, line, '\n');
    ecmdParseTokens(line, " \t\n", splitArgs);

    if (splitArgs.size() == 0) continue;
    if ((splitArgs.size() < 3) || (splitArgs.size() > 4)) {
      continue;
    }

    //Setup the dcardEntryForEveryLine 
    curaddress = strtoull(splitArgs[1].c_str(), NULL, 16);
    curaddress += i_addressOffset;
    dcardEntryForEveryLine.address = curaddress;

    bitlength = splitArgs[2].length() * 4;
    dcardEntryForEveryLine.data.setBitLength(bitlength);
    rc = dcardEntryForEveryLine.data.insertFromHexLeft(splitArgs[2].c_str(), 0, bitlength);
    if (rc) return rc;

    if (splitArgs.size() == 4) {
      dcardEntryForEveryLine.tags.setBitLength(1);
      if (splitArgs[3] == "1") {
        dcardEntryForEveryLine.tags.setBit(0);
      } else if (splitArgs[3] == "0") {
        dcardEntryForEveryLine.tags.clearBit(0);
      } else {
        return ECMD_INVALID_ARGS; //tag neither 0 nor 1
      }
    } else { dcardEntryForEveryLine.tags.setBitLength(0); }

    memdata.push_back(dcardEntryForEveryLine);

    //Combine the lines if contiguous address
    if (isFirstTimeInLoop) {
      isFirstTimeInLoop = false;
      dcardEntry.address = curaddress;
    } else {
      if (curaddress != nextaddress) {  
        //found a hole, so setup the buffer with bitlength so far 
        dcardEntry.data.setBitLength(databitlength);
        dcardEntry.tags.setBitLength(tagbitlength);
        o_data.push_back(dcardEntry);

        //Set it up for the next data buffer
        dcardEntry.address = curaddress;
        databitlength = 0;
        tagbitlength = 0;
        setTag = true;
      } 
    }
    //If data bit length is not 64 OR tags are not present in the line then
    //set the tag buffer bitlength to 0 and ignore the tags for that buffer
    //For every 64 bits of data there's 1 tag bit
    if ((bitlength != 64) || (dcardEntryForEveryLine.tags.getBitLength() == 0) ) {
      tagbitlength = 0;
      setTag = false;
    }
    databitlength += bitlength;
    if (setTag) tagbitlength++;

    nextaddress = dcardEntry.address + databitlength/8;
  }
  //Set the last one from the loop
  dcardEntry.data.setBitLength(databitlength);
  dcardEntry.tags.setBitLength(tagbitlength);
  o_data.push_back(dcardEntry);
  
  ins.close();

  //Loop to set the buffers in the list
  o_dataIter = o_data.begin();
  for (memdataIter = memdata.begin(); memdataIter != memdata.end(); memdataIter++) {
   
   if (startOffset >= o_dataIter->data.getBitLength()) {
    startOffset = 0;
    tagsBitOffset = 0;
    if (o_dataIter != o_data.end()) {
      o_dataIter++;
    } else { break; }
   }
   memdataIter->data.extractPreserve(o_dataIter->data, 0,  memdataIter->data.getBitLength(), startOffset);
   startOffset += memdataIter->data.getBitLength();
   
   if (o_dataIter->tags.getBitLength() != 0) {
     memdataIter->tags.extractPreserve(o_dataIter->tags, 0,  memdataIter->tags.getBitLength(), tagsBitOffset);
     tagsBitOffset++;
   }
   
  }
  
  
  return rc;
}

uint32_t ecmdPadAndMerge(std::list<ecmdMemoryEntry> &io_data, uint32_t i_size)
{
    uint32_t rc = 0;

    // mask values for address checking based on cache line size
    uint64_t blockalignmask = i_size - 1;
    uint64_t blockstartmask = ~blockalignmask;

    // create map of input data
    std::map<uint64_t, std::list<ecmdMemoryEntry>::iterator> representation;
    std::list<ecmdMemoryEntry>::iterator io_dataIter;
    for (io_dataIter = io_data.begin(); io_dataIter != io_data.end(); io_dataIter++)
    {
        representation[io_dataIter->address] = io_dataIter;
    }

    std::map<uint64_t, std::list<ecmdMemoryEntry>::iterator>::iterator repIter = representation.begin();
    while(repIter != representation.end())
    {
#ifdef DEBUG_MEM_MERGE
#ifndef UINT64_HEX_FORMAT
  #define UINT64_HEX_FORMAT "%lX"
#endif
        printf("block " UINT64_HEX_FORMAT ",%X %s\n",
            repIter->first, repIter->second->data.getByteLength(),
            repIter->second->data.genHexLeftStr().c_str());
#endif
        uint64_t blockstart = repIter->first & blockstartmask;
        if (blockstart != repIter->first)
        {
            // extend current iter forward
            uint64_t prepend = repIter->first - blockstart;
#ifdef DEBUG_MEM_MERGE
            uint64_t newlength = repIter->second->data.getByteLength() + prepend;
            printf("prepend change " UINT64_HEX_FORMAT ",%X to " UINT64_HEX_FORMAT "," UINT64_HEX_FORMAT "\n",
                repIter->first, repIter->second->data.getByteLength(), blockstart, newlength);
#endif
            // record change
            rc = repIter->second->data.shiftRightAndResize(prepend * 8);
            if (rc) return rc;
            repIter->second->address = blockstart;
            std::pair<std::map<uint64_t, std::list<ecmdMemoryEntry>::iterator>::iterator, bool> insertresult =
                representation.insert(std::pair<uint64_t, std::list<ecmdMemoryEntry>::iterator>(blockstart, repIter->second));
            representation.erase(repIter);
            repIter = insertresult.first;

            // check if previous block is now adjacent
            if (repIter != representation.begin())
            {
                std::map<uint64_t, std::list<ecmdMemoryEntry>::iterator>::iterator prevIter = repIter;
                prevIter--;
                if ((prevIter->first + prevIter->second->data.getByteLength()) == repIter->first)
                {
                    // switch back to previous block and continue
                    repIter = prevIter;
#ifdef DEBUG_MEM_MERGE
                    printf("rewinding\n");
#endif
                    continue;
                }
            }
        }

        // check if ends on cache line
        if (repIter->second->data.getByteLength() & blockalignmask)
        {
            // does not end on cache line
            uint64_t curstop = blockstart + repIter->second->data.getByteLength();
            uint64_t blockstop = (curstop & 0xFFFFFFFFFFFFFF80ull) + 0x80ull;
            // check if next entry is inside current data cache line
            std::map<uint64_t, std::list<ecmdMemoryEntry>::iterator>::iterator nextIter = repIter;
            nextIter++;
            if (nextIter != representation.end())
            {
                if (nextIter->first < blockstop)
                {
                    // check what padding is needed between blocks
                    uint64_t paddingsize = nextIter->first - curstop;
#ifdef DEBUG_MEM_MERGE
                    printf("merge " UINT64_HEX_FORMAT ",%X %s with " UINT64_HEX_FORMAT ",%X %s\n",
                        repIter->first, repIter->second->data.getByteLength(), repIter->second->data.genHexLeftStr().c_str(),
                        nextIter->first, nextIter->second->data.getByteLength(), nextIter->second->data.genHexLeftStr().c_str());
#endif
                    // merge this block with next
                    uint32_t originallength = repIter->second->data.getBitLength();
                    rc = repIter->second->data.growBitLength(originallength + (8 * paddingsize) + nextIter->second->data.getBitLength());
                    if (rc) return rc;
                    rc = repIter->second->data.insert(nextIter->second->data, originallength + (8 * paddingsize), nextIter->second->data.getBitLength());
                    if (rc) return rc;
                    io_data.erase(nextIter->second);
                    representation.erase(nextIter);
                    continue; // recheck current block after merge
                }
            }

            // otherwise extend block to end of cache line
            uint64_t postpend = blockstop - curstop;
            uint64_t newlength = repIter->second->data.getByteLength() + postpend;
#ifdef DEBUG_MEM_MERGE
            printf("postpend change " UINT64_HEX_FORMAT ",%X to " UINT64_HEX_FORMAT "," UINT64_HEX_FORMAT "\n",
                repIter->first, repIter->second->data.getByteLength(), repIter->first, newlength);
#endif
            repIter->second->data.growBitLength(8 * newlength);
        }

        // check if next block is adjacent
        std::map<uint64_t, std::list<ecmdMemoryEntry>::iterator>::iterator nextIter = repIter;
        nextIter++;
        if (nextIter != representation.end())
        {
            if ((repIter->first + repIter->second->data.getByteLength()) == nextIter->first)
            {
#ifdef DEBUG_MEM_MERGE
                printf("merge " UINT64_HEX_FORMAT ",%X %s with " UINT64_HEX_FORMAT ",%X %s\n",
                    repIter->first, repIter->second->data.getByteLength(), repIter->second->data.genHexLeftStr().c_str(),
                    nextIter->first, nextIter->second->data.getByteLength(), nextIter->second->data.genHexLeftStr().c_str());
#endif
                // merge this block with next
                uint32_t originallength = repIter->second->data.getBitLength();
                rc = repIter->second->data.growBitLength(originallength + nextIter->second->data.getBitLength());
                if (rc) return rc;
                rc = repIter->second->data.insert(nextIter->second->data, originallength, nextIter->second->data.getBitLength());
                if (rc) return rc;
                io_data.erase(nextIter->second);
                representation.erase(nextIter);
                continue; // recheck current block after merge
            }
        }
        repIter++;
    }

#ifdef DEBUG_MEM_MERGE
    for (io_dataIter = io_data.begin(); io_dataIter != io_data.end(); io_dataIter++)
    {
        printf("resulting block " UINT64_HEX_FORMAT ",%X\n",
            io_dataIter->address, io_dataIter->data.getByteLength());
    }
#endif

    return rc;
}

uint32_t ecmdGenB32FromHex (uint32_t * o_numPtr, const char * i_hexChars, int startPos) {

  uint32_t tempB32 = 0;
  uint32_t counter = 0;
  char	twoChars[2] = "0";

  if ((o_numPtr == NULL) || (i_hexChars == NULL) || (strlen(i_hexChars) == 0))
    return 0xFFFFFFFF;

  o_numPtr[0] = 0x0;
  o_numPtr[startPos>>3]= 0x0;

  for (counter = 0; counter < strlen(i_hexChars); counter++) {
    if (((counter + startPos) & 0xFFF8) == (counter + startPos))
      o_numPtr[(counter+startPos)>>3] = 0x0;
    twoChars[0] = i_hexChars[counter];
    tempB32 = strtoul((char *) & twoChars[0], NULL, 16);
    o_numPtr[(counter+startPos)>>3] |= (tempB32 << (28 - (4 * ((counter + startPos) & 0x07))));
  }

  return o_numPtr[0];

}


uint32_t ecmdGenB32FromHexLeft (uint32_t * o_numPtr, const char * i_hexChars) {

  if ((i_hexChars == NULL) || (o_numPtr == NULL))
    return 0xFFFFFFFF;

  return ecmdGenB32FromHex(o_numPtr, i_hexChars, 0);

}

uint32_t ecmdGenB32FromHexRight (uint32_t * o_numPtr, const char * i_hexChars, int expectBits) {

  int stringSize;

  if ((i_hexChars == NULL) || (o_numPtr == NULL))
    return 0xFFFFFFFF;

  /* ----------------------------------------------------------------- */	
  /* Telling us we are expecting few bits than the user input          */	
  /* ----------------------------------------------------------------- */	
  stringSize = strlen(i_hexChars);

  if (expectBits==0) 
    expectBits = ((stringSize << 2) + 31) & 0xFFFE0;

  /* ----------------------------------------------------------------- */
  /* Can't figure out if this case is bad...                           */
  /* ----------------------------------------------------------------- */
  if (expectBits<(stringSize*4)) 
    expectBits = (4-(expectBits & 0x3))+expectBits+((stringSize*4-expectBits)&0xFFFC);

  expectBits = (expectBits - stringSize*4)>>2; 

  return ecmdGenB32FromHex(o_numPtr, i_hexChars, expectBits );


}

