/* IBM_PROLOG_BEGIN_TAG                                                   */
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
/* IBM_PROLOG_END_TAG                                                     */

/// @file plat_hw_access.H
///
/// @brief Platform hardware-access definitions
///

#ifndef _FAPI2_PLAT_HWACCESS_H_
#define _FAPI2_PLAT_HWACCESS_H_

#include <hw_access_def.H>
#include <stdint.h>
#include <thread>
#include <buffer.H>
#include <variable_buffer.H>
#include <return_code.H>
#include <target.H>

namespace fapi2plat
{
    fapi2::ReturnCode setMulticastGroupMap(const ecmdChipTarget & i_target,
                                           const std::vector< fapi2::MulticastGroupMapping > & i_mappings);

    fapi2::ReturnCode getScomMulticast(const ecmdChipTarget& i_target,
                                       const uint32_t i_type,
                                       const uint32_t i_group,
                                       const uint64_t i_address,
                                       fapi2::buffer<uint64_t>& o_data);

    fapi2::ReturnCode putScomMulticast(const ecmdChipTarget & i_target,
                                       const uint32_t i_type,
                                       const uint32_t i_group,
                                       const uint64_t i_address,
                                       const fapi2::buffer<uint64_t> i_data);

    fapi2::ReturnCode getScom(const ecmdChipTarget& i_target,
                              const uint64_t i_address,
                              fapi2::buffer<uint64_t>& o_data);

    fapi2::ReturnCode putScom(const ecmdChipTarget & i_target,
                              const uint64_t i_address,
                              const fapi2::buffer<uint64_t> i_data);

    fapi2::ReturnCode putScomUnderMask(const ecmdChipTarget & i_target,
                                       const uint64_t i_address,
                                       const fapi2::buffer<uint64_t> i_data,
                                       const fapi2::buffer<uint64_t> i_mask);

    fapi2::ReturnCode getCfamRegister(const ecmdChipTarget& i_target,
                                      const uint32_t i_address,
                                      fapi2::buffer<uint32_t>& o_data);

    fapi2::ReturnCode putCfamRegister(const ecmdChipTarget& i_target,
                                      const uint32_t i_address,
                                      const fapi2::buffer<uint32_t> i_data);

    fapi2::ReturnCode getRing(const ecmdChipTarget & i_target,
                              const scanRingId_t i_address,
                              fapi2::variable_buffer & o_data,
                              const uint32_t i_ringMode);

    fapi2::ReturnCode putRing(const ecmdChipTarget & i_target,
                              const RingId_t i_ringID,
                              const fapi2::RingMode i_ringMode);

    fapi2::ReturnCode putRing(const ecmdChipTarget & i_target,
                              const char* i_ringID,
                              const fapi2::RingMode i_ringMode);

    fapi2::ReturnCode plat_putRingImpl(const ecmdChipTarget & i_target,
                                       const void* i_scanImage,
                                       const uint32_t i_modifiedRingAddress,
                                       const fapi2::RingMode i_ringMode);

    fapi2::ReturnCode putRing(const ecmdChipTarget & i_target,
                              const scanRingId_t i_address,
                              const fapi2::variable_buffer & i_data,
                              const uint32_t i_ringMode);

    fapi2::ReturnCode getSpy(const ecmdChipTarget & i_target,
                             const char * const i_spyId,
                             fapi2::variable_buffer& o_data);

    fapi2::ReturnCode putSpy(const ecmdChipTarget & i_target,
                             const char * const i_spyId,
                             const fapi2::variable_buffer& i_data);

    fapi2::ReturnCode getSpyImage(const ecmdChipTarget & i_target,
                                  const char * const i_spyId,
                                  fapi2::variable_buffer& o_data,
                                  const fapi2::variable_buffer& i_imageData);

    fapi2::ReturnCode putSpyImage(const ecmdChipTarget & i_target,
                                  const char* const i_spyId,
                                  const fapi2::variable_buffer& i_data,
                                  fapi2::variable_buffer& o_imageData);

    const fapi2::TargetType TARGET_TYPE_SCOM_TARGET = fapi2::TARGET_TYPE_PROC_CHIP |
#ifdef FAPI_2_Z
        fapi2::TARGET_TYPE_SC_CHIP |
#endif
        fapi2::TARGET_TYPE_MEMBUF_CHIP |
        fapi2::TARGET_TYPE_OCMB_CHIP |
        fapi2::TARGET_TYPE_EX |
        fapi2::TARGET_TYPE_MBA |
        fapi2::TARGET_TYPE_MCS |
        fapi2::TARGET_TYPE_XBUS |
        fapi2::TARGET_TYPE_ABUS |
        fapi2::TARGET_TYPE_L4 |
        fapi2::TARGET_TYPE_CORE |
        fapi2::TARGET_TYPE_EQ |
        fapi2::TARGET_TYPE_MCA |
        fapi2::TARGET_TYPE_MCBIST |
        fapi2::TARGET_TYPE_MI |
        fapi2::TARGET_TYPE_DMI |
        fapi2::TARGET_TYPE_OBUS |
        fapi2::TARGET_TYPE_OBUS_BRICK |
        fapi2::TARGET_TYPE_SBE |
        fapi2::TARGET_TYPE_PPE |
        fapi2::TARGET_TYPE_PERV |
        fapi2::TARGET_TYPE_PEC |
        fapi2::TARGET_TYPE_PHB |
        fapi2::TARGET_TYPE_CAPP |
        fapi2::TARGET_TYPE_MC |
        fapi2::TARGET_TYPE_OMI |
        fapi2::TARGET_TYPE_OMIC |
        fapi2::TARGET_TYPE_MCC |
        fapi2::TARGET_TYPE_MEM_PORT |
        fapi2::TARGET_TYPE_NMMU |
        fapi2::TARGET_TYPE_PAU |
        fapi2::TARGET_TYPE_IOHS |
        fapi2::TARGET_TYPE_PMIC |
        fapi2::TARGET_TYPE_MDS_CTLR |
        fapi2::TARGET_TYPE_PAUC;

    const fapi2::TargetType TARGET_TYPE_CFAM_TARGET = fapi2::TARGET_TYPE_PROC_CHIP |
#ifdef FAPI_2_Z
        fapi2::TARGET_TYPE_SC_CHIP |
#endif
        fapi2::TARGET_TYPE_OCMB_CHIP |
        fapi2::TARGET_TYPE_MEMBUF_CHIP;

    const fapi2::TargetType TARGET_TYPE_SCAN_TARGET = fapi2::TARGET_TYPE_PROC_CHIP |
#ifdef FAPI_2_Z
        fapi2::TARGET_TYPE_SC_CHIP |
#endif
        fapi2::TARGET_TYPE_OCMB_CHIP |
        fapi2::TARGET_TYPE_MEMBUF_CHIP |
        fapi2::TARGET_TYPE_CORE |
        fapi2::TARGET_TYPE_EQ |
        fapi2::TARGET_TYPE_EX |
        fapi2::TARGET_TYPE_OBUS |
        fapi2::TARGET_TYPE_MCBIST |
        fapi2::TARGET_TYPE_MC |
        fapi2::TARGET_TYPE_PEC |
        fapi2::TARGET_TYPE_IOHS |
        fapi2::TARGET_TYPE_PERV |
        fapi2::TARGET_TYPE_MULTICAST;

    template< fapi2::TargetType K, fapi2::TargetType L >
    constexpr void checkValidAccess()
    {
        static_assert( ((K & L) != fapi2::TARGET_TYPE_NONE), "Invalid TargetType for HW access" );
    }

    template< fapi2::TargetType K, fapi2::MulticastType M, typename V >
    inline fapi2::ReturnCode setMulticastGroupMap(const fapi2::Target<K, M, V>& i_chip,
                                                  const std::vector< fapi2::MulticastGroupMapping >& i_mappings)
    {
        checkValidAccess<K, fapi2::TARGET_TYPE_MULTICAST_CHIP>();
        ecmdChipTarget ecmdTarget;
        fapiTargetToEcmdTarget(i_chip, ecmdTarget);

        return fapi2plat::setMulticastGroupMap(ecmdTarget, i_mappings);
    }

    template< fapi2::TargetType K, fapi2::MulticastType M, typename V >
    inline fapi2::ReturnCode getScom(const fapi2::Target<K, M, V>& i_target,
                                     const uint64_t i_address,
                                     fapi2::buffer<uint64_t>& o_data)
    {
        checkValidAccess<K, TARGET_TYPE_SCOM_TARGET>();
        ecmdChipTarget ecmdTarget;
        fapiTargetToEcmdTarget(i_target, ecmdTarget);

        const fapi2::plat_target_handle_t & l_handle = i_target.get();

        if (l_handle.multicast)
            return fapi2plat::getScomMulticast(ecmdTarget, static_cast<uint32_t>(M), static_cast<uint32_t>(l_handle.multicast_group), i_address, o_data);

        return fapi2plat::getScom(ecmdTarget, i_address, o_data);
    }

    template< fapi2::TargetType K, fapi2::MulticastType M, typename V >
    inline fapi2::ReturnCode putScom(const fapi2::Target<K, M, V>& i_target,
                                     const uint64_t i_address,
                                     const fapi2::buffer<uint64_t> i_data)
    {
        checkValidAccess<K, TARGET_TYPE_SCOM_TARGET>();
        ecmdChipTarget ecmdTarget;
        fapiTargetToEcmdTarget(i_target, ecmdTarget);

        const fapi2::plat_target_handle_t & l_handle = i_target.get();

        if (l_handle.multicast)
            return fapi2plat::putScomMulticast(ecmdTarget, static_cast<uint32_t>(M), static_cast<uint32_t>(l_handle.multicast_group), i_address, i_data);

        return fapi2plat::putScom(ecmdTarget, i_address, i_data);
    }

    template< fapi2::TargetType K, fapi2::MulticastType M, typename V >
    inline fapi2::ReturnCode putScomUnderMask(const fapi2::Target<K, M, V>& i_target,
                                              const uint64_t i_address,
                                              const fapi2::buffer<uint64_t> i_data,
                                              const fapi2::buffer<uint64_t> i_mask)
    {
        checkValidAccess<K, TARGET_TYPE_SCOM_TARGET>();
        ecmdChipTarget ecmdTarget;
        fapiTargetToEcmdTarget(i_target, ecmdTarget);
        return fapi2plat::putScomUnderMask(ecmdTarget, i_address, i_data, i_mask);
    }

    template< fapi2::TargetType K, fapi2::MulticastType M, typename V >
    inline fapi2::ReturnCode getCfamRegister(const fapi2::Target<K, M, V>& i_target,
                                             const uint32_t i_address,
                                             fapi2::buffer<uint32_t>& o_data)
    {
        checkValidAccess<K, TARGET_TYPE_CFAM_TARGET>();
        ecmdChipTarget ecmdTarget;
        fapiTargetToEcmdTarget(i_target, ecmdTarget); 
        return fapi2plat::getCfamRegister(ecmdTarget, i_address, o_data);
    }

    template< fapi2::TargetType K, fapi2::MulticastType M, typename V >
    inline fapi2::ReturnCode putCfamRegister(const fapi2::Target<K, M, V>& i_target,
                                             const uint32_t i_address,
                                             const fapi2::buffer<uint32_t> i_data)
    {
        checkValidAccess<K, TARGET_TYPE_CFAM_TARGET>();
        ecmdChipTarget ecmdTarget;
        fapiTargetToEcmdTarget(i_target, ecmdTarget); 
        return fapi2plat::putCfamRegister(ecmdTarget, i_address, i_data);
    }

    template< fapi2::TargetType K, fapi2::MulticastType M, typename V >
    inline fapi2::ReturnCode getRing(const fapi2::Target<K, M, V>& i_target,
                                     const scanRingId_t i_address,
                                     fapi2::variable_buffer& o_data,
                                     const fapi2::RingMode i_ringMode)
    {
        checkValidAccess<K, TARGET_TYPE_SCAN_TARGET>();
        ecmdChipTarget ecmdTarget;
        fapiTargetToEcmdTarget(i_target, ecmdTarget);
        return fapi2plat::getRing(ecmdTarget, i_address, o_data, i_ringMode);
    }

    template< fapi2::TargetType K, fapi2::MulticastType M, typename V >
    fapi2::ReturnCode putRing(const fapi2::Target<K, M, V>& i_target,
                              const RingId_t i_ringID,
                              const fapi2::RingMode i_ringMode)
    {
        checkValidAccess<K, TARGET_TYPE_SCAN_TARGET>();
        ecmdChipTarget ecmdTarget;
        fapiTargetToEcmdTarget(i_target, ecmdTarget);
        fapi2::RingMode l_ringMode = i_ringMode;
        uint32_t l_mode = static_cast<uint32_t>(l_ringMode);
        const fapi2::plat_target_handle_t & l_handle = i_target.get();

        if (l_handle.multicast)
        {
            // Pass the encoded mcast group and core select within ring mode
            l_mode |= (static_cast<uint32_t>(0x000000FF & l_handle.multicast_group) << 16);
            l_mode |= (static_cast<uint32_t>(0x0F000000 & l_handle.multicast_group));           
            l_ringMode = static_cast<fapi2::RingMode>(l_mode);
        }
        return fapi2plat::putRing(ecmdTarget, i_ringID, l_ringMode);
    }

    template< fapi2::TargetType K, fapi2::MulticastType M, typename V >
    fapi2::ReturnCode putRing(const fapi2::Target<K, M, V>& i_target,
                              const char* i_ringID,
                              const fapi2::RingMode i_ringMode)
    {
        checkValidAccess<K, TARGET_TYPE_SCAN_TARGET>();
        ecmdChipTarget ecmdTarget;
        fapiTargetToEcmdTarget(i_target, ecmdTarget);
        fapi2::RingMode l_ringMode = i_ringMode;
        uint32_t l_mode = static_cast<uint32_t>(l_ringMode);
        const fapi2::plat_target_handle_t & l_handle = i_target.get();

        if (l_handle.multicast)
        {
            // Pass the encoded mcast group and core select within ring mode
            l_mode |= (static_cast<uint32_t>(0x000000FF & l_handle.multicast_group) << 16);
            l_mode |= (static_cast<uint32_t>(0x0F000000 & l_handle.multicast_group));           
            l_ringMode = static_cast<fapi2::RingMode>(l_mode);
        }
        return fapi2plat::putRing(ecmdTarget, i_ringID, l_ringMode);
    }

    template< fapi2::TargetType K, fapi2::MulticastType M, typename V >
    fapi2::ReturnCode plat_putRingImpl(const fapi2::Target<K, M, V>& i_target,
                                       const void* i_scanImage,
                                       const uint32_t i_modifiedRingAddress,
                                       const fapi2::RingMode i_ringMode)
    {
        checkValidAccess<K, TARGET_TYPE_SCAN_TARGET>();
        ecmdChipTarget ecmdTarget;
        fapiTargetToEcmdTarget(i_target, ecmdTarget);
        fapi2::RingMode l_ringMode = i_ringMode;
        uint32_t l_mode = static_cast<uint32_t>(l_ringMode);
        const fapi2::plat_target_handle_t & l_handle = i_target.get();

        if (l_handle.multicast)
        {
            // Pass the encoded mcast group and core select within ring mode
            l_mode |= (static_cast<uint32_t>(0x000000FF & l_handle.multicast_group) << 16);
            l_mode |= (static_cast<uint32_t>(0x0F000000 & l_handle.multicast_group));           
            l_ringMode = static_cast<fapi2::RingMode>(l_mode);
        }
        return fapi2plat::plat_putRingImpl(ecmdTarget, i_scanImage, i_modifiedRingAddress, l_ringMode);
    }

    template< fapi2::TargetType K, fapi2::MulticastType M, typename V >
    fapi2::ReturnCode putRing(const fapi2::Target<K, M, V>& i_target,
                              const scanRingId_t i_address,
                              const fapi2::variable_buffer& i_data,
                              const fapi2::RingMode i_ringMode)
    {
        checkValidAccess<K, TARGET_TYPE_SCAN_TARGET>();
        ecmdChipTarget ecmdTarget;
        fapiTargetToEcmdTarget(i_target, ecmdTarget);
        return fapi2plat::putRing(ecmdTarget, i_address, i_data, i_ringMode);
    }

    template< fapi2::TargetType K, fapi2::MulticastType M, typename V >
    fapi2::ReturnCode getSpy(const fapi2::Target<K, M, V>& i_target,
                             const char * const i_spyId,
                             fapi2::variable_buffer& o_data)
    {
        checkValidAccess<K, TARGET_TYPE_SCAN_TARGET>();
        ecmdChipTarget ecmdTarget;
        fapiTargetToEcmdTarget(i_target, ecmdTarget);
        return fapi2plat::getSpy(ecmdTarget, i_spyId, o_data);
    }

    template< fapi2::TargetType K, fapi2::MulticastType M, typename V >
    fapi2::ReturnCode putSpy(const fapi2::Target<K, M, V>& i_target,
                             const char * const i_spyId,
                             const fapi2::variable_buffer& i_data)
    {
        checkValidAccess<K, TARGET_TYPE_SCAN_TARGET>();
        ecmdChipTarget ecmdTarget;
        fapiTargetToEcmdTarget(i_target, ecmdTarget);
        return fapi2plat::putSpy(ecmdTarget, i_spyId, i_data);
    }

    template< fapi2::TargetType K, fapi2::MulticastType M, typename V >
    fapi2::ReturnCode getSpyImage(const fapi2::Target<K, M, V>& i_target,
                                  const char * const i_spyId,
                                  fapi2::variable_buffer& o_data,
                                  const fapi2::variable_buffer& i_imageData)
    {
        checkValidAccess<K, TARGET_TYPE_SCAN_TARGET>();
        ecmdChipTarget ecmdTarget;
        fapiTargetToEcmdTarget(i_target, ecmdTarget);
        return fapi2plat::getSpyImage(ecmdTarget, i_spyId, o_data, i_imageData);
    }

    template< fapi2::TargetType K, fapi2::MulticastType M, typename V >
    fapi2::ReturnCode putSpyImage(const fapi2::Target<K, M, V>& i_target,
                                  const char* const i_spyId,
                                  const fapi2::variable_buffer& i_data,
                                  fapi2::variable_buffer& o_imageData)
    {
        checkValidAccess<K, TARGET_TYPE_SCAN_TARGET>();
        ecmdChipTarget ecmdTarget;
        fapiTargetToEcmdTarget(i_target, ecmdTarget);
        return fapi2plat::putSpyImage(ecmdTarget, i_spyId, i_data, o_imageData);
    }
};

#endif // _FAPI2_PLAT_HWACCESS_H_
