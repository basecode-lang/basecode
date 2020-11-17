Name:           pstreams-devel
Version:        1.0.1
Release:        1%{?dist}
Summary:        POSIX Process Control in C++

Group:          Development/Libraries
License:        Boost
URL:            http://pstreams.sourceforge.net/
Source0:        http://downloads.sourceforge.net/pstreams/pstreams-%{version}.tar.gz

BuildRequires:  doxygen
BuildArch:      noarch

%description
PStreams classes are like C++ wrappers for the POSIX.2 functions
popen(3) and pclose(3), using C++ iostreams instead of C's stdio
library.

%prep
%setup -q -n pstreams-%{version}

%build
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make install  DESTDIR=$RPM_BUILD_ROOT includedir=%{_includedir}

%clean
rm -rf $RPM_BUILD_ROOT

%files
%doc doc/html LICENSE_1_0.txt README AUTHORS ChangeLog
%{_includedir}/pstreams

%changelog
* Thu Feb 02 2017 Jonathan Wakely <jwakely@redhat.com> - 1.0.1-1
- Update version and %%License.

* Thu Dec 01 2016 Jonathan Wakely <pstreams@kayari.org> - 1.0.0-1
- Update version.

* Tue Jan 26 2016 Jonathan Wakely <jwakely@redhat.com> - 0.8.1-4
- Remove redundant %%defattr.

* Tue Jan 05 2016 Jonathan Wakely <pstreams@kayari.org> - 0.8.1-3
- Replace packagename macro and remove BuildRoot tag.

* Thu Jul 10 2014 Jonathan Wakely <pstreams@kayari.org> - 0.8.1-2
- Fix rpmlint warning and package description.

* Fri Feb 07 2014 Jonathan Wakely <pstreams@kayari.org> - 0.8.1-1
- Update version.

* Wed Jan 23 2013 Jonathan Wakely <pstreams@kayari.org> - 0.8.0-1
- Update version.

* Thu Oct 14 2010 Jonathan Wakely <pstreams@kayari.org> - 0.7.1-1
- Update version and override includedir make variable instead of prefix.

* Wed May 12 2010 Jonathan Wakely <pstreams@kayari.org> - 0.7.0-1
- Add spec file to upstream repo and update.

* Sun Jul 26 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.6.0-8
- Rebuilt for https://fedoraproject.org/wiki/Fedora_12_Mass_Rebuild

* Thu Feb 26 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.6.0-7
- Rebuilt for https://fedoraproject.org/wiki/Fedora_11_Mass_Rebuild

* Fri Nov 07 2008 Rakesh Pandit <rakesh@fedoraproject.org> 0.6.0-6
- timestamp patch (Till Mass)

* Fri Nov 07 2008 Rakesh Pandit <rakesh@fedoraproject.org> 0.6.0-5
- saving timestamp using "install -p"

* Fri Nov 07 2008 Rakesh Pandit <rakesh@fedoraproject.org> 0.6.0-4
- included docs, license and other missing files.

* Fri Nov 07 2008 Rakesh Pandit <rakesh@fedoraproject.org> 0.6.0-3
- consistent use of macros - replaced %%{buildroot} with $RPM_BUILD_ROOT

* Thu Nov 06 2008 Rakesh Pandit <rakesh@fedoraproject.org> 0.6.0-2
- Cleaned up buildrequire

* Tue Nov 04 2008 Rakesh Pandit <rakesh@fedoraproject.org> 0.6.0-1
- initial package
