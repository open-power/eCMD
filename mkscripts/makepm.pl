#!/usr/bin/perl
# File makepm.pl created by Chris Engel

use strict;

my $curdir = ".";


#functions to ignore in parsing <ext>ClientPerlapi.H because they are hand generated in ecmdClientPerlApi.C
my @ignores = qw( initDll cleanup InitExtension ecmdCommandArgs simFusionRand getLatch putLatch getLatchHidden putLatchHidden getLatchImage putLatchImage);
my $ignore_re = join '|', @ignores;

my $genAll = 0;
my $didFile = 0;
if ($ARGV[1] eq "") {
  $genAll = 1;
}


if ($ARGV[1] =~ /ClientPerlapiFunc.H/ || $genAll) {
  # So we don't error at the end
  $didFile = 1;

  open IN, "${curdir}/../capi/$ARGV[0]ClientCapi.H" or die "Could not find $ARGV[0]ClientCapi.H: $!\n";
  open OUT, ">" . $ENV{"SRCPATH"} . "/$ARGV[0]ClientPerlapiFunc.H" or die $!;

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
    if ($fileLine =~ "extern \"C\" \\\{") {
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
}


if ($ARGV[1] =~ /ClientPerlapiFunc.C/ || $genAll) {
  # So we don't error at the end
  $didFile = 1;

  open IN, $ENV{"SRCPATH"} . "/$ARGV[0]ClientPerlapiFunc.H" or die "Could not find $ARGV[0]ClientPerlapiFunc.H: $!\n";
  open OUT, ">" . $ENV{"SRCPATH"} . "/$ARGV[0]ClientPerlapiFunc.C" or die $!;


  print OUT "/* The following has been auto-generated by makepm.pl */\n\n";

  print OUT "#include <string.h>\n";
  print OUT "#include <stdio.h>\n";
  print OUT "#include <ctype.h>\n";
  print OUT "\n";

  print OUT "#include <ecmdDefines.H>\n";
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
    # Maintain the remove_sim and other ifdef's
    if (/REMOVE_SIM/ || 
        /_REMOVE_/ ) {
          print OUT $_;

        } elsif (/^(uint32_t|uint64_t|std::string|void|bool|int)/) {

          next if (/$ignore_re/o);

          chomp; chop;  
          my ($func, $args) = split(/\(|\)/, $_);

          my ($type, $funcname) = split(/\s+/, $func);
          my @argnames = split(/,/,$args);

          #join any split std::pair or std::map arguments
          foreach my $i (0..$#argnames) {
            if ($argnames[$i] =~ /std::pair|std::map/) {
              $argnames[$i] = "$argnames[$i], $argnames[$i+1]";
              splice(@argnames, $i+1, 1);
            }
          }

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
}

# Following added by Jason Albert to handle the auto generation of iterator classes

if ($ARGV[1] =~ /ClientPerlapiIterators.H/ || $genAll) {
  # So we don't error at the end
  $didFile = 1;

  # See if the main .i is out there.  If not, do nothing.
  if (-e "$curdir/$ARGV[0]ClientPerlapi.i") {
    open IN, "$curdir/$ARGV[0]ClientPerlapi.i" or die "Could not find $ARGV[0]ClientPerlapi.i: $!\n";
    open HEADEROUT, ">" . $ENV{"SRCPATH"} . "/$ARGV[0]ClientPerlapiIterators.H" or die $!;

    # Write some basic headers out to the autogen files
    print HEADEROUT "// Autogenerated by makepm.pl based upon the \"%template\"'s defined in $ARGV[0]ClientPerlapi.i\n";

    my @fileLines = <IN>;
    my $fileLine;
    my @fileLineArr;
    my $swigName;
    my $codeName;
    my $codeDataType;
    foreach $fileLine (@fileLines) {

      if ((substr($fileLine,0,9) eq "%template") && ($fileLine =~ /list|vector/))  {

        @fileLineArr = split(/\s+/, $fileLine);
        # Get the swig name out of the first half
        $swigName = (split(/\(|\)/, $fileLineArr[0]))[1] . "Iterator";
        # Get the code out of the second half (eat the ; on the end)
        $codeName = $fileLineArr[1];
        chop($codeName);
        # Get the data type out
        $codeDataType = (split(/\<|\>/, $codeName))[1];

        # Move on to the header file
        print HEADEROUT "class $swigName {\n";
        print HEADEROUT "  public:\n";
        print HEADEROUT "    $codeDataType getValue() { return *iter; }\n";
        print HEADEROUT "    void setValue($codeDataType in) { *iter = in; }\n";
        print HEADEROUT "    $swigName getIter() { $swigName ret = *this; return ret; }\n";
        print HEADEROUT "    void setIter($codeName\::iterator in) { iter = in; }\n\n";
        print HEADEROUT "    $swigName& operator++ ()    { iter++; return *this; }\n";
        print HEADEROUT "    $swigName  operator++ (int) { $swigName ret = *this; iter++; return ret; }\n";
        print HEADEROUT "    $swigName& operator-- ()    { iter--; return *this; }\n";
        print HEADEROUT "    $swigName  operator-- (int) { $swigName ret = *this; iter--; return ret; }\n";
        print HEADEROUT "    int operator== (const $swigName &rhs) const { return (iter == rhs.iter); }\n";
        print HEADEROUT "    int operator== (const $codeName\::iterator &rhs) const { return (iter == rhs); }\n";
        print HEADEROUT "    int operator!= (const $swigName &rhs) const { return (iter != rhs.iter); }\n";
        print HEADEROUT "    int operator!= (const $codeName\::iterator &rhs) const { return (iter != rhs); }\n\n";
        print HEADEROUT "  private:\n";
        print HEADEROUT "    $codeName\::iterator iter;\n";
        print HEADEROUT "};\n\n";
      }
    }

    close IN;
    close HEADEROUT;
  }
}

if (!$didFile) {
  printf("ERROR: Unknown file type \"$ARGV[1]\" passed in!\n");
}
