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
#include <multicast.H>
#include <stdio.h>
#include <attribute_ids.H>

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
}

namespace fapi2
{
///
/// @brief Assignment Operator.
/// @param[in] i_right Reference to Target to assign from.
/// @return Reference to 'this' Target
///
template<TargetType K, typename V>
Target<K, V>& Target<K, V>::operator=(const Target& i_right)
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
template<TargetType K, typename V>
bool Target<K, V>::operator==(const Target& i_right) const
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
template<TargetType K, typename V>
bool Target<K, V>::operator!=(const Target& i_right) const
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
template<TargetType K, typename V>
bool Target<K, V>::operator<(const Target& i_right) const
{
    return iv_handle < i_right.iv_handle;
}

///
/// @brief Get this target's immediate parent
/// @tparam T The type of the parent
/// @return Target<T> a target representing the parent
///
template<TargetType K, typename V>
template<TargetType T>
inline Target<T, V> Target<K, V>::getParent(void) const
{
    // General compound chiplet check for proc parent

#ifdef FAPI_2_Z_TARGET_RULES
    const fapi2::TargetType TARGET_TYPE_EP_CHIPLETS =
            fapi2::TARGET_TYPE_NEST |
            fapi2::TARGET_TYPE_XBUS0 |
            fapi2::TARGET_TYPE_XBUS1 |
            fapi2::TARGET_TYPE_PERV |
            fapi2::TARGET_TYPE_GXA |
            fapi2::TARGET_TYPE_CORE |
            fapi2::TARGET_TYPE_MCS |
            fapi2::TARGET_TYPE_PCIE;

    static_assert( !((T == fapi2::TARGET_TYPE_EP_CHIP) &&
                     ((K & TARGET_TYPE_EP_CHIPLETS) == fapi2::TARGET_TYPE_NONE)),
                   "fapi2::TARGET_TYPE_EP_CHIP is not a valid parent" );

    const fapi2::TargetType TARGET_TYPE_SC_CHIPLETS =
            fapi2::TARGET_TYPE_ABUS |
            fapi2::TARGET_TYPE_NEST |
            fapi2::TARGET_TYPE_XBUS0 |
            fapi2::TARGET_TYPE_XBUS1 |
            fapi2::TARGET_TYPE_PERV;

    static_assert( !((T == fapi2::TARGET_TYPE_SC_CHIP) &&
                     ((K & TARGET_TYPE_SC_CHIPLETS) == fapi2::TARGET_TYPE_NONE)),
                   "fapi2::TARGET_TYPE_SC_CHIP is not a valid parent" );
#else
    const fapi2::TargetType TARGET_TYPE_PROC_CHIPLETS = fapi2::TARGET_TYPE_EX |
            fapi2::TARGET_TYPE_MCS |
            fapi2::TARGET_TYPE_XBUS |
            fapi2::TARGET_TYPE_CORE |
            fapi2::TARGET_TYPE_EQ |
            fapi2::TARGET_TYPE_MCA |
            fapi2::TARGET_TYPE_MCBIST |
            fapi2::TARGET_TYPE_MI |
            fapi2::TARGET_TYPE_CAPP |
            fapi2::TARGET_TYPE_DMI |
            fapi2::TARGET_TYPE_OBUS |
            fapi2::TARGET_TYPE_OBUS_BRICK |
            fapi2::TARGET_TYPE_SBE |
            fapi2::TARGET_TYPE_PPE |
            fapi2::TARGET_TYPE_PERV |
            fapi2::TARGET_TYPE_PEC |
            fapi2::TARGET_TYPE_PHB |
            fapi2::TARGET_TYPE_PROC_CHIP |
            fapi2::TARGET_TYPE_MC |
            fapi2::TARGET_TYPE_OMI |
            fapi2::TARGET_TYPE_OMIC |
            fapi2::TARGET_TYPE_MCC |
            fapi2::TARGET_TYPE_MEM_PORT;

    static_assert( !((T == fapi2::TARGET_TYPE_PROC_CHIP) &&
                     ((K & TARGET_TYPE_PROC_CHIPLETS) == fapi2::TARGET_TYPE_NONE)),
                   "fapi2::TARGET_TYPE_PROC_CHIP is not a valid parent" );

    const fapi2::TargetType TARGET_TYPE_PERV_CHIPLETS = fapi2::TARGET_TYPE_EQ |
            fapi2::TARGET_TYPE_CORE |
            fapi2::TARGET_TYPE_XBUS |
            fapi2::TARGET_TYPE_OBUS |
            fapi2::TARGET_TYPE_CAPP |
            fapi2::TARGET_TYPE_OBUS_BRICK |
            fapi2::TARGET_TYPE_MCBIST |
            fapi2::TARGET_TYPE_MCS |
            fapi2::TARGET_TYPE_MCA |
            fapi2::TARGET_TYPE_MI |
            fapi2::TARGET_TYPE_DMI |
            fapi2::TARGET_TYPE_PEC |
            fapi2::TARGET_TYPE_PHB |
            fapi2::TARGET_TYPE_MC |
            fapi2::TARGET_TYPE_OMI |
            fapi2::TARGET_TYPE_OMIC |
            fapi2::TARGET_TYPE_MCC |
            fapi2::TARGET_TYPE_MEM_PORT;

    static_assert( !((T == fapi2::TARGET_TYPE_PERV) &&
                     ((K & TARGET_TYPE_PERV_CHIPLETS) == fapi2::TARGET_TYPE_NONE)),
                   "fapi2::TARGET_TYPE_PERV is not a valid parent" );
#endif

    // Specific parent checks for each TargetType
#ifdef FAPI_2_Z_TARGET_RULES
    // valid parents for SC_CHIP
    // SC_CHIP -> SYSTEM
    static_assert(!((K == fapi2::TARGET_TYPE_SC_CHIP) &&
                    (T != fapi2::TARGET_TYPE_SYSTEM)),
                  "improper parent of fapi2::TARGET_TYPE_SC_CHIP");

    // valid parents for EP_CHIP
    // EP_CHIP -> SYSTEM
    static_assert(!((K == fapi2::TARGET_TYPE_EP_CHIP) &&
                    (T != fapi2::TARGET_TYPE_SYSTEM)),
                  "improper parent of fapi2::TARGET_TYPE_EP_CHIP");

    // valid parents for ABUS
    // ABUS -> SC_CHIP
    static_assert(!((K == fapi2::TARGET_TYPE_ABUS) &&
                    (T != fapi2::TARGET_TYPE_SC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_ABUS");

    // valid parents for NEST
    // NEST -> SC_CHIP
    // NEST -> EP_CHIP
    static_assert(!((K == fapi2::TARGET_TYPE_NEST) &&
                    (T != fapi2::TARGET_TYPE_SC_CHIP) &&
                    (T != fapi2::TARGET_TYPE_EP_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_NEST");

    // valid parents for XBUS0
    // XBUS0 -> SC_CHIP
    // XBUS0 -> EP_CHIP
    static_assert(!((K == fapi2::TARGET_TYPE_XBUS0) &&
                    (T != fapi2::TARGET_TYPE_SC_CHIP) &&
                    (T != fapi2::TARGET_TYPE_EP_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_XBUS0");

    // valid parents for XBUS1
    // XBUS1 -> SC_CHIP
    // XBUS1 -> EP_CHIP
    static_assert(!((K == fapi2::TARGET_TYPE_XBUS1) &&
                    (T != fapi2::TARGET_TYPE_SC_CHIP) &&
                    (T != fapi2::TARGET_TYPE_EP_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_XBUS1");

    // valid parents for PERV
    // PERV -> SC_CHIP
    // PERV -> EP_CHIP
    static_assert(!((K == fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_SC_CHIP) &&
                    (T != fapi2::TARGET_TYPE_EP_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_PERV");

    // valid parents for GXA
    // GXA -> EP_CHIP
    static_assert(!((K == fapi2::TARGET_TYPE_GXA) &&
                    (T != fapi2::TARGET_TYPE_EP_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_GXA");

    // valid parents for CORE
    // CORE -> EP_CHIP
    static_assert(!((K == fapi2::TARGET_TYPE_CORE) &&
                    (T != fapi2::TARGET_TYPE_EP_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_CORE");

    // valid parents for MCS
    // MCS -> EP_CHIP
    static_assert(!((K == fapi2::TARGET_TYPE_MCS) &&
                    (T != fapi2::TARGET_TYPE_EP_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_MCS");

    // valid parents for PCIE
    // PCIE -> EP_CHIP
    static_assert(!((K == fapi2::TARGET_TYPE_PCIE) &&
                    (T != fapi2::TARGET_TYPE_EP_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_PCIE");

#else
    // valid parents for DIMM
    // DIMM -> MEM_PORT
    // DIMM -> MCA
    // DIMM -> MBA
    // DIMM -> OCMB
    static_assert(!((K == fapi2::TARGET_TYPE_DIMM) &&
                    (T != fapi2::TARGET_TYPE_MEM_PORT) &&
                    (T != fapi2::TARGET_TYPE_MCA) &&
                    (T != fapi2::TARGET_TYPE_MBA) &&
                    (T != fapi2::TARGET_TYPE_OCMB_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_DIMM");

    // valid parents for PROC
    // PROC -> SYSTEM
    static_assert(!((K == fapi2::TARGET_TYPE_PROC_CHIP) &&
                    (T != fapi2::TARGET_TYPE_SYSTEM)),
                  "improper parent of fapi2::TARGET_TYPE_PROC_CHIP");

    // valid parents for MEMBUF
    // MEMBUF -> SYSTEM
    // MEMBUF -> DMI
    static_assert(!((K == fapi2::TARGET_TYPE_MEMBUF_CHIP) &&
                    (T != fapi2::TARGET_TYPE_SYSTEM) &&
                    (T != fapi2::TARGET_TYPE_DMI)),
                  "improper parent of fapi2::TARGET_TYPE_MEMBUF_CHIP");

    // valid parents for OCMB
    // OCMB -> SYSTEM
    // OCMB -> OMI
    static_assert(!((K == fapi2::TARGET_TYPE_OCMB_CHIP) &&
                    (T != fapi2::TARGET_TYPE_SYSTEM) &&
                    (T != fapi2::TARGET_TYPE_OMI)),
                  "improper parent of fapi2::TARGET_TYPE_OCMB_CHIP");

    // valid parents for EX
    // EX -> EQ
    // EX -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_EX) &&
                    (T != fapi2::TARGET_TYPE_EQ) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_EX");

    // valid parents for MBA
    // MBA -> MEMBUF
    static_assert(!((K == fapi2::TARGET_TYPE_MBA) &&
                    (T != fapi2::TARGET_TYPE_MEMBUF_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_MBA");

    // valid parents for MCS
    // MCS -> MCBIST
    // MCS -> PERV
    // MCS -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_MCS) &&
                    (T != fapi2::TARGET_TYPE_MCBIST) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_MCS");

    // valid parents for L4
    // L4 -> MEMBUF
    static_assert(!((K == fapi2::TARGET_TYPE_L4) &&
                    (T != fapi2::TARGET_TYPE_MEMBUF_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_L4");

    // valid parents for CORE
    // CORE -> EX
    // CORE -> EQ
    // CORE -> PERV
    // CORE -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_CORE) &&
                    (T != fapi2::TARGET_TYPE_EX) &&
                    (T != fapi2::TARGET_TYPE_EQ) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_CORE");

    // valid parents for EQ
    // EQ -> PROC
    // EQ -> PERV
    static_assert(!((K == fapi2::TARGET_TYPE_EQ) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_EQ");

    // valid parents for MEM_PORT
    // MEM_PORT -> MCS
    // MEM_PORT -> MCBIST
    // MEM_PORT -> PERV
    // MEM_PORT -> PROC
    // MEM_PORT -> OCMB
    static_assert(!((K == fapi2::TARGET_TYPE_MEM_PORT) &&
                    (T != fapi2::TARGET_TYPE_MCS) &&
                    (T != fapi2::TARGET_TYPE_MCBIST) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP) &&
                    (T != fapi2::TARGET_TYPE_OCMB_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_MEM_PORT");

    // valid parents for MCA
    // MCA -> MCS
    // MCA -> MCBIST
    // MCA -> PERV
    // MCA -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_MCA) &&
                    (T != fapi2::TARGET_TYPE_MCS) &&
                    (T != fapi2::TARGET_TYPE_MCBIST) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_MCA");

    // valid parents for MCBIST
    // MCBIST -> PERV
    // MCBIST -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_MCBIST) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_MCBIST");

    // valid parents for MI
    // MI -> MC
    // MI -> PERV
    // MI -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_MI) &&
                    (T != fapi2::TARGET_TYPE_MC) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_MI");

    // valid parents for DMI
    // DMI -> MI
    // DMI -> MC
    // DMI -> PERV
    // DMI -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_DMI) &&
                    (T != fapi2::TARGET_TYPE_MI) &&
                    (T != fapi2::TARGET_TYPE_MC) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_DMI");

    // valid parents for OMI
    // OMI -> MI
    // OMI -> MC
    // OMI -> OMIC
    // OMI -> MCC
    // OMI -> PERV
    // OMI -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_OMI) &&
                    (T != fapi2::TARGET_TYPE_MI) &&
                    (T != fapi2::TARGET_TYPE_MC) &&
                    (T != fapi2::TARGET_TYPE_OMIC) &&
                    (T != fapi2::TARGET_TYPE_MCC) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_OMI");

    // valid parents for OMIC
    // OMIC -> MC
    // OMIC -> PERV
    // OMIC -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_OMIC) &&
                    (T != fapi2::TARGET_TYPE_MC) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_OMIC");

    // valid parents for MCC
    // MCC -> MI
    // MCC -> MC
    // MCC -> PERV
    // MCC -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_MCC) &&
                    (T != fapi2::TARGET_TYPE_MI) &&
                    (T != fapi2::TARGET_TYPE_MC) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_MCC");

    // valid parents for MC
    // MC -> PERV
    // MC -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_MC) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_MC");

    // valid parents for SBE
    // SBE -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_SBE) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_SBE");

    // valid parents for PPE
    // PPE -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_PPE) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_PPE");

    // valid parents for PERV
    // PERV -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_PERV");

    // valid parents for PEC
    // PEC -> PERV
    // PEC -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_PEC) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_PEC");

    // valid parents for PHB
    // PHB -> PERV
    // PHB -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_PHB) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_PHB");

    // valid parents for XBUS
    // XBUS -> PERV
    // XBUS -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_XBUS) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_XBUS");

    // valid parents for OBUS
    // OBUS -> PERV
    // OBUS -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_OBUS) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_OBUS");

    // valid parents for OBUS_BRICK
    // OBUS_BRICK -> PERV
    // OBUS_BRICK -> OBUS
    // OBUS_BRICK -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_OBUS_BRICK) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_OBUS) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_OBUS_BRICK");

    // valid parents for CAPP
    // CAPP -> PERV
    // CAPP -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_CAPP) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_CAPP");
#endif

    std::list<const ecmdChipTarget*> l_targets;

    plat_target_handle_t l_handle = this->get();
    const ecmdChipTarget * l_pEcmdTarget = l_handle;
    uint32_t l_rc = fapi2GetAssociatedTargets(*l_pEcmdTarget, T, l_targets, TARGET_STATE_FUNCTIONAL);

    if (l_rc)
    {
        // ERROR
        return Target<T, V>();
    }

    if (l_targets.empty())
    {
        // ERROR
        return Target<T, V>();
    }

    return Target<T, V>(plat_target_handle_t(l_targets.front()));
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
template<TargetType K, typename V>
template< TargetType T>
inline std::vector<Target<T, V> >
Target<K, V>::getChildren(const TargetState i_state) const
{
#ifdef FAPI_2_Z_TARGET_RULES
    // valid children for SC_CHIP
    // SC_CHIP -> ABUS
    // SC_CHIP -> NEST
    // SC_CHIP -> XBUS0
    // SC_CHIP -> XBUS1
    // SC_CHIP -> PERV
    static_assert(!((K == fapi2::TARGET_TYPE_SC_CHIP) &&
                    (T != fapi2::TARGET_TYPE_ABUS) &&
                    (T != fapi2::TARGET_TYPE_NEST) &&
                    (T != fapi2::TARGET_TYPE_XBUS0) &&
                    (T != fapi2::TARGET_TYPE_XBUS1) &&
                    (T != fapi2::TARGET_TYPE_PERV)),
                  "improper child of fapi2::TARGET_TYPE_SC_CHIP");

    // valid children for EP_CHIP
    // EP_CHIP -> NEST
    // EP_CHIP -> XBUS0
    // EP_CHIP -> XBUS1
    // EP_CHIP -> PERV
    // EP_CHIP -> GXA
    // EP_CHIP -> CORE
    // EP_CHIP -> MCS
    // EP_CHIP -> PCIE
    static_assert(!((K == fapi2::TARGET_TYPE_EP_CHIP) &&
                    (T != fapi2::TARGET_TYPE_NEST) &&
                    (T != fapi2::TARGET_TYPE_XBUS0) &&
                    (T != fapi2::TARGET_TYPE_XBUS1) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_GXA) &&
                    (T != fapi2::TARGET_TYPE_CORE) &&
                    (T != fapi2::TARGET_TYPE_MCS) &&
                    (T != fapi2::TARGET_TYPE_PCIE)),
                  "improper child of fapi2::TARGET_TYPE_EP_CHIP");

#else
#define INVALID_CHILD(PARENT, CHILD) \
    static_assert(!((K == PARENT) && (T == CHILD)), \
                  #CHILD " is not a child of " #PARENT );

    // invalid children for proc
    INVALID_CHILD(fapi2::TARGET_TYPE_PROC_CHIP, fapi2::TARGET_TYPE_NONE)
    INVALID_CHILD(fapi2::TARGET_TYPE_PROC_CHIP, fapi2::TARGET_TYPE_SYSTEM)
    INVALID_CHILD(fapi2::TARGET_TYPE_PROC_CHIP, fapi2::TARGET_TYPE_DIMM)
    INVALID_CHILD(fapi2::TARGET_TYPE_PROC_CHIP, fapi2::TARGET_TYPE_PROC_CHIP)
    INVALID_CHILD(fapi2::TARGET_TYPE_PROC_CHIP, fapi2::TARGET_TYPE_MEMBUF_CHIP)
    INVALID_CHILD(fapi2::TARGET_TYPE_PROC_CHIP, fapi2::TARGET_TYPE_OCMB_CHIP)
    INVALID_CHILD(fapi2::TARGET_TYPE_PROC_CHIP, fapi2::TARGET_TYPE_MBA)
#undef INVALID_CHILD

    // valid children for PERV
    // PERV -> EQ
    // PERV -> CORE
    // PERV -> XBUS
    // PERV -> OBUS
    // PERV -> CAPP
    // PERV -> NV
    // PERV -> MCBIST
    // PERV -> MCS
    // PERV -> MEM_PORT
    // PERV -> MCA
    // PERV -> PEC
    // PERV -> PHB
    // PERV -> MI
    // PERV -> DMI
    // PERV -> MC
    // PERV -> OMI
    // PERV -> OMIC
    // PERV -> MCC
    static_assert(!((K == fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_EQ) &&
                    (T != fapi2::TARGET_TYPE_CORE) &&
                    (T != fapi2::TARGET_TYPE_XBUS) &&
                    (T != fapi2::TARGET_TYPE_OBUS) &&
                    (T != fapi2::TARGET_TYPE_CAPP) &&
                    (T != fapi2::TARGET_TYPE_OBUS_BRICK) &&
                    (T != fapi2::TARGET_TYPE_MCBIST) &&
                    (T != fapi2::TARGET_TYPE_MCS) &&
                    (T != fapi2::TARGET_TYPE_MEM_PORT) &&
                    (T != fapi2::TARGET_TYPE_MCA) &&
                    (T != fapi2::TARGET_TYPE_PEC) &&
                    (T != fapi2::TARGET_TYPE_PHB) &&
                    (T != fapi2::TARGET_TYPE_MI) &&
                    (T != fapi2::TARGET_TYPE_DMI) &&
                    (T != fapi2::TARGET_TYPE_MC) &&
                    (T != fapi2::TARGET_TYPE_OMI) &&
                    (T != fapi2::TARGET_TYPE_OMIC) &&
                    (T != fapi2::TARGET_TYPE_MCC)),
                  "improper child of fapi2::TARGET_TYPE_PERV");

    // valid children for MEMBUF
    // MEMBUF -> L4
    // MEMBUF -> MBA
    static_assert(!((K == fapi2::TARGET_TYPE_MEMBUF_CHIP) &&
                    (T != fapi2::TARGET_TYPE_L4) &&
                    (T != fapi2::TARGET_TYPE_MBA)),
                  "improper child of fapi2::TARGET_TYPE_MEMBUF_CHIP");

    // valid children for OCMB
    // OCMB -> MEM_PORT
    // OCMB -> DIMM
    static_assert(!((K == fapi2::TARGET_TYPE_OCMB_CHIP) &&
                    (T != fapi2::TARGET_TYPE_MEM_PORT) &&
                    (T != fapi2::TARGET_TYPE_DIMM)),
                  "improper child of fapi2::TARGET_TYPE_OCMB_CHIP");

    // valid children for system
    // SYSTEM -> PROC
    // SYSTEM -> MEMBUF
    // SYSTEM -> OCMB
    // SYSTEM -> DIMM ??? disabled for now
    static_assert(!((K == fapi2::TARGET_TYPE_SYSTEM) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP) &&
                    (T != fapi2::TARGET_TYPE_MEMBUF_CHIP) &&
                    (T != fapi2::TARGET_TYPE_OCMB_CHIP)),
                  "improper child of fapi2::TARGET_TYPE_SYSTEM");

    // valid children for EQ
    // EQ -> CORE
    // EQ -> EX
    static_assert(!((K == fapi2::TARGET_TYPE_EQ) &&
                    (T != fapi2::TARGET_TYPE_CORE) &&
                    (T != fapi2::TARGET_TYPE_EX)),
                  "improper child of fapi2::TARGET_TYPE_EQ");

    // valid children for EX
    // EX -> CORE
    static_assert(!((K == fapi2::TARGET_TYPE_EX) &&
                    (T != fapi2::TARGET_TYPE_CORE)),
                  "improper child of fapi2::TARGET_TYPE_EX");

    // Nimbus Memory
    // valid children for MCS
    // MCS -> MEM_PORT
    // MCS -> MCA
    static_assert(!((K == fapi2::TARGET_TYPE_MCS) &&
                    (T != fapi2::TARGET_TYPE_MEM_PORT) &&
                    (T != fapi2::TARGET_TYPE_MCA)),
                  "improper child of fapi2::TARGET_TYPE_MCS");

    // valid children for MEM_PORT
    // MEM_PORT -> DIMM
    static_assert(!((K == fapi2::TARGET_TYPE_MEM_PORT) &&
                    (T != fapi2::TARGET_TYPE_DIMM)),
                  "improper child of fapi2::TARGET_TYPE_MEM_PORT");

    // valid children for MCA
    // MCA -> DIMM
    static_assert(!((K == fapi2::TARGET_TYPE_MCA) &&
                    (T != fapi2::TARGET_TYPE_DIMM)),
                  "improper child of fapi2::TARGET_TYPE_MCA");

    // valid children for MCBIST
    // MCBIST -> MEM_PORT
    // MCBIST -> MCA
    // MCBIST -> MCS
    static_assert(!((K == fapi2::TARGET_TYPE_MCBIST) &&
                    (T != fapi2::TARGET_TYPE_MEM_PORT) &&
                    (T != fapi2::TARGET_TYPE_MCA) &&
                    (T != fapi2::TARGET_TYPE_MCS)),
                  "improper child of fapi2::TARGET_TYPE_MCBIST");

    // Cumulus Memory
    // valid children for MI
    // MI -> DMI
    // MI -> OMI
    // MI -> MCC
    static_assert(!((K == fapi2::TARGET_TYPE_MI) &&
                    (T != fapi2::TARGET_TYPE_DMI) &&
                    (T != fapi2::TARGET_TYPE_OMI) &&
                    (T != fapi2::TARGET_TYPE_MCC)),
                  "improper child of fapi2::TARGET_TYPE_MI");

    // valid children for DMI
    // DMI -> MEMBUF
    static_assert(!((K == fapi2::TARGET_TYPE_DMI) &&
                    (T != fapi2::TARGET_TYPE_MEMBUF_CHIP)),
                  "improper child of fapi2::TARGET_TYPE_DMI");

    // valid children for OMI
    // OMI -> OCMB
    static_assert(!((K == fapi2::TARGET_TYPE_OMI) &&
                    (T != fapi2::TARGET_TYPE_OCMB_CHIP)),
                  "improper child of fapi2::TARGET_TYPE_OMI");

    // valid children for OMIC
    // OMIC -> OMI
    static_assert(!((K == fapi2::TARGET_TYPE_OMIC) &&
                    (T != fapi2::TARGET_TYPE_OMI)),
                  "improper child of fapi2::TARGET_TYPE_OMIC");

    // valid children for MCC
    // MCC -> OMI
    static_assert(!((K == fapi2::TARGET_TYPE_MCC) &&
                    (T != fapi2::TARGET_TYPE_OMI)),
                  "improper child of fapi2::TARGET_TYPE_MCC");

    // valid children for MBA
    // MBA -> DIMM
    static_assert(!((K == fapi2::TARGET_TYPE_MBA) &&
                    (T != fapi2::TARGET_TYPE_DIMM)),
                  "improper child of fapi2::TARGET_TYPE_MBA");

    // valid children for MC
    // MC -> MI
    // MC -> DMI
    // MC -> OMI
    // MC -> OMIC
    // MC -> MCC
    static_assert(!((K == fapi2::TARGET_TYPE_MC) &&
                    (T != fapi2::TARGET_TYPE_MI) &&
                    (T != fapi2::TARGET_TYPE_DMI) &&
                    (T != fapi2::TARGET_TYPE_OMI) &&
                    (T != fapi2::TARGET_TYPE_OMIC) &&
                    (T != fapi2::TARGET_TYPE_MCC)),
                  "improper child of fapi2::TARGET_TYPE_MC");

    // valid children for OBUS
    // OBUS -> OBUS_BRICK
    // OBUS -> OBUS
    static_assert(!((K == fapi2::TARGET_TYPE_OBUS) &&
                    (T != fapi2::TARGET_TYPE_OBUS_BRICK) &&
                    (T != fapi2::TARGET_TYPE_OBUS)),
                  "improper child of fapi2::TARGET_TYPE_OBUS");

#endif

    std::list<const ecmdChipTarget*> l_targets;
    std::vector<Target<T, V> > l_children;

    plat_target_handle_t l_handle = this->get();
    const ecmdChipTarget * l_pEcmdTarget = l_handle;
    uint32_t l_rc = fapi2GetAssociatedTargets(*l_pEcmdTarget, T, l_targets, i_state);

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
        l_children.push_back(Target<T, V>(plat_target_handle_t(*l_target)));
        l_target++;
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
template<TargetType K, typename V>
template< TargetType T>
inline std::vector<Target<T, V> >
Target<K, V>::getChildren(const TargetFilter i_filter,
    const TargetState i_state) const
{
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
            fapi2::TARGET_TYPE_MCC;

    static_assert( !((T & TARGET_TYPE_FILTER_CHIPLETS) == fapi2::TARGET_TYPE_NONE),
                   "not a valid chiplet for filtering" );

    std::list<const ecmdChipTarget*> l_targets;
    std::vector<Target<T, V> > l_children;

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
        l_children.push_back(Target<T, V>(plat_target_handle_t(*l_target)));
        l_target++;
    }

    return l_children;
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

template<TargetType K, typename V>
template<TargetType T>
inline fapi2::ReturnCodes
Target<K, V>::getOtherEnd(Target<T, V>& o_target, const TargetState i_state) const
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

    o_target = Target<T, V>(l_targets.front());
    return FAPI2_RC_SUCCESS;
}

///
/// @brief Is the target functional?
/// @return true if target is functional, false if non-functional
///

template<TargetType K, typename V>
inline bool
Target<K, V>::isFunctional(void) const
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
template<TargetType K, typename V>
inline uint8_t
Target<K, V>::getChipletNumber(void) const
{
    uint8_t l_pervasive_chipunit = 0;
    plat_target_handle_t l_handle = this->get();
    const ecmdChipTarget * l_pEcmdTarget = l_handle;

    // check if this is a PERV target
    if ((l_pEcmdTarget->chipUnitTypeState == ECMD_TARGET_FIELD_VALID) &&
        (l_pEcmdTarget->chipUnitType == "perv") &&
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
              (l_pEcmdTarget->chipUnitType == "mcc")) &&
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
template< TargetType T, typename V >
inline void toString(const Target<T, V>& i_target, char* i_buffer, size_t i_bsize)
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
template< TargetType T, typename V >
inline void toString(const Target<T, V>* i_target, char* i_buffer, size_t i_bsize)
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
template<TargetType T, typename V>
inline Target<T, V> getTarget(uint64_t Ordinal)
{
    // For testing
    return Target<T, V>(NULL);
}

template<>
inline Target<fapi2::TARGET_TYPE_PROC_CHIP, fapi2::plat_target_handle_t> getTarget(uint64_t Ordinal)
{
    fapi2::Target<fapi2::TARGET_TYPE_SYSTEM, fapi2::plat_target_handle_t> l_system;
    ecmdChipTarget l_ecmdTarget;
    fapiTargetToEcmdTarget(l_system, l_ecmdTarget);

    std::list<const ecmdChipTarget*> l_targets;
    uint32_t l_ecmd_rc = fapi2GetAssociatedTargets(l_ecmdTarget, fapi2::TARGET_TYPE_PROC_CHIP, l_targets, TARGET_STATE_PRESENT);

    if ((l_ecmd_rc) || (l_targets.empty()))
    {
        // ERROR
        return Target<fapi2::TARGET_TYPE_PROC_CHIP, fapi2::plat_target_handle_t>(NULL);
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
        return Target<fapi2::TARGET_TYPE_PROC_CHIP, fapi2::plat_target_handle_t>(NULL);
    }

    return Target<fapi2::TARGET_TYPE_PROC_CHIP, fapi2::plat_target_handle_t>(l_matching_targets.front());
}

#ifdef FAPI2_ENABLE_PLATFORM_GET_TARGET
template<TargetType T, typename V=fapi2::plat_target_handle_t>
inline Target<T,V>* getTarget(TargetType i_type, uint8_t instance)
{
    fapi2::Target<fapi2::TARGET_TYPE_SYSTEM, V> l_system;
    ecmdChipTarget l_ecmdTarget;
    fapiTargetToEcmdTarget(l_system, l_ecmdTarget);

    std::list<const ecmdChipTarget*> l_targets;
    uint32_t l_ecmd_rc = fapi2GetAssociatedTargets(l_ecmdTarget, i_type, l_targets, TARGET_STATE_PRESENT);

    if ((l_ecmd_rc) || (l_targets.empty()))
    {
        // ERROR
        return nullptr;
    }

    printf("getTarget, %08X, %d\n", i_type, instance);
    // There is no ATTR_FAPI_POS for the system target
    if (i_type == fapi2::TARGET_TYPE_SYSTEM)
    {
        return new Target<T,V>(l_targets.front());
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

    return new Target<T,V>(l_matching_targets.front());
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
template<TargetType T, typename V, typename B>
inline void toString(const Target<T, V>& i_target, B& i_buffer)
{
}

}

#endif
