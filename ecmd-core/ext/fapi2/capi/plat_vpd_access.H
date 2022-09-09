/* IBM_PROLOG_BEGIN_TAG                                                   */
/* 
 * Copyright 2017,2022 IBM International Business Machines Corp.
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
/* IBM_PROLOG_END_TAG                                                     */
/// @file plat_vpd_access.H
///
/// @brief Platform vpd-access definitions
///

#ifndef _FAPI2_PLAT_VPDACCESS_H_
#define _FAPI2_PLAT_VPDACCESS_H_

#include <fapi2_vpd_access.H>

namespace fapi2plat
{
    fapi2::ReturnCode getVPD(const ecmdChipTarget& i_target,
                             const uint32_t i_version,
                             void * io_vpd_info,
                             uint8_t* o_blob);
};

namespace fapi2
{

    template<TargetType T, MulticastType M, typename V>
    inline ReturnCode platGetVPD(const Target<T, M, V>& i_target,
                                 VPDInfo<T>& io_vpd_info,
                                 uint8_t* o_blob)
    {
        static_assert((T == TARGET_TYPE_MCS) || (T == TARGET_TYPE_OCMB_CHIP),
                      "getVPD requires TARGET_TYPE_MCS or TARGET_TYPE_OCMB_CHIP");

        ecmdChipTarget ecmdTarget;
        fapiTargetToEcmdTarget(i_target, ecmdTarget);
	    uint32_t l_version = 2;

        if (T == TARGET_TYPE_OCMB_CHIP)
            l_version = 4;
#if 0
///
/// version 1 of MCS VPDInfo
///
template<>
class VPDInfo<TARGET_TYPE_MCS>
{
    public:
        // @brief VPDInfo constructor
        VPDInfo( const fapi2::MemVpdData& i_vpd_type)
            : iv_vpd_type(i_vpd_type),
              iv_size(0), iv_freq(0), iv_rank_count_dimm_0(0),
              iv_rank_count_dimm_1(0), iv_dimm_drop_per_port(0)
        {};

        // type of vpd field to return
        fapi2::MemVpdData_t iv_vpd_type;

        // size of the vpd data
        size_t   iv_size;
        uint64_t iv_freq;
        uint64_t iv_rank_count_dimm_0;
        uint64_t iv_rank_count_dimm_1;
        uint64_t iv_dimm_drop_per_port;
};
///
/// version 2 of MCS VPDInfo
///
template<>
class VPDInfo<TARGET_TYPE_MCS>
{
    public:
        // @brief VPDInfo constructor
        VPDInfo( const fapi2::MemVpdData& i_vpd_type)
            : iv_vpd_type(i_vpd_type),
              iv_size(VPD_INFO_INVALID),
              iv_freq_mhz(VPD_INFO_INVALID),
              iv_rank_count_dimm_0(VPD_INFO_INVALID),
              iv_rank_count_dimm_1(VPD_INFO_INVALID),
              iv_is_config_ffdc_enabled(true)
        {};
        // type of vpd field to return
        fapi2::MemVpdData_t iv_vpd_type;

        // size of the vpd data
        size_t   iv_size;
        uint64_t iv_freq_mhz;
        uint64_t iv_rank_count_dimm_0;
        uint64_t iv_rank_count_dimm_1;
        bool     iv_is_config_ffdc_enabled;

};
///
/// version 3 of OCMB VPDInfo
///
template<>
class VPDInfo<TARGET_TYPE_OCMB_CHIP>
{
    public:
        /// @brief VPDInfo constructor
        /// @param[in]  Type of VPD data to return
        VPDInfo( const fapi2::MemVpdData& i_vpd_type)
            : iv_vpd_type(i_vpd_type),
              iv_size(VPD_INFO_INVALID_64),
              iv_omi_freq_mhz(VPD_INFO_INVALID_64),
              iv_rank(VPD_INFO_INVALID_64),
              iv_is_config_ffdc_enabled(true),
              iv_efd_type(VPD_INFO_INVALID_8),
              iv_dmb_mfg_id(VPD_INFO_INVALID_16),
              iv_dmb_revision(VPD_INFO_INVALID_8),
              iv_ddr_mode(VPD_INFO_INVALID_8)
        {};

        // *** INPUT DATA ***

        // type of vpd field to return
        fapi2::MemVpdData_t iv_vpd_type;

        // size of the vpd data in bytes
        size_t   iv_size;

        // frequency of attached OMI bus
        uint64_t iv_omi_freq_mhz;

        // rank for which settings are valid
        uint64_t iv_rank;

        // set to false to avoid collecting a real ReturnCode
        bool     iv_is_config_ffdc_enabled;


        // *** OUTPUT DATA ***

        // metadata describing the EFD data that was returned
        uint8_t iv_efd_type;     // e.g. byte 288 of DDIMM SPD
        uint16_t iv_dmb_mfg_id;  // buffer manufacturer
        uint8_t iv_dmb_revision; // buffer revision
        uint8_t iv_ddr_mode;     // DDR4 or DDR5
};

///
/// version 4 of OCMB VPDInfo
///
template<>
class VPDInfo<TARGET_TYPE_OCMB_CHIP>
{
    public:
        /// @brief VPDInfo constructor
        /// @param[in]  Type of VPD data to return
        VPDInfo( const fapi2::MemVpdData& i_vpd_type)
            : iv_vpd_type(i_vpd_type),
              iv_size(VPD_INFO_INVALID_64),
              iv_omi_freq_mhz(VPD_INFO_INVALID_64),
              iv_rank(VPD_INFO_INVALID_64),
              iv_dimm_count(VPD_INFO_INVALID_64),
              iv_total_ranks_dimm0(VPD_INFO_INVALID_64),
              iv_total_ranks_dimm1(VPD_INFO_INVALID_64),
              iv_dimm_type(VPD_INFO_INVALID_64),
              iv_is_config_ffdc_enabled(true),
              iv_efd_type(VPD_INFO_INVALID_8),
              iv_dmb_mfg_id(VPD_INFO_INVALID_16),
              iv_dmb_revision(VPD_INFO_INVALID_8),
              iv_ddr_mode(VPD_INFO_INVALID_8)
        {};

        // *** INPUT DATA ***

        // type of vpd field to return
        fapi2::MemVpdData_t iv_vpd_type;

        // size of the vpd data in bytes
        size_t   iv_size;

        // frequency of attached OMI bus
        uint64_t iv_omi_freq_mhz;

        // rank for which settings are valid
        uint64_t iv_rank;

        // total number of DIMMs attached
        uint64_t iv_dimm_count;

        // total number of ranks on each of the DIMMs attached
        uint64_t iv_total_ranks_dimm0;
        uint64_t iv_total_ranks_dimm1;

        // DIMM type encoding, matches 'DIMMs Supported' byte 7 in EFD:
        // (bits numbered right to left)
        // bit7: reserved, code as 0
        // bit6: '1' if the DIMM is 3DS
        // bit5: '1' if the DIMM supports DDP
        // bit4: '1' if the DIMM supports quad rank mode
        // bits3,2: b00 for UDIMM, b01 for RDIMM
        // bit1: '1' for DIMM in slot 1
        // bit0: '1' for DIMM in slot 0
        uint64_t iv_dimm_type;

        // set to false to avoid collecting a real ReturnCode
        bool     iv_is_config_ffdc_enabled;


        // *** OUTPUT DATA ***

        // metadata describing the EFD data that was returned
        uint8_t iv_efd_type;     // e.g. byte 288 of DDIMM SPD
        uint16_t iv_dmb_mfg_id;  // buffer manufacturer
        uint8_t iv_dmb_revision; // buffer revision
        uint8_t iv_ddr_mode;     // DDR4 or DDR5
};
#endif
        return fapi2plat::getVPD(ecmdTarget, l_version, static_cast<void *>(&io_vpd_info), o_blob);
    }

};
#endif // _FAPI2_PLAT_VPDACCESS_H_
