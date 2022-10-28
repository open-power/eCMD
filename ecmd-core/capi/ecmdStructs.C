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


//----------------------------------------------------------------------
//  Includes
//----------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <inttypes.h>
#include <netinet/in.h>

#include <ecmdReturnCodes.H>
#include <ecmdStructs.H>

#ifndef _AIX
  #include <byteswap.h>
  #ifndef htonll
    #if BYTE_ORDER == BIG_ENDIAN
      #define htonll(x) (x)
    #else
      #define htonll(x) bswap_64(x)
    #endif
  #endif

  #ifndef ntohll
    #if BYTE_ORDER == BIG_ENDIAN
      #define ntohll(x) (x)
    #else
      #define ntohll(x) ((((uint64_t)ntohl(x)) << 32) + ntohl(x >> 32))
    #endif
  #endif
#endif

//----------------------------------------------------------------------
//  Constants
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//  Forward declarations
//----------------------------------------------------------------------

//---------------------------------------------------------------------
//  Constructors
//---------------------------------------------------------------------

//---------------------------------------------------------------------
//  Destructor
//---------------------------------------------------------------------

//---------------------------------------------------------------------
//  Public Member Function Specifications
//---------------------------------------------------------------------
std::string ecmdGetSharedLibVersion() {
  return ECMD_CAPI_VERSION;
}

/** @brief Used to compare ecmdChipTarget targeting variables that have their state
 field set to valid.  If the state field is not set to valid on both sides, a comparision is not done
 */
int ecmdChipTarget::operator==(const ecmdChipTarget& rhs) const {
  int equal = 1; // Assume a match to start

  /* Use a breakout loop so we can bail on the comparisions once we find an invalid case */
  /* The check for VALID with an || and then comparing them may seem redundant, but it is not.
   The previous code had just a staight check && that both were valid, that caused problems when
   one was valid and the other wasn't.  The comparision would never be done, defaulting to true.
   By doing the check this way, if either is valid, but both aren't, we'll flag that as a mismatch.
   This diatribe added by JTA 05/19/08 */
  do {
    /* CAGE */
    if (cageState == ECMD_TARGET_FIELD_VALID || rhs.cageState == ECMD_TARGET_FIELD_VALID) {
      if ((cage != rhs.cage) || (cageState != rhs.cageState)) {
        equal = false;
        break;
      }
    }

    /* NODE */
    if (nodeState == ECMD_TARGET_FIELD_VALID || rhs.nodeState == ECMD_TARGET_FIELD_VALID) {
      if ((node != rhs.node) || (nodeState != rhs.nodeState)) {
        equal = false;
        break;
      }
    }

    /* SLOT */
    if (slotState == ECMD_TARGET_FIELD_VALID || rhs.slotState == ECMD_TARGET_FIELD_VALID) {
      if ((slot != rhs.slot) || (slotState != rhs.slotState)) {
        equal = false;
        break;
      }
    }

    /* CHIPTYPE */
    if (chipTypeState == ECMD_TARGET_FIELD_VALID || rhs.chipTypeState == ECMD_TARGET_FIELD_VALID) {
      if ((chipType != rhs.chipType) || (chipTypeState != rhs.chipTypeState)) {
        equal = false;
        break;
      }
    }

    /* POS */
    if (posState == ECMD_TARGET_FIELD_VALID || rhs.posState == ECMD_TARGET_FIELD_VALID) {
      if ((pos != rhs.pos) || (posState != rhs.posState)) {
        equal = false;
        break;
      }
    }

    /* CHIPUNIT TYPE */
    if (chipUnitTypeState == ECMD_TARGET_FIELD_VALID || rhs.chipUnitTypeState == ECMD_TARGET_FIELD_VALID) {
      if ((chipUnitType != rhs.chipUnitType) || (chipUnitTypeState != rhs.chipUnitTypeState)) {
        equal = false;
        break;
      }
    }

    /* CHIPUNIT NUM */
    if (chipUnitNumState == ECMD_TARGET_FIELD_VALID || rhs.chipUnitNumState == ECMD_TARGET_FIELD_VALID) {
      if ((chipUnitNum != rhs.chipUnitNum) || (chipUnitNumState != rhs.chipUnitNumState)) {
        equal = false;
        break;
      }
    }

    /* THREAD */
    if (threadState == ECMD_TARGET_FIELD_VALID || rhs.threadState == ECMD_TARGET_FIELD_VALID) {
      if ((thread != rhs.thread) || (threadState != rhs.threadState)) {
        equal = false;
        break;
      }
    }
  } while (0);
        
  return equal;
}

/** @brief Used to sort ecmdChipTarget variables */
bool ecmdChipTarget::operator<(const ecmdChipTarget& rhs) const {

  /* CAGE */
  if (cage < rhs.cage) {
    /* NODE */
    if (node < rhs.node) {
      /* SLOT */
      if (slot < rhs.slot) {
        /* CHIPTYPE */
        if (chipType < rhs.chipType) {
          /* POS */
          if (pos < rhs.pos) {
            /* CHIPUNIT TYPE */
            if (chipUnitType < rhs.chipUnitType) {
              /* CHIPUNIT NUM */
              if (chipUnitNum < rhs.chipUnitNum) {
                /* THREAD */
                if (thread < rhs.thread) {
                  /* We made it all the way down here, we've got a winner */
                  return true;
                }
              }
            }
          }
        }
      }
    }
  }

  /* We failed up above, return false */
  return false;
}

/*
 * This function will flatten the ecmdChipTarget struct for transport accross
 * the user <-> cecserver interface.  
 */
uint32_t ecmdChipTarget::flatten(uint8_t *o_buf, uint32_t i_len) const {

	uint32_t rc    = ECMD_SUCCESS;
	int      l_len = (int) i_len;
	uint8_t *l_ptr = (uint8_t *) o_buf;

	ecmdChipTarget tmpTarget;

	do {	// Single entry, single exit.

		// Check for buffer overflow conditions.
		if (this->flattenSize() <= i_len) {
			// Make sure buffer is clear.
			memset(o_buf, 0, i_len);
		} else {
			// Handle the error case for buffer overflow.
			ETRAC("Buffer overflow occured - "
			       "structure size = %d; input length = %d", 
			       this->flattenSize(), i_len);
			rc = ECMD_DATA_OVERFLOW;
			break;
		}

		// Copy everything into temporary struct in htonl format.
		tmpTarget.cage	  = htonl(cage);
		tmpTarget.node	  = htonl(node);
		tmpTarget.slot	  = htonl(slot);
		tmpTarget.pos	   = htonl(pos);
		tmpTarget.chipUnitNum  = chipUnitNum;
		tmpTarget.thread	= thread;
		tmpTarget.unitId	= htonl(unitId);
		tmpTarget.cageState     = (ecmdChipTargetState_t) htonl((uint32_t)cageState);
		tmpTarget.nodeState     = (ecmdChipTargetState_t) htonl((uint32_t)nodeState);
		tmpTarget.slotState     = (ecmdChipTargetState_t) htonl((uint32_t)slotState);
		tmpTarget.chipTypeState = (ecmdChipTargetState_t) htonl((uint32_t)chipTypeState);
		tmpTarget.posState      = (ecmdChipTargetState_t) htonl((uint32_t)posState);
		tmpTarget.chipUnitNumState = (ecmdChipTargetState_t) htonl((uint32_t)chipUnitNumState);
        tmpTarget.chipUnitTypeState     = (ecmdChipTargetState_t) htonl((uint32_t)chipUnitTypeState);
 		tmpTarget.threadState   = (ecmdChipTargetState_t) htonl((uint32_t)threadState);
		tmpTarget.unitIdState   = (ecmdChipTargetState_t) htonl((uint32_t)unitIdState);
		tmpTarget.chipType	= chipType;
        tmpTarget.chipUnitType  = chipUnitType;

		// Now copy everything to the output buffer.
		memcpy(l_ptr, &(tmpTarget.cage), sizeof(tmpTarget.cage));
		l_ptr += sizeof(tmpTarget.cage);
		l_len -= sizeof(tmpTarget.cage);

		memcpy(l_ptr, &(tmpTarget.node), sizeof(tmpTarget.node));
		l_ptr += sizeof(tmpTarget.node);
		l_len -= sizeof(tmpTarget.node);
	
		memcpy(l_ptr, &(tmpTarget.slot), sizeof(tmpTarget.slot));
		l_ptr += sizeof(tmpTarget.slot);
		l_len -= sizeof(tmpTarget.slot);

		memcpy(l_ptr, &(tmpTarget.pos), sizeof(tmpTarget.pos));
		l_ptr += sizeof(tmpTarget.pos);
		l_len -= sizeof(tmpTarget.pos);

		memcpy(l_ptr, &(tmpTarget.chipUnitNum), sizeof(tmpTarget.chipUnitNum));
		l_ptr += sizeof(tmpTarget.chipUnitNum);
		l_len -= sizeof(tmpTarget.chipUnitNum);

		memcpy(l_ptr, &(tmpTarget.thread), sizeof(tmpTarget.thread));
		l_ptr += sizeof(tmpTarget.thread);
		l_len -= sizeof(tmpTarget.thread);

		memcpy(l_ptr, &(tmpTarget.unitId), sizeof(tmpTarget.unitId));
		l_ptr += sizeof(tmpTarget.unitId);
		l_len -= sizeof(tmpTarget.unitId);

		memcpy(l_ptr, 
		       &(tmpTarget.cageState), 
		       sizeof(tmpTarget.cageState));
		l_ptr += sizeof(tmpTarget.cageState);
		l_len -= sizeof(tmpTarget.cageState);

		memcpy(l_ptr, 
		       &(tmpTarget.nodeState), 
		       sizeof(tmpTarget.nodeState));
		l_ptr += sizeof(tmpTarget.nodeState);
		l_len -= sizeof(tmpTarget.nodeState);

		memcpy(l_ptr, 
		       &(tmpTarget.slotState), 
		       sizeof(tmpTarget.slotState));
		l_ptr += sizeof(tmpTarget.slotState);
		l_len -= sizeof(tmpTarget.slotState);

		memcpy(l_ptr, 
		       &(tmpTarget.chipTypeState), 
		       sizeof(tmpTarget.chipTypeState));
		l_ptr += sizeof(tmpTarget.chipTypeState);
		l_len -= sizeof(tmpTarget.chipTypeState);

		memcpy(l_ptr, 
		       &(tmpTarget.posState), 
		       sizeof(tmpTarget.posState));
		l_ptr += sizeof(tmpTarget.posState);
		l_len -= sizeof(tmpTarget.posState);

		memcpy(l_ptr, 
		       &(tmpTarget.chipUnitNumState),
		       sizeof(tmpTarget.chipUnitNumState));
		l_ptr += sizeof(tmpTarget.chipUnitNumState);
		l_len -= sizeof(tmpTarget.chipUnitNumState);

        memcpy(l_ptr,
               &(tmpTarget.chipUnitTypeState),
               sizeof(tmpTarget.chipUnitTypeState));
        l_ptr += sizeof(tmpTarget.chipUnitTypeState);
        l_len -= sizeof(tmpTarget.chipUnitTypeState);

		memcpy(l_ptr, 
		       &(tmpTarget.threadState), 
		       sizeof(tmpTarget.threadState));
		l_ptr += sizeof(tmpTarget.threadState);
		l_len -= sizeof(tmpTarget.threadState);

		memcpy(l_ptr, 
		       &(tmpTarget.unitIdState), 
		       sizeof(tmpTarget.unitIdState));
		l_ptr += sizeof(tmpTarget.unitIdState);
		l_len -= sizeof(tmpTarget.unitIdState);

		memcpy(l_ptr, 
		       tmpTarget.chipType.c_str(),
		       tmpTarget.chipType.size() + 1);  // +1 for sizeof NULL.
		l_ptr += tmpTarget.chipType.size() + 1;
		l_len -= tmpTarget.chipType.size() +1;

       memcpy(l_ptr,
               tmpTarget.chipUnitType.c_str(),
               tmpTarget.chipUnitType.size() + 1);  // +1 for sizeof NULL.
        l_ptr += tmpTarget.chipUnitType.size() + 1;
        l_len -= tmpTarget.chipUnitType.size() +1;

	} while (0);	// Single exit point.

	return rc; 

}	// End ecmdChipTarget::flatten()

uint32_t ecmdChipTarget::unflatten(const uint8_t *i_buf, uint32_t i_len) {

	uint32_t rc    = ECMD_SUCCESS;
	int      l_len = (int) i_len;
	uint8_t *l_ptr = (uint8_t *)i_buf;

	// Find offset to chipType string.
	uint8_t l_byteCount = sizeof(cage)
			      + sizeof(node)
			      + sizeof(slot)
			      + sizeof(pos)
			      + sizeof(chipUnitNum)
			      + sizeof(thread)
			      + sizeof(unitId)
			      + sizeof(cageState)
			      + sizeof(nodeState)
			      + sizeof(slotState)
			      + sizeof(chipTypeState)
			      + sizeof(posState)
			      + sizeof(chipUnitNumState)
                  + sizeof(chipUnitTypeState)
 			      + sizeof(threadState)
			      + sizeof(unitIdState);

	const char *l_chipTypePtr = (char *)(l_ptr + l_byteCount);
    const char *l_chipUnitTypePtr = (char*)(l_ptr + (l_byteCount + strlen(l_chipTypePtr) + 1));
	
	l_ptr = (uint8_t *)i_buf;
	// Finish calculating the size of the structure to be built.
	l_byteCount += (strlen(l_chipTypePtr) + 1);  // +1 for NULL terminator.
    l_byteCount += (strlen(l_chipUnitTypePtr) + 1);  // +1 for NULL terminator.
	do {	// Single entry ->

		if (l_byteCount > i_len) {
			// Generate an error for buffer overflow conditions.
			ETRAC("Buffer overflow occured - "
                               "estimated structure size = %d; "
			       "input length = %d",
                               l_byteCount, i_len);
			rc = ECMD_DATA_OVERFLOW;
			break;
		}
		
		memcpy(&cage, l_ptr, sizeof(cage));
		cage = ntohl(cage);
		l_ptr += sizeof(cage);
		l_len -= sizeof(cage);

		memcpy(&node, l_ptr, sizeof(node));
		node = ntohl(node);
		l_ptr += sizeof(node);
		l_len -= sizeof(node);

		memcpy(&slot, l_ptr, sizeof(slot));
		slot = ntohl(slot);
		l_ptr += sizeof(slot);
		l_len -= sizeof(slot);

		memcpy(&pos, l_ptr, sizeof(pos));
		pos = ntohl(pos);
		l_ptr += sizeof(pos);
		l_len -= sizeof(pos);

		memcpy(&chipUnitNum, l_ptr, sizeof(chipUnitNum));
		l_ptr += sizeof(chipUnitNum);
		l_len -= sizeof(chipUnitNum);

		memcpy(&thread, l_ptr, sizeof(thread));
		l_ptr += sizeof(thread);
		l_len -= sizeof(thread);

		memcpy(&unitId, l_ptr, sizeof(unitId));
		unitId = ntohl(unitId);
		l_ptr += sizeof(unitId);
		l_len -= sizeof(unitId);

		memcpy(&cageState, l_ptr, sizeof(cageState));
		cageState = (ecmdChipTargetState_t) ntohl((uint32_t) cageState);
		l_ptr += sizeof((uint32_t) cageState);
		l_len -= sizeof((uint32_t) cageState);

		memcpy(&nodeState, l_ptr, sizeof(nodeState));
		nodeState = (ecmdChipTargetState_t) ntohl((uint32_t) nodeState);
		l_ptr += sizeof((uint32_t) nodeState);
		l_len -= sizeof((uint32_t) nodeState);

		memcpy(&slotState, l_ptr, sizeof(slotState));
		slotState = (ecmdChipTargetState_t) ntohl((uint32_t) slotState);
		l_ptr += sizeof((uint32_t) slotState);
		l_len -= sizeof((uint32_t) slotState);

		memcpy(&chipTypeState, l_ptr, sizeof(chipTypeState));
		chipTypeState = (ecmdChipTargetState_t) ntohl(
						      (uint32_t) chipTypeState);
		l_ptr += sizeof((uint32_t) chipTypeState);
		l_len -= sizeof((uint32_t) chipTypeState);

		memcpy(&posState, l_ptr, sizeof(posState));
		posState = (ecmdChipTargetState_t) ntohl((uint32_t) posState);
		l_ptr += sizeof((uint32_t) posState);
		l_len -= sizeof((uint32_t) posState);

		memcpy(&chipUnitNumState, l_ptr, sizeof(chipUnitNumState));
		chipUnitNumState = (ecmdChipTargetState_t) ntohl((uint32_t) chipUnitNumState);
		l_ptr += sizeof((uint32_t) chipUnitNumState);
		l_len -= sizeof((uint32_t) chipUnitNumState);

        memcpy(&chipUnitTypeState, l_ptr, sizeof(chipUnitTypeState));
        chipUnitTypeState = (ecmdChipTargetState_t) ntohl((uint32_t) chipUnitTypeState);
        l_ptr += sizeof((uint32_t) chipUnitTypeState);
        l_len -= sizeof((uint32_t) chipUnitTypeState);

		memcpy(&threadState, l_ptr, sizeof(threadState));
		threadState = (ecmdChipTargetState_t) ntohl(
							(uint32_t) threadState);
		l_ptr += sizeof((uint32_t) threadState);
		l_len -= sizeof((uint32_t) threadState);

		memcpy(&unitIdState, l_ptr, sizeof(unitIdState));
		unitIdState = (ecmdChipTargetState_t) ntohl(
							(uint32_t) unitIdState);
		l_ptr += sizeof((uint32_t) unitIdState);
		l_len -= sizeof((uint32_t) unitIdState);

		chipType = (char *) l_ptr;
        l_ptr += (chipType.size() + 1);
		l_len -= (chipType.size() + 1);

        chipUnitType = (char *) l_ptr;
        l_len -= (chipUnitType.size() + 1);

		// Check for buffer underflow conditions.
		if (l_len > 0) {
			ETRAC("Buffer underflow occured - "
                               "estimated structure size = %d; "
                               "input length = %d",
                               l_byteCount, l_len);
			rc = ECMD_DATA_UNDERFLOW; 
			break;
		}

	} while(0);	// <- single exit.

	return rc;

}	// End ecmdChipTarget::unflatten()

uint32_t ecmdChipTarget::flattenSize() const {
	
	return (sizeof(cage)
		+ sizeof(node)
		+ sizeof(slot)
		+ sizeof(pos)
		+ sizeof(chipUnitNum)
		+ sizeof(thread)
		+ sizeof(unitId)
		+ sizeof(cageState)
		+ sizeof(nodeState)
		+ sizeof(slotState)
		+ sizeof(chipTypeState)
		+ sizeof(posState)
		+ sizeof(chipUnitNumState)
        + sizeof(chipUnitTypeState)
		+ sizeof(threadState)
		+ sizeof(unitIdState)
		+ (chipType.size() + 1)
        + (chipUnitType.size() + 1));
}

#ifndef ECMD_STRIP_DEBUG
void ecmdChipTarget::printStruct() const {

	// Print struct data.
	printf("\tCage: %d\n", cage);
	printf("\tNode: %d\n", node);
	printf("\tSlot: %d\n", slot);
	printf("\tPosition: %d\n", pos);
	printf("\tChipUnitNum: %d\n", chipUnitNum);
	printf("\tThread: %d\n", thread);
	printf("\tUnitId: %d\n", unitId);
	printf("\tCage state: 0x%08x\n", (uint32_t) cageState);
	printf("\tNode state: 0x%08x\n", (uint32_t) nodeState);
	printf("\tSlot state: 0x%08x\n", (uint32_t) slotState);
	printf("\tChip Type state: 0x%08x\n", (uint32_t) chipTypeState);
	printf("\tPosition state: 0x%08x\n", (uint32_t) posState);
	printf("\tChipUnitNum state: 0x%08x\n", (uint32_t) chipUnitNumState);
    printf("\tChipUnitType state: 0x%08x\n", (uint32_t) chipUnitTypeState);
	printf("\tThread state: 0x%08x\n", (uint32_t) threadState);
	printf("\tUnit ID state: 0x%08x\n", (uint32_t) unitIdState);
	printf("\tChip Type: %s\n", chipType.c_str());
    printf("\tChipUnit Type: %s\n", chipUnitType.c_str());
}
#endif

/*
 * The following methods for the ecmdThreadData struct will flatten, unflatten &
 * get the flattened size of the struct.
 */
uint32_t ecmdThreadData::flatten(uint8_t *o_buf, uint32_t &i_len) const{

	uint32_t tmpData32 = 0;
	uint32_t rc        = ECMD_SUCCESS;

	uint8_t *l_ptr = o_buf;

	do {	// Single entry ->

		// Check for buffer overflown conditions.
		if (this->flattenSize() > i_len) {
			// Generate an error for buffer overflow conditions.
			ETRAC("Buffer overflow occured - "
                               "structure size = %d; "
                               "input length = %d",
                               this->flattenSize(), i_len);
			rc = ECMD_DATA_OVERFLOW;
			break;
		}

		// Insert magic header in the buffer.
		tmpData32 = htonl(THREAD_HDR_MAGIC);      
		memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
		l_ptr += sizeof(THREAD_HDR_MAGIC);
		i_len -= sizeof(THREAD_HDR_MAGIC);

		// Copy non-list data.
		memcpy(l_ptr, &threadId, sizeof(threadId));
		l_ptr += sizeof(threadId);
		i_len -= sizeof(threadId);

		tmpData32 = htonl(unitId);//@0b
		memcpy(l_ptr, &tmpData32, sizeof(unitId));//@0b
		l_ptr += sizeof(unitId);
		i_len -= sizeof(unitId);

		tmpData32 = htonl(threadFlags);//@0b
		memcpy(l_ptr, &tmpData32, sizeof(threadFlags));//@0b
		l_ptr += sizeof(threadFlags);
		i_len -= sizeof(threadFlags);

	} while (0);	// <- single exit.

	return rc;
}

uint32_t ecmdThreadData::unflatten(const uint8_t *i_buf, uint32_t &i_len) {

	uint8_t *l_ptr = (uint8_t *)i_buf;

	uint32_t hdrCheck = 0;
	uint32_t rc       = ECMD_SUCCESS;

	do {	// Single entry ->

		// Check for buffer overflown conditions. 
		if (this->flattenSize() > i_len) {                //@01c
			// Generate an error for buffer overflow conditions.
			ETRAC("Buffer overflow occured - "
                               "structure size = %d; "
                               "input length = %d",
                               this->flattenSize(), i_len);
			rc = ECMD_DATA_OVERFLOW;
			break;
		}

		// Get and verify the magic header.
		memcpy(&hdrCheck, l_ptr, sizeof(hdrCheck));
		hdrCheck = ntohl(hdrCheck);
		l_ptr += sizeof(hdrCheck);
		i_len -= sizeof(hdrCheck);

		if (THREAD_HDR_MAGIC != hdrCheck) {
			ETRAC("Buffer header does not match struct header - "
			       "Struct header: 0x%08x; read from buffer as: "
			       "0x%08x", THREAD_HDR_MAGIC, hdrCheck);
			rc = ECMD_INVALID_ARRAY;
			break;
		}

		// Copy non-list data.
		memcpy(&threadId, l_ptr, sizeof(threadId));
		l_ptr += sizeof(threadId);
		i_len -= sizeof(threadId);

		memcpy(&unitId, l_ptr, sizeof(unitId));
		l_ptr += sizeof(unitId);
		i_len -= sizeof(unitId);
		unitId = ntohl(unitId); //@0b - for threadId in showconfig

		memcpy(&threadFlags, l_ptr, sizeof(threadFlags));
		l_ptr += sizeof(threadFlags);
		i_len -= sizeof(threadFlags);
		threadFlags =ntohl(threadFlags);//@0b

	} while (0);	// <- single exit.

	return rc;
}

uint32_t ecmdThreadData::flattenSize() const{

	uint32_t flatSize = 0;

	/*
	 * Every struct entry shall place in the buffer a 32bit value to
	 * contain a magic header used to identify itself.  This will 
	 * be used to make sure the code is looking at the expected 
	 * struct.  So...
	 * 
	 * Add the size of magic header.
	 */
	flatSize += sizeof(THREAD_HDR_MAGIC);

	// Size of non-list member data.
	flatSize += sizeof(threadId);
	flatSize += sizeof(unitId);
	flatSize += sizeof(threadFlags);

	return flatSize;
}

/** @brief Used to sort Thread entries in an ecmdThreadData list. */
bool ecmdThreadData::operator<(const ecmdThreadData& rhs) const {

  if (threadId < rhs.threadId) {
    return true;
  }
        
  return false;
}

#ifndef ECMD_STRIP_DEBUG
void ecmdThreadData::printStruct() const {

	printf("\n\t\t\t\t\t\t\tThread Data:\n");
	printf("\t\t\t\t\t\t\tThread ID: %d\n", threadId);
	printf("\t\t\t\t\t\t\tUnit ID: 0x%x\n", unitId);
	printf("\t\t\t\t\t\t\tThread Flags: 0x%x\n", threadFlags);
}
#endif




/** @brief Used to sort chipUnit entries in an ecmdChipUnitData list. */
bool ecmdChipUnitData::operator<(const ecmdChipUnitData& rhs) const {

  if (chipUnitType < rhs.chipUnitType) {
    return true;
  } else if ((chipUnitType == rhs.chipUnitType) && (chipUnitNum < rhs.chipUnitNum)) {
    return true;
  }

  return false;
}
 
uint32_t ecmdChipUnitData::flatten(uint8_t *o_buf, uint32_t &i_len) const{


    uint32_t tmpData32 = 0;
    uint32_t listSize  = 0;
    uint32_t rc        = ECMD_SUCCESS ;

    uint8_t *l_ptr = o_buf;

    std::list<ecmdThreadData>::const_iterator threaditor = threadData.begin();


    
    do {    // Single entry ->
        // Check for buffer overflow conditions.
        if (this->flattenSize() > i_len) {
            // Generate an error for buffer overflow conditions.
            ETRAC("Buffer overflow occured - "
                  "structure size = %d; "
                  "input length = %d",
                  this->flattenSize(), i_len);
            rc = ECMD_DATA_OVERFLOW;
            break;
        }

        // Insert magic header in the buffer.
        tmpData32 = htonl(CHIPUNIT_HDR_MAGIC);      
        memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
        l_ptr += sizeof(CHIPUNIT_HDR_MAGIC);
        i_len -= sizeof(CHIPUNIT_HDR_MAGIC);
        
        // Copy non-list data.
        memcpy(l_ptr, chipUnitType.c_str(), chipUnitType.size() + 1);
        l_ptr += chipUnitType.size() + 1;
        i_len -= chipUnitType.size() + 1;

        // chipUnitShortType.
        memcpy(l_ptr, chipUnitShortType.c_str(), chipUnitShortType.size() + 1);
        l_ptr += chipUnitShortType.size() + 1;
        i_len -= chipUnitShortType.size() + 1;
        
        memcpy(l_ptr, &chipUnitNum, sizeof(chipUnitNum)); 
        l_ptr += sizeof(chipUnitNum);
        i_len -= sizeof(chipUnitNum);

        memcpy(l_ptr, &numThreads, sizeof(numThreads)); 
        l_ptr += sizeof(numThreads);
        i_len -= sizeof(numThreads);

        tmpData32 = htonl(unitId);
        memcpy(l_ptr, &tmpData32, sizeof(unitId)); 
        l_ptr += sizeof(unitId);
        i_len -= sizeof(unitId);

        tmpData32 = htonl(chipUnitFlags);
        memcpy(l_ptr, &tmpData32, sizeof(chipUnitFlags));
        l_ptr += sizeof(chipUnitFlags);
        i_len -= sizeof(chipUnitFlags);

        
        // Figure out how many threadData structs are in the list for 
        // future unflattening.
         
        listSize = threadData.size();
        tmpData32 = htonl(listSize);
        memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
        l_ptr += sizeof(listSize);
        i_len -= sizeof(listSize);

        if (0 == listSize) {
            
             // There are no threadData structs in this list. Don't 
             // bother attempting to loop on cageData list.
             
            break;
        }

        // Copy list data.
        while (threaditor != threadData.end()) {
            rc = threaditor->flatten(l_ptr, i_len);

            if (rc) break;  // stop on fail and exit
            
                         // l_ptr is not passed by reference so now that we 
                         // have it populated increment by the actual size.
                         
            l_ptr += threaditor->flattenSize();
            threaditor++;
        }
        if (rc) break; // make sure we get to single exit with bad rc

    } while (0);    // <- single exit.

    return rc;
}
 
uint32_t ecmdChipUnitData::unflatten(const uint8_t *i_buf, uint32_t &i_len) {


    uint8_t *l_ptr = (uint8_t *)i_buf;

    uint32_t hdrCheck = 0;
    uint32_t listSize = 0;
    uint32_t rc       = ECMD_SUCCESS;
    

    do {    // Single entry ->
        
        // Check for buffer overflow conditions.
        if (this->flattenSize() > i_len) {
            // Generate an error for buffer overflow conditions.
            ETRAC("Buffer overflow occured - "
                  "structure size = %d; "
                  "input length = %d",
                  this->flattenSize(), i_len);
            rc = ECMD_DATA_OVERFLOW;
            break;
        }

        // Get and verify the magic header.
        memcpy(&hdrCheck, l_ptr, sizeof(hdrCheck));
        hdrCheck = ntohl(hdrCheck);
        l_ptr += sizeof(hdrCheck);
        i_len -= sizeof(hdrCheck);

        if (CHIPUNIT_HDR_MAGIC != hdrCheck) {

            ETRAC("Buffer header does not match struct header - "
                  "Struct header: 0x%08x; read from buffer as: "
                  "0x%08x", CHIPUNIT_HDR_MAGIC, hdrCheck);
            rc = ECMD_INVALID_ARRAY;
            break;
        }

        // Unflatten non-list data.
        
        std::string l_chipUnitType = (const char *) l_ptr;
        l_ptr += l_chipUnitType.size() + 1;
        chipUnitType = l_chipUnitType;
        i_len -= l_chipUnitType.size() + 1;
        
        std::string l_chipUnitShortType = (const char *) l_ptr;
        l_ptr += l_chipUnitShortType.size() + 1;
        chipUnitShortType = l_chipUnitShortType;
        i_len -= l_chipUnitShortType.size() + 1;
        
        memcpy(&chipUnitNum, l_ptr, sizeof(chipUnitNum));
        l_ptr += sizeof(chipUnitNum);
        i_len -= sizeof(chipUnitNum);

        memcpy(&numThreads, l_ptr, sizeof(numThreads));
        l_ptr += sizeof(numThreads);
        i_len -= sizeof(numThreads);

        memcpy(&unitId, l_ptr, sizeof(unitId));
        l_ptr += sizeof(unitId);
        i_len -= sizeof(unitId);
        unitId = ntohl(unitId);

        memcpy(&chipUnitFlags, l_ptr, sizeof(chipUnitFlags));
        l_ptr += sizeof(chipUnitFlags);
        i_len -= sizeof(chipUnitFlags);
        chipUnitFlags = ntohl(chipUnitFlags);

        memcpy(&listSize, l_ptr, sizeof(listSize));
        listSize = ntohl(listSize);
        l_ptr += sizeof(listSize);
        i_len -= sizeof(listSize);

        // Since the return query data structure may be reused from a
        // previous call make sure the list is cleared.  Make sure this
        // is done before the if check below that may break us out to
        // prevent returning old data from previous iteration.     @09a
        threadData.clear();

        // Check to see if the list is populated.
        if (0 == listSize) {
            // Nothing to create, just leave.
            break;
        }

        // Create any list entries.
        for (uint32_t i = 0; i < listSize; i++) {
            threadData.push_back(ecmdThreadData());
        }

        std::list<ecmdThreadData>::iterator threaditor = 
                            threadData.begin();
        // Unflatten list data.
        while (threaditor != threadData.end()) {
            rc = threaditor->unflatten(l_ptr, i_len);

            if (rc) break;  // stop on fail and exit
            
                         //l_ptr is not passed by reference so now that we 
                         //have it populated increment by the actual size.
                         
            l_ptr += threaditor->flattenSize();
            threaditor++;
        }
        if (rc) break; // make sure we get to single exit with bad rc

    } while (0);    // <- single exit.

    return rc;
}

uint32_t ecmdChipUnitData::flattenSize() const {

    uint32_t flatSize = 0;
    std::list<ecmdThreadData>::const_iterator threaditor = threadData.begin();

    do {    // Single entry ->

        /*
         * Every struct entry shall place in the buffer a 32bit value to         * contain a magic header used to identify itself.  This will 
         * be used to make sure the code is looking at the expected 
         * struct.  So...
         * 
         * Add the size of magic header.
         */
        flatSize += sizeof(CHIPUNIT_HDR_MAGIC);
        // Size of non-list member data.
        flatSize += (sizeof(chipUnitNum) 
                 + sizeof(numThreads)
                 + sizeof(unitId) 
                 + sizeof(chipUnitFlags) 
                 + chipUnitType.size() + 1
                 + chipUnitShortType.size() + 1);
                 
        /* 
         * Every struct entry which contains a list of other structs is
         * required to put into the buffer a 32bit value describing the
         * number of structures in its list.  So...
         * 
         * Add one for the chipUnitData list counter.
         */
        flatSize += sizeof(uint32_t);
        // If the chipUnitData list does not contain any structs break out.

        if (0 == threadData.size()) {
            break;
        }       
        // Size of list member data.
        while (threaditor != threadData.end()) {
            flatSize += threaditor->flattenSize();
            threaditor++;
        }

    } while (0);    // <- single exit.
    return flatSize;
}

#ifndef ECMD_STRIP_DEBUG
void ecmdChipUnitData::printStruct() const {

    std::list<ecmdThreadData>::const_iterator threaditor = threadData.begin();

    printf("\n\t\t\t\t\t\tChipUnit Data:\n");

    // Print non-list data.
    printf("\t\t\t\t\t\tchipUnitType :%s\n",chipUnitType.c_str());
    printf("\t\t\t\t\t\tchipUnitShortType :%s\n",chipUnitShortType.c_str());
    printf("\t\t\t\t\t\tchipUnitNum : %d\n", chipUnitNum);
    printf("\t\t\t\t\t\tNumber of threads: %d\n", numThreads);
    printf("\t\t\t\t\t\tUnit ID: 0x%x\n", unitId);
    printf("\t\t\t\t\t\tChipUnit Flags: 0x%x\n", chipUnitFlags);

    // Print list data.
    if (threadData.size() == 0) {
        printf("\t\t\t\t\t\tNo thread data.\n");
    }

    while (threaditor != threadData.end()) {
        threaditor->printStruct();
        threaditor++;
    }
}
#endif

/*
 * The following methods for the ecmdChipData struct will flatten, unflatten &
 * get the flattened size of the struct.
 */
uint32_t ecmdChipData::flatten(uint8_t *o_buf, uint32_t &i_len) const{
	uint32_t rc	   = ECMD_SUCCESS;

	uint32_t tmpData32 = 0;
	uint32_t listSize  = 0;

	uint8_t *l_ptr = o_buf;

    std::list<ecmdChipUnitData>::const_iterator chipunititor = chipUnitData.begin();

	do {	// Single entry ->

		// Check for buffer overflow conditions.
		if (this->flattenSize() > i_len) {
			// Generate an error for buffer overflow conditions.
			ETRAC("Buffer overflow occured - "
                               "structure size = %d; "
                               "input length = %d",
                               this->flattenSize(), i_len);
			rc = ECMD_DATA_OVERFLOW;
			break;
		}

		// Insert magic header in the buffer.
		tmpData32 = htonl(CHIP_HDR_MAGIC);
		memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
		l_ptr += sizeof(CHIP_HDR_MAGIC);
		i_len -= sizeof(CHIP_HDR_MAGIC);

		// Copy non-list data.
		memcpy(l_ptr, chipType.c_str(), chipType.size() + 1);
		l_ptr += chipType.size() + 1;
		i_len -= chipType.size() + 1;
	
		memcpy(l_ptr, 
		       chipShortType.c_str(), 
		       chipShortType.size() + 1);
		l_ptr += chipShortType.size() + 1;
		i_len -= chipShortType.size() + 1;

		memcpy(l_ptr, 
		       chipCommonType.c_str(), 
		       chipCommonType.size() + 1);
		l_ptr += chipCommonType.size() + 1;
		i_len -= chipCommonType.size() + 1;
	
		tmpData32 = htonl(pos);
		memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
		l_ptr += sizeof(pos);
		i_len -= sizeof(pos);
	
		tmpData32 = htonl(unitId);
		memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
		l_ptr += sizeof(unitId);
		i_len -= sizeof(unitId);

		tmpData32 = htonl(chipEc);
		memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
		l_ptr += sizeof(chipEc);
		i_len -= sizeof(chipEc);

		tmpData32 = htonl(simModelEc);
		memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
		l_ptr += sizeof(simModelEc);
		i_len -= sizeof(simModelEc);
	
		tmpData32 = htonl((uint32_t) interfaceType);
		memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
		l_ptr += sizeof(interfaceType);
		i_len -= sizeof(interfaceType);

		tmpData32 = htonl(chipFlags);
		memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
		l_ptr += sizeof(chipFlags);
		i_len -= sizeof(chipFlags);

		/*
		 * Figure out how many chipUnitData structs are in the list for 
		 * future unflattening.
		 */
        listSize = chipUnitData.size();
		tmpData32 = htonl(listSize);
		memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
		l_ptr += sizeof(listSize);
		i_len -= sizeof(listSize);

		if (0 == listSize) {
			/*
			 * There are no chipUnitData structs in this list. Don't 
			 * bother attempting to loop on cageData list.
			 */
			break;
		}

        // Copy list data.
        while (chipunititor != chipUnitData.end()){
                rc = chipunititor->flatten(l_ptr, i_len);

            if (rc) break; // stop on fail and exit
            /*
                         * l_ptr is not passed by reference so now that we 
                         * have it populated increment by the actual size.
                         */
            l_ptr += chipunititor->flattenSize();
            chipunititor++;
        }
		if (rc) break; // make sure we get to single exit with bad rc
		

	} while (0);	// <- single exit.

	return rc;
}

uint32_t ecmdChipData::unflatten(const uint8_t *i_buf, uint32_t &i_len) {
	uint32_t rc       = ECMD_SUCCESS;

	uint8_t *l_ptr = (uint8_t *)i_buf;

	uint32_t hdrCheck = 0;
	uint32_t listSize = 0;

	do {	// Single entry ->
		/*
		 * Because of the strings in this struct we must check the magic
		 * header early.  So...Get and verify the magic header.
		 */
		memcpy(&hdrCheck, l_ptr, sizeof(hdrCheck));
		hdrCheck = ntohl(hdrCheck);
		l_ptr += sizeof(hdrCheck);
		i_len -= sizeof(hdrCheck);

		if (CHIP_HDR_MAGIC != hdrCheck) {
			ETRAC("Buffer header does not match struct header - "
                               "Struct header: 0x%08x; read from buffer as: "
                               "0x%08x", CHIP_HDR_MAGIC, hdrCheck);
			rc = ECMD_INVALID_ARRAY;
			break;
		}

		std::string l_chipType = (const char *) l_ptr;
		l_ptr += l_chipType.size() + 1;

		std::string l_chipShortType = (const char *) l_ptr;
		l_ptr += l_chipShortType.size() + 1;

		std::string l_chipCommonType = (const char *) l_ptr;
		l_ptr += l_chipCommonType.size() + 1;

		uint32_t l_byteCount = sizeof(pos)
				       + sizeof(unitId)
				       + sizeof(chipEc)
				       + sizeof(simModelEc)
				       + sizeof(interfaceType)
				       + sizeof(chipFlags)
				       + l_chipType.size() + 1
		                       + l_chipShortType.size() + 1
				       + l_chipCommonType.size() + 1;

		if (l_byteCount > i_len) {
			// Generate an error for buffer overflow conditions.
			ETRAC("Buffer overflow occured - "
                               "structure size = %d; "
                               "input length = %d",
                               this->flattenSize(), i_len);
			rc = ECMD_DATA_OVERFLOW;
			break;
		}

		// Unflatten non-list data.
		chipType = l_chipType;
		i_len -= l_chipType.size() + 1;

		chipShortType = l_chipShortType;
		i_len -= l_chipShortType.size() + 1;

		chipCommonType = l_chipCommonType;
		i_len -= l_chipCommonType.size() + 1;

		memcpy(&pos, l_ptr, sizeof(pos));
		pos = ntohl(pos);
		l_ptr += sizeof(pos);
		i_len -= sizeof(pos);

		memcpy(&unitId, l_ptr, sizeof(unitId));
		unitId = ntohl(unitId);
		l_ptr += sizeof(unitId);
		i_len -= sizeof(unitId);

		memcpy(&chipEc, l_ptr, sizeof(chipEc));
		chipEc = ntohl(chipEc);
		l_ptr += sizeof(chipEc);
		i_len -= sizeof(chipEc);

		memcpy(&simModelEc, l_ptr, sizeof(simModelEc));
		simModelEc = ntohl(simModelEc);
		l_ptr += sizeof(simModelEc);
		i_len -= sizeof(simModelEc);

		memcpy(&interfaceType, l_ptr, sizeof(interfaceType));
		interfaceType = (ecmdChipInterfaceType_t) ntohl(
						      (uint32_t) interfaceType);
		l_ptr += sizeof(interfaceType);
		i_len -= sizeof(interfaceType);

		memcpy(&chipFlags, l_ptr, sizeof(chipFlags));
		chipFlags = ntohl(chipFlags);
		l_ptr += sizeof(chipFlags);
		i_len -= sizeof(chipFlags);

		// Get the number of chipUnitData structs from the buffer.
		memcpy(&listSize, l_ptr, sizeof(listSize));
		listSize = ntohl(listSize);
		l_ptr += sizeof(listSize);
		i_len -= sizeof(listSize);

        // Since the return query data structure may be reused from a
        // previous call make sure the list is cleared.  Make sure this
        // is done before the if check below that may break us out to
        // prevent returning old data from previous iteration.     @09a
        chipUnitData.clear();

		// Check to see if the list is populated.
		if (0 == listSize) {
			// Nothing to create, just leave.
			break;
		}

		// Create any list entries.
		for (uint32_t i = 0; i < listSize; i++) {
			chipUnitData.push_back(ecmdChipUnitData());
		}

        std::list<ecmdChipUnitData>::iterator chipunititor = chipUnitData.begin();
	
		// Unflatten list data.
        while (chipunititor != chipUnitData.end()) {
            rc = chipunititor->unflatten(l_ptr, i_len);

            if (rc) break;  // stop on fail and exit
            /*
                         * l_ptr is not passed by reference so now that we 
                         * have it populated increment by the actual size.
                         */
            l_ptr += chipunititor->flattenSize();
            chipunititor++;
        }
		if (rc) break; // make sure we get to single exit with bad rc

	} while (0);	// <- single exit.

	return rc;
}

uint32_t ecmdChipData::flattenSize() const {
	uint32_t flatSize = 0;
	
    std::list<ecmdChipUnitData>::const_iterator chipunititor = chipUnitData.begin();

	do {	// Single entry ->

		/*
		 * Every struct entry shall place in the buffer a 32bit value to		 * contain a magic header used to identify itself.  This will 
		 * be used to make sure the code is looking at the expected 
		 * struct.  So...
		 * 
		 * Add the size of magic header.
		 */
		flatSize += sizeof(CHIP_HDR_MAGIC);

		// Size of non-list member data.
		flatSize += (sizeof(pos) 
			     + sizeof(unitId)
			     + sizeof(chipEc) 
			     + sizeof(simModelEc) 
			     + sizeof(interfaceType) 
			     + sizeof(chipFlags) 
			     + chipType.size() + 1
			     + chipShortType.size() + 1
			     + chipCommonType.size() + 1);

		/* 
		 * Every struct entry which contains a list of other structs is
		 * required to put into the buffer a 32bit value describing the
		 * number of structures in its list.  So...
		 * 
		 * Add one for the chipUnitData list counter.
		 */
		flatSize += sizeof(uint32_t);

        // If the chipUnitData list does not contain any structs break out.
        if (0 == chipUnitData.size()) {
            break;
        }

        // Size of list member data.
        while (chipunititor != chipUnitData.end()) {
            flatSize += chipunititor->flattenSize();
            chipunititor++;
        }

	} while (0);	// <- single exit.

	return flatSize;
}

/** @brief Used to sort Chip entries (based on Pos) in an ecmdChipData list. */
bool ecmdChipData::operator<(const ecmdChipData& rhs) const {

  if (chipType < rhs.chipType) {
    return true;
  } else if ((chipType == rhs.chipType) && (pos < rhs.pos)) {
    return true;
  }
   
  return false;
}


#ifndef ECMD_STRIP_DEBUG
void ecmdChipData::printStruct() const{

    std::list<ecmdChipUnitData>::const_iterator chipunititor = chipUnitData.begin();

	printf("\n\t\t\t\t\tChip Data:\n");

	// Print non-list data.
	printf("\t\t\t\t\tChip type: %s\n", chipType.c_str());
	printf("\t\t\t\t\tChip short type: %s\n", chipShortType.c_str());
	printf("\t\t\t\t\tChip common type: %s\n", chipCommonType.c_str());
	printf("\t\t\t\t\tPosition: %d\n", pos);
	printf("\t\t\t\t\tUnitId: 0x%x\n", unitId);
	printf("\t\t\t\t\tChip EC: %X\n", chipEc);
	printf("\t\t\t\t\tSim mode EC: %X\n", simModelEc);
	printf("\t\t\t\t\tInterface: 0x%08x\n", (uint32_t) interfaceType);
	printf("\t\t\t\t\tChip flags: 0x%08x\n", chipFlags);

    // Print list data.
    if (chipUnitData.size() == 0) {
        printf("\t\t\t\t\tNo chipUnit data.\n");
    }
    while (chipunititor != chipUnitData.end()) {
        chipunititor->printStruct();
        chipunititor++;
    }
}
#endif

/*
 * The following methods for the ecmdSlotData struct will flatten, unflatten &
 * get the flattened size of the struct.
 */
uint32_t ecmdSlotData::flatten(uint8_t *o_buf, uint32_t &i_len) const{

	uint32_t tmpData32 = 0;
	uint32_t listSize  = 0;
	uint32_t rc	= ECMD_SUCCESS;

	uint8_t *l_ptr = o_buf;

	std::list<ecmdChipData>::const_iterator chipitor = chipData.begin();

	do {	// Single entry -> 

		// Check for buffer overflow conditions.
		if (this->flattenSize() > i_len) {
			// Generate an error for buffer overflow conditions.
			ETRAC("Buffer overflow occured - "
                               "structure size = %d; "
                               "input length = %d",
                               this->flattenSize(), i_len);
			rc = ECMD_DATA_OVERFLOW;
			break;
		}

		// Insert magic header in the buffer.
		tmpData32 = htonl(SLOT_HDR_MAGIC);		
		memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
		l_ptr += sizeof(SLOT_HDR_MAGIC);
		i_len -= sizeof(SLOT_HDR_MAGIC);

		// Copy non-list data.
		tmpData32 = htonl((uint32_t) slotId);
		memcpy(l_ptr, &tmpData32, sizeof(slotId));
		l_ptr += sizeof(slotId);
		i_len -= sizeof(slotId);

        memcpy(l_ptr, slotName.c_str(), slotName.size() + 1);   
        l_ptr += slotName.size() + 1;
        i_len -= slotName.size() + 1;

		tmpData32 = htonl((uint32_t) unitId);
		memcpy(l_ptr, &tmpData32, sizeof(unitId));
		l_ptr += sizeof(unitId);
		i_len -= sizeof(unitId);

		tmpData32 = htonl((uint32_t) slotFlags);
		memcpy(l_ptr, &tmpData32, sizeof(slotFlags));
		l_ptr += sizeof(slotFlags);
		i_len -= sizeof(slotFlags);

   
		/* 
		 * Figure out how many chipData structs are in the list for 
		 * future unflattening.
		 */
		listSize = chipData.size();
		tmpData32 = htonl(listSize);
		memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
		l_ptr += sizeof(listSize);
		i_len -= sizeof(listSize);

		if (0 == listSize) {
			/*
			 * There are no chipData structs in this list. Don't 
			 * bother attempting to loop on cageData list.
			 */
			break;
		}

		// Copy list data.
		while (chipitor != chipData.end()) {
			rc = chipitor->flatten(l_ptr, i_len);
			if (rc) break;  // stop on fail and exit
			l_ptr += chipitor->flattenSize();
			chipitor++;
		}
		if (rc) break; // make sure we get to single exit with bad rc

	} while (0);	// <- single exit.

	return rc;
}

uint32_t ecmdSlotData::unflatten(const uint8_t *i_buf, uint32_t &i_len) {

	uint8_t *l_ptr = (uint8_t *) i_buf;

	uint32_t hdrCheck = 0;
	uint32_t listSize = 0;
	uint32_t rc       = ECMD_SUCCESS;

	do {	// Single entry ->

		// Check for buffer overflow conditions.		
		if (this->flattenSize() > i_len) {
			// Generate an error for buffer overflow conditions.
			ETRAC("Buffer overflow occured - "
                               "structure size = %d; "
                               "input length = %d",
                               this->flattenSize(), i_len);
			rc = ECMD_DATA_OVERFLOW;
			break;
		}

		// Get and verify the magic header.
		memcpy(&hdrCheck, l_ptr, sizeof(hdrCheck));
		hdrCheck = ntohl(hdrCheck);
		l_ptr += sizeof(hdrCheck);
		i_len -= sizeof(hdrCheck);

		if (SLOT_HDR_MAGIC != hdrCheck) {
			ETRAC("Buffer header does not match struct header - "
                               "Struct header: 0x%08x; read from buffer as: "
                               "0x%08x", SLOT_HDR_MAGIC, hdrCheck);
			rc = ECMD_INVALID_ARRAY;
			break;
		}

		// Unflatten non-list data.
		memcpy(&slotId, l_ptr, sizeof(slotId));
		slotId = ntohl(slotId);
		l_ptr += sizeof(slotId);
		i_len -= sizeof(slotId);

        std::string l_slotName = (const char *) l_ptr;  
        l_ptr += l_slotName.size() + 1;
        l_slotName = l_slotName;
        i_len -= l_slotName.size() + 1;

		memcpy(&unitId, l_ptr, sizeof(unitId));
		unitId = ntohl(unitId);
		l_ptr += sizeof(unitId);
		i_len -= sizeof(unitId);

		memcpy(&slotFlags, l_ptr, sizeof(slotFlags));
		slotFlags = ntohl(slotFlags);
		l_ptr += sizeof(slotFlags);
		i_len -= sizeof(slotFlags);

		// Get the number of chipData structs from the buffer.
		memcpy(&listSize, l_ptr, sizeof(listSize));
		listSize = ntohl(listSize);
		l_ptr += sizeof(listSize);
		i_len -= sizeof(listSize);

        // Since the return query data structure may be reused from a
        // previous call make sure the list is cleared.  Make sure this
        // is done before the if check below that may break us out to
        // prevent returning old data from previous iteration.     @09a
        chipData.clear();

		// Check to see if the list is populated.
		if (0 == listSize) {
			// Nothing to create, just leave.
			break;
		}

		// Create any list entries.
		for (uint32_t i = 0; i < listSize; i++) {
			chipData.push_back(ecmdChipData());
		}

		std::list<ecmdChipData>::iterator chipitor = chipData.begin();

		// Unflatten list data.
		while (chipitor != chipData.end()) {
			rc = chipitor->unflatten(l_ptr, i_len);
			if (rc) break; // stop on fail and exit
			/*
			 * l_ptr is not passed by reference so now that we 
			 * have it populated increment by the actual size.
			 */
			l_ptr += chipitor->flattenSize();
			chipitor++;
		}
		if (rc) break; // make sure we get to single exit with bad rc

	} while (0);	// <- single exit.

	return rc;
}

uint32_t ecmdSlotData::flattenSize() const {

	uint32_t flatSize = 0;
	std::list<ecmdChipData>::const_iterator chipitor = chipData.begin();

	do {	// Single entry ->

		/*
		 * Every struct entry shall place in the buffer a 32bit value to		 * contain a magic header used to identify itself.  This will 
		 * be used to make sure the code is looking at the expected 
		 * struct.  So...
		 * 
		 * Add the size of magic header.
		 */
		flatSize += sizeof(SLOT_HDR_MAGIC);

		// Size of non-list member data.
		flatSize += sizeof(slotId) + (slotName.size() + 1) + sizeof(unitId) + sizeof(slotFlags);

		/* 
		 * Every struct entry which contains a list of other structs is
		 * required to put into the buffer a 32bit value describing the
		 * number of structures in its list.  So...
		 * 
		 * Add one for the chipData list counter.
		 */
		flatSize += sizeof(uint32_t);

		// If the chipData list does not contain any structs break out.
		if (0 == chipData.size()) {
			break;
		}

		// Size of list member data.
		while (chipitor != chipData.end()) {
			flatSize += chipitor->flattenSize();
			chipitor++;
		}

	} while (0);	// <- single exit.

	return flatSize;
}

/** @brief Used to sort Slot entries in an ecmdSlotData list. */
bool ecmdSlotData::operator<(const ecmdSlotData& rhs) const {

  if (slotId < rhs.slotId) {
    return true;
  }

  return false;
}

#ifndef ECMD_STRIP_DEBUG
void ecmdSlotData::printStruct() const {

	std::list<ecmdChipData>::const_iterator chipitor = chipData.begin();

	printf("\n\t\t\t\tSlot Data:\n");

	// Print non-list data.
	printf("\t\t\t\tSlot ID: 0x%08x\n", slotId);
    printf("\t\t\t\tSlot Name: %s\n", slotName.c_str());
	printf("\t\t\t\tUnit ID: 0x%08x\n", unitId);
	printf("\t\t\t\tslot Flags: 0x%08x\n", slotFlags);

	// Print list data.
	if (chipData.size() == 0) {
		printf("\t\t\t\tNo chip data.\n");
	}

	while (chipitor != chipData.end()) {
		chipitor->printStruct();
		chipitor++;
	}
}
#endif


/*
 * The following methods for the ecmdNodeData struct will flatten, unflatten &
 * get the flattened size of the struct.
 */
uint32_t ecmdNodeData::flatten(uint8_t *o_buf, uint32_t &i_len) const{

	uint32_t tmpData32 = 0;
	uint32_t listSize  = 0;
	uint32_t rc	= ECMD_SUCCESS;

	uint8_t *l_ptr = o_buf;

	std::list<ecmdSlotData>::const_iterator slotitor = slotData.begin();

	do {	// Single entry ->

		// Check for buffer overflow conditions.
		if (this->flattenSize() > i_len) {
			// Generate an error for buffer overflow conditions.
			ETRAC("Buffer overflow occured - "
                               "structure size = %d; "
                               "input length = %d",
                               this->flattenSize(), i_len);
			rc = ECMD_DATA_OVERFLOW;
			break;
		}

		// Insert magic header in the buffer.
		tmpData32 = htonl(NODE_HDR_MAGIC);      
		memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
		l_ptr += sizeof(NODE_HDR_MAGIC);
		i_len -= sizeof(NODE_HDR_MAGIC);

		// Copy non-list data.
		tmpData32 = htonl((uint32_t) nodeId);
		memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
		l_ptr += sizeof(nodeId);
		i_len -= sizeof(nodeId);

        memcpy(l_ptr, nodeName.c_str(), nodeName.size() + 1);   
        l_ptr += nodeName.size() + 1;
        i_len -= nodeName.size() + 1;

		tmpData32 = htonl((uint32_t) unitId);
		memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
		l_ptr += sizeof(unitId);
		i_len -= sizeof(unitId);

		tmpData32 = htonl((uint32_t) nodeFlags);
		memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
		l_ptr += sizeof(nodeFlags);
		i_len -= sizeof(nodeFlags);

		/*
		 * Figure out how many slotData structs are in the list for 
		 * future unflattening.
		 */
		listSize = slotData.size();
		tmpData32 = htonl(listSize);
		memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
		l_ptr += sizeof(listSize);
		i_len -= sizeof(listSize);

		if (0 == listSize) {
			/*
			 * There are no slotData structs in this list. Don't 
			 * bother attempting to loop on cageData list.
			 */
			break;
		}

		// Copy list data.
		while (slotitor != slotData.end()) {
			rc = slotitor->flatten(l_ptr, i_len);

			if (rc) break;  // stop on fail and exit
			/*
                         * l_ptr is not passed by reference so now that we 
                         * have it populated increment by the actual size.
                         */
			l_ptr += slotitor->flattenSize();
			slotitor++;
		}
		if (rc) break; // make sure we get to single exit with bad rc

	} while (0);	// <- single exit.

	return rc;
}

uint32_t ecmdNodeData::unflatten(const uint8_t *i_buf, uint32_t &i_len) {

	uint8_t *l_ptr = (uint8_t *) i_buf;

	uint32_t hdrCheck = 0;
	uint32_t listSize = 0;
	uint32_t rc       = ECMD_SUCCESS;

	do {	// Single entry ->

		// Check for buffer overflow conditions.
		if (this->flattenSize() > i_len) {
			// Generate an error for buffer overflow conditions.
			ETRAC("Buffer overflow occured - "
                               "structure size = %d; "
                               "input length = %d",
                               this->flattenSize(), i_len);
			rc = ECMD_DATA_OVERFLOW;
			break;
		}

		// Get and verify the magic header.
		memcpy(&hdrCheck, l_ptr, sizeof(hdrCheck));
		hdrCheck = ntohl(hdrCheck);
		l_ptr += sizeof(hdrCheck);
		i_len -= sizeof(hdrCheck);

		if (NODE_HDR_MAGIC != hdrCheck) {
			ETRAC("Buffer header does not match struct header - "
                               "Struct header: 0x%08x; read from buffer as: "
                               "0x%08x", NODE_HDR_MAGIC, hdrCheck);
			rc = ECMD_INVALID_ARRAY;
			break;
		}

		// Unflatten non-list data.
		memcpy(&nodeId, l_ptr, sizeof(nodeId));
		nodeId = ntohl(nodeId);
		l_ptr += sizeof(nodeId);
		i_len -= sizeof(nodeId);

        std::string l_nodeName = (const char *) l_ptr; 
        l_ptr += l_nodeName.size() + 1;
        l_nodeName = l_nodeName;
        i_len -= l_nodeName.size() + 1;

		memcpy(&unitId, l_ptr, sizeof(unitId)); //@03 chg dest from nodeId to unitId
		unitId = ntohl(unitId);                 //@03 chg dest from nodeId to unitId
		l_ptr += sizeof(unitId);
		i_len -= sizeof(unitId);

		memcpy(&nodeFlags, l_ptr, sizeof(nodeFlags));
		nodeFlags = ntohl(nodeFlags);
		l_ptr += sizeof(nodeFlags);
		i_len -= sizeof(nodeFlags);

		// Get the number of slotData structs from the buffer.
		memcpy(&listSize, l_ptr, sizeof(listSize));
		listSize = ntohl(listSize);
		l_ptr += sizeof(listSize);
		i_len -= sizeof(listSize);

        // Since the return query data structure may be reused from a
        // previous call make sure the list is cleared.  Make sure this
        // is done before the if check below that may break us out to
        // prevent returning old data from previous iteration.     @09a
        slotData.clear();

		// Check to see if the list is populated.
		if (0 == listSize) {
			// Nothing to create, just leave.
			break;
		}

		// Create any list entries.
		for (uint32_t i = 0; i < listSize; i++) {
			slotData.push_back(ecmdSlotData());
		}

		std::list<ecmdSlotData>::iterator slotitor = slotData.begin();
	
		// Unflatten list data.
		while (slotitor != slotData.end()) {
			rc = slotitor->unflatten(l_ptr, i_len);

			if (rc) break;  // stop on fail and exit
			/*
                         * l_ptr is not passed by reference so now that we 
                         * have it populated increment by the actual size.
                         */
			l_ptr += slotitor->flattenSize();
			slotitor++;
		}
		if (rc) break; // make sure we get to single exit with bad rc

	} while (0);	// <- single exit.

	return rc;	
}

uint32_t ecmdNodeData::flattenSize() const {

	uint32_t flatSize = 0;
	std::list<ecmdSlotData>::const_iterator slotitor = slotData.begin();

	do {    // Single entry ->

		/*
		 * Every struct entry shall place in the buffer a 32bit value to		 * contain a magic header used to identify itself.  This will 
		 * be used to make sure the code is looking at the expected 
		 * struct.  So...
		 * 
		 * Add the size of magic header.
		 */
		flatSize += sizeof(NODE_HDR_MAGIC);

		// Size of non-list member data.
		flatSize += sizeof(nodeId) + (nodeName.size() + 1) + sizeof(unitId) + sizeof(nodeFlags);

		/* 
		 * Every struct entry which contains a list of other structs is
		 * required to put into the buffer a 32bit value describing the
		 * number of structures in its list.  So...
		 * 
		 * Add one for the slotData list counter.
		 */
		flatSize += sizeof(uint32_t);

		// If the slotData list does not contain any structs break out.
		if (0 == slotData.size()) {
			break;
		}

		// Size of list member data.
		while (slotitor != slotData.end()) {
			flatSize += slotitor->flattenSize();
			slotitor++;
		}

	} while (0);    // <- single exit.

	return flatSize;
}

/** @brief Used to sort Node entries in an ecmdNodeData list. */
bool ecmdNodeData::operator<(const ecmdNodeData& rhs) const {

  if (nodeId < rhs.nodeId) {
    return true;
  }

  return false;
}

#ifndef ECMD_STRIP_DEBUG
void ecmdNodeData::printStruct() const {

	std::list<ecmdSlotData>::const_iterator slotitor = slotData.begin();

	printf("\n\t\t\tNode Data:\n");

	// Print non-list data.
	printf("\t\t\tNode ID: 0x%08x\n", nodeId);
    printf("\t\t\tNode Name: %s\n", nodeName.c_str());
	printf("\t\t\tUnit ID: 0x%08x\n", unitId);
	printf("\t\t\tNode Flags: 0x%08x\n", nodeFlags);

	// Print list data.
	if (slotData.size() == 0) {
		printf("\t\t\tNo slot data.\n");
	}

	while (slotitor != slotData.end()) {
		slotitor->printStruct();
		slotitor++;
	}
}
#endif


/*
 * The following methods for the ecmdCageData struct will flatten, unflatten &
 * get the flattened size of the struct.
 */
uint32_t ecmdCageData::flatten(uint8_t *o_buf, uint32_t &i_len) const{

	uint32_t tmpData32 = 0;
	uint32_t listSize  = 0;
	uint32_t rc	= ECMD_SUCCESS;

	uint8_t *l_ptr = o_buf;

	std::list<ecmdNodeData>::const_iterator nodeitor = nodeData.begin();

	do {    // Single entry ->

		if (this->flattenSize() > i_len) {
			// Generate an error for buffer overflow conditions.
			ETRAC("Buffer overflow occured - "
                               "structure size = %d; "
                               "input length = %d",
                               this->flattenSize(), i_len);
			rc = ECMD_DATA_OVERFLOW;
			break;
		}

		// Insert magic header in the buffer.
		tmpData32 = htonl(CAGE_HDR_MAGIC);
		memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
		l_ptr += sizeof(CAGE_HDR_MAGIC);
		i_len -= sizeof(CAGE_HDR_MAGIC);

		// Copy non-list data.
		tmpData32 = htonl((uint32_t) cageId);
		memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
		l_ptr += sizeof(cageId);
		i_len -= sizeof(cageId);

        memcpy(l_ptr, cageName.c_str(), cageName.size() + 1);   
        l_ptr += cageName.size() + 1; 
        i_len -= cageName.size() + 1;

		tmpData32 = htonl((uint32_t) unitId);
		memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
		l_ptr += sizeof(unitId);
		i_len -= sizeof(unitId);

		tmpData32 = htonl((uint32_t) cageFlags);
		memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
		l_ptr += sizeof(cageFlags);
		i_len -= sizeof(cageFlags);

		/*
		 * Figure out how many nodeData structs are in the list for 
		 * future unflattening.
		 */
		listSize = nodeData.size();
		tmpData32 = htonl(listSize);
		memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
		l_ptr += sizeof(listSize);
		i_len -= sizeof(listSize);

		if (0 == listSize) {
			/*
			 * There are no nodeData structs in this list. Don't 
			 * bother attempting to loop on cageData list.
			 */
			break;
		}

		// Copy list data.
		while (nodeitor != nodeData.end()) {
			rc = nodeitor->flatten(l_ptr, i_len);

			if (rc) break; // stop on fail and exit
			/*
                         * l_ptr is not passed by reference so now that we 
                         * have it populated increment by the actual size.
                         */
			l_ptr += nodeitor->flattenSize();
			nodeitor++;
		}
		if (rc) break; // make sure we get to single exit with bad rc

	} while (0);    // <- single exit.

	return rc;
}

uint32_t ecmdCageData::unflatten(const uint8_t *i_buf, uint32_t &i_len) {

	uint8_t *l_ptr = (uint8_t *) i_buf;

	uint32_t hdrCheck = 0;
	uint32_t listSize = 0;
	uint32_t rc       = ECMD_SUCCESS;

	do {    // Single entry ->

		// Check for buffer overflow conditions.
		if (this->flattenSize() > i_len) {
			// Generate an error for buffer overflow conditions.
			ETRAC("Buffer overflow occured - "
                               "structure size = %d; "
                               "input length = %d",
                               this->flattenSize(), i_len);
			rc = ECMD_DATA_OVERFLOW;
			break;
		}

		// Get and verify the magic header.
		memcpy(&hdrCheck, l_ptr, sizeof(hdrCheck));
		hdrCheck = ntohl(hdrCheck);
		l_ptr += sizeof(hdrCheck);
		i_len -= sizeof(hdrCheck);

		if (CAGE_HDR_MAGIC != hdrCheck) {
			ETRAC("Buffer header does not match struct header - "
                               "Struct header: 0x%08x; read from buffer as: "
                               "0x%08x", CAGE_HDR_MAGIC, hdrCheck);
			rc = ECMD_INVALID_ARRAY;
			break;
		}

		// Unflatten non-list data.
		memcpy(&cageId, l_ptr, sizeof(cageId));
		cageId = ntohl(cageId);
		l_ptr += sizeof(cageId);
		i_len -= sizeof(cageId);

        std::string l_cageName = (const char *) l_ptr;  
        l_ptr += l_cageName.size() + 1;
        l_cageName = l_cageName;
        i_len -= l_cageName.size() + 1;

        memcpy(&unitId, l_ptr, sizeof(unitId));
        unitId = ntohl(unitId);
        l_ptr += sizeof(unitId);
        i_len -= sizeof(unitId);

		memcpy(&cageFlags, l_ptr, sizeof(cageFlags));
		cageFlags = ntohl(cageFlags);
		l_ptr += sizeof(cageFlags);
		i_len -= sizeof(cageFlags);

		// Get the number of nodeData structs from the buffer.
		memcpy(&listSize, l_ptr, sizeof(listSize));
		listSize = ntohl(listSize);
		l_ptr += sizeof(listSize);
		i_len -= sizeof(listSize);

        // Since the return query data structure may be reused from a
        // previous call make sure the list is cleared.  Make sure this
        // is done before the if check below that may break us out to
        // prevent returning old data from previous iteration.     @09a
        nodeData.clear();

		// Check to see if the list is populated.
		if (0 == listSize) {
			// Nothing to create, just leave.
			break;
		}

		// Create any list entries.
		for (uint32_t i = 0; i < listSize; i++) {
			nodeData.push_back(ecmdNodeData());
		}

		std::list<ecmdNodeData>::iterator nodeitor = nodeData.begin();

		// Unflatten list data.
		while (nodeitor != nodeData.end()) {
			rc = nodeitor->unflatten(l_ptr, i_len);

			if (rc) break;
			/*
                         * l_ptr is not passed by reference so now that we 
                         * have it populated increment by the actual size.
                         */
			l_ptr += nodeitor->flattenSize();
			nodeitor++;
		}
		if (rc) break; // make sure we get to single exit with bad rc

	} while (0);    // <- single exit.

	return rc;
}

uint32_t ecmdCageData::flattenSize() const {

	uint32_t flatSize = 0;
	std::list<ecmdNodeData>::const_iterator nodeitor = nodeData.begin();

	do {    // Single entry ->

		/*
		 * Every struct entry shall place in the buffer a 32bit value to		 * contain a magic header used to identify itself.  This will 
		 * be used to make sure the code is looking at the expected 
		 * struct.  So...
		 * 
		 * Add the size of magic header.
		 */
		flatSize += sizeof(CAGE_HDR_MAGIC);

		// Size of non-list member data.
		flatSize += sizeof(cageId) + (cageName.size() + 1) + sizeof(unitId) + sizeof(cageFlags);

		/* 
		 * Every struct entry which contains a list of other structs is
		 * required to put into the buffer a 32bit value describing the
		 * number of structures in its list.  So...
		 * 
		 * Add one for the nodeData list counter.
		 */
		flatSize += sizeof(uint32_t);

		// If the nodeData list does not contain any structs break out.
		if (0 == nodeData.size()) {
			break;
		}

		// Size of list member data.
		while (nodeitor != nodeData.end()) {
			flatSize += nodeitor->flattenSize();
			nodeitor++;
		}

	} while (0);    // <- single exit.

	return flatSize;
}

/** @brief Used to sort Cage entries in an ecmdCageData list. */
bool ecmdCageData::operator<(const ecmdCageData& rhs) const {

  if (cageId < rhs.cageId) {
    return true;
  }
       
  return false;
}

#ifndef ECMD_STRIP_DEBUG
void ecmdCageData::printStruct() const {

	std::list<ecmdNodeData>::const_iterator nodeitor = nodeData.begin();

	printf("\n\t\tCage Data:\n");

	// Print non-list data.
	printf("\t\tCage ID: 0x%08x\n", cageId);
    printf("\t\tCage Name: %s\n", cageName.c_str());
	printf("\t\tUnit ID: 0x%08x\n", unitId);
	printf("\t\tCage Flags: 0x%08x\n", cageFlags);

	// Print list data.
	if (nodeData.size() == 0) {
		printf("\t\tNo node data.\n");
	}

	while (nodeitor != nodeData.end()) {
		nodeitor->printStruct();	
		nodeitor++;
	}
}
#endif


/*
 * The following methods for the ecmdQueryData struct will flatten, unflatten &
 * get the flattened size of the struct.
 */
uint32_t ecmdQueryData::flatten(uint8_t *o_buf, uint32_t &i_len) const{

	uint32_t tmpData32 = 0;
	uint32_t listSize  = 0;
	uint32_t rc	= ECMD_SUCCESS;

	uint8_t *l_ptr = o_buf;

	std::list<ecmdCageData>::const_iterator cageitor = cageData.begin();

	do {    // Single entry ->

		if (this->flattenSize() > i_len) {
			// Generate an error for buffer overflow conditions.
			ETRAC("Buffer overflow occured - "
                               "structure size = %d; "
                               "input length = %d",
                               this->flattenSize(), i_len);
			rc = ECMD_DATA_OVERFLOW;
			break;
		}

		// Insert magic header in the buffer.
		tmpData32 = htonl(QD_HDR_MAGIC);		
		memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
		l_ptr += sizeof(QD_HDR_MAGIC);
		i_len -= sizeof(QD_HDR_MAGIC);

		// Copy non-list data.
		tmpData32 = htonl((uint32_t)detailLevel);
		memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
		l_ptr += sizeof(detailLevel);
		i_len -= sizeof(detailLevel);

		/*
		 * Figure out how many cageData structs are in the list for 
		 * future unflattening.
		 */
		listSize = cageData.size();
		tmpData32 = htonl(listSize);
		memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
		l_ptr += sizeof(listSize);
		i_len -= sizeof(listSize);

		if (0 == listSize) {
			/*
			 * There are no cageData structs in this list. Don't 
			 * bother attempting to loop on cageData list.
			 */
			break;
		}

		// Copy list data.
		while (cageitor != cageData.end()) {
			rc = cageitor->flatten(l_ptr, i_len);

			if (rc) break;  // stop on fail and exit
			/*
                         * l_ptr is not passed by reference so now that we 
                         * have it populated increment by the actual size.
                         */
			l_ptr += cageitor->flattenSize();
			cageitor++;
		}
		if (rc) break; // make sure we get to single exit with bad rc

	} while (0);    // <- single exit.

	return rc;
}

uint32_t ecmdQueryData::unflatten(const uint8_t *i_buf, uint32_t &i_len) {

	uint8_t *l_ptr = (uint8_t *) i_buf;

	uint32_t hdrCheck = 0;
	uint32_t listSize = 0;
	uint32_t rc       = ECMD_SUCCESS;
    uint32_t l_orig_i_len = i_len;      //@01a

	do {    // Single entry ->

		if (this->flattenSize() > i_len) {
			// Generate an error for buffer overflow conditions.
			ETRAC("Buffer overflow occured - "
                               "structure size = %d; "
                               "input length = %d",
                               this->flattenSize(), i_len);
			rc = ECMD_DATA_OVERFLOW;
			break;
		}

		// Get and verify the magic header.
		memcpy(&hdrCheck, l_ptr, sizeof(hdrCheck));
		hdrCheck = ntohl(hdrCheck);
		l_ptr += sizeof(hdrCheck);
		i_len -= sizeof(hdrCheck);

		if (QD_HDR_MAGIC != hdrCheck) {
			ETRAC("Buffer header does not match struct header - "
                               "Struct header: 0x%08x; read from buffer as: "
                               "0x%08x", QD_HDR_MAGIC, hdrCheck);
			rc = ECMD_INVALID_ARRAY;
			break;
		}

		// Unflatten non-list data.
		memcpy(&detailLevel, l_ptr, sizeof(detailLevel));
		detailLevel = (ecmdQueryDetail_t) ntohl((uint32_t) detailLevel);
		l_ptr += sizeof(detailLevel);
		i_len -= sizeof(detailLevel);

		// Get the number of cageData structs from the buffer.
		memcpy(&listSize, l_ptr, sizeof(listSize));
		listSize = ntohl(listSize);
		l_ptr += sizeof(listSize);
		i_len -= sizeof(listSize);

        // Since the return query data structure may be reused from a
        // previous call make sure the list is cleared.  Make sure this
        // is done before the if check below that may break us out to
        // prevent returning old data from previous iteration.     @09a
        cageData.clear();

		// Check to see if the list is populated.
		if (0 == listSize) {
			// Nothing to create, just leave.
			break;
		}

		// Create any list entries.
		for (uint32_t i = 0; i < listSize; i++) {
			cageData.push_back(ecmdCageData());
		}

		std::list<ecmdCageData>::iterator cageitor = cageData.begin();

		// Unflatten list data.
		while (cageitor != cageData.end()) {
		        rc = cageitor->unflatten(l_ptr, i_len);

			if (rc) break;  // stop on fail and exit
			/*
                         * l_ptr is not passed by reference so now that we 
                         * have it populated increment by the actual size.
                         */
			l_ptr += cageitor->flattenSize();
			cageitor++;
		}
		if (rc) break; // make sure we get to single exit with bad rc

		// Check for underflow condition.  As each nested structure is
		// unflattened it decrements i_len.  The buffer and the unflattened
		// data should match in size if everything worked correctly.  So after
		// everything is unflattened we should have i_len = 0        @01a
		if (i_len)
		{
		    // Error.  Unflattened data didn't fill the whole buffer
		    // Either the calculation of the buffer size needed is
		    // wrong (to big) or there was an error during the unflatten
		    // and not all the data was restored.
		    ETRAC("Buffer underflow occured - "
			   "input length = %d, "
			   "length left over = %d ",
			   l_orig_i_len, i_len);
		    rc = ECMD_DATA_UNDERFLOW;
		    break;
		}

	} while (0);    // <- single exit.

	return rc;
}

uint32_t ecmdQueryData::flattenSize() const {

	uint32_t flatSize = 0;
	std::list<ecmdCageData>::const_iterator cageitor = cageData.begin();

	do {    // Single entry ->

		/*
		 * Every struct entry shall place in the buffer a 32bit value to		 * contain a magic header used to identify itself.  This will 
		 * be used to make sure the code is looking at the expected 
		 * struct.  So...
		 * 
		 * Add the size of magic header.
		 */
		flatSize += sizeof(QD_HDR_MAGIC);

		// Size of non-list member data.
		flatSize += sizeof(detailLevel);

		/* 
		 * Every struct entry which contains a list of other structs is
		 * required to put into the buffer a 32bit value describing the
		 * number of structures in its list.  So...
		 * 
		 * Add one for the cageData list counter.
		 */
		flatSize += sizeof(uint32_t);

		// If the cageData list does not contain any structs break out.
		if (0 == cageData.size()) {
			break;
		}

		// Size of list member data.
		while (cageitor != cageData.end()) {
			flatSize += cageitor->flattenSize();
			cageitor++;
		}

	} while (0);    // <- single exit.

	return flatSize;
}

#ifndef ECMD_STRIP_DEBUG
void  ecmdQueryData::printStruct() const {

	std::list<ecmdCageData>::const_iterator cageitor = cageData.begin();

	printf("\n\tQuery Data:\n");

	// Print non-list data.
	printf("\tDetail level: 0x%08x\n", (uint32_t) detailLevel);

	// Print list data.
	if (cageData.size() == 0) {
		printf("\tNo cage data.\n");
	}

	while (cageitor != cageData.end()) {
		cageitor->printStruct();
		cageitor++;
	}
}
#endif




/*
 * The following methods for the ecmdSpyData struct will flatten, unflatten &
 * get the flattened size of the struct.
 */
bool ecmdSpyData::isChipUnitMatch(std::string &i_chipUnitType) {
  /* If either matches, return true */
  if (i_chipUnitType == relatedChipUnit || i_chipUnitType == relatedChipUnitShort) {
    return true;
  }

  return false;
}

uint32_t ecmdSpyData::flatten(uint8_t *o_buf, uint32_t i_len) const{

        uint32_t tmpData32 = 0;
	uint64_t tmpData64 = 0;
        uint32_t enumsListSize  = 0;
        uint32_t epCheckersListSize  = 0;
        uint32_t rc     = ECMD_SUCCESS;

        uint8_t *l_ptr = o_buf;

        std::list<std::string>:: const_iterator listStringIter;


        do {    // Single entry ->

	    if (this->flattenSize() > i_len) {
		// Generate an error for buffer overflow conditions.
		ETRAC("Buffer overflow occured - "
		       "structure size = %d; "
		       "input length = %d",
		       this->flattenSize(), i_len);
		rc = ECMD_DATA_OVERFLOW;
		break;
	    }

	    // Copy non-list data.
	    // spyName  
	    memcpy(l_ptr, spyName.c_str(), spyName.size() + 1);
	    l_ptr += spyName.size() + 1;
	    i_len -= spyName.size() + 1;

	    // spyId
	    tmpData64 = htonll(spyId);
	    memcpy(l_ptr, &tmpData64, sizeof(tmpData64));
	    l_ptr += sizeof(spyId);
	    i_len -= sizeof(spyId);

	    // bitLength
	    tmpData32 = htonl((uint32_t)bitLength);
	    memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
	    l_ptr += sizeof(bitLength);
	    i_len -= sizeof(bitLength);

	    // spyType
	    tmpData32 = htonl((uint32_t)spyType);
	    memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
	    l_ptr += sizeof(spyType);
	    i_len -= sizeof(spyType);

	    // isEccChecked (bool, store in uint32_t) @08 start
            tmpData32 = htonl( (uint32_t)isEccChecked );
	    memcpy( l_ptr, &tmpData32, sizeof(tmpData32) ); 
	    l_ptr += sizeof(tmpData32);
	    i_len -= sizeof(tmpData32);

	    // isEnumerated (bool, store in uint32_t)
            tmpData32 = htonl( (uint32_t)isEnumerated );
	    memcpy( l_ptr, &tmpData32, sizeof(tmpData32) ); 
	    l_ptr += sizeof(tmpData32);
	    i_len -= sizeof(tmpData32); 
	    
	    //"isChipUnitRelated" (bool, store in uint32_t)        
	    tmpData32 = htonl( (uint32_t)isChipUnitRelated ); 
	    memcpy( l_ptr, &tmpData32, sizeof(tmpData32) ); 
	    l_ptr += sizeof(tmpData32); 
	    i_len -= sizeof(tmpData32);
        
            //relatedChipUnit     
	    memcpy(l_ptr, relatedChipUnit.c_str(), relatedChipUnit.size() + 1); 
	    l_ptr += relatedChipUnit.size() + 1;
	    i_len -= relatedChipUnit.size() + 1; 
	    
	    //relatedChipUnitShort     
	    memcpy(l_ptr, relatedChipUnitShort.c_str(), relatedChipUnitShort.size() + 1);
	    l_ptr += relatedChipUnitShort.size() + 1;
	    i_len -= relatedChipUnitShort.size() + 1;

	    // clockDomain
	    memcpy(l_ptr, clockDomain.c_str(), clockDomain.size() + 1);
	    l_ptr += clockDomain.size() + 1;
	    i_len -= clockDomain.size() + 1;

	    // clockState
	    tmpData32 = htonl((uint32_t)clockState);
	    memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
	    l_ptr += sizeof(clockState);
	    i_len -= sizeof(clockState);


	    /*
	     * Figure out how many enum strings are in the list for
	     * future unflattening.
	     */
	    enumsListSize = enums.size();

	    // add enumsListSize
	    tmpData32 = htonl(enumsListSize);
	    memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
	    l_ptr += sizeof(enumsListSize);
	    i_len -= sizeof(enumsListSize);

	    if (enumsListSize != 0)   // don't bother grabbing list of enums if there aren't any
	    {
		// Copy enum List of strings
		for (listStringIter = enums.begin(); listStringIter != enums.end(); ++listStringIter)
		{
		    memcpy(l_ptr, (*listStringIter).c_str(), (*listStringIter).size() + 1);
		    l_ptr += (*listStringIter).size() + 1;
		    i_len -= (*listStringIter).size() + 1;
		}

	    }

	    /*
	     * Figure out how many epChecker strings are in the list for
	     * future unflattening.
	     */
	    epCheckersListSize = epCheckers.size();

	    // add epCheckersListSize
	    tmpData32 = htonl(epCheckersListSize);
	    memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
	    l_ptr += sizeof(epCheckersListSize);
	    i_len -= sizeof(epCheckersListSize);

	    if (0 != epCheckersListSize) // don't bother grabbing list of enums if there aren't any
	    {
		// Copy epCheckers List of strings
		for (listStringIter = epCheckers.begin(); listStringIter != epCheckers.end(); ++listStringIter)
		{
		    memcpy(l_ptr, (*listStringIter).c_str(), (*listStringIter).size() + 1);
		    l_ptr += (*listStringIter).size() + 1;
		    i_len -= (*listStringIter).size() + 1;
		}
	    }

	    // Do final check
	    if (i_len != 0)
	    {
		ETRAC("Buffer size mismacth occured - "
		       "structure size = %d; "
		       "leftover length = %d",
		       this->flattenSize(), i_len);
		rc = ECMD_DATA_OVERFLOW;
		break;
	    }

        } while (0);    // <- single exit.

        return rc;
}



uint32_t ecmdSpyData::unflatten(const uint8_t *i_buf, uint32_t i_len) {

        uint8_t *l_ptr = (uint8_t *) i_buf;
        uint32_t rc       = ECMD_SUCCESS;

        uint32_t tmpData32 = 0;  // @08
        uint32_t enumsListSize  = 0;
        uint32_t epCheckersListSize  = 0;
	uint32_t loop = 0;
	int l_left = (int) i_len;


        do {    // Single entry ->

	    // Unflatten non-list data.
	    // spyName
	    std::string l_spyName = (const char *) l_ptr;
	    spyName = (const char *) l_ptr;
	    l_ptr += l_spyName.size() + 1;
	    l_left -= l_spyName.size() + 1;

	    // spyId
	    memcpy(&spyId, l_ptr, sizeof(spyId));
	    spyId = ntohll(spyId);
	    l_ptr += sizeof(spyId);
	    l_left -= sizeof(spyId);

	    // bitLength
	    memcpy(&bitLength, l_ptr, sizeof(bitLength));
	    bitLength = ntohl(bitLength);
	    l_ptr += sizeof(bitLength);
	    l_left -= sizeof(bitLength);

	    // spyType
	    memcpy(&spyType, l_ptr, sizeof(spyType));
	    spyType = (ecmdSpyType_t) ntohl((uint32_t)spyType);
	    l_ptr += sizeof(spyType);
	    l_left -= sizeof(spyType);

	    // isEccChecked (bool, stored as uint32_t) @08 start
	    memcpy( &tmpData32, l_ptr, sizeof(tmpData32) );
            isEccChecked = (bool)ntohl( tmpData32 );
	    l_ptr += sizeof(tmpData32);
	    l_left -= sizeof(tmpData32);

	    // isEnumerated (bool, stored as uint32_t)
	    memcpy( &tmpData32, l_ptr, sizeof(tmpData32) );
            isEnumerated = (bool)ntohl( tmpData32 );
	    l_ptr += sizeof(tmpData32);
	    l_left -= sizeof(tmpData32);

        //isChipUnitRelated
        memcpy(&tmpData32, l_ptr, sizeof(tmpData32));
        isChipUnitRelated = (bool)ntohl(tmpData32);
        l_ptr += sizeof(tmpData32);
        l_left -= sizeof(tmpData32);

        //relatedChipUnit
        std::string l_relatedChipUnit = (const char *) l_ptr;  //maybe this can be 1 line?
        relatedChipUnit = l_relatedChipUnit;
        l_ptr += l_relatedChipUnit.size() + 1;
        l_left -= l_relatedChipUnit.size() + 1;

        //relatedChipUnitShort
        std::string l_relatedChipUnitShort = (const char *) l_ptr;  //maybe this can be 1 line?
        relatedChipUnitShort = l_relatedChipUnitShort;
        l_ptr += l_relatedChipUnitShort.size() + 1;
        l_left -= l_relatedChipUnitShort.size() + 1;

	    // clockDomain
	    std::string l_clockDomain = (const char *) l_ptr;
	    clockDomain = l_clockDomain;
	    l_ptr += l_clockDomain.size() + 1;
	    l_left -= l_clockDomain.size() + 1;

	    // clockState
	    memcpy(&clockState, l_ptr, sizeof(clockState));
	    clockState = (ecmdClockState_t) ntohl((uint32_t)clockState);
	    l_ptr += sizeof(clockState);
	    l_left -= sizeof(clockState);

	    // Figure out how many enum strings are in the list and then unflatten
	    memcpy(&enumsListSize, l_ptr, sizeof(enumsListSize));
	    enumsListSize = ntohl(enumsListSize);
	    l_ptr += sizeof(enumsListSize);
	    l_left -= sizeof(enumsListSize);

        // Since the return query data structure may be reused from a
        // previous call make sure the list is cleared            @09a
        enums.clear();

	    // create list of string of enums (nothing happens if enumsListSize=0)
	    for (loop = 0; loop < enumsListSize; loop++)
	    {
		std::string l_str_enum = (char *)l_ptr; 
		enums.push_back(l_str_enum);
                l_ptr += l_str_enum.size() + 1;
                l_left -= l_str_enum.size() + 1;
	    }

            // Figure out how many epChecker strings are in the list and then unflatten
	    memcpy(&epCheckersListSize, l_ptr, sizeof(epCheckersListSize));
	    epCheckersListSize = ntohl(epCheckersListSize);
	    l_ptr += sizeof(epCheckersListSize);
	    l_left -= sizeof(epCheckersListSize);

        // Since the return query data structure may be reused from a
        // previous call make sure the list is cleared            @09a
        epCheckers.clear();

	    // create list of string of enums (nothing happens if enumsListSize=0)
	    for (loop = 0; loop < epCheckersListSize; loop++)
	    {
		std::string l_str_epCheck = (char *)l_ptr; 
		epCheckers.push_back(l_str_epCheck);
                l_ptr += l_str_epCheck.size() + 1;
                l_left -= l_str_epCheck.size() + 1;
	    }

	    if (l_left < 0)
	    {	
		// Generate an error for buffer overflow conditions.
		ETRAC("Buffer overflow occured - "
		       "structure size = %d; "
		       "input length = %x; "
		       "remainder = %d\n",
		       this->flattenSize(), i_len, l_left);
		rc = ECMD_DATA_OVERFLOW;
		break;
	    }
	    if (l_left > 0)
	    {	
		// Generate an error for buffer underflow conditions.
		ETRAC("Buffer underflow occured - "
		       "structure size = %d; "
		       "input length = %x; "
		       "remainder = %d\n",
		       this->flattenSize(), i_len, l_left);
		rc = ECMD_DATA_UNDERFLOW;
		break;
	    }

        } while (0);    // <- single exit.

        return rc;
}


uint32_t ecmdSpyData::flattenSize() const {

        uint32_t flatSize = 0;
        uint32_t enumsListSize  = 0;
        uint32_t epCheckersListSize  = 0;
        std::list<std::string>:: const_iterator listStringIter;


        do {    // Single entry ->


                // Size of non-list member data.
                flatSize += (spyName.size() + 1
                             + sizeof(spyId)
                             + sizeof(bitLength)
                             + sizeof(spyType)
                             + sizeof(uint32_t) // for isEccChecked @08
                             + sizeof(uint32_t) // for isEnumerated
                             + sizeof(uint32_t) // for isChipUnitRelated 
                             + relatedChipUnit.size() + 1               
                             + relatedChipUnitShort.size() + 1               
                             + clockDomain.size() + 1
                             + sizeof(clockState));

                /*
                 * Figure out how many enum strings are in the list for
                 * future unflattening.
                 */
                enumsListSize = enums.size();

                // add enumsListSize
                flatSize += sizeof(enumsListSize);

                if (0 != enumsListSize) {  // don't bother grabbing list of enums if there aren't any
                    // add length of each string
                    for (listStringIter = enums.begin(); listStringIter != enums.end(); ++listStringIter)
                    {
                        flatSize += (*listStringIter).size() + 1;
                    }

                }

                epCheckersListSize = epCheckers.size();

                // add epCheckersListSize
                flatSize += sizeof(epCheckersListSize);

                if (0 != epCheckersListSize) {  // don't bother grabbing list of epCheckers if there aren't any
                    // add length of each epCheckers string
                    for (listStringIter = epCheckers.begin(); listStringIter != epCheckers.end(); ++listStringIter)
                    {
                        flatSize += (*listStringIter).size() + 1;
                    }

                }


        } while (0);    // <- single exit.

        return flatSize;
}

#ifndef ECMD_STRIP_DEBUG
void  ecmdSpyData::printStruct() const {

        uint32_t enumsListSize  = enums.size();
        uint32_t epCheckersListSize  = epCheckers.size();

        std::list<std::string>:: const_iterator listStringIter;


        printf("\n\t--- Spy Data Structure ---\n");

        // Print non-list data.
        printf("\tSpy Name:  %s\n", spyName.c_str());
#ifdef _LP64
        printf("\tSpy ID: 0x%016lx\n", spyId);
#else
        printf("\tSpy ID: 0x%016llx\n", spyId);
#endif
        printf("\tBit Length: 0x%08x\n", (uint32_t) bitLength);
        printf("\tSpy Type: 0x%08x\n", (uint32_t) spyType);
        printf("\tisEccChecked: 0x%08x\n", (uint32_t) isEccChecked);
        printf("\tisEnumerated: 0x%08x\n", (uint32_t) isEnumerated);
        printf("\tisChipUnitRelated: 0x%08x\n", (uint32_t) isChipUnitRelated);
        printf("\trelatedChipUnit:  %s\n", relatedChipUnit.c_str());
        printf("\trelatedChipUnitShort:  %s\n", relatedChipUnitShort.c_str());
        printf("\tSpy Name:  %s\n", clockDomain.c_str());
        printf("\tClock State: 0x%08x\n", (uint32_t) clockState);

        // print enums List Data
        if (0 == enumsListSize) {
            printf("\tNo entries in enums list\n");
        }
        else {
            // display each enum string
            printf("\tList of Enum Strings: \n");
            for (listStringIter = enums.begin(); listStringIter != enums.end(); ++listStringIter)
            {
                printf("\t\t%s\n", (*listStringIter).c_str());
            }
        }

        // print epCheckers List Data
        if (0 == epCheckersListSize) {
            printf("\tNo entries in epCheckers List\n");
        }
        else {
            // display each enum string
            printf("\tList of epCheckers Strings: \n");
            for (listStringIter = epCheckers.begin(); listStringIter != epCheckers.end(); ++listStringIter)
            {
                printf("\t\t%s\n", (*listStringIter).c_str());
            }
        }

}
#endif

// @02
/*
 * The following methods for the ecmdArrayEntry struct will flatten, unflatten &
 * get the flattened size of the struct.
 */
uint32_t ecmdArrayEntry::flatten(uint8_t * o_buf, uint32_t i_len) const{

    uint32_t tmpData32 = 0;
    uint32_t dataBufSize = 0;    // temp holder for ecmdDataBuffer size
    uint32_t l_rc = ECMD_SUCCESS;

    uint8_t * l_ptr = o_buf;

    do {    // Single entry ->

        // Check for buffer size mismatch (overflow or underflow)
        if ( this->flattenSize() != i_len ) {
	    ETRAC("Buffer overflow occurred - "
                   "structure size = %d; input length = %d",
                   this->flattenSize(), i_len );
	    l_rc = ECMD_DATA_OVERFLOW;
	    break;
        }

        // Write the size of "address", to check against when unflattening
        dataBufSize = address.flattenSize();
        tmpData32 = htonl( dataBufSize );
        memcpy( l_ptr, &tmpData32, sizeof(tmpData32) );
        l_ptr += sizeof( dataBufSize );
        i_len -= sizeof( dataBufSize );

        // Write address into the output buffer
        l_rc = address.flatten( l_ptr, dataBufSize );
        if ( l_rc != ECMD_DBUF_SUCCESS ) {
            break;
        } else {
           l_rc = ECMD_SUCCESS;
        }
        l_ptr += dataBufSize;
        i_len -= dataBufSize;

        // Write the size of "buffer", to check against when unflattening
        dataBufSize = buffer.flattenSize();
        tmpData32 = htonl( dataBufSize );
        memcpy( l_ptr, &tmpData32, sizeof(tmpData32) );
        l_ptr += sizeof( dataBufSize );
        i_len -= sizeof( dataBufSize );

        // Write contents of "buffer" into the output buffer
        l_rc = buffer.flatten( l_ptr, dataBufSize );
        if ( l_rc != ECMD_DBUF_SUCCESS ) {
            break;
        } else {
           l_rc = ECMD_SUCCESS;
        }
        l_ptr += dataBufSize;
        i_len -= dataBufSize;

        // Write "rc" into the output buffer
        tmpData32 = htonl( rc );
        memcpy( l_ptr, &tmpData32, sizeof(tmpData32) );
        l_ptr += sizeof( rc );
        i_len -= sizeof( rc );

        // If the length has not decremented to 0, something is wrong
        if ( i_len != 0 ) {
	    ETRAC("Buffer overflow occurred - "
                   "leftover data bytes = %d", i_len );
           l_rc = ECMD_DATA_OVERFLOW;
           break;
        }

    } while (0);	// <- single exit.

    return l_rc;
}

uint32_t ecmdArrayEntry::unflatten(const uint8_t * i_buf, uint32_t i_len) {

    uint32_t rcTemp = 0;
    uint32_t dataBufSize = 0;
    uint32_t l_rc = ECMD_SUCCESS;

    uint8_t * l_ptr = (uint8_t *) i_buf;

    do {	// Single entry ->

        // Get the size of "address" to pass to unflatten()
        memcpy( &dataBufSize, l_ptr, sizeof(dataBufSize) );
        dataBufSize = ntohl( dataBufSize );
        l_ptr += sizeof( dataBufSize );
        i_len -= sizeof( dataBufSize );

        // Unflatten "address" from the input buffer
        l_rc = address.unflatten( l_ptr, dataBufSize );
        if ( l_rc != ECMD_DBUF_SUCCESS ) {
            break;
        } else {
            l_rc = ECMD_SUCCESS;
        }
        l_ptr += dataBufSize;
        i_len -= dataBufSize;

        // Get the size of "buffer" to pass to unflatten()
        memcpy( &dataBufSize, l_ptr, sizeof(dataBufSize) );
        dataBufSize = ntohl( dataBufSize );
        l_ptr += sizeof( dataBufSize );
        i_len -= sizeof( dataBufSize );

        // Unflatten "buffer" from the input buffer
        l_rc = buffer.unflatten( l_ptr, dataBufSize );
        if ( l_rc != ECMD_DBUF_SUCCESS ) {
            break;
        } else {
            l_rc = ECMD_SUCCESS;
        }
        l_ptr += dataBufSize;
        i_len -= dataBufSize;

        // Get "rc" from the input buffer
        memcpy( &rcTemp, l_ptr, sizeof(rcTemp) );
        rc = ntohl( rcTemp );
        l_ptr += sizeof( rcTemp );
        i_len -= sizeof( rcTemp );

        // If the length has not decremented to 0, something is wrong
        if ( i_len != 0 )
        {
          ETRAC("Buffer overflow occurred - "
                 "leftover data bytes = %d", i_len );
          l_rc = ECMD_DATA_OVERFLOW;
          break;
        }
	
    } while (0);	// <- single exit.

    return l_rc;
}

uint32_t ecmdArrayEntry::flattenSize() const {

	uint32_t flatSize = 0;

        uint32_t dataBufSize = 0;  // A size is stored for each ecmdDataBuf

	flatSize += sizeof( dataBufSize );  // Size of member "address"
        flatSize += address.flattenSize();  // Space for "address" flattened
        
        flatSize += sizeof( dataBufSize );  // Size of member "buffer"
        flatSize += buffer.flattenSize();   // Space for "buffer" flattened

        flatSize += sizeof( rc );           // Space for member "rc"

	return flatSize;
}  


#ifndef ECMD_STRIP_DEBUG
void ecmdArrayEntry::printStruct() const {

    uint32_t tmpData = 0;
    std::string tmpString;

    printf("\n\t\teCMD Array Entry:\n");

    tmpData = address.getBitLength();
    tmpString = address.genHexLeftStr(0, tmpData);
    printf("\t\t\taddress bitlength: %d\n", tmpData );
    printf("\t\t\taddress wordlength: %d\n", address.getWordLength() );
    printf("\t\t\taddress data: %s\n", tmpString.c_str() );

    tmpData = buffer.getBitLength();
    tmpString = buffer.genHexLeftStr(0, tmpData);
    printf("\t\t\tbuffer bitlength: %d\n", tmpData );
    printf("\t\t\tbuffer wordlength: %d\n", buffer.getWordLength() );
    printf("\t\t\tbuffer data: %s\n", tmpString.c_str() );

    printf("\t\t\treturn code (rc): 0x%x\n", rc );
}
#endif
// @02 end


/*
 * The following methods for the ecmdRingData struct will flatten, unflatten &
 * get the flattened size of the struct.
 */
bool ecmdRingData::isChipUnitMatch(std::string &i_chipUnitType) {
  /* If either matches, return true */
  if (i_chipUnitType == relatedChipUnit || i_chipUnitType == relatedChipUnitShort) {
    return true;
  }

  return false;
}

uint32_t ecmdRingData::flatten(uint8_t *o_buf, uint32_t i_len) const{

        uint32_t tmpData32 = 0;
        uint64_t tmpData64 = 0;
        uint32_t ringNamesListSize  = 0;
        uint32_t ringIdListSize = 0;
        uint32_t rc     = ECMD_SUCCESS;

        uint8_t *l_ptr = o_buf;

        std::list<std::string>:: const_iterator ringNamesIter;
        std::list<uint64_t>::const_iterator ringIdIter;


        do {    // Single entry ->

	    if (this->flattenSize() > i_len) {
		// Generate an error for buffer overflow conditions.
		ETRAC("Buffer overflow occured - "
		       "structure size = %d; "
		       "input length = %d",
		       this->flattenSize(), i_len);
		rc = ECMD_DATA_OVERFLOW;
		break;
	    }

	    // Copy non-list data.
	    // address
	    tmpData32 = htonl(address);
	    memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
	    l_ptr += sizeof(address);
	    i_len -= sizeof(address);

	    // bitLength
	    tmpData32 = htonl((uint32_t)bitLength);
	    memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
	    l_ptr += sizeof(bitLength);
	    i_len -= sizeof(bitLength);

	    // hasInversionMask (bool, store in uint32_t) @08 start
	    tmpData32 = htonl((uint32_t)hasInversionMask);
	    memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
	    l_ptr += sizeof(tmpData32);
	    i_len -= sizeof(tmpData32);

	    // supportsBroadsideLoad (bool, store in uint32_t)
	    tmpData32 = htonl((uint32_t)supportsBroadsideLoad);
	    memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
	    l_ptr += sizeof(tmpData32);
	    i_len -= sizeof(tmpData32);

	    // isCheckable (bool, store in uint32_t)
	    tmpData32 = htonl((uint32_t)isCheckable);
	    memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
	    l_ptr += sizeof(tmpData32);
	    i_len -= sizeof(tmpData32);

        //isChipUnitRelated
        tmpData32 = htonl( (uint32_t)isChipUnitRelated );
        memcpy( l_ptr, &tmpData32, sizeof(tmpData32) );
        l_ptr += sizeof(tmpData32);
        i_len -= sizeof(tmpData32);

        //relatedChipUnit
        memcpy(l_ptr, relatedChipUnit.c_str(), relatedChipUnit.size() + 1);
        l_ptr += relatedChipUnit.size() + 1;
        i_len -= relatedChipUnit.size() + 1;

        //relatedChipUnitShort
        memcpy(l_ptr, relatedChipUnitShort.c_str(), relatedChipUnitShort.size() + 1);
        l_ptr += relatedChipUnitShort.size() + 1;
        i_len -= relatedChipUnitShort.size() + 1;

	    // clockDomain
	    memcpy(l_ptr, clockDomain.c_str(), clockDomain.size() + 1);
	    l_ptr += clockDomain.size() + 1;
	    i_len -= clockDomain.size() + 1;

	    // clockState
	    tmpData32 = htonl((uint32_t)clockState);
	    memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
	    l_ptr += sizeof(clockState);
	    i_len -= sizeof(clockState);


	    /*
	     * Figure out how many ringName strings are in the list for
	     * future unflattening.
	     */
	    ringNamesListSize = ringNames.size();

	    // add ringNamesListSize
	    tmpData32 = htonl(ringNamesListSize);
	    memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
	    l_ptr += sizeof(ringNamesListSize);
	    i_len -= sizeof(ringNamesListSize);

	    if (ringNamesListSize != 0)   // don't bother grabbing list of ringNames if there aren't any
	    {
		// Copy List of ringName strings
		for (ringNamesIter = ringNames.begin(); ringNamesIter != ringNames.end(); ++ringNamesIter)
		{
		    memcpy(l_ptr, (*ringNamesIter).c_str(), (*ringNamesIter).size() + 1);
		    l_ptr += (*ringNamesIter).size() + 1;
		    i_len -= (*ringNamesIter).size() + 1;
		}

	    }

            // Store the ring ID list
            ringIdListSize = ringIds.size();

            // Save the number of ring ID elements
            tmpData32 = htonl(ringIdListSize);
            memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
            l_ptr += sizeof(ringIdListSize);
            i_len -= sizeof(ringIdListSize);
 
            // Store each ring ID (won't loop if list is empty)
            for (ringIdIter = ringIds.begin(); ringIdIter != ringIds.end(); ++ringIdIter)
            {
               tmpData64 = htonll(*ringIdIter);
               memcpy(l_ptr, &tmpData64, sizeof(tmpData64));
               l_ptr += sizeof(*ringIdIter);
               i_len -= sizeof(*ringIdIter);
            }

	    // Do final check
	    if (i_len != 0)
	    {
		ETRAC("Buffer size mismatch occured - "
		       "structure size = %d; "
		       "leftover length = %d",
		       this->flattenSize(), i_len);
		rc = ECMD_DATA_OVERFLOW;
		break;
	    }

        } while (0);    // <- single exit.

        return rc;
}



uint32_t ecmdRingData::unflatten(const uint8_t *i_buf, uint32_t i_len) {

        uint8_t *l_ptr = (uint8_t *) i_buf;
        uint32_t rc       = ECMD_SUCCESS;

        uint32_t tmpData32 = 0;
        uint32_t ringNamesListSize  = 0;
        uint32_t ringIdListSize = 0;
	uint32_t loop = 0;
	int l_left = (int) i_len;


        do {    // Single entry ->

	    // Unflatten non-list data.
	    // address
	    memcpy(&address, l_ptr, sizeof(address));
	    address = ntohl(address);
	    l_ptr += sizeof(address);
	    l_left -= sizeof(address);

	    // bitLength
	    memcpy(&bitLength, l_ptr, sizeof(bitLength));
	    bitLength = (int) ntohl(bitLength);
	    l_ptr += sizeof(bitLength);
	    l_left -= sizeof(bitLength);

	    // hasInversionMask (bool stored in uint32_t) @08 start
	    memcpy(&tmpData32, l_ptr, sizeof(tmpData32));
	    hasInversionMask = (bool)ntohl(tmpData32);
	    l_ptr += sizeof(tmpData32);
	    l_left -= sizeof(tmpData32);

	    // supportsBroadsideLoad (bool stored in uint32_t)
	    memcpy(&tmpData32, l_ptr, sizeof(tmpData32));
	    supportsBroadsideLoad = (bool)ntohl(tmpData32);
	    l_ptr += sizeof(tmpData32);
	    l_left -= sizeof(tmpData32);

	    // isCheckable (bool stored in uint32_t)
	    memcpy(&tmpData32, l_ptr, sizeof(tmpData32));
	    isCheckable = (bool)ntohl(tmpData32);
	    l_ptr += sizeof(tmpData32);
	    l_left -= sizeof(tmpData32);

        //isChipUnitRelated
        memcpy(&tmpData32, l_ptr, sizeof(tmpData32));
        isChipUnitRelated = (bool)ntohl(tmpData32);
        l_ptr += sizeof(tmpData32);
        l_left -= sizeof(tmpData32);

        //relatedChipUnit
        std::string l_relatedChipUnit = (const char *) l_ptr;  //maybe this can be 1 line?
        relatedChipUnit = l_relatedChipUnit;
        l_ptr += l_relatedChipUnit.size() + 1;
        l_left -= l_relatedChipUnit.size() + 1;

        //relatedChipUnitShort
        std::string l_relatedChipUnitShort = (const char *) l_ptr; 
        relatedChipUnitShort = l_relatedChipUnitShort;
        l_ptr += l_relatedChipUnitShort.size() + 1;
        l_left -= l_relatedChipUnitShort.size() + 1;

	    // clockDomain
	    std::string l_clockDomain = (const char *) l_ptr;
	    clockDomain = l_clockDomain;
	    l_ptr += l_clockDomain.size() + 1;
	    l_left -= l_clockDomain.size() + 1;

	    // clockState
	    memcpy(&clockState, l_ptr, sizeof(clockState));
	    clockState = (ecmdClockState_t) ntohl((uint32_t)clockState);
	    l_ptr += sizeof(clockState);
	    l_left -= sizeof(clockState);

	    // Figure out how many ringNames strings are in the list and then unflatten
	    memcpy(&ringNamesListSize, l_ptr, sizeof(ringNamesListSize));
	    ringNamesListSize = ntohl(ringNamesListSize);
	    l_ptr += sizeof(ringNamesListSize);
	    l_left -= sizeof(ringNamesListSize);

        // Since the return query data structure may be reused from a
        // previous call make sure the list is cleared            @09a
        ringNames.clear();

	    // create list of string of ringNames (nothing happens if ringNamesListSize=0)
	    for (loop = 0; loop < ringNamesListSize; loop++)
	    {
		std::string l_str_ringName = (char *)l_ptr; 
		ringNames.push_back(l_str_ringName);
                l_ptr += l_str_ringName.size() + 1;
                l_left -= l_str_ringName.size() + 1;
	    }

        // Fetch the number of ring IDs then unflatten them
        memcpy(&ringIdListSize, l_ptr, sizeof(ringIdListSize));
        ringIdListSize = ntohl(ringIdListSize);
        l_ptr += sizeof(ringIdListSize);
        l_left -= sizeof(ringIdListSize);

        // Since the return query data structure may be reused from a
        // previous call make sure the list is cleared            @09a
        ringIds.clear();

        // Re-create the list of ring IDs
        for (loop = 0; loop < ringIdListSize; ++loop)
        {
            uint64_t l_ringId = 0;
            memcpy(&l_ringId, l_ptr, sizeof(l_ringId));
            l_ringId = ntohll(l_ringId);

            ringIds.push_back(l_ringId);

            l_ptr += sizeof(l_ringId);
            l_left -= sizeof(l_ringId);
        }

	    // Do Final Checks
	    if (l_left < 0)
	    {	
		// Generate an error for buffer overflow conditions.
		ETRAC("Buffer overflow occured - "
		       "structure size = %d; "
		       "input length = %x; "
		       "remainder = %d\n",
		       this->flattenSize(), i_len, l_left);
		rc = ECMD_DATA_OVERFLOW;
		break;
	    }
	    if (l_left > 0)
	    {	
		// Generate an error for buffer underflow conditions.
		ETRAC("Buffer underflow occured - "
		       "structure size = %d; "
		       "input length = %x; "
		       "remainder = %d\n",
		       this->flattenSize(), i_len, l_left);
		rc = ECMD_DATA_UNDERFLOW;
		break;
	    }

        } while (0);    // <- single exit.

        return rc;
}


uint32_t ecmdRingData::flattenSize() const {

        uint32_t flatSize = 0;
        uint32_t ringNamesListSize  = 0;
        uint32_t ringIdListSize = 0;
        std::list<std::string>:: const_iterator ringNamesIter;


        do {    // Single entry ->


                // Size of non-list member data.
                flatSize += (sizeof(address)
                             + sizeof(bitLength)
                             + sizeof(uint32_t) // hasInversionMask @08
                             + sizeof(uint32_t) // supportsBroadsideLoad
			     + sizeof(uint32_t) // isCheckable
                             + sizeof(uint32_t) // isChipUnitRelated 
                             + relatedChipUnit.size() + 1           
                             + relatedChipUnitShort.size() + 1           
                             + clockDomain.size() + 1
                             + sizeof(clockState));

                /*
                 * Figure out how many enum strings are in the list for
                 * future unflattening.
                 */
                ringNamesListSize = ringNames.size();

                // add ringNamesListSize
                flatSize += sizeof(ringNamesListSize);

                if (0 != ringNamesListSize) {  // don't bother grabbing list of ringNamess if there aren't any
                    // add length of each string
                    for (ringNamesIter = ringNames.begin(); ringNamesIter != ringNames.end(); ++ringNamesIter)
                    {
                        flatSize += (*ringNamesIter).size() + 1;
                    }

                }

                // Add the count and size of hashed ring IDs
                ringIdListSize = ringIds.size();

                flatSize += sizeof(ringIdListSize);

                // each ring ID entry is a uint32_t
                flatSize += ringIdListSize * sizeof(uint32_t);

        } while (0);    // <- single exit.

        return flatSize;
}

#ifndef ECMD_STRIP_DEBUG
void  ecmdRingData::printStruct() const {

        uint32_t ringNamesListSize  = ringNames.size();

        std::list<std::string>:: const_iterator ringNamesIter;
        std::list<uint64_t>::const_iterator ringIdIter;


        printf("\n\t--- Ring Data Structure ---\n");

        // Print non-list data.
        printf("\tAddress:  0x%08x\n", address);
        printf("\tBit Length: 0x%08x\n", (uint32_t) bitLength);
        printf("\thasInversionMask: 0x%08x\n", (uint32_t) hasInversionMask);
        printf("\tsupportsBroadsideLoad: 0x%08x\n", (uint32_t) supportsBroadsideLoad);
        printf("\tisCheckable: 0x%08x\n", (uint32_t) isCheckable);
        printf("\tisChipUnitRelated: 0x%08x\n", (uint32_t) isChipUnitRelated);
        printf("\trelatedChipUnit:  %s\n", relatedChipUnit.c_str());
        printf("\trelatedChipUnitShort:  %s\n", relatedChipUnitShort.c_str());
        printf("\tClock Domain:  %s\n", clockDomain.c_str());
        printf("\tClock State: 0x%08x\n", (uint32_t) clockState);

        // print ringNames List Data
        if (ringNamesListSize == 0) {
            printf("\tNo entries in ringNames list\n");
        }
        else {
            // display each ringName string
            printf("\tList of ringName Strings: \n");
            for (ringNamesIter = ringNames.begin(); ringNamesIter != ringNames.end(); ++ringNamesIter)
            {
                printf("\t\t%s\n", (*ringNamesIter).c_str());
            }
        }

        // Print ring ID list
        if (ringIds.size() == 0) {
            printf("\tNo entries in ringIds list\n");
        } else {
            // Display each ring ID entry
            printf("\tList of ringId entries:\n");
            for (ringIdIter = ringIds.begin(); ringIdIter != ringIds.end(); ++ ringIdIter) {
#ifdef _LP64
                printf("\t\t0x%016lx\n", *ringIdIter);
#else
                printf("\t\t0x%016llx\n", *ringIdIter);
#endif
            }
        }

}
#endif  // end of ECMD_STRIP_DEBUG


/*
 * The following methods for the ecmdArrayData struct will flatten, unflatten &
 * get the flattened size of the struct.
 */
bool ecmdArrayData::isChipUnitMatch(std::string &i_chipUnitType) {
  /* If either matches, return true */
  if (i_chipUnitType == relatedChipUnit || i_chipUnitType == relatedChipUnitShort) {
    return true;
  }

  return false;
}

uint32_t ecmdArrayData::flatten(uint8_t *o_buf, uint32_t i_len) const{

        uint32_t tmpData32 = 0;
        uint32_t rc     = ECMD_SUCCESS;
        uint8_t *l_ptr = o_buf;


        do {    // Single entry ->

	    if (this->flattenSize() > i_len) {
		// Generate an error for buffer overflow conditions.
		ETRAC("Buffer overflow occured - "
		       "structure size = %d; "
		       "input length = %d",
		       this->flattenSize(), i_len);
		rc = ECMD_DATA_OVERFLOW;
		break;
	    }

	    // Copy non-list data.
	    // arrayName  
	    memcpy(l_ptr, arrayName.c_str(), arrayName.size() + 1);
	    l_ptr += arrayName.size() + 1;
	    i_len -= arrayName.size() + 1;

	    // arrayId
	    tmpData32 = htonl(arrayId);
	    memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
	    l_ptr += sizeof(arrayId);
	    i_len -= sizeof(arrayId);

        // arrayType
        tmpData32 = htonl((uint32_t)arrayType);
        memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
        l_ptr += sizeof(arrayType);
        i_len -= sizeof(arrayType);

	    // readAddressLength
	    tmpData32 = htonl((uint32_t)readAddressLength);
	    memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
	    l_ptr += sizeof(readAddressLength);
	    i_len -= sizeof(readAddressLength);

	    // writeAddressLength
	    tmpData32 = htonl((uint32_t)writeAddressLength);
	    memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
	    l_ptr += sizeof(writeAddressLength);
	    i_len -= sizeof(writeAddressLength);

	    // length - uint64_t with eCMD rel v8.0
	    // first part of uint64_t
	    tmpData32 = htonl( (uint64_t)( length >>32));  // first part
	    memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
	    l_ptr += sizeof(tmpData32);
	    i_len -= sizeof(tmpData32);

	    // 2nd part of uint64_t
	    tmpData32 = htonl(length);
	    memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
	    l_ptr += sizeof(tmpData32);
	    i_len -= sizeof(tmpData32);

	    // width
	    tmpData32 = htonl((uint32_t)width);
	    memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
	    l_ptr += sizeof(width);
	    i_len -= sizeof(width);

        //isChipUnitRelated        
        tmpData32 = htonl( (uint32_t)isChipUnitRelated );
        memcpy( l_ptr, &tmpData32, sizeof(tmpData32) );
        l_ptr += sizeof(tmpData32);
        i_len -= sizeof(tmpData32);

        //relatedChipUnit
        memcpy(l_ptr, relatedChipUnit.c_str(), relatedChipUnit.size() + 1);
        l_ptr += relatedChipUnit.size() + 1;
        i_len -= relatedChipUnit.size() + 1;

        //relatedChipUnitShort
        memcpy(l_ptr, relatedChipUnitShort.c_str(), relatedChipUnitShort.size() + 1);
        l_ptr += relatedChipUnitShort.size() + 1;
        i_len -= relatedChipUnitShort.size() + 1;

	    // clockDomain
	    memcpy(l_ptr, clockDomain.c_str(), clockDomain.size() + 1);
	    l_ptr += clockDomain.size() + 1;
	    i_len -= clockDomain.size() + 1;

	    // clockState
	    tmpData32 = htonl((uint32_t)clockState);
	    memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
	    l_ptr += sizeof(clockState);
	    i_len -= sizeof(clockState);


	    // Do final check
	    if (i_len != 0)
	    {
		ETRAC("Buffer size mismacth occured - "
		       "structure size = %d; "
		       "leftover length = %d",
		       this->flattenSize(), i_len);
		rc = ECMD_DATA_OVERFLOW;
		break;
	    }

        } while (0);    // <- single exit.

        return rc;
}



uint32_t ecmdArrayData::unflatten(const uint8_t *i_buf, uint32_t i_len) {
        uint32_t rc       = ECMD_SUCCESS;

        uint8_t *l_ptr = (uint8_t *) i_buf;
	int l_left = (int) i_len;
        uint32_t tmpData32 = 0;


        do {    // Single entry ->

	    // Unflatten non-list data.
	    // arrayName
	    std::string l_arrayName = (const char *) l_ptr;  //maybe this can be 1 line?
	    arrayName = l_arrayName;
	    l_ptr += l_arrayName.size() + 1;
	    l_left -= l_arrayName.size() + 1;

	    // arrayId
	    memcpy(&arrayId, l_ptr, sizeof(arrayId));
	    arrayId = ntohl(arrayId);
	    l_ptr += sizeof(arrayId);
	    l_left -= sizeof(arrayId);

        // arrayType  
        memcpy(&arrayType, l_ptr, sizeof(arrayType));
        arrayType = (ecmdArrayType_t) ntohl((uint32_t)arrayType);
        l_ptr += sizeof(arrayType);
        l_left -= sizeof(arrayType);

	    // readAddressLength
	    memcpy(&readAddressLength, l_ptr, sizeof(readAddressLength));
	    readAddressLength = (int) ntohl(readAddressLength);
	    l_ptr += sizeof(readAddressLength);
	    l_left -= sizeof(readAddressLength);

	    // writeAddressLength
	    memcpy(&writeAddressLength, l_ptr, sizeof(writeAddressLength));
	    writeAddressLength = (int) ntohl(writeAddressLength);
	    l_ptr += sizeof(writeAddressLength);
	    l_left -= sizeof(writeAddressLength);

	    // length - uint64_t with eCMD rel v8.0
	    // first part of uint64_t
	    uint32_t tmp32_0=0, tmp32_1=0;
	    memcpy(&tmp32_0, l_ptr, sizeof(tmp32_0));
	    tmp32_0 = ntohl(tmp32_0); 
	    l_ptr += sizeof(tmp32_0);
	    l_left -= sizeof(tmp32_0);

	    // second uint32_t of uint64_t
	    memcpy(&tmp32_1, l_ptr, sizeof(tmp32_1));
	    tmp32_1 = ntohl(tmp32_1);
	    l_ptr += sizeof(tmp32_1);
	    l_left -= sizeof(tmp32_1);

	    // put parts of length together
	    length = ( ((uint64_t) tmp32_0) << 32) | ( (uint64_t) tmp32_1);


	    // width
	    memcpy(&width, l_ptr, sizeof(width));
	    width = (int) ntohl(width);
	    l_ptr += sizeof(width);
	    l_left -= sizeof(width);

        //isChipUnitRelated
        memcpy(&tmpData32, l_ptr, sizeof(tmpData32));
        isChipUnitRelated = (bool)ntohl(tmpData32);
        l_ptr += sizeof(tmpData32);
        l_left -= sizeof(tmpData32);

        //relatedChipUnit
        std::string l_relatedChipUnit = (const char *) l_ptr;  //maybe this can be 1 line?
        relatedChipUnit = l_relatedChipUnit;
        l_ptr += l_relatedChipUnit.size() + 1;
        l_left -= l_relatedChipUnit.size() + 1;

        //relatedChipUnitShort
        std::string l_relatedChipUnitShort = (const char *) l_ptr; 
        relatedChipUnitShort = l_relatedChipUnitShort;
        l_ptr += l_relatedChipUnitShort.size() + 1;
        l_left -= l_relatedChipUnitShort.size() + 1;

	    // clockDomain
	    std::string l_clockDomain = (const char *) l_ptr;
	    clockDomain = l_clockDomain;
	    l_ptr += l_clockDomain.size() + 1;
	    l_left -= l_clockDomain.size() + 1;

	    // clockState
	    memcpy(&clockState, l_ptr, sizeof(clockState));
	    clockState = (ecmdClockState_t) ntohl((uint32_t)clockState);
	    l_ptr += sizeof(clockState);
	    l_left -= sizeof(clockState);


	    // Do Final Checks
	    if (l_left < 0)
	    {	
		// Generate an error for buffer overflow conditions.
		ETRAC("Buffer overflow occured - "
		       "structure size = %d; "
		       "input length = %x; "
		       "remainder = %d\n",
		       this->flattenSize(), i_len, l_left);
		rc = ECMD_DATA_OVERFLOW;
		break;
	    }
	    if (l_left > 0)
	    {	
		// Generate an error for buffer underflow conditions.
		ETRAC("Buffer underflow occured - "
		       "structure size = %d; "
		       "input length = %x; "
		       "remainder = %d\n",
		       this->flattenSize(), i_len, l_left);
		rc = ECMD_DATA_UNDERFLOW;
		break;
	    }

        } while (0);    // <- single exit.

        return rc;
}


uint32_t ecmdArrayData::flattenSize() const {

        uint32_t flatSize = 0;


        do {    // Single entry ->

                // Size of non-list member data.
                flatSize += (arrayName.size() + 1
                             + sizeof(arrayId)
                        + sizeof(arrayType)    
			     + sizeof(readAddressLength)
			     + sizeof(writeAddressLength)
                             + sizeof(length)
                             + sizeof(width)
                             + sizeof(uint32_t)          //isChipUnitRelated
                             + relatedChipUnit.size() + 1                    
                             + relatedChipUnitShort.size() + 1                    
                             + clockDomain.size() + 1
                             + sizeof(clockState));

        } while (0);    // <- single exit.

        return flatSize;
}

#ifndef ECMD_STRIP_DEBUG
void  ecmdArrayData::printStruct() const {

        printf("\n\t--- Array Data Structure ---\n");

        // Print non-list data.
        printf("\tArray Name:  %s\n", arrayName.c_str());
        printf("\tArray ID:  0x%08x\n", arrayId);
        printf("\tArray Type:  %d\n", arrayType);
        printf("\tRead Address Length:  0x%08x\n", (uint32_t) readAddressLength);
        printf("\tWrite Address Length:  0x%08x\n", (uint32_t) writeAddressLength);
        printf("\tLength: 0x%08x\n", (uint32_t) length);
        printf("\tWidth: 0x%08x\n", (uint32_t) width);
        printf("\tisChipUnitRelated: 0x%08x\n", (uint32_t) isChipUnitRelated);
        printf("\trelatedChipUnit:  %s\n", relatedChipUnit.c_str());
        printf("\trelatedChipUnitShort:  %s\n", relatedChipUnitShort.c_str());
        printf("\tClock Domain:  %s\n", clockDomain.c_str());
        printf("\tClock State: 0x%08x\n", (uint32_t) clockState);

}
#endif  // end of ECMD_STRIP_DEBUG


/*
 * The following methods for the ecmdIndexEntry struct will flatten, unflatten &
 * get the flattened size of the struct.
 */
uint32_t ecmdIndexEntry::flatten(uint8_t * o_buf, uint32_t i_len) const {

    uint32_t tmpData32 = 0;
    uint32_t dataBufSize = 0;    // temp holder for ecmdDataBuffer size
    uint32_t l_rc = ECMD_SUCCESS;
    int l_left = (int) i_len;	// keeps i_len constant

    uint8_t * l_ptr = o_buf;

    do {    // Single entry ->

        // Check for buffer size mismatch (overflow or underflow)
        if ( this->flattenSize() != i_len ) {
	    ETRAC("Buffer overflow/mismatch occurred - "
                   "structure size = %d; input length = %d",
                   this->flattenSize(), i_len );
	    l_rc = ECMD_DATA_OVERFLOW;
	    break;
        }

	// Copy in index integer
	tmpData32 = htonl((uint32_t)index);
	memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
	l_ptr += sizeof(index);
	l_left -= sizeof(index);

        // Write the size of "buffer", to check against when unflattening
        dataBufSize = buffer.flattenSize();
        tmpData32 = htonl( dataBufSize );
        memcpy( l_ptr, &tmpData32, sizeof(tmpData32) );
        l_ptr += sizeof( dataBufSize );
        l_left -= sizeof( dataBufSize );

        // Write contents of "buffer" into the output buffer
        l_rc = buffer.flatten( l_ptr, dataBufSize );
        if ( l_rc != ECMD_DBUF_SUCCESS ) {
            break;
        } else {
           l_rc = ECMD_SUCCESS;
        }
        l_ptr += dataBufSize;
        l_left -= dataBufSize;

        // Write "rc" into the output buffer
        tmpData32 = htonl( rc );
        memcpy( l_ptr, &tmpData32, sizeof(tmpData32) );
        l_ptr += sizeof( rc );
        l_left -= sizeof( rc );

	// Do Final Checks - If the length has not decremented to 0, something is wrong
	if (l_left < 0)
	{	
	    // Generate an error for buffer overflow conditions.
	    ETRAC("Buffer overflow occured - "
		   "structure size = %d; "
		   "input length = %x; "
		   "remainder = %d\n",
		   this->flattenSize(), i_len, l_left);
	    l_rc = ECMD_DATA_OVERFLOW;
	    break;
	}
	if (l_left > 0)
	{	
	    // Generate an error for buffer underflow conditions.
	    ETRAC("Buffer underflow occured - "
		   "structure size = %d; "
		   "input length = %x; "
		   "remainder = %d\n",
		   this->flattenSize(), i_len, l_left);
	    l_rc = ECMD_DATA_UNDERFLOW;
	    break;
	}


    } while (0);	// <- single exit.

    return l_rc;
}

uint32_t ecmdIndexEntry::unflatten(const uint8_t * i_buf, uint32_t i_len) {

    uint32_t l_tmp = 0;
    uint32_t dataBufSize = 0;
    uint32_t l_rc = ECMD_SUCCESS;
    int l_left = (int) i_len;
    uint8_t * l_ptr = (uint8_t *) i_buf;

    do {	// Single entry ->

        // Get "index" from the input buffer
        memcpy( &l_tmp, l_ptr, sizeof(l_tmp) );
        index = ntohl( l_tmp );
        l_ptr += sizeof( l_tmp );
        l_left -= sizeof( l_tmp );

        // Get the size of "buffer" to pass to unflatten()
        memcpy( &dataBufSize, l_ptr, sizeof(dataBufSize) );
        dataBufSize = ntohl( dataBufSize );
        l_ptr += sizeof( dataBufSize );
        l_left -= sizeof( dataBufSize );

        // Unflatten "buffer" from the input buffer
        l_rc = buffer.unflatten( l_ptr, dataBufSize );
        if ( l_rc != ECMD_DBUF_SUCCESS ) {
            break;
        } else {
            l_rc = ECMD_SUCCESS;
        }
        l_ptr += dataBufSize;
        l_left -= dataBufSize;

        // Get "rc" from the input buffer
        memcpy( &l_tmp, l_ptr, sizeof(l_tmp) );
        rc = ntohl( l_tmp );
        l_ptr += sizeof( l_tmp );
        l_left -= sizeof( l_tmp );

	// Do Final Checks
	if (l_left < 0)
	{	
	    // Generate an error for buffer overflow conditions.
	    ETRAC("Buffer overflow occured - "
		   "structure size = %d; "
		   "input length = %x; "
		   "remainder = %d\n",
		   this->flattenSize(), i_len, l_left);
	    rc = ECMD_DATA_OVERFLOW;
	    break;
	}
	if (l_left > 0)
	{	
	    // Generate an error for buffer underflow conditions.
	    ETRAC("Buffer underflow occured - "
		   "structure size = %d; "
		   "input length = %x; "
		   "remainder = %d\n",
		   this->flattenSize(), i_len, l_left);
	    rc = ECMD_DATA_UNDERFLOW;
	    break;
	}


    } while (0);	// <- single exit.

    return l_rc;
}

uint32_t ecmdIndexEntry::flattenSize() const {

    return (sizeof(index)		// Space for member "index"
	    + sizeof(uint32_t)		// Size of member "buffer"
	    + buffer.flattenSize()	// Space for "buffer" flattened
	    + sizeof(rc));		// Space for member "rc"
}  


#ifndef ECMD_STRIP_DEBUG
void ecmdIndexEntry::printStruct() const {

    uint32_t tmpData = 0;
    std::string tmpString;

    printf("\neCMD Index Entry:\n");

    printf("Index: %d. ", index );

    tmpData = buffer.getBitLength();
    tmpString = buffer.genHexLeftStr(0, tmpData);
    printf("Buffer Data: %s. ", tmpString.c_str() );

    printf("RC: 0x%x\n", rc );
}
#endif


/*
 * The following methods for the ecmdTraceArrayData struct will flatten, 
 * unflatten & get the flattened size of the struct.
 */
bool ecmdTraceArrayData::isChipUnitMatch(std::string &i_chipUnitType) {
  /* If either matches, return true */
  if (i_chipUnitType == relatedChipUnit || i_chipUnitType == relatedChipUnitShort) {
    return true;
  }

  return false;
}

uint32_t ecmdTraceArrayData::flatten(uint8_t *o_buf, uint32_t i_len) const 
{
        uint32_t tmpData32 = 0;
        uint32_t strLen = 0;
        uint32_t l_rc = ECMD_SUCCESS;

        int l_len = (int)i_len;  // use a local copy to decrement
	uint8_t *l_ptr8 = o_buf;

        do      // Single entry ->
        {
            // Check for buffer overflow conditions.
            if (this->flattenSize() > i_len) 
            {
                // Generate an error for buffer overflow conditions.
                ETRAC("ECMD: Buffer overflow occurred - "
                       "structure size = %d; input length = %d",
                       this->flattenSize(), i_len);
                l_rc = ECMD_DATA_OVERFLOW;
                break;
            }

            // Flatten and store the data in the ouput buffer

            // traceArrayName
            strLen = traceArrayName.size();
            memcpy( l_ptr8, traceArrayName.c_str(), strLen + 1 );
            l_ptr8 += strLen + 1;
            l_len -= strLen + 1;

            // traceArrayId
            tmpData32 = htonl( traceArrayId );
            memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
            l_ptr8 += sizeof(traceArrayId);
            l_len -= sizeof(traceArrayId);

            // length
            tmpData32 = htonl( (uint32_t)length );
            memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
            l_ptr8 += sizeof(length);
            l_len -= sizeof(length);

            // width
            tmpData32 = htonl( (uint32_t)width );
            memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
            l_ptr8 += sizeof(width);
            l_len -= sizeof(width);

            //isChipUnitRelated
            tmpData32 = htonl( (uint32_t)isChipUnitRelated );
            memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
            l_ptr8 += sizeof(tmpData32);
            l_len -= sizeof(tmpData32);

            //relatedChipUnit
            memcpy(l_ptr8, relatedChipUnit.c_str(), relatedChipUnit.size() + 1);
            l_ptr8 += relatedChipUnit.size() + 1;
            l_len -= relatedChipUnit.size() + 1;

            //relatedChipUnitShort
            memcpy(l_ptr8, relatedChipUnitShort.c_str(), relatedChipUnitShort.size() + 1);
            l_ptr8 += relatedChipUnitShort.size() + 1;
            l_len -= relatedChipUnitShort.size() + 1;

            // clockDomain
            strLen = clockDomain.size();
            memcpy( l_ptr8, clockDomain.c_str(), strLen + 1 );
            l_ptr8 += strLen + 1;
            l_len -= strLen + 1;

	    // clockState
	    tmpData32 = htonl( (uint32_t)clockState );
	    memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
	    l_ptr8 += sizeof(clockState);
	    l_len -= sizeof(clockState);

            // Final check: if the length isn't 0, something went wrong
            if (l_len < 0)
            {	
               // Generate an error for buffer overflow conditions.
               ETRAC("ECMD: Buffer overflow occurred - "
                      "struct size= %d; "
                      "input length= %x; remainder= %d\n",
                      this->flattenSize(), i_len, l_len);
               l_rc = ECMD_DATA_OVERFLOW;
               break;
            }

            if (l_len > 0)
            {	
               // Generate an error for buffer underflow conditions.
               ETRAC("ECMD: Buffer underflow occurred - "
                      "struct size= %d; "
                      "input length= %x; remainder= %d\n",
                      this->flattenSize(), i_len, l_len);
               l_rc = ECMD_DATA_UNDERFLOW;
               break;
            }

        } while (false);   // <- single exit

        return l_rc;
}


uint32_t ecmdTraceArrayData::unflatten(const uint8_t *i_buf, uint32_t i_len) 
{
        uint32_t l_rc = ECMD_SUCCESS;
        uint32_t tmpData32 = 0;

        const uint8_t *l_ptr8 = i_buf;
	int l_len = (int)i_len;

        do    // Single entry ->
        {
            // Unflatten data from the input buffer

	    // traceArrayName
	    std::string l_trace_array_name = (const char *)l_ptr8;
	    traceArrayName = l_trace_array_name;
	    l_ptr8 += l_trace_array_name.size() + 1;
	    l_len -= l_trace_array_name.size() + 1;

            // traceArrayId
            memcpy( &traceArrayId, l_ptr8, sizeof(traceArrayId) );
            traceArrayId = ntohl( traceArrayId );
            l_ptr8 += sizeof(traceArrayId);
            l_len -= sizeof(traceArrayId);

	    // length
	    memcpy( &tmpData32, l_ptr8, sizeof(tmpData32) );
	    length = (int)ntohl( tmpData32 );
	    l_ptr8 += sizeof(length);
	    l_len -= sizeof(length);

	    // width
	    memcpy( &tmpData32, l_ptr8, sizeof(tmpData32) );
	    width = (int)ntohl( tmpData32 );
	    l_ptr8 += sizeof(width);
	    l_len -= sizeof(width);

        //isChipUnitRelated
        memcpy(&tmpData32, l_ptr8, sizeof(tmpData32));
        isChipUnitRelated = (bool)ntohl(tmpData32);
        l_ptr8 += sizeof(tmpData32);
        l_len -= sizeof(tmpData32);

        //relatedChipUnit
        std::string l_relatedChipUnit = (const char *) l_ptr8;  //maybe this can be 1 line?
        relatedChipUnit = l_relatedChipUnit;
        l_ptr8 += l_relatedChipUnit.size() + 1;
        l_len -= l_relatedChipUnit.size() + 1;

        //relatedChipUnitShort
        std::string l_relatedChipUnitShort = (const char *) l_ptr8; 
        relatedChipUnitShort = l_relatedChipUnitShort;
        l_ptr8 += l_relatedChipUnitShort.size() + 1;
        l_len -= l_relatedChipUnitShort.size() + 1;

	    // clockDomain
            std::string l_clock_domain = (const char *)l_ptr8;
            clockDomain = l_clock_domain;
            l_ptr8 += l_clock_domain.size() + 1;
            l_len -= l_clock_domain.size() + 1;

	    // clockState
	    memcpy( &clockState, l_ptr8, sizeof(clockState) );
	    clockState = (ecmdClockState_t)ntohl( (uint32_t)clockState );
	    l_ptr8 += sizeof(clockState);
	    l_len -= sizeof(clockState);

            // Final check: if the length isn't 0, something went wrong
            if (l_len < 0)
            {	
               // Generate an error for buffer overflow conditions.
               ETRAC("ECMD: Buffer overflow occurred - "
                      "struct size= %d; "
                      "input length= %x; remainder= %d\n",
                      this->flattenSize(), i_len, l_len);
               l_rc = ECMD_DATA_OVERFLOW;
               break;
            }

            if (l_len > 0)
            {	
               // Generate an error for buffer underflow conditions.
               ETRAC("ECMD: Buffer underflow occurred - "
                      "struct size= %d; "
                      "input length= %x; remainder= %d\n",
                      this->flattenSize(), i_len, l_len);
               l_rc = ECMD_DATA_UNDERFLOW;
               break;
            }

        } while (false);   // <- single exit

        return l_rc;
}

uint32_t ecmdTraceArrayData::flattenSize() const 
{
        uint32_t flatSize = 0;

        // size needed to store the data structure
        flatSize = traceArrayName.size() + 1
                   + sizeof( traceArrayId )
                   + sizeof( length )
                   + sizeof( width )
                   + sizeof( uint32_t ) // isChipUnitRelated in uint32_t
                   + relatedChipUnit.size() + 1                          
                   + relatedChipUnitShort.size() + 1                          
                   + clockDomain.size() + 1
                   + sizeof( clockState );

        return flatSize;
}

#ifndef ECMD_STRIP_DEBUG
void  ecmdTraceArrayData::printStruct() const {

        printf("\n\t--- Trace Array Data Structure ---\n");

        printf("\tTrace Array Name: %s\n", traceArrayName.c_str() );
        printf("\tTrace Array ID: 0x%08x\n", traceArrayId );
        printf("\tLength: %d\n", length );
        printf("\tWidth: %d\n", width );
        printf("\tisChipUnitRelated: 0x%08x\n", (uint32_t) isChipUnitRelated);
        printf("\trelatedChipUnit:  %s\n", relatedChipUnit.c_str());
        printf("\trelatedChipUnitShort:  %s\n", relatedChipUnitShort.c_str());
        printf("\tClock Domain: %s\n", clockDomain.c_str() );
        printf("\tClock State: 0x%08x\n", (uint32_t)clockState );

}
#endif  // end of ECMD_STRIP_DEBUG

/*
 * The following methods for the ecmdFastArrayData struct will flatten, 
 * unflatten & get the flattened size of the struct.
 */
bool ecmdFastArrayData::isChipUnitMatch(std::string &i_chipUnitType) {
  /* If either matches, return true */
  if (i_chipUnitType == relatedChipUnit || i_chipUnitType == relatedChipUnitShort) {
    return true;
  }

  return false;
}

uint32_t ecmdFastArrayData::flatten(uint8_t *o_buf, uint32_t i_len) const
{
        uint32_t tmpData32 = 0;
        uint32_t strLen = 0;
        uint32_t l_rc = ECMD_SUCCESS;

        int l_len = (int)i_len;  // use a local copy to decrement
	uint8_t *l_ptr8 = o_buf;

        do      // Single entry ->
        {
            // Check for buffer overflow conditions.
            if (this->flattenSize() > i_len) 
            {
                // Generate an error for buffer overflow conditions.
                ETRAC("ECMD: Buffer overflow occurred - "
                       "structure size = %d; input length = %d",
                       this->flattenSize(), i_len);
                l_rc = ECMD_DATA_OVERFLOW;
                break;
            }

            // Flatten and store the data in the ouput buffer

            // fastArrayName
            strLen = fastArrayName.size();
            memcpy( l_ptr8, fastArrayName.c_str(), strLen + 1 );
            l_ptr8 += strLen + 1;
            l_len -= strLen + 1;

            // fastArrayId
            tmpData32 = htonl( fastArrayId );
            memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
            l_ptr8 += sizeof(fastArrayId);
            l_len -= sizeof(fastArrayId);

            // length
            tmpData32 = htonl( (uint32_t)length );
            memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
            l_ptr8 += sizeof(length);
            l_len -= sizeof(length);

            // width
            tmpData32 = htonl( (uint32_t)width );
            memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
            l_ptr8 += sizeof(width);
            l_len -= sizeof(width);

            //isChipUnitRelated
            tmpData32 = htonl( (uint32_t)isChipUnitRelated );
            memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
            l_ptr8 += sizeof(tmpData32);
            l_len -= sizeof(tmpData32);

            //relatedChipUnit
            memcpy(l_ptr8, relatedChipUnit.c_str(), relatedChipUnit.size() + 1);
            l_ptr8 += relatedChipUnit.size() + 1;
            l_len -= relatedChipUnit.size() + 1;

            //relatedChipUnitShort
            memcpy(l_ptr8, relatedChipUnitShort.c_str(), relatedChipUnitShort.size() + 1);
            l_ptr8 += relatedChipUnitShort.size() + 1;
            l_len -= relatedChipUnitShort.size() + 1;

            // clockDomain
            strLen = clockDomain.size();
            memcpy( l_ptr8, clockDomain.c_str(), strLen + 1 );
            l_ptr8 += strLen + 1;
            l_len -= strLen + 1;

	    // clockState
	    tmpData32 = htonl( (uint32_t)clockState );
	    memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
	    l_ptr8 += sizeof(clockState);
	    l_len -= sizeof(clockState);

            // Final check: if the length isn't 0, something went wrong
            if (l_len < 0)
            {	
               // Generate an error for buffer overflow conditions.
               ETRAC("ECMD: Buffer overflow occurred - "
                      "struct size= %d; "
                      "input length= %x; remainder= %d\n",
                      this->flattenSize(), i_len, l_len);
               l_rc = ECMD_DATA_OVERFLOW;
               break;
            }

            if (l_len > 0)
            {	
               // Generate an error for buffer underflow conditions.
               ETRAC("ECMD: Buffer underflow occurred - "
                      "struct size= %d; "
                      "input length= %x; remainder= %d\n",
                      this->flattenSize(), i_len, l_len);
               l_rc = ECMD_DATA_UNDERFLOW;
               break;
            }

        } while (false);   // <- single exit

        return l_rc;
}


uint32_t ecmdFastArrayData::unflatten(const uint8_t *i_buf, uint32_t i_len) 
{
        uint32_t l_rc = ECMD_SUCCESS;
        uint32_t tmpData32 = 0;

        const uint8_t *l_ptr8 = i_buf;
	int l_len = (int)i_len;

        do    // Single entry ->
        {
            // Unflatten data from the input buffer

	    // fastArrayName
	    std::string l_trace_array_name = (const char *)l_ptr8;
	    fastArrayName = l_trace_array_name;
	    l_ptr8 += l_trace_array_name.size() + 1;
	    l_len -= l_trace_array_name.size() + 1;

            // fastArrayId
            memcpy( &fastArrayId, l_ptr8, sizeof(fastArrayId) );
            fastArrayId = ntohl( fastArrayId );
            l_ptr8 += sizeof(fastArrayId);
            l_len -= sizeof(fastArrayId);

	    // length
	    memcpy( &tmpData32, l_ptr8, sizeof(tmpData32) );
	    length = (int)ntohl( tmpData32 );
	    l_ptr8 += sizeof(length);
	    l_len -= sizeof(length);

	    // width
	    memcpy( &tmpData32, l_ptr8, sizeof(tmpData32) );
	    width = (int)ntohl( tmpData32 );
	    l_ptr8 += sizeof(width);
	    l_len -= sizeof(width);

        //isChipUnitRelated
        memcpy(&tmpData32, l_ptr8, sizeof(tmpData32));
        isChipUnitRelated = (bool)ntohl(tmpData32);
        l_ptr8 += sizeof(tmpData32);
        l_len -= sizeof(tmpData32);

        //relatedChipUnit
        std::string l_relatedChipUnit = (const char *) l_ptr8;  //maybe this can be 1 line?
        relatedChipUnit = l_relatedChipUnit;
        l_ptr8 += l_relatedChipUnit.size() + 1;
        l_len -= l_relatedChipUnit.size() + 1;

        //relatedChipUnitShort
        std::string l_relatedChipUnitShort = (const char *) l_ptr8; 
        relatedChipUnitShort = l_relatedChipUnitShort;
        l_ptr8 += l_relatedChipUnitShort.size() + 1;
        l_len -= l_relatedChipUnitShort.size() + 1;

	    // clockDomain
            std::string l_clock_domain = (const char *)l_ptr8;
            clockDomain = l_clock_domain;
            l_ptr8 += l_clock_domain.size() + 1;
            l_len -= l_clock_domain.size() + 1;

	    // clockState
	    memcpy( &clockState, l_ptr8, sizeof(clockState) );
	    clockState = (ecmdClockState_t)ntohl( (uint32_t)clockState );
	    l_ptr8 += sizeof(clockState);
	    l_len -= sizeof(clockState);

            // Final check: if the length isn't 0, something went wrong
            if (l_len < 0)
            {	
               // Generate an error for buffer overflow conditions.
               ETRAC("ECMD: Buffer overflow occurred - "
                      "struct size= %d; "
                      "input length= %x; remainder= %d\n",
                      this->flattenSize(), i_len, l_len);
               l_rc = ECMD_DATA_OVERFLOW;
               break;
            }

            if (l_len > 0)
            {	
               // Generate an error for buffer underflow conditions.
               ETRAC("ECMD: Buffer underflow occurred - "
                      "struct size= %d; "
                      "input length= %x; remainder= %d\n",
                      this->flattenSize(), i_len, l_len);
               l_rc = ECMD_DATA_UNDERFLOW;
               break;
            }

        } while (false);   // <- single exit

        return l_rc;
}

uint32_t ecmdFastArrayData::flattenSize() const
{
        uint32_t flatSize = 0;

        // size needed to store the data structure
        flatSize = fastArrayName.size() + 1
                   + sizeof( fastArrayId )
                   + sizeof( length )
                   + sizeof( width )
                   + sizeof( uint32_t ) // isChipUnitRelated in uint32_t
                   + relatedChipUnit.size() + 1                          
                   + relatedChipUnitShort.size() + 1                          
                   + clockDomain.size() + 1
                   + sizeof( clockState );

        return flatSize;
}

#ifndef ECMD_STRIP_DEBUG
void  ecmdFastArrayData::printStruct() const {

        printf("\n\t--- Fast Array Data Structure ---\n");

        printf("\tFast Array Name: %s\n", fastArrayName.c_str() );
        printf("\tFast Array ID: 0x%08x\n", fastArrayId );
        printf("\tLength: %d\n", length );
        printf("\tWidth: %d\n", width );
        printf("\tisChipUnitRelated: 0x%08x\n", (uint32_t) isChipUnitRelated);
        printf("\trelatedChipUnit:  %s\n", relatedChipUnit.c_str());
        printf("\trelatedChipUnitShort:  %s\n", relatedChipUnitShort.c_str());
        printf("\tClock Domain: %s\n", clockDomain.c_str() );
        printf("\tClock State: 0x%08x\n", (uint32_t)clockState );

}
#endif  // end of ECMD_STRIP_DEBUG


// @05 start
/*
 * The following method will check if the chipUnitType matches with the list of relatedChipUnit's and relatedChipUnitShort's
 *
 */
bool ecmdScomData::isChipUnitMatch(std::string &i_chipUnitType) {

  /* If either matches, return true */
  // walk through the list to see if it matches with relatedChipUnit
  std::list<std::string>::iterator relatedChipUnitIter;
  for ( relatedChipUnitIter = relatedChipUnit.begin(); relatedChipUnitIter != relatedChipUnit.end(); relatedChipUnitIter++) {
    if (i_chipUnitType == *relatedChipUnitIter) {
      return true;
    }
  }
  std::list<std::string>::iterator relatedChipUnitShortIter;
  for ( relatedChipUnitShortIter = relatedChipUnitShort.begin(); relatedChipUnitShortIter != relatedChipUnitShort.end(); relatedChipUnitShortIter++) {
    if (i_chipUnitType == *relatedChipUnitShortIter) {
      return true;
    }
  }
  return false;

}

// @05 start
/*
 * The following methods for the ecmdScomData struct will flatten, unflatten &
 * get the flattened size of the struct.
 */
//FIXME these are not implemented
#if 0
uint32_t ecmdScomData::flatten(uint8_t *o_buf, uint32_t i_len) const
{
        uint32_t listSize  = 0;
        uint32_t tmpData32 = 0;
        uint32_t strLen = 0;
        uint32_t l_rc = ECMD_SUCCESS;

        int l_len = (int)i_len;   // use a local copy to decrement
	uint8_t *l_ptr8 = o_buf;  // pointer to the output buffer

        do      // Single entry ->
        {
            // Check for buffer overflow conditions.
            if (this->flattenSize() > i_len) 
            {
                // Generate an error for buffer overflow conditions.
                ETRAC("ECMD: Buffer overflow occurred - "
                       "structure size = %d; input length = %d",
                       this->flattenSize(), i_len);
                l_rc = ECMD_DATA_OVERFLOW;
                break;
            }

            // Flatten and store each data member in the ouput buffer

            // "address" (uint64_t) : first part
            tmpData32 = htonl( address >> 32 );
            memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
            l_ptr8 += sizeof(tmpData32);
            l_len -= sizeof(tmpData32);

            // "address" (uint64_t) : second part
	    tmpData32 = htonl(address);
	    memcpy(l_ptr8, &tmpData32, sizeof(tmpData32));
	    l_ptr8 += sizeof(tmpData32);
	    l_len -= sizeof(tmpData32); // @nk typo fix

            //"length"  (uint32_t)
            tmpData32 = htonl( length );
            memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
            l_ptr8 += sizeof(length);
            l_len -= sizeof(length);

            //isChipUnitRelated     
            tmpData32 = htonl( (uint32_t)isChipUnitRelated );
            memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
            l_ptr8 += sizeof(tmpData32);
            l_len -= sizeof(tmpData32);

            // Figure out how many relatedChipUnit strings are in the list for future unflattening
            listSize = relatedChipUnit.size();
            tmpData32 = htonl(listSize);
            memcpy(l_ptr8, &tmpData32, sizeof(tmpData32));
            l_ptr8 += sizeof(listSize);
            i_len -= sizeof(listSize);

            //relatedChipUnit
            std::list<std::string>::iterator relatedChipUnitIter;
            for (relatedChipUnitIter = relatedChipUnit.begin(); relatedChipUnitIter != relatedChipUnit.end(); relatedChipUnitIter++) {
              memcpy(l_ptr8, relatedChipUnitIter->c_str(), relatedChipUnitIter->size() + 1);
              l_ptr8 += relatedChipUnitIter->size() + 1;
              l_len -= relatedChipUnitIter->size() + 1;
            }

            // Figure out how many relatedChipUnitShort strings are in the list for future unflattening
            listSize = relatedChipUnitShort.size();
            tmpData32 = htonl(listSize);
            memcpy(l_ptr8, &tmpData32, sizeof(tmpData32));
            l_ptr8 += sizeof(listSize);
            i_len -= sizeof(listSize);

            //relatedChipUnitShort
            std::list<std::string>::iterator relatedChipUnitShortIter;
            for (relatedChipUnitShortIter = relatedChipUnitShort.begin(); relatedChipUnitShortIter != relatedChipUnitShort.end(); relatedChipUnitShortIter++) {
              memcpy(l_ptr8, relatedChipUnitShortIter->c_str(), relatedChipUnitShortIter->size() + 1);
              l_ptr8 += relatedChipUnitShortIter->size() + 1;
              l_len -= relatedChipUnitShortIter->size() + 1;
            }
            
            // "endianMode" (ecmdEndianMode_t, stored as uint32_t)
            tmpData32 = htonl( (uint32_t)endianMode );
            memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
            l_ptr8 += sizeof(tmpData32);
            l_len -= sizeof(tmpData32);

            // "clockDomain" (std::string)
            strLen = clockDomain.size();
            memcpy( l_ptr8, clockDomain.c_str(), strLen + 1 );
            l_ptr8 += strLen + 1;
            l_len -= strLen + 1;

            // "clockState" (ecmdClockState_t, store in uint32_t)
	    tmpData32 = htonl( (uint32_t)clockState );
	    memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
	    l_ptr8 += sizeof(tmpData32);
	    l_len -= sizeof(tmpData32);

            // Final check: if the length isn't 0, something went wrong
            if (l_len < 0)
            {	
               // Generate an error for buffer overflow conditions.
               ETRAC("ECMD: Buffer overflow occurred - "
                      "struct size= %d; "
                      "input length= %d; remainder= %d\n",
                      this->flattenSize(), i_len, l_len);
               l_rc = ECMD_DATA_OVERFLOW;
               break;
            }

            if (l_len > 0)
            {	
               // Generate an error for buffer underflow conditions.
               ETRAC("ECMD: Buffer underflow occurred - "
                      "struct size= %d; "
                      "input length= %d; remainder= %d\n",
                      this->flattenSize(), i_len, l_len);
               l_rc = ECMD_DATA_UNDERFLOW;
               break;
            }

        } while (false);   // <- single exit

        return l_rc;
}


uint32_t ecmdScomData::unflatten(const uint8_t *i_buf, uint32_t i_len)
{
        uint32_t l_rc = ECMD_SUCCESS;
        uint32_t listSize  = 0;
        uint32_t tmpData32 = 0;

	int l_len = (int)i_len;         // use a local copy to decrement
        const uint8_t *l_ptr8 = i_buf;  // pointer to the input buffer

        do    // Single entry ->
        {
            // Unflatten each data member from the input buffer

            // "address" (uint64_t)
	    //first part(uint32_t)
            uint32_t tmp32_0 = 0, tmp32_1 = 0;
            memcpy( &tmp32_0, l_ptr8, sizeof(tmp32_0) );
            tmp32_0 = ntohl( tmp32_0);
            l_ptr8 += sizeof(tmp32_0);
            l_len -= sizeof(tmp32_0);
 
	    //second part(uint32_t)
            memcpy( &tmp32_1, l_ptr8, sizeof(tmp32_1) );
            tmp32_1 = ntohl( tmp32_1);
            l_ptr8 += sizeof(tmp32_1);
            l_len -= sizeof(tmp32_1);
            //combine both uint32_t to uint64_t
	    address = ( ((uint64_t) tmp32_0) << 32) | ( (uint64_t) tmp32_1);

            //"length" (uint32_t)
            memcpy( &length, l_ptr8, sizeof(length) );
            length = ntohl( length );
            l_ptr8 += sizeof(length);
            l_len -= sizeof(length);

            //isChipUnitRelated
            memcpy(&tmpData32, l_ptr8, sizeof(tmpData32));
            isChipUnitRelated = (bool)ntohl(tmpData32);
            l_ptr8 += sizeof(tmpData32);
            l_len -= sizeof(tmpData32);

            // Get the size of the list to create
            memcpy(&listSize, l_ptr8, sizeof(listSize));
            listSize = ntohl(listSize);
            l_ptr8 += sizeof(listSize);
            i_len -= sizeof(listSize);
        
            //relatedChipUnit
            while (relatedChipUnit.size() < listSize) {
              std::string l_relatedChipUnit = (const char *) l_ptr8;  //maybe this can be 1 line?
              relatedChipUnit.push_back(l_relatedChipUnit);
              l_ptr8 += l_relatedChipUnit.size() + 1;
              l_len -= l_relatedChipUnit.size() + 1;
            }

            // Get the size of the list to create
            memcpy(&listSize, l_ptr8, sizeof(listSize));
            listSize = ntohl(listSize);
            l_ptr8 += sizeof(listSize);
            i_len -= sizeof(listSize);
        
            //relatedChipUnitShort
            while (relatedChipUnitShort.size() < listSize) {
              std::string l_relatedChipUnitShort = (const char *) l_ptr8; 
              relatedChipUnitShort.push_back(l_relatedChipUnitShort);
              l_ptr8 += l_relatedChipUnitShort.size() + 1;
              l_len -= l_relatedChipUnitShort.size() + 1;
            }
            
            // "endianMode" (ecmdEndianMode_t, stored as uint32_t)
            memcpy( &tmpData32, l_ptr8, sizeof(tmpData32) );
            endianMode = (ecmdEndianMode_t) ntohl( tmpData32 );
            l_ptr8 += sizeof(tmpData32);
            l_len -= sizeof(tmpData32);

            // "clockDomain" (std::string)
            std::string l_clock_domain = (const char *)l_ptr8;
            clockDomain = l_clock_domain;
            l_ptr8 += l_clock_domain.size() + 1;
            l_len -= l_clock_domain.size() + 1;

            // "clockState" (ecmdClockState_t, stored as uint32_t)
	    memcpy( &tmpData32, l_ptr8, sizeof(tmpData32) );
	    clockState = (ecmdClockState_t)ntohl( tmpData32 );
	    l_ptr8 += sizeof(tmpData32);
	    l_len -= sizeof(tmpData32);

            // Final check: if the length isn't 0, something went wrong
            if (l_len < 0)
            {	
               // Generate an error for buffer overflow conditions.
               ETRAC("ECMD: Buffer overflow occurred - "
                      "struct size= %d; "
                      "input length= %d; remainder= %d\n",
                      this->flattenSize(), i_len, l_len);
               l_rc = ECMD_DATA_OVERFLOW;
               break;
            }

            if (l_len > 0)
            {	
               // Generate an error for buffer underflow conditions.
               ETRAC("ECMD: Buffer underflow occurred - "
                      "struct size= %d; "
                      "input length= %d; remainder= %d\n",
                      this->flattenSize(), i_len, l_len);
               l_rc = ECMD_DATA_UNDERFLOW;
               break;
            }

        } while (false);   // <- single exit

        return l_rc;
}


uint32_t ecmdScomData::flattenSize() const
{
        uint32_t flatSize = 0;

        // Calculate the size needed to store the flattened struct
        flatSize = sizeof(address)
                   + sizeof(length)                                              
                   + sizeof(uint32_t)   // isChipUnitRelated stored as uint32_t 
                   + relatedChipUnit.size() + 1                                
                   + relatedChipUnitShort.size() + 1
                   + sizeof(uint32_t)  // ecmdEndianMode_t stored as uint32_t
                   + clockDomain.size() + 1
                   + sizeof(uint32_t);  // ecmdClockState stored as uint32_t

        return flatSize;
}


#ifndef ECMD_STRIP_DEBUG
void  ecmdScomData::printStruct() const
{

        printf("\n\t--- Scom Data Structure ---\n");

#ifdef _LP64
        printf("\tAddress: 0x%016lx\n", address );
#else
        printf("\tAddress: 0x%016llx\n", address );
#endif
        printf("\tLength: %d\n", length );
        printf("\tisChipUnitRelated: 0x%08x\n", (uint32_t) isChipUnitRelated);
        printf("\trelatedChipUnit:");
        std::list<std::string>::iterator relatedChipUnitIter;
        for (relatedChipUnitIter = relatedChipUnit.begin(); relatedChipUnitIter != relatedChipUnit.end(); relatedChipUnitIter++) {
          printf(" %s", relatedChipUnitIter->c_str());
        }
        printf("\n");
        printf("\trelatedChipUnitShort:");
        std::list<std::string>::iterator relatedChipUnitShortIter;
        for (relatedChipUnitShortIter = relatedChipUnitShort.begin(); relatedChipUnitShortIter != relatedChipUnitShort.end(); relatedChipUnitShortIter++) {
          printf(" %s", relatedChipUnitShortIter->c_str());
        }
        printf("\n");
        printf("\tEndian Mode: 0x%x\n", (uint32_t)endianMode );
        printf("\tClock Domain: %s\n", clockDomain.c_str() );
        printf("\tClock State: 0x%x\n", (uint32_t)clockState );
        
}
#endif  // end of ECMD_STRIP_DEBUG
#endif // end of #if 0
// @05 end

// @06 start
/*
 * The following methods for the ecmdLatchData struct will flatten, unflatten &
 * get the flattened size of the struct.
 */
bool ecmdLatchData::isChipUnitMatch(std::string &i_chipUnitType) {
  /* If either matches, return true */
  if (i_chipUnitType == relatedChipUnit || i_chipUnitType == relatedChipUnitShort) {
    return true;
  }

  return false;
}

uint32_t ecmdLatchData::flatten(uint8_t *o_buf, uint32_t i_len) const
{
        uint32_t tmpData32 = 0;
        uint32_t strLen = 0;
        uint32_t l_rc = ECMD_SUCCESS;

        int l_len = (int)i_len;   // use a local copy to decrement
	uint8_t *l_ptr8 = o_buf;  // pointer to the output buffer

        do      // Single entry ->
        {
            // Check for buffer overflow conditions.
            if (this->flattenSize() > i_len) 
            {
                // Generate an error for buffer overflow conditions.
                ETRAC("ECMD: Buffer overflow occurred - "
                       "structure size = %d; input length = %d",
                       this->flattenSize(), i_len);
                l_rc = ECMD_DATA_OVERFLOW;
                break;
            }

            // Flatten and store each data member in the ouput buffer

            // "latchName" (std::string)
            strLen = latchName.size();
            memcpy( l_ptr8, latchName.c_str(), strLen + 1 );
            l_ptr8 += strLen + 1;
            l_len -= strLen + 1;

            // "ringName" (std::string)
            strLen = ringName.size();
            memcpy( l_ptr8, ringName.c_str(), strLen + 1 );
            l_ptr8 += strLen + 1;
            l_len -= strLen + 1;

            // "latchId" (uint32_t)
            tmpData32 = htonl( latchId );
            memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
            l_ptr8 += sizeof(latchId);
            l_len -= sizeof(latchId);

            // "ringId" (uint32_t)
            tmpData32 = htonl( ringId );
            memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
            l_ptr8 += sizeof(ringId);
            l_len -= sizeof(ringId);

            // "bitLength" (int)
            tmpData32 = htonl( (uint32_t)bitLength );
            memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
            l_ptr8 += sizeof(bitLength);
            l_len -= sizeof(bitLength);

            //isChipUnitRelated
            tmpData32 = htonl( (uint32_t)isChipUnitRelated );
            memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
            l_ptr8 += sizeof(tmpData32);
            l_len -= sizeof(tmpData32);

            //relatedChipUnit
            memcpy(l_ptr8, relatedChipUnit.c_str(), relatedChipUnit.size() + 1);
            l_ptr8 += relatedChipUnit.size() + 1;
            l_len -= relatedChipUnit.size() + 1;

            //relatedChipUnitShort
            memcpy(l_ptr8, relatedChipUnitShort.c_str(), relatedChipUnitShort.size() + 1);
            l_ptr8 += relatedChipUnitShort.size() + 1;
            l_len -= relatedChipUnitShort.size() + 1;

            // "clockDomain" (std::string)
            strLen = clockDomain.size();
            memcpy( l_ptr8, clockDomain.c_str(), strLen + 1 );
            l_ptr8 += strLen + 1;
            l_len -= strLen + 1;

            // "clockState" (ecmdClockState_t, store in uint32_t)
	    tmpData32 = htonl( (uint32_t)clockState );
	    memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
	    l_ptr8 += sizeof(tmpData32);
	    l_len -= sizeof(tmpData32);

            // Final check: if the length isn't 0, something went wrong
            if (l_len < 0)
            {	
               // Generate an error for buffer overflow conditions.
               ETRAC("ECMD: Buffer overflow occurred - "
                      "struct size= %d; "
                      "input length= %d; remainder= %d\n",
                      this->flattenSize(), i_len, l_len);
               l_rc = ECMD_DATA_OVERFLOW;
               break;
            }

            if (l_len > 0)
            {	
               // Generate an error for buffer underflow conditions.
               ETRAC("ECMD: Buffer underflow occurred - "
                      "struct size= %d; "
                      "input length= %d; remainder= %d\n",
                      this->flattenSize(), i_len, l_len);
               l_rc = ECMD_DATA_UNDERFLOW;
               break;
            }

        } while (false);   // <- single exit

        return l_rc;
}

uint32_t ecmdLatchData::unflatten(const uint8_t *i_buf, uint32_t i_len)
{
        uint32_t l_rc = ECMD_SUCCESS;
        uint32_t tmpData32 = 0;

	int l_len = (int)i_len;         // use a local copy to decrement
        const uint8_t *l_ptr8 = i_buf;  // pointer to the input buffer

        do    // Single entry ->
        {
            // Unflatten each data member from the input buffer

            // "latchName" (std::string)
            std::string l_latch_name = (const char *)l_ptr8;
            latchName = l_latch_name;
            l_ptr8 += l_latch_name.size() + 1;
            l_len -= l_latch_name.size() + 1;

            // "ringName" (std::string)
            std::string l_ring_name = (const char *)l_ptr8;
            ringName = l_ring_name;
            l_ptr8 += l_ring_name.size() + 1;
            l_len -= l_ring_name.size() + 1;

            // "latchId" (uint32_t)
            memcpy( &latchId, l_ptr8, sizeof(latchId) );
            latchId = ntohl( latchId );
            l_ptr8 += sizeof(latchId);
            l_len -= sizeof(latchId);

            // "ringId" (uint32_t)
            memcpy( &ringId, l_ptr8, sizeof(ringId) );
            ringId = ntohl( ringId );
            l_ptr8 += sizeof(ringId);
            l_len -= sizeof(ringId);
 
            // "bitLength" (int)
	    memcpy( &tmpData32, l_ptr8, sizeof(tmpData32) );
	    bitLength = (int)ntohl( tmpData32 );
	    l_ptr8 += sizeof(bitLength);
	    l_len -= sizeof(bitLength);

            //isChipUnitRelated
            memcpy(&tmpData32, l_ptr8, sizeof(tmpData32));
            isChipUnitRelated = (bool)ntohl(tmpData32);
            l_ptr8 += sizeof(tmpData32);
            l_len -= sizeof(tmpData32);

            //relatedChipUnit
            std::string l_relatedChipUnit = (const char *) l_ptr8;
            relatedChipUnit = l_relatedChipUnit;
            l_ptr8 += l_relatedChipUnit.size() + 1;
            l_len -= l_relatedChipUnit.size() + 1;

            //relatedChipUnitShort
            std::string l_relatedChipUnitShort = (const char *) l_ptr8;
            relatedChipUnitShort = l_relatedChipUnitShort;
            l_ptr8 += l_relatedChipUnitShort.size() + 1;
            l_len -= l_relatedChipUnitShort.size() + 1;

            // "clockDomain" (std::string)
            std::string l_clock_domain = (const char *)l_ptr8;
            clockDomain = l_clock_domain;
            l_ptr8 += l_clock_domain.size() + 1;
            l_len -= l_clock_domain.size() + 1;

            // "clockState" (ecmdClockState_t, stored as uint32_t)
	    memcpy( &tmpData32, l_ptr8, sizeof(tmpData32) );
	    clockState = (ecmdClockState_t)ntohl( tmpData32 );
	    l_ptr8 += sizeof(tmpData32);
	    l_len -= sizeof(tmpData32);

            // Final check: if the length isn't 0, something went wrong
            if (l_len < 0)
            {	
               // Generate an error for buffer overflow conditions.
               ETRAC("ECMD: Buffer overflow occurred - "
                      "struct size= %d; "
                      "input length= %d; remainder= %d\n",
                      this->flattenSize(), i_len, l_len);
               l_rc = ECMD_DATA_OVERFLOW;
               break;
            }

            if (l_len > 0)
            {	
               // Generate an error for buffer underflow conditions.
               ETRAC("ECMD: Buffer underflow occurred - "
                      "struct size= %d; "
                      "input length= %d; remainder= %d\n",
                      this->flattenSize(), i_len, l_len);
               l_rc = ECMD_DATA_UNDERFLOW;
               break;
            }

        } while (false);   // <- single exit

        return l_rc;
}

uint32_t ecmdLatchData::flattenSize() const
{
        uint32_t flatSize = 0;

        // Calculate the size needed to store the flattened struct
        flatSize = latchName.size() + 1
                   + ringName.size() + 1
                   + sizeof(latchId)
                   + sizeof(ringId)
                   + sizeof(bitLength)
                   + sizeof(uint32_t)   // isChipUnitRelated stored as uint32_t   
                   + relatedChipUnit.size() + 1                                  
                   + relatedChipUnitShort.size() + 1                                  
                   + clockDomain.size() + 1
                   + sizeof(uint32_t);  // ecmdClockState stored as uint32_t

        return flatSize;
}

#ifndef ECMD_STRIP_DEBUG
void  ecmdLatchData::printStruct() const
{

        printf("\n\t--- Latch Data Structure ---\n");

        printf("\tLatch Name: %s\n", latchName.c_str() );
        printf("\tRing Name: %s\n", ringName.c_str() );
        printf("\tLatch ID: 0x%08x\n", latchId );
        printf("\tRing ID: 0x%08x\n", ringId );
        printf("\tBit Length: %d\n", bitLength );
        printf("\tisChipUnitRelated: 0x%08x\n", (uint32_t) isChipUnitRelated);
        printf("\trelatedChipUnit:  %s\n", relatedChipUnit.c_str());
        printf("\trelatedChipUnitShort:  %s\n", relatedChipUnitShort.c_str());
        printf("\tClock Domain: %s\n", clockDomain.c_str() );
        printf("\tClock State: 0x%x\n", (uint32_t)clockState );

}
#endif  // end of ECMD_STRIP_DEBUG
// @06 end


// @07 start
/*
 * The following methods for the ecmdI2CCmdEntry struct will flatten,
 * unflatten & get the flattened size of the struct.
 */
uint32_t ecmdI2CCmdEntry::flatten(uint8_t *o_buf, uint32_t i_len) const
{
        uint32_t tmpData32 = 0;
        uint32_t l_rc = ECMD_SUCCESS;

        uint32_t dataBufSize = 0; // holder for ecmdDataBuffer size
        int l_len = (int)i_len;   // use a local copy to decrement
	uint8_t *l_ptr8 = o_buf;  // pointer to the output buffer

        do      // Single entry ->
        {
            // Check for buffer overflow conditions.
            if (this->flattenSize() > i_len) 
            {
                // Generate an error for buffer overflow conditions.
                ETRAC("ECMD: Buffer overflow occurred - "
                       "structure size = %d; input length = %d",
                       this->flattenSize(), i_len);
                l_rc = ECMD_DATA_OVERFLOW;
                break;
            }

            // Flatten and store each data member in the ouput buffer

            // "ecmdI2CCmd" (ecmdI2CCmds_t, stored as uint32_t)
	    tmpData32 = htonl( (uint32_t)ecmdI2CCmd );
	    memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
	    l_ptr8 += sizeof(tmpData32);
	    l_len -= sizeof(tmpData32);

            // "engineId" (uint32_t)
            tmpData32 = htonl( engineId );
            memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
            l_ptr8 += sizeof(engineId);
            l_len -= sizeof(engineId);

            // "port" (uint32_t)
            tmpData32 = htonl( port );
            memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
            l_ptr8 += sizeof(port);
            l_len -= sizeof(port);

            // "slaveaddress" (uint32_t)
            tmpData32 = htonl( slaveaddress );
            memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
            l_ptr8 += sizeof(slaveaddress);
            l_len -= sizeof(slaveaddress);

            // "busSpeed" (ecmdI2cBusSpeed_t, stored as uint32_t)
	    tmpData32 = htonl( (uint32_t)busSpeed );
	    memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
	    l_ptr8 += sizeof(tmpData32);
	    l_len -= sizeof(tmpData32);

            // "byteOffset" (uint32_t)
            tmpData32 = htonl( byteOffset );
            memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
            l_ptr8 += sizeof(byteOffset);
            l_len -= sizeof(byteOffset);

            // "offsetFieldSize" (uint32_t)
            tmpData32 = htonl( offsetFieldSize );
            memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
            l_ptr8 += sizeof(offsetFieldSize);
            l_len -= sizeof(offsetFieldSize);

            // "readByteLength" (uint32_t)
            tmpData32 = htonl( readByteLength );
            memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
            l_ptr8 += sizeof(readByteLength);
            l_len -= sizeof(readByteLength);

            // "data" (ecmdDataBuffer)
            //  1st save the size of the buffer
            dataBufSize = data.flattenSize();
            tmpData32 = htonl( dataBufSize );
            memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
            l_ptr8 += sizeof( dataBufSize );
            l_len -= sizeof( dataBufSize );

            //  2nd, flatten and save the buffer
            l_rc = data.flatten( l_ptr8, dataBufSize );

            // If the flatten failed, exit now with an error
            if ( l_rc != ECMD_DBUF_SUCCESS )
            {
               break;  // exit the do loop
            } 
            else
            {
               l_rc = ECMD_SUCCESS;  // restore standard success value
            }
            l_ptr8 += dataBufSize;
            l_len -= dataBufSize;

            // "i2cFlags" (uint32_t)
            tmpData32 = htonl( i2cFlags );
            memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
            l_ptr8 += sizeof(i2cFlags);
            l_len -= sizeof(i2cFlags);

            // Final check: if the length isn't 0, something went wrong
            if (l_len < 0)
            {	
               // Generate an error for buffer overflow conditions.
               ETRAC("ECMD: Buffer overflow occurred - "
                      "struct size= %d; "
                      "input length= %d; remainder= %d\n",
                      this->flattenSize(), i_len, l_len);
               l_rc = ECMD_DATA_OVERFLOW;
               break;
            }

            if (l_len > 0)
            {	
               // Generate an error for buffer underflow conditions.
               ETRAC("ECMD: Buffer underflow occurred - "
                      "struct size= %d; "
                      "input length= %d; remainder= %d\n",
                      this->flattenSize(), i_len, l_len);
               l_rc = ECMD_DATA_UNDERFLOW;
               break;
            }

        } while (false);   // <- single exit

        return l_rc;
}


uint32_t ecmdI2CCmdEntry::unflatten(const uint8_t *i_buf, uint32_t i_len)
{
        uint32_t l_rc = ECMD_SUCCESS;
        uint32_t tmpData32 = 0;

        uint32_t dataBufSize = 0;       // holds size of data buffer
	int l_len = (int)i_len;         // use a local copy to decrement
        const uint8_t *l_ptr8 = i_buf;  // pointer to the input buffer

        do    // Single entry ->
        {
            // Unflatten each data member from the input buffer

            // "ecmdI2CCmd" (ecmdI2CCmds_t, stored as uint32_t)
	    memcpy( &tmpData32, l_ptr8, sizeof(tmpData32) );
            ecmdI2CCmd = (ecmdI2CCmds_t)ntohl( tmpData32 );
	    l_ptr8 += sizeof(tmpData32);
	    l_len -= sizeof(tmpData32);

            // "engineId" (uint32_t)
            memcpy( &engineId, l_ptr8, sizeof(engineId) );
            engineId = ntohl( engineId );
            l_ptr8 += sizeof(engineId);
            l_len -= sizeof(engineId);

            // "port" (uint32_t)
            memcpy( &port, l_ptr8, sizeof(port) );
            port = ntohl( port );
            l_ptr8 += sizeof(port);
            l_len -= sizeof(port);

            // "slaveaddress" (uint32_t)
            memcpy( &slaveaddress, l_ptr8, sizeof(slaveaddress) );
            slaveaddress = ntohl( slaveaddress );
            l_ptr8 += sizeof(slaveaddress);
            l_len -= sizeof(slaveaddress);

            // "busSpeed" (ecmdI2cBusSpeed_t, stored as uint32_t)
	    memcpy( &tmpData32, l_ptr8, sizeof(tmpData32) );
            busSpeed = (ecmdI2cBusSpeed_t)ntohl( tmpData32 );
	    l_ptr8 += sizeof(tmpData32);
	    l_len -= sizeof(tmpData32);

            // "byteOffset" (uint32_t)
            memcpy( &byteOffset, l_ptr8, sizeof(byteOffset) );
            byteOffset = ntohl( byteOffset );
            l_ptr8 += sizeof(byteOffset);
            l_len -= sizeof(byteOffset);

            // "offsetFieldSize" (uint32_t)
            memcpy( &offsetFieldSize, l_ptr8, sizeof(offsetFieldSize) );
            offsetFieldSize = ntohl( offsetFieldSize );
            l_ptr8 += sizeof(offsetFieldSize);
            l_len -= sizeof(offsetFieldSize);

            // "readByteLength" (uint32_t)
            memcpy( &readByteLength, l_ptr8, sizeof(readByteLength) );
            readByteLength = ntohl( readByteLength );
            l_ptr8 += sizeof(readByteLength);
            l_len -= sizeof(readByteLength);

            // "data" (ecmdDataBuffer)
            //  1st get the size of the data buffer
            memcpy( &dataBufSize, l_ptr8, sizeof(dataBufSize) );
            dataBufSize = ntohl( dataBufSize );
            l_ptr8 += sizeof( dataBufSize );
            l_len -= sizeof( dataBufSize );

            //  2nd, unflatten the data buffer "data"
            l_rc = data.unflatten( l_ptr8, dataBufSize );

            // If the unflatten failed, exit now with an error
            if ( l_rc != ECMD_DBUF_SUCCESS )
            {
               break;
            }
            else
            {
               l_rc = ECMD_SUCCESS;  // restore standard success value
            }
            l_ptr8 += dataBufSize;
            l_len -= dataBufSize;

            // "i2cFlags" (uint32_t)
            memcpy( &i2cFlags, l_ptr8, sizeof(i2cFlags) );
            i2cFlags = ntohl( i2cFlags );
            l_ptr8 += sizeof(i2cFlags);
            l_len -= sizeof(i2cFlags);

            // Final check: if the length isn't 0, something went wrong
            if (l_len < 0)
            {	
               // Generate an error for buffer overflow conditions.
               ETRAC("ECMD: Buffer overflow occurred - "
                      "struct size= %d; "
                      "input length= %d; remainder= %d\n",
                      this->flattenSize(), i_len, l_len);
               l_rc = ECMD_DATA_OVERFLOW;
               break;
            }

            if (l_len > 0)
            {	
               // Generate an error for buffer underflow conditions.
               ETRAC("ECMD: Buffer underflow occurred - "
                      "struct size= %d; "
                      "input length= %d; remainder= %d\n",
                      this->flattenSize(), i_len, l_len);
               l_rc = ECMD_DATA_UNDERFLOW;
               break;
            }

        } while (false);   // <- single exit

        return l_rc;
}


uint32_t ecmdI2CCmdEntry::flattenSize() const
{
        uint32_t flatSize = 0;

        // Calculate the size needed to store the flattened struct
        flatSize = sizeof(uint32_t)  // ecmdI2CCmd stored as uint32_t
                   + sizeof(engineId)
                   + sizeof(port)
                   + sizeof(slaveaddress)
                   + sizeof(uint32_t)  // busSpeed stored as uint32_t
                   + sizeof(byteOffset)
                   + sizeof(offsetFieldSize)
                   + sizeof(readByteLength)
                   + sizeof(uint32_t)  // size of "data" stored in uint32_t
                   + data.flattenSize()
                   + sizeof(i2cFlags);

        return flatSize;
}


#ifndef ECMD_STRIP_DEBUG
void  ecmdI2CCmdEntry::printStruct() const
{
        uint32_t bufSize = 0;
        std::string bufString;

        printf("\n\t--- I2C CMD Entry Structure ---\n");

        printf("\tI2C Command: 0x%x\n", (uint32_t)ecmdI2CCmd);
        printf("\tEngine ID: 0x%08x\n", engineId);
        printf("\tPort: 0x%08x\n", port);
        printf("\tSlave Address: 0x%08x\n", slaveaddress);
        printf("\tBus Speed: 0x%x\n", (uint32_t)busSpeed);
        printf("\tOffset: 0x%08x\n", byteOffset);
        printf("\tOffset Field Size: 0x%08x\n", offsetFieldSize);
        printf("\tByte: 0x%08x\n", readByteLength);

        bufSize = data.getBitLength();
        bufString = data.genHexLeftStr(0, bufSize);
        printf("\tData: bit length = %d\n", bufSize);
        printf("\tData: contents = %s\n", bufString.c_str() );

        printf("\tI2CFlags: 0x%08x\n", i2cFlags);
}
#endif  // end of ECMD_STRIP_DEBUG
// @07 end

/*
 * The following methods for the ecmdNameEntry struct will flatten, unflatten &
 * get the flattened size of the struct.
 */
uint32_t ecmdNameEntry::flatten(uint8_t *, uint32_t ) const{

        return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t ecmdNameEntry::unflatten(const uint8_t *, uint32_t ) {
        return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t ecmdNameEntry::flattenSize() const {
        return ECMD_FUNCTION_NOT_SUPPORTED;
}

#ifndef ECMD_STRIP_DEBUG
void  ecmdNameEntry::printStruct() const {

        printf("\n\t--- Name Entry Structure ---\n");

        // Print non-list data.

}
#endif  // end of ECMD_STRIP_DEBUG

/*
 * The following methods for the ecmdNameVectorEntry struct will flatten, unflatten &
 * get the flattened size of the struct.
 */
uint32_t ecmdNameVectorEntry::flatten(uint8_t *, uint32_t ) const{

        return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t ecmdNameVectorEntry::unflatten(const uint8_t *, uint32_t ) {
        return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t ecmdNameVectorEntry::flattenSize() const{
        return ECMD_FUNCTION_NOT_SUPPORTED;
}

#ifndef ECMD_STRIP_DEBUG
void  ecmdNameVectorEntry::printStruct() const {

        printf("\n\t--- Name Vector Entry Structure ---\n");

        // Print non-list data.

}
#endif  // end of ECMD_STRIP_DEBUG


/*
 * The following methods for the ecmdIndexVectorEntry struct will flatten, unflatten &
 * get the flattened size of the struct.
 */
uint32_t ecmdIndexVectorEntry::flatten(uint8_t *o_buf, uint32_t i_len) const
{
	uint32_t tmpData32 = 0;
	uint32_t numElements = 0;
        uint32_t flatSize = 0;
	uint32_t l_rc = ECMD_SUCCESS ;

        int l_len = (int)i_len;  // use a local copy to decrement
	uint8_t *l_ptr8 = o_buf;

        std::vector<ecmdDataBuffer>::const_iterator bufIter;

        do      // Single entry ->
        {
            // Check for buffer overflow conditions.
            if (this->flattenSize() > i_len) 
            {
                // Generate an error for buffer overflow conditions.
                ETRAC("ECMD: Buffer overflow occurred - "
                       "structure size = %d; input length = %d",
                       this->flattenSize(), i_len);
                l_rc = ECMD_DATA_OVERFLOW;
                break;
            }

            // Copy non-list data
            tmpData32 = htonl( (uint32_t)index );
            memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
            l_ptr8 += sizeof(index);
            l_len -= sizeof(index);

            tmpData32 = htonl( rc );
            memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
            l_ptr8 += sizeof(rc);
            l_len -= sizeof(rc);

            // Store the number of elements in the vector "buffer"
            numElements = buffer.size();
            tmpData32 = htonl( numElements );
            memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
            l_ptr8 += sizeof(numElements);
            l_len -= sizeof(numElements);

            // Loop through the vector and for each element, store the
            //  struct size and the flattened data
            for (bufIter = buffer.begin(); bufIter != buffer.end(); ++bufIter)
            {
                flatSize = bufIter->flattenSize();
                tmpData32 = htonl( flatSize );
                memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
                l_ptr8 += sizeof(flatSize);
                l_len -= sizeof(flatSize);

                l_rc = bufIter->flatten( l_ptr8, flatSize );
                if ( l_rc != ECMD_DBUF_SUCCESS ) { break; }  // exit for loop
                l_ptr8 += flatSize;
                l_len -= flatSize;
            }

            // Error checking - if there was a flatten error, report it
            if ( l_rc != ECMD_DBUF_SUCCESS )
            {
                ETRAC("ECMD: flatten error occurred - "
                       "rc= 0x%x", l_rc );
                break;
            }
            else  // only check the length if there was no flatten error
            {     // if the length isn't 0, something went wrong

               if (l_len < 0)
               {	
                  // Generate an error for buffer overflow conditions.
                  ETRAC("ECMD: Buffer overflow occurred - "
                         "struct size= %d; "
                         "input length= %x; remainder= %d\n",
                         this->flattenSize(), i_len, l_len);
                  l_rc = ECMD_DATA_OVERFLOW;
                  break;
               }

               if (l_len > 0)
               {	
                  // Generate an error for buffer underflow conditions.
                  ETRAC("ECMD: Buffer underflow occurred - "
                         "struct size= %d; "
                         "input length= %x; remainder= %d\n",
                         this->flattenSize(), i_len, l_len);
                  l_rc = ECMD_DATA_UNDERFLOW;
                  break;
               }

            }  // else

         } while (false);   // <- single exit

         return l_rc;
}

uint32_t ecmdIndexVectorEntry::unflatten(const uint8_t *i_buf, uint32_t i_len) {
        uint32_t l_rc = ECMD_SUCCESS;

        uint32_t tmpData32  = 0;
	uint32_t numElements = 0;
        uint32_t flatSize = 0;
	uint32_t i = 0;

        const uint8_t *l_ptr8 = i_buf;
	int l_len = (int)i_len;

        do      // Single entry ->
        {
           // Unflatten non-list data
           memcpy( &tmpData32, l_ptr8, sizeof(tmpData32) );
           index = (int)ntohl( tmpData32 );
           l_ptr8 += sizeof(index);
           l_len -= sizeof(index);

           memcpy( &rc, l_ptr8, sizeof(rc) );
           rc = ntohl( rc );
           l_ptr8 += sizeof(rc);
           l_len -= sizeof(rc);

           // Get the number of elements in the vector to restore it
           memcpy( &numElements, l_ptr8, sizeof(numElements) );
           numElements = ntohl( numElements );
           l_ptr8 += sizeof(numElements);
           l_len -= sizeof(numElements);

           // Now reconstruct the vector "buffer"
           buffer.clear();

           for (i = 0; i < numElements; ++i)
           {
              ecmdDataBuffer l_ecmdBuffer;

              memcpy( &flatSize, l_ptr8, sizeof(flatSize) );
              flatSize = ntohl(flatSize);
              l_ptr8 += sizeof(flatSize);
              l_len -= sizeof(flatSize);

              l_rc = l_ecmdBuffer.unflatten( l_ptr8, flatSize );
              if (l_rc != ECMD_DBUF_SUCCESS) { break; }  // exit for loop
              buffer.push_back( l_ecmdBuffer );
              l_ptr8 += flatSize;
              l_len -= flatSize;
           }

            // Error checking - if there was an unflatten error, report it
            if ( l_rc != ECMD_DBUF_SUCCESS )
            {
                ETRAC("ECMD: unflatten error occurred - "
                       "rc= 0x%x", l_rc );
                break;
            }
            else  // only check the length if there was no unflatten error
            {     // if the length isn't 0, something went wrong

               if (l_len < 0)
               {	
                  // Generate an error for buffer overflow conditions.
                  ETRAC("ECMD: Buffer overflow occurred - "
                         "struct size= %d; "
                         "input length= %x; remainder= %d\n",
                         this->flattenSize(), i_len, l_len);
                  l_rc = ECMD_DATA_OVERFLOW;
                  break;
               }

               if (l_len > 0)
               {	
                  // Generate an error for buffer underflow conditions.
                  ETRAC("ECMD: Buffer underflow occurred - "
                         "struct size= %d; "
                         "input length= %x; remainder= %d\n",
                         this->flattenSize(), i_len, l_len);
                  l_rc = ECMD_DATA_UNDERFLOW;
                  break;
               }

            }  // else

        } while (false);   // <- single exit

        return l_rc;
}

uint32_t ecmdIndexVectorEntry::flattenSize() const
{
        uint32_t flatSize = 0;
        std::vector<ecmdDataBuffer>::const_iterator bufIter;

        // Size of non-list member data.
        flatSize = sizeof(index) +
                   sizeof(rc);

        // Size of the vector data
        flatSize += sizeof(uint32_t);  // space for the number of elements
            
        // For each element, leave room for a struct size and the
        //  flattened size
        for (bufIter = buffer.begin(); bufIter != buffer.end(); ++bufIter)
        {
            flatSize += sizeof(uint32_t);  // space for the struct size
            flatSize += bufIter->flattenSize();  // for the flattened struct
        }

        return flatSize;
}

#ifndef ECMD_STRIP_DEBUG
void  ecmdIndexVectorEntry::printStruct() const {

        printf("\n\t--- Index Vector Entry Structure ---\n");

        // Print non-list data.
        printf("\tIndex: %d\n", index);
        printf("\trc: 0x%08x\n", rc);

        printf("\tNumber of elements in buffer vector: %lu\n", (unsigned long)buffer.size());

}
#endif  // end of ECMD_STRIP_DEBUG

/*
 * The following methods for the ecmdLatchEntry struct will flatten, unflatten &
 * get the flattened size of the struct.
 */
uint32_t ecmdLatchEntry::flatten(uint8_t *, uint32_t ) const {

        return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t ecmdLatchEntry::unflatten(const uint8_t *, uint32_t ) {
        return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t ecmdLatchEntry::flattenSize() const {
        return ECMD_FUNCTION_NOT_SUPPORTED;
}

#ifndef ECMD_STRIP_DEBUG
void  ecmdLatchEntry::printStruct() const {

        printf("\n\t--- Latch Entry Structure ---\n");

        // Print non-list data.

}
#endif  // end of ECMD_STRIP_DEBUG

/*
 * The following methods for the ecmdLatchQueryData struct will flatten, unflatten &
 * get the flattened size of the struct.
 */

bool ecmdLatchQueryData::isChipUnitMatch(std::string &i_chipUnitType) {
  /* If either matches, return true */
  if (i_chipUnitType == relatedChipUnit || i_chipUnitType == relatedChipUnitShort) {
    return true;
  }

  return false;
}

uint32_t ecmdLatchQueryData::flatten(uint8_t *, uint32_t ) const {

        return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t ecmdLatchQueryData::unflatten(const uint8_t *, uint32_t ) {
        return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t ecmdLatchQueryData::flattenSize() const {
        return ECMD_FUNCTION_NOT_SUPPORTED;
}

#ifndef ECMD_STRIP_DEBUG
void  ecmdLatchQueryData::printStruct() const {

        printf("\n\t--- Latch Entry Structure ---\n");

        // Print non-list data.

}
#endif  // end of ECMD_STRIP_DEBUG

/*
 * The following methods for the ecmdProcRegisterInfo struct will flatten, unflatten &
 * get the flattened size of the struct.
 */
bool ecmdProcRegisterInfo::isChipUnitMatch(std::string &i_chipUnitType) {
  /* If either matches, return true */
  if (i_chipUnitType == relatedChipUnit || i_chipUnitType == relatedChipUnitShort) {
    return true;
  }

  return false;
}

uint32_t ecmdProcRegisterInfo::flatten(uint8_t *o_buf, uint32_t i_len) const
{
   // @0aa - below
   uint32_t l_rc = ECMD_SUCCESS ;
   uint8_t *l_ptr = o_buf;
   uint32_t l_temp32;

   do
   {  // Single entry ->

      // Check for buffer overflown conditions.
      if (this->flattenSize() > i_len)
      {
         // Generate an error for buffer overflow conditions.
         ETRAC("Buffer overflow occured - "
                "structure size = %d; "
                "input length = %d",
                this->flattenSize(), i_len);
         l_rc = ECMD_DATA_OVERFLOW;
         break;
      }
      
      l_temp32 = htonl(bitLength);//@0b
      memcpy(l_ptr, &l_temp32, sizeof(bitLength));//@0b
      l_ptr += sizeof(bitLength);

      l_temp32 = htonl(totalEntries);//@0b
      memcpy(l_ptr, &l_temp32, sizeof(totalEntries));//@0b
      l_ptr += sizeof(totalEntries);

      //"isChipUnitRelated" (bool, store in uint32_t)        
      l_temp32 = htonl( (uint32_t)isChipUnitRelated );
      memcpy( l_ptr, &l_temp32, sizeof(l_temp32) );
      l_ptr += sizeof(l_temp32);

      //relatedChipUnit     
      memcpy(l_ptr, relatedChipUnit.c_str(), relatedChipUnit.size() + 1);
      l_ptr += relatedChipUnit.size() + 1;

      //relatedChipUnitShort 
      memcpy(l_ptr, relatedChipUnitShort.c_str(), relatedChipUnitShort.size() + 1);
      l_ptr += relatedChipUnitShort.size() + 1;

      // Store boolean as uint32_t, just to be careful...
      l_temp32 = htonl((uint32_t)threadReplicated);
      memcpy(l_ptr, &l_temp32, sizeof(l_temp32));
      l_ptr += sizeof(l_temp32);

   } while (0);  // <- single exit

   return l_rc;
   // @0aa - above
}

uint32_t ecmdProcRegisterInfo::unflatten(const uint8_t *i_buf, uint32_t i_len)
{
   // @0aa - below
   uint32_t l_rc       = ECMD_SUCCESS;

   uint8_t *l_ptr = (uint8_t *)i_buf;
   uint32_t l_temp32;

   do
   {  // Single entry ->

      // Check for buffer overflown conditions. 
      if (this->flattenSize() > i_len) {                //@01c
         // Generate an error for buffer overflow conditions.
         ETRAC("Buffer overflow occured - "
                "structure size = %d; "
                "input length = %d",
                this->flattenSize(), i_len);
         l_rc = ECMD_DATA_OVERFLOW;
         break;
      }

      memcpy(&bitLength, l_ptr, sizeof(bitLength));
      l_ptr += sizeof(bitLength);
      bitLength = ntohl(bitLength); //@0b - for putspr error

      memcpy(&totalEntries, l_ptr, sizeof(totalEntries));
      l_ptr += sizeof(totalEntries);
      totalEntries = ntohl(totalEntries);//@0b  

      //isChipUnitRelated (bool, but stored in uint32_t
      memcpy(&l_temp32, l_ptr, sizeof(l_temp32));
      isChipUnitRelated = (bool)ntohl(l_temp32);
      l_ptr += sizeof(l_temp32);

      //relatedChipUnit
      std::string l_relatedChipUnit = (const char *) l_ptr;  //maybe this can be 1 line?
      relatedChipUnit = l_relatedChipUnit;
      l_ptr += l_relatedChipUnit.size() + 1;
      
      //relatedChipUnitShort
      std::string l_relatedChipUnitShort = (const char *) l_ptr;  //maybe this can be 1 line?
      relatedChipUnitShort = l_relatedChipUnitShort;
      l_ptr += l_relatedChipUnitShort.size() + 1;

      // threadReplicated stored as uint32_t, so convert back to bool here...
      memcpy(&l_temp32, l_ptr, sizeof(l_temp32));
      threadReplicated = (bool)ntohl(l_temp32);
      l_ptr += sizeof(l_temp32);

   } while (0);  // <- single exit

   return l_rc;
   // @0aa - above
}

uint32_t ecmdProcRegisterInfo::flattenSize() const
{
   // @0aa - below
   uint32_t flatSize = 0;

   flatSize += sizeof(bitLength);
   flatSize += sizeof(totalEntries);
   flatSize += sizeof(uint32_t); // for isChipUnitRelated (bool stored as uint32_t)
   flatSize += relatedChipUnit.size() + 1;  
   flatSize += relatedChipUnitShort.size() + 1;
   // threadReplicated stored as uint32_t...
   flatSize += sizeof(uint32_t);

   return flatSize;
   // @0aa - above
}

#ifndef ECMD_STRIP_DEBUG
void  ecmdProcRegisterInfo::printStruct() const
{

   printf("\n\t--- Proc Register Info Structure ---\n");

   // Print non-list data.
   // @0aa - below
   printf("\t\tbitLength:        %d\n", bitLength);
   printf("\t\ttotalEntries:     %d\n", totalEntries);
   printf("\tisChipUnitRelated: 0x%08x\n", (uint32_t) isChipUnitRelated);
   printf("\trelatedChipUnit:  %s\n", relatedChipUnit.c_str());
   printf("\trelatedChipUnitShort:  %s\n", relatedChipUnitShort.c_str());
   printf("\t\tthreadReplicated: %d\n", threadReplicated);
   // @0aa - above

}
#endif  // end of ECMD_STRIP_DEBUG

/*
 * The following methods for the ecmdCacheData struct will flatten, unflatten &
 * get the flattened size of the struct.
 */
bool ecmdCacheData::isChipUnitMatch(std::string &i_chipUnitType) {
  /* If either matches, return true */
  if (i_chipUnitType == relatedChipUnit || i_chipUnitType == relatedChipUnitShort) {
    return true;
  }

  return false;
}

uint32_t ecmdCacheData::flatten(uint8_t *o_buf, uint32_t i_len) const
{
   // @0aa - below
   uint32_t l_rc = ECMD_SUCCESS ;
   uint8_t *l_ptr = o_buf;
   uint32_t l_temp32;

   do
   {  // Single entry -> 

      // Check for buffer overflown conditions.
      if (this->flattenSize() > i_len)
      {
         // Generate an error for buffer overflow conditions.
         ETRAC("Buffer overflow occured - "
                "structure size = %d; "
                "input length = %d",
                this->flattenSize(), i_len);
         l_rc = ECMD_DATA_OVERFLOW;
         break;
      }

      //"isChipUnitRelated" (bool, store in uint32_t)        
      l_temp32 = htonl( (uint32_t)isChipUnitRelated );
      memcpy( l_ptr, &l_temp32, sizeof(l_temp32) );
      l_ptr += sizeof(l_temp32);

      //relatedChipUnit     
      memcpy(l_ptr, relatedChipUnit.c_str(), relatedChipUnit.size() + 1);
      l_ptr += relatedChipUnit.size() + 1;

      //relatedChipUnitShort
      memcpy(l_ptr, relatedChipUnitShort.c_str(), relatedChipUnitShort.size() + 1);
      l_ptr += relatedChipUnitShort.size() + 1;

   } while (0);  // <- single exit

   return l_rc;
}



uint32_t ecmdCacheData::unflatten(const uint8_t *i_buf, uint32_t i_len)
{
   // @0aa - below
   uint32_t l_rc       = ECMD_SUCCESS;

   uint8_t *l_ptr = (uint8_t *)i_buf;
   uint32_t l_temp32;

   do
   {  // Single entry ->

      // Check for buffer overflown conditions. 
      if (this->flattenSize() > i_len) {                //@01c
         // Generate an error for buffer overflow conditions.
         ETRAC("Buffer overflow occured - "
                "structure size = %d; "
                "input length = %d",
                this->flattenSize(), i_len);
         l_rc = ECMD_DATA_OVERFLOW;
         break;
      }

      //isChipUnitRelated (bool, but stored in uint32_t
      memcpy(&l_temp32, l_ptr, sizeof(l_temp32));
      isChipUnitRelated = (bool)ntohl(l_temp32);
      l_ptr += sizeof(l_temp32);

      //relatedChipUnit
      std::string l_relatedChipUnit = (const char *) l_ptr;  //maybe this can be 1 line?
      relatedChipUnit = l_relatedChipUnit;
      l_ptr += l_relatedChipUnit.size() + 1;

      //relatedChipUnitShort
      std::string l_relatedChipUnitShort = (const char *) l_ptr; 
      relatedChipUnitShort = l_relatedChipUnitShort;
      l_ptr += l_relatedChipUnitShort.size() + 1;


   } while (0);  // <- single exit

   return l_rc;
}

uint32_t ecmdCacheData::flattenSize() const
{
   // @0aa - below
   uint32_t flatSize = 0;

   flatSize += sizeof(uint32_t); // for isChipUnitRelated (bool stored as uint32_t)
   flatSize += relatedChipUnit.size() + 1;
   flatSize += relatedChipUnitShort.size() + 1;

   return flatSize;
}

#ifndef ECMD_STRIP_DEBUG
void  ecmdCacheData::printStruct() const
{

   printf("\n\t--- Proc Register Info Structure ---\n");

   // Print non-list data.
   printf("\tisChipUnitRelated: 0x%08x\n", (uint32_t) isChipUnitRelated);
   printf("\trelatedChipUnit:  %s\n", relatedChipUnit.c_str());
   printf("\trelatedChipUnitShort:  %s\n", relatedChipUnitShort.c_str());

}
#endif  // end of ECMD_STRIP_DEBUG


/*
 * The following methods for the ecmdScomEntry struct will flatten, unflatten &
 * get the flattened size of the struct.
 */
uint32_t ecmdScomEntry::flatten(uint8_t * o_buf, uint32_t i_len) const {

    uint32_t tmpData32 = 0;
    uint32_t dataBufSize = 0;    // temp holder for ecmdDataBuffer size
    uint32_t l_rc = ECMD_SUCCESS;

    uint8_t * l_ptr = o_buf;

    do {    // Single entry ->

        // Check for buffer size mismatch (overflow or underflow)
        if ( this->flattenSize() != i_len ) {
        ETRAC("Buffer overflow occurred - "
              "structure size = %d; input length = %d",
              this->flattenSize(), i_len );
        l_rc = ECMD_DATA_OVERFLOW;
        break;
        }

        // Write "address" into the output buffer
        // "address" (uint64_t) : first part
        tmpData32 = htonl( address >> 32 );
        memcpy( l_ptr, &tmpData32, sizeof(tmpData32) );
        l_ptr += sizeof(tmpData32);
        i_len -= sizeof(tmpData32);

        // "address" (uint64_t) : second part
	tmpData32 = htonl(address);
	memcpy(l_ptr, &tmpData32, sizeof(tmpData32));
	l_ptr += sizeof(tmpData32);
	i_len -= sizeof(tmpData32);

        // Write the size of "scomData", to check against when unflattening
        dataBufSize = data.flattenSize();
        tmpData32 = htonl( dataBufSize );
        memcpy( l_ptr, &tmpData32, sizeof(tmpData32) );
        l_ptr += sizeof( dataBufSize );
        i_len -= sizeof( dataBufSize );

        // Write contents of "data" into the output buffer
        l_rc = data.flatten( l_ptr, dataBufSize );
        if ( l_rc != ECMD_DBUF_SUCCESS ) {
            break;
        } else {
           l_rc = ECMD_SUCCESS;
        }
        l_ptr += dataBufSize;
        i_len -= dataBufSize;

        // Write the size of "dataMask", to check against when unflattening
        dataBufSize = dataMask.flattenSize();
        tmpData32 = htonl( dataBufSize );
        memcpy( l_ptr, &tmpData32, sizeof(tmpData32) );
        l_ptr += sizeof( dataBufSize );
        i_len -= sizeof( dataBufSize );

        // Write contents of "dataMask" into the output buffer
        l_rc = dataMask.flatten( l_ptr, dataBufSize );
        if ( l_rc != ECMD_DBUF_SUCCESS ) {
            break;
        } else {
           l_rc = ECMD_SUCCESS;
        }
        l_ptr += dataBufSize;
        i_len -= dataBufSize;

        // "operation" (ecmdScomMode_t, stored as uint32_t)
        tmpData32 = htonl( (uint32_t) operation );
        memcpy( l_ptr, &tmpData32, sizeof(tmpData32) );
        l_ptr += sizeof(tmpData32);
        i_len -= sizeof(tmpData32);

        // Write "rc" into the output buffer
        tmpData32 = htonl( rc );
        memcpy( l_ptr, &tmpData32, sizeof(tmpData32) );
        l_ptr += sizeof( rc );
        i_len -= sizeof( rc );

        // If the length has not decremented to 0, something is wrong
        if ( i_len != 0 ) {
        ETRAC("Buffer overflow occurred - "
              "leftover data bytes = %d", i_len );
           l_rc = ECMD_DATA_OVERFLOW;
           break;
        }

    } while (0);    // <- single exit.

    return l_rc;
}

uint32_t ecmdScomEntry::unflatten(const uint8_t * i_buf, uint32_t i_len) {

    uint32_t tmpData32 = 0;
    uint32_t dataBufSize = 0;
    uint32_t l_rc = ECMD_SUCCESS;

    uint8_t * l_ptr = (uint8_t *) i_buf;

    do {    // Single entry ->

        // Get "address" from the input buffer
        // "address" (uint64_t)
	//first part(uint32_t)
        uint32_t tmp32_0 = 0, tmp32_1 = 0;
        memcpy( &tmp32_0, l_ptr, sizeof(tmp32_0) );
        tmp32_0 = ntohl( tmp32_0);
        l_ptr += sizeof(tmp32_0);
        i_len -= sizeof(tmp32_0);
 
	//second part(uint32_t)
        memcpy( &tmp32_1, l_ptr, sizeof(tmp32_1) );
        tmp32_1 = ntohl( tmp32_1);
        l_ptr += sizeof(tmp32_1);
        i_len -= sizeof(tmp32_1);
        //combine both uint32_t to uint64_t
	address = ( ((uint64_t) tmp32_0) << 32) | ( (uint64_t) tmp32_1);

        // Get the size of "data" to pass to unflatten()
        memcpy( &dataBufSize, l_ptr, sizeof(dataBufSize) );
        dataBufSize = ntohl( dataBufSize );
        l_ptr += sizeof( dataBufSize );
        i_len -= sizeof( dataBufSize );

        // Unflatten "data" from the input buffer
        l_rc = data.unflatten( l_ptr, dataBufSize );
        if ( l_rc != ECMD_DBUF_SUCCESS ) {
            break;
        } else {
            l_rc = ECMD_SUCCESS;
        }
        l_ptr += dataBufSize;
        i_len -= dataBufSize;

        // Get the size of "dataMask" to pass to unflatten()
        memcpy( &dataBufSize, l_ptr, sizeof(dataBufSize) );
        dataBufSize = ntohl( dataBufSize );
        l_ptr += sizeof( dataBufSize );
        i_len -= sizeof( dataBufSize );

        // Unflatten "dataMask" from the input buffer
        l_rc = dataMask.unflatten( l_ptr, dataBufSize );
        if ( l_rc != ECMD_DBUF_SUCCESS ) {
            break;
        } else {
            l_rc = ECMD_SUCCESS;
        }
        l_ptr += dataBufSize;
        i_len -= dataBufSize;

        // "operation" (ecmdScomMode_t, stored as uint32_t)
        memcpy( &tmpData32, l_ptr, sizeof(tmpData32) );
        operation = (ecmdScomMode_t) ntohl( tmpData32 );
        l_ptr += sizeof(tmpData32);
        i_len -= sizeof(tmpData32);

        // Get "rc" from the input buffer
        memcpy( &tmpData32, l_ptr, sizeof(tmpData32) );
        rc = ntohl( tmpData32 );
        l_ptr += sizeof( tmpData32 );
        i_len -= sizeof( tmpData32 );

        // If the length has not decremented to 0, something is wrong
        if ( i_len != 0 )
        {
          ETRAC("Buffer overflow occurred - "
                "leftover data bytes = %d", i_len );
          l_rc = ECMD_DATA_OVERFLOW;
          break;
        }

    } while (0);    // <- single exit.

    return l_rc;
}

uint32_t ecmdScomEntry::flattenSize() const {

    uint32_t flatSize = 0;

        uint32_t dataBufSize = 0;  // A size is stored for each ecmdDataBuf

        flatSize += sizeof( address );           // Space for member "address"

        flatSize += sizeof( dataBufSize );  // Size of member "data"
        flatSize += data.flattenSize();  // Space for "data" flattened

        flatSize += sizeof( dataBufSize );  // Size of member "dataMask"
        flatSize += dataMask.flattenSize();   // Space for "dataMask" flattened

        flatSize += sizeof(uint32_t);  //  Space for operation, ecmdScomMode_t stored as uint32_t
        flatSize += sizeof( rc );           // Space for member "rc"

    return flatSize;
}


#ifndef ECMD_STRIP_DEBUG
void ecmdScomEntry::printStruct() const {

    printf("\n\t\teCMD Scom Entry:\n");

#ifdef _LP64
    printf("\t\t\taddress: 0x%lx\n", address );
#else
    printf("\t\t\taddress: 0x%llx\n", address );
#endif

    printf("\t\t\tdata bitlength: %d\n", data.getBitLength() );
    printf("\t\t\tdata wordlength: %d\n", data.getWordLength() );
    printf("\t\t\tdata data: %s\n", data.genHexLeftStr().c_str() );

    printf("\t\t\tdataMask bitlength: %d\n", dataMask.getBitLength() );
    printf("\t\t\tdataMask wordlength: %d\n", dataMask.getWordLength() );
    printf("\t\t\tdataMask data: %s\n", dataMask.genHexLeftStr().c_str() );

    printf("\t\t\toperation: 0x%x\n", (uint32_t) operation );
    printf("\t\t\treturn code (rc): 0x%x\n", rc );
}
#endif
// @02 end



/*
 * The following methods for the ecmdConnectionData struct will flatten, unflatten &
 * get the flattened size of the struct.
 */
uint32_t ecmdConnectionData::flatten(uint8_t *o_buf, uint32_t i_len) const {
  uint32_t tmpData32 = 0;
  uint32_t l_rc = ECMD_SUCCESS;

  int l_len = (int)i_len;   // use a local copy to decrement
  uint8_t *l_ptr8 = o_buf;  // pointer to the output buffer

  do      // Single entry ->
  {
      // Check for buffer overflow conditions.
      if (this->flattenSize() > i_len)
      {
        // Generate an error for buffer overflow conditions.
	ETRAC("ECMD: Buffer overflow occurred - "
              "structure size = %d; input length = %d",
              this->flattenSize(), i_len);
	l_rc = ECMD_DATA_OVERFLOW;
	break;
      }

      // Flatten and store each data member in the ouput buffer
      // Write the size of "target", to check against when unflattening
      uint32_t dataBufSize = targetA.flattenSize();
      tmpData32 = htonl( dataBufSize );
      memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
      l_ptr8 += sizeof( dataBufSize );
      l_len -= sizeof( dataBufSize );

      // Write targetA into the output buffer
      l_rc = targetA.flatten( l_ptr8, dataBufSize );
      if ( l_rc != ECMD_DBUF_SUCCESS ) {
	      break;
      } else {
	      l_rc = ECMD_SUCCESS;
      }
      l_ptr8 += dataBufSize;
      l_len -= dataBufSize;

      //portA
      memcpy(l_ptr8, portA.c_str(), portA.size() + 1);
      l_ptr8 += (portA.size() + 1);
      l_len  -= (portA.size() + 1);

      //connectionType
      memcpy(l_ptr8, connectionType.c_str(), connectionType.size() + 1);
      l_ptr8 += (connectionType.size() + 1);
      l_len  -= (connectionType.size() + 1);


      // "portB" (std::string)
      memcpy( l_ptr8, portB.c_str(), portB.size() + 1 );
      l_ptr8 += (portB.size() + 1);
      l_len  -= (portB.size() + 1);

      // Write the size of "targetB", to check against when unflattening
      dataBufSize = targetB.flattenSize();
      tmpData32 = htonl( dataBufSize );
      memcpy( l_ptr8, &tmpData32, sizeof(tmpData32) );
      l_ptr8 += sizeof( dataBufSize );
      l_len -= sizeof( dataBufSize );

      // Write targetB into the output buffer
      l_rc = targetB.flatten( l_ptr8, dataBufSize );
      if ( l_rc != ECMD_DBUF_SUCCESS ) {
	      break;
      } else {
	      l_rc = ECMD_SUCCESS;
      }
      l_ptr8 += dataBufSize;
      l_len -= dataBufSize;

      // Final check: if the length isn't 0, something went wrong
      if (l_len < 0)
      {
         // Generate an error for buffer overflow conditions.
	 ETRAC("ECMD: Buffer overflow occurred in "
		      "ecmdConnectionData::flatten(), struct size= %d; "
		      "input length= %d; over-length size= %d\n",
		      this->flattenSize(), i_len, l_len);
	 l_rc = ECMD_DATA_OVERFLOW;
	 break;
      }

      if (l_len > 0)
      {
	  // Generate an error for buffer underflow conditions.
	  ETRAC("ECMD: Buffer underflow occurred - "
                "struct size= %d; "
                "input length= %d; remainder= %d\n",
                this->flattenSize(), i_len, l_len);
	  l_rc = ECMD_DATA_UNDERFLOW;
	  break;
      }
  } while (false);   // <- single exit
  return l_rc;
}

uint32_t ecmdConnectionData::unflatten(const uint8_t *i_buf, uint32_t i_len) {
   uint32_t l_rc = ECMD_SUCCESS;
   int l_len = (int)i_len;         // use a local copy to decrement
   const uint8_t *l_ptr8 = i_buf;  // pointer to the input buffer

   do    // Single entry ->
   {
	// Unflatten each data member from the input buffer

	uint32_t dataBufSize = 0;
	// Get the size of "targetA" to pass to unflatten()
	memcpy( &dataBufSize, l_ptr8, sizeof(dataBufSize) );
	dataBufSize = ntohl( dataBufSize );
	l_ptr8 += sizeof( dataBufSize );
	l_len -= sizeof( dataBufSize );
	
	// Unflatten "targetA" from the input buffer
	l_rc = targetA.unflatten( l_ptr8, dataBufSize );
	if ( l_rc != ECMD_DBUF_SUCCESS ) {
		break;
	} else {
		l_rc = ECMD_SUCCESS;
	}
	l_ptr8 += dataBufSize;
	l_len -= dataBufSize;
            
	//portA
	std::string l_portA = (const char *) l_ptr8;  //maybe this can be 1 line?
	portA = l_portA;
	l_ptr8 += l_portA.size() + 1;
	l_len -= l_portA.size() + 1;
	
	//connectionType
	std::string l_connectionType = (const char *) l_ptr8;
	connectionType = l_connectionType;
	l_ptr8 += l_connectionType.size() + 1;
	l_len -= l_connectionType.size() + 1;

	//portB
	std::string l_portB = (const char *) l_ptr8;  //maybe this can be 1 line?
	portB = l_portB;
	l_ptr8 += l_portB.size() + 1;
	l_len -= l_portB.size() + 1;

	// Get the size of "targetB" to pass to unflatten()
	memcpy( &dataBufSize, l_ptr8, sizeof(dataBufSize) );
	dataBufSize = ntohl( dataBufSize );
	l_ptr8 += sizeof( dataBufSize );
	l_len -= sizeof( dataBufSize );

	// Unflatten "targetB" from the input buffer
	l_rc = targetB.unflatten( l_ptr8, dataBufSize );
	if ( l_rc != ECMD_DBUF_SUCCESS ) {
		break;
	} else {
		l_rc = ECMD_SUCCESS;
	}
	l_ptr8 += dataBufSize;
	l_len -= dataBufSize;

	// Final check: if the length isn't 0, something went wrong
	if (l_len < 0)
	{
	   // Generate an error for buffer overflow conditions.
	   ETRAC("ECMD: Buffer overflow occurred - "
                 "struct size= %d; "
                 "input length= %d; over-length size= %d\n",
                 this->flattenSize(), i_len, l_len);
	   l_rc = ECMD_DATA_OVERFLOW;
	   break;
	}
	if (l_len > 0)
	{
	   // Generate an error for buffer underflow conditions.
	   ETRAC("ECMD: Buffer underflow occurred - "
                 "struct size= %d; "
                 "input length= %d; remainder= %d\n",
                 this->flattenSize(), i_len, l_len);
	   l_rc = ECMD_DATA_UNDERFLOW;
	   break;
	}
   } while (false);   // <- single exit
   
   return l_rc;
}

uint32_t ecmdConnectionData::flattenSize() const {
  uint32_t flatSize = 0;
  uint32_t dataBufSize = 0;
  // Calculate the size needed to store the flattened struct
  flatSize = sizeof(dataBufSize)+ targetA.flattenSize()
	  + portA.size()+1
	  + connectionType.size() + 1
	  + portB.size() + 1
	  + sizeof(dataBufSize)+ targetB.flattenSize();

  return flatSize;
}

#ifndef ECMD_STRIP_DEBUG
void  ecmdConnectionData::printStruct() const {

  printf("\n\t--- Connection Data Structure ---\n");

  // Print non-list data.
  printf("\tPort A:  %s\n", portA.c_str());
  printf("\tConnection Type:  %s\n", connectionType.c_str());
  printf("\tPort B:  %s\n", portB.c_str());

}
#endif
