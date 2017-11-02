#!/usr/bin/perl
# File ecmdsetup.pl created by Jason Albert,6A5244 at 13:50:19 on Fri May 13 2005. 

# Why wouldn't you use strict???
use strict;
use Cwd 'chdir';
##########################################################################
# Figure out where the user is calling this script from
#

# return value
my $rc = 0;

my $callingPwd;
my $pwd;

BEGIN {
  # Some plugins needs the directory the script was called from, save that here
  #Commenting this out since this seems to be resolving the links in the path
  #chomp($callingPwd = `pwd`);
  #This keeps the directory intact.
  chomp($callingPwd = `cd .;pwd`);
  # Now do the rest
  my @tempArr = split(/\/+/,$0);
  pop(@tempArr); # The script name
  $" = "/"; # Make it so the array below seperates on / instead of space
  #If it starts with a ., it's being called with relative path
  if (substr($0,0,1) ne "/") {
    $pwd = $callingPwd;  # Get where I got called from
    $pwd = $pwd . "/" . "@tempArr";  # Now use the relative to find out where the script is 
  } else { # Absolute path
    $pwd = "@tempArr";
  }
  $" = " "; # Reset
  chomp($pwd = `cd $pwd;pwd`);  # Get to where this script resides
}

# installpath points to root of install
my $installPath = $pwd;
$installPath =~ s/\/([^\/..]*\/?)$/\//;
# Remove the trailing slash if there
$installPath =~ s/\/$//;

#########################################
# Setup the modules to include
# Base support, always there
use lib "$pwd";
use ecmdsetup;
my $ecmd = new ecmdsetup();

# Any installed plugins are located a dir below where this script resides
# use lib that path so we can load those modules
# The load all those instances into a plugin hash that is used thru out
# This allows the code to be adaptable to any number of plugins
use lib "$pwd/plugins";

my %plugins;
my $pluginKey;

# Get a list of all plugins installed
my @tempArr = split(/\s+/, `ls $pwd/plugins`);

# Loop over that list and load everything in
for (my $x=0; $x <= $#tempArr; $x++) {
  # Create the key into the hash and the name of the module to load
  $pluginKey = $tempArr[$x];
  my $pluginSetup = $tempArr[$x];
  $pluginKey =~ s/setup.pm//g;
  $pluginSetup =~ s/.pm//g;
  # Load the module and create the object
  eval "require $pluginSetup";
  $plugins{$pluginKey} = new $pluginSetup();
}

##########################################################################
#  Variables
#
my %modified;  # 0 is no change, 1 set needed, -1 unset needed
my $shell;
my $release;
my $prevRelease;
my $plugin;
my $product;
my $bits = 32;
my $arch;
my $temp;
my $shortcut = 0;
my $singleInstall = 1;  # Assume it's a single install and then disprove it by looking at the path
my $copyLocal = 0;  # Does the user want ECMD_EXE and ECMD_DLL_FILE copied to /tmp and run from there?
my $cleanup = 0;  # Call only cleanup on the plugins to remove anything they might have put out there.

#####################################################
# Call the main function, then add the rc from that to the output
#
$rc = main();
exit($rc);

sub main {
  #####################################################
  # Look to see if help was requested
  #
  if ("@ARGV" =~ /-h/) {
    help();
    foreach $pluginKey ( keys %plugins ) {
      $plugins{$pluginKey}->help();
    }
    return 1;
  }

  ##########################################################################
  # Figure out if the user is on a multi-instance install
  #
  # $installPath will point to the root of this install
  # In a single install this would be /<something>/ecmd
  # In a multi install this would be /<something>/ecmd/<ver>
  my @installPathArray = split(/\//, $installPath);
  if ($installPathArray[$#installPathArray - 1] eq "ecmd") {
    $singleInstall = 0;
  }

  ##########################################################################
  # Get the users shell
  #
  $shell = shift(@ARGV);

  # If you add a shell here, you need to update the output printing below
  if ($shell eq "ksh") {
  } elsif ($shell eq "csh") {
  } else {
    ecmd_print("Your shell is unsupported!", 1);
    return 1;
  }

  ##########################################################################
  # Get the release
  #

  # Save the previous release the user may have had
  $prevRelease = $ENV{"ECMD_RELEASE"};

  $release = shift(@ARGV);

  # Here is where we put in the magic to allow the user to just put a period to cover all four ecmd parms
  if ($release eq ".") {
    if ($ENV{"ECMD_RELEASE"} eq "" || $ENV{"ECMD_PLUGIN"} eq "" || $ENV{"ECMD_PRODUCT"} eq "" || $ENV{"ECMD_ARCH"} eq "") {
      ecmd_print("You can't specify the '.' shortcut without having specified the release, product and plugin previously!", 1);
      return 1;
    } else {
      $shortcut = 1;
    }
  }

  if ($shortcut) {
    $release = $ENV{"ECMD_RELEASE"};
  }

  ##########################################################################
  # Get the plugin
  #
  if ($shortcut) {
    $plugin = $ENV{"ECMD_PLUGIN"};
  } else {
    $plugin = shift(@ARGV);
  }
  # See if the plugin passed in matches any of the ones we have modules for
  my $pluginFound = 0;
  foreach $pluginKey ( keys %plugins ) {
    if ($pluginKey eq $plugin) {
      $pluginFound = 1;
    }
  }
  if ($pluginFound == 0) {
    ecmd_print("The eCMD plugin '$plugin' you specified is not known!", 1);
    return 1;
  }


  ##########################################################################
  # Get the product
  #
  if ($shortcut) {
    $product = $ENV{"ECMD_PRODUCT"};
  } else {
    $product = shift(@ARGV);
  }

  ##########################################################################
  # Loop through the args left and see if any are for ecmd
  #
  for (my $x = 0; $x <= $#ARGV;) {
    if ($ARGV[$x] eq "copylocal") {
      $copyLocal = 1;
      splice(@ARGV,$x,1);  # Remove so plugin doesn't see it
    } elsif ($ARGV[$x] eq "cleanup") {
      $cleanup = 1;
      ecmd_print("Removing eCMD and Plugin settings from environment", 1);
      splice(@ARGV,$x,1);  # Remove so plugin doesn't see it
    } elsif ($ARGV[$x] eq "64") {   
	$bits = 64;
	splice(@ARGV,$x,1);  # Remove so plugin doesn't see it
    } elsif ($ARGV[$x] eq "32") {   
	$bits = 32;
	splice(@ARGV,$x,1);  # Remove so plugin doesn't see it
    } elsif ($ARGV[$x] eq "quiet") {
	$ecmdsetup::quiet = 1;
	splice(@ARGV,$x,1);  # Remove so plugin doesn't see it
    } else {
      # We have to walk the array here because the splice shortens up the array
      $x++;
    }
  }

  ##########################################################################
  # Determine the desired architecture for $ECMD_ARCH
  #
  if ($shortcut) {
    $arch = $ENV{"ECMD_ARCH"};
  } else {
    # AIX
    if (`uname` eq "AIX\n") {
      if ($bits eq "32") {
        $arch = "aix";
      }
      elsif ($bits eq "64") {
        $arch = "aix64";
      }
      else {
        ecmd_print("'$bits' is not a valid bit value!", 1);
        return 1;
      }
      # PPC64LE
    } elsif (`uname -a|grep ppc64le` ne "") {
      if ($bits eq "64") {
        $arch = "ppc64le";
      }
      else {
        ecmd_print("'$bits' is not a valid bit value!", 1);
        return 1;
      }
      # PPC
    } elsif (`uname -a|grep ppc` ne "") {
      if ($bits eq "32") {
        $arch = "ppc";
      }
      elsif ($bits eq "64") {
        $arch = "ppc64";
      }
      else {
        ecmd_print("'$bits' is not a valid bit value!", 1);
        return 1;
      }
      # X86
    } else {
      if ($bits eq "32") {
        $arch = "x86";
      }
      elsif ($bits eq "64") {
        $arch = "x86_64";
      }
      else {
        ecmd_print("'$bits' is not a valid bit value!", 1);
        return 1;
      }    
    }
  }

  ##########################################################################
  # Cleanup any ecmd bin dirs that might be in the path
  #
  # Pull out any of the matching cases
  if ($singleInstall) {
    $ENV{"PATH"} =~ s!$installPath/$arch/bin!:!g;
    $ENV{"PATH"} =~ s!$installPath/bin!:!g;
  } else {
    # This expression matches anything after a : up to /<something>/ecmd/<ver>/bin and then a : or end of line
    $ENV{"PATH"} =~ s!([^:]*?)/ecmd/([^\/]*?)/$arch/bin(:|$)!:!g;
    $ENV{"PATH"} =~ s!([^:]*?)/ecmd/([^\/]*?)/bin(:|$)!:!g;
  }
  # Any multiple : cases, reduce to one
  $ENV{"PATH"} =~ s/(:+)/:/g;
  # We might have left a : on the front, remove it
  $ENV{"PATH"} =~ s/^://g;
  # Same with the back, might have left a :
  $ENV{"PATH"} =~ s/:$//g;
  # Now mark the path modifed
  $modified{"PATH"} = 1;

  ##########################################################################
  # Cleanup any old dirs that might be in the python path
  #
  # Pull out any of the matching cases
  if ($singleInstall) {
    $ENV{"PYTHONPATH"} =~ s!$installPath/$arch/python!:!g;
  } else {
    # This expression matches anything after a : up to /<something>/ecmd/<ver>/bin and then a : or end of line
    $ENV{"PYTHONPATH"} =~ s!([^:]*?)/ecmd/([^\/]*?)/$arch/python(:|$)!:!g;
  }
  # Any multiple : cases, reduce to one
  $ENV{"PYTHONPATH"} =~ s/(:+)/:/g;
  # We might have left a : on the front, remove it
  $ENV{"PYTHONPATH"} =~ s/^://g;
  # Same with the back, might have left a :
  $ENV{"PYTHONPATH"} =~ s/:$//g;
  # Now mark the path modifed
  $modified{"PYTHONPATH"} = 1;

  ##########################################################################
  # Call cleanup on plugins
  #
  # Only do this if the plugin has changed from last time
  if ($ENV{"ECMD_PLUGIN"} ne $plugin || $cleanup) {
    foreach $pluginKey ( keys %plugins ) {
      $rc = $plugins{$pluginKey}->cleanup(\%modified);
      if ($rc) {
        return $rc;
      }
    }
  }

  ##########################################################################
  # Flag the ECMD_* variables as modified if appropriate
  #
  if (!$shortcut) {
    $ENV{"ECMD_RELEASE"} = $release;
    $modified{"ECMD_RELEASE"} = 1;
    $ENV{"ECMD_PLUGIN"} = $plugin;
    $modified{"ECMD_PLUGIN"} = 1;
    $ENV{"ECMD_PRODUCT"} = $product;
    $modified{"ECMD_PRODUCT"} = 1;
    $ENV{"ECMD_ARCH"} = $arch;
    $modified{"ECMD_ARCH"} = 1;
  }
  if ($cleanup) {
    $modified{"ECMD_RELEASE"} = -1;
    $modified{"ECMD_PLUGIN"} = -1;
    $modified{"ECMD_PRODUCT"} = -1;
    $modified{"ECMD_ARCH"} = -1;
    $modified{"ECMD_PATH"} = -1;
  }

  ##########################################################################
  # Call setup on plugin specified
  #
  if (!$cleanup) {
    foreach $pluginKey ( keys %plugins ) {
      if ($plugin eq $pluginKey) { # Only call setup on our selected plugin
        $rc = $plugins{$pluginKey}->setup(\%modified,
                                          { ARGV => "@ARGV",
                                            singleInstall => $singleInstall,
                                            arch => $arch,
                                            product => $product,
                                            ecmdsetup => 1,
                                            callingPwd => $callingPwd,
                                            installPath => $installPath,
                                          });
        if ($rc) {
          return $rc;
        }
      }
    }
  }


  ##########################################################################
  # Figure out the ecmd release if the user passed in auto
  # Calling ecmdVersion will work now that everything is setup
  #
  if ($ENV{"ECMD_RELEASE"} eq "auto") {
    # Use the full path to get to ecmdVersion to dynamically establish the version
    # We have to call the full path because PATH won't always be established
    my $command = "$installPath/bin/ecmdVersion_$arch full";
    $ENV{"ECMD_RELEASE"} = `/bin/sh -c \"$command\"`;
    $modified{"ECMD_RELEASE"} = 1;
    $release = $ENV{"ECMD_RELEASE"};
  }

  ##########################################################################
  # If release was set to rel by plugin setup, change local variable
  #
  if ($ENV{"ECMD_RELEASE"} eq "rel") {
    $release = $ENV{"ECMD_RELEASE"};
  }

  # We'll see if the release is supported based upon the directory existing
  my @releasePathArray = @installPathArray;
  if (!$singleInstall) {
    # Replace the release path for the script with the one passed in
    $releasePathArray[$#releasePathArray] = $release;
    $temp = sprintf join("/", @releasePathArray);
    if (!(-d $temp)) {
      ecmd_print("The eCMD release '$release' you specified is not known!", 1);
      return 1;
    }
  }
  # Setup release path for use throughout the rest of the script
  my $releasePath = sprintf join("/", @releasePathArray);

  ##########################################################################
  # Set ECMD_PATH based upon the releasePath figured out above
  #
  $ENV{"ECMD_PATH"} = $releasePath . "/";
  $modified{"ECMD_PATH"} = 1;

  ##########################################################################
  # Add bin directory to path
  #
  if (!$cleanup) {
    $ENV{"PATH"} = $releasePath . "/bin:" . $ENV{"PATH"};
    $ENV{"PATH"} = $releasePath . "/" . $arch . "/bin:" . $ENV{"PATH"};
    $modified{"PATH"} = 1;
  }

  ##########################################################################
  # Change shared lib path to point to release for cronus
  # This is because the release may have changed from the installPath that was used
  #
  if ((!$cleanup) && ($plugin eq "cro") && (!$singleInstall)) {
      my $sharedLib;
      if ($arch =~ m/aix/) {
          $sharedLib = "LIBPATH";
      } else {
          $sharedLib = "LD_LIBRARY_PATH";
      }
      # This expression matches anything after a : up to /<something>/ecmd/<ver>/<arch>/lib and then a : or end of line
      $ENV{"$sharedLib"} =~ s!([^:]*?)/ecmd/([^\/]*?)/([^\/]*?)/lib(:|$)!:!g;

      # Any multiple : cases, reduce to one
      $ENV{"$sharedLib"} =~ s/(:+)/:/g;
      # We might have left a : on the front, remove it
      $ENV{"$sharedLib"} =~ s/^://g;
      # Same with the back, might have left a :
      $ENV{"$sharedLib"} =~ s/:$//g;

      # Now add the releasePath that we've determined to the sharedLib path
      $ENV{"$sharedLib"} = $ENV{"$sharedLib"} . ":" . $releasePath . "/" . $arch . "/lib";
      $modified{"$sharedLib"} = 1;

  }

  ##########################################################################
  # Add python directory to PYTHONPATH
  #
  if (!$cleanup) {
    $ENV{"PYTHONPATH"} = $releasePath . "/" . $arch . "/python:" . $ENV{"PYTHONPATH"};
    
    # Any multiple : cases, reduce to one
    $ENV{"PYTHONPATH"} =~ s/(:+)/:/g;
    # We might have left a : on the front, remove it
    $ENV{"PYTHONPATH"} =~ s/^://g;
    # Same with the back, might have left a :
    $ENV{"PYTHONPATH"} =~ s/:$//g;

    $modified{"PYTHONPATH"} = 1;
  }

  ##########################################################################
  # Updates setup scripts if release changed
  # All we need to do is resource the setup scripts
  #
  if (($prevRelease ne $ENV{"ECMD_RELEASE"}) && !$singleInstall) {
    my $file;
    if ($shell eq "csh") {
      $file = sprintf("%s/bin/ecmdaliases.csh", $releasePath);
      printf("source $file;");
    } else {
      $file = sprintf("%s/bin/ecmdaliases.ksh", $releasePath);
      printf(". $file;");
    }
  }

  ####################################################
  # Do the copy to /tmp if local was given
  #
  if ($copyLocal) {
    foreach $pluginKey ( keys %plugins ) {
      if ($plugin eq $pluginKey) { # Only call setup on our selected plugin
        $rc = $plugins{$pluginKey}->copyLocal(\%modified,
                                          { cleanup => $cleanup,
                                          });
        if ($rc) {
          return $rc;
        }
      }
    }
  }

  ##########################################################################
  # Write out the modified environment
  #
  $ecmd->write_environment($shell,\%modified);
}

#  Umm.. yeah.. I'm going to need you to work this weekend on the help text.  Mkay..
sub help {
  ecmd_print("ecmdsetup <release> <plugin> <product> [32|64] [copylocal] [cleanup] <plugin options>");
  ecmd_print("<release> - Any eCMD Version currently supported in CVS (ex rel, ver5, ver4-3)");
  ecmd_print("<plugin> - varies based upon your ecmd install");
  ecmd_print("<product> - varies based upon plugin");
  ecmd_print("[32|64] - Use the 32 or 64-bit versions of eCMD and plugins.  Defaults to 32.");
  ecmd_print("[copylocal] - Copy the \$ECMD_EXE and \$ECMD_DLL_FILE to /tmp/\$ECMD_TARGET/");
  ecmd_print("[cleanup] - Remove all eCMD and Plugin settings from environment");
  ecmd_print("[quiet] - Disables status output");
  ecmd_print("<plugin options> - anything else passed into the script is passed onto the plugin");
  ecmd_print("-h - this help text");
}
