#!/usr/bin/env python3

import os
import sys
import errno

# A script to create the files generated by the configured extensions
# Configured extensions come from the EXTENSIONS environment variable

# Create the ecmdExtInterpreter.C
# This is a combo of generated code from this script and code snippets
# stored with the extension

# -----------------------
# cmd files
# -----------------------
if (sys.argv[1] == "cmd"):
  # Pull EXT_CMD out of the environment
  extlist = os.environ["EXT_CMD"].split()

  # Open our file and start writing
  extfile = open(os.environ["SRCPATH"] + "/ecmdExtInterpreter.C", 'w')

  # Write out all the static stuff
  extfile.write("//This file was autogenerated by makeext.py\n\n")
  extfile.write("#include <inttypes.h>\n")
  extfile.write("#include <dlfcn.h>\n")
  extfile.write("#include <stdio.h>\n")
  extfile.write("#include <string.h>\n")
  extfile.write("\n")
  extfile.write("#include <ecmdClientCapi.H>\n")
  extfile.write("#include <ecmdExtInterpreter.H>\n")
  extfile.write("#include <ecmdReturnCodes.H>\n")
  extfile.write("#include <ecmdCommandUtils.H>\n")
  extfile.write("#include <ecmdSharedUtils.H>\n\n")

  # Now loop through all the extensions and write out their defines and includes
  for ext in extlist:
    extfile.write("#ifdef ECMD_" + ext.upper() + "_EXTENSION_SUPPORT\n")
    extfile.write("#include <" + ext + "ClientCapi.H>\n")
    extfile.write("#include <" + ext + "Interpreter.H>\n")
    extfile.write("#endif\n\n")

  # Write the function definition
  extfile.write("uint32_t ecmdCallExtInterpreters(int argc, char* argv[], uint32_t & io_rc) {\n")
  extfile.write("  uint32_t rc = ECMD_SUCCESS;\n\n")

  # Now go through and suck in all the extension init code snippets
  # Once read in, place it in the new output file
  for ext in extlist:
    snippetfile = open(os.environ["EXT_" + ext + "_PATH"] + "/cmd/snippet/callInterpreter.C", 'r')
    for line in snippetfile.readlines():
      extfile.write(line)
    extfile.write("\n")
    snippetfile.close()	

  # Write the end of the function and close the file
  extfile.write("  return rc;\n")
  extfile.write("}\n")
  extfile.close()

# -----------------------
# Doxygen files
# -----------------------
if (sys.argv[1] == "doxygen"):

  # arg1 is doxygen
  # arg2 is the api
  # arg3 is where to write the file

  # Pull the right EXT list out of the env based on arg2
  extlist = list()
  if (sys.argv[2] == "capi"):
    extlist = os.environ["EXT_CAPI"].split()
  elif (sys.argv[2] == "perlapi"):
    extlist = os.environ["EXT_PERLAPI"].split()
  elif (sys.argv[2] == "pyapi"):
    extlist = os.environ["EXT_PYAPI"].split()
  else:
    print("Unknown API \"%s\" passed in!  Exiting.." % (sys.argv[2]))
    exit(1)

  # We're going to write out a header file to be used by doxygen to get the extensions

  # Open our file and start writing
  extfile = open(sys.argv[3] + "/ecmdExt" + sys.argv[2].title() + ".H", 'w')

  # Write out all the static stuff
  extfile.write("/**\n\n")
  extfile.write(" @file ecmdExt" + sys.argv[2].title() + ".H\n")
  extfile.write(" @brief eCMD Extension Information\n\n")
  extfile.write(" @section ext eCMD Extensions\n")
  extfile.write(" These are extensions to the core eCMD interface, not all eCMD Plugins support these extensions.<br>\n")
  extfile.write(" To use an eCMD extension you will need to link in the appropriate library, see the example Makefile on main page.<br>\n\n")
  extfile.write("<ul>\n")

  # Now loop through all the extensions and write out their includes
  for ext in extlist:
    extfile.write("<li> " + ext + "Client" + sys.argv[2].title() + ".H\n")

  # Write the end of the file
  extfile.write("</ul>\n\n")
  extfile.write("*/")
  extfile.close()

# -----------------------
# Python files
# -----------------------
if (sys.argv[1] == "pyapi"):
  
  # Pull EXT_PYAPI out of the environment
  extlist = os.environ["EXT_PYAPI"].split()

  # Open our file and start writing
  extfile = open(os.environ["SRCPATH"] + "/ecmdExtPyIncludes.i", 'w')

  extfile.write("/*********** Start Files to swigify ***********/\n")
  extfile.write("// The extensions\n")
  
  # Now auto generate the rest based on the list
  for ext in extlist:
    extfile.write("#ifdef ECMD_" + ext.upper() + "_EXTENSION_SUPPORT\n")
    extfile.write("  %include " + ext + "ClientPyapi.i\n")
    extfile.write("#endif\n")

  extfile.write("/*********** End Files to swigify ***********/\n")
  extfile.close()

  # Open our file and start writing
  extfile = open(os.environ["SRCPATH"] + "/ecmdExtPyInserts.i", 'w')

  extfile.write("/*********** Start Insert Code ***********/\n")
  extfile.write("// Insert C code into the file swig generates\n")
  extfile.write("%{\n")
  
  # Now auto generate the rest based on the list
  for ext in extlist:
    snippetfile = open(os.environ["EXT_" + ext + "_PATH"] + "/pyapi/snippet/extInsert.i", 'r')
    for line in snippetfile.readlines():
      extfile.write(line)
    snippetfile.close()	

  extfile.write("\%}\n")
  extfile.close()


# -----------------------
# Perl files
# -----------------------
if (sys.argv[1] == "perlapi"):
  
  # Pull EXT_PERLAPI out of the environment
  extlist = os.environ["EXT_PERLAPI"].split()

  # Open our file and start writing
  extfile = open(os.environ["SRCPATH"] + "/ecmdExtPerlIncludes.i", 'w')

  extfile.write("/*********** Start Files to swigify ***********/\n")
  extfile.write("// The extensions\n")
  
  # Now auto generate the rest based on the list
  for ext in extlist:
    extfile.write("#ifdef ECMD_" + ext.upper() + "_EXTENSION_SUPPORT\n")
    extfile.write("  %include " + ext + "ClientPerlapi.i\n")
    extfile.write("#endif\n")

  extfile.write("/*********** End Files to swigify ***********/\n")
  extfile.close()

  # Open our file and start writing
  extfile = open(os.environ["SRCPATH"] + "/ecmdExtPerlInserts.i", 'w')

  extfile.write("/*********** Start Insert Code ***********/\n")
  extfile.write("// Insert C code into the file swig generates\n")
  extfile.write("%{\n")
  
  # Now auto generate the rest based on the list
  for ext in extlist:
    snippetfile = open(os.environ["EXT_" + ext + "_PATH"] + "/perlapi/snippet/extInsert.i", 'r')
    for line in snippetfile.readlines():
      extfile.write(line)
    snippetfile.close()	

  extfile.write("\%}\n")
  extfile.close()
