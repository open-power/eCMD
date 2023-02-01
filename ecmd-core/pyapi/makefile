# Makefile for the ecmd Python Module

# *****************************************************************************
# Include the common base makefile
# *****************************************************************************
include ../../makefile.base

### Includes
INCLUDES := ecmdClientCapi.H ecmdClientPyapi.H ecmdDataBufferBase.H ecmdDataBuffer.H 
INCLUDED := ${INCLUDES} ecmdReturnCodes.H ecmdStructs.H

### Source
SOURCE := ecmdClientCapi.C ecmdClientPyapi.C

### Generated source by swig
# This variable can be overriden by the py3api makefile, hence the ?= assignment
SWIG_SOURCE ?= ecmdClientPyapiSwig.C

### Generated swig input files by makeext.pm
GENERATED_SWIG_INPUT := ecmdExtPyIncludes.i ecmdExtPyInserts.i

### SWIG input files
SWIG_INPUT := ecmdClientPyapi.i ecmdCommon.i ecmdExtPyIncludes.i ecmdExtPyInserts.i ecmdConst.i

# Setup all the options to pass into swig
SWIGFLAGS := ${SWIGFLAGS} -c++ -python -O -keyword -I${ECMD_CORE}/capi -I${ECMD_CORE}/dll -I${SRCPATH} -I../pyapi -I../pyapi/distro -DECMD_PYAPI

#### Setup our CXXFLAGS and VPATH
CXXFLAGS := ${CXXFLAGS} -I${ECMD_CORE}/capi -I${ECMD_CORE}/dll -I${ECMD_CORE}/cmd -I${SRCPATH}
VPATH    := ${VPATH}:${ECMD_CORE}/capi:${SRCPATH}:${OBJPATH}

### Add all of the extension information onto our variables
INCLUDES  := ${INCLUDES} $(foreach ext, ${EXT_PYAPI}, ${ext}ClientCapi.H ${ext}ClientPyapi.H)
SOURCE    := ${SOURCE} $(foreach ext, ${EXT_PYAPI}, ${ext}ClientCapi.C ${ext}ClientPyapi.C)
CXXFLAGS  := ${CXXFLAGS} $(foreach ext, ${EXT_PYAPI},-I${EXT_${ext}_PATH}/capi -I${EXT_${ext}_PATH}/pyapi)
SWIGFLAGS := ${SWIGFLAGS} $(foreach ext, ${EXT_PYAPI},-I${EXT_${ext}_PATH}/capi -I${EXT_${ext}_PATH}/pyapi)
VPATH     := ${VPATH}$(foreach ext, ${EXT_PYAPI},:${EXT_${ext}_PATH}/capi:${EXT_${ext}_PATH}/pyapi)

### Setup all of the additional objects to link
EXTLIBS   := ${EXTLIBS} ${foreach ext,${EXT_PYAPI},$(shell if [ -e ${OUTLIB}/lib${ext}.so ]; then echo -l${ext}; fi)}

# Set our defaults for these values, but they can be overriden by the py3api makefile
PYTHON_NAME ?= python2
VERINSTALL ?= pyapi_install

# The reference to libecmd.so has to go at the end of the link in Ubuntu
# Make it it's own variable instead of a SLDFLAGS
LDLIBS    := ${LDLIBS} -L${OUTLIB} -lecmd

# *****************************************************************************
# Define our output targets
# *****************************************************************************
PY_TARGET := ecmd.py
SO_TARGET := _ecmd.so

# *****************************************************************************
# The Main Targets
# *****************************************************************************
# The run-all rule is defined in makefile.rules
all:
	${run-all}

generate: ${SWIG_SOURCE}

build: ${SO_TARGET}

install: ${VERINSTALL}

pyapi_install:
	@echo "Creating ${TARGET_ARCH}/python/ecmd/${PYTHON_NAME} dir ..."
	@mkdir -p ${INSTALL_PATH}/${TARGET_ARCH}/python/ecmd/${PYTHON_NAME}
	@echo " "

	@echo "Installing eCMD Python API ${INSTALL_PATH}/${TARGET_ARCH}/python/ecmd/${PYTHON_NAME}.."
	cp ${OUTPY}/${PY_TARGET} ${INSTALL_PATH}/${TARGET_ARCH}/python/ecmd/${PYTHON_NAME}/__init__.py
	cp ${OUTPY}/${SO_TARGET} ${INSTALL_PATH}/${TARGET_ARCH}/python/ecmd/${PYTHON_NAME}/.
	cp ${ECMD_CORE}/pyapi/init/__init__.py ${INSTALL_PATH}/${TARGET_ARCH}/python/ecmd/.
	@echo " "

test:
	@echo "***** If you see python load errors this build of the python module is invalid ****"
	${VERBOSE}ECMD_DLL_FILE=${OUTLIB}/stub.dll PYTHONPATH=${OUTPY} LD_LIBRARY_PATH=${OUTLIB}:${OUTPY} ${ECMDPYTHONBIN} ../pyapi/testBuild.py

doxygen-pyapi:
	@cp ${ECMD_CORE}/pyapi/ecmdPyApiTypes.H ${DOXYGEN_PYAPI_PATH}/.
	@cat ${ECMD_CORE}/pyapi/ecmdClientPyapi.H ${ECMD_CORE}/capi/ecmdClientCapi.H > ${DOXYGEN_PYAPI_PATH}/ecmdClientPyapi.H

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
${SOURCE_OBJS} ${SWIG_OBJS}: ${OBJPATH}%.o : %.C ${INCLUDES}
	@echo Compiling $<
	${VERBOSE}${CXX_R} -c ${CXXFLAGS} ${PYCXXFLAGS} ${PYINC} $< -o $@ ${DEFINES} ${PYDEFINES}

${SWIG_SOURCE}: ${INCLUDES} ${SWIG_INPUT}
	@echo Determining SWIG dependencies of $@
	${VERBOSE}${SWIG} ${SWIGFLAGS} ${DEFINES} ${PYDEFINES} -M -o ${SRCPATH}/${SWIG_SOURCE} -outdir ${OUTPY} ../pyapi/ecmdClientPyapi.i
	@echo Generating mapping of output parameters
	${VERBOSE}${ECMDPYTHONBIN} ${ECMD_CORE}/pyapi/gen_apply.py $$(grep "\.H" $(basename ${SRCPATH}/${SWIG_SOURCE}).d | sed 's/\\//;' ) > ${SRCPATH}/ecmdApplyOutArgs.i
	@echo Swigging $@
	${VERBOSE}${SWIG} ${SWIGFLAGS} ${DEFINES} ${PYDEFINES} -DAPPLY_OUTARGS -o ${SRCPATH}/${SWIG_SOURCE} -outdir ${OUTPY} ../pyapi/ecmdClientPyapi.i

# *****************************************************************************
# Create the Python Shared Library
# *****************************************************************************
${SO_TARGET}: ${SOURCE_OBJS} ${SWIG_OBJS} ${LINK_OBJS}
	@echo Linking $@
	${VERBOSE}${LD_R} ${SLDFLAGS} ${PYSLDFLAGS} $^ -o ${OUTPY}/${SO_TARGET} ${LDLIBS} ${EXTLIBS}

# *****************************************************************************
# Autogenerate some of the extension swig
# *****************************************************************************
${GENERATED_SWIG_INPUT}:
	@echo Generating $@
	@${ECMD_ROOT}/mkscripts/makeext.py pyapi

# *****************************************************************************
# Include any global default rules
# *****************************************************************************
include ${ECMD_ROOT}/makefile.rules
