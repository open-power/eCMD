# Makefile for the ecmd Extensions

# *****************************************************************************
# Include the common base makefile
# *****************************************************************************
include ${ECMD_ROOT}/makefile.base

EXTENSION_NAME   := fapi2
DISABLE_AUTOGEN := @
DEFINES := ${DEFINES}  -DFAPIARCHIVE -DFAPI_SUPPORT_SPY_AS_STRING -DFAPI_2 -DFAPI_2_Z
#DEFINES := ${DEFINES}  -DFAPIARCHIVE -DFAPI_SUPPORT_SPY_AS_STRING -DFAPI_SUPPORT_MULTI_SCOM -DFAPI_2

SCRIPTS := scripts/fapi2CreateAttrGetSetMacros.pl
SCRIPTS += scripts/parseAttributeInfo.pl
SCRIPTS += scripts/parseErrorInfo.pl

# Additional Source files for the Client API
CAPI_SOURCE := plat_utils.C
CAPI_SOURCE += plat_hw_access.C
CAPI_SOURCE += plat_hw_access_multicast.C
CAPI_SOURCE += plat_i2c_access.C
CAPI_SOURCE += plat_mmio_access.C
CAPI_SOURCE += plat_vpd_access.C
CAPI_SOURCE += plat_spd_access.C
CAPI_SOURCE += plat_mbvpd_access.C
CAPI_SOURCE += plat_file_access.C
CAPI_SOURCE += hwp_executor.C
CAPI_SOURCE += fapi2PlatSystemConfig.C
CAPI_SPECIAL_SOURCE = tls_storage.C

# Added for fapi2 source files
CAPI_INCLUDES := fapi2_target.H
CAPI_INCLUDES += plat_target.H
CAPI_INCLUDES += target.H
CAPI_INCLUDES += target_types.H
CAPI_INCLUDES += target_states.H
CAPI_INCLUDES += plat_target_rules.H
CAPI_INCLUDES += multicast_defs.H
CAPI_INCLUDES += buffer_parameters.H
CAPI_INCLUDES += buffer_traits.H
CAPI_INCLUDES += buffer.H
CAPI_INCLUDES += variable_buffer.H
CAPI_INCLUDES += variable_buffer_utils.H
CAPI_INCLUDES += fapi2_error_scope.H
CAPI_INCLUDES += plat_error_scope.H
CAPI_INCLUDES += error_scope.H
CAPI_INCLUDES += plat_trace.H
CAPI_INCLUDES += fapi2_hw_access.H
CAPI_INCLUDES += hw_access.H
CAPI_INCLUDES += plat_hw_access.H
CAPI_INCLUDES += hw_access_def.H
CAPI_INCLUDES += fapi2_i2c_access.H
CAPI_INCLUDES += i2c_access.H
CAPI_INCLUDES += plat_i2c_access.H
CAPI_INCLUDES += fapi2_mmio_access.H
CAPI_INCLUDES += mmio_access.H
CAPI_INCLUDES += plat_mmio_access.H
CAPI_INCLUDES += mvpd_access.H
CAPI_INCLUDES += mvpd_access_defs.H
CAPI_INCLUDES += ffdc.H
CAPI_INCLUDES += error_info.H
CAPI_INCLUDES += error_info_defs.H
CAPI_INCLUDES += collect_reg_ffdc.H
CAPI_INCLUDES += return_code.H
CAPI_INCLUDES += return_code_defs.H
CAPI_INCLUDES += hwp_return_codes.H
CAPI_INCLUDES += hwp_error_info.H
CAPI_INCLUDES += hwp_ffdc_classes.H
CAPI_INCLUDES += set_sbe_error.H
CAPI_INCLUDES += utils.H
CAPI_INCLUDES += plat_utils.H
CAPI_INCLUDES += plat_types.H
CAPI_INCLUDES += fapi2SharedUtils.H
CAPI_INCLUDES += fapi2DllCapi.H
CAPI_INCLUDES += fapi2ClientEnums.H
CAPI_INCLUDES += fapi2_hwp_executor.H
CAPI_INCLUDES += hwp_executor.H
CAPI_INCLUDES += fapi2_attribute_service.H
CAPI_INCLUDES += plat_attribute_service.H
CAPI_INCLUDES += fapi2_vpd_access.H
CAPI_INCLUDES += plat_vpd_access.H
CAPI_INCLUDES += vpd_access.H
CAPI_INCLUDES += vpd_access_defs.H
CAPI_INCLUDES += fapi2_spd_access.H
CAPI_INCLUDES += plat_spd_access.H
CAPI_INCLUDES += fapi2_file_access.H
CAPI_INCLUDES += file_access.H
CAPI_INCLUDES += plat_file_access.H
CAPI_INCLUDES += fapi2_subroutine_executor.H
CAPI_INCLUDES += subroutine_executor.H
CAPI_INCLUDES += fapi2_mbvpd_access.H
CAPI_INCLUDES += fapi2_mbvpd_access_defs.H
CAPI_INCLUDES += plat_mbvpd_access.H

# fapiMultiScom.H

# These files are needed by the client code
CAPI_INCLUDES += fapi2.H
CAPI_INCLUDES += attribute_ids.H
CAPI_INCLUDES += fapi2_attribute_service.C
CAPI_INCLUDES += fapi2_chip_ec_feature.H

# Additional Source files for the Shared Lib
SLIB_SOURCE := ffdc.C
SLIB_SOURCE += error_info.C
SLIB_SOURCE += plat_utils_assert.C

# Include the common extension makefile to do all the work
# include ../../template/capi/makefile.capi

# For this extension we're including a local makefile since it needed to be modified to link to libecmd
include makefile.capi

