%module MODULE_NAME
%include typemaps.i
%include pointer.i

%typemap(perl5,in) char ** retval(char* cvalue) {
  SV* tempsv;
  if (!SvROK($source)) {
    croak("expected a reference\n");
  }
  tempsv = SvRV($source);
  if (!SvPOK(tempsv)) {
    croak("expected a string reference\n");
  }
  cvalue = SvPV(tempsv,PL_na);
  $target = (char**)malloc(sizeof(char*));

  *$target = cvalue;
}

%typemap(perl5,argout) char ** retval {
  SV *tempsv;
  tempsv = SvRV($arg);
  sv_setpv(tempsv, *$source);
  free(*$source);
  *$source = NULL;
  free($source);
}

%except(perl5) {
	$function
	if (ecmdPerlInterfaceErrorCheck(-1)) {
		croak("Error occured in Perl module\n");
	}
}

%{
#include "ecmdClientPerlapi.H"
%}

%include ecmdClientPerlapi.H
