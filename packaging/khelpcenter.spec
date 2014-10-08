# 
# Do NOT Edit the Auto-generated Part!
# Generated by: spectacle version 0.27
# 

Name:       khelpcenter

# >> macros
# << macros

Summary:    Application to show KDE Application documentation
Version:    5.0.0
Release:    1
Group:      System/Base
License:    GPLv2+
URL:        http://www.kde.org
Source0:    %{name}-%{version}.tar.xz
Source100:  khelpcenter.yaml
Source101:  khelpcenter-rpmlintrc
Requires:   kf5-filesystem
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Xml)
BuildRequires:  pkgconfig(Qt5Network)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Widgets)
BuildRequires:  pkgconfig(Qt5Concurrent)
BuildRequires:  pkgconfig(Qt5Test)
BuildRequires:  pkgconfig(Qt5PrintSupport)
BuildRequires:  pkgconfig(Qt5Script)
BuildRequires:  kf5-rpm-macros
BuildRequires:  extra-cmake-modules
BuildRequires:  qt5-tools
BuildRequires:  kconfig-devel
BuildRequires:  kinit-devel
BuildRequires:  kcmutils-devel
BuildRequires:  khtml-devel
BuildRequires:  kdelibs4support-devel
BuildRequires:  kdoctools-devel
BuildRequires:  desktop-file-utils

%description
An advanced editor component which is used in numerous KDE applications
requiring a text editing component.


%prep
%setup -q -n %{name}-%{version}

# >> setup
# << setup

%build
# >> build pre
%kf5_make
# << build pre



# >> build post
# << build post

%install
rm -rf %{buildroot}
# >> install pre
%kf5_make_install
# << install pre

# >> install post
# << install post

desktop-file-install --delete-original       \
  --dir %{buildroot}%{_datadir}/applications             \
   %{buildroot}%{_datadir}/applications/*.desktop

%files
%defattr(-,root,root,-)
%doc README.htdig README.metadata COPYING
%{_kf5_bindir}/khelpcenter
%{_kf5_libdir}/libexec/khc_indexbuilder
%{_kf5_libdir}/libexec/khc_htdig.pl
%{_kf5_libdir}/libexec/khc_htsearch.pl
%{_kf5_libdir}/libexec/khc_mansearch.pl
%{_kf5_libdir}/libexec/khc_docbookdig.pl
%{_kf5_libdir}/libkdeinit5_khelpcenter.so
%{_kf5_sharedir}/khelpcenter
%{_kf5_sharedir}/kxmlgui5/khelpcenter/*
%{_datadir}/applications/Help.desktop
%{_datadir}/config.kcfg/khelpcenter.kcfg
%{_kf5_servicesdir}/*
%{_kf5_dbusinterfacesdir}/org.kde.khelpcenter.kcmhelpcenter.xml
%{_kf5_htmldir}/en/khelpcenter
%{_kf5_htmldir}/en/fundamentals
%{_kf5_htmldir}/en/onlinehelp
# >> files
# << files
