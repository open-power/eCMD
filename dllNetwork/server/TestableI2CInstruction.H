#ifndef _TestableI2CInstruction_H
#define _TestableI2CInstruction_H
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
 * @file TestableI2CInstruction.H
 * @brief Provides a class for test I2C instructions
 *
*/

//--------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------
#include <I2CInstruction.H>

class TestableI2CInstruction : public I2CInstruction
{
    public:
    uint32_t getCfamid() { return cfamid; }
    uint32_t getLinkid() { return linkid; }
    uint32_t getEngineId() { return engineId; }
    uint32_t getPort() { return port; }
    uint32_t getSlaveAddress() { return slaveAddress; }
    uint32_t getBusSpeed() { return busSpeed; }
    uint32_t getOffset() { return offset; }
    uint32_t getOffsetFieldSize() { return offsetFieldSize; }
    uint32_t getLength() { return length; }
    uint32_t getI2cFlags() { return i2cFlags; }
    uint64_t getAddress() { return address; }
    const ecmdDataBuffer * getData() { return (data.getBitLength() ? &data : NULL); }
    const ecmdDataBuffer * getMask() { return (mask.getBitLength() ? &mask : NULL); }
    const std::string & getDeviceString() { return deviceString; }
};

#endif // _TestableI2CInstruction_H

