# Makefile for the ecmd capi

# *****************************************************************************
# Include the common base makefile
# *****************************************************************************
include ../../makefile.base

### Includes
CAPI_INCLUDES := ecmdClientCapi.H  ecmdDataBufferBase.H ecmdDataBuffer.H ecmdReturnCodes.H 
CAPI_INCLUDES := ${CAPI_INCLUDES} ecmdStructs.H ecmdUtils.H ecmdSharedUtils.H ecmdChipTargetCompare.H
CAPI_INCLUDES := ${CAPI_INCLUDES} ecmdDefines.H prdfCompressBuffer.H ecmdTraceDefines.H ecmdIstep.H
INT_INCLUDES  := ecmdDllCapi.H

### Source
CAPI_SOURCE := ecmdClientCapi.C ecmdUtils.C ecmdClientCapiFunc.C
SLIB_SOURCE := ecmdDataBufferBase.C
SLIB_SOURCE += ecmdDataBuffer.C
SLIB_SOURCE += ecmdStructs.C
SLIB_SOURCE += ecmdSharedUtils.C
SLIB_SOURCE += ecmdWriteTarget.C
SLIB_SOURCE += ecmdParseTokens.C
SLIB_SOURCE += ecmdChipTargetCompare.C
SLIB_SOURCE += ecmdIstep.C

### Generated source by makedll.pl
GENERATED_INCLUDES := ecmdDllCapi.H ecmdClientEnums.H
GENERATED_SOURCE   := ecmdClientCapiFunc.C
CXXFLAGS           := ${CXXFLAGS} -I${SRCPATH}
VPATH              := ${VPATH}:${SRCPATH}

# Tell the SEDC code we import that we just want the scomdef parser
SEDC_INCLUDES := sedcScomdefParser.H sedcScomdefClasses.H sedcDefines.H 
SEDC_INCLUDES := ${SEDC_INCLUDES} sedcCommonParser.H sedcCommonClasses.H
CAPI_SOURCE   := ${CAPI_SOURCE} sedcScomdefParser.C sedcScomdefClasses.C 
CAPI_SOURCE   := ${CAPI_SOURCE} sedcCommonParser.C sedcCommonClasses.C
VPATH         := ${VPATH}:sedcScomdef
CXXFLAGS      := ${CXXFLAGS} -IsedcScomdef
# Variables for install
SEDC_INCLUDES_INSTALL := $(addprefix sedcScomdef/, ${SEDC_INCLUDES})

# Add on the object output path
VPATH := ${VPATH}:${OBJPATH}

# Compile libecmd optimized
# Set optimaztion level to 3 if not set in the included makefiles
OPT_LEVEL     ?= -O3
SLIB_CXXFLAGS := ${CXXFLAGS} ${OPT_LEVEL}
# Do not compile all with optimization until move to shared object
#CXXFLAGS := ${CXXFLAGS} ${OPT_LEVEL}

# Direct where to get zlib from
ifeq (${TARGET_ARCH},aix)
  ZLIB := /gsa/${GSACELL}/projects/e/ecmd/utils/aix/lib/libz.a
else ifeq (${TARGET_ARCH},x86)
  ZLIB := /gsa/${GSACELL}/projects/e/ecmd/utils/x86/lib/libz.a
else
  ZLIB := -lz
endif

### Concat variables together for the build
INCLUDES := ${CAPI_INCLUDES} ${SEDC_INCLUDES}

# *****************************************************************************
# Define our output files
# *****************************************************************************
TARGET   := ecmdClientCapi.a
SLIB     := libecmd.so

# *****************************************************************************
# The Main Targets
# *****************************************************************************
# The run-all rule is defined in makefile.rules
all:
	${run-all}

generate: ${GENERATED_SOURCE} ${GENERATED_INCLUDES}

build: ${TARGET} ${SLIB}

test:
  # Do nothing

install:
	@echo "Installing eCMD Client API to ${INSTALL_PATH}/capi/ ..."
	@mkdir -p ${INSTALL_PATH}/capi/
	cp ${OUTLIB}/${TARGET} ${INSTALL_PATH}/${TARGET_ARCH}/lib/.
	cp ${CAPI_INCLUDES} ${INSTALL_PATH}/capi/.
	cp ${SEDC_INCLUDES_INSTALL} ${INSTALL_PATH}/capi/.
	chmod 644 ${INSTALL_PATH}/capi/*
	@echo "Installing eCMD Shared Lib to ${INSTALL_PATH}/${TARGET_ARCH}/lib/ ..."
	cp ${OUTLIB}/${SLIB} ${INSTALL_PATH}/${TARGET_ARCH}/lib/.

doxygen-capi:
	@cp ${CAPI_INCLUDES} ${DOXYGEN_CAPI_PATH}/.

doxygen-perlapi:
	@cp ${CAPI_INCLUDES} ${DOXYGEN_PERLAPI_PATH}/.

doxygen-pyapi:
	@cp ${CAPI_INCLUDES} ${DOXYGEN_PYAPI_PATH}/.

# *****************************************************************************
# Object Build Targets
# *****************************************************************************
CAPI_SOURCE_OBJS := $(basename ${CAPI_SOURCE})
CAPI_SOURCE_OBJS := $(addprefix ${OBJPATH}, ${CAPI_SOURCE_OBJS})
CAPI_SOURCE_OBJS := $(addsuffix .o, ${CAPI_SOURCE_OBJS})
SLIB_SOURCE_OBJS := $(basename ${SLIB_SOURCE})
SLIB_SOURCE_OBJS := $(addprefix ${OBJPATH}, ${SLIB_SOURCE_OBJS})
SLIB_SOURCE_OBJS := $(addsuffix .o, ${SLIB_SOURCE_OBJS})

# *****************************************************************************
# Compile code for the common C++ objects if their respective
# code has been changed.  Or, compile everything if a header
# file has changed.
# *****************************************************************************
${CAPI_SOURCE_OBJS}: ${OBJPATH}%.o : %.C ${INCLUDES} ${INT_INCLUDES}
	@echo Compiling $<
	${VERBOSE}${CXX} -c ${CXXFLAGS} $< -o $@ ${DEFINES}
${SLIB_SOURCE_OBJS}: ${OBJPATH}%.o : %.C ${INCLUDES} ${INT_INCLUDES}
	@echo Compiling $<
	${VERBOSE}${CXX} -c ${SLIB_CXXFLAGS} $< -o $@ ${DEFINES}

# *****************************************************************************
# Create the Client Archive
# *****************************************************************************
${TARGET}: ${CAPI_SOURCE_OBJS}
	@echo Creating $@
	${VERBOSE}${AR} r ${OUTLIB}/${TARGET} $^

# *****************************************************************************
# Create the Shared Library
# *****************************************************************************
${SLIB}: ${SLIB_SOURCE_OBJS}
	@echo Linking $@
	${VERBOSE}${LD} ${SLDFLAGS} -o ${OUTLIB}/${SLIB} $^ ${ZLIB} -ldl

# *****************************************************************************
# Autogenerate the Client side of the Dll
# *****************************************************************************
${GENERATED_SOURCE} ${GENERATED_INCLUDES}: ecmdClientCapi.H ${ECMD_ROOT}/mkscripts/makedll.pl
	@echo "Generating $@"
	@${ECMD_ROOT}/mkscripts/makedll.pl ecmd $@

# *****************************************************************************
# Include any global default rules
# *****************************************************************************
include ${ECMD_ROOT}/makefile.rules
