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
/**
 * @file target.H
 * @brief platform specializations for fapi2 targets
 */

#ifndef __FAPI2_TARGET__
#define __FAPI2_TARGET__

#include <plat_target.H>
#include <fapi2_target.H>
#include <stdio.h>
#include <attribute_ids.H>
#include <plat_target_rules.H>

#include <fapi2Structs.H>
#include <fapi2SharedUtils.H>
#include <ecmdSharedUtils.H>
#include <ecmdStructs.H>
extern "C" {
    uint32_t fapi2GetAssociatedTargets(
        const ecmdChipTarget& i_target,
        const fapi2::TargetType i_associatedTargetType,
        std::list<const ecmdChipTarget*>& o_targets,
        const fapi2::TargetState i_state);
    uint32_t fapi2GetFilteredTargets(
        const ecmdChipTarget& i_target,
        const fapi2::TargetType i_associatedTargetType,
        std::list<const ecmdChipTarget*>& o_targets,
        const fapi2::TargetFilter i_filter,
        const fapi2::TargetState i_state);
    uint32_t fapi2GetAttribute(const ecmdChipTarget & i_target, const uint32_t i_id, fapi2::AttributeData & o_data);
    uint32_t fapi2GetMcastGroupTargets(
        const ecmdChipTarget& i_target,
        const fapi2::TargetType i_associatedTargetType,
        const uint32_t i_group,
        std::list<const ecmdChipTarget*>& o_targets,
        const fapi2::TargetState i_state);
    uint32_t fapi2GetTargetType(const ecmdChipTarget & i_target, fapi2::TargetType & o_targetType);
}


namespace fapi2
{

template<TargetType K, MulticastType M, typename V>
static constexpr void plat_apply_target_limits(void)
{
}

template<TargetType T, MulticastType M, typename V>
inline void Target<T, M, V>::mcUpdateHandle()
{
    iv_handle.multicast_type = M;
}

///
/// @brief Assignment Operator.
/// @param[in] i_right Reference to Target to assign from.
/// @return Reference to 'this' Target
///
template<TargetType K, MulticastType M, typename V>
Target<K, M, V>& Target<K, M, V>::operator=(const Target& i_right)
{
    iv_handle = i_right.iv_handle;
    return *this;
}

///
/// @brief Equality Comparison Operator
/// @param[in] i_right Reference to Target to compare.
/// @return bool. True if equal.
/// @note Platforms need to define this so that the physical
/// targets are determined to be equivilent rather than just the handles
///
template<TargetType K, MulticastType M, typename V>
bool Target<K, M, V>::operator==(const Target& i_right) const
{
    return iv_handle == i_right.iv_handle;
}

///
/// @brief Inquality Comparison Operator
/// @param[in] i_right Reference to Target to compare.
/// @return bool. True if not equal.
/// @note Platforms need to define this so that the physical
/// targets are determined to be equivilent rather than just the handles
///
template<TargetType K, MulticastType M, typename V>
bool Target<K, M, V>::operator!=(const Target& i_right) const
{
    return iv_handle != i_right.iv_handle;
}

///
/// @brief Less Than Comparison Operator
/// @param[in] i_right Reference to Target to compare.
/// @return bool. True if less than i_right.
/// @note Platforms need to define this so that the physical
/// targets are determined to be less than rather than just the handles
///
template<TargetType K, MulticastType M, typename V>
bool Target<K, M, V>::operator<(const Target& i_right) const
{
    return iv_handle < i_right.iv_handle;
}

///
/// @brief Get this target's immediate parent
/// @tparam T The type of the parent
/// @return Target<T> a target representing the parent
///
template<TargetType K, MulticastType M, typename V>
template<TargetType T>
inline Target<T, M, V> Target<K, M, V>::getParent(void) const
{
    fapi2plat::_check_valid_multicast_parent<K, T>();
    fapi2plat::_check_valid_unicast_parent<K, T>();

    plat_target_handle_t l_handle = this->get();

    if ((T == (T | fapi2::TARGET_TYPE_MULTICAST)) &&
        (K == (K | fapi2::TARGET_TYPE_MULTICAST)) &&
        (l_handle.multicast == true ))
    {
        // strip off core select
        l_handle.multicast_group = static_cast<MulticastGroup>(static_cast<uint32_t>(l_handle.multicast_group) & 0xFFFFFF);
        return Target<T, M, V>(l_handle);
    }

    std::list<const ecmdChipTarget*> l_targets;
    uint32_t l_rc = 0;

    const ecmdChipTarget * l_pEcmdTarget = l_handle;
    l_rc = fapi2GetAssociatedTargets(*l_pEcmdTarget, T, l_targets, TARGET_STATE_FUNCTIONAL);

    if (l_rc)
    {
        // ERROR
        return Target<T, M, V>();
    }

    if (l_targets.empty())
    {
        // ERROR
        return Target<T, M, V>();
    }

    return Target<T, M, V>(plat_target_handle_t(l_targets.front()));
}

///
/// @brief Get this target's children
/// @tparam T The type of the parent
/// @param[in] i_state The desired TargetState of the children
/// @return std::vector<Target<T> > a vector of present/functional
/// children
/// @warning The children of EX's (cores) are expected to be returned
/// in order. That is, core 0 is std::vector[0].
///
template<TargetType K, MulticastType M, typename V>
template< TargetType T>
inline std::vector<Target<T, M, V> >
Target<K, M, V>::getChildren(const TargetState i_state) const
{
    fapi2plat::_check_valid_multicast_children<K, T>();
    fapi2plat::_check_valid_unicast_children<K, T>();

    std::list<const ecmdChipTarget*> l_targets;
    std::vector<Target<T, M, V> > l_children;

    plat_target_handle_t l_handle = this->get();

    if ((K & TARGET_TYPE_MULTICAST) && (l_handle.multicast == false))
    {
        // If this target is a unicast target in disguise, return just this target
        // with the multicast qualifier stripped.
        l_children.push_back(Target<T, M, V>(l_handle));
    }
    else
    {
        const ecmdChipTarget * l_pEcmdTarget = l_handle;
        uint32_t l_rc = 0;

        if ((K & TARGET_TYPE_MULTICAST) && (l_handle.multicast == true))
            l_rc = fapi2GetMcastGroupTargets(*l_pEcmdTarget, T, static_cast<uint32_t>(l_handle.multicast_group), l_targets, i_state);
        else
            l_rc = fapi2GetAssociatedTargets(*l_pEcmdTarget, T, l_targets, i_state);

        if (l_rc)
        {
            // ERROR
            return l_children;
        }

        if (l_targets.empty())
        {
            // ERROR
            return l_children;
        }

        auto l_target = l_targets.begin();

        while (l_target != l_targets.end())
        {
            l_children.push_back(Target<T, M, V>(plat_target_handle_t(*l_target)));
            l_target++;
        }
    }

    return l_children;
}

///
/// @brief Get this target's children, filtered
/// @tparam T The type of the parent
/// @param[in] i_filter The desired chiplet filter
/// @param[in] i_state The desired TargetState of the children
/// @return std::vector<Target<T> > a vector of present/functional
/// children
///
template<TargetType K, MulticastType M, typename V>
template< TargetType T>
inline std::vector<Target<T, M, V> >
Target<K, M, V>::getChildren(const TargetFilter i_filter,
    const TargetState i_state) const
{
    // multicast targets are not valid for this interface
    static_assert(!(K & TARGET_TYPE_MULTICAST),
                  "fapi2::TARGET_TYPE_MULTICAST not valid with filter getChildren");

    const fapi2::TargetType TARGET_TYPE_FILTER_CHIPLETS =
            fapi2::TARGET_TYPE_XBUS |
            fapi2::TARGET_TYPE_OBUS |
            fapi2::TARGET_TYPE_CAPP |
            fapi2::TARGET_TYPE_OBUS_BRICK |
            fapi2::TARGET_TYPE_MCBIST |
            fapi2::TARGET_TYPE_MCS |
            fapi2::TARGET_TYPE_MEM_PORT |
            fapi2::TARGET_TYPE_MCA |
            fapi2::TARGET_TYPE_MI |
            fapi2::TARGET_TYPE_DMI |
            fapi2::TARGET_TYPE_PEC |
            fapi2::TARGET_TYPE_PHB |
            fapi2::TARGET_TYPE_PERV |
            fapi2::TARGET_TYPE_CORE |
            fapi2::TARGET_TYPE_EX |
            fapi2::TARGET_TYPE_MC |
            fapi2::TARGET_TYPE_OMI |
            fapi2::TARGET_TYPE_OMIC |
            fapi2::TARGET_TYPE_MCC |
            fapi2::TARGET_TYPE_NMMU |
            fapi2::TARGET_TYPE_PAU |
            fapi2::TARGET_TYPE_IOHS |
            fapi2::TARGET_TYPE_IOLINK |
            fapi2::TARGET_TYPE_PAUC;

    static_assert( !((T & TARGET_TYPE_FILTER_CHIPLETS) == fapi2::TARGET_TYPE_NONE),
                   "not a valid chiplet for filtering" );

    std::list<const ecmdChipTarget*> l_targets;
    std::vector<Target<T, M, V> > l_children;

    plat_target_handle_t l_handle = this->get();
    const ecmdChipTarget * l_pEcmdTarget = l_handle;
    uint32_t l_rc = fapi2GetFilteredTargets(*l_pEcmdTarget, T, l_targets, i_filter, i_state);

    if (l_rc)
    {
        // ERROR
        return l_children;
    }

    if (l_targets.empty())
    {
        // ERROR
        return l_children;
    }

    auto l_target = l_targets.begin();

    while (l_target != l_targets.end())
    {
        l_children.push_back(Target<T, M, V>(plat_target_handle_t(*l_target)));
        l_target++;
    }

    return l_children;
}

template<TargetType K, MulticastType M, typename V>
template< TargetType T >
inline ReturnCodes
Target<K, M, V>::reduceType(Target<T, M, V>& o_target) const
{
    check_reduceType<T>();

    bool compatible = true;
    plat_target_handle_t l_handle = this->get();
    const ecmdChipTarget * l_pEcmdTarget = l_handle;
    fapi2::TargetType l_type = TARGET_TYPE_NONE;

    uint32_t l_rc = fapi2GetTargetType(*l_pEcmdTarget, l_type);
    if (l_rc)
    {
        compatible = false;
    }

    // Here you would check that the platform target's type is compatible with T,
    // also taking multicast into account (iff the plat target is a real multicast
    // target, T must include TARGET_TYPE_MULTICAST).

    if (((T & TARGET_TYPE_MULTICAST) == TARGET_TYPE_NONE) &&
        (l_handle.multicast == true))
    {
        compatible = false;
    }

    if (((l_type & T) == TARGET_TYPE_NONE) &&
        (l_handle.multicast == false))
    {
        compatible = false;
    }

    if (!compatible)
    {
        return FAPI2_RC_INVALID_PARAMETER;
    }

    o_target = Target < T, M, V > (l_handle);
    return FAPI2_RC_SUCCESS;
}

///
/// @brief Get a multicast target for a given chip
/// @tparam T The type of target to return; TARGET_TYPE_MULTICAST is added automatically
/// @tparam O The type of multicast read operation for the target; defaults to OR
/// @param[in] i_group The abstract multicast group the target should point to
/// @return The multicast target
///
/// This method is only applicable to chip-level targets.
/// If the requested multicast group cannot be mapped to an available HW multicast
/// group, a platform specific error will be thrown.
///
template<TargetType K, MulticastType M, typename V>
template< TargetType T, MulticastType O >
inline Target < T | TARGET_TYPE_MULTICAST, O, V >
Target<K, M, V>::getMulticast(const MulticastGroup i_group) const
{
    static_assert((K == TARGET_TYPE_PROC_CHIP), "Parent target must be the proc chip");
    static_assert((T & TARGET_TYPE_MULTICASTABLE) != 0,
                  "Requested target type must be part of TARGET_TYPE_MULTICASTABLE");
    static_assert(bitCount<T>::count == 1,
                  "Requested target type must be a single type, TARGET_TYPE_MULTICAST will be added automatically");

    plat_target_handle_t l_handle = this->get();
    l_handle.multicast = true;
    l_handle.multicast_group = i_group;

    // FIXME check if group is valid

    Target <T | TARGET_TYPE_MULTICAST, O, V> t(l_handle);
    return t;
}

///
/// @brief Get a multicast core target for a given chip
/// @tparam O The type of multicast read operation for the target; defaults to OR
/// @param[in] i_group The abstract multicast group (selecting EQs) the target should point to
/// @param[in] i_cores Which cores inside the selected EQ should be targeted
/// @return The multicast target
///
/// This method is only applicable to chip-level targets.
/// If the requested multicast group cannot be mapped to an available HW multicast
/// group, a platform specific error will be thrown.
///
template<TargetType K, MulticastType M, typename V>
template< MulticastType O >
inline Target < TARGET_TYPE_CORE | TARGET_TYPE_MULTICAST, O, V >
Target<K, M, V>::getMulticast(const MulticastGroup i_group, const MulticastCoreSelect i_cores) const
{
    static_assert((K == TARGET_TYPE_PROC_CHIP), "Parent target must be the proc chip");

    plat_target_handle_t l_handle = this->get();
    l_handle.multicast = true;
    l_handle.multicast_group = static_cast<MulticastGroup>(static_cast<uint32_t>(i_group) | (static_cast<uint32_t>(i_cores) << 24));

    // FIXME check if group is valid

    Target <TARGET_TYPE_CORE | TARGET_TYPE_MULTICAST, O, V> t(l_handle);
    return t;
}

///
/// @brief Get the target at the other end of a bus
/// @tparam T The type of the target on the other end
/// @param[out] o_target A target representing the thing on the other end
/// @param[in] i_state The desired TargetState of the other end
/// @return FAPI2_RC_SUCCESS if OK, platforms will return a non-success
/// ReturnCode in the event of failure
/// @note o_target is only valid if return is FAPI2_RC_SUCCESS
///

template<TargetType K, MulticastType M, typename V>
template<TargetType T>
inline fapi2::ReturnCodes
Target<K, M, V>::getOtherEnd(Target<T, M, V>& o_target, const TargetState i_state) const
{
    // FIXME add static asserts
    std::list<const ecmdChipTarget*> l_targets;

    plat_target_handle_t l_handle = this->get();
    const ecmdChipTarget * l_pEcmdTarget = l_handle;
    uint32_t l_rc = fapi2GetAssociatedTargets(*l_pEcmdTarget, T, l_targets, i_state);

    if ((l_rc) || (l_targets.empty()))
    {
        // ERROR
        return FAPI2_RC_FALSE;
    }

    o_target = Target<T, M, V>(l_targets.front());
    return FAPI2_RC_SUCCESS;
}

///
/// @brief Is the target functional?
/// @return true if target is functional, false if non-functional
///

template<TargetType K, MulticastType M, typename V>
inline bool
Target<K, M, V>::isFunctional(void) const
{
    // static target test
    static_assert( is_same<K, fapi2::ATTR_FUNCTIONAL_TargetType>(),
                   "fapi2::Target::isFunctional() not valid for this target type" );
    // return non-functional by default
    fapi2::ATTR_FUNCTIONAL_Type l_functional = fapi2::ENUM_ATTR_FUNCTIONAL_NON_FUNCTIONAL;
    // see if this target has a good ecmdChipTarget
    plat_target_handle_t l_handle = this->get();
    const ecmdChipTarget * l_pEcmdTarget = l_handle;
    if (l_pEcmdTarget != nullptr)
    {
        fapi2::AttributeData o_data;
        uint32_t l_ecmd_rc = 0;

        o_data.faValidMask = FAPI_ATTRIBUTE_TYPE_UINT8;
        l_ecmd_rc = fapi2GetAttribute(*l_pEcmdTarget, fapi2::ATTR_FUNCTIONAL, o_data); 
        if (l_ecmd_rc == 0)
        {
            l_functional = o_data.faUint8;
        }
    }
    return (l_functional == fapi2::ENUM_ATTR_FUNCTIONAL_FUNCTIONAL);
}

///
/// @brief Returns the chiplet number associated with the Target
/// @return The chiplet number for the Target. 0 is returned if the
/// Target does not have a chiplet number (for ex, the PROC_CHIP Target)
/// @note For logical targets such as the EX, the chiplet number of
/// their immediate parent chiplet is returned
///
template<TargetType K, MulticastType M, typename V>
inline uint8_t
Target<K, M, V>::getChipletNumber(void) const
{
    uint8_t l_pervasive_chipunit = 0;
    plat_target_handle_t l_handle = this->get();
    const ecmdChipTarget * l_pEcmdTarget = l_handle;

    // check if this is a PERV target
    if ((l_pEcmdTarget->chipUnitTypeState == ECMD_TARGET_FIELD_VALID) &&
        ((l_pEcmdTarget->chipUnitType == "perv") || (l_pEcmdTarget->chipUnitType == "pervasive")) &&
        (l_pEcmdTarget->chipUnitNumState == ECMD_TARGET_FIELD_VALID))
    {
        l_pervasive_chipunit = l_pEcmdTarget->chipUnitNum;
    }
    // check if we can find a perv target from this target
    else if ((l_pEcmdTarget->chipUnitTypeState == ECMD_TARGET_FIELD_VALID) &&
             ((l_pEcmdTarget->chipUnitType == "eq") ||
              (l_pEcmdTarget->chipUnitType == "c") ||
              (l_pEcmdTarget->chipUnitType == "xbus") ||
              (l_pEcmdTarget->chipUnitType == "obus") ||
              (l_pEcmdTarget->chipUnitType == "obrick") ||
              (l_pEcmdTarget->chipUnitType == "capp") ||
              (l_pEcmdTarget->chipUnitType == "nv") ||
              (l_pEcmdTarget->chipUnitType == "mcbist") ||
              (l_pEcmdTarget->chipUnitType == "mcs") ||
              (l_pEcmdTarget->chipUnitType == "mca") ||
              (l_pEcmdTarget->chipUnitType == "mi") ||
              (l_pEcmdTarget->chipUnitType == "dmi") ||
              (l_pEcmdTarget->chipUnitType == "pec")  ||
              (l_pEcmdTarget->chipUnitType == "phb") ||
              (l_pEcmdTarget->chipUnitType == "mc") ||
              (l_pEcmdTarget->chipUnitType == "omi") ||
              (l_pEcmdTarget->chipUnitType == "omic") ||
              (l_pEcmdTarget->chipUnitType == "mcc") ||
              (l_pEcmdTarget->chipUnitType == "nmmu") ||
              (l_pEcmdTarget->chipUnitType == "pau") ||
              (l_pEcmdTarget->chipUnitType == "iohs") ||
              (l_pEcmdTarget->chipUnitType == "fc") ||
              (l_pEcmdTarget->chipUnitType == "pauc") ||
              (l_pEcmdTarget->chipUnitType == "mp") ||
#ifdef FAPI_2_Z
              (l_pEcmdTarget->chipUnitType == "nest") ||
              (l_pEcmdTarget->chipUnitType == "abus") ||
              (l_pEcmdTarget->chipUnitType == "core") ||
              (l_pEcmdTarget->chipUnitType == "pcie") ||
              (l_pEcmdTarget->chipUnitType == "ex") ||
#endif
              (l_pEcmdTarget->chipUnitType == "ppe")) &&
             (l_pEcmdTarget->chipUnitNumState == ECMD_TARGET_FIELD_VALID))
    {
        std::list<const ecmdChipTarget*> l_targets;
        uint32_t l_rc = fapi2GetAssociatedTargets(*l_pEcmdTarget, fapi2::TARGET_TYPE_PERV, l_targets, TARGET_STATE_FUNCTIONAL);
        if ((l_rc == 0) && (!l_targets.empty()))
        {
            l_pervasive_chipunit = l_targets.front()->chipUnitNum;
        }
    }
    // Special case for ex target 
    else if  ((l_pEcmdTarget->chipUnitTypeState == ECMD_TARGET_FIELD_VALID) &&
        (l_pEcmdTarget->chipUnitType == "ex") &&
        (l_pEcmdTarget->chipUnitNumState == ECMD_TARGET_FIELD_VALID))
    {
        std::list<const ecmdChipTarget*> l_eq_targets;
        uint32_t l_rc = fapi2GetAssociatedTargets(*l_pEcmdTarget, fapi2::TARGET_TYPE_EQ, l_eq_targets, TARGET_STATE_FUNCTIONAL);
        if ((l_rc == 0) && (!l_eq_targets.empty()))
        {
            std::list<const ecmdChipTarget*> l_targets;
            //auto l_eq_target = l_eq_targets.front();
            l_rc = fapi2GetAssociatedTargets(*(l_eq_targets.front()), fapi2::TARGET_TYPE_PERV, l_targets, TARGET_STATE_FUNCTIONAL);
            if ((l_rc == 0) && (!l_targets.empty()))
            {
                l_pervasive_chipunit = l_targets.front()->chipUnitNum;
            }

        }
    }

    return l_pervasive_chipunit;
}

#ifdef FAPI2_ENABLE_PLATFORM_GET_CORE_SELECT
inline MulticastCoreSelect
_getUnicastCoreSelectFromHandle(const plat_target_handle_t & i_handle)
{
    MulticastCoreSelect l_mcs = static_cast<MulticastCoreSelect>(0);

    const ecmdChipTarget * l_pEcmdTarget = i_handle;

    if (l_pEcmdTarget != nullptr)
    {
        switch (l_pEcmdTarget->chipUnitNum % 4)
        {
            case 0:
                l_mcs = MCCORE_0;
                break;
            case 1:
                l_mcs = MCCORE_1;
                break;
            case 2:
                l_mcs = MCCORE_2;
                break;
            case 3:
                l_mcs = MCCORE_3;
                break;
            default:
                break;
        }
    }

    return l_mcs;
}

template<TargetType K, MulticastType M, typename V>
inline MulticastCoreSelect
Target<K, M, V>::_getCoreSelect(void) const
{
    plat_target_handle_t l_handle = this->get();

    if (l_handle.multicast == true)
        return static_cast<MulticastCoreSelect>(static_cast<uint32_t>(l_handle.multicast_group) >> 24);
    else
        return _getUnicastCoreSelectFromHandle(l_handle);
}
#endif // FAPI2_ENABLE_PLATFORM_GET_CORE_SELECT

///
///
/// @brief Return the string interpretation of this target
/// @tparam T The type of the target
/// @param[in] i_target Target<T>
/// @param[in] i_buffer buffer to write in to
/// @param[in] i_bsize size of the buffer
/// @return void
/// @post The contents of the buffer is replaced with the string
/// representation of the target
///
template< TargetType T, MulticastType M, typename V >
inline void toString(const Target<T, M, V>& i_target, char* i_buffer, size_t i_bsize)
{
    ecmdChipTarget l_ecmdTarget;
    fapiTargetToEcmdTarget(i_target, l_ecmdTarget);
    std::string l_ecmdTargetString = ecmdWriteTarget(l_ecmdTarget, ECMD_DISPLAY_TARGET_HYBRID);
    snprintf(i_buffer, i_bsize, "%s", l_ecmdTargetString.c_str());
}

///
/// @brief Return the string interpretation of this target
/// @tparam T The type of the target
/// @tparam B The type of the buffer
/// @param[in] i_target A pointer to the Target<T>
/// @param[in] i_buffer buffer to write in to
/// @param[in] i_bsize  size of the buffer
/// @return void
/// @post The contents of the buffer is replaced with the string
/// representation of the target
///
template< TargetType T, MulticastType M, typename V >
inline void toString(const Target<T, M, V>* i_target, char* i_buffer, size_t i_bsize)
{
    ecmdChipTarget l_ecmdTarget;
    fapiTargetPointerToEcmdTarget(i_target, l_ecmdTarget);
    std::string l_ecmdTargetString = ecmdWriteTarget(l_ecmdTarget, ECMD_DISPLAY_TARGET_HYBRID);
    snprintf(i_buffer, i_bsize, "%s", l_ecmdTargetString.c_str());
}

///
/// @brief Get an enumerated target of a specific type
/// @tparam T The type of the target
/// @param[in] Ordinal representing the ordinal number of
/// the desired target
/// @return Target<T> the target requested
///
template<TargetType T, MulticastType M, typename V>
inline Target<T, M, V> getTarget(uint64_t Ordinal)
{
    // For testing
    return Target<T, M, V>(NULL);
}

template<>
inline Target<fapi2::TARGET_TYPE_PROC_CHIP, fapi2::MULTICAST_OR, fapi2::plat_target_handle_t> getTarget(uint64_t Ordinal)
{
    fapi2::Target<fapi2::TARGET_TYPE_SYSTEM, fapi2::MULTICAST_OR, fapi2::plat_target_handle_t> l_system;
    ecmdChipTarget l_ecmdTarget;
    fapiTargetToEcmdTarget(l_system, l_ecmdTarget);

    std::list<const ecmdChipTarget*> l_targets;
    uint32_t l_ecmd_rc = fapi2GetAssociatedTargets(l_ecmdTarget, fapi2::TARGET_TYPE_PROC_CHIP, l_targets, TARGET_STATE_PRESENT);

    if ((l_ecmd_rc) || (l_targets.empty()))
    {
        // ERROR
        return Target<fapi2::TARGET_TYPE_PROC_CHIP, fapi2::MULTICAST_OR, fapi2::plat_target_handle_t>(NULL);
    }

    // search fapi positions
    std::list<const ecmdChipTarget*> l_matching_targets;
    uint32_t l_matches = 0;
    for (auto l_target : l_targets)
    {
        fapi2::AttributeData o_data;

        o_data.faValidMask = FAPI_ATTRIBUTE_TYPE_UINT32;
        l_ecmd_rc = fapi2GetAttribute(*l_target, fapi2::ATTR_FAPI_POS, o_data); 
        if ((l_ecmd_rc == 0) && (Ordinal == o_data.faUint32))
        {
            l_matching_targets.push_back(l_target);
            l_matches++;
        }
    }

    if ((l_matches > 1) || (l_matches == 0))
    {
        // ERROR
        return Target<fapi2::TARGET_TYPE_PROC_CHIP, fapi2::MULTICAST_OR, fapi2::plat_target_handle_t>(NULL);
    }

    return Target<fapi2::TARGET_TYPE_PROC_CHIP, fapi2::MULTICAST_OR, fapi2::plat_target_handle_t>(l_matching_targets.front());
}

#ifdef FAPI2_ENABLE_PLATFORM_GET_TARGET
template<TargetType T, MulticastType M=fapi2::MULTICAST_OR, typename V=fapi2::plat_target_handle_t>
inline Target<T, M, V>* getTarget(TargetType i_type, uint8_t instance)
{
    fapi2::Target<fapi2::TARGET_TYPE_SYSTEM, M, V> l_system;
    ecmdChipTarget l_ecmdTarget;
    fapiTargetToEcmdTarget(l_system, l_ecmdTarget);

    std::list<const ecmdChipTarget*> l_targets;
    uint32_t l_ecmd_rc = fapi2GetAssociatedTargets(l_ecmdTarget, i_type, l_targets, TARGET_STATE_PRESENT);

    if ((l_ecmd_rc) || (l_targets.empty()))
    {
        // ERROR
        return nullptr;
    }

    //printf("getTarget, %08X, %d\n", i_type, instance);
    // There is no ATTR_FAPI_POS for the system target
    if (i_type == fapi2::TARGET_TYPE_SYSTEM)
    {
        return new Target<T, M, V>(l_targets.front());
    }

    // search fapi positions
    std::list<const ecmdChipTarget*> l_matching_targets;
    uint32_t l_matches = 0;
    for (auto l_target : l_targets)
    {
        fapi2::AttributeData o_data;

        o_data.faValidMask = FAPI_ATTRIBUTE_TYPE_UINT32;
        l_ecmd_rc = fapi2GetAttribute(*l_target, fapi2::ATTR_FAPI_POS, o_data); 
        if ((l_ecmd_rc == 0) && (instance == o_data.faUint32))
        {
            l_matching_targets.push_back(l_target);
            l_matches++;
        }
    }

    if ((l_matches > 1) || (l_matches == 0))
    {
        // ERROR
        return nullptr;
    }

    return new Target<T, M, V>(l_matching_targets.front());
}
#endif // FAPI2_ENABLE_PLATFORM_GET_TARGET

///
/// @brief Return the string interpretation of this target
/// @tparam T The type of the target
/// @tparam B The type of the buffer
/// @param[in] i_target Target<T>
/// @param[in] i_buffer buffer
/// @return void
/// @post The contents of the buffer is replaced with the string
/// representation of the target
///
template<TargetType T, MulticastType M, typename V, typename B>
inline void toString(const Target<T, M, V>& i_target, B& i_buffer)
{
}

}

#endif
