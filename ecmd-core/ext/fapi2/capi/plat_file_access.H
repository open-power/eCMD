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

/// @file plat_file_access.H
///
/// @brief Platform file access definitions
///

#ifndef _FAPI2_PLAT_FILE_ACCESS_H_
#define _FAPI2_PLAT_FILE_ACCESS_H_

#include <stdint.h>

#include <return_code.H>
#include <target.H>

namespace fapi2plat
{
    fapi2::ReturnCode loadEmbeddedFile(const ecmdChipTarget& i_target,
                                       const char* i_path,
                                       const void*& o_data, 
                                       size_t& o_size,
                                       uint32_t i_flags);

    fapi2::ReturnCode freeEmbeddedFile(const void* i_data);


    inline fapi2::ReturnCode loadEmbeddedFile(const fapi2::Target<fapi2::TARGET_TYPE_ANY_POZ_CHIP>& i_target,
                                    const char* i_path,
                                    const void*& o_data,
                                    size_t& o_size,
                                    uint32_t i_flags)
    {
        ecmdChipTarget ecmdTarget;
        fapiTargetToEcmdTarget(i_target, ecmdTarget);
        return fapi2plat::loadEmbeddedFile(ecmdTarget, i_path, o_data, o_size, i_flags);
    }

    /***** Old versions *******/
    // These can possibly be removed in next release
    fapi2::ReturnCode freeEmbeddedFile(void* i_data);

    fapi2::ReturnCode loadEmbeddedFile(const ecmdChipTarget& i_target,
                                       const char* i_path,
                                       void*& o_data, 
                                       size_t& o_size);

    inline fapi2::ReturnCode loadEmbeddedFile(const fapi2::Target<fapi2::TARGET_TYPE_ANY_POZ_CHIP>& i_target,
                                    const char* i_path,
                                    void*& o_data,
                                    size_t& o_size)
    {
        ecmdChipTarget ecmdTarget;
        fapiTargetToEcmdTarget(i_target, ecmdTarget);
        return fapi2plat::loadEmbeddedFile(ecmdTarget, i_path, o_data, o_size);
    }

};

#endif // _FAPI2_PLAT_FILE_ACCESS_H_

