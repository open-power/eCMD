#ifndef _TestableGPIOInstruction_H
#define _TestableGPIOInstruction_H
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
 * @file TestableGPIOInstruction.H
 * @brief Provides a class for test GPIO instructions
 *
*/

//--------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------
#include <GPIOInstruction.H>

class TestableGPIOInstruction : public GPIOInstruction
{
    public:
    uint32_t getCfamid() { return cfamid; }
    uint32_t getLinkid() { return linkid; }
    uint32_t getCmaster() { return cmaster; }
    uint32_t getEngineId() { return engineId; }
    uint32_t getPin() { return pin; }
    uint32_t getMask() { return mask; }
    gpioDioMode_t getMode() { return mode; }
    uint32_t getData() { return data; }
    const std::string & getDeviceString() { return deviceString; }
};

#endif // _TestableGPIOInstruction_H

