Summary: locks one or more virtual consoles
Name: vlock
Version: 1.3
Release: 1
Copyright: GPL
Group: Utilities/Console
Source: ftp://tsx-11.mit.edu:/pub/linux/sources/usr.bin/vlock-1.3.tar.gz
Requires: pam >= 0.59
BuildRoot: /tmp/vlock

%description
vlock either locks the current terminal (which may be any kind of
terminal, local or remote), or locks the entire virtual console
system, completely disabling all console access.  vlock gives up
these locks when either the password of the user who started vlock
or the root password is typed.

%changelog
* Wed Jan 13 1999 Michael Johnson <johnsonm@redhat.com>
- released 1.3

* Thu Mar 12 1998 Michael K. Johnson <johnsonm@redhat.com>
- Does not create a DoS attack if pty is closed (not applicable
  to use on a VC)

* Fri Oct 10 1997 Michael K. Johnson <johnsonm@redhat.com>
- Moved to new pam conventions.
- Use pam according to spec, rather than abusing it as before.
- Updated to version 1.1.
- BuildRoot

* Mon Jul 21 1997 Erik Troan <ewt@redhat.com>
- built against glibc

* Mon Mar 03 1997 Michael K. Johnson <johnsonm@redhat.com>
- moved from pam.conf to pam.d

%prep 
%setup

%build
make RPM_OPT_FLAGS="${RPM_OPT_FLAGS}"

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/bin
mkdir -p $RPM_BUILD_ROOT/usr/man/man1
mkdir -p $RPM_BUILD_ROOT/etc/pam.d
install -m 755 -o 0 -g 0 -s vlock $RPM_BUILD_ROOT/usr/bin
install -m 644 -o 0 -g 0 vlock.1 $RPM_BUILD_ROOT/usr/man/man1
install -m 644 -o 0 -g 0 vlock.pamd $RPM_BUILD_ROOT/etc/pam.d/vlock

%clean
rm -rf $RPM_BUILD_ROOT

%files
%config /etc/pam.d/vlock
/usr/bin/vlock
/usr/man/man1/vlock.1
