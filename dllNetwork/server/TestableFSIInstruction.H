#ifndef _TestableFSIInstruction_H
#define _TestableFSIInstruction_H
//IBM_PROLOG_BEGIN_TAG
/* 
 * Copyright 2003,2017 IBM International Business Machines Corp.
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
 * @file TestableFSIInstruction.H
 * @brief Provides a class to test FSI instructions
 *
*/

//--------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------
#include <FSIInstruction.H>

class TestableFSIInstruction : public FSIInstruction
{
    public:
    uint32_t getCfamid() { return cfamid; }
    uint32_t getLinkid() { return linkid; }
    uint32_t getCmaster() { return cmaster; }
    uint32_t getAddress() { return address; }
    uint64_t getAddress64() { return address64; }
    uint32_t getLength() { return length; }
    const ecmdDataBuffer * getData() { return (data.getBitLength() ? &data : NULL); }
    const ecmdDataBuffer * getMask() { return (mask.getBitLength() ? &mask : NULL); }
    const std::string & getDeviceString() { return deviceString; }
};

#endif // _TestableFSIInstruction_H
