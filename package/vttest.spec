Summary: vttest - test VT100-type terminal
%define AppProgram vttest
%define AppPatched 20180811
%define AppVersion 2.7.%{AppPatched}
# $XTermId: vttest.spec,v 1.29 2018/08/11 12:03:11 tom Exp $
Name: %{AppProgram}
Version: %{AppVersion}
Release: 1
License: MIT
Group: Applications/Development
URL: ftp://invisible-island.net/%{AppProgram}
Source0: %{AppProgram}-%{AppPatched}.tgz
Packager: Thomas Dickey <dickey@invisible-island.net>

%description
Vttest is a program designed to test the functionality of a VT100
terminal (or emulator thereof).  It tests both display (escape sequence
handling) and keyboard.

The program is menu-driven and contains full on-line operating
instructions.  To run a given menu-item, you must enter its number.  You
can run all menu-items (for a given level) by entering an asterisk, i.e,
`*'.

%prep

# no need for debugging symbols...
%define debug_package %{nil}

%setup -q -n %{AppProgram}-%{AppPatched}

%build

INSTALL_PROGRAM='${INSTALL}' \
%configure \
  --target %{_target_platform} \
  --prefix=%{_prefix} \
  --bindir=%{_bindir} \
  --libdir=%{_libdir} \
  --mandir=%{_mandir}

make

%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

make install                    DESTDIR=$RPM_BUILD_ROOT

strip $RPM_BUILD_ROOT%{_bindir}/%{AppProgram}

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_prefix}/bin/%{AppProgram}
%{_mandir}/man1/%{AppProgram}.*

%changelog
# each patch should add its ChangeLog entries here

* Mon Jul 02 2018 Thomas Dickey
- use recommended flags for build

* Thu Aug 26 2010 Thomas Dickey
- initial version
