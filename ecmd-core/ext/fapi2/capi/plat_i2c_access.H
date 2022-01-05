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

/// @file plat_i2c_access.H
///
/// @brief Platform I2C access definitions
///

#ifndef _FAPI2_PLAT_I2C_ACCESS_H_
#define _FAPI2_PLAT_I2C_ACCESS_H_

#include <stdint.h>
#include <vector>

#include <return_code.H>
#include <target.H>

namespace fapi2plat
{
    fapi2::ReturnCode getI2c(const ecmdChipTarget& i_target,
                             const size_t i_get_size,
                             const std::vector<uint8_t>& i_data,
                             std::vector<uint8_t>& o_data);

    fapi2::ReturnCode putI2c(const ecmdChipTarget& i_target,
                             const std::vector<uint8_t>& i_data);

    template< fapi2::TargetType K, fapi2::MulticastType M, typename V >
    inline fapi2::ReturnCode getI2c(const fapi2::Target<K, M, V>& i_target,
                                    const size_t i_get_size,
                                    const std::vector<uint8_t>& i_data,
                                    std::vector<uint8_t>& o_data)
    {
        static_assert(((K == fapi2::TARGET_TYPE_OCMB_CHIP) || (K == fapi2::TARGET_TYPE_PMIC) ||
                       (K == fapi2::TARGET_TYPE_GENERICI2CRESPONDER) || (K == fapi2::TARGET_TYPE_MDS_CTLR)), 
                      "getI2C requires TARGET_TYPE_OCMB_CHIP, TARGET_TYPE_PMIC, TARGET_TYPE_GENERICI2CRESPONDER, or TARGET_TYPE_MDS_CTLR");
        ecmdChipTarget ecmdTarget;
        fapiTargetToEcmdTarget(i_target, ecmdTarget);
        return fapi2plat::getI2c(ecmdTarget, i_get_size, i_data, o_data);
    }

    template< fapi2::TargetType K, fapi2::MulticastType M, typename V >
    inline fapi2::ReturnCode putI2c(const fapi2::Target<K, M, V>& i_target,
                                    const std::vector<uint8_t>& i_data)
    {
        static_assert(((K == fapi2::TARGET_TYPE_OCMB_CHIP) || (K == fapi2::TARGET_TYPE_PMIC) ||
                       (K == fapi2::TARGET_TYPE_GENERICI2CRESPONDER) || (K == fapi2::TARGET_TYPE_MDS_CTLR)), 
                      "putI2C requires TARGET_TYPE_OCMB_CHIP, TARGET_TYPE_PMIC, TARGET_TYPE_GENERICI2CRESPSONDER, or TARGET_TYPE_MDS_CTLR");
        ecmdChipTarget ecmdTarget;
        fapiTargetToEcmdTarget(i_target, ecmdTarget);
        return fapi2plat::putI2c(ecmdTarget, i_data);
    }

};

#endif // _FAPI2_PLAT_I2C_ACCESS_H_

