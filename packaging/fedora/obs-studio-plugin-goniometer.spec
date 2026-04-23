Name: obs-studio-plugin-goniometer
Version: @VERSION@
Release: @RELEASE@%{?dist}
Summary: plugin to display audio goniometer on OBS Studio
License: GPLv2+

Source0: %{name}-%{version}.tar.bz2
Source1: @submodule:noriscommonui@
BuildRequires: cmake, gcc, gcc-c++
BuildRequires: obs-studio-devel
BuildRequires: qt6-qtbase-devel qt6-qtbase-private-devel

%description
This plugin provides a goniometer on OBS Studio, which is a visual
representation of the stereo relationship between the left and the right
channels of the audio signal.

%prep
%autosetup -p1
tar -xjf %{SOURCE1}

%build
%{cmake} -DQT_VERSION=6 -DINSTALL_LICENSE_FILES:BOOL=OFF
%{cmake_build}

%install
%{cmake_install}

%files
%{_libdir}/obs-plugins/*.so
%{_datadir}/obs/obs-plugins/*/
%license LICENSE
