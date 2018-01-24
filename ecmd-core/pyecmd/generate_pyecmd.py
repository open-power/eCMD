#!/usr/bin/python
#IBM_PROLOG_BEGIN_TAG
#
# Copyright 2015,2018 IBM International Business Machines Corp.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#IBM_PROLOG_END_TAG

"""
Generate most of the pyecmd code by parsing function headers out of the eCMD Python API Doxygen HTML files.

@author Joachim Fenkes <fenkes@de.ibm.com>

Instructions:
 1. ./generate_pyecmd.py <all the HTML files you want to transform> > generated.py
 2. If some function is not as you'd expect, the lists at the beginning of this script
    allow some customization.
"""

from __future__ import print_function
import re, sys, os

"""
Override the with of fixed input data functions from the default of 64
"""
special_input_widths = {
"putCfamRegister" : 32,
}

"""
Functions that return an uint32_t that is _not_ a return code and should be returned to the caller
"""
force_return_value_names = (
"flattenSize",
"simFusionRand32",
"hashString32"
)

"""
Functions to not export to Python
"""
non_exported_names = (
# manually wrapped by pyecmd
"loadDll",
"commandArgs",
"unloadDll",
"looperInit",
"existLooperNext",
"existLooperInit",
"looperNext",
"configLooperNext",
"configLooperInit",
"relatedTargets",
# aren't being converted correctly - FIXME
"parseTokens",
"queryHostMemInfoRanges",
# not necessary because there are Pythonic replacements
"queryErrorState",
"hexToUInt32",
"genB32FromHex",
"genB32FromHexLeft",
"genB32FromHexRight",
"decToUInt32",
)

"""
Functions with multiple overloaded definitions. Most of these can be handled by using the most
generic variant with parameter defaults, so we substitute modified prototypes. This doesn't work
for some; those have to be omitted via non_exported_names and implemented manually in base.py.
"""
overloaded_functions = {
"writeTarget":        "const ecmdChipTarget &i_target, ecmdTargetDisplayMode_t i_displayMode=ECMD_DISPLAY_TARGET_PLUGIN_MODE)",
"displayScomData":    "ecmdChipTarget &i_target, ecmdScomData &i_scomData, ecmdDataBuffer &i_data, const char *i_format, std::string *o_strData=NULL)",
# Patch o_ringFile to be i_ringFile so we generate correct Python wrapper code
"croFastLoadL2":      "ecmdChipTarget &i_target, croFastL2LoadModes &i_modes, ecmdMemoryEntryList &i_data, const char *i_ringFile=NULL)",
}

FUNC_GLOBAL = 0
FUNC_TARGETED = 1

def python_type(type):
    if   type[-2:] == "_t":           return "int"
    elif type == "bool":              return "bool"
    elif type == "char" \
      or type == "std::string":       return "str"
    elif type == "std::stringVector": return "list(str)"
    elif type == "std::stringList":   return "list(str)"
    elif type == "ecmdDataBuffer":    return "EcmdBitArray"
    elif type[-4:] == "List":         return "list(ecmd." + type[0:-4] + ")"
    elif type[-6:] == "Vector":       return "list(ecmd." + type[0:-6] + ")"
    elif type == "ecmdChipTarget":    return "Target"
    else:                             return "ecmd." + type

class FunctionArgument(object):
    """
    Wrapper class for a single function argument, takes care of all type transformations
    """
    __slots__ = ("type", "name", "def_value")

    def __init__(self, argstr):
        parts = argstr.split()
        self.type = " ".join(parts[:-1])
        if "=" in parts[-1]:
            self.name, self.def_value = parts[-1].split("=")
        else:
            self.name = parts[-1]
            self.def_value = ""

    @property
    def name_with_def_value(self):
        result = self.name
        if self.def_value:
            result += "=" + ("ecmd." if re.match(r"[A-Z]+_", self.def_value) else "") + self.def_value
        return result

    @property
    def name_stripped(self):
        return re.sub(r"^(i|o|io)_", "", self.name)

    @property
    def is_input(self):
        return self.name[:2] == "i_"

    @property
    def is_output(self):
        return self.name[:2] == "o_"

    @property
    def is_inout(self):
        return self.name[:3] == "io_"

    @property
    def is_returned_out_arg(self):
        return self.type.endswith("_t") or self.type in ("std::string")

    @property
    def is_ecmd_list_type(self):
        return self.type[:11] != "std::string" and (self.type[-4:] == "List" or self.type[-6:] == "Vector")

    @property
    def retval_construction(self):
        if   self.type == "std::string":       return '""'
        elif self.type == "std::stringVector": return "[]"
        elif self.type == "std::stringList":   return "[]"
        elif self.type[-2:] == "_t":           return "0"
        elif self.type == "ecmdChipTarget":    return "Target()"
        #elif self.type == "ecmdDataBuffer":    return "DataBuffer()"
        else:                                 return "ecmd." + self.type + "()"

    @property
    def python_type(self):
        return python_type(self.type)

"""
Emit a wrapping Python function for an eCMD function, tailored towards the specific function
"""
def print_function(data, func_type):
    global cur_header

    (new_funcname, header, line1, comment, returntype, funcname, argstr) = data

    # indent target and buffer functions
    prefix = "" if func_type == FUNC_GLOBAL else "    "

    # if the header changes, print it
    if header != cur_header:
        cur_header = header
        print("")
        print(prefix + '# -------------------------------------------------------------------------')
        print(prefix + '# ' + header)
        print(prefix + '# -------------------------------------------------------------------------')

    # Pythonify and clean up the string of function arguments, then split into (type, name) tuples
    argstr = argstr.replace("NULL", "None").replace("true", "True").replace("false", "False").replace("void", "")
    argstr = re.sub(r"\).*$", "", argstr).strip(" \t\n()")

    args = argstr.split(",") if argstr else []
    args = [FunctionArgument(re.sub(r"(0)ULL$", r"\1", re.sub(r"[&*]|\[.*\]|ull$", "", arg.strip()))) for arg in args]

    # Remove the i_target parameter for target functions; we'll replace it by self
    if func_type == FUNC_TARGETED:
        if args[0].name != "i_target":
            raise ValueError("No i_target parameter in targeted function "+new_funcname)
        del args[0]

    calls_wrapped_method = returntype == "void"
    returns_wrapped_method = returntype not in ("uint32_t", "void") \
        or new_funcname in force_return_value_names

    # find output parameters and split them into those that get passed in by-ref and those that will be
    # returned by the SWIG function in a tuple.
    out_args_returned = []
    out_args_passed_in = []
    out_args = []
    if not returns_wrapped_method:
        for arg in args:
            if not arg.is_output:
                continue

            out_args.append(arg)
            if arg.is_returned_out_arg:
                out_args_returned.append(arg)
            else:
                out_args_passed_in.append(arg)

    # build list of parameter names for the Python function definition; add self and remove default values
    definition_args = [arg.name_with_def_value for arg in args if arg not in out_args]
    if func_type == FUNC_TARGETED:
        definition_args = ["self"] + definition_args

    # build list of parameter names for the SWIG function invocation
    invocation_args = ["self"] if func_type == FUNC_TARGETED else []
    for arg in args:
        if arg in out_args_returned:
            continue

        argname = arg.name
        if arg.is_output:
            pass # don't transform internally constructed by-ref output parameters
        elif arg.type == "ecmdDataBuffer":
            if new_funcname in special_input_widths:
                argname = argname + (", %d" % special_input_widths[new_funcname])
            argname = "base._to_ecmdDataBuffer(" + argname + ")"
        elif arg.name == "i_len" and arg.def_value == "None":
            argname = "i_len or self.getBitLength()"
        invocation_args.append(argname)

    # determine type of return values
    if returns_wrapped_method:
        returntypes = python_type(returntype)
    else:
        returntypes = ", ".join(arg.python_type for arg in out_args)
        if len(out_args) > 1:
            returntypes = "tuple(" + returntypes + ")"

    # construct return values
    return_values = []
    for arg in out_args:
        argname = arg.name
        if arg.type == "ecmdDataBuffer":
            argname = "base._from_ecmdDataBuffer(" + argname + ")"
        return_values.append(argname)

    # construct usage example, mainly for functions returning tuples
    example = "pyecmd." if func_type == FUNC_GLOBAL else "target." if func_type == FUNC_TARGETED else "buffer."
    example = example + new_funcname + "(" + ", ".join(arg.name_stripped for arg in args if arg not in out_args) + ")"
    if returns_wrapped_method:
        example = "result = " + example
    elif out_args:
        example = ", ".join(arg.name_stripped for arg in out_args) + " = " + example

    # miscellaneous
    has_retval = out_args or returns_wrapped_method

    # and now print the entire shebang
    print("")
    print(prefix + "def " + new_funcname + "(" + ", ".join(definition_args) + "):")
    print(prefix + '    """')
    print(prefix + "    " + comment)
    print(prefix + "    C++ signature: " + line1)
    print(prefix + "    Example: " + example)
    if has_retval:
        print(prefix + "    @rtype: " + returntypes)
    print(prefix + '    """')

    for arg in out_args_passed_in:
        print(prefix + "    %s = %s" % (arg.name, arg.retval_construction))

    invocation = "ecmd." + funcname + "(" + ", ".join(invocation_args) + ")"
    if calls_wrapped_method:
        print(prefix + "    " + invocation)
    elif returns_wrapped_method:
        print(prefix + "    return " + invocation)
    else:
        if out_args_returned:
            print(prefix + "    " + ", ".join(["rc"] + [arg.name for arg in out_args_returned]) + " = " + invocation)
            print(prefix + "    base._rcwrap(rc)")
        else:
            print(prefix + "    base._rcwrap(" + invocation + ")")
        if out_args:
            print(prefix + "    return " + ", ".join(return_values))

HTML_REPLACEMENTS = (
    ("&amp;", "&"), ("&lt;", "<"), ("&gt;", ">"), ("&#160;", " "), ("&quot;", '"'), ("&nbsp;", " ")
)

def unhtml(line):
    str = re.sub(r"<.*?>", "", line)
    for replacement in HTML_REPLACEMENTS:
        str = str.replace(*replacement)
    return str.strip()

function_re = re.compile(r"(\S+)\s+(\S+)\s*(\(.*$)")

##### Main program #####
if __name__ == "__main__":
    global_functions = []
    target_functions = []
    found_functions = set()

    # parse input files
    for fname in sys.argv:
        next_is_header = False
        with open(fname, "r") as f:
            cur_header = ""
            for line in f:
                line_unhtml = unhtml(line)
                if 'memtitle' in line:
                    break
                elif '<h2 class="groupheader"' in line:
                    next_is_header = True
                elif '<div class="groupHeader"' in line or next_is_header:
                    cur_header = line_unhtml
                    next_is_header = False
                elif '<td class="memItemLeft"' in line:
                    function = line_unhtml.strip()
                elif '<td class="mdescLeft"' in line:
                    comment = line_unhtml.replace("More...", "").strip()

                    # The regex will filter out things like enums, class definitions and constructors
                    m = function_re.match(function.replace("*", "").replace("("," (").replace("= ","="))
                    if not m:
                        continue
                    (returntype, funcname, argstr) = m.group(1, 2, 3)

                    # filter out #defines, destructors and operators
                    if returntype == "#define" or funcname[0] == "~" or funcname[0:8] == "operator":
                        continue

                    # transform function name
                    if funcname[0:4] == "ecmd":
                        new_funcname = funcname[4].lower() + funcname[5:]
                    else:
                        new_funcname = funcname

                    if new_funcname in non_exported_names:
                        continue

                    # export each function only once, warn if duplicate and not marked as "overloaded"
                    if new_funcname in found_functions:
                        if new_funcname not in overloaded_functions:
                            sys.stderr.write("WARNING: %s looks overloaded without special handling!\n" % new_funcname)
                        continue

                    # if overloaded, replace arguments
                    if new_funcname in overloaded_functions:
                        argstr = overloaded_functions[new_funcname]
                        function = returntype + " " + funcname + " (" + argstr

                    # add function to the right list
                    found_functions.add(new_funcname)
                    func_data = (new_funcname, cur_header, function, comment, returntype, funcname, argstr)
                    if "ecmdChipTarget i_target" in argstr.replace("&", ""):
                        target_functions.append(func_data)
                    else:
                        global_functions.append(func_data)

    # header
    print("""# IBM Confidential
# Licensed Materials - Property of IBM
# (c) Copyright IBM Corp. 2015, 2017
'''
Autogenerated from %s -- modify at your own risk!

This module takes the hand-written code from pyecmd_base and extends it
with autogenerated wrappers for most of the eCmd functions.

@author: Joachim Fenkes <fenkes@de.ibm.com>
'''

import ecmd
from . import base""" % ", ".join(os.path.basename(x) for x in sys.argv[1:]))

    # module scope functions
    cur_header = ""
    for data in global_functions:
        print_function(data, FUNC_GLOBAL)

    # target scope functions
    print("\n\nclass _Target(ecmd.ecmdChipTarget):")
    cur_header = ""
    for data in target_functions:
        print_function(data, FUNC_TARGETED)
