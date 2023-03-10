/* IBM_PROLOG_BEGIN_TAG                                                   */
/* 
 * Copyright 2022 IBM International Business Machines Corp.
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
///
/// @file fapi2_file_access.H
///
/// @brief Access to the embedded archive
///

#pragma once

#include <stdint.h>
#include <return_code.H>
#include <target.H>

namespace fapi2
{

enum loadEmbeddedFileFlags
{
    // No flags defined at the moment, just for future expansion
};

/// @brief Load a file from the embedded archive
/// Attempts to locate a file with given name inside the embedded archive
/// and makes it available in memory.
/// @param[in]  i_target Chip target associated with the access;
///                      platforms may use this to pick an archive to search
/// @param[in]  i_path   Full path name of the file to look for
/// @param[out] o_data   Will point to the file data on success
/// @param[out] o_size   Will receive the file size on success
/// @param[in]  i_flags  Optional flags to affect the operation
/// @return FAPI2_RC_SUCCESS on success,
///         FAPI2_RC_FILE_NOT_FOUND if the file was not found, or another error.
ReturnCode loadEmbeddedFile(const Target<TARGET_TYPE_ANY_POZ_CHIP>& i_target,
                            const char* i_path,
                            const void*& o_data, size_t& o_size,
                            uint32_t i_flags = 0);

/// @brief Free memory allocated by loadEmbeddedFile()
/// @param[in] i_data  Pointer to file contents
/// @return FAPI2_RC_SUCCESS on success, else error code
/// @note freeEmbeddedFile() must be called only if the corresponding call to
///       loadEmbeddedFile() was successful
/// @note If more than 1 file is loaded in memory with loadEmbeddedFile(), then
///       freeEmbeddedFile() must be called in a last-in-first-out (LIFO) order.
///       Calling freeEmbeddedFile () in a random order can lead to unexpected
//        behavior
ReturnCode freeEmbeddedFile(const void* i_data);

}

