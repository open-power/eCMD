# Makefile for pyeCMD, the Pythonic eCMD wrapper

# *****************************************************************************
# Include the common base makefile
# *****************************************************************************
include ../../makefile.base

GENERATED_PY := ${OUTPYECMD}/generated.py
CONSTANTS_PY := ${OUTPYECMD}/constants.py
COPIED_PY := base.py __init__.py ecmdbitstring.py
HEADERS_TO_PARSE := $(addprefix ${ECMD_CORE}/capi/, ecmdClientCapi.H ecmdSharedUtils.H ecmdUtils.H)
PYTHON_INSTALL_PATH := ${INSTALL_PATH}/${TARGET_ARCH}/python
PYECMD_INSTALL_PATH := ${PYTHON_INSTALL_PATH}/pyecmd

# For extensions, include <ext>ClientCapi.H if and only if it's also referenced in <ext>ClientPyapi.i
# skip fapi2 api for now
EXTENSIONS_TO_PARSE := $(filter-out fapi2, $(EXT_PYAPI))
HEADERS_TO_PARSE += $(foreach ext, $(EXTENSIONS_TO_PARSE),$(if $(findstring $(ext)ClientCapi.H,$(shell cat $(EXT_$(ext)_PATH)/pyapi/$(ext)ClientPyapi.i)),$(EXT_$(ext)_PATH)/capi/$(ext)ClientCapi.H))

ifeq ($(CREATE_PYAPI),yes)
  OUTPYECMDPY := ${OUTPY}/ecmd.py
else ifeq ($(CREATE_PY3API),yes)
  OUTPYECMDPY := ${OUTPY3}/ecmd.py
endif

# *****************************************************************************
# The Main Targets
# *****************************************************************************
# The run-all rule is defined in makefile.rules
all:
	${run-all}

doxygen:
	${VERBOSE}${MAKE} -C ../.. doxygen-create ${MAKEFLAGS} --no-print-directory

generate: ${GENERATED_PY} ${CONSTANTS_PY}

build:

install:
	@echo "Creating ${PYECMD_INSTALL_PATH} dir ..."
	${VERBOSE}mkdir -p ${PYECMD_INSTALL_PATH}
	@echo " "

	@echo "Installing Pythonic eCMD Wrapper pyeCMD ${PYECMD_INSTALL_PATH} ..."
	${VERBOSE}cp ${GENERATED_PY} ${CONSTANTS_PY} ${COPIED_PY} ${PYECMD_INSTALL_PATH}
	${VERBOSE}cp bitstring.py ${PYTHON_INSTALL_PATH}
	@echo " "

test:
	@echo "***** If you see python load errors this build of the python module is invalid ****"
	${VERBOSE}ln -sf `pwd`/*.py ${OUTPYECMD}
ifeq ($(CREATE_PYAPI),yes)
	@echo "Testing on python2"
	${VERBOSE}ECMD_DLL_FILE=${OUTLIB}/stub.dll PYTHONPATH=${OUTPY}:${OUTPYECMD}/.. LD_LIBRARY_PATH=${OUTLIB}:${OUTPY} ${ECMDPYTHONBIN} test_api.py
endif
ifeq ($(CREATE_PY3API),yes)
	@echo "Testing on python3"
	${VERBOSE}ECMD_DLL_FILE=${OUTLIB}/stub.dll PYTHONPATH=${OUTPY3}:${OUTPYECMD}/.. LD_LIBRARY_PATH=${OUTLIB}:${OUTPY3} ${ECMDPYTHON3BIN} test_api.py
endif

test-interactive:
	${VERBOSE}ECMD_DLL_FILE=${OUTLIB}/stub.dll PYTHONPATH=${OUTPY3}:${OUTPYECMD}/.. LD_LIBRARY_PATH=${OUTLIB}:${OUTPY3} ${ECMDPYTHON3BIN}

# *****************************************************************************
# Generate most of the wrapper Python code
# *****************************************************************************
${GENERATED_PY}: ./generate_pyecmd.py $(HEADERS_TO_PARSE)
	@echo "Generating $@ ..."
	${VERBOSE}./generate_pyecmd.py $(HEADERS_TO_PARSE) > $@

${CONSTANTS_PY}: ${OUTPYECMDPY}
	@echo "Generating $@ ..."
	${VERBOSE}{ \
	    echo -n "from ecmd import "; \
	    grep -E "([[:upper:][:digit:]_]+)\s+=\s+_ecmd\.\1" $< | cut -d' ' -f1 | tr '\n' ',' | sed 's/,$$//'; \
	 } > $@

# *****************************************************************************
# Include any global default rules
# *****************************************************************************
include ${ECMD_ROOT}/makefile.rules
