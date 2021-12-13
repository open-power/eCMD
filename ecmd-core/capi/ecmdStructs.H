//IBM_PROLOG_BEGIN_TAG
/* 
 * Copyright 2003,2016 IBM International Business Machines Corp.
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

#ifndef ecmdStructs_H
#define ecmdStructs_H
/**
  @file ecmdStructs.H
  @brief All the Structures required for the eCMD Capi
*/

//--------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------
#include <stdlib.h>
#include <inttypes.h>
#include <list>  /* For STL list */
#include <vector>
#include <string>
#include <map>

#include <ecmdDefines.H>
#include <ecmdDataBuffer.H>

//--------------------------------------------------------------------
//  Forward References                                                
//--------------------------------------------------------------------

//--------------------------------------------------------------------
//  Defines                                                
//--------------------------------------------------------------------

#define ECMD_CAPI_VERSION "15.0"          ///< eCMD API Version

// Magic headers for packing/unpacking the ecmdQueryData structure heirarchy.
#define QD_HDR_MAGIC     0xFFFFFFF1
#define CAGE_HDR_MAGIC   0xFFFFFF2F
#define NODE_HDR_MAGIC   0xFFFFF3FF
#define SLOT_HDR_MAGIC   0xFFFF4FFF
#define CHIP_HDR_MAGIC   0xFFF5FFFF
#define THREAD_HDR_MAGIC 0xF7FFFFFF
#define CHIPUNIT_HDR_MAGIC 0x8FFFFFFF
/**
 Predefined common chip names for ecmdChipData.chipCommonType
*/
#define ECMD_CHIPT_PROCESSOR            "pu"
#define ECMD_CHIPT_MEM_BUF              "memb"
#define ECMD_CHIPT_MEM_CNTRL            "memc"
#define ECMD_CHIPT_MEM_FPGA             "memf"
#define ECMD_CHIPT_MEM_CACHE            "cache"
#define ECMD_CHIPT_MEM_L2CACHE          "l2cache"
#define ECMD_CHIPT_MEM_L3CACHE          "l3cache"
#define ECMD_CHIPT_IOBDG                "iobdg"
#define ECMD_CHIPT_IOHUB                "iohub"
#define ECMD_CHIPT_MUX                  "mux"
#define ECMD_CHIPT_SERVICE_PROCESSOR    "sp"
#define ECMD_CHIPT_IOU                  "iou"
#define ECMD_CHIPT_NOCHIP               "nochip"

#if !defined(ECMD_PERLAPI) && !defined(ECMD_PYAPI)
#define ECMD_UNSET          0xFFFFFFFF ///< Used to signify unset or -1 for uint32_t types


#define ECMD_TARGETDEPTH_NA 0xFFFFFFFF ///< Used to signify a level of the ecmdChipTarget depth is not used (only valid for node/slot depths)
#endif

#define ECMD_ARG_LIMIT      100        ///< Used in a few spots set the limit of command line args

//--------------------------------------------------------------------
//  TypeDefs                                               
//--------------------------------------------------------------------

/**
 @brief Used by ecmdCacheFlush to specify which level of cache to flush
*/
typedef enum {
  ECMD_CACHE_UNKNOWN = 0,       ///< Unknown Cache Type
  ECMD_CACHE_LEVEL1D,           ///< L1 Data Cache 
  ECMD_CACHE_LEVEL1I,           ///< L1 Instruction Cache 
  ECMD_CACHE_LEVEL2,            ///< L2 Cache 
  ECMD_CACHE_LEVEL3,            ///< L3 Cache 
  ECMD_CACHE_LEVEL4,            ///< L4 Cache 
} ecmdCacheType_t;

/**
 @brief Used in ecmdChipData to describe the interface macro used by the chip
*/
typedef enum {
  ECMD_INTERFACE_UNKNOWN,               ///< Unknown Interface
  ECMD_INTERFACE_ACCESS,                ///< Standard Jtag Access Macro
  ECMD_INTERFACE_CFAM                   ///< CommonFirmwareAccessMacro 
} ecmdChipInterfaceType_t;

/**
 @brief Used by ecmdChipTarget to describe the value in the state fields
*/

typedef enum {
  ECMD_TARGET_UNKNOWN_STATE,    ///< State field has not been initialized
  ECMD_TARGET_FIELD_VALID,      ///< Associated State Field is set to a valid value
  ECMD_TARGET_FIELD_UNUSED,     ///< Associated State Field is unused and should be ignored
  ECMD_TARGET_FIELD_WILDCARD,   ///< Associated State Field is a wildcard and should be iterated on in query functions
  ECMD_TARGET_THREAD_ALIVE,     ///< Used when calling thread dependent functions tell the function to check for the thread to be alive before running
} ecmdChipTargetState_t;

/**
 @brief Used by SetClockSpeed interfaces to adjust clock steering procedure
 */
typedef enum {
  ECMD_CLOCK_RANGE_UNKNOWN,
  ECMD_CLOCK_RANGE_DEFAULT,
  ECMD_CLOCK_RANGE_LOWEST,
  ECMD_CLOCK_RANGE_LOW,
  ECMD_CLOCK_RANGE_MIDDLE,
  ECMD_CLOCK_RANGE_HIGH,
  ECMD_CLOCK_RANGE_HIGHEST
} ecmdClockRange_t;

/**
 @brief Used by SetClockSpeed interfaces to adjust clock steering procedure
 */
typedef enum {
  ECMD_CLOCK_SINGLE_FREQ_MODE,
  ECMD_CLOCK_MINMAX_FREQ_MODE,
} ecmdClockFreqMode_t;

/**
 @brief Used by SetClockSpeed interfaces to specify to do adjustment in one operation or to steer to new value
*/
typedef enum {
  ECMD_CLOCK_UNKNOWN,           ///< Default for not initialized
  ECMD_CLOCK_ONE_STEP,          ///< Change to new frequency in one operation
  ECMD_CLOCK_STEER              ///< Steer to new frequency
} ecmdClockSetMode_t;

/**
 @brief Used by SetClockSpeed interfaces to specify what notation the speed is provided in
 */
typedef enum {
  ECMD_CLOCK_FREQUENCY_UNKNOWN,      ///< Default for not initialized
  ECMD_CLOCK_FREQUENCY_KHZ_SPEC,        ///< Clock speed is specified in Khz
  ECMD_CLOCK_FREQUENCY_MHZ_SPEC,        ///< Clock speed is specified in Mhz
  ECMD_CLOCK_CYCLETIME_PS_SPEC,         ///< Clock speed is specified in picoseconds cycle time
  ECMD_CLOCK_FREQUENCY_SPEC = ECMD_CLOCK_FREQUENCY_MHZ_SPEC,    ///< Clock speed is specified in Mhz
  ECMD_CLOCK_CYCLETIME_SPEC = ECMD_CLOCK_CYCLETIME_PS_SPEC,     ///< Clock speed is specified in picoseconds cycle time
  ECMD_CLOCK_NOMINAL_PERCENT_UP,     ///< Clock speed specified in % above nominal
  ECMD_CLOCK_NOMINAL_PERCENT_DOWN,   ///< Clock speed specified in % below nominal
  ECMD_CLOCK_CURRENT_PERCENT_UP,     ///< Clock speed specified in % above current value
  ECMD_CLOCK_CURRENT_PERCENT_DOWN,   ///< Clock speed specified in % below current value
  ECMD_CLOCK_POWERSAVE_PERCENT_UP,   ///< Clock speed specified in % above powersave value
  ECMD_CLOCK_POWERSAVE_PERCENT_DOWN, ///< Clock speed specified in % below powersave value
  ECMD_CLOCK_TURBO_PERCENT_UP,       ///< Clock speed specified in % above turbo value  
  ECMD_CLOCK_TURBO_PERCENT_DOWN,      ///< Clock speed specified in % below turbo value  
  ECMD_CLOCK_SUPER_TURBO_PERCENT_UP,   ///< Clock speed specified in % above super turbo value  
  ECMD_CLOCK_SUPER_TURBO_PERCENT_DOWN ///< Clock speed specified in % below super turbo value  
} ecmdClockSpeedType_t;

/**
 @brief Used by ecmdBiasVoltage interfaces to specify what notation the voltage is provided in
 */
typedef enum {
  ECMD_VOLTAGE_UNKNOWN,                 ///< Default
  ECMD_VOLTAGE_NOMINAL,                 ///< Set the voltaget to nominal
  ECMD_VOLTAGE_PERCENT_DOWN,            ///< Voltage in % below nominal
  ECMD_VOLTAGE_PERCENT_UP,              ///< Voltage in % above nominal value
  ECMD_VOLTAGE_POWERSAVE_PERCENT_DOWN,  ///< Voltage in % below powersave value
  ECMD_VOLTAGE_POWERSAVE_PERCENT_UP     ///< Voltage in % above powersave value
} ecmdVoltageType_t;

/**
 @brief Used by Ring/Array/Spy Query functions to return a required clock state
*/
typedef enum {
  ECMD_CLOCKSTATE_UNKNOWN,              ///< Unable to determine a required clock state
  ECMD_CLOCKSTATE_ON,                   ///< Chip clocks must be on to access
  ECMD_CLOCKSTATE_OFF,                  ///< Chip clocks must be off to access
  ECMD_CLOCKSTATE_NA                    ///< Chip clocks can be in any state to access
} ecmdClockState_t;

/**
 @brief Used by scomQuery function to return endian mode of data
*/
typedef enum {
  ECMD_UNKNOWN_ENDIAN,                  ///< Unable to determine the endianness of the data
  ECMD_BIG_ENDIAN,                      ///< The data for this chip is big endian
  ECMD_LITTLE_ENDIAN,                   ///< The data for this chip is little endian
} ecmdEndianMode_t;

/**
 @brief Used by SetClockSpeed interfaces to specify the clock to control
 */
typedef enum {
  ECMD_CLOCKTYPE_UNKNOWN,               ///< Default for not initialized
  ECMD_PROC_REFCLOCK,                   ///< Processor reference clock
  ECMD_MEMCTRL_REFCLOCK,                ///< Memory Controller reference clock
  ECMD_PROC_CORE_CLOCK,                 ///< Processor Core Clock (actual speed of the domain)
  ECMD_PROC_NEST_CLOCK,                 ///< Processor Nest Clock (actual speed of the domain)
  ECMD_MEMCTRL_CLOCK,                   ///< Memory Controller Clock (actual speed of the chip)
  ECMD_IO_REFCLOCK,                     ///< IO Chip reference clock
  ECMD_IO_CLOCK,                        ///< IO Chip Clock (actual speed of the chip)
  ECMD_GX_REFCLOCK,                     ///< GX reference Clock
  ECMD_DRAM_FREQ,                       ///< DRAM Frequency
  ECMD_PROC_CORE_DPLL_CLOCK,            ///< Processor Core DPLL Clock (actual speed of the domain)
} ecmdClockType_t;

/**
 @brief Used by ecmdConfigLooperInit to enable/disable variable depth looping
*/
typedef enum {
  ECMD_LOOP_MODE_UNKNOWN,               ///< Default for not initialized
  ECMD_DYNAMIC_LOOP,                    ///< Uses the value of ECMD_GLOBALVAR_LOOPMODE to determine loop mode.  Default is ECMD_CONFIG_LOOP
  ECMD_CONFIG_LOOP,                     ///< Use the ecmdConfigLooper* functions
  ECMD_EXIST_LOOP,                      ///< Use the ecmdExistLooper* functions
  ECMD_DYNAMIC_REVERSE_LOOP,            ///< Uses the value of ECMD_GLOBALVAR_LOOPMODE to determine loop mode, then set it to the reverse version
  ECMD_CONFIG_REVERSE_LOOP,             ///< Use the ecmdConfigLooper* functions, but reverse the order of the data after the init
  ECMD_EXIST_REVERSE_LOOP,              ///< Use the ecmdExistLooper* functions, but reverse the order of the data after the init
} ecmdLoopMode_t;

/**
 @brief Used by ecmdConfigLooperInit function to specify what type of data to loop on

 Using one of the 4 ..SELECTED_TARGETS.. enumerations require that ecmdCommandArgs is called before ecmdLooperInit()
 to parse the eCMD target parms (-p#, -c#, etc..) from the command line.
*/
typedef enum {
  ECMD_LOOP_TYPE_UNKNOWN,               ///< Default for not initialized
  ECMD_SELECTED_TARGETS_LOOP,           ///< Loop on only targets in the system the user specified with -p# -c# -n#, etc. if not specified default to 0
  ECMD_SELECTED_TARGETS_LOOP_DEFALL,    ///< Loop on only targets in the system the user specified with -p# -c# -n#, etc. if not specified default to all
  ECMD_SELECTED_TARGETS_LOOP_VD,        ///< Loop only on targets in the system to the depth user specified on command line (ie if user said only '-n0' then -s and below are unused) if not specified default to 0
  ECMD_SELECTED_TARGETS_LOOP_VD_DEFALL, ///< Loop only on targets in the system to the depth user specified on command line (ie if user said only '-n0' then -s and below are unused) if not specified default to all
  ECMD_ALL_TARGETS_LOOP                 ///< Loop on all valid targets in the system
} ecmdLoopType_t;


/**
 @brief Used by the get/set configuration functions to specify what data is good
*/
typedef enum {
  ECMD_CONFIG_VALID_FIELD_NONE,         ///< No field is valid, must have been an error
  ECMD_CONFIG_VALID_FIELD_ALPHA,        ///< The string field contains valid data
  ECMD_CONFIG_VALID_FIELD_NUMERIC,      ///< The numeric field contains valid data
  ECMD_CONFIG_VALID_FIELD_BOTH          ///< Bothe the string and numeric fields contain valid data
} ecmdConfigValid_t;

/**
 @brief Used by the GPIO functions to specify the different modes for the GPIO pin

             |             High (1)            |             Low (0)             |
             | Output Enable | Output Register | Output Enable | Output Register |
 Push Pull   |     true      |       1         |     true      |       0         |
 Open Drain  |    false      |       0         |     true      |       0         |
 Open Source |     true      |       1         |    false      |       1         |
*/
typedef enum {
  ECMD_DIO_UNKNOWN,                     ///< Default for not initialized
  ECMD_DIO_INPUT,                       ///< Input pin
  ECMD_DIO_OPEN_DRAIN,                  ///< See detailed table
  ECMD_DIO_OPEN_SOURCE,                 ///< See detailed table
  ECMD_DIO_PUSH_PULL                    ///< See detailed table
} ecmdDioMode_t;


/**
 @brief This is used by ecmdQueryDllInfo to return what environment the dll is designed to run in (i.e Simulation vs Hardware)
*/
typedef enum {
  ECMD_DLL_ENV_UNKNOWN,         ///< Default for not initialized
  ECMD_DLL_ENV_HW,              ///< Hardware Environment
  ECMD_DLL_ENV_SIM              ///< Simulation Environment
} ecmdDllEnv_t;

/**
 @brief Used for the ecmdQueryFileLocation function to specify the file type you are looking for
*/
typedef enum {
  ECMD_FILE_UNKNOWN,    ///< Default for not initialized
  ECMD_FILE_SCANDEF,    ///< Scandef file type
  ECMD_FILE_SPYDEF,     ///< Spy Definition file
  ECMD_FILE_ARRAYDEF,   ///< Array Definition file
  ECMD_FILE_HELPTEXT,   ///< eCMD Help Text file - target field of ecmdQueryFileLocation is not used for this and just a path is returned
  ECMD_FILE_SCOMDATA,   ///< eCMD ScanComm Parse data files, used by getscom - target field of ecmdQueryFileLocation is not used for this and just a path is returned
  ECMD_FILE_SPYDEFHASH, ///< Hash file for spy definition
  ECMD_FILE_SCANDEFHASH ///< Hash file for the scandef
} ecmdFileType_t;

/**
 @brief Used by ecmdGetGlobalVar to specify what variable you are looking for
*/
typedef enum {
  ECMD_GLOBALVAR_UNKNOWN,               ///< Default for not initialized
  ECMD_GLOBALVAR_DEBUG,                 ///< Retrieve the value of the ecmd debug flag set by ECMD_DEBUG env var
  ECMD_GLOBALVAR_QUIETMODE,             ///< Retrieve the value of the quiet mode debug flag = set by -quiet default = 0
  ECMD_GLOBALVAR_COEMODE,               ///< Retrieve the value of the continueOnError  mode  flag = set by -coe default = 0
  ECMD_GLOBALVAR_QUIETERRORMODE,        ///< Retrieve the value of the quiet error mode debug flag. Set by -quieterror
  ECMD_GLOBALVAR_LOOPMODE,              ///< Retrieve the value of the loopMode mode flag set by -exist default = ECMD_CONFIG_LOOP
  ECMD_GLOBALVAR_CMDLINEMODE,           ///< Retrieve the value of the cmdLine mode value.  cmdline default = 1, other = 0
} ecmdGlobalVarType_t;

/**
 @brief Used by I2C functions to specify bus speed
*/
typedef enum {
  ECMD_I2C_BUSSPEED_UNKNOWN,    ///< Default for not initialized
  ECMD_I2C_BUSSPEED_50KHZ,      ///< Run I2c bus at 50Khz
  ECMD_I2C_BUSSPEED_100KHZ,     ///< Run I2c bus at 100Khz
  ECMD_I2C_BUSSPEED_400KHZ,     ///< Run I2c bus at 400Khz
  ECMD_I2C_BUSSPEED_1MHZ,       ///< Run I2c bus at 1Mhz
} ecmdI2cBusSpeed_t;

/**
 @brief Used by I2C functions to specify the contents of the eCMD i2c flags parameter
*/
#define ECMD_I2C_FLAGS_I2C_SLAVE_FORCE         0x00000001


/**
 @brief Used by ecmdGpioWriteConfigRegister to specify the type of operation to do
*/
typedef enum {
  ECMD_GPIO_UNKNOWN,            ///< Default for not initialized
  ECMD_GPIO_CONFIG_WRITE,       ///< Set the current value of i_configReg to i_value
  ECMD_GPIO_SET_BIT,            ///< Set each bit of  i_configReg that is true in i_value. 
  ECMD_GPIO_CLEAR_BIT           ///< Clear each bit of i_configReg that is set to true in i_value. 
} ecmdGpioWriteMode_t;

/**
 @brief Used by get/putLatch functions to specify what mode should be used to find latches in the scandef
*/
typedef enum {
  ECMD_LATCHMODE_UNKNOWN,               ///< Default for not initialized
  ECMD_LATCHMODE_FULL,                  ///< Latch must match exactly
  ECMD_LATCHMODE_PARTIAL,               ///< Latch can be a partial match
  ECMD_LATCHMODE_PARTIAL_NO_RING_CHECK  ///< Latch can be a partial match - bypasses check for latches within multiple rings
} ecmdLatchMode_t;

/**
 @brief Used by readScandef functions to specify what type of latch was found in the scandef
*/
typedef enum {
  ECMD_LATCHTYPE_UNKNOWN,       ///< Default for not initialized
  ECMD_LATCHTYPE_NOBIT,         ///< Latch was specified with out a position, 0 is implied
  ECMD_LATCHTYPE_SINGLEBIT,     ///< Latch was a single bit entry (4)
  ECMD_LATCHTYPE_MULTIBIT,      ///< Latch was multiple bits (3:5)
  ECMD_LATCHTYPE_ARRAY          ///< Latch was an Array Latch (8,28)
} ecmdLatchType_t;

/**
 @brief Used by ecmdQueryConfig to specify detail level of query
*/
typedef enum {
  ECMD_QUERY_DETAIL_UNKNOWN,         ///< Default for not initialized
  ECMD_QUERY_DETAIL_LOW,             ///< Only config info is returned
  ECMD_QUERY_DETAIL_HIGH             ///< All info is returned
} ecmdQueryDetail_t;

/**
 @brief Used for the ecmdQuerySpy function to specify which type of spy we have @see ecmdSpyData
*/
typedef enum {
  ECMD_SPYTYPE_UNKNOWN, ///< Default for not initialized
  ECMD_SPYTYPE_ALIAS,   ///< Spy is an alias
  ECMD_SPYTYPE_IDIAL,   ///< Spy is an iDial
  ECMD_SPYTYPE_EDIAL,   ///< Spy is an eDial
  ECMD_SPYTYPE_ECCGROUP ///< Spy is an eccGrouping
} ecmdSpyType_t;

/**
 @brief Used by ecmdSetTraceMode to specify which trace to control
*/
typedef enum {
  ECMD_TRACE_UNKNOWN,           ///< Default for not initialized
  ECMD_TRACE_SCAN,              ///< Scan Trace
  ECMD_TRACE_PROCEDURE          ///< Procedure Trace
} ecmdTraceType_t;

/**
 @brief Used by ecmdI2CCmdEntry to specify which commands to run
*/
typedef enum {
  ECMD_I2C_UNKNOWN,        ///< Default for not initialized
  ECMD_I2C_RESET,          ///< Call should go ecmdI2cReset
  ECMD_I2C_READ,           ///< Call should go ecmdI2cRead
  ECMD_I2C_READOFFSET,     ///< Call should go ecmdI2cReadOffset
  ECMD_I2C_WRITE,          ///< Call should go ecmdI2cWrite
  ECMD_I2C_WRITEOFFSET     ///< Call should go ecmdI2cWriteOffset
} ecmdI2CCmds_t;

/**
 @brief Used for the ecmdQueryArray function to specify which type of array we have @see ecmdArrayData
*/
typedef enum {
  ECMD_ARRAYTYPE_UNKNOWN,               ///< Default for not initialized
  ECMD_ARRAYTYPE_DIRECT_ACCESS,         ///< Array is a direct access array
  ECMD_ARRAYTYPE_SIMPLE,                ///< Array is a simple array
  ECMD_ARRAYTYPE_HARDWARE_ASSIST,       ///< Array is a hardware assist array
  ECMD_ARRAYTYPE_HARDWARE_ASSIST_2,     ///< Array is a hardware assist 2 array
  ECMD_ARRAYTYPE_HARDWARE_ASSIST_3,     ///< Array is a hardware assist 3 array
  ECMD_ARRAYTYPE_SCOM_ONE,              ///< Array is a scom one 
  ECMD_ARRAYTYPE_SCOM_TWO,              ///< Array is a scom two 
  ECMD_ARRAYTYPE_SCOM_ONE_RETRY,        ///< Array is a scom one with retry attempts

} ecmdArrayType_t;

/**
 @brief Used by ecmdProcRegisterInfo to specify read/write mode
*/
typedef enum {
  ECMD_PROCREG_UNKNOWN,         ///< The unknown state for this enum, not a valid output type
  ECMD_PROCREG_READ_AND_WRITE,  ///< Register can be written and read
  ECMD_PROCREG_WRITE_ONLY,      ///< Register can only be written 
  ECMD_PROCREG_READ_ONLY        ///< Register can only be read 
} ecmdProcRegMode_t;

/**
 @brief Used by ecmdScomEntry to specify scom multiple d/a type
*/
typedef enum {
  ECMD_SCOM_UNKNOWN,            ///< The unknown state for this enum, not a valid output type
  ECMD_GETSCOM_OP,              ///< Do a getScom operation
  ECMD_PUTSCOM_OP,              ///< Do a putScom operation
  ECMD_PUTSCOMUNDERMASK_OP,     ///< Do a putScomUnderMask operation
  ECMD_BULK_GETSCOM_OP,         ///< Do a bulk getScom operation
  ECMD_BULK_PUTSCOM_OP,         ///< Do a bulk putScom operation
} ecmdScomMode_t;

/**
 @brief Used by ecmdWriteTarget to specify displayMode
*/
typedef enum {
  ECMD_DISPLAY_TARGET_UNKNOWN,          ///< The unknown state for this enum, not a valid output type
  ECMD_DISPLAY_TARGET_PLUGIN_MODE,      ///< Uses whatever mode the plugin has set
  ECMD_DISPLAY_TARGET_DEFAULT,          ///< Default mode : 'pu k0:n0:s0:p01:c1' or 'k0:n0'
  ECMD_DISPLAY_TARGET_COMPRESSED,       ///< Compressed : 'k0:n0:s0:pu:p01:c1' or 'k0:n0'
  ECMD_DISPLAY_TARGET_HEX_DEFAULT,      ///< Default mode : '0x[pu k0:n0:s0:p01:c1]' or '0x[k0:n0]'
  ECMD_DISPLAY_TARGET_HEX_COMPRESSED,   ///< Compressed : '0x[k0:n0:s0:pu:p01:c1]' or '0x[k0:n0]'
  ECMD_DISPLAY_TARGET_HYBRID,           ///< Hybrid mode : '0x[pu:k0:n0:s0:p01:c1]' or '0x[pu.core:k0:n0:s0:p01:c1]'
  ECMD_DISPLAY_TARGET_HEX_HYBRID,       ///< Hybrid mode : '0x[pu:k0:n0:s0:p01:c1]' or '0x[pu.core:k0:n0:s0:p01:c1]'
  ECMD_DISPLAY_TARGET_COMMANDLINE,      ///< Command Line mode : pu -k0 -n1 -s0 -p2
  ECMD_DISPLAY_TARGET_STATES_DECIMAL,   ///< States Mode Decimal: pu(V).cu(V) k0(V):n0(V):s0(V):p01(WC):c0(U):t0(U) (uid 0x0(UNK))
  ECMD_DISPLAY_TARGET_STATES_HEX        ///< States Mode Hex: pu(V).cu(V) 0x[k0(V):n0(V):s0(V):p01(WC):c0(U):t0(U)] (uid 0x0(UNK))
} ecmdTargetDisplayMode_t;


/**
 @brief Used by Sensor interfaces to specify what mode to use when reading the sensor values
*/
typedef enum {
  ECMD_SENSOR_READ_MODE_UNKNOWN,       ///< The unknown state for this enum, not a valid output type
  ECMD_SENSOR_READ_MODE_DEFAULT,       ///< To be used as the default for any Sensor Interface
  ECMD_SENSOR_READ_MODE_MIN,       ///< To be used as the default for any Sensor Interface
  ECMD_SENSOR_READ_MODE_MAX,       ///< To be used as the default for any Sensor Interface
  ECMD_SENSOR_READ_MODE_1ms,           ///< 1 miliSecond ('instanteous')
  ECMD_SENSOR_READ_MODE_8ms,           ///< 8 miliSeconds
  ECMD_SENSOR_READ_MODE_32ms,          ///< 32 miliSeconds
  ECMD_SENSOR_READ_MODE_1s,            ///< 1 second
  ECMD_SENSOR_READ_MODE_8s             ///< 8 seconds
} ecmdSensorReadMode_t;

/**
 @brief Used by ecmdGetThermalSensor() interface to specify thermal return value
*/
typedef enum {
  ECMD_THERMAL_UNIT_UNKNOWN,      ///< The unknown state for this enum, not a valid output type
  ECMD_THERMAL_UNIT_dC,           ///< Celcius deciDegrees
  ECMD_THERMAL_UNIT_C             ///< Celcius Degrees
} ecmdThermalUnit_t;

/**
 @brief Used by ecmdGetVoltageSensor() interface to specify voltage return value
*/
typedef enum {
  ECMD_VOLTAGE_UNIT_UNKNOWN,      ///< The unknown state for this enum, not a valid output type
  ECMD_VOLTAGE_UNIT_mV,           ///< miliVolt
  ECMD_VOLTAGE_UNIT_V             ///< Volt
} ecmdVoltageUnit_t;

/**
 @brief Used by ecmdGetCurrentSensor() interface to specify current return value
*/
typedef enum {
  ECMD_CURRENT_UNIT_UNKNOWN,      ///< The unknown state for this enum, not a valid output type
  ECMD_CURRENT_UNIT_mA,           ///< milliampere
  ECMD_CURRENT_UNIT_A             ///< ampere
} ecmdCurrentUnit_t;

/**
 @brief Used by ecmdGetAirDensitySensor() interface to specify air density return value
*/
typedef enum {
  ECMD_AIRDENSITY_UNIT_UNKNOWN,      ///< The unknown state for this enum, not a valid output type
  ECMD_AIRDENSITY_UNIT_KG_M3,         ///< kilogram per qubic meter
  ECMD_AIRDENSITY_UNIT_H_PA,          ///< hekto Pascal
  ECMD_AIRDENSITY_UNIT_dH_PA         ///< deci hekto Pascal
} ecmdAirDensityUnit_t;

/**
 @brief Used by ecmdGetHumiditySensor() interface to specify humidity return value
*/
typedef enum {
  ECMD_HUMIDITY_UNIT_UNKNOWN,         ///< The unknown state for this enum, not a valid output type
  ECMD_HUMIDITY_UNIT_G_M3,            ///< Absolute humidity in grams per cubic meter
  ECMD_HUMIDITY_UNIT_REL_PERCENTAGE,  ///< Relative humidity in percentage,
  ECMD_HUMIDITY_UNIT_dREL_PERCENTAGE  ///< Relative humidity in deci percentage,
} ecmdHumidityUnit_t;

/**
 @brief Used by ecmdPowerSensor() interface to specify power return value
*/
typedef enum {
  ECMD_POWER_UNIT_UNKNOWN,      ///< The unknown state for this enum, not a valid output type
  ECMD_POWER_UNIT_uW,           ///< microWatt
  ECMD_POWER_UNIT_mW,           ///< milliWatt
  ECMD_POWER_UNIT_dW,           ///< deciWatt
  ECMD_POWER_UNIT_W             ///< watt
} ecmdPowerUnit_t;

/**
 @brief Used by ecmdSetPowerMode() interface to specify power mode
*/
typedef enum {
  ECMD_POWER_MODE_UNKNOWN,      ///< The unknown state for this enum, not a valid output type
  ECMD_POWER_MODE_NORMAL,       ///< normal power mode
  ECMD_POWER_MODE_SAVE,         ///< power save mode
  ECMD_POWER_MODE_TURBO,         ///< turbo mode
  ECMD_POWER_MODE_PARTIAL_SAVE,  ///< part of the target is in power save mode
  ECMD_POWER_MODE_IN_TRANSITION, ///< target is in transition mode
  ECMD_POWER_MODE_SAVE_DYNAMIC,  ///< target is in dynamic power save mode
  ECMD_POWER_MODE_SAVE_DYNAMIC_MP,  ///< target is in dynamic power save max performance mode
  ECMD_POWER_MODE_SUPER_TURBO   ///< super turbo mode
} ecmdPowerMode_t;

/**
 @brief Used by ecmdGetUtilizationSensor() interface to specify utilization return value
*/
typedef enum {
  ECMD_UTILIZATION_UNIT_UNKNOWN,      ///< The unknown state for this enum, not a valid output type
  ECMD_UTILIZATION_UNIT_dP,           ///< deciPercentage
  ECMD_UTILIZATION_UNIT_P             ///< Percentage
} ecmdUtilizationUnit_t;

/**
 @brief Used by ecmdQuerySystemPower() and ecmdQueryFruPower() interface to return the current Power State
*/
typedef enum {
  ECMD_POWER_STATE_UNKNOWN,      ///< The unknown state for this enum, not a valid output type
  ECMD_POWER_STATE_ON,           ///< The selected Target is powered on
  ECMD_POWER_STATE_OFF,          ///< The selected Target is powered off
  ECMD_POWER_STATE_TRANSITION    ///< The selected Target is in a power transition state
  } ecmdPowerState_t;

typedef enum {
  ECMD_BW_UNIT_UNKNOWN,      ///< The unknown state for this enum, not a valid output type
  ECMD_BW_UNIT_MRW           ///< The selected Target is powered on
} ecmdBandwidthUnit_t;


//--------------------------------------------------------------------
//  Structures                                    
//--------------------------------------------------------------------

/**
 @brief This is used by ecmdQueryDllInfo to return info to the client about what Dll instance they are actually running with
*/
struct ecmdDllInfo {
  std::string           dllType;        ///< Dll instance type running
  std::string           dllProduct;     ///< Dll product supported
  std::string           dllProductType; ///< Dll product type currently configured
  ecmdDllEnv_t          dllEnv;         ///< Dll environment (Simulation vs Hardware)
  std::string           dllBuildDate;   ///< Date the Dll was built
  std::string           dllCapiVersion; ///< should be set to ECMD_CAPI_VERSION
  std::string           dllBuildInfo;   ///< Any additional info the Dll/Plugin would like to pass
};

/**
 @brief Structure used to designate which cec object/chip you would like the function to operate on

  - The state bits are not required on all hardware access functions.  Every hardware access function should have a "TARGET STATES" block telling if the state fields have to be set or not
  - The state bits are used slightly differently for the queryFunctions they are used there to signify
     what data coming in is valid to refine a query

 Operators Supported : < ==

*/
struct ecmdChipTarget {

#ifndef DOCUMENTATION
  // Constructor
  ecmdChipTarget();

  // Destructor
  ~ecmdChipTarget();

  // Methods
  uint32_t flatten(uint8_t *o_buf, uint32_t i_len) const;
  uint32_t unflatten(const uint8_t *i_buf, uint32_t i_len);
  uint32_t flattenSize(void) const;
  int operator==(const ecmdChipTarget& rhs) const; ///< Used to compare ecmdChipTarget variables based on state fields
  bool operator<(const ecmdChipTarget& rhs) const; ///< Used to sort ecmdChipTarget entries in an ecmdChipTarget list.
#ifndef ECMD_STRIP_DEBUG
  void     printStruct(void) const;
#endif
#endif
    
  // Members
  uint32_t    cage;             ///< cage that contains node with chip
  uint32_t    node;             ///< node that contains chip
  uint32_t    slot;             ///< card slot/fru to target
  std::string chipType;         ///< name of chip to access , either actual or common name
  uint32_t    pos;              ///< position of chip within node
  std::string chipUnitType;     ///< name of the chip unit to access
  uint8_t     chipUnitNum;      ///< number of the chip unit to access
  uint8_t     thread;           ///< which thread on chip to access, if chip is multi-threaded
  uint32_t    unitId;           ///< This is an optional field if unitid's are used to specify the target, the above info still needs to be filled in
  ecmdChipTargetState_t cageState;            ///< cage field state
  ecmdChipTargetState_t nodeState;            ///< node field state
  ecmdChipTargetState_t slotState;            ///< slot field state
  ecmdChipTargetState_t chipTypeState;        ///< chipType field state
  ecmdChipTargetState_t posState;             ///< pos field state
  ecmdChipTargetState_t chipUnitTypeState;    ///< chipUnitType field state
  ecmdChipTargetState_t chipUnitNumState;   ///< chipUnitNum field state    
  ecmdChipTargetState_t threadState;          ///< thread field state
  ecmdChipTargetState_t unitIdState;          ///< unitId field state
};

#ifndef DOCUMENTATION
// ecmdChipTarget Constructor
inline ecmdChipTarget::ecmdChipTarget():
cage(0),
node(0),
slot(0),
pos(0),
chipUnitNum(0),
thread(0),
unitId(0),
cageState(ECMD_TARGET_UNKNOWN_STATE),
nodeState(ECMD_TARGET_UNKNOWN_STATE),
slotState(ECMD_TARGET_UNKNOWN_STATE),
chipTypeState(ECMD_TARGET_UNKNOWN_STATE),
posState(ECMD_TARGET_UNKNOWN_STATE),
// The chipUnitTypeState field is init'd differently.  By doing this, we fix this problem: p6.  k0:n0:s0:p00
// By setting to UNUSED, the P6 code doesn't have to be touched.  And for P7/Z7 above, people don't have to
// set the chipUnitTypeState field if not used by their chip.  Setting the chipUnitNumState = UNUSED is still
// required for chip level queries - JTA 10/21/08
chipUnitTypeState(ECMD_TARGET_FIELD_UNUSED),
chipUnitNumState(ECMD_TARGET_UNKNOWN_STATE),
threadState(ECMD_TARGET_UNKNOWN_STATE),
unitIdState(ECMD_TARGET_UNKNOWN_STATE)
{
}

// ecmdChipTarget Destructor
inline ecmdChipTarget::~ecmdChipTarget() { }
#endif

/**
 @brief Used for the ecmdQueryConfig function to return thread data

 Operators Supported : <

*/
struct ecmdThreadData {

#ifndef DOCUMENTATION
  // Constructor
  ecmdThreadData();

  // Destructor
  ~ecmdThreadData();

  // Methods
  uint32_t flatten(uint8_t *o_buf, uint32_t &i_len) const;
  uint32_t unflatten(const uint8_t *i_buf, uint32_t &i_len);
  uint32_t flattenSize(void) const;
  bool operator<(const ecmdThreadData& rhs) const; ///< Used to sort Thread entries in an ecmdThreadData list.
#ifndef ECMD_STRIP_DEBUG
  void printStruct(void) const;
#endif
#endif

  // Members
  uint8_t   threadId;       ///< (Detail: Low) Thread number of this entry
  uint32_t  unitId;     ///< (Detail: High) Unit Id of this entry
  uint32_t  threadFlags;        ///< (Detail: High) Various additional info about the thread - bitmask of defines
};

#ifndef DOCUMENTATION
// ecmdThreadData Constructor
inline ecmdThreadData::ecmdThreadData():
threadId(0),
unitId(0),
threadFlags(0x0)
{
}

// ecmdThreadData Destructor
inline ecmdThreadData::~ecmdThreadData() { }
#endif

/**
 @brief Used for the ecmdQueryConfig function to return chipUnit data

 Operators Supported : <

*/
struct ecmdChipUnitData {

#ifndef DOCUMENTATION
  // Constructor
  ecmdChipUnitData();

  // Destructor
  ~ecmdChipUnitData();

  // Methods
  uint32_t flatten(uint8_t *o_buf, uint32_t &i_len) const;
  uint32_t unflatten(const uint8_t *i_buf, uint32_t &i_len);
  uint32_t flattenSize(void) const;
  bool operator<(const ecmdChipUnitData& rhs) const;  ///< Used to sort Chip Unit entries in an ecmdChipUnitData list.
#ifndef ECMD_STRIP_DEBUG

  void printStruct(void) const;

#endif
#endif

  // Members
  std::string                   chipUnitType;        ///< (Detail: Low)  Full name of chipUnit.. i.e. core, mc, etc..
  std::string                   chipUnitShortType;   ///< (Detail: Low)  Full name of chipUnit.. i.e. core, mc, etc..
  uint8_t                       chipUnitNum;         ///< (Detail: Low) chipUnit number of this entry
  uint8_t                       numThreads;          ///< (Detail: Low) Number of threads this entry supports
  uint32_t                      unitId;              ///< (Detail: High) Unit Id of this entry
  uint32_t                      chipUnitFlags;       ///< (Detail: High) Various additional info about the chipUnit - bitmask of defines
  std::list<ecmdThreadData>     threadData;          ///< (Detail: Low) List of all threads avaliable for this chipUnit - only valid for Processor compute cores - in numerical order
};

#ifndef DOCUMENTATION
// ecmdChipUnitData Constructor
inline ecmdChipUnitData::ecmdChipUnitData():
chipUnitNum(0),
numThreads(0),
unitId(0),
chipUnitFlags(0x0)
{
}

// ecmdChipUnitData Destructor
inline ecmdChipUnitData::~ecmdChipUnitData() { }
#endif

/**
 Defines for the ecmdChipData chipFlags field
*/
/* The Bus type flags use the first two bits of the chipflags field */
#if !defined(ECMD_PERLAPI) && !defined(ECMD_PYAPI)
#define ECMD_CHIPFLAG_BUSMASK   0xC0000000
#endif
#define ECMD_CHIPFLAG_RSVDBUS1  0x00000000      ///< This is reserved for later expansion (should not be used)
#define ECMD_CHIPFLAG_JTAG      0x40000000
#if !defined(ECMD_PERLAPI) && !defined(ECMD_PYAPI)
#define ECMD_CHIPFLAG_FSI       0x80000000
#define ECMD_CHIPFLAG_RSVDBUS2  0xC0000000      ///< This is reserved for later expansion (should not be used)
#endif

#define ECMD_CHIPFLAG_32BIT_HEADERCHECK  0x08000000 
#define ECMD_CHIPFLAG_64BIT_HEADERCHECK  0x04000000

#define ECMD_CHIPFLAG_DEF_USE_SPARSE_SCAN 0x00800000

#define ECMD_FLAG_STATEMASK         0x0000001F
#define ECMD_FLAG_CONFIGED_USEABLE  0x00000001
#define ECMD_FLAG_DELAYED_FENCED    0x00000002
#define ECMD_FLAG_FENCED            0x00000004
#define ECMD_FLAG_SPARE             0x00000008
#define ECMD_FLAG_DEFECTIVE         0x00000010

/**
 @brief Used for the ecmdQueryConfig function to return chip data

 Operators Supported : <

*/
struct ecmdChipData {

#ifndef DOCUMENTATION
  // Constructor
  ecmdChipData();

  // Destructor
  ~ecmdChipData();

  // Methods
  uint32_t flatten(uint8_t *o_buf, uint32_t &i_len) const;
  uint32_t unflatten(const uint8_t *i_buf, uint32_t &i_len);
  uint32_t flattenSize(void) const;
  bool operator<(const ecmdChipData& rhs) const; ///< Used to sort Chip entries (based on Pos) in an ecmdChipData list.
#ifndef ECMD_STRIP_DEBUG
  void printStruct(void) const;
#endif
#endif

  // Members
  std::string                   chipType;           ///< (Detail: Low)  Full name of chip , ie. p6, enterprise, corona
  std::string                   chipShortType;      ///< (Detail: Low)  Short name of chip, ie. p6, ent, cor  (should be 3chars or less)
  std::string                   chipCommonType;     ///< (Detail: Low)  common name of chip, ie. pu, iohub, l3cache
  uint32_t                      pos;                ///< (Detail: Low)  Position of this entry
  uint32_t                      unitId;             ///< (Detail: High) Unit Id of this entry
  uint32_t                      chipEc;             ///< (Detail: High) EC level of this chip, (ec read from 'jtag' chip id or CFAM id)
  uint32_t                      simModelEc;         ///< (Detail: High) Model EC level of this chip
  ecmdChipInterfaceType_t       interfaceType;      ///< (Detail: High) Interface Macro used by the chip
  uint32_t                      chipFlags;          ///< (Detail: High) Various additional info about the chip - bitmask of defines
  std::list<ecmdChipUnitData>   chipUnitData;       ///< (Detail: Low)  List of all chipUnits requested in this chip - sorted by chipUnitType and chipUnitNum
};

#ifndef DOCUMENTATION
// ecmdChipData Constructor
inline ecmdChipData::ecmdChipData():
pos(0),
unitId(0),
chipEc(0),
simModelEc(0),
interfaceType(ECMD_INTERFACE_UNKNOWN),
chipFlags(0x0)
{
}

// ecmdChipData Destructor
inline ecmdChipData::~ecmdChipData() { }
#endif

/**
 @brief Used for the ecmdQueryConfig function to return slot data

 Operators Supported : <

*/
struct ecmdSlotData {

#ifndef DOCUMENTATION
  // Constructor
  ecmdSlotData();

  // Destructor
  ~ecmdSlotData();

  // Methods
  uint32_t flatten(uint8_t *o_buf, uint32_t &i_len) const;
  uint32_t unflatten(const uint8_t *i_buf, uint32_t &i_len);
  uint32_t flattenSize(void) const;
  bool operator<(const ecmdSlotData& rhs) const; ///< Used to sort Slot entries in an ecmdSlotData list. 
#ifndef ECMD_STRIP_DEBUG
  void printStruct(void) const;
#endif
#endif

  // Members
  uint32_t slotId;                      ///< (Detail: Low) Slot number of this entry
  std::string slotName;                 ///< (Detail: High) The name assigned to this slot
  uint32_t unitId;                      ///< (Detail: High) Unit Id of this entry
  uint32_t slotFlags;                   ///< (Detail: High) Various additional info about the slot - bitmask of defines
  std::list<ecmdChipData> chipData;     ///< (Detail: Low) List of all chips requested in this slot - in order by chipType and pos
};

#ifndef DOCUMENTATION
// ecmdSlotData Constructor
inline ecmdSlotData::ecmdSlotData():
slotId(0),
unitId(0),
slotFlags(0x0)
{
}

// ecmdSlotData Destructor
inline ecmdSlotData::~ecmdSlotData() { }
#endif

/**
 @brief Used for the ecmdQueryConfig function to return node data

  Operators Supported : <

*/
struct ecmdNodeData {

#ifndef DOCUMENTATION
  // Constructor
  ecmdNodeData();

  // Destructor
  ~ecmdNodeData();

  // Methods
  uint32_t flatten(uint8_t *o_buf, uint32_t &i_len) const;
  uint32_t unflatten(const uint8_t *i_buf, uint32_t &i_len);
  uint32_t flattenSize(void) const;
  bool operator<(const ecmdNodeData& rhs) const; ///< Used to sort Node entries in an ecmdNodeData list. 
#ifndef ECMD_STRIP_DEBUG
  void printStruct(void) const;
#endif
#endif

  // Members
  uint32_t nodeId;                      ///< (Detail: Low) Node number of this entry
  std::string nodeName;                 ///< (Detail: High) The name assigned to this node
  uint32_t unitId;                      ///< (Detail: High) Unit Id of this entry
  uint32_t nodeFlags;                   ///< (Detail: High) Various additional info about the node - bitmask of defines
  std::list<ecmdSlotData> slotData;     ///< (Detail: Low) List of all slots requested in this node - in numerical order by slotId
};

#ifndef DOCUMENTATION
// ecmdNodeData Constructor
inline ecmdNodeData::ecmdNodeData():
nodeId(0),
unitId(0),
nodeFlags(0x0)
{
}

// ecmdNodeData Destructor
inline ecmdNodeData::~ecmdNodeData() { }
#endif

/**
 @brief Used for the ecmdQueryConfig function to return cage data

 Operators Supported : <

*/
struct ecmdCageData {

#ifndef DOCUMENTATION
  // Constructor
  ecmdCageData();

  // Destructor
  ~ecmdCageData();

  // Methods
  uint32_t flatten(uint8_t *o_buf, uint32_t &i_len) const;
  uint32_t unflatten(const uint8_t *i_buf, uint32_t &i_len);
  uint32_t flattenSize(void) const;
  bool operator<(const ecmdCageData& rhs) const;  ///< Used to sort Cage entries in an ecmdCageData list.
#ifndef ECMD_STRIP_DEBUG
  void printStruct(void) const;
#endif
#endif

  // Members
  uint32_t cageId;                      ///< (Detail: Low) Cage number of this entry
  std::string cageName;                 ///< (Detail: High) The name assigned to this cage
  uint32_t unitId;                      ///< (Detail: High) Unit Id of this entry
  uint32_t cageFlags;                   ///< (Detail: High) Various additional info about the cage - bitmask of defines
  std::list<ecmdNodeData> nodeData;     ///< (Detail: Low) List of all nodes requested in this cage - in numerical order by nodeId
};

#ifndef DOCUMENTATION
// ecmdCageData Constructor
inline ecmdCageData::ecmdCageData():
cageId(0),
unitId(0),
cageFlags(0x0)
{
}

// ecmdCageData Destructor
inline ecmdCageData::~ecmdCageData() { }
#endif

/**
 @brief Used by the ecmdQueryConfig function to return data
*/
struct ecmdQueryData {

#ifndef DOCUMENTATION
  // Constructor
  ecmdQueryData();

  // Destructor
  ~ecmdQueryData();

  // Methods
  uint32_t flatten(uint8_t *o_buf, uint32_t &i_len) const;
  uint32_t unflatten(const uint8_t *i_buf, uint32_t &i_len);
  uint32_t flattenSize(void) const;
#ifndef ECMD_STRIP_DEBUG
  void printStruct(void) const;
#endif
#endif

  // Members
  ecmdQueryDetail_t       detailLevel;  ///< (Detail: Low) This is set to the detail level of the data contained within
  std::list<ecmdCageData> cageData;     ///< (Detail: Low) List of all cages in the system - in nummerical order by cageId
};

#ifndef DOCUMENTATION
// ecmdQueryData Constructor
inline ecmdQueryData::ecmdQueryData():
detailLevel((ecmdQueryDetail_t)0x0)
{
}

// ecmdQueryData Destructor
inline ecmdQueryData::~ecmdQueryData() { }
#endif

/**
 @brief Used internally by ecmdConfigLooper to store looping state information
*/
struct ecmdLooperData {
  bool             ecmdLooperInitFlag;                          ///< Is fresh ?
  bool             ecmdUseUnitid;                               ///< This looper is looping on unitid targets not config data
  bool             initialized;                                 ///< Has the struture been intialized by LooperInit

  ecmdQueryData    ecmdSystemConfigData;                        ///< Config data queried from the system
  std::list<ecmdCageData>::iterator ecmdCurCage;                ///< Pointer to current Cage
  std::list<ecmdNodeData>::iterator ecmdCurNode;                ///< Pointer to current Node
  std::list<ecmdSlotData>::iterator ecmdCurSlot;                ///< Pointer to current Slot
  std::list<ecmdChipData>::iterator ecmdCurChip;                ///< Pointer to current Chip
  std::list<ecmdChipUnitData>::iterator ecmdCurChipUnit;        ///< Pointer to current ChipUnit
  std::list<ecmdThreadData>::iterator ecmdCurThread;            ///< Pointer to current Thread
  ecmdChipTarget prevTarget;                                    ///< Pointer to previous target


  std::list<ecmdChipTarget>     unitIdTargets;                  ///< List of targets if looping on a unitid
  std::list<ecmdChipTarget>::iterator curUnitIdTarget;          ///< Pointer to current unitid target
};

#if !defined(ECMD_PERLAPI) && !defined(ECMD_PYAPI)
#define ECMD_CONFIG_VALID_FIELD_STRING  0x80000000
#endif
#define ECMD_CONFIG_VALID_FIELD_UINT32  0x40000000
#define ECMD_CONFIG_VALID_FIELD_UINT64  0x20000000
#define ECMD_CONFIG_VALID_FIELD_FLOAT   0x10000000
/**
 @brief Used by the get/set configuration functions to return the data
*/
struct ecmdConfigData {

#ifndef DOCUMENTATION
  // Constructor
  ecmdConfigData();

  // Destructor
  ~ecmdConfigData();
#endif

  // Members
  uint32_t              validMask;  ///< Indicates which of the fields in this structure are valid
  std::string           cdString;   ///< If the output data is a string, will be populated
  uint32_t              cdUint32;   ///< If the output data is a uint32_t, will be populated
  uint64_t              cdUint64;   ///< If the output data is a uint64_t, will be populated
  float                 cdFloat;    ///< If the output data is a float, will be populated
};

#ifndef DOCUMENTATION
// ecmdConfigData Constructor
inline ecmdConfigData::ecmdConfigData():
validMask(0),
cdUint32(0),
cdUint64(0),
cdFloat(0)
{
}

inline ecmdConfigData::~ecmdConfigData() { }
#endif

/**
 @brief A pair of file names returned by ecmdQueryFileLocation()
*/
struct ecmdFileLocation {
    std::string textFile; ///< The human-readable version of the file
    std::string hashFile; ///< The corresponding hash file
};

/**
 @brief Used for the ecmdQueryRing function to return ring info
*/
struct ecmdRingData {
#ifndef DOCUMENTATION
  // Constructor
  ecmdRingData();

  // Destructor
  ~ecmdRingData();

  // Methods
  uint32_t flatten(uint8_t *o_buf, uint32_t i_len) const;
  uint32_t unflatten(const uint8_t *i_buf, uint32_t i_len);
  uint32_t flattenSize(void) const;
#ifndef ECMD_STRIP_DEBUG
  void printStruct(void) const;
#endif
#endif

  bool isChipUnitMatch(std::string &i_chipUnitType); ///< Checks if relatedChipUnit variable matches input

  // Members
  std::list<std::string> ringNames;              ///< (Detail: Low)  Names used to reference this ring
  std::list<uint64_t>    ringIds;                ///< (Detail: Low)  Hashed ring IDs
  uint32_t               address;                ///< (Detail: Low)  Address modifier
  uint32_t               bitLength;              ///< (Detail: Low)  length of ring
  bool                   hasInversionMask;       ///< (Detail: High) Ring has an inversion mask applied before scanning
  bool                   supportsBroadsideLoad;  ///< (Detail: High) This ring supports broadside load in simulation
  bool                   isCheckable;            ///< (Detail: High) This ring can be run through the check_rings command
  bool                   isChipUnitRelated;      ///< (Detail: Low)  This ring is related to the chipUnit level of a chip
  std::string            relatedChipUnit;        ///< (Detail: Low)  If isChipUnitRelated, this is the name of the chipUnit
  std::string            relatedChipUnitShort;   ///< (Detail: Low)  If isChipUnitRelated, this is the short name of the chipUnit
  std::string            clockDomain;            ///< (Detail: High) Clock domain this ring belongs to
  ecmdClockState_t       clockState;             ///< (Detail: High) Required clock state to access this ring
};

#ifndef DOCUMENTATION
// ecmdRingData Constructor
inline ecmdRingData::ecmdRingData():
address(0),
bitLength(0),
hasInversionMask(false),
supportsBroadsideLoad(false),
isCheckable(false),
isChipUnitRelated(false),
clockState((ecmdClockState_t) ECMD_CLOCKSTATE_UNKNOWN)
{
}

// ecmdRingData Destructor
inline ecmdRingData::~ecmdRingData() { }
#endif


/**
 @brief Used for the ecmdQueryArray function to return array info
*/
struct ecmdArrayData {
#ifndef DOCUMENTATION
  // Constructor
  ecmdArrayData();

  // Destructor
  ~ecmdArrayData();

  // Methods
  uint32_t flatten(uint8_t *o_buf, uint32_t i_len) const;
  uint32_t unflatten(const uint8_t *i_buf, uint32_t i_len);
  uint32_t flattenSize(void) const;
#ifndef ECMD_STRIP_DEBUG
  void printStruct(void) const;
#endif
#endif

  bool isChipUnitMatch(std::string &i_chipUnitType); ///< Checks if relatedChipUnit variable matches input

  // Members
  std::string      arrayName;              ///< (Detail: Low)  Names used to reference this array
  uint32_t         arrayId;                ///< (Detail: Low)  Hashed array ID
  ecmdArrayType_t  arrayType;              ///< (Detail: Low)  The type of array this is
  uint32_t         readAddressLength;      ///< (Detail: Low)  Bit length of read address
  uint32_t         writeAddressLength;     ///< (Detail: Low)  Bit length of write address
  uint64_t         length;                 ///< (Detail: Low)  Length of array (number of entries)
  uint32_t         width;                  ///< (Detail: Low)  Bit width of array entry
  bool             isChipUnitRelated;      ///< (Detail: Low)  This array is related to the chipUnit level of a chip
  std::string      relatedChipUnit;        ///< (Detail: Low)  If isChipUnitRelated, this is the name of the chipUnit
  std::string      relatedChipUnitShort;   ///< (Detail: Low)  If isChipUnitRelated, this is the short name of the chipUnit
  std::string      clockDomain;            ///< (Detail: High) Clock domain this array belongs to
  ecmdClockState_t clockState;             ///< (Detail: High) Required clock state to access this array
};

#ifndef DOCUMENTATION
// ecmdArrayData Constructor
inline ecmdArrayData::ecmdArrayData():
arrayId(0),
arrayType(ECMD_ARRAYTYPE_UNKNOWN),
readAddressLength(0),
writeAddressLength(0),
length(0),
width(0),
isChipUnitRelated(false),
clockState((ecmdClockState_t) ECMD_CLOCKSTATE_UNKNOWN)
{
}

// ecmdArrayData Destructor
inline ecmdArrayData::~ecmdArrayData() { }
#endif


/**
 @brief Used for the ecmdQueryTraceArray function to return trace array info
*/
struct ecmdTraceArrayData {
#ifndef DOCUMENTATION
  // Constructor
  ecmdTraceArrayData();

  // Destructor
  ~ecmdTraceArrayData();

  // Methods
  uint32_t flatten(uint8_t *o_buf, uint32_t i_len) const;
  uint32_t unflatten(const uint8_t *i_buf, uint32_t i_len);
  uint32_t flattenSize(void) const;
#ifndef ECMD_STRIP_DEBUG
  void printStruct(void) const;
#endif
#endif

  bool isChipUnitMatch(std::string &i_chipUnitType); ///< Checks if relatedChipUnit variable matches input

  // Members
  std::string      traceArrayName;       ///< (Detail: Low)  Name of Trace array
  uint32_t         traceArrayId;         ///< (Detail: Low)  Hashed trace array ID
  uint32_t         length;               ///< (Detail: Low)  Length of trace array (number of entries)
  uint32_t         width;                ///< (Detail: Low)  Bit width of trace array entry
  bool             isChipUnitRelated;    ///< (Detail: Low)  This trace array is related to the chipUnit level of a chip
  std::string      relatedChipUnit;      ///< (Detail: Low)  If isChipUnitRelated, this is the name of the chipUnit
  std::string      relatedChipUnitShort; ///< (Detail: Low)  If isChipUnitRelated, this is the short name of the chipUnit
  std::string      clockDomain;          ///< (Detail: High) Clock domain this array belongs to
  ecmdClockState_t clockState;           ///< (Detail: High) Required clock state to access this array
};

#ifndef DOCUMENTATION
// ecmdTraceArrayData Constructor
inline ecmdTraceArrayData::ecmdTraceArrayData():
traceArrayId(0),
length(0),
width(0),
isChipUnitRelated(false),
clockState((ecmdClockState_t) ECMD_CLOCKSTATE_UNKNOWN)
{
}

// ecmdTraceArrayData Destructor
inline ecmdTraceArrayData::~ecmdTraceArrayData() { }
#endif

/**
 @brief Used for the ecmdQueryFastArray function to return trace array info
*/
struct ecmdFastArrayData {
#ifndef DOCUMENTATION
  // Constructor
  ecmdFastArrayData();

  // Destructor
  ~ecmdFastArrayData();

  // Methods
  uint32_t flatten(uint8_t *o_buf, uint32_t i_len) const;
  uint32_t unflatten(const uint8_t *i_buf, uint32_t i_len);
  uint32_t flattenSize(void) const;
#ifndef ECMD_STRIP_DEBUG
  void printStruct(void) const;
#endif
#endif

  bool isChipUnitMatch(std::string &i_chipUnitType); ///< Checks if relatedChipUnit variable matches input

  // Members
  std::string      fastArrayName;        ///< (Detail: Low)  Name of fast array
  uint32_t         fastArrayId;          ///< (Detail: Low)  Hashed fast array ID
  uint32_t         length;               ///< (Detail: Low)  Length of fast array (number of entries)
  uint32_t         width;                ///< (Detail: Low)  Bit width of fast array entry
  uint32_t         readAddressLength;    ///< (Detail: Low)  Bit length of read address
  bool             isChipUnitRelated;    ///< (Detail: Low)  This trace array is related to the chipUnit level of a chip
  std::string      relatedChipUnit;      ///< (Detail: Low)  If isChipUnitRelated, this is the name of the chipUnit
  std::string      relatedChipUnitShort; ///< (Detail: Low)  If isChipUnitRelated, this is the short name of the chipUnit
  std::string      clockDomain;          ///< (Detail: High) Clock domain this array belongs to
  ecmdClockState_t clockState;           ///< (Detail: High) Required clock state to access this array
};


#ifndef DOCUMENTATION
// ecmdFastArrayData Constructor
inline ecmdFastArrayData::ecmdFastArrayData():
fastArrayId(0),
length(0),
width(0),
readAddressLength(0),
isChipUnitRelated(false),
clockState((ecmdClockState_t) ECMD_CLOCKSTATE_UNKNOWN)
{
}

// ecmdFastArrayData Destructor
inline ecmdFastArrayData::~ecmdFastArrayData() { }
#endif

/**
 @brief Used for the ecmdQueryScom function to return scom info
*/
struct ecmdScomData {
#ifndef DOCUMENTATION
  // Constructor
  ecmdScomData();

  // Destructor
  ~ecmdScomData();

  // Methods
  //uint32_t flatten(uint8_t *o_buf, uint32_t i_len) const;
  //uint32_t unflatten(const uint8_t *i_buf, uint32_t i_len);
  //uint32_t flattenSize(void) const;
#ifndef ECMD_STRIP_DEBUG
  //void printStruct(void) const;
#endif
#endif

  bool isChipUnitMatch(std::string &i_chipUnitType); ///< Checks if relatedChipUnit variable matches input

  // Members
  uint64_t         address;              ///< (Detail: Low)  Scom Address
  uint32_t         length;               ///< (Detail: Low)  The length of the scom in bits
  bool             isChipUnitRelated;    ///< (Detail: Low)  This scom is related to the chipUnit level of a chip
  std::list<std::string>        relatedChipUnit;      ///< (Detail: Low)  If isChipUnitRelated, this is a list of names of the chipUnit
  std::list<std::string>        relatedChipUnitShort; ///< (Detail: Low)  If isChipUnitRelated, this is short list of names of the chipUnit
  ecmdEndianMode_t endianMode;           ///< (Detail: Low)  Indicate the endianness of the data
  std::string      clockDomain;          ///< (Detail: High) Clock domain this scom belongs to
  ecmdClockState_t clockState;           ///< (Detail: High) Required clock state to access this scom
};

#ifndef DOCUMENTATION
// ecmdScomData Constructor
inline ecmdScomData::ecmdScomData():
address(0),
length(0),
isChipUnitRelated(false),
endianMode(ECMD_UNKNOWN_ENDIAN),
clockState((ecmdClockState_t) ECMD_CLOCKSTATE_UNKNOWN)
{
}

// ecmdScomData Destructor
inline ecmdScomData::~ecmdScomData() { }
#endif

/**
 @brief Used for the ecmdQueryLatch function to return latch info
*/
struct ecmdLatchData {
#ifndef DOCUMENTATION
  // Constructor
  ecmdLatchData();

  // Destructor
  ~ecmdLatchData();

  // Methods
  uint32_t flatten(uint8_t *o_buf, uint32_t i_len) const;
  uint32_t unflatten(const uint8_t *i_buf, uint32_t i_len);
  uint32_t flattenSize(void) const;
#ifndef ECMD_STRIP_DEBUG
  void printStruct(void) const;
#endif
#endif

  bool isChipUnitMatch(std::string &i_chipUnitType); ///< Checks if relatedChipUnit variable matches input

  // Members
  std::string      latchName;            ///< (Detail: Low)  Latch Name
  std::string      ringName;             ///< (Detail: Low)  Ring that this latch belongs to
  uint32_t         latchId;              ///< (Detail: Low)  Hashed latch ID
  uint32_t         ringId;               ///< (Detail: Low)  Hashed ring ID
  uint32_t         bitLength;            ///< (Detail: Low)  length of latch, sum of all the parts
  bool             isChipUnitRelated;    ///< (Detail: Low)  This latch is related to the chipUnit level of a chip
  std::string      relatedChipUnit;      ///< (Detail: Low)  If isChipUnitRelated, this is the name of the chipUnit
  std::string      relatedChipUnitShort; ///< (Detail: Low)  If isChipUnitRelated, this is the short name of the chipUnit
  std::string      clockDomain;          ///< (Detail: High) Clock domain this latch belongs to
  ecmdClockState_t clockState;           ///< (Detail: High) Required clock state to access this latch
};

#ifndef DOCUMENTATION
// ecmdLatchData Constructor
inline ecmdLatchData::ecmdLatchData():
latchId(0),
ringId(0),
bitLength(0),
isChipUnitRelated(false),
clockState((ecmdClockState_t) ECMD_CLOCKSTATE_UNKNOWN)
{
}

// ecmdLatchData Destructor
inline ecmdLatchData::~ecmdLatchData() { }
#endif

/**
 @brief Used by the getArrayMultiple function to pass data
*/
struct ecmdScomEntry {
#ifndef DOCUMENTATION
  // Constructor
  ecmdScomEntry();

  // Destructor
  ~ecmdScomEntry();

  // Methods
  uint32_t flatten(uint8_t *o_buf, uint32_t i_len) const;
  uint32_t unflatten(const uint8_t *i_buf, uint32_t i_len);
  uint32_t flattenSize(void) const;
#ifndef ECMD_STRIP_DEBUG
  void printStruct(void) const;
#endif
#endif

  // Members
  uint64_t         address;     ///< The address to read or write
  ecmdDataBuffer   data;        ///< The data to read or write depending on the operation
  ecmdDataBuffer   dataMask;    ///< The mask data, only used on a mask operation
  ecmdScomMode_t   operation;   ///< The operation type to do.. getScom, putScom or putScomUnderMask
  uint32_t rc;                  ///< The return code for the operation
};

#ifndef DOCUMENTATION
// ecmdScomEntry Constructor
inline ecmdScomEntry::ecmdScomEntry():
address(0),
operation(ECMD_SCOM_UNKNOWN),
rc(0)
{
}

// ecmdScomEntry Destructor
inline ecmdScomEntry::~ecmdScomEntry() { }
#endif

/**
 @brief Used by the getArrayMultiple function to pass data
*/
struct ecmdArrayEntry {
#ifndef DOCUMENTATION
  // Constructor
  ecmdArrayEntry();

  // Destructor
  ~ecmdArrayEntry();

  // Methods
  uint32_t flatten(uint8_t *o_buf, uint32_t i_len) const;
  uint32_t unflatten(const uint8_t *i_buf, uint32_t i_len);
  uint32_t flattenSize(void) const;
#ifndef ECMD_STRIP_DEBUG
  void printStruct(void) const;
#endif
#endif

  // Members
  ecmdDataBuffer address;               ///< Array address/element to access
  ecmdDataBuffer buffer;                ///< Array data from address
  uint32_t       rc;                    ///< Error code in retrieving this entry
};

#ifndef DOCUMENTATION
// ecmdArrayEntry Constructor
inline ecmdArrayEntry::ecmdArrayEntry():
rc(0)
{
}

// ecmdArrayEntry Destructor
inline ecmdArrayEntry::~ecmdArrayEntry() { }
#endif


/**
 @brief Used by get/putspy function to create the return data from a group
*/
struct ecmdSpyGroupData {
  ecmdDataBuffer extractBuffer;         ///< The data read from the ring buffer
  ecmdDataBuffer deadbitsMask;          ///< A mask of the bits that were deadbits in that buffer
};


/**
 @brief Used by get/putSprMultiple function to pass data
*/
struct ecmdNameEntry {
#ifndef DOCUMENTATION
  // Constructor
  ecmdNameEntry();

  // Destructor
  ~ecmdNameEntry();

  // Methods
  uint32_t flatten(uint8_t *o_buf, uint32_t i_len) const;
  uint32_t unflatten(const uint8_t *i_buf, uint32_t i_len);
  uint32_t flattenSize(void) const;
#ifndef ECMD_STRIP_DEBUG
  void printStruct(void) const;
#endif
#endif

  // Members
  std::string    name;                  ///< Name of entry 
  ecmdDataBuffer buffer;                ///< Data to/from entry
  uint32_t       rc;                    ///< Error code in retrieving this entry
};

#ifndef DOCUMENTATION
// ecmdNameEntry Constructor
inline ecmdNameEntry::ecmdNameEntry():
rc(0)
{
}

// ecmdNameEntry Destructor
inline ecmdNameEntry::~ecmdNameEntry() { }
#endif

/**
 @brief Used by getTraceArrayMultiple and getTraceArray functions to pass data
*/
struct ecmdNameVectorEntry {
#ifndef DOCUMENTATION
  // Constructor
  ecmdNameVectorEntry();

  // Destructor
  ~ecmdNameVectorEntry();

  // Methods
  uint32_t flatten(uint8_t *o_buf, uint32_t i_len) const;
  uint32_t unflatten(const uint8_t *i_buf, uint32_t i_len);
  uint32_t flattenSize(void) const;
#ifndef ECMD_STRIP_DEBUG
  void printStruct(void) const;
#endif
#endif

  // Members
  std::string    name;                  ///< Name of entry 
  std::vector< std::vector<ecmdDataBuffer> > buffer;   ///< Vector of data to/from entry
  std::vector<uint32_t> iteration;      ///< Vector of iteration values to retrieve
  uint32_t       rc;                    ///< Error code in retrieving this entry
};

#ifndef DOCUMENTATION
// ecmdNameVectorEntry Constructor
inline ecmdNameVectorEntry::ecmdNameVectorEntry():
rc(0)
{
}

// ecmdNameVectorEntry Destructor
inline ecmdNameVectorEntry::~ecmdNameVectorEntry() { }
#endif

/**
 @brief Used by ???? function to pass data
*/
struct ecmdIndexVectorEntry {
#ifndef DOCUMENTATION
  // Constructor
  ecmdIndexVectorEntry();

  // Destructor
  ~ecmdIndexVectorEntry();

  // Methods
  uint32_t flatten(uint8_t *o_buf, uint32_t i_len) const;
  uint32_t unflatten(const uint8_t *i_buf, uint32_t i_len);
  uint32_t flattenSize(void) const;
#ifndef ECMD_STRIP_DEBUG
  void printStruct(void) const;
#endif
#endif

  // Members
  int            index;                 ///< Index of entry
  std::vector<ecmdDataBuffer> buffer;   ///< Vector of data to/from entry
  uint32_t       rc;                    ///< Error code in retrieving this entry
};

#ifndef DOCUMENTATION
// ecmdIndexEntry Constructor
inline ecmdIndexVectorEntry::ecmdIndexVectorEntry():
index(0),
rc(0)
{
}

// ecmdIndexEntry Destructor
inline ecmdIndexVectorEntry::~ecmdIndexVectorEntry() { }
#endif


/**
 @brief Used by get/put Gpr/Fpr Multiple function to pass data
*/
struct ecmdIndexEntry {
#ifndef DOCUMENTATION
  // Constructor
  ecmdIndexEntry();

  // Destructor
  ~ecmdIndexEntry();

  // Methods
  uint32_t flatten(uint8_t *o_buf, uint32_t i_len) const;
  uint32_t unflatten(const uint8_t *i_buf, uint32_t i_len);
  uint32_t flattenSize(void) const;
#ifndef ECMD_STRIP_DEBUG
  void printStruct(void) const;
#endif
#endif

  // Members
  int            index;                 ///< Index of entry
  ecmdDataBuffer buffer;                ///< Data to/from entry
  uint32_t       rc;                    ///< Error code in retrieving this entry
}; 

#ifndef DOCUMENTATION
// ecmdIndexEntry Constructor
inline ecmdIndexEntry::ecmdIndexEntry():
index(0),
rc(0)
{
}

// ecmdIndexEntry Destructor
inline ecmdIndexEntry::~ecmdIndexEntry() { }
#endif

/**
 @brief Used by getlatch function to return data
*/
struct ecmdLatchEntry {
#ifndef DOCUMENTATION
  // Constructor
  ecmdLatchEntry();

  // Destructor
  ~ecmdLatchEntry();

  // Methods
  uint32_t flatten(uint8_t *o_buf, uint32_t i_len) const;
  uint32_t unflatten(const uint8_t *i_buf, uint32_t i_len);
  uint32_t flattenSize(void) const;
#ifndef ECMD_STRIP_DEBUG
  void printStruct(void) const;
#endif
#endif

  // Members
  std::string      ringName;              ///< Ring that latch came from
  std::string      latchName;             ///< Latch name of entry
  ecmdLatchType_t  latchType;             ///< Specifies what type of latch this is (single bit, multibit, etc..)
  uint32_t         fsiRingOffset;         ///< Ring Offset for FSI
  uint32_t         jtagRingOffset;        ///< Ring Offset for JTAG
  uint32_t         length;                ///< Length of entry
  uint32_t         latchStartBit;         ///< Start bit of data inside latch
  uint32_t         latchEndBit;           ///< End bit of data inside latch
  ecmdDataBuffer   buffer;                ///< Latch data (When used by query functions this value is not used.)
  uint32_t         rc;                    ///< Error code in retrieving this entry
};

#ifndef DOCUMENTATION
// ecmdLatchEntry Constructor
inline ecmdLatchEntry::ecmdLatchEntry():
latchType(ECMD_LATCHTYPE_UNKNOWN),
fsiRingOffset(0),
jtagRingOffset(0),
length(0),
latchStartBit(0),
latchEndBit(0),
rc(0)
{
}

// ecmdLatchEntry Destructor
inline ecmdLatchEntry::~ecmdLatchEntry() { }
#endif

/**
 @brief Used by ecmdQueryLatchInfo function to return data
*/
struct ecmdLatchQueryData {
#ifndef DOCUMENTATION
  // Constructor
  ecmdLatchQueryData();

  // Destructor
  ~ecmdLatchQueryData();

  // Methods
  uint32_t flatten(uint8_t *o_buf, uint32_t i_len) const;
  uint32_t unflatten(const uint8_t *i_buf, uint32_t i_len);
  uint32_t flattenSize(void) const;
#ifndef ECMD_STRIP_DEBUG
  void printStruct(void) const;
#endif
#endif

  bool isChipUnitMatch(std::string &i_chipUnitType); ///< Checks if relatedChipUnit variable matches input

  // Members
  std::string               ringName;              ///< (Detail: Low)  Ring that latch came from
  std::string               latchName;             ///< (Detail: Low)  Latch name of entry
  uint32_t                  fsiStartRingOffset;    ///< (Detail: Low)  Starting Ring Offset for FSI
  uint32_t                  jtagStartRingOffset;   ///< (Detail: Low)  Starting Ring Offset for JTAG
  uint32_t                  fsiEndRingOffset;      ///< (Detail: Low)  Ending Ring Offset for FSI
  uint32_t                  jtagEndRingOffset;     ///< (Detail: Low)  Ending Ring Offset for JTAG
  uint32_t                  latchStartBit;         ///< (Detail: Low)  Start bit of data inside latch
  uint32_t                  latchEndBit;           ///< (Detail: Low)  End bit of data inside latch
  uint32_t                  bitLength;             ///< (Detail: Low)  length of latch, sum of all the parts
  bool                      isChipUnitRelated;     ///< (Detail: Low)  This latch is related to the chipUnit level of a chip
  std::string               relatedChipUnit;       ///< (Detail: Low)  If isChipUnitRelated, this is the name of the chipUnit
  std::string               relatedChipUnitShort;  ///< (Detail: Low)  If isChipUnitRelated, this is the short name of the chipUnit
  std::list<ecmdLatchEntry> scandefLatchInfo;      ///< (Detail: Low)  Information about this latch from the scandef that can be used to pass into other latch functions to improve performance.  Many of the other elements of this structure are derived from this data as well.
  std::string               clockDomain;           ///< (Detail: High) Clock domain this latch belongs to
  ecmdClockState_t          clockState;            ///< (Detail: High) Required clock state to access this latch
};

#ifndef DOCUMENTATION
// ecmdLatchQueryData Constructor
inline ecmdLatchQueryData::ecmdLatchQueryData():
fsiStartRingOffset(0),
jtagStartRingOffset(0),
fsiEndRingOffset(0),
jtagEndRingOffset(0),
latchStartBit(0),
latchEndBit(0),
bitLength(0),
isChipUnitRelated(false),
clockState((ecmdClockState_t) ECMD_CLOCKSTATE_UNKNOWN)
{
}

// ecmdLatchQueryData Destructor
inline ecmdLatchQueryData::~ecmdLatchQueryData() { }
#endif

/**
 @brief Used by ecmdQueryProcRegisterInfo function to return data about a Architected register
*/
struct ecmdProcRegisterInfo {
#ifndef DOCUMENTATION
  // Constructor
  ecmdProcRegisterInfo();

  // Destructor
  ~ecmdProcRegisterInfo();

  // Methods
  uint32_t flatten(uint8_t *o_buf, uint32_t i_len) const;
  uint32_t unflatten(const uint8_t *i_buf, uint32_t i_len);
  uint32_t flattenSize(void) const;
#ifndef ECMD_STRIP_DEBUG
  void printStruct(void) const;
#endif
#endif

  bool isChipUnitMatch(std::string &i_chipUnitType); ///< Checks if relatedChipUnit variable matches input

  // Members
  uint32_t      bitLength;              ///< Bit length of each entry
  uint32_t      totalEntries;           ///< Total number of entries available
  ecmdProcRegMode_t mode;               ///< (Detail: Low)  The mode associated with this register
  bool          isChipUnitRelated;      ///< (Detail: Low)  This spy is related to the chipUnit level of a chip
  std::string   relatedChipUnit;        ///< (Detail: Low)  If isChipUnitRelated, this is the name of the chipUnit
  std::string   relatedChipUnitShort;   ///< (Detail: Low)  If isChipUnitRelated, this is the short name of the chipUnit
  bool          threadReplicated;       ///< Register is replicated for each thread
};

#ifndef DOCUMENTATION
// ecmdProcRegisterInfo Constructor
inline ecmdProcRegisterInfo::ecmdProcRegisterInfo():
bitLength(0),
totalEntries(0),
isChipUnitRelated(false),
threadReplicated(false)
{
}

// ecmdProcRegisterInfo Destructor
inline ecmdProcRegisterInfo::~ecmdProcRegisterInfo() { }
#endif

/**
 @brief Used to return information for cache query information
*/
struct ecmdCacheData {
#ifndef DOCUMENTATION
  // Constructor
  ecmdCacheData();

  // Destructor
  ~ecmdCacheData();
  
  // Methods
  uint32_t flatten(uint8_t *o_buf, uint32_t i_len) const;
  uint32_t unflatten(const uint8_t *i_buf, uint32_t i_len);
  uint32_t flattenSize(void) const;
#ifndef ECMD_STRIP_DEBUG
  void printStruct(void) const;
#endif
#endif

  bool isChipUnitMatch(std::string &i_chipUnitType); ///< Checks if relatedChipUnit variable matches input

  bool          isChipUnitRelated;      ///< Is the level of cache chipUnit related
  std::string   relatedChipUnit;        ///< (Detail: Low)  If isChipUnitRelated, this is the name of the chipUnit
  std::string   relatedChipUnitShort;   ///< (Detail: Low)  If isChipUnitRelated, this is the short name of the chipUnit
};


#ifndef DOCUMENTATION
// ecmdCacheData Constructor
inline ecmdCacheData::ecmdCacheData():
isChipUnitRelated(false)
{
}

// ecmdCacheData Destructor
inline ecmdCacheData::~ecmdCacheData() { }
#endif

// This Structure is moved here from  ecmdRingUser.C
struct ecmdLatchDataEntry {
  std::string ringName;                 ///< Name of ring that contains this latch
  std::string latchName;                ///< Full Latch Name (including any parens)
  uint32_t fsiRingOffset;               ///< Ring Offset for FSI
  uint32_t jtagRingOffset;              ///< Ring Offset for JTAG
  uint32_t length;                      ///< Length of entry
  uint32_t latchStartBit;               ///< Start bit in latch (comes from parens in latch name)
  uint32_t latchEndBit;                 ///< End bit in latch (comes from parens in latch name)
  ecmdLatchType_t  latchType;           ///< Specifies what type of latch this is (single bit, multibit, etc..)
};


/**
 @brief Used by ecmdSpyData when returning the latches that are part of a spy
*/
struct ecmdSpyLatchData {
  std::string latchName;     ///< The latch name, including the ()'s
  uint32_t hashKey;          ///< The hashKey for the latchName
  int offset;                ///< The starting bit offset of this latch in the spy
  uint32_t length;           ///< The length in bits of this latch
  int lhsNum;                ///< left hand side Number - Value x of an (x:y) entry
  int rhsNum;                ///< right hand side Number - Value y of an (x:y) entry
};


/**
 @brief Used for the ecmdQuerySpy function to return spy info
*/
struct ecmdSpyData {

#ifndef DOCUMENTATION
  // Constructor
  ecmdSpyData();

  // Destructor
  ~ecmdSpyData();

  // Methods
  uint32_t flatten(uint8_t *o_buf, uint32_t i_len) const;
  uint32_t unflatten(const uint8_t *i_buf, uint32_t i_len);
  uint32_t flattenSize(void) const;
#ifndef ECMD_STRIP_DEBUG
  void printStruct(void) const;
#endif
#endif

  bool isChipUnitMatch(std::string &i_chipUnitType); ///< Checks if relatedChipUnit variable matches input

  // Members
  std::string   spyName;                   ///< Names used to reference this spy
  uint64_t      spyId;                     ///< (Detail: Low)  Hashed spy ID
  uint32_t      bitLength;                 ///< length of spy
  ecmdSpyType_t spyType;                   ///< Type of spy
  bool          isEccChecked;              ///< This spy affects some ECC groupings
  bool          isEnumerated;              ///< This spy has enumerated values
  bool          isChipUnitRelated;         ///< (Detail: Low)  This spy is related to the chipUnit level of a chip
  std::string   relatedChipUnit;           ///< (Detail: Low)  If isChipUnitRelated, this is the name of the chipUnit
  std::string   relatedChipUnitShort;      ///< (Detail: Low)  If isChipUnitRelated, this is the short name of the chipUnit

  std::string           clockDomain;       ///< Clock domain this spy belongs to
  ecmdClockState_t       clockState;       ///< Required clock state to access this spy
  
  std::list<std::string> enums;            ///< Possible enum values for Spy - I/P Can only provide this on a client, not on the FSP
  std::list<std::string> epCheckers;       ///< Possible epChecker names affected by this Spy

  std::list<ecmdSpyLatchData> spyLatches;  ///< The latches that are associated with a spy, not valid with group spies
};

#ifndef DOCUMENTATION
// ecmdSpyData Constructor
inline ecmdSpyData::ecmdSpyData():
spyId(0),
bitLength(0),
isEccChecked(false),
isEnumerated(false),
isChipUnitRelated(false)
{
}

// ecmdSpyData Destructor
inline ecmdSpyData::~ecmdSpyData() { }
#endif

/**
 @brief Used by ecmdReadDcard
*/
struct ecmdMemoryEntry {
  uint64_t address;
  ecmdDataBuffer data;
  ecmdDataBuffer tags;
};

/**
 @brief Used by ecmdI2CMultipleCmds API
*/
struct ecmdI2CCmdEntry {

#ifndef DOCUMENTATION
  // Constructor
  ecmdI2CCmdEntry();

  // Destructor
  ~ecmdI2CCmdEntry();

  // Methods
  uint32_t flatten(uint8_t *o_buf, uint32_t i_len) const;
  uint32_t unflatten(const uint8_t *i_buf, uint32_t i_len);
  uint32_t flattenSize(void) const;
#ifndef ECMD_STRIP_DEBUG
  void printStruct(void) const;
#endif
#endif

  // Members
  ecmdI2CCmds_t ecmdI2CCmd;     ///< The eCMD API to target with this command
  uint32_t engineId;            ///< The i2c engine to use
  uint32_t port;                ///< The i2c port to use
  uint32_t slaveaddress;        ///< The i2c slave device address to use
  ecmdI2cBusSpeed_t busSpeed;   ///< The i2c bus speed to use
  uint32_t byteOffset;          ///< Byte offset in the device (needed for offset commands only)
  uint32_t offsetFieldSize;     ///< Specifies the field size used in the i2c protocal of the slave device (needed for offset commands only)
  uint32_t readByteLength;      ///< Byte length to read (only needed on reads)
  ecmdDataBuffer data;          ///< The data returned from the i2c operation
  uint32_t i2cFlags;            ///< The i2c flags to use
};

#ifndef DOCUMENTATION
// ecmdI2CCmdEntry Constructor
inline ecmdI2CCmdEntry::ecmdI2CCmdEntry():
ecmdI2CCmd(ECMD_I2C_UNKNOWN),                //fix beam error
engineId(0),
port(0),
slaveaddress(0),
busSpeed(ECMD_I2C_BUSSPEED_UNKNOWN),         //fix beam error
byteOffset(0),
offsetFieldSize(0),
readByteLength(0),                           //fix beam error
i2cFlags(0)
{
}

// ecmdI2CCmdEntry Destructor
inline ecmdI2CCmdEntry::~ecmdI2CCmdEntry() { }
#endif

/**
 @brief Used for the ecmdQueryConnectedTargets function to return connection info
*/
struct ecmdConnectionData {

#ifndef DOCUMENTATION
  // Constructor
  ecmdConnectionData();

  // Destructor
  ~ecmdConnectionData();

  // Methods
  uint32_t flatten(uint8_t *o_buf, uint32_t i_len) const;
  uint32_t unflatten(const uint8_t *i_buf, uint32_t i_len);
  uint32_t flattenSize(void) const;
#ifndef ECMD_STRIP_DEBUG
  void printStruct(void) const;
#endif
#endif

  // Members
  ecmdChipTarget targetA;       ///< The chip target of chip A, the one that initiated the connection
  std::string portA;            ///< The port on chip A that the connection is off of
  std::string connectionType;   ///< The connectionType between ports A and B
  std::string portB;            ///< The port on chip B that the connection is off of
  ecmdChipTarget targetB;       ///< The chip target of chip B, the chip connected to A
};

#ifndef DOCUMENTATION
// ecmdConnectionData Constructor
inline ecmdConnectionData::ecmdConnectionData() {
}

// ecmdConnectionData Destructor
inline ecmdConnectionData::~ecmdConnectionData() { }
#endif

/* @name Misc functions */
//@{

/** @brief Returns the version of the shared lib so it can be compared with the other versions */
std::string ecmdGetSharedLibVersion();

//@}

/**
 Defines for getMemPba 
*/
#define PBA_MODE_LCO           0x00000001  ///< Use LCO mode with putMemPba
#define PBA_MODE_INJECT        0x00000002  ///< Use Inject mode with putMemPba
#define PBA_MODE_DMA           0x00000003  ///< Use DMA mode with putMemPba
#define PBA_MODE_CACHE_INHIBIT 0x00000004  ///< Use Cache inhibit mode with putMemPba
#define PBA_MODE_MASK          0x000000FF  ///< Mask for determining PBA mode
#if !defined(ECMD_PERLAPI) && !defined(ECMD_PYAPI)
#define PBA_OPTION_PASSTHROUGH 0x80000000  ///< Use SBE Host Passthrough, input address will be ignored
#endif

/**
 Defines for getTraceArrayMultiple 
*/
#define ECMD_TRACE_ARRAY_START 0x00000001
#define ECMD_TRACE_ARRAY_STOP  0x00000002
#define ECMD_TRACE_IGNORE_MUX  0x00000004

/**
 Defines for chipCleanup
*/
#define ECMD_CHIP_CLEANUP_UNUSED    0x00000000
#define ECMD_CHIP_CLEANUP_MEMDA     0x00000001
#define ECMD_CHIP_CLEANUP_PM        0x00000002

/**
 Defines for stopClocks
*/
#define ECMD_STOP_CLOCK_MODE_SKIP_IOVALID   0x00000001
#define ECMD_STOP_CLOCK_MODE_ASYNC          0x00000002

/**
 Defines for SPI access
*/
#define ECMD_SPI_ECC_ENABLE     0x00000001

/**
@brief Used in ecmdReadDigitalThermalSensor function to get the dts readings for an ex target
*/
struct ecmdDTSData{
  uint8_t                   num_of_sensors;             ///< Number of sensors in an ex target as specified in the h/w procedure
  std::vector<std::string>  name_of_sensors;            ///< Name of sensors as returned by the h/w procedure
  std::vector<uint64_t>     temp_of_sensors;            ///< Temperature readings in requested ecmdThermalUnit
  std::vector<uint64_t>     validity_of_sensor_reading; ///< Flag indicating whether the reading is valid; If set, then reading is valid 
  std::vector<uint64_t>     tripLevel_of_reading;       ///< Indicates whether the sensor result is above threshold trip level:
                                                        ///< 0 - no trip; 1 - warning; 2 - critical; 3 - fatal
};

/**
 Defines for getMemProc and putMemProc
*/
#define MEMPROC_CACHE_INHIBIT  0x00000001


/**
 Defines for ring/spy/latch functions 
*/
#define ECMD_RING_MODE_HEADER_CHECK            0x00000000
#if !defined(ECMD_PERLAPI) && !defined(ECMD_PYAPI)
#define ECMD_RING_MODE_MULTICAST               0x80000000
#endif
#define ECMD_RING_MODE_SET_PULSE_NO_OPCG_COND  0x40000000
#define ECMD_RING_MODE_NO_HEADER_CHECK         0x20000000
#define ECMD_RING_MODE_SET_PULSE_NSL           0x10000000
#define ECMD_RING_MODE_SET_PULSE_SL            0x08000000
#define ECMD_RING_MODE_SET_PULSE_ALL           0x04000000
#define ECMD_RING_MODE_SPARSE_ACCESS           0x02000000
#define ECMD_RING_MODE_FASTARRAY               0x01000000
#define ECMD_RING_MODE_MULTICAST_MASK          0x000000FF

/**
 @brief Used for the ecmdGetPnor and ecmdPutPnor function
*/

typedef enum {
    ECMD_PNOR_TYPE_DATA      = 1,
    ECMD_PNOR_TYPE_LOGICAL   = 2,
    ECMD_PNOR_TYPE_PARTITION = 3
} ecmd_pnor_types;

typedef enum {
    ECMD_PNOR_FLAGS_U_BOOT_ENV = 0x0002,
    ECMD_PNOR_FLAGS_PROTECTED  = 0x0001
} ecmd_pnor_flags;

typedef enum {
    ECMD_PNOR_MODE_DEFAULT,
    ECMD_PNOR_MODE_ERASE
} ecmd_pnor_modes;

struct ecmdPnorListEntryData {
#ifndef DOCUMENTATION
    // Constructor
    ecmdPnorListEntryData();

    // Destructor
    ~ecmdPnorListEntryData();
#endif

    // Members
    std::string     entryName;      ///< (Detail: Low) The entry name
    uint32_t        entryBase;      ///< (Detail: Low) The entry base
    uint32_t        entrySize;      ///< (Detail: Low) The entry size
    uint32_t        entryPid;       ///< (Detail: Low) The entry PID
    uint32_t        entryId;        ///< (Detail: Low) The entry ID
    uint32_t        entryType;      ///< (Detail: Low) The entry type
    uint32_t        entryFlags;     ///< (Detail: Low) The entry flags
    uint32_t        entryActual;    ///< (Detail: Low) The entry actual 
    uint32_t        entryChecksum;  ///< (Detail: Low) The entry checksum
};

#ifndef DOCUMENTATION
//ecmdPnorListEntryData Constructor
inline ecmdPnorListEntryData::ecmdPnorListEntryData():
entryName(""),
entryBase(0x0),
entrySize(0x0),
entryPid(0x0),
entryId(0x0),
entryType(0x0),
entryFlags(0x0),
entryActual(0x0),
entryChecksum(0x0)
{
}

// ecmdPnorListEntryData destructor
inline ecmdPnorListEntryData::~ecmdPnorListEntryData() { }
#endif

struct ecmdPnorListData {
#ifndef DOCUMENTATION
    // Constructor
    ecmdPnorListData();

    // Destructor
    ~ecmdPnorListData();
#endif

    // Members
    uint32_t          hdrMagic;           ///< (Detail: Low) Magic value
    uint32_t          hdrVersion;         ///< (Detail: Low) Version value
    uint32_t          hdrSize;            ///< (Detail: Low) Size of the structure
    uint32_t          hdrEntrySize;       ///< (Detail: Low) Entry size
    uint32_t          hdrEntryCount;      ///< (Detail: Low) Number of Entries
    uint32_t          hdrBlockSize;       ///< (Detail: Low) Block size
    uint32_t          hdrBlockCount;      ///< (Detail: Low) Block count
    uint32_t          hdrChecksum;        ///< (Detail: Low) Checksum of image
    std::list<ecmdPnorListEntryData>    hdrEntries; ///< (Detail: Low) List of entry data
};

#ifndef DOCUMENTATION
// ecmdPnorListData Constructor
inline ecmdPnorListData::ecmdPnorListData():
hdrMagic(0x0),
hdrVersion(0x0),
hdrSize(0x0),
hdrEntrySize(0x0),
hdrEntryCount(0x0),
hdrBlockSize(0x0),
hdrBlockCount(0x0),
hdrChecksum(0x0)
{
}

// ecmdPnorListData Destructor
inline ecmdPnorListData::~ecmdPnorListData() 
{
    if ( hdrEntries.size() != 0 )
    {
        std::list<ecmdPnorListEntryData>::iterator myIt;
        for( myIt = hdrEntries.begin(); myIt != hdrEntries.end(); myIt++ )
        {
            myIt = hdrEntries.erase(myIt);
        }
    }
}
#endif

/* 
 * Defines for ecmdSetConfigurationComplexHidden mode
 */
#define ECMD_SETCONFIG_MODE_REPLACE 0x00000000
#define ECMD_SETCONFIG_MODE_ADD     0x00000001
#define ECMD_SETCONFIG_MODE_REMOVE  0x00000002

/* 
 * Defines for getsram mode
 */
#define ECMD_SRAM_MODE_CHANNEL_MASK     0x0000000F
#define ECMD_SRAM_MODE_CHANNEL_SHIFT    0
#define ECMD_SRAM_MODE_PECTOP_MASK      0x000000F0
#define ECMD_SRAM_MODE_PECTOP_SHIFT     4
#define ECMD_SRAM_MODE_PECPHY_MASK      0x00000F00
#define ECMD_SRAM_MODE_PECPHY_SHIFT     8

#endif
