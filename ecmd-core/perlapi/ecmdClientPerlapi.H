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

#ifndef _ecmdClientPerlapi_H
#define _ecmdClientPerlapi_H




//--------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------
#include <string>

#include <ecmdDefines.H>
#include <ecmdStructs.H>
#include <ecmdDataBuffer.H>

/**
 * @file ecmdClientPerlapi.H
 * @brief eCMD Perl API
 * Usage : 
   <pre>
 use ecmd;
   </pre>

*/

#ifndef DOCUMENTATION
namespace ECMDPERLAPI {

#endif

  /** @name Load/Unload Functions */
  //@{

  /**
   @brief Initialize the eCMD DLL
   @retval 0 if successful load  1 if unsuccessful
   @param i_dllName Full path and filename of the eCMD Dll to load
   @param i_clientVersion Comma seperated list of eCMD Perl api major numbers this script supports, see details
   @pre ecmdClientPerlapi constructor must have been called

   - initializes the eCMD Dll.<br>

   VERSIONS :<br>
   eCMD at times has to make changes to add/remove functionality and parameters to functions.  This could cause
   incompatability in your script if you used functions that have changed.  The i_clientVersion string is used
   to tell eCMD which major releases you support such that your script will not continue execution if it encounters
   a version that is either not known about or not supported.  This is similar to how the eCMD C-Api works except in
   Perl you can support multiple versions with one script as long as the changes that were made between the versions
   do not affect your script.

   USAGE :<br>
   if (ecmdLoadDll("","ver3,ver4")) { die "Fatal errors initializing DLL"; }<br>

   */
  uint32_t ecmdLoadDll(const char * i_dllName, const char * i_clientVersion);


  /**
   @brief Clean up the Perl Module - MUST BE CALLED JUST BEFORE SCRIPT EXIT
   */
  void ecmdUnloadDll();

  /**
   @brief Pass any unknown command line paramaters to the DLL for processing (ex. -p#, Cronus -debug)
   @retval ECMD_SUCCESS if successful
   @retval nonzero if unsuccessful
   @param io_argv Passed from Command line Arguments
   @pre initDll must have been called
   @post Global options (ex. -p#, -c#) will be removed from arg list
   @see initDll

   - argv gets passed to the eCMD DLL.<br>
   - Global options such as -p#, -c# will be parsed out.<br>
   - Target flags can be queried later with functions like ecmdQuerySelected<br>
   NOTE : This function does not affect ring caching<br>

   USAGE :<br>
   ecmdCommandArgs(\@ARGV);<br>

   */

  uint32_t ecmdCommandArgs(char** io_argv); 

  //@}



  /** @name Command Line Parsing Functions */
  //@{

  /**
   * @brief Iterates over argv, looking for given option string, removes it if found
   * @retval 1 if option found, 0 otherwise
   * @param io_argv Array of strings passed in from command line
   * @param i_option Option to look for
   * @see ecmdParseOptionWithArgs

   */
  bool ecmdParseOption (char ** io_argv, const char * i_option);

  /**
   * @brief Iterates over argv, looking for given option string, removes it if found
   * @retval Value of option arg if found, NULL otherwise
   * @param io_argv Array of strings passed in from command line
   * @param i_option Option to look for
   * @see ecmdParseOptionWithArgs

   */
  char * ecmdParseOptionWithArgs(char ** io_argv, const char * i_option);

  //@}


#ifndef DOCUMENTATION
}
#endif




/*** -------------------------------------- ***/
/*** The following is doxygen documentation ***/
/*** -------------------------------------- ***/


/**
 * @mainpage

<br>

@section perlapi eCMD Perl API
 In addition to a C/C++ API, eCMD provides a Perl API.  To do this, Perl's DynaLoader module handles the loading of a special eCMD shared object and the Perl XS interface handles the function calls between eCMD C code and Perl.
<br><br>
 The API interface is implemented as a "ecmdClientPerlapi" object.  The ecmdClientPerlapi class has methods to initialize eCMD, make standard function calls. 
<br>
 <hr>
 @section files Related Information
  NOTE : Some of this documentation may look like C/C++ files/headers but that is a side effect of the tool we are using to generated the PerlAPI and documentation.  Treat as Perl functions and especially look at any usage information associated with each function description

 <ul>
 <li>ecmdClientPerlapi.H</li>
 <li>ecmdBit64.H</li>
 <li>ecmdPerlApiTypes.H</li>
 <li>ecmdDataBuffer</li>
 <li>ecmdStructs.H</li>
 <li>ecmdUtils.H</li>
 <li>ecmdSharedUtils.H</li>
 </ul>

<hr>
@section ext eCMD Extensions
Check ecmdExtPerlapi.H to see the supported extensions and how to use them

 <hr>
 @section perlv Perl Version
 eCMD All scripts using the perl module should start with the below.  The perl version of ECMDPERLBIN needs to match the perl version used to build the perl module.

 <pre>
 #!/bin/ksh 
 #! -*- perl -*-

 eval '
 exec $ECMDPERLBIN -x -S $0 ${1+"$@"}
 '
 if 0;

 use strict;
 use ecmd;


 # Initialize the Plugin - has to be first thing done
 # Here we pass in that we know this script supports eCMD Releases 1.x if we know we support more we could pass like "ver1,ver2"
 if (ecmdLoadDll("", "ver1")) { die "Fatal errors initializing DLL"; }


 </pre>


 <br>
 <hr>
 @section extinit Using eCMD Perl Extensions
 If you need functionality from an eCMD Extension that provides the Perl Api, it is initialized like below.  This is a 'cip'
extension example but all extensions work the same way just replace the prefix with the extension of choice.
<br>
<pre>

 # Initialize the Plugin - has to be first thing done 
 if (ecmdLoadDll("", "ver1")) { die "Fatal errors initializing DLL"; }

 # Now initialize the extension as well
 # With the version 15 changes, this is no longer required by the user if the plugin you are using already does it.
 if (cipInitExtension("ver1")) { die "Fatal errors initializing CIP Extension"; }

</pre>

 <br>
 <hr>
 @section dform Data Passing
 @subsection ecmdDataBuffer ecmdDataBuffer
 Data is passed between the client Perl script and the eCMD shared object in the form of the same ecmdDataBuffer that is part of the C-Api.  The function set is the identical.  For additional documentation on the ecmdDataBuffer see the C/C++ Api Documentation.
<br>
Here is an example usage of the ecmdDataBuffer:
<pre>

  # Create a pointer to an DataBuffer class
  my $data = new ecmd::ecmdDataBuffer();

  # Set the size of my buffer
  $data->setBitLength(32);

  # Set the first word of data in this class
  $data->setWord(0,0xFEEDBEEF);

  # Read data from the chip
  my $rc = 0;
  $rc = getRing($target, "idreg", $data);

  # What is in the first word
  printf("Data : %.08X",$data->getWord(0));

  # Change the value
  $data->setWord(0,0xAAAA5555);
  printf("Data : %.08X",$data->getWord(0));

  # Write my new value to the chip
  $rc = putRing($target, "idreg", $data);
</pre>

 <b>WARNING: </b> The '=' operator DOES NOT work between ecmdDataBuffer's or any other structure like you would expect it to in C. See 'Perl API Usage' below
<br>
 @subsection uint64_t uint64_t (64 bit data)
 Perl doesn't support 64 bit variables, so eCMD provides an ecmdBit64 class to try to make up for this.
 The following operators have been overloaded with this class :
<ul>
<li> + - * / mod
<li> == != &lt; &lt;= &gt; &gt;=
<li> & | ^ ! ~
<li> ++ --
<li> &lt;&lt; &gt;&gt;
</ul>
This will allow you to do things like the following :
<pre>
  my $addr = new ecmd::ecmdBit64("0x00100FFE00000000");
  $addr |= 0x100200;
  $addr += 100;
  putMemProc($target, $addr, $data);

  $addr->setValue(1000);  # Set to decimal value 1000
  $addr->setValue(0x1000); # Set to hex value 0x1000

  if ($addr != 0x1000) { die ("failed") }
  
  my $addr2 = new ecmd::ecmdBit64();
  $addr->copy($addr2);                # Make addr2 == addr , you CANNOT do a '$addr2 = $addr;' see warning below
  # Or you could do this
  my $addr2 = new ecmd::ecmdBit64($addr);

</pre>
 <b>WARNING: </b> The '=' operator DOES NOT work between ecmdBit64's or any other structure like you would expect it to in C. See 'Perl API Usage' below

 <br><br>
 <hr>
@section target Chip/Object targeting
Perl functions use the same ecmdChipTarget structure as the C-Api.

<pre>
  my $target = new ecmd::ecmdChipTarget();
  $target->{chipType} = "pu";
  $target->{cage} = 0;
  $target->{node} = 0;
  $target->{slot} = 0;
  $target->{pos} = 1;
  $target->{chipUnitNum} = 0;

  $rc = putRing($target, "idreg", $data);

</pre>
 <b>WARNING: </b> The '=' operator DOES NOT work between ecmdChipTarget's or any other structure like you would expect it to in C. See 'Perl API Usage' below

<br><br>
 <hr>
 @section perluse Perl API Usage

 The following should be observed when using the Perl API.

 <ol>
 <li> The ecmdLoadDll() function should ALWAYS be the first function called.</li>
 <li> The ecmdUnloadDll() function should be called just before script exit</li>
 <li> Perl should be picked up as shown in the example below to grab the eCMD supported version</li>
 </ol>

<br>
 @subsection operator The '=' operator
 <b>WARNING:</b> Because of the behavior of Perl the following statements results in two variables pointing to the same data:
<pre>
  my $data1 = new ecmd::ecmdDataBuffer();
  $data1.setBitLength(32);
  $data1.setWord(0,0xFEEDBEEF);

  # Use the = operator
  my $data2 = $data1;

  printf("Data 1 : %.08X", $data1.getWord(0));        # This will echo 'Data 1 : FEEDBEEF'
  printf("Data 2 : %.08X", $data2.getWord(0));        # This will echo 'Data 2 : FEEDBEEF'

  # Now just set Data 2
  $data2.setWord(0,0xAAAA5555);

  printf("Data 1 : %.08X", $data1.getWord(0));        # This will echo 'Data 1 : AAAA5555'
  printf("Data 2 : %.08X", $data2.getWord(0));        # This will echo 'Data 2 : AAAA5555'
</pre>  
 NOTE : Since both $data1 and $data2 point to the same memory space when one is modified they are both modified.  This behavior is the same for the ecmdDataBuffer and all eCMD structures found in ecmdStructs.H

 To work around this problem you need to copy the contents of a structure manually :
 <ul>
 <li> For ecmdDataBuffer/ecmdBit64 : $data1->copy($data2)
 <li> For Structures : $target2->{cage} = $target->{cage}
 </ul>

 <br>
 <hr>
 @section perlscript Example Perl Script

 \include example.pl


*/


#endif /* _ecmdClientPerlapi_H */
