# Makefile for the ecmd Perl Module

# *****************************************************************************
# Include the common base makefile
# *****************************************************************************
include ../../makefile.base

### Includes
INCLUDES := ecmdClientCapi.H  ecmdDataBufferBase.H  ecmdDataBuffer.H ecmdReturnCodes.H 
INCLUDES := ${INCLUDES} ecmdStructs.H ecmdClientPerlapi.H ecmdBit64.H

### Source
SOURCE := ecmdClientPerlapi.C ecmdClientPerlapiFunc.C ecmdBit64.C

### Generated source by makepm.pl
GENERATED_SOURCE   := ecmdClientPerlapiFunc.C
GENERATED_INCLUDES := ecmdClientPerlapiFunc.H ecmdClientPerlapiIterators.H

### Generated source by swig
SWIG_SOURCE := ecmdClientPerlapiSwig.C

### Generated swig input files by makeext.pm
GENERATED_SWIG_INPUT := ecmdExtPerlIncludes.i ecmdExtPerlInserts.i

### SWIG input files
SWIG_INPUT := ecmdClientPerlapi.i ecmdString.i ecmdVector.i ecmdList.i ecmdCommon.i ecmdConst.i 
SWIG_INPUT := ${SWIG_INPUT} ecmdClientPerlapi.pl ecmdExtPerlIncludes.i ecmdExtPerlInserts.i

# We may have to pickup a different version of ecmdClientPerlapi.pl depending upon the distro
# This is the logic to setup the variable before it's used below
ifeq (aix,${DISTRO})
  INC_DISTRO := ${DISTRO}
else
  INC_DISTRO := default
endif

# Setup all the options to pass into swig
SWIGFLAGS := ${SWIGFLAGS} -c++ ${SWIGCPPPERL} -exportall -const -I${ECMD_CORE}/capi -I${ECMD_CORE}/dll -I${SRCPATH} -Idistro/${INC_DISTRO} -DECMD_PERLAPI -I../perlapi

### Setup our CXXFLAGS and VPATH
CXXFLAGS := ${CXXFLAGS} -I${ECMD_CORE}/capi -I${ECMD_CORE}/dll -I${SRCPATH}
VPATH    := ${VPATH}:${ECMD_CORE}/capi:${SRCPATH}:distro/${INC_DISTRO}:${OBJPATH}

### Add all of the extension information onto our variables
INCLUDES  := ${INCLUDES} $(foreach ext, ${EXT_PYAPI}, ${ext}ClientPerlapi.H ${ext}ClientCapi.H)
SOURCE    := ${SOURCE} $(foreach ext, ${EXT_PYAPI}, ${ext}ClientPerlapi.C ${ext}ClientPerlapiFunc.C)
# Remove the fapi2ClientPerlapiFunc.C from the build, only will use fapi2ClientPerlapi.C which has the init extension
SOURCE    := $(subst fapi2ClientPerlapiFunc.C,,${SOURCE})
CXXFLAGS  := ${CXXFLAGS} $(foreach ext, ${EXT_PYAPI},-I${EXT_${ext}_PATH}/capi -I${EXT_${ext}_PATH}/perlapi)
SWIGFLAGS := ${SWIGFLAGS} $(foreach ext, ${EXT_PYAPI},-I${EXT_${ext}_PATH}/perlapi -I${EXT_${ext}_PATH}/capi)
VPATH     := ${VPATH}$(foreach ext, ${EXT_PYAPI},:${EXT_${ext}_PATH}/capi:${EXT_${ext}_PATH}/perlapi)

### Setup all of the additional objects to link
EXTLIBS   := ${EXTLIBS} ${foreach ext,${EXT_PERLAPI},$(shell if [ -e ${OUTLIB}/lib${ext}.so ]; then echo -l${ext}; fi)}

# The reference to libecmd.so has to go at the end of the link in Ubuntu
# Make it it's own variable instead of a SLDFLAGS
LDLIBS    := ${LDLIBS} -L${OUTLIB} -lecmd

# *****************************************************************************
# Define our output targets
# *****************************************************************************
PM_TARGET := ecmd.pm
SO_TARGET := ecmd.so

# *****************************************************************************
# The Main Targets
# *****************************************************************************
# The run-all rule is defined in makefile.rules
all:
	${run-all}

generate: ${SWIG_SOURCE} ${GENERATED_SOURCE} ${GENERATED_INCLUDES}

build: ${SO_TARGET}

install:
	@echo "Installing eCMD PerlApi to ${INSTALL_PATH}/${TARGET_ARCH}/perl ..."
	cp ${OUTPERL}/${PM_TARGET} ${INSTALL_PATH}/${TARGET_ARCH}/perl/.
	cp ${OUTPERL}/${SO_TARGET} ${INSTALL_PATH}/${TARGET_ARCH}/perl/.

test:
	@echo "***** If you see perl load errors this build of the perl module is invalid, ignore ECMD_DLL_FILE errors ****"
        ifeq (${TARGET_BARCH},aix)
	@slibclean
	ECMD_DLL_FILE=${OUTLIB}/stub.dll PERL5LIB=${OUTPERL} LIBPATH=${OUTLIB}:${OUTPERL} ECMD_ARCH=${TARGET_ARCH} ./testBuild.pl
        else
	ECMD_DLL_FILE=${OUTLIB}/stub.dll PERL5LIB=${OUTPERL} LD_LIBRARY_PATH=${OUTLIB}:${OUTPERL} ECMD_ARCH=${TARGET_ARCH} ./testBuild.pl
        endif

doxygen-perlapi: ecmdClientPerlapiFunc.H
	@cp ecmdBit64.H ecmdPerlApiTypes.H ${DOXYGEN_PERLAPI_PATH}/.
        # grep out PerlapiFunc.H to eliminate doxygen error that happens from having mismatched file names in comments
	@cat ecmdClientPerlapi.H ${SRCPATH}/ecmdClientPerlapiFunc.H | grep -v ecmdClientPerlapiFunc.H > ${DOXYGEN_PERLAPI_PATH}/ecmdClientPerlapi.H

# *****************************************************************************
# Object Build Targets
# *****************************************************************************
SOURCE_OBJS := $(basename ${SOURCE})
SOURCE_OBJS := $(addprefix ${OBJPATH}, ${SOURCE_OBJS})
SOURCE_OBJS := $(addsuffix .o, ${SOURCE_OBJS})
SWIG_OBJS   := $(basename ${SWIG_SOURCE})
SWIG_OBJS   := $(addprefix ${OBJPATH}, ${SWIG_OBJS})
SWIG_OBJS   := $(addsuffix .o, ${SWIG_OBJS})

# *****************************************************************************
# Compile code for the common C++ objects if their respective
# code has been changed.  Or, compile everything if a header
# file has changed.
# *****************************************************************************
${SWIG_SOURCE}: ${INCLUDES} ${GENERATED_INCLUDES} ${SWIG_INPUT}
	@echo Swigging $@
	${VERBOSE}${SWIG} ${SWIGFLAGS} ${DEFINES} ${PERLDEFINES} -o ${SRCPATH}/${SWIG_SOURCE} -outdir ${OUTPERL} ecmdClientPerlapi.i

${SOURCE_OBJS}: ${OBJPATH}%.o : %.C ${INCLUDES} ${GENERATED_INCLUDES}
	@echo Compiling $<
	${VERBOSE}${CXX_R} -c ${CXXFLAGS} ${PERLCXXFLAGS} ${PERLINC} $< -o $@ ${DEFINES} ${PERLDEFINES}

${SWIG_OBJS}: ${OBJPATH}%.o : %.C ${SWIG_SOURCE}
	@echo Compiling $<
	${VERBOSE}${CXX_R} -c ${CXXFLAGS} ${PERLCXXFLAGS} ${PERLINC} $< -o $@ ${DEFINES} ${PERLDEFINES}

# *****************************************************************************
# Create the Perl Shared Library
# *****************************************************************************
${SO_TARGET}: ${SOURCE_OBJS} ${SWIG_OBJS} ${LINK_OBJS}
	@echo Linking $@
	${VERBOSE}${LD_R} ${SLDFLAGS} ${PERLSLDFLAGS} $^ -o ${OUTPERL}/${SO_TARGET} ${LDLIBS} ${EXTLIBS}

# *****************************************************************************
# Autogenerate the Perl Wrapper
# *****************************************************************************
# These have to be two separate targets 
# The generate include has to be there to build to generated source
${GENERATED_INCLUDES}: ${INCLUDES} ${ECMD_ROOT}/mkscripts/makepm.pl 
	@echo "Generating $@"
	@${ECMD_ROOT}/mkscripts/makepm.pl ecmd $@

${GENERATED_SOURCE}: ${GENERATED_INCLUDES} ${ECMD_ROOT}/mkscripts/makepm.pl 
	@echo "Generating $@"
	@${ECMD_ROOT}/mkscripts/makepm.pl ecmd $@

# *****************************************************************************
# Autogenerate some of the extension swig
# *****************************************************************************
${GENERATED_SWIG_INPUT}:
	@echo "Generating $@"
	@${ECMD_ROOT}/mkscripts/makeext.py perlapi

# *****************************************************************************
# Include any global default rules
# *****************************************************************************
include ${ECMD_ROOT}/makefile.rules
