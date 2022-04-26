#!/usr/bin/env python3

# This script will look for any ecmd-* dirs at the top level and
# setup a number of variables used through out the make
# Those variables are then written out to a makefile.config
# makefile.config is included by makefile.base

# Python module imports
import os
import sys
import glob
import platform
# The linux_distribution function was removed from platform in Python 3.8+
# It's now available in a different module - distro
# Try to use it from platform if still available there
# If not, see if the user has distro installed and get it from there
# We abstract the function name into get_distro to keep the usage code common
if "linux_distribution" in dir(platform):
    get_distro = platform.linux_distribution
else:
    # Not in platform, try to get this from the distro module
    try:
        import distro
    except:
        # If not installed, try to provide a helpful message
        print("ERROR: Unable to import distro module to get OS info!")
        print("ERROR: Please install the module and try again!")
        sys.exit(1)
    # It works, assign it to our common function name
    get_distro = distro.linux_distribution
import textwrap
import re
import argparse
import subprocess

#######################################
# Create the cmdline objects and args #
#######################################

# Add into -h text to describe variable determination via this script
# 1) Script command line args
# 2) From environment variables
# 3) Automatic determination if possible

parser = argparse.ArgumentParser(add_help=False, formatter_class=argparse.RawTextHelpFormatter,
                                 description=textwrap.dedent('''\
                                 This script creates all the variables necessary to build eCMD

                                 It determines the proper values in 3 ways:
                                 1) Command line options to this script
                                 2) Environment variables defined when script is invoked
                                 3) Looking in default locations (i.e. /usr/bin/g++)

                                 For most users building using the default packages of their distro,
                                 no options should be required.
                                 '''), 
                                 epilog=textwrap.dedent('''\
                                 Examples:
                                   ./config.py
                                   ./config.py --swig /usr/local/swig/bin/swig
                                   EXTENSIONS="cmd cip" ./config.py
                                 ''')
)

# Group for required args so the help displays properly
reqgroup = parser.add_argument_group('Required Arguments')
# No required command line args!

# Group for the optional args so the help displays properly
optgroup = parser.add_argument_group('Optional Arguments')

# These args can also be set by declaring their environment variable
# before calling this script.
# If you specify both, the cmdline arg wins over the env variable
# --help
optgroup.add_argument("-h", "--help", help="Show this message and exit", action="help")

# --install-path
#optgroup.add_argument("--install-path", help="Path to install to")
optgroup.add_argument("--install-path", help="Path to install to\n"
                                              "INSTALL_PATH from the environment")

# --host
optgroup.add_argument("--host", help="The host architecture\n"
                                     "HOST_ARCH from the environment")

# --target
optgroup.add_argument("--target", help="The target architecture\n"
                                       "TARGET_ARCH from the environment")

# --cxx
optgroup.add_argument("--cxx", help="The compiler to use\n"
                                    "CXX from the environment")

# --cxx_r
optgroup.add_argument("--cxx_r", help="The reentrant compiler to use (AIX only)\n"
                                      "CXX_R from the environment")

# --cxxflags
optgroup.add_argument("--cxxflags", help="Any CXXFLAGS to use in the build\n"
                                         "CXXFLAGS from the environment")

# --firstinc
optgroup.add_argument("--firstinc", help="Set the first include path (-I) to be defined\n"
                                         "in CXXFLAGS and SWIGFLAGS")

# --header-defines
optgroup.add_argument("--header-defines", action='store_true',
                      help="Rely on ecmdDefines.H to set extension defines")

# --ld
optgroup.add_argument("--ld", help="The linker to use\n"
                                   "LD from the environment")

# --ld_r
optgroup.add_argument("--ld_r", help="The reentrant linker to use (AIX only)\n"
                                     "LD_R from the environment")

# --ar
optgroup.add_argument("--ar", help="The archive creator to use\n"
                                   "AR from the environment")

# --sysroot
optgroup.add_argument("--sysroot", help="The system root to us.  Default is /", default='/')

# --swig
optgroup.add_argument("--swig", help="The swig executable to use\n"
                                     "SWIG from the environment")

# --perl
optgroup.add_argument("--perl", help="The perl executable to use\n"
                                     "ECMDPERLBIN from the environment")

# --perlinc
optgroup.add_argument("--perlinc", help="The perl include path to use\n"
                                        "PERLINC from the environment")

# --python
optgroup.add_argument("--python", help="The python executable to use\n"
                                       "ECMDPYTHONBIN from the environment")

# --pythoninc
optgroup.add_argument("--pythoninc", help="The python include path to use\n"
                                          "PYINC from the environment")

# --python3
optgroup.add_argument("--python3", help="The python3 executable to use\n"
                                        "ECMDPYTHON3BIN from the environment")

# --python3inc
optgroup.add_argument("--python3inc", help="The python3 include path to use\n"
                                           "PY3INC from the environment")

# --catchinc
optgroup.add_argument("--catchinc", help="The catch include path to use for testing\n"
                                         "CATCHINC from the environment")

# --doxygen
optgroup.add_argument("--doxygen", help="The doxygen executable to use\n"
                                        "DOXYGENBIN from the environment")

# --output-root
optgroup.add_argument("--output-root", help="The location to place build output\n"
                                            "OUTPUT_ROOT from the environment")

# --extensions
optgroup.add_argument("--extensions", help="Filter down the list of extensions to build\n"
                                           "EXTENSIONS from the environment")
# --ecmd-repos
optgroup.add_argument("--ecmd-repos", help="Other ecmd extension/plugin repos to include in build\n")

# --remove-sim
optgroup.add_argument("--remove-sim", action='store_true', help="Enable REMOVE_SIM in build")

# --without-swig
optgroup.add_argument("--without-swig", action='store_true', help="Disable all swig actions")

# --without-perl
optgroup.add_argument("--without-perl", action='store_true', help="Disable perl module build\n"
                                                                  "CREATE_PERLAPI from the environment")

# --without-python
optgroup.add_argument("--without-python", action='store_true', help="Disable python module build\n"
                                                                    "CREATE_PYAPI from the environment")

# --without-python3
optgroup.add_argument("--without-python3", action='store_true', help="Disable python3 module build\n"
                                                                     "CREATE_PY3API from the environment")

# --without-pyecmd
optgroup.add_argument("--without-pyecmd", action='store_true', help="Disable pyecmd module build\n"
                                                                    "CREATE_PYECMD from the environment")

# --build-disable-test
optgroup.add_argument("--build-disable-test", action='store_true', help="Disable any build tests.  Useful for cross compiling")

# --build-verbose
optgroup.add_argument("--build-verbose", action='store_true', help="Enable verbose messaging during builds.\n"
                                                                   "Displays compiler calls, etc..\n"
                                                                   "VERBOSE from the environment")

# Parse the cmdline for the args we just added
args = parser.parse_args()

# Store any variables we wish to write to the makefiles here
buildvars = dict()

############################################################
# Determine all the build variables required to build eCMD #
############################################################

print("Establishing eCMD build variables..")

# First, determine our ECMD_ROOT variable
# ECMD_ROOT is the top level directory of the ecmd source repo
# ECMD_ROOT is used to derive a number of variable throughout this script
ECMD_ROOT = os.path.dirname(os.path.realpath(__file__))
buildvars["ECMD_ROOT"] = ECMD_ROOT

# The main ecmd repo contains the ecmd-core subdir
# This is where the main eCMD source code is stored
# ECMD_CORE will point to this location
ECMD_CORE = os.path.join(ECMD_ROOT, "ecmd-core")
buildvars["ECMD_CORE"] = ECMD_CORE

# In addition to ecmd-core, a user could add any number of ecmd-* repos under main
# Look for those other repos and create ECMD_REPOS that stores all of them
# We store the results in a form that make can loop over using foreach
ECMD_REPOS = ""
for repo in glob.glob(ECMD_ROOT + "/ecmd-*"):
    ECMD_REPOS += repo + " "
ECMD_REPOS = ECMD_REPOS[:-1] # Pull off trailing space

# Add any cmdline repos given into what was found above
if (args.ecmd_repos is not None):
    ECMD_REPOS += " " + args.ecmd_repos
buildvars["ECMD_REPOS"] = ECMD_REPOS

# Each of the sub repos could contain a utils directory
# We want the top level make to be able to build any utils out there
# Loop through each of the ECMD_REPOS above and see if they have a utils dir
# We create ECMD_REPOS_UTILS that has all the repos that have a util dir
ECMD_REPOS_UTILS = ""
for repo in ECMD_REPOS.split(" "):
    # Create the utils dir path
    testpath = os.path.join(repo, "utils")
    # See if this repo has a utils dir, if it does add it to the list
    if (os.path.exists(testpath)):
        ECMD_REPOS_UTILS += repo + " "
ECMD_REPOS_UTILS = ECMD_REPOS_UTILS[:-1] # Pull off trailing space
buildvars["ECMD_REPOS_UTILS"] = ECMD_REPOS_UTILS

# The plugins logic is similar to the utils logic, with one complication
# The things we are interested in are actually contained in each plugin directory present
# So we will create ECMD_REPOS_PLUGINS to show who has a plugins dir
# We will also create ECMD_PLUGINS, that has the name of each plugin found in the plugins dir
# Since ECMD_PLUGINS, won't be tied to the repo is comes from, 
# We also create a number of PATH variables that can be used to access the plugin path
# By looping over ECMD_PLUGINS, we can dynamically access PLUGINS_plugin_PATH
ECMD_REPOS_PLUGINS = ""
ECMD_PLUGINS = ""
for repo in ECMD_REPOS.split(" "):
    # Create the plugins dir path
    testpath = os.path.join(repo, "plugins")
    # If it exists, setup a number of variables from that dir
    if (os.path.exists(testpath)):
        ECMD_REPOS_PLUGINS += repo + " "
        # Look to see what plugins exist in the directory
        for plugin in glob.glob(os.path.join(testpath, "*")):
            baseplugin = os.path.basename(plugin)
            ECMD_PLUGINS += baseplugin + " "
            buildvars["PLUGIN_" + baseplugin + "_PATH"] = os.path.join(testpath, baseplugin)
ECMD_REPOS_PLUGINS = ECMD_REPOS_PLUGINS[:-1] # Pull off trailing space
buildvars["ECMD_REPOS_PLUGINS"] = ECMD_REPOS_PLUGINS
ECMD_PLUGINS = ECMD_PLUGINS[:-1] # Pull off trailing space
# Take our random order string and create a sorted string
buildvars["ECMD_PLUGINS"] = ' ' . join(sorted(ECMD_PLUGINS.split(" ")))

# The extensions logic is also similar to utils/plugins - but with yet another level of complication
# The user can set EXTENSIONS before calling this script, and it's what the script will use
# If the user doesn't set it, loop through all the repos and look for what extenions exist
# See if the user specified it via the script cmdline
# If not, pull it from the env or set the default
EXTENSIONS = ""
if (args.extensions is not None):
    EXTENSIONS = args.extensions
elif ("EXTENSIONS" in os.environ):
    EXTENSIONS = os.environ["EXTENSIONS"]
else:
    # Not given, so dynamically create the list based on what's in the ECMD_REPOS
    for repo in ECMD_REPOS.split(" "):
        for ext in glob.glob(os.path.join(repo, "ext/*")):
            EXTENSIONS += os.path.basename(ext) + " "
    EXTENSIONS = EXTENSIONS[:-1] # Pull off trailing space

# Remove "template" from the extensions list
EXTENSIONS = EXTENSIONS.replace("template", "")
# Now cleanup any multiple, leading or trailing spaces
# Those throw off the make foreach looping
EXTENSIONS = re.sub("\s+", " ", EXTENSIONS) # Multiple spaces into 1
EXTENSIONS = EXTENSIONS.strip() # Remove leading/trailing whitespace
# Take our random order string and create a sorted string
buildvars["EXTENSIONS"] = ' ' . join(sorted(EXTENSIONS.split(" ")))

# We now have our EXTENSIONS to use
# However, every extension doesn't support every build type
# For example, and extension may have a C-API, but not a cmdline
# Loop through the extensions and create a variable for each build type
# We also create an EXT_ext_PATH variable that points to the topdir of the extension
EXT_CAPI = ""
EXT_CMD = ""
EXT_PERLAPI = ""
EXT_PYAPI = ""
# Loop through the extensions
for ext in sorted(EXTENSIONS.split()):
    # Loop through the repos to find where the extension lives
    for repo in ECMD_REPOS.split(" "):
        testpath = os.path.join(repo, "ext", ext)
        # If we found it, setup a number of things
        if (os.path.exists(testpath)):
            buildvars["EXT_" + ext + "_PATH"] = testpath

            # Now that we have a valid ext dir, test it for each build type
            testpath = os.path.join(repo, "ext", ext, "capi")
            if (os.path.exists(testpath)):
                EXT_CAPI += ext + " "

            testpath = os.path.join(repo, "ext", ext, "cmd")
            if (os.path.exists(testpath)):
                EXT_CMD += ext + " "

            testpath = os.path.join(repo, "ext", ext, "perlapi")
            if (os.path.exists(testpath)):
                EXT_PERLAPI += ext + " "

            testpath = os.path.join(repo, "ext", ext, "pyapi")
            if (os.path.exists(testpath)):
                EXT_PYAPI += ext + " "
EXT_CAPI = EXT_CAPI[:-1] # Pull off trailing space
buildvars["EXT_CAPI"] = EXT_CAPI
EXT_CMD = EXT_CMD[:-1] # Pull off trailing space
buildvars["EXT_CMD"] = EXT_CMD
EXT_PERLAPI = EXT_PERLAPI[:-1] # Pull off trailing space
buildvars["EXT_PERLAPI"] = EXT_PERLAPI
EXT_PYAPI = EXT_PYAPI[:-1] # Pull off trailing space
buildvars["EXT_PYAPI"] = EXT_PYAPI

# The extension template is contained in ecmd-core, just set that path
EXT_TEMPLATE_PATH = os.path.join(ECMD_CORE, "ext", "template")
buildvars["EXT_TEMPLATE_PATH"] = EXT_TEMPLATE_PATH

###################################################################
# We've deterimined a bunch of stuff about what to build for eCMD #
# Now let's setup up all the info about our build environment     #
###################################################################

print("Determining host and distro..")

# Determine the HOST_ARCH
HOST_ARCH = ""
if (args.host is not None):
    HOST_ARCH = args.host
elif ("HOST_ARCH" in os.environ):
    HOST_ARCH = os.environ["HOST_ARCH"]
else:
    if (platform.system() == "AIX"):
        atuple = platform.architecture()
        if (atuple[0] == "32bit"):
            HOST_ARCH="aix"
        else:
            HOST_ARCH="aix64"
    else:
        # Linux
        HOST_ARCH = platform.machine()
buildvars["HOST_ARCH"] = HOST_ARCH

# Set the host base arch.  Just happens to be the first 3 characters
HOST_BARCH = HOST_ARCH[0:3]
buildvars["HOST_BARCH"] = HOST_BARCH

# Determine the TARGET_ARCH
TARGET_ARCH = ""
if (args.target is not None):
    TARGET_ARCH = args.target
elif ("TARGET_ARCH" in os.environ):
    TARGET_ARCH = os.environ["TARGET_ARCH"]
else:
    # Not given, default to the HOST_ARCH
    TARGET_ARCH = HOST_ARCH
buildvars["TARGET_ARCH"] = TARGET_ARCH

# Set the target base arch.  Just happens to be the first 3 characters
TARGET_BARCH = TARGET_ARCH[0:3]
buildvars["TARGET_BARCH"] = TARGET_BARCH

# Determine the distro
DISTRO = ""
if (platform.system() == "Linux"):
    dtuple = get_distro()
    if ("Red Hat Enterprise" in dtuple[0]):
        DISTRO = "el" + dtuple[1][0]
    elif (dtuple[0] == "Ubuntu"):
        DISTRO = "ub" + dtuple[1]
    elif (dtuple[0] == "Fedora"):
        DISTRO = "fc" + dtuple[1]
    elif (dtuple[0] == "debian"):
        DISTRO = "deb" + dtuple[1][0]
else:
    DISTRO="aix"
buildvars["DISTRO"] = DISTRO

###################################
# Look for a distro override file #
###################################

# If found or given, it will be included at the end of the generated makefile
# That may undo or override anything established via this script
# We'll also use the presence of that file to disable some aspects of this script
# 1) We won't do anything swig related and assume that's in the override
# 2) We won't do any version checking of executables
# Now that we have the distro and arch, look for the disto overrride file
# If more than 1 override file is found in the included repos, throw an error
# The user can set DISTRO_OVERRIDE and then we don't do this search logic at all
DISTRO_OVERRIDE = ""
if "DISTRO_OVERRIDE" in os.environ:
    DISTRO_OVERRIDE = os.environ["DISTRO_OVERRIDE"]
else:
    # This loop includes ecmd-core, which should never have mkconfigs
    for repo in ECMD_REPOS.split(" "):
        mkfile = os.path.join(ECMD_ROOT, repo, "mkconfigs", DISTRO, "make-" + HOST_ARCH + "-" + TARGET_ARCH)
        if os.path.exists(mkfile):
            # This logic is triggered if multiple of the subrepos have the same override
            if (DISTRO_OVERRIDE != ""):
                print("ERROR: conflicting distro overrides found!")
                print("ERROR: %s" % DISTRO_OVERRIDE)
                print("ERROR: %S" % mkfile)
                print("Please set DISTRO_OVERRIDE to your choice and re-run this command")
                sys.exit(1)
            else:
                DISTRO_OVERRIDE = mkfile
    
    # If there wasn't one found in the sub repos, see if the top level has one
    if (DISTRO_OVERRIDE == ""):
        mkfile = os.path.join(ECMD_ROOT, "mkconfigs", DISTRO, "make-" + HOST_ARCH + "-" + TARGET_ARCH)
        if (os.path.exists(mkfile)):
            DISTRO_OVERRIDE = mkfile

# If we have a distro makefile, print out the file we found
# Also disable swig for the rest of the script
if (DISTRO_OVERRIDE != ""):
    print("Using DISTRO_OVERRIDE %s" % DISTRO_OVERRIDE)
    args.without_swig = True

################################################
# Set our output locations for build artifacts #
################################################

print("Establishing output locations..")

# If the OUTPUT_ROOT was passed in, use that for base directory for generated
# files. Otherwise use ECMD_ROOT.
# OUTPUT_ROOT establishes the top level of where all build artifacts will go
if (args.output_root is not None):
    OUTPUT_ROOT = args.output_root
elif ("OUTPUT_ROOT" in os.environ):
    OUTPUT_ROOT = os.environ["OUTPUT_ROOT"]
else:
    OUTPUT_ROOT = ECMD_ROOT
buildvars["OUTPUT_ROOT"] = OUTPUT_ROOT

# All objects from the build go to a common dir at the top level
# OBJPATH includes TARGET_ARCH to allow for side by side builds
# This does come with the stipulation that all source must have unique names
OBJPATH = os.path.join(OUTPUT_ROOT, "obj_" + TARGET_ARCH)
OBJPATH += "/" # Tack this on so the .C->.o rules run properly
buildvars["OBJPATH"] = OBJPATH

# All generated source from makedll.pl, swig, etc.. go into this directory
# Previous source was dumped in the local dir and it was hard to distinguish it from normal source
SRCPATH = os.path.join(OUTPUT_ROOT, "src_" + TARGET_ARCH)
buildvars["SRCPATH"] = SRCPATH

# Setup the output path info for the created binaries and libraries
# We have one top level output path where all output binaries go
# This could be shared libs, archives or executables
# OUTPATH includes the TARGET_ARCH to allow for side by side builds
OUTPATH = os.path.join(OUTPUT_ROOT, "out_" + TARGET_ARCH)
buildvars["OUTPATH"] = OUTPATH
buildvars["OUTBIN"] = os.path.join(OUTPATH, "bin")
buildvars["OUTLIB"] = os.path.join(OUTPATH, "lib")
buildvars["OUTPERL"] = os.path.join(OUTPATH, "perl")
buildvars["OUTPY"] = os.path.join(OUTPATH, "pyapi")
buildvars["OUTPY3"] = os.path.join(OUTPATH, "py3api")
buildvars["OUTPYECMD"] = os.path.join(OUTPATH, "pyecmd")

##################################################
# Default things we need setup for every compile #
##################################################
# CXX = the compiler
# CXX_R = the reentrant compiler, only different for AIX
# CXXFLAGS = flags to pass to the compiler
# LD = the linker
# LD_R = the reentrant linker, only different for AIX
# LDFLAGS = flags to pass to the linker when linking exe's
# SLDFLAGS = flags to pass to the linker when linking shared libs
# AR = the archive creator
# DEFINES = -D defines to pass thru

print("Establishing compiler locations..")

# Compiler - CXX
CXX = ""
if (args.cxx is not None):
    CXX = args.cxx
elif ("CXX" in os.environ):
    CXX = os.environ["CXX"]
else:
    CXX = "/usr/bin/g++"
buildvars["CXX"] = CXX

# Compiler Reentrant - CXX_R
CXX_R = ""
if (args.cxx_r is not None):
    CXX_R = args.cxx_r
elif ("CXX_R" in os.environ):
    CXX_R = os.environ["CXX_R"]
else:
    # If not specified, use best guess for AIX
    # Everyone else, use the CXX value
    if (TARGET_BARCH == "aix"):
        CXX_R = "/usr/bin/g++"
    else:
        CXX_R = CXX
buildvars["CXX_R"] = CXX_R

# Linker - LD
LD = ""
if (args.ld is not None):
    LD = args.ld
elif ("LD" in os.environ):
    LD = os.environ["LD"]
else:
    LD = "/usr/bin/g++"
buildvars["LD"] = LD

# Linker Reentrant - LD_R
LD_R = ""
if (args.ld_r is not None):
    LD_R = args.ld_r
elif ("LD_R" in os.environ):
    LD_R = os.environ["LD_R"]
else:
    # If not specified, use best guess for AIX
    # Everyone else, use the LD value
    if (TARGET_BARCH == "aix"):
        LD_R = "/usr/bin/g++"
    else:
        LD_R = LD
buildvars["LD_R"] = LD_R

# Archive - AR
AR = ""
if (args.ar is not None):
    AR = args.ar
elif ("AR" in os.environ):
    AR = os.environ["AR"]
else:
    AR = "/usr/bin/ar"
buildvars["AR"] = AR

print("Establishing compiler options..")

# Setup the variable defaults
DEFINES = ""
GPATH = ""
CXXFLAGS = ""
LDFLAGS = ""
SLDFLAGS = ""

# C++ compiler flags - CXXFLAGS
if (args.cxxflags is not None):
    CXXFLAGS = args.cxxflags
elif ("CXXFLAGS" in os.environ):
    CXXFLAGS = os.environ["CXXFLAGS"]

# Take the ones from the environment or command line,
# then add them to the ones we'll define here

# Common compile flags across any OS
CXXFLAGS += " -g -I."

# If the option was given, make sure the given path is
# at the front of the list
SWIGFLAGS = None
if (args.firstinc):
    firstinc = " -I%s" % args.firstinc
    CXXFLAGS += firstinc
    SWIGFLAGS = firstinc

# If the user passed thru extra defines, grab them
if "DEFINES" in os.environ:
    DEFINES = os.environ["DEFINES"]
# linker flags - LDFLAGS, SLDFLAGS
if ("LDFLAGS" in os.environ):
    LDFLAGS = os.environ["LDFLAGS"]
if ("SLDFLAGS" in os.environ):
    SLDFLAGS = os.environ["SLDFLAGS"]

# Setup common variables across distros
if (TARGET_BARCH == "x86" or TARGET_BARCH == "ppc"):
    GPATH += " " + OBJPATH
    CXXFLAGS += " -Wall"
    if (TARGET_ARCH.find("64") != -1):
        CXXFLAGS += " -m64 -fPIC"
        LDFLAGS += " -m64 -fPIC"
        SLDFLAGS += " -shared -m64 -fPIC"
    else:
        CXXFLAGS += " -m32 -fPIC"
        LDFLAGS += " -m32 -fPIC"
        SLDFLAGS += " -shared -m32 -fPIC"
elif (TARGET_BARCH == "arm"):
    GPATH += " " + OBJPATH
    CXXFLAGS += " -Wall -fPIC"
    LDFLAGS += " -fPIC"
    SLDFLAGS += " -shared -fPIC"

# See if REMOVE_SIM is enabled from the cmdline
if (args.remove_sim):
    DEFINES += " -DREMOVE_SIM"

# Added the extension defines if not using the header
if (not args.header_defines):
    for ext in sorted(EXTENSIONS.split()):
        DEFINES += " -DECMD_" + ext.upper() + "_EXTENSION_SUPPORT"
    
# Export everything we defined
buildvars["DEFINES"] = DEFINES
buildvars["GPATH"] = GPATH
buildvars["CXXFLAGS"] = CXXFLAGS
buildvars["LDFLAGS"] = LDFLAGS
buildvars["SLDFLAGS"] = SLDFLAGS
if (SWIGFLAGS):
    buildvars["SWIGFLAGS"] = SWIGFLAGS

###################################
# Setup for creating SWIG outputs #
###################################

# Check for optional args to disable building of perl and python module
if (args.without_swig or args.without_perl):
    CREATE_PERLAPI = "no"
elif ("CREATE_PERLAPI" in os.environ):
    CREATE_PERLAPI = os.environ["CREATE_PERLAPI"]
else:
    CREATE_PERLAPI = "yes"
buildvars["CREATE_PERLAPI"] = CREATE_PERLAPI

if (args.without_swig or args.without_python):
    CREATE_PYAPI = "no"
elif ("CREATE_PYAPI" in os.environ):
    CREATE_PYAPI = os.environ["CREATE_PYAPI"]
else:
    CREATE_PYAPI = "yes"
buildvars["CREATE_PYAPI"] = CREATE_PYAPI

if (args.without_swig or args.without_python3):
    CREATE_PY3API = "no"
elif ("CREATE_PY3API" in os.environ):
    CREATE_PY3API = os.environ["CREATE_PY3API"]
else:
    CREATE_PY3API = "yes"
buildvars["CREATE_PY3API"] = CREATE_PY3API

if (args.without_swig or (args.without_python and args.without_python3) or args.without_pyecmd):
    CREATE_PYECMD = "no"
elif ("CREATE_PYECMD" in os.environ):
    CREATE_PYECMD = os.environ["CREATE_PYECMD"]
else:
    CREATE_PYECMD = "yes"
buildvars["CREATE_PYECMD"] = CREATE_PYECMD

# The swig executable to use
if (not args.without_swig):
    SWIG = ""
    if (args.swig is not None):
        SWIG = args.swig
    elif ("SWIG" in os.environ):
        SWIG = os.environ["SWIG"]
    else:
        SWIG = "/usr/bin/swig"
    buildvars["SWIG"] = SWIG

# Location of the perl binary
if (CREATE_PERLAPI == "yes"):
    ECMDPERLBIN = ""
    if (args.perl is not None):
        ECMDPERLBIN = args.perl
    elif ("ECMDPERLBIN" in os.environ):
        ECMDPERLBIN = os.environ["ECMDPERLBIN"]
    else:
        ECMDPERLBIN = "/usr/bin/perl"
    buildvars["ECMDPERLBIN"] = ECMDPERLBIN

# Location of the python binary
if (CREATE_PYAPI == "yes"):
    ECMDPYTHONBIN = ""
    if (args.python is not None):
        ECMDPYTHONBIN = args.python
    elif ("ECMDPYTHONBIN" in os.environ):
        ECMDPYTHONBIN = os.environ["ECMDPYTHONBIN"]
    else:
        ECMDPYTHONBIN = "/usr/bin/python2"
    buildvars["ECMDPYTHONBIN"] = ECMDPYTHONBIN

# Location of the python3 binary
if (CREATE_PY3API == "yes"):
    ECMDPYTHON3BIN = ""
    if (args.python3 is not None):
        ECMDPYTHON3BIN = args.python3
    elif ("ECMDPYTHON3BIN" in os.environ):
        ECMDPYTHON3BIN = os.environ["ECMDPYTHON3BIN"]
    else:
        ECMDPYTHON3BIN = "/usr/bin/python3"
    buildvars["ECMDPYTHON3BIN"] = ECMDPYTHON3BIN

# perl include path
PERLINC = None
if (CREATE_PERLAPI == "yes"):
    PERLINC = ""
    if (args.perlinc is not None):
        PERLINC = args.perlinc
    elif ("PERLINC" in os.environ):
        PERLINC = os.environ["PERLINC"]

# python include path
PYINC = None
if (CREATE_PYAPI == "yes"):
    PYINC = ""
    if (args.pythoninc is not None):
        PYINC = args.pythoninc
    elif ("PYINC" in os.environ):
        PYINC = os.environ["PYINC"]

# python3
PY3INC = None
if (CREATE_PY3API == "yes"):
    PY3INC = ""
    if (args.python3inc is not None):
        PY3INC = args.python3inc
    elif ("PY3INC" in os.environ):
        PY3INC = os.environ["PY3INC"]

# Do the search for the includes path
if ((PERLINC == "") or (PYINC == "") or (PY3INC == "")):
    print("Finding swig include files..")
    for root, dirs, files in os.walk(os.path.join(args.sysroot, 'usr'), topdown=True):
        # exit if we found everything before the dir walk is over
        if PERLINC and PYINC and PY3INC:
            break
        for file in files:
            # Same include for python 2/3, so figure that out after finding the file
            if (file == "Python.h"):
                if ((PYINC == "") and ("python2" in root)):
                    PYINC = "-I" + root
                if ((PY3INC == "") and ("python3" in root)):
                    PY3INC = "-I" + root
            # CORE/EXTERN.h is unique to perl
            if (file == "EXTERN.h"):
                if ((PERLINC == "") and ("CORE" in root)):
                    PERLINC = "-I" + root

# If set, make sure it's valid then save it
if (PERLINC is not None):
    if (PERLINC == ""):
        print("ERROR: Unable to determine path to perl includes")
        print("ERROR: Please install the perl includes, specify the path via --perlinc or disable perl module build")
        sys.exit(1)
    buildvars["PERLINC"] = PERLINC
if (PYINC is not None):
    if (PYINC == ""):
        print("ERROR: Unable to determine path to python includes")
        print("ERROR: Please install the python includes, specify the path via --pythoninc or disable python module build")
        sys.exit(1)
    buildvars["PYINC"] = PYINC
if (PY3INC is not None):
    if (PY3INC == ""):
        print("ERROR: Unable to determine path to python3 includes")
        print("ERROR: Please install the python3 includes, specify the path via --python3inc or disable python3 module build")
        sys.exit(1)
    buildvars["PY3INC"] = PY3INC

#################################
# Misc. variables for the build #
#################################

# Where to get the doxygen binary
DOXYGENBIN = ""
if (args.doxygen is not None):
    DOXYGENBIN = args.doxygen
elif ("DOXYGENBIN" in os.environ):
    DOXYGENBIN = os.environ["DOXYGENBIN"]
else:
    DOXYGENBIN = "/usr/bin/doxygen"
buildvars["DOXYGENBIN"] = DOXYGENBIN

# Enable build test as long as we aren't cross compiling
if (HOST_ARCH == TARGET_ARCH):
    TEST_BUILD = "yes"
else:
    TEST_BUILD = "no"

# Enabling build test doesn't matter if you user disabled it from the command line
if (args.build_disable_test):
    TEST_BUILD = "no"
buildvars["TEST_BUILD"] = TEST_BUILD

# Enable Catch-based testcases if TEST_BUILD enabled and catch include path is set
CATCH_TEST_BUILD = "no"
CATCHINC = None
if (TEST_BUILD == "yes"):
    if (args.catchinc is not None):
        CATCHINC = args.catchinc
    elif ("CATCHINC" in os.environ):
        CATCHINC = os.environ["CATCHINC"]
if (CATCHINC is not None):
    if (CATCHINC == ""):
        print("ERROR: Unable to determine path to Catch includes")
        print("ERROR: Please install the catch includes, specify the path via --catchinc")
        sys.exit(1)
    buildvars["CATCHINC"] = CATCHINC
    CATCH_TEST_BUILD = "yes"
buildvars["CATCH_TEST_BUILD"] = CATCH_TEST_BUILD

# Enable verbose build option
# By default, we want it quiet which is @
VERBOSE = "@"
if (args.build_verbose):
    VERBOSE = ""
elif ("VERBOSE" in os.environ):
    VERBOSE = os.environ["VERBOSE"]
buildvars["VERBOSE"] = VERBOSE

#######################################
# Setup info around doing the install #
#######################################

# See if the user specified it via the script cmdline
# If not, pull it from the env or set the default
if (args.install_path is not None):
    INSTALL_PATH = args.install_path
elif ("INSTALL_PATH" in os.environ):
    INSTALL_PATH = os.environ["INSTALL_PATH"]
else:    
    # If INSTALL_PATH wasn't given, install into our local dir
    INSTALL_PATH = os.path.join(ECMD_ROOT, "install")
buildvars["INSTALL_PATH"] = INSTALL_PATH

# If not given, create INSTALL_BIN_PATH off INSTALL_PATH
# INSTALL_BIN_PATH is in ecmdaliases.ksh/ecmdaliases.csh
# This can be different from INSTALL_PATH to enable a network install
# You could set it a shell variable that is evaluated at runtime
# Useful in common environments that may be shadowed around sites
# Setting it properly is tricky in that case.
# If you want $COMMONPATH to end up in the aliases files,
# INSTALL_BIN_PATH needs to be \\\\$$"COMMONPATH" in makefile.config
# That accounts for \'s and $'s that get ate by make during the install rule
# If you want to call config.py to set the value in the env, do it like this:
# INSTALL_BIN_PATH="\\\\\\\\\$\$\"COMMONPATH\"" ./config.py
# That escapes all the right things so you end up with \\\\$$ in makefile.config
if ("INSTALL_BIN_PATH" in os.environ):
    INSTALL_BIN_PATH = os.environ["INSTALL_BIN_PATH"]
else:
    INSTALL_BIN_PATH = os.path.join(INSTALL_PATH, "bin")
buildvars["INSTALL_BIN_PATH"] = INSTALL_BIN_PATH

# If DOXYGEN_PATH wasn't given, use OUTPATH
if ("DOXYGEN_PATH" in os.environ):
    DOXYGEN_PATH = os.environ["DOXYGEN_PATH"]
else:
    DOXYGEN_PATH = os.path.join(OUTPATH, "doxygen")

# Setup our capi/perl/python paths based on DOXYGEN_PATH
DOXYGEN_CAPI_PATH = os.path.join(DOXYGEN_PATH, "Capi")
DOXYGEN_PERLAPI_PATH = os.path.join(DOXYGEN_PATH, "Perlapi")
DOXYGEN_PYAPI_PATH = os.path.join(DOXYGEN_PATH, "Pythonapi")
buildvars["DOXYGEN_PATH"] = DOXYGEN_PATH
buildvars["DOXYGEN_CAPI_PATH"] = DOXYGEN_CAPI_PATH
buildvars["DOXYGEN_PERLAPI_PATH"] = DOXYGEN_PERLAPI_PATH
buildvars["DOXYGEN_PYAPI_PATH"] = DOXYGEN_PYAPI_PATH

# Pull the version out of ecmdStructs.H if not given
DOXYGEN_ECMD_VERSION = ""
if "DOXYGEN_ECMD_VERSION" not in os.environ:
    verfile = open(os.path.join(ECMD_CORE, "capi", "ecmdStructs.H"), "r")
    for line in verfile:
        linesplit = line.split()
        if ((len(linesplit) > 1) and (linesplit[1] == "ECMD_CAPI_VERSION")):
            DOXYGEN_ECMD_VERSION = line.split()[2] # "M.m"
            DOXYGEN_ECMD_VERSION = DOXYGEN_ECMD_VERSION[1:] # M.m"
            DOXYGEN_ECMD_VERSION = DOXYGEN_ECMD_VERSION[0:-1] # M.m
            break
    verfile.close()
else:
    DOXYGEN_ECMD_VERSION = os.environ["DOXYGEN_ECMD_VERSION"]
buildvars["DOXYGEN_ECMD_VERSION"] = DOXYGEN_ECMD_VERSION

print("eCMD %s found in ecmdStructs.H" % DOXYGEN_ECMD_VERSION)

###########################################################
# Test certain components of the build for version, etc.. #
###########################################################

print("Testing program versions..")

# The minimum required version of SWIG for eCMD to build
# properly is version 2.0.11.  Check that version.
if (not args.without_swig):
    # Ensure the file we are going to check exists
    if (not os.path.isfile(SWIG)):
        print("ERROR: The swig executable \"%s\" doesn't exist!" % SWIG)
        print("ERROR: Please run again after resolving the issue")
        sys.exit(1)
    
    # It does exist, check it
    cmdout = subprocess.Popen([SWIG, "-version"],
                          stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE,
                          stdin=subprocess.PIPE).communicate()[0]
    cmdsplit = cmdout.decode('utf-8').split('\n')

    for line in cmdsplit:
        if ("SWIG Version" in line):
            # Return value is "SWIG Version 3.1.8"
            # Swig versioning is 3 numbers and doesn't usually go above a 10's value
            # To pad it, we'll use 100's
            # So we'll turn 3.1.8 into 003001008 (well, drop the leading zeros so we don't go octal)
            # The version we want to check is 2.0.11.  If our converted number is less than 2000011, it's not valid
            version = line.split()[2]
            versplit = version.split('.')
            verNoFloat = (int(versplit[0]) * 1000000) + (int(versplit[1]) * 1000) + int(versplit[2])
            if (verNoFloat < 2000011):
                print("ERROR: Your swig version %s is less than the minimum version of 2.0.11" % version)
                print("ERROR: Please run again and specify a different swig (--swig) or disable swig (--without-swig)")
                sys.exit(1)

##################################################
# Write out all our variables to makefile.config #
##################################################

# Get the makefile.config to use, otherwise use the default
if ("MAKEFILE_CONFIG_NAME" in os.environ):
    MAKEFILE_CONFIG_NAME = os.environ["MAKEFILE_CONFIG_NAME"]
else:
    MAKEFILE_CONFIG_NAME = "makefile.config"

# Now go thru everything that has been setup and write it out to the file
print("Writing %s" % os.path.join(ECMD_ROOT, MAKEFILE_CONFIG_NAME))
config = open(os.path.join(ECMD_ROOT, MAKEFILE_CONFIG_NAME), 'w')
config.write("\n")
config.write("# These variables are generated by config.py\n")
config.write("#   " + ' '.join(sys.argv) + "\n")
config.write("\n")

# Write out all the variables
for var in sorted(buildvars):
    config.write("%s := %s\n" % (var, buildvars[var]))
config.write("\n")

# Export them so they can be referenced by any scripts used in the build
for var in sorted(buildvars):
    config.write("export %s\n" % var)
config.write("\n")

# If the DISTRO_OVERRIDE was given, write it out to the file
if (DISTRO_OVERRIDE != ""):
    config.write("# Include the distro specific overrides found/specified\n")
    config.write("# It may undo/change any values established above\n")
    config.write("include %s\n" % DISTRO_OVERRIDE)
    config.write("\n")


# Write our optional extension makefile.config includes
# This allows you to add values to variables defined above
config.write("# Optionally include any extension specific makefile.config overrides\n")
for ext in sorted(EXTENSIONS.split()):
    config.write("-include %s\n" % os.path.join(buildvars["EXT_" + ext + "_PATH"], "makefile.config")) 

config.close()
