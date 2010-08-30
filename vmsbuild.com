$! $Id: vmsbuild.com,v 1.19 2010/08/28 12:17:47 tom Exp $
$! VMS build-script for VTTEST.  Requires installed C compiler
$!
$! Tested with:
$!	VMS system version 5.4-2
$!	VAX-C version 3.2
$!
$!	Build the option-file
$!
$ open/write optf vms_link.opt
$ write optf "Identification=""VtTest 2.7"""
$ write optf "charsets.obj"
$ write optf "color.obj"
$ write optf "draw.obj"
$ write optf "esc.obj"
$ write optf "keyboard.obj"
$ write optf "mouse.obj"
$ write optf "nonvt100.obj"
$ write optf "printer.obj"
$ write optf "reports.obj"
$ write optf "reset.obj"
$ write optf "setup.obj"
$ write optf "sixel.obj"
$ write optf "status.obj"
$ write optf "tek4014.obj"
$ write optf "utf8.obj"
$ write optf "vms_io.obj"
$ write optf "vt220.obj"
$ write optf "vt320.obj"
$ write optf "vt420.obj"
$ write optf "vt52.obj"
$ write optf "vt520.obj"
$ write optf "xterm.obj"
$!
$! Look for the compiler used
$!
$ CC = "CC"
$ arch = "UNKNOWN"
$!
$ if f$getsyi("ARCH_NAME") .eqs. "Alpha"
$ then
$  arch = "__alpha__=1"
$  comp  = "__decc__=1"
$  CFLAGS = "/prefix=all"
$  DEFS = "HAVE_ALARM"
$  if f$trnlnm("SYS").eqs."" then define sys sys$library:
$ endif
$!
$ if f$getsyi("ARCH_NAME") .eqs. "IA64"
$ then
$  arch = "__ia64__=1"
$  comp  = "__decc__=1"
$  CFLAGS = "/prefix=all"
$  DEFS = "HAVE_ALARM,USE_IEEE_FLOAT"
$  if f$trnlnm("SYS").eqs."" then define sys sys$library:
$ endif
$!
$ if f$getsyi("ARCH_NAME") .eqs. "VAX"
$ then
$  arch = "__vax__=1"
$  if f$search("SYS$SYSTEM:DECC$COMPILER.EXE").eqs.""
$   then
$    if f$trnlnm("SYS").eqs."" then define sys sys$library:
$    DEFS = "HAVE_SYS_ERRLIST"
$    write optf "sys$library:vaxcrtl.exe/share"
$    if f$search("SYS$SYSTEM:VAXC.EXE").eqs.""
$     then
$     if f$trnlnm("GNU_CC").eqs.""
$      then
$       write sys$output "C compiler required to rebuild vttest"
$       close optf
$       exit
$     else
$      write optf "gnu_cc:[000000]gcclib.olb/lib"
$      comp = "__gcc__=1"
$      CC = "GCC"
$     endif 
$    else
$    comp  = "__vaxc__=1"
$    endif
$   else
$    if f$trnlnm("SYS").eqs."" then define sys decc$library_include:
$    comp  = "__decc__=1"
$  endif
$ endif
$
$ if "''arch'" .eqs. "UNKNOWN"
$ then
$   write sys$output "Cannot determine architecture type"
$   exit 1
$ endif
$
$ close optf
$
$! used /G_FLOAT with vaxcrtlg/share in vms_link.opt
$! can also use /Debug /Listing, /Show=All
$
$! -------------- vms_link.opt is created -------------
$ if f$edit("''p1'", "UPCASE") .eqs. "VMS_LINK.OPT"
$ then
$!  mms called this script to build vms_link.opt.  all done
$   exit
$ endif
$
$ if f$search("SYS$SYSTEM:MMS.EXE").eqs.""
$  then
$
$   CFLAGS := 'CFLAGS/Diagnostics /Define=("''DEFS'") /Include=([])
$
$	if "''p1'" .nes. "" then goto 'p1
$
$ all :
$	call make charsets
$	call make color
$	call make draw
$	call make esc
$	call make keyboard
$	call make main
$	call make mouse
$	call make nonvt100
$	call make printer
$	call make reports
$	call make reset
$	call make setup
$	call make sixel
$	call make status
$	call make tek4014
$	call make utf8
$	call make vt220
$	call make vt320
$	call make vt420
$	call make vt52
$	call make vt520
$	call make vms_io
$	call make xterm
$
$	link /exec=VTTEST/map/cross main.obj, vms_link/opt 
$	goto build_last
$
$ install :
$	WRITE SYS$ERROR "** no rule for install"
$	goto build_last
$	
$ clean :
$	if f$search("*.obj") .nes. "" then delete *.obj;*
$	if f$search("*.bak") .nes. "" then delete *.bak;*
$	if f$search("*.lis") .nes. "" then delete *.lis;*
$	if f$search("*.log") .nes. "" then delete *.log;*
$	if f$search("*.map") .nes. "" then delete *.map;*
$	if f$search("*.opt") .nes. "" then delete *.opt;*
$	goto build_last
$
$ clobber :
$	if f$search("vttest.com") .nes. "" then delete vttest.com;*
$	if f$search("*.exe") .nes. "" then delete *.exe;*
$	goto build_last
$
$ build_last :
$	if f$search("*.dia") .nes. "" then delete *.dia;*
$	if f$search("*.lis") .nes. "" then purge *.lis
$	if f$search("*.obj") .nes. "" then purge *.obj
$	if f$search("*.map") .nes. "" then purge *.map
$	if f$search("*.opt") .nes. "" then purge *.opt
$	if f$search("*.exe") .nes. "" then purge *.exe
$	if f$search("*.log") .nes. "" then purge *.log
$	exit
$
$ vms_link_opt :
$	exit 1
$!
$! Runs VTTEST from the current directory (used for testing)
$ vttest_com :
$	if "''f$search("vttest.com")'" .nes. "" then delete vttest.com;*
$	copy nl: vttest.com
$	open/append  test_script vttest.com
$	write test_script "$ temp = f$environment(""procedure"")"
$	write test_script "$ temp = temp -"
$	write test_script "		- f$parse(temp,,,""version"",""syntax_only"") -"
$	write test_script "		- f$parse(temp,,,""type"",""syntax_only"")"
$	write test_script "$ vttest :== $ 'temp'.exe"
$	write test_script "$ define/user_mode sys$input  sys$command"
$	write test_script "$ define/user_mode sys$output sys$command"
$	write test_script "$ vttest 'p1 'p2 'p3 'p4 'p5 'p6 'p7 'p8"
$	close test_script
$	write sys$output "** made vttest.com"
$	exit
$!
$! We have MMS installed
$  else
$   mms/macro=('comp','arch') 'p1
$  endif
$ exit
$
$ make: subroutine
$	if f$search("''p1'.obj") .eqs. ""
$	then
$		write sys$output "compiling ''p1'"
$		'CC 'CFLAGS 'p1.c
$		if f$search("''p1'.dia") .nes. "" then delete 'p1.dia;*
$	endif
$	exit
$ endsubroutine
