# The main makefile for all rules

# *****************************************************************************
# Include the common base makefile
# *****************************************************************************
include makefile.base

# *****************************************************************************
# Some basic setup before we start trying to build stuff
# *****************************************************************************

# For an extension specified, we build a complete set of rules for it
EXT_CAPI_RULES    := $(foreach ext, ${EXT_CAPI}, ${ext}capi)
EXT_CMD_RULES     := $(foreach ext, ${EXT_CMD}, ${ext}cmd)
EXT_PERLAPI_RULES := $(foreach ext, ${EXT_PERLAPI}, ${ext}perlapi)
EXT_PYAPI_RULES   := $(foreach ext, ${EXT_PYAPI}, ${ext}pyapi)

# Build our targets
EXT_CAPI_TARGETS    := $(foreach ext, ${EXT_CAPI}, ${ext}capi)
EXT_CMD_TARGETS     := $(foreach ext, ${EXT_CMD}, ${ext}cmd)
EXT_PERLAPI_TARGETS := $(foreach ext, ${EXT_PERLAPI}, ${ext}perlapi)
EXT_PYAPI_TARGETS   := $(foreach ext, ${EXT_PYAPI}, ${ext}pyapi)

# The cmd extension has to be built after ecmdcmd, so if it's in the extension list pull it out and add after ecmdcmd
ifneq (,$(findstring cmdcapi,${EXT_CAPI_TARGETS}))
  CMD_EXT_BUILD := cmdcapi
endif
# Pull cmd then use the list to build ext targets
EXT_CAPI_TARGETS := $(subst cmdcapi,,${EXT_CAPI_TARGETS})

# Set the actual build targets used in the build if yes
ifeq (${CREATE_PERLAPI},yes)
  PERLAPI_BUILD := ecmdperlapi
endif
ifeq (${CREATE_PYAPI},yes)
  PYAPI_BUILD := ecmdpyapi
  PYAPI_PREREQ := ${PYAPI_BUILD}
endif
ifeq (${CREATE_PY3API},yes)
  PY3API_BUILD := ecmdpy3api
endif
ifeq (${CREATE_PYAPI},no)
  ifeq (${CREATE_PY3API},yes)
    PYAPI_PREREQ := ${PY3API_BUILD}
  endif
endif
ifeq (${CREATE_PYECMD},yes)
  PYECMD_BUILD := pyecmd
endif

# Now create our build targets
BUILD_TARGETS := ecmdcapi ecmdcmd ${CMD_EXT_BUILD} dllstub ${PERLAPI_BUILD} ${PYAPI_BUILD} ${PY3API_BUILD} ${PYECMD_BUILD}

# *****************************************************************************
# The Main Targets
# *****************************************************************************

# The default rule is going to do a number of things:
# 1) Create the output directories
# 2) Generate source
# 3) Build source
# 4) Test the build

# The default
all:
	${run-all}

generate: ${BUILD_TARGETS}

build: ${BUILD_TARGETS}

test: ${BUILD_TARGETS}

doxygen: doxygen-create
	${VERBOSE}cp -r ${DOXYGEN_PATH} ${INSTALL_PATH}

doxygen-create: dir
	@mkdir -p ${DOXYGEN_CAPI_PATH}
	${VERBOSE}${MAKE} doxygen-capi ${MAKEFLAGS} --no-print-directory
	@mkdir -p ${DOXYGEN_PERLAPI_PATH}
	${VERBOSE}${MAKE} doxygen-perlapi ${MAKEFLAGS} --no-print-directory
	@mkdir -p ${DOXYGEN_PYAPI_PATH}
	${VERBOSE}${MAKE} doxygen-pyapi ${MAKEFLAGS} --no-print-directory

doxygen-capi: ecmdcapi ${EXT_CAPI_TARGETS}
	@echo "Creating C API doxygen ..."
	${VERBOSE}${ECMD_ROOT}/mkscripts/makeext.py doxygen capi ${DOXYGEN_CAPI_PATH}
	${VERBOSE}mkdir -p ${DOXYGEN_CAPI_PATH}/examples
	${VERBOSE}cp ${ECMD_ROOT}/docs/examples/ecmdclient.C ${DOXYGEN_CAPI_PATH}/examples/.
	${VERBOSE}cp ${ECMD_ROOT}/docs/examples/makefile ${DOXYGEN_CAPI_PATH}/examples/.
	${VERBOSE}$(shell sed "s!DOXYGEN_ECMD_VERSION!${DOXYGEN_ECMD_VERSION}!g" ${ECMD_ROOT}/docs/ecmdDoxygen.config | sed "s!INPUTOUTPUT_PATH!${DOXYGEN_CAPI_PATH}!g" > ${DOXYGEN_CAPI_PATH}/ecmdDoxygen.config)
	${VERBOSE}$(shell sed -i "s!ECMDEXTDEFINES!$(foreach ext,${EXT_CAPI},ECMD_$(shell echo ${ext} | tr "a-zA-Z" "A-Za-z")_EXTENSION_SUPPORT)!g" ${DOXYGEN_CAPI_PATH}/ecmdDoxygen.config)
	${VERBOSE}cd ${DOXYGEN_CAPI_PATH}; ${DOXYGENBIN} ecmdDoxygen.config > /dev/null
#	${VERBOSE}cd ${DOXYGEN_CAPI_PATH}/latex; make; mv refman.pdf ecmdClientCapi.pdf

doxygen-perlapi: ecmdcapi ${EXT_PERLAPI_TARGETS} ecmdperlapi
	@echo "Creating Perl API doxygen ..."
	${VERBOSE}${ECMD_ROOT}/mkscripts/makeext.py doxygen perlapi ${DOXYGEN_PERLAPI_PATH}
	${VERBOSE}mkdir -p ${DOXYGEN_PERLAPI_PATH}/examples
	${VERBOSE}cp ${ECMD_ROOT}/docs/examples/example.pl ${DOXYGEN_PERLAPI_PATH}/examples/.
	${VERBOSE}$(shell sed "s!DOXYGEN_ECMD_VERSION!${DOXYGEN_ECMD_VERSION}!g" ${ECMD_ROOT}/docs/ecmdDoxygenPm.config | sed "s!INPUTOUTPUT_PATH!${DOXYGEN_PERLAPI_PATH}!g" > ${DOXYGEN_PERLAPI_PATH}/ecmdDoxygenPm.config)
	${VERBOSE}$(shell sed -i "s!ECMDEXTDEFINES!$(foreach ext,${EXT_PERLAPI},ECMD_$(shell echo ${ext} | tr "a-zA-Z" "A-Za-z")_EXTENSION_SUPPORT)!g" ${DOXYGEN_PERLAPI_PATH}/ecmdDoxygenPm.config)
	${VERBOSE}${ECMD_ROOT}/docs/perlizeDocs.pl
	${VERBOSE}cd ${DOXYGEN_PERLAPI_PATH}; ${DOXYGENBIN} ecmdDoxygenPm.config > /dev/null
#	${VERBOSE}cd ${DOXYGEN_PERLAPI_PATH}/latex; make; mv refman.pdf ecmdClientPerlapi.pdf

doxygen-pyapi: ecmdcapi ${EXT_PYAPI_TARGETS} ${PYAPI_PREREQ}
	@echo "Creating Python API doxygen ..."
	${VERBOSE}${ECMD_ROOT}/mkscripts/makeext.py doxygen pyapi ${DOXYGEN_PYAPI_PATH}
	${VERBOSE}mkdir -p ${DOXYGEN_PYAPI_PATH}/examples
	${VERBOSE}cp ${ECMD_ROOT}/docs/examples/example.py ${DOXYGEN_PYAPI_PATH}/examples/.
	${VERBOSE}$(shell sed "s!DOXYGEN_ECMD_VERSION!${DOXYGEN_ECMD_VERSION}!g" ${ECMD_ROOT}/docs/ecmdDoxygenPython.config | sed "s!INPUTOUTPUT_PATH!${DOXYGEN_PYAPI_PATH}!g" > ${DOXYGEN_PYAPI_PATH}/ecmdDoxygenPython.config)
	${VERBOSE}$(shell sed -i "s!ECMDEXTDEFINES!$(foreach ext,${EXT_PYAPI},ECMD_$(shell echo ${ext} | tr "a-zA-Z" "A-Za-z")_EXTENSION_SUPPORT)!g" ${DOXYGEN_PYAPI_PATH}/ecmdDoxygenPython.config)
	${VERBOSE}${ECMD_ROOT}/docs/pythonizeDocs.pl
	${VERBOSE}cd ${DOXYGEN_PYAPI_PATH}; ${DOXYGENBIN} ecmdDoxygenPython.config > /dev/null
#	${VERBOSE}cd ${DOXYGEN_PYAPI_PATH}/latex; make; mv refman.pdf ecmdClientPythonapi.pdf

# The core eCMD pieces
ecmdcapi:
	@echo "eCMD Core Client C-API ${TARGET_ARCH} ..."
	@${MAKE} -C ${ECMD_CORE}/capi ${MAKECMDGOALS} ${MAKEFLAGS}
	@echo " "

ecmdcmd: ecmdcapi ${EXT_CAPI_TARGETS} ${EXT_CMD_TARGETS}
	@echo "eCMD Core Command line Client ${TARGET_ARCH} ..."
	@${MAKE} -C ${ECMD_CORE}/cmd ${MAKECMDGOALS} ${MAKEFLAGS}
	@echo " "

ecmdperlapi: ecmdcmd ${EXT_PERLAPI_TARGETS}
	@echo "eCMD Perl Module ${TARGET_ARCH} ..."
	@${MAKE} -C ${ECMD_CORE}/perlapi ${MAKECMDGOALS} ${MAKEFLAGS}
	@echo " "

ecmdpyapi: ecmdcmd ${EXT_PYAPI_TARGETS}
	@echo "eCMD Python Module ${TARGET_ARCH} ..."
	@${MAKE} -C ${ECMD_CORE}/pyapi ${MAKECMDGOALS} ${MAKEFLAGS}
	@echo " "

ecmdpy3api: ecmdcmd ${EXT_PYAPI_TARGETS}
	@echo "eCMD Python3 Module ${TARGET_ARCH} ..."
	@${MAKE} -C ${ECMD_CORE}/py3api ${MAKECMDGOALS} ${MAKEFLAGS}
	@echo " "

pyecmd: ${PYAPI_PREREQ}
	@echo "pyeCMD Module ${TARGET_ARCH} ..."
	@${MAKE} -C ${ECMD_CORE}/pyecmd ${MAKECMDGOALS} ${MAKEFLAGS}
	@echo " "

########################
# Build EXTENSIONS
########################
${EXT_CAPI_RULES}: ecmdcapi
	@echo "$(subst capi,,$@) Extension C API ${TARGET_ARCH} ..."
	@${MAKE} -C ${EXT_$(subst capi,,$@)_PATH}/capi ${MAKECMDGOALS} ${MAKEFLAGS}
	@echo " "

${EXT_CMD_RULES}: ecmdcapi
	@echo "$(subst cmd,,$@) Extension cmdline ${TARGET_ARCH} ..."
	@${MAKE} -C ${EXT_$(subst cmd,,$@)_PATH}/cmd ${MAKECMDGOALS} ${MAKEFLAGS}
	@echo " "

${EXT_PERLAPI_RULES}:
	@echo "$(subst perlapi,,$@) Extension Perl API ${TARGET_ARCH} ..."
	@${MAKE} -C ${EXT_$(subst perlapi,,$@)_PATH}/perlapi ${MAKECMDGOALS} ${MAKEFLAGS}
	@echo " "

${EXT_PYAPI_RULES}:
	@echo "$(subst pyapi,,$@) Extension Python API ${TARGET_ARCH} ..."
	@${MAKE} -C ${EXT_$(subst pyapi,,$@)_PATH}/pyapi ${MAKECMDGOALS} ${MAKEFLAGS}
	@echo " "

########################
# Utils
########################
ecmdutils:
	@echo "eCMD Utilities ${TARGET_ARCH} ..."
	@${MAKE} -C ${ECMD_CORE}/utils ${MAKECMDGOALS} ${MAKEFLAGS}
	@echo " "

########################
# DLL Stub
########################
dllstub:
	@echo "eCMD DLL Stub ${TARGET_ARCH} ..."
	@${MAKE} -C ${ECMD_ROOT}/dllStub ${MAKECMDGOALS} ${MAKEFLAGS}
	@echo " "

# Runs the install routines for all targets
install: install_setup ${BUILD_TARGETS} install_finish

# Copy over the help files, etc.. before installing the executables and libraries
install_setup:

        # Create the install path
	@mkdir -p ${INSTALL_PATH}

	@echo "Creating bin dir ..."
	@mkdir -p ${INSTALL_PATH}/bin
	@echo " "

	@echo "Creating ${TARGET_ARCH}/bin dir ..."
	@mkdir -p ${INSTALL_PATH}/${TARGET_ARCH}/bin
	@echo " "

	@echo "Creating include dir ..."
	@mkdir -p ${INSTALL_PATH}/include
	@echo " "

	@echo "Creating ${TARGET_ARCH}/lib dir ..."
	@mkdir -p ${INSTALL_PATH}/${TARGET_ARCH}/lib
	@echo " "

	@echo "Creating ${TARGET_ARCH}/perl dir ..."
	@mkdir -p ${INSTALL_PATH}/${TARGET_ARCH}/perl
	@echo " "

	@echo "Copying cmdline wrappers to bin"
	@cp -R `find ${ECMD_CORE}/bin/*` ${INSTALL_PATH}/bin/.
	@$(foreach ext, ${EXT_CMD}, echo "  copying ${ext}"; cp -an `find ${EXT_${ext}_PATH}/bin/*` ${INSTALL_PATH}/bin/.;) 
	@echo " "

	@echo "Setting up ecmdaliases files ..."
	@sed "s@INSTALL_BIN_PATH@${INSTALL_BIN_PATH}@g" ${ECMD_CORE}/bin/ecmdaliases.ksh > ${INSTALL_PATH}/bin/ecmdaliases.ksh
	@sed "s@INSTALL_BIN_PATH@${INSTALL_BIN_PATH}@g" ${ECMD_CORE}/bin/ecmdaliases.csh > ${INSTALL_PATH}/bin/ecmdaliases.csh
	@echo " "

	@echo "Creating help dir ..."
	@mkdir -p ${INSTALL_PATH}/help
	@cp -R `find ${ECMD_CORE}/cmd/help/*` ${INSTALL_PATH}/help/.
	@$(foreach ext, ${EXTENSIONS}, if [ -d ${EXT_${ext}_PATH}/cmd/help ]; then find ${EXT_${ext}_PATH}/cmd/help -type f -exec cp {} ${INSTALL_PATH}/help/. \; ; fi;)
	@echo " "

	@echo "Copying over setup perl modules ..."
	@mkdir -p ${INSTALL_PATH}/bin/plugins
	@$(foreach plugin, ${ECMD_PLUGINS}, cp ${PLUGIN_${plugin}_PATH}/*setup.pm ${INSTALL_PATH}/bin/plugins/.;)
	@echo " "

# Do final cleanup things such as fixing permissions
install_finish: install_setup ${BUILD_TARGETS}

# Build the utils and install them
	@echo "Building utils ..."
	@$(foreach util, ${ECMD_REPOS_UTILS}, ${MAKE} -C ${util}/utils/ ${MAKEFLAGS};)
	@echo " "

	@echo "Installing utils ..."
	@$(foreach util, ${ECMD_REPOS_UTILS}, ${MAKE} -C ${util}/utils/ ${MAKECMDGOALS} ${MAKEFLAGS};)
	@echo " "

	@echo "Fixing include dir file permissions ..."
	@chmod 644 ${INSTALL_PATH}/include/*

	@echo "Fixing help dir file permissions ..."
	@chmod 644 ${INSTALL_PATH}/help/*

	@echo "Fixing bin dir file permissions ..."
	@chmod 775 ${INSTALL_PATH}/bin/*

	@echo "Fixing ${TARGET_ARCH}/bin dir file permissions ..."
	@chmod 775 ${INSTALL_PATH}/${TARGET_ARCH}/bin/*

	@echo ""
	@echo "*** Install Done! ***"
	@echo ""

# *****************************************************************************
# Include any global default rules
# *****************************************************************************
include ${ECMD_ROOT}/makefile.rules
