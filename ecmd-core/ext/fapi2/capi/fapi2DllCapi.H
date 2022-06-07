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

#ifndef fapi2DllCapi_H
#define fapi2DllCapi_H

#include <inttypes.h>
#include <vector>
#include <string>

#include <ecmdDefines.H>
#include <ecmdStructs.H>
#include <ecmdReturnCodes.H>
#include <ecmdDataBufferBase.H>

#include <target_states.H>
//#include <hw_access_def.H>
#include <error_info_defs.H>
#include <return_code.H>

#include <fapi2Structs.H>
#include <attribute_ids.H>
#include <mvpd_access_defs.H>
#include <fapi2_mbvpd_access_defs.H>
//#include <fapi2Util.H>

extern "C" {

/* Extension initialization function - verifies version */
uint32_t dllFapi2InitExtension(const char * i_version);

/* Extension Specific load function - used by Cronus/GFW to see if extension is supported */
uint32_t dllFapi2InitExtensionInPlugin();

uint32_t dllFapi2GetSpdBlob(
	const ecmdChipTarget & i_target,
	uint8_t * o_blob,
	size_t & s); 

uint32_t dllFapi2GetVpdBlob(
	const ecmdChipTarget & i_target,
	const uint32_t i_version, 
	void * io_vpd_info, 
	uint8_t * o_blob); 

uint32_t dllFapi2GetRing(
	const ecmdChipTarget & i_target,
	const uint64_t i_address, 
	ecmdDataBufferBase & o_data, 
	const uint32_t i_ringMode); 

uint32_t dllFapi2PutRingByID(
    const ecmdChipTarget & i_target,
    const uint32_t i_ringID,
    const uint32_t i_ringMode);

uint32_t dllFapi2PutRingByName(
    const ecmdChipTarget & i_target,
    const char* i_ringID,
    const uint32_t i_ringMode);

uint32_t dllFapi2PutRing(
	const ecmdChipTarget & i_target, 
	const uint64_t i_address, 
	ecmdDataBufferBase & i_data, 
	const uint32_t i_ringMode); 

uint32_t dllFapi2QueryFileLocation(
	fapi2::FileType_t i_fileType, 
	std::string & i_fileName, 
	std::string & o_fileLocation, 
	std::string i_version);

uint32_t dllFapi2GetAssociatedDimms(
	const ecmdChipTarget & i_target, 
	std::list<const ecmdChipTarget *> & o_dimms, 
	const fapi2::TargetState i_state);

uint32_t dllFapi2GetAttributeOverride(
	const ecmdChipTarget & i_target, 
	const uint32_t i_id, 
	fapi2::AttributeData & o_data);

uint32_t dllFapi2GetAttribute(
	const ecmdChipTarget & i_target, 
	const uint32_t i_id, 
	fapi2::AttributeData & o_data);

uint32_t dllFapi2SetAttribute(
	const ecmdChipTarget & i_target, 
	const uint32_t i_id, 
	fapi2::AttributeData & i_data);

uint32_t dllFapi2AttributeDataToString(
	fapi2::AttributeId i_attrId, 
	const fapi2::AttributeData & i_attrData, 
	std::string & o_attrDataString, 
	bool i_fullData, 
	const char * i_format);

uint32_t dllFapi2AttrEnumStrToEnumVal(fapi2::AttributeData & io_attrData);

uint32_t dllFapi2AttributeStringToId(
	std::string i_attrString, 
	fapi2::AttributeId & o_attrId);

uint32_t dllFapi2GetAttrInfo(
	fapi2::AttributeId i_attrId, 
	uint32_t & o_attrType, 
	uint32_t & o_numOfEntries, 
	uint32_t & o_numOfBytes, 
	bool & o_attrEnum);

uint32_t dllFapi2GetAttrIdsByType(
	fapi2::TargetType i_targetTypes, 
	uint32_t i_attributeSources, 
	std::list<fapi2::AttributeId> & o_attributeIds);

void dllFapi2OutputError(const char* i_message); 

void dllFapi2OutputInfo(const char* i_message); 

void dllFapi2OutputImportant(const char* i_message); 

void dllFapi2OutputDebug(const char* i_message); 

void dllFapi2OutputScanTrace(const char* i_message); 

void dllFapi2OutputManufacturing(const char* i_message); 

char* dllFapi2FixOutputFormat(char* o_message, const char* i_message, size_t i_num); 

uint32_t dllFapi2Delay(uint64_t i_nanoSeconds, uint64_t i_simCycles, bool i_fixed); 

bool dllFapi2PlatIsScanTraceEnabled(); 

void dllFapi2PlatSetScanTrace(bool i_enable);

void dllFapi2LogError(fapi2::ReturnCode & io_rc,
                     fapi2::errlSeverity_t i_sev,
                     bool i_unitTestError);

#if 0

fapi2::ReturnCode dllFapi2LoadInitFile(
	const fapi2::Target<fapi2::TARGET_TYPE_ALL_CHIP> &, 
	const char * i_file, 
	const char *& o_addr, 
	size_t & o_size);

fapi2::ReturnCode dllFapi2UnloadInitFile(
	const char * i_file, 
	const char *& io_addr, 
	size_t & io_size);
#endif

uint32_t dllFapi2GetMvpdField(
	const fapi2::MvpdRecord i_record,
	const fapi2::MvpdKeyword i_keyword,
	const ecmdChipTarget & i_target,
	uint8_t * const i_pBuffer,
	uint32_t & io_fieldSize);

uint32_t dllFapi2SetMvpdField(
	const fapi2::MvpdRecord i_record,
	const fapi2::MvpdKeyword i_keyword,
	const ecmdChipTarget & i_target,
	const uint8_t * const i_pBuffer,
	const uint32_t i_fieldSize);

uint32_t dllFapi2SpecialWakeup(
	const ecmdChipTarget & i_target, 
	const bool i_enable);

#if 0
fapi2::ReturnCode dllFapi2GetOtherSideOfMemChannel(
	const fapi2::Target<fapi2::TARGET_TYPE_MEMBUF_CHIP | fapi2::TARGET_TYPE_PROC_CHIP> & i_target, 
	fapi2::Target<fapi2::TARGET_TYPE_MEMBUF_CHIP | fapi2::TARGET_TYPE_PROC_CHIP> & o_target, 
	const fapi2::TargetState i_state);

#endif

uint32_t dllFapi2GetMBvpdField(
	const fapi2::MBvpdRecord i_record,
	const fapi2::MBvpdKeyword i_keyword,
	const ecmdChipTarget & i_target,
	uint8_t * const i_pBuffer,
	size_t &io_fieldSize);

uint32_t dllFapi2SetMBvpdField(
	const fapi2::MBvpdRecord i_record,
	const fapi2::MBvpdKeyword i_keyword,
	const ecmdChipTarget & i_target,
	const uint8_t * const i_pBuffer,
	const size_t i_fieldSize);

bool dllFapi2IsOutputInfoEnabled(); 

bool dllFapi2IsOutputDebugEnabled(); 

bool dllFapi2IsOutputManufacturingEnabled(); 

// general interface for child/parent/on-chip-peer discovery
uint32_t dllFapi2GetAssociatedTargets(
	const ecmdChipTarget & i_target, 
	const fapi2::TargetType i_associatedTargetType, 
	std::list<const ecmdChipTarget *> & o_targets, 
	const fapi2::TargetState i_state); 

uint32_t dllFapi2GetFilteredTargets(
    const ecmdChipTarget& i_target,
    const fapi2::TargetType i_associatedTargetType,
    std::list<const ecmdChipTarget*>& o_targets,
    const fapi2::TargetFilter i_filter,
    const fapi2::TargetState i_state);

uint32_t dllFapi2GetTargetType(const ecmdChipTarget & i_target, fapi2::TargetType & o_targetType);

bool dllFapi2IsOutputLabEnabled(); 

void dllFapi2OutputLab(const char* i_message); 

uint32_t dllFapi2GeneralApi(std::list<void *> & io_args);

uint32_t dllFapi2GetI2c(const ecmdChipTarget& i_target,
                        const size_t i_get_size,
                        const std::vector<uint8_t>& i_data,
                        std::vector<uint8_t>& o_data);

uint32_t dllFapi2PutI2c(const ecmdChipTarget& i_target,
                        const std::vector<uint8_t>& i_data);

uint32_t dllFapi2GetMMIO(const ecmdChipTarget& i_target,
                         const uint64_t i_mmioAddr,
                         const size_t i_transSize,
                         std::vector<uint8_t>& o_data);

uint32_t dllFapi2PutMMIO(const ecmdChipTarget& i_target,
                         const uint64_t i_mmioAddr,
                         const size_t i_transSize,
                         const std::vector<uint8_t>& i_data);

uint32_t dllFapi2SetMcastGroupMap(const ecmdChipTarget & i_target,
                                  const std::vector< fapi2::MulticastGroupMapping > & i_mappings);

uint32_t dllFapi2GetScomMulticast(const ecmdChipTarget& i_target,
                                  const uint32_t i_type,
                                  const uint32_t i_group,
                                  const uint64_t i_address,
                                  ecmdDataBuffer& o_data);

uint32_t dllFapi2PutScomMulticast(const ecmdChipTarget& i_target,
                                  const uint32_t i_type,
                                  const uint32_t i_group,
                                  const uint64_t i_address,
                                  const ecmdDataBuffer& i_data);

uint32_t dllFapi2GetMcastGroupTargets(const ecmdChipTarget& i_target,
                                      const fapi2::TargetType i_associatedTargetType,
                                      const uint32_t i_group,
                                      std::list<const ecmdChipTarget*>& o_targets,
                                      const fapi2::TargetState i_state);
} //extern C

#endif
