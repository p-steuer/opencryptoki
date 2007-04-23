%define        token     tpm
%define        base      opencryptoki

Name:          opencryptoki-%{token}tok
Version:       2.2.4
Release:       1%{?dist}
Summary:       An opencryptoki %{token} token 

Group:         Applications/Productivity 
License:       CPL 
URL:           http://sourceforge.net/projects/opencryptoki 
Source0:       %{base}-%{version}.tar.bz2 
BuildRoot:     %{_tmppath}/%{base}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires: autoconf automake libtool 
Requires:      opencryptoki = %{version}-%{release}


%description
The opencryptoki-tpmtok package provides an opencryptoki token for the  
Trusted Computing Platform (TPM) cryptographic device.


%prep 
%setup -q -n %{base}-%{version}


%build 
autoreconf --force --install
%configure --disable-static --disable-daemon --disable-swtok --disable-library --enable-%{token}tok
make %{?_smp_mflags}


%install 
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT 
rm -f $RPM_BUILD_ROOT/%{_libdir}/%{base}/*.la
rm -f $RPM_BUILD_ROOT/%{_libdir}/%{base}/stdll/*.la


%clean 
rm -rf $RPM_BUILD_ROOT


%files 
%defattr(-,root,root,-)
%doc LICENSE
%dir %{_libdir}/%{base}
%dir %{_libdir}/%{base}/stdll
%{_libdir}/%{base}/stdll/libpkcs11_%{token}.so
%{_libdir}/%{base}/stdll/libpkcs11_%{token}.so.0
%attr(755,root,root) %{_libdir}/%{base}/stdll/libpkcs11_%{token}.so.0.0.0


%changelog
* Tue Aug 7 2006 Daniel H Jones <danjones@us.ibm.com> 
- initial file created
