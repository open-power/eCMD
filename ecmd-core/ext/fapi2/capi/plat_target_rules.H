/* IBM_PROLOG_BEGIN_TAG                                                   */
/* 
 * Copyright 2017, 2020 IBM International Business Machines Corp.
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
 * @file plat_target_rules.H
 * @brief relationship rules for fapi2 targets
 */

#ifndef __FAPI2_PLAT_TARGET_RULES__
#define __FAPI2_PLAT_TARGET_RULES__

#include <target_types.H>

namespace fapi2plat
{

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_multicast_parent()
{
    // It's forbidden to strip multicast from a target below PROC_CHIP level:
    // Since a multicast might end up expanding to more than one unicast target,
    // users are required to use getChildren if they want to map back to unicast.
    static_assert(!(((K ^ T) & fapi2::TARGET_TYPE_MULTICAST) &&
                    ((T & fapi2::TARGET_TYPE_CHIPLETS) != fapi2::TARGET_TYPE_NONE)),
                  "Parent target for a multicast target must also be multicast or PROC_CHIP");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_multicast_children()
{
    // valid children for multicast targets are the same with multicast stripped
    static_assert(!((K & fapi2::TARGET_TYPE_MULTICAST) &&
                  (K != (T | fapi2::TARGET_TYPE_MULTICAST))),
                  "improper child of fapi2::TARGET_TYPE_MULTICAST");
}

#ifdef FAPI_2_Z_TARGET_RULES

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_ep_chip_valid_parent()
{
    const fapi2::TargetType TARGET_TYPE_EP_CHIPLETS =
            fapi2::TARGET_TYPE_NEST |
            fapi2::TARGET_TYPE_XBUS0 |
            fapi2::TARGET_TYPE_XBUS1 |
            fapi2::TARGET_TYPE_PERV |
            fapi2::TARGET_TYPE_GXA |
            fapi2::TARGET_TYPE_CORE |
            fapi2::TARGET_TYPE_MCS |
            fapi2::TARGET_TYPE_PCIE |
            fapi2::TARGET_TYPE_ABUS |
            fapi2::TARGET_TYPE_EX |
            fapi2::TARGET_TYPE_L3;

    static_assert( !((T == fapi2::TARGET_TYPE_EP_CHIP) &&
                     ((K & TARGET_TYPE_EP_CHIPLETS) == fapi2::TARGET_TYPE_NONE)),
                   "fapi2::TARGET_TYPE_EP_CHIP is not a valid parent" );
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_sc_chip_valid_parent()
{
    const fapi2::TargetType TARGET_TYPE_SC_CHIPLETS =
            fapi2::TARGET_TYPE_ABUS |
            fapi2::TARGET_TYPE_NEST |
            fapi2::TARGET_TYPE_XBUS0 |
            fapi2::TARGET_TYPE_XBUS1 |
            fapi2::TARGET_TYPE_PERV;

    static_assert( !((T == fapi2::TARGET_TYPE_SC_CHIP) &&
                     ((K & TARGET_TYPE_SC_CHIPLETS) == fapi2::TARGET_TYPE_NONE)),
                   "fapi2::TARGET_TYPE_SC_CHIP is not a valid parent" );
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_sc_chip()
{
    // valid parents for SC_CHIP
    // SC_CHIP -> SYSTEM
    static_assert(!((K == fapi2::TARGET_TYPE_SC_CHIP) &&
                    (T != fapi2::TARGET_TYPE_SYSTEM)),
                  "improper parent of fapi2::TARGET_TYPE_SC_CHIP");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_ep_chip()
{
    // valid parents for EP_CHIP
    // EP_CHIP -> SYSTEM
    static_assert(!((K == fapi2::TARGET_TYPE_EP_CHIP) &&
                    (T != fapi2::TARGET_TYPE_SYSTEM)),
                  "improper parent of fapi2::TARGET_TYPE_EP_CHIP");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_abus()
{
    // valid parents for ABUS
    // ABUS -> SC_CHIP
    // ABUS -> EP_CHIP
    // ABUS -> PERV
    static_assert(!((K == fapi2::TARGET_TYPE_ABUS) &&
                    (T != fapi2::TARGET_TYPE_SC_CHIP) &&
                    (T != fapi2::TARGET_TYPE_EP_CHIP) &&
                    (T != fapi2::TARGET_TYPE_PERV)),
                  "improper parent of fapi2::TARGET_TYPE_ABUS");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_nest()
{
    // valid parents for NEST
    // NEST -> SC_CHIP
    // NEST -> EP_CHIP
    // NEST -> PERV
    static_assert(!((K == fapi2::TARGET_TYPE_NEST) &&
                    (T != fapi2::TARGET_TYPE_SC_CHIP) &&
                    (T != fapi2::TARGET_TYPE_EP_CHIP) &&
                    (T != fapi2::TARGET_TYPE_PERV)),
                  "improper parent of fapi2::TARGET_TYPE_NEST");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_xbus0()
{
    // valid parents for XBUS0
    // XBUS0 -> SC_CHIP
    // XBUS0 -> EP_CHIP
    // XBUS0 -> PERV
    static_assert(!((K == fapi2::TARGET_TYPE_XBUS0) &&
                    (T != fapi2::TARGET_TYPE_SC_CHIP) &&
                    (T != fapi2::TARGET_TYPE_EP_CHIP) &&
                    (T != fapi2::TARGET_TYPE_PERV)),
                  "improper parent of fapi2::TARGET_TYPE_XBUS0");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_xbus1()
{
    // valid parents for XBUS1
    // XBUS1 -> SC_CHIP
    // XBUS1 -> EP_CHIP
    static_assert(!((K == fapi2::TARGET_TYPE_XBUS1) &&
                    (T != fapi2::TARGET_TYPE_SC_CHIP) &&
                    (T != fapi2::TARGET_TYPE_EP_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_XBUS1");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_perv()
{
    // valid parents for PERV
    // PERV -> SC_CHIP
    // PERV -> EP_CHIP
    static_assert(!((K == fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_SC_CHIP) &&
                    (T != fapi2::TARGET_TYPE_EP_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_PERV");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_gxa()
{
    // valid parents for GXA
    // GXA -> EP_CHIP
    static_assert(!((K == fapi2::TARGET_TYPE_GXA) &&
                    (T != fapi2::TARGET_TYPE_EP_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_GXA");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_ex()
{
    // valid parents for EX
    // EX -> EP_CHIP
    // EX -> PERV
    static_assert(!((K == fapi2::TARGET_TYPE_EX) &&
                    (T != fapi2::TARGET_TYPE_EP_CHIP) &&
                    (T != fapi2::TARGET_TYPE_PERV)),
                  "improper parent of fapi2::TARGET_TYPE_EX");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_core()
{
    // valid parents for CORE
    // CORE -> EP_CHIP
    // CORE -> EX
    // CORE -> PERV
    static_assert(!((K == fapi2::TARGET_TYPE_CORE) &&
                    (T != fapi2::TARGET_TYPE_EP_CHIP) &&
                    (T != fapi2::TARGET_TYPE_EX) &&
                    (T != fapi2::TARGET_TYPE_PERV)),
                  "improper parent of fapi2::TARGET_TYPE_CORE");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_l3()
{
    // valid parents for L3
    // L3 -> EP_CHIP
    // L3 -> EX
    // L3 -> PERV
    static_assert(!((K == fapi2::TARGET_TYPE_L3) &&
                    (T != fapi2::TARGET_TYPE_EP_CHIP) &&
                    (T != fapi2::TARGET_TYPE_EX) &&
                    (T != fapi2::TARGET_TYPE_PERV)),
                  "improper parent of fapi2::TARGET_TYPE_L3");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_mcs()
{
    // valid parents for MCS
    // MCS -> EP_CHIP
    // MCS -> PERV
    static_assert(!((K == fapi2::TARGET_TYPE_MCS) &&
                    (T != fapi2::TARGET_TYPE_EP_CHIP) &&
                    (T != fapi2::TARGET_TYPE_PERV)),
                  "improper parent of fapi2::TARGET_TYPE_MCS");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_pcie()
{
    // valid parents for PCIE
    // PCIE -> EP_CHIP
    // PCIE -> PERV
    static_assert(!((K == fapi2::TARGET_TYPE_PCIE) &&
                    (T != fapi2::TARGET_TYPE_EP_CHIP) &&
                    (T != fapi2::TARGET_TYPE_PERV)),
                  "improper parent of fapi2::TARGET_TYPE_PCIE");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_unicast_parent()
{
    // General compound chiplet check for parent
    fapi2plat::_check_ep_chip_valid_parent<K, T>();
    fapi2plat::_check_sc_chip_valid_parent<K, T>();
    // Specific parent checks for each TargetType
    fapi2plat::_check_valid_parent_of_sc_chip<K, T>();
    fapi2plat::_check_valid_parent_of_ep_chip<K, T>();
    fapi2plat::_check_valid_parent_of_abus<K, T>();
    fapi2plat::_check_valid_parent_of_nest<K, T>();
    fapi2plat::_check_valid_parent_of_xbus0<K, T>();
    fapi2plat::_check_valid_parent_of_xbus1<K, T>();
    fapi2plat::_check_valid_parent_of_perv<K, T>();
    fapi2plat::_check_valid_parent_of_gxa<K, T>();
    fapi2plat::_check_valid_parent_of_ex<K, T>();
    fapi2plat::_check_valid_parent_of_core<K, T>();
    fapi2plat::_check_valid_parent_of_l3<K, T>();
    fapi2plat::_check_valid_parent_of_mcs<K, T>();
    fapi2plat::_check_valid_parent_of_pcie<K, T>();
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_sc_chip_valid_children()
{
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
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_ep_chip_valid_children()
{
    // valid children for EP_CHIP
    // EP_CHIP -> NEST
    // EP_CHIP -> XBUS0
    // EP_CHIP -> XBUS1
    // EP_CHIP -> PERV
    // EP_CHIP -> GXA
    // EP_CHIP -> CORE
    // EP_CHIP -> MCS
    // EP_CHIP -> PCIE
    // EP_CHIP -> ABUS
    // EP_CHIP -> EX
    // EP_CHIP -> L3
    static_assert(!((K == fapi2::TARGET_TYPE_EP_CHIP) &&
                    (T != fapi2::TARGET_TYPE_NEST) &&
                    (T != fapi2::TARGET_TYPE_XBUS0) &&
                    (T != fapi2::TARGET_TYPE_XBUS1) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_GXA) &&
                    (T != fapi2::TARGET_TYPE_CORE) &&
                    (T != fapi2::TARGET_TYPE_MCS) &&
                    (T != fapi2::TARGET_TYPE_PCIE) &&
                    (T != fapi2::TARGET_TYPE_ABUS) &&
                    (T != fapi2::TARGET_TYPE_EX) &&
                    (T != fapi2::TARGET_TYPE_L3)),
                  "improper child of fapi2::TARGET_TYPE_EP_CHIP");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_perv_valid_children()
{
    // valid children for PERV
    // PERV -> EX
    // PERV -> CORE
    // PERV -> L3
    // PERV -> XBUS
    // PERV -> ABUS
    // PERV -> MCS
    // PERV -> NEST
    // PERV -> PCIE
    static_assert(!((K == fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_EX) &&
                    (T != fapi2::TARGET_TYPE_CORE) &&
                    (T != fapi2::TARGET_TYPE_L3) &&
                    (T != fapi2::TARGET_TYPE_XBUS) &&
                    (T != fapi2::TARGET_TYPE_ABUS) &&
                    (T != fapi2::TARGET_TYPE_MCS) &&
                    (T != fapi2::TARGET_TYPE_NEST) &&
                    (T != fapi2::TARGET_TYPE_PCIE)),
                  "improper child of fapi2::TARGET_TYPE_PERV");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_ex_valid_children()
{
    // valid children for EX
    // EX -> CORE
    // EX -> L3
    static_assert(!((K == fapi2::TARGET_TYPE_EX) &&
                    (T != fapi2::TARGET_TYPE_CORE) &&
                    (T != fapi2::TARGET_TYPE_L3)),
                  "improper child of fapi2::TARGET_TYPE_EX");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_unicast_children()
{
    fapi2plat::_check_sc_chip_valid_children<K, T>();
    fapi2plat::_check_ep_chip_valid_children<K, T>();
    fapi2plat::_check_perv_valid_children<K, T>();
    fapi2plat::_check_ex_valid_children<K, T>();
}

#else

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_proc_chip_valid_parent()
{
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
            fapi2::TARGET_TYPE_MEM_PORT |
            fapi2::TARGET_TYPE_NMMU |
            fapi2::TARGET_TYPE_PAU |
            fapi2::TARGET_TYPE_IOHS |
            fapi2::TARGET_TYPE_FC |
            fapi2::TARGET_TYPE_PAUC;

    static_assert( !((T == fapi2::TARGET_TYPE_PROC_CHIP) &&
                     ((K & TARGET_TYPE_PROC_CHIPLETS) == fapi2::TARGET_TYPE_NONE)),
                   "fapi2::TARGET_TYPE_PROC_CHIP is not a valid parent" );
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_perv_valid_parent()
{
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
            fapi2::TARGET_TYPE_MEM_PORT |
            fapi2::TARGET_TYPE_NMMU |
            fapi2::TARGET_TYPE_PAU |
            fapi2::TARGET_TYPE_IOHS |
            fapi2::TARGET_TYPE_FC |
            fapi2::TARGET_TYPE_PPE |
            fapi2::TARGET_TYPE_PAUC;

    static_assert( !((T == fapi2::TARGET_TYPE_PERV) &&
                     ((K & TARGET_TYPE_PERV_CHIPLETS) == fapi2::TARGET_TYPE_NONE)),
                   "fapi2::TARGET_TYPE_PERV is not a valid parent" );
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_dimm()
{
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
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_pmic()
{
    // valid parents for PMIC
    // PMIC -> OCMB
    static_assert(!((K == fapi2::TARGET_TYPE_PMIC) &&
                    (T != fapi2::TARGET_TYPE_OCMB_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_PMIC");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_generici2cresponder()
{
    // valid parents for GENERICI2CRESPONDER
    // GENERICI2CRESPONDER -> OCMB
    static_assert(!((K == fapi2::TARGET_TYPE_GENERICI2CRESPONDER) &&
                    (T != fapi2::TARGET_TYPE_OCMB_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_GENERICI2CRESPONDER");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_mdsctlr()
{
    // valid parents for MDS_CTLR
    // MDS_CTLR -> OCMB
    static_assert(!((K == fapi2::TARGET_TYPE_MDS_CTLR) &&
                    (T != fapi2::TARGET_TYPE_OCMB_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_MDS_CTLR");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_proc_chip()
{
    // valid parents for PROC
    // PROC -> SYSTEM
    static_assert(!((K == fapi2::TARGET_TYPE_PROC_CHIP) &&
                    (T != fapi2::TARGET_TYPE_SYSTEM)),
                  "improper parent of fapi2::TARGET_TYPE_PROC_CHIP");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_membuf_chip()
{
    // valid parents for MEMBUF
    // MEMBUF -> SYSTEM
    // MEMBUF -> DMI
    static_assert(!((K == fapi2::TARGET_TYPE_MEMBUF_CHIP) &&
                    (T != fapi2::TARGET_TYPE_SYSTEM) &&
                    (T != fapi2::TARGET_TYPE_DMI)),
                  "improper parent of fapi2::TARGET_TYPE_MEMBUF_CHIP");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_ocmb_chip()
{
    // valid parents for OCMB
    // OCMB -> SYSTEM
    // OCMB -> OMI
    static_assert(!((K == fapi2::TARGET_TYPE_OCMB_CHIP) &&
                    (T != fapi2::TARGET_TYPE_SYSTEM) &&
                    (T != fapi2::TARGET_TYPE_OMI)),
                  "improper parent of fapi2::TARGET_TYPE_OCMB_CHIP");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_ex()
{
    // valid parents for EX
    // EX -> EQ
    // EX -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_EX) &&
                    (T != fapi2::TARGET_TYPE_EQ) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_EX");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_mba()
{
    // valid parents for MBA
    // MBA -> MEMBUF
    static_assert(!((K == fapi2::TARGET_TYPE_MBA) &&
                    (T != fapi2::TARGET_TYPE_MEMBUF_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_MBA");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_mcs()
{
    // valid parents for MCS
    // MCS -> MCBIST
    // MCS -> PERV
    // MCS -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_MCS) &&
                    (T != fapi2::TARGET_TYPE_MCBIST) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_MCS");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_l4()
{
    // valid parents for L4
    // L4 -> MEMBUF
    static_assert(!((K == fapi2::TARGET_TYPE_L4) &&
                    (T != fapi2::TARGET_TYPE_MEMBUF_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_L4");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_core()
{
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
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_eq()
{
    // valid parents for EQ
    // EQ -> PROC
    // EQ -> PERV
    static_assert(!((K == fapi2::TARGET_TYPE_EQ) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_EQ");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_mem_port()
{
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
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_mca()
{
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
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_mcbist()
{
    // valid parents for MCBIST
    // MCBIST -> PERV
    // MCBIST -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_MCBIST) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_MCBIST");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_mi()
{
    // valid parents for MI
    // MI -> MC
    // MI -> PERV
    // MI -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_MI) &&
                    (T != fapi2::TARGET_TYPE_MC) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_MI");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_dmi()
{
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
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_omi()
{
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
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_omic()
{
    // valid parents for OMIC
    // OMIC -> MC
    // OMIC -> PAUC
    // OMIC -> PERV
    // OMIC -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_OMIC) &&
                    (T != fapi2::TARGET_TYPE_MC) &&
                    (T != fapi2::TARGET_TYPE_PAUC) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_OMIC");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_mcc()
{
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
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_mc()
{
    // valid parents for MC
    // MC -> PERV
    // MC -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_MC) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_MC");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_sbe()
{
    // valid parents for SBE
    // SBE -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_SBE) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_SBE");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_ppe()
{
    // valid parents for PPE
    // PPE -> PERV
    // PPE -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_PPE) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_PPE");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_perv()
{
    // valid parents for PERV
    // PERV -> PROC
    // PERV -> OCMB
    static_assert(!((K == fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_OCMB_CHIP) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_PERV");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_pec()
{
    // valid parents for PEC
    // PEC -> PERV
    // PEC -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_PEC) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_PEC");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_phb()
{
    // valid parents for PHB
    // PHB -> PERV
    // PHB -> PEC
    // PHB -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_PHB) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PEC) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_PHB");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_xbus()
{
    // valid parents for XBUS
    // XBUS -> PERV
    // XBUS -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_XBUS) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_XBUS");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_obus()
{
    // valid parents for OBUS
    // OBUS -> PERV
    // OBUS -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_OBUS) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_OBUS");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_obus_brick()
{
    // valid parents for OBUS_BRICK
    // OBUS_BRICK -> PERV
    // OBUS_BRICK -> OBUS
    // OBUS_BRICK -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_OBUS_BRICK) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_OBUS) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_OBUS_BRICK");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_capp()
{
    // valid parents for CAPP
    // CAPP -> PERV
    // CAPP -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_CAPP) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_CAPP");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_nmmu()
{
    // valid parents for NMMU
    // NMMU -> PERV
    // NMMU -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_NMMU) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_NMMU");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_iohs()
{
    // valid parents for IOHS
    // IOHS -> PAUC
    // IOHS -> PERV
    // IOHS -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_IOHS) &&
                    (T != fapi2::TARGET_TYPE_PAUC) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_IOHS");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_iolink()
{
    // valid parents for IOLINK
    // IOLINK -> IOHS
    static_assert(!((K == fapi2::TARGET_TYPE_IOLINK) &&
                    (T != fapi2::TARGET_TYPE_IOHS)),
                  "improper parent of fapi2::TARGET_TYPE_IOLINK");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_parent_of_pau()
{
    // valid parents for PAU
    // PAU -> PERV
    // PAU -> PAUC
    // PAU -> PROC
    static_assert(!((K == fapi2::TARGET_TYPE_PAU) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_PAUC) &&
                    (T != fapi2::TARGET_TYPE_PROC_CHIP)),
                  "improper parent of fapi2::TARGET_TYPE_PAU");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_unicast_parent()
{
    // General compound chiplet check for parent
    fapi2plat::_check_proc_chip_valid_parent<K, T>();
    fapi2plat::_check_perv_valid_parent<K, T>();
    // Specific parent checks for each TargetType
    fapi2plat::_check_valid_parent_of_dimm<K, T>();
    fapi2plat::_check_valid_parent_of_pmic<K, T>();
    fapi2plat::_check_valid_parent_of_generici2cresponder<K, T>();
    fapi2plat::_check_valid_parent_of_mdsctlr<K, T>();
    fapi2plat::_check_valid_parent_of_proc_chip<K, T>();
    fapi2plat::_check_valid_parent_of_membuf_chip<K, T>();
    fapi2plat::_check_valid_parent_of_ocmb_chip<K, T>();
    fapi2plat::_check_valid_parent_of_ex<K, T>();
    fapi2plat::_check_valid_parent_of_mba<K, T>();
    fapi2plat::_check_valid_parent_of_mcs<K, T>();
    fapi2plat::_check_valid_parent_of_l4<K, T>();
    fapi2plat::_check_valid_parent_of_core<K, T>();
    fapi2plat::_check_valid_parent_of_eq<K, T>();
    fapi2plat::_check_valid_parent_of_mem_port<K, T>();
    fapi2plat::_check_valid_parent_of_mca<K, T>();
    fapi2plat::_check_valid_parent_of_mcbist<K, T>();
    fapi2plat::_check_valid_parent_of_mi<K, T>();
    fapi2plat::_check_valid_parent_of_dmi<K, T>();
    fapi2plat::_check_valid_parent_of_omi<K, T>();
    fapi2plat::_check_valid_parent_of_omic<K, T>();
    fapi2plat::_check_valid_parent_of_mcc<K, T>();
    fapi2plat::_check_valid_parent_of_mc<K, T>();
    fapi2plat::_check_valid_parent_of_sbe<K, T>();
    fapi2plat::_check_valid_parent_of_ppe<K, T>();
    fapi2plat::_check_valid_parent_of_perv<K, T>();
    fapi2plat::_check_valid_parent_of_pec<K, T>();
    fapi2plat::_check_valid_parent_of_phb<K, T>();
    fapi2plat::_check_valid_parent_of_xbus<K, T>();
    fapi2plat::_check_valid_parent_of_obus<K, T>();
    fapi2plat::_check_valid_parent_of_obus_brick<K, T>();
    fapi2plat::_check_valid_parent_of_capp<K, T>();
    fapi2plat::_check_valid_parent_of_nmmu<K, T>();
    fapi2plat::_check_valid_parent_of_iohs<K, T>();
    fapi2plat::_check_valid_parent_of_iolink<K, T>();
    fapi2plat::_check_valid_parent_of_pau<K, T>();
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_proc_chip_valid_children()
{
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
    INVALID_CHILD(fapi2::TARGET_TYPE_PROC_CHIP, fapi2::TARGET_TYPE_L4)
    INVALID_CHILD(fapi2::TARGET_TYPE_PROC_CHIP, fapi2::TARGET_TYPE_GENERICI2CRESPONDER)
    INVALID_CHILD(fapi2::TARGET_TYPE_PROC_CHIP, fapi2::TARGET_TYPE_PMIC)
    INVALID_CHILD(fapi2::TARGET_TYPE_PROC_CHIP, fapi2::TARGET_TYPE_MDS_CTLR)

#undef INVALID_CHILD
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_perv_valid_children()
{
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
    // PERV -> NMMU
    // PERV -> PAU
    // PERV -> IOHS
    // PERV -> FC
    // PERV -> PPE
    // PERV -> PAUC
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
                    (T != fapi2::TARGET_TYPE_MCC) &&
                    (T != fapi2::TARGET_TYPE_NMMU) &&
                    (T != fapi2::TARGET_TYPE_PAU) &&
                    (T != fapi2::TARGET_TYPE_IOHS) &&
                    (T != fapi2::TARGET_TYPE_FC) &&
                    (T != fapi2::TARGET_TYPE_PPE) &&
                    (T != fapi2::TARGET_TYPE_PAUC)),
                  "improper child of fapi2::TARGET_TYPE_PERV");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_membuf_chip_valid_children()
{
    // valid children for MEMBUF
    // MEMBUF -> L4
    // MEMBUF -> MBA
    static_assert(!((K == fapi2::TARGET_TYPE_MEMBUF_CHIP) &&
                    (T != fapi2::TARGET_TYPE_L4) &&
                    (T != fapi2::TARGET_TYPE_MBA)),
                  "improper child of fapi2::TARGET_TYPE_MEMBUF_CHIP");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_ocmb_chip_valid_children()
{
    // valid children for OCMB
    // OCMB -> MEM_PORT
    // OCMB -> PERV
    // OCMB -> DIMM
    // OCMB -> PMIC
    // OCMB -> GENERICI2CRESPONDER
    // OCMB -> MDS_CTLR
    static_assert(!((K == fapi2::TARGET_TYPE_OCMB_CHIP) &&
                    (T != fapi2::TARGET_TYPE_MEM_PORT) &&
                    (T != fapi2::TARGET_TYPE_PERV) &&
                    (T != fapi2::TARGET_TYPE_DIMM) &&
                    (T != fapi2::TARGET_TYPE_PMIC) &&
                    (T != fapi2::TARGET_TYPE_GENERICI2CRESPONDER) &&
                    (T != fapi2::TARGET_TYPE_MDS_CTLR)),
                  "improper child of fapi2::TARGET_TYPE_OCMB_CHIP");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_system_valid_children()
{
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
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_eq_valid_children()
{
    // valid children for EQ
    // EQ -> CORE
    // EQ -> EX
    static_assert(!((K == fapi2::TARGET_TYPE_EQ) &&
                    (T != fapi2::TARGET_TYPE_CORE) &&
                    (T != fapi2::TARGET_TYPE_EX)),
                  "improper child of fapi2::TARGET_TYPE_EQ");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_ex_valid_children()
{
    // valid children for EX
    // EX -> CORE
    static_assert(!((K == fapi2::TARGET_TYPE_EX) &&
                    (T != fapi2::TARGET_TYPE_CORE)),
                  "improper child of fapi2::TARGET_TYPE_EX");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_mcs_valid_children()
{
    // Nimbus Memory
    // valid children for MCS
    // MCS -> MEM_PORT
    // MCS -> MCA
    static_assert(!((K == fapi2::TARGET_TYPE_MCS) &&
                    (T != fapi2::TARGET_TYPE_MEM_PORT) &&
                    (T != fapi2::TARGET_TYPE_MCA)),
                  "improper child of fapi2::TARGET_TYPE_MCS");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_mem_port_valid_children()
{
    // valid children for MEM_PORT
    // MEM_PORT -> DIMM
    static_assert(!((K == fapi2::TARGET_TYPE_MEM_PORT) &&
                    (T != fapi2::TARGET_TYPE_DIMM)),
                  "improper child of fapi2::TARGET_TYPE_MEM_PORT");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_mca_valid_children()
{
    // valid children for MCA
    // MCA -> DIMM
    static_assert(!((K == fapi2::TARGET_TYPE_MCA) &&
                    (T != fapi2::TARGET_TYPE_DIMM)),
                  "improper child of fapi2::TARGET_TYPE_MCA");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_mcbist_valid_children()
{
    // valid children for MCBIST
    // MCBIST -> MEM_PORT
    // MCBIST -> MCA
    // MCBIST -> MCS
    static_assert(!((K == fapi2::TARGET_TYPE_MCBIST) &&
                    (T != fapi2::TARGET_TYPE_MEM_PORT) &&
                    (T != fapi2::TARGET_TYPE_MCA) &&
                    (T != fapi2::TARGET_TYPE_MCS)),
                  "improper child of fapi2::TARGET_TYPE_MCBIST");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_mi_valid_children()
{
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
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_dmi_valid_children()
{
    // valid children for DMI
    // DMI -> MEMBUF
    static_assert(!((K == fapi2::TARGET_TYPE_DMI) &&
                    (T != fapi2::TARGET_TYPE_MEMBUF_CHIP)),
                  "improper child of fapi2::TARGET_TYPE_DMI");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_omi_valid_children()
{
    // valid children for OMI
    // OMI -> OCMB
    static_assert(!((K == fapi2::TARGET_TYPE_OMI) &&
                    (T != fapi2::TARGET_TYPE_OCMB_CHIP)),
                  "improper child of fapi2::TARGET_TYPE_OMI");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_omic_valid_children()
{
    // valid children for OMIC
    // OMIC -> OMI
    static_assert(!((K == fapi2::TARGET_TYPE_OMIC) &&
                    (T != fapi2::TARGET_TYPE_OMI)),
                  "improper child of fapi2::TARGET_TYPE_OMIC");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_mcc_valid_children()
{
    // valid children for MCC
    // MCC -> OMI
    static_assert(!((K == fapi2::TARGET_TYPE_MCC) &&
                    (T != fapi2::TARGET_TYPE_OMI)),
                  "improper child of fapi2::TARGET_TYPE_MCC");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_mba_valid_children()
{
    // valid children for MBA
    // MBA -> DIMM
    static_assert(!((K == fapi2::TARGET_TYPE_MBA) &&
                    (T != fapi2::TARGET_TYPE_DIMM)),
                  "improper child of fapi2::TARGET_TYPE_MBA");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_mc_valid_children()
{
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
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_obus_valid_children()
{
    // valid children for OBUS
    // OBUS -> OBUS_BRICK
    // OBUS -> OBUS
    static_assert(!((K == fapi2::TARGET_TYPE_OBUS) &&
                    (T != fapi2::TARGET_TYPE_OBUS_BRICK) &&
                    (T != fapi2::TARGET_TYPE_OBUS)),
                  "improper child of fapi2::TARGET_TYPE_OBUS");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_pauc_valid_children()
{
    // valid children for PAUC
    // PAUC -> IOHS
    // PAUC -> PAU
    // PAUC -> OMIC
    static_assert(!((K == fapi2::TARGET_TYPE_PAUC) &&
                    (T != fapi2::TARGET_TYPE_IOHS) &&
                    (T != fapi2::TARGET_TYPE_PAU) &&
                    (T != fapi2::TARGET_TYPE_OMIC)),
                  "improper child of fapi2::TARGET_TYPE_PAUC");
}

template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_iohs_valid_children()
{
    // valid children for IOHS
    // IOHS -> IOLINK
    static_assert(!((K == fapi2::TARGET_TYPE_IOHS) &&
                    (T != fapi2::TARGET_TYPE_IOLINK)),
                  "improper child of fapi2::TARGET_TYPE_IOHS");
}


template< fapi2::TargetType K, fapi2::TargetType T >
constexpr void _check_valid_unicast_children()
{
    fapi2plat::_check_proc_chip_valid_children<K, T>();
    fapi2plat::_check_perv_valid_children<K, T>();
    fapi2plat::_check_membuf_chip_valid_children<K, T>();
    fapi2plat::_check_ocmb_chip_valid_children<K, T>();
    fapi2plat::_check_system_valid_children<K, T>();
    fapi2plat::_check_eq_valid_children<K, T>();
    fapi2plat::_check_ex_valid_children<K, T>();
    fapi2plat::_check_mcs_valid_children<K, T>();
    fapi2plat::_check_mem_port_valid_children<K, T>();
    fapi2plat::_check_mca_valid_children<K, T>();
    fapi2plat::_check_mcbist_valid_children<K, T>();
    fapi2plat::_check_mi_valid_children<K, T>();
    fapi2plat::_check_dmi_valid_children<K, T>();
    fapi2plat::_check_omi_valid_children<K, T>();
    fapi2plat::_check_omic_valid_children<K, T>();
    fapi2plat::_check_mcc_valid_children<K, T>();
    fapi2plat::_check_mba_valid_children<K, T>();
    fapi2plat::_check_mc_valid_children<K, T>();
    fapi2plat::_check_obus_valid_children<K, T>();
    fapi2plat::_check_pauc_valid_children<K, T>();
    fapi2plat::_check_iohs_valid_children<K, T>();
}

#endif

}

#endif //  __FAPI2_PLAT_TARGET_RULES__
