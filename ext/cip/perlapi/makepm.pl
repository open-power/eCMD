#!/bin/sh 
#! -*- perl -*-

eval '
if [ "X$CTEPERLBIN" = "X" ] ; then
 if [ "X$CTEPATH" = "X" ]; then echo "CTEPATH env var is not set."; exit 1; fi
 export CTEPERLBIN=$CTEPATH/tools/perl/5.8.1/bin/perl;
 export CTEPERLPATH=$CTEPATH/tools/perl/5.8.1;
 export CTEPERLLIB=$CTEPERLPATH/lib/5.8.1:$CTEPERLLIB;
fi
exec $CTEPERLBIN -x -S $0 ${1+"$@"}
'
if 0;

# File makepm.pl created by Chris Engel
# $Header$

use strict;

my $curdir = ".";


#functions to ignore in parsing <ext>ClientPerlapi.H because they are hand generated in ecmdClientPerlApi.C
my @ignores = qw( initDll cleanup ecmdConfigLooperNext InitExtension ecmdCommandArgs ecmdEnablePerlSafeMode ecmdDisablePerlSafeMode ecmdPerlInterfaceErrorCheck ecmdQuerySafeMode simFusionRand getLatch putLatch);
my $ignore_re = join '|', @ignores;



if ($ARGV[0] ne "ecmd") {
  open IN, "${curdir}/../ext/$ARGV[0]/capi/$ARGV[0]ClientCapi.H" or die "Could not find $ARGV[0]ClientCapi.H: $!\n";
} else {
  open IN, "${curdir}/../capi/$ARGV[0]ClientCapi.H" or die "Could not find $ARGV[0]ClientCapi.H: $!\n";
}
open OUT, ">${curdir}/$ARGV[0]ClientPerlapiFunc.H" or die $!;

print OUT "/* The following has been auto-generated by makepm.pl */\n\n";

my @fileLines = <IN>;
my $fileLine;
my @ifdefs;
my $print = 2;
my $tempstr;
my $beforePopPerlapi;
foreach $fileLine (@fileLines) {

  chomp($fileLine);

  # If we don't fix the names on the ifdefs and defines, we'll get all sorts of screwy compile errors
  $fileLine =~ s/ClientCapi/ClientPerlapiFunc/;

  if (substr($fileLine, 0, 6) eq "#ifdef" || substr($fileLine, 0, 7) eq "#ifndef" || substr($fileLine, 0, 11) eq "#if defined" || substr($fileLine, 0, 12) eq "#if !defined") {
    push(@ifdefs, $fileLine);
    if ((substr($fileLine, 0, 7) eq "#ifndef" || substr($fileLine, 0, 12) eq "#if !defined") && "@ifdefs" =~ "ECMD_PERLAPI") {
      $print = 0;
    }
  }

  if ($fileLine =~ "#endif") {
    $beforePopPerlapi = 0;
    if ("@ifdefs" =~ "ECMD_PERLAPI") {
      $beforePopPerlapi = 1;
    }
    pop(@ifdefs);
    if (!("@ifdefs" =~ "ECMD_PERLAPI")) {
      if ($beforePopPerlapi) {
        $print = 1;
      } else {
        $print = 2;
      }
    }
  }

  # This relies upon using the ending extern C } to close the namespace
  if ($fileLine =~ "extern \"C\" {") {
    $tempstr = sprintf("namespace %sPERLAPI {\n", uc($ARGV[0]));
    print OUT $tempstr;
    next;
  }

  if ($print) {
    if ($print == 1) {
      $print++;  #Make it so we don't print the ending endif for the PERLAPI
    } else {
      print OUT $fileLine . "\n";
    }
  }
}

close IN;
close OUT;



open IN, "${curdir}/$ARGV[0]ClientPerlapiFunc.H" or die "Could not find $ARGV[0]ClientPerlapiFunc.H: $!\n";
open OUT, ">${curdir}/$ARGV[0]ClientPerlapiFunc.C" or die $!;


print OUT "/* The following has been auto-generated by makepm.pl */\n\n";

print OUT "#include <string.h>\n";
print OUT "#include <stdio.h>\n";
print OUT "#include <ctype.h>\n";
print OUT "\n";

print OUT "#include <ecmdClientCapi.H>\n";
print OUT "#include <ecmdReturnCodes.H>\n";
print OUT "#include <ecmdUtils.H>\n";
print OUT "#include <ecmdSharedUtils.H>\n";
print OUT "#include <ecmdClientPerlapi.H>\n";
print OUT "#include <ecmdClientPerlapiFunc.H>\n";
if ($ARGV[0] ne "ecmd") {
  print OUT "#include <$ARGV[0]ClientPerlapi.H>\n";
  print OUT "#include <$ARGV[0]ClientPerlapiFunc.H>\n";
  print OUT "#include <$ARGV[0]ClientCapi.H>\n";
}
print OUT "\n";


#parse file spec'd by $ARGV[0]
while (<IN>) {
  # Maintain the remove_sim ifdef's
    if (/REMOVE_SIM/) {
      print OUT $_;

    } elsif (/^(uint32_t|uint64_t|std::string|void|bool|int)/) {

	next if (/$ignore_re/o);

	chomp; chop;  
	my ($func, $args) = split(/\(|\)/, $_);

	my ($type, $funcname) = split(/\s+/, $func);
	my @argnames = split(/,/,$args);

        #remove the default initializations
        foreach my $i (0..$#argnames) {
            if ($argnames[$i] =~ /=/) {
              $argnames[$i] =~ s/=.*//;
            }
        }
        $" = ",";

	my $argstring;
	my $typestring;
        my $tmptypestring;
	foreach my $curarg (@argnames) {

	    my @argsplit = split /\s+/, $curarg;

	    my @typeargs = @argsplit[0..$#argsplit-1];
	    $tmptypestring = "@typeargs";

	    my $tmparg = $argsplit[-1];
	    if ($tmparg =~ /\[\]$/) {
		chop $tmparg; chop $tmparg;
		$tmptypestring .= "[]";
	    }

	    $typestring .= $tmptypestring . ", ";
	    $argstring .= $tmparg . ", ";
	}

	chop ($typestring, $argstring);
	chop ($typestring, $argstring);


        my $namespace = uc($ARGV[0]) . "PERLAPI";
        if ($type eq "void") {
          print OUT "$type $namespace\::$funcname(@argnames) { \n";
          print OUT "  ::$funcname($argstring);\n";
          print OUT "}\n\n";
        } elsif (($type eq "uint32_t") || ($type eq "uint64_t") || ($type eq "int")) {
          print OUT "$type $namespace\::$funcname(@argnames) { \n";
          print OUT "  $type rc = ::$funcname($argstring);\n";
          print OUT "  return rc;\n";
          print OUT "}\n\n";
        } else {
          print OUT "$type $namespace\::$funcname(@argnames) { \n";
          print OUT "  return ::$funcname($argstring);\n";
          print OUT "}\n\n";
        }


    }

}
close IN;

print OUT "/* The previous has been auto-generated by makepm.pl */\n";

close OUT;  #<ext>ClientPerlapiFunc.C

# Following added by Jason Albert to handle the auto generation of iterator classes

# See if the main .i is out there.  If not, do nothing.
if (-e "$curdir/$ARGV[0]ClientPerlapi.i") {
  open IN, "$curdir/$ARGV[0]ClientPerlapi.i" or die "Could not find $ARGV[0]ClientPerlapi.i: $!\n";
  open RENAMEOUT, ">$curdir/$ARGV[0]Rename.i" or die $!;
  open HEADEROUT, ">$curdir/$ARGV[0]ClientPerlapiIterators.H" or die $!;
  open PERLOUT, ">$curdir/$ARGV[0]ClientPerlapiIterators.pl" or die $!;

  # Write some basic headers out to the autogen files
  print RENAMEOUT "// Autogenerated by makepm.pl based upon the \"%template\"'s defined in $ARGV[0]ClientPerlapi.i\n";
  print HEADEROUT "// Autogenerated by makepm.pl based upon the \"%template\"'s defined in $ARGV[0]ClientPerlapi.i\n";
  print PERLOUT   "#  Autogenerated by makepm.pl based upon the \"%template\"'s defined in $ARGV[0]ClientPerlapi.i\n";

  my @fileLines = <IN>;
  my $fileLine;
  my @fileLineArr;
  my $swigName;
  my $codeName;
  my $codeDataType;
  foreach $fileLine (@fileLines) {

    if (substr($fileLine,0,9) eq "%template") {

      @fileLineArr = split(/\s+/, $fileLine);
      # Get the swig name out of the first half
      $swigName = (split(/\(|\)/, $fileLineArr[0]))[1] . "Iterator";
      # Get the code out of the second half (eat the ; on the end)
      $codeName = $fileLineArr[1];
      chop($codeName);
      # Get the data type out
      $codeDataType = (split(/\<|\>/, $codeName))[1];

      # Now Write it out
      print RENAMEOUT "%rename(operatorEqualTo)    operator==(const $codeName\::iterator& rhs) const;\n";
      print RENAMEOUT "%rename(operatorNotEqualTo) operator!=(const $codeName\::iterator& rhs) const;\n";
      print RENAMEOUT "%rename(operatorIncrement)  operator++(int);\n";
      print RENAMEOUT "%rename(operatorDecrement)  operator--(int);\n";

      # Move on to the header file
      print HEADEROUT "struct $swigName {\n";
      print HEADEROUT "  public:\n";
      print HEADEROUT "    $codeDataType getValue() { return *iter; }\n";
      print HEADEROUT "    void setValue($codeDataType in) { *iter = in; }\n";
      print HEADEROUT "    $codeName\::iterator getIter() { return iter; }\n";
      print HEADEROUT "    void setIter($codeName\::iterator in) { iter = in; }\n\n";
      print HEADEROUT "    $codeName\::iterator operator++ (int) { return iter++; }\n";
      print HEADEROUT "    $codeName\::iterator operator-- (int) { return iter--; }\n";
      print HEADEROUT "    int operator== (const $codeName\::iterator &rhs) const { return (iter == rhs); }\n";
      print HEADEROUT "    int operator!= (const $codeName\::iterator &rhs) const { return (iter != rhs); }\n\n";
      print HEADEROUT "  private:\n";
      print HEADEROUT "    $codeName\::iterator iter;\n";
      print HEADEROUT "};\n\n";

      # Do the perl operators last
      print PERLOUT "\n# -------------------------------------------\n\n";
      print PERLOUT "package $ARGV[0]\::$swigName;\n\n";
      print PERLOUT "use overload \n";
      print PERLOUT "  fallback => 1,\n";
      print PERLOUT "  '==' => \\&operatorEqualTo,\n";
      print PERLOUT "  '!=' => \\&operatorNotEqualTo,\n";
      print PERLOUT "  '++' => \\&operatorIncrement,\n";
      print PERLOUT "  '--' => \\&operatorDecrement,\n";
      print PERLOUT ";\n\n";
      print PERLOUT "sub operatorEqualTo {\n";
      print PERLOUT "  my (\$self,\$rhs) = \@_;\n";
      print PERLOUT "  return \$self->operatorEqualTo(\$rhs);\n";
      print PERLOUT "};\n\n";
      print PERLOUT "sub operatorNotEqualTo {\n";
      print PERLOUT "  my (\$self,\$rhs) = \@_;\n";
      print PERLOUT "  return \$self->operatorNotEqualTo(\$rhs);\n";
      print PERLOUT "};\n\n";
      print PERLOUT "sub operatorIncrement {\n";
      print PERLOUT "  my (\$self) = \@_;\n";
      #print PERLOUT "  printf(\"In plusplus\\n\");\n";
      print PERLOUT "  return \$self->operatorIncrement(1);\n";
      print PERLOUT "};\n\n";
      print PERLOUT "sub operatorDecrement {\n";
      print PERLOUT "  my (\$self) = \@_;\n";
      print PERLOUT "  return \$self->operatorDecrement(1);\n";
      print PERLOUT "};\n\n";
    }
  }

  close IN;
  close RENAMEOUT;
  close HEADEROUT;
  close PERLOUT;
}
