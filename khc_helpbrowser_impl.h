/*
 *  khc_helpbrowser_impl.h - part of the KDE Help Center
 *
 *  Copyright (c) 1999 Matthias Elter (me@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _khc_helpbrowser_impl_h_
#define _khc_helpbrowser_impl_h_

#include "khelpcenter.h"

class khcMainWindow;

class HelpBrowser_Impl : virtual public KHelpCenter::HelpBrowser_skel
{

 public:
    HelpBrowser_Impl();
    virtual ~HelpBrowser_Impl();

    void open(const char *url);

 private:
    khcMainWindow *khb;
};

class HelpBrowserFactory_Impl : virtual public KHelpCenter::HelpBrowserFactory_skel
{
public:
  HelpBrowserFactory_Impl() {}

  HelpBrowserFactory_Impl(const CORBA::ORB::ObjectTag &tag)
   : KHelpCenter::HelpBrowserFactory_skel(tag) {}
  
  HelpBrowserFactory_Impl(CORBA::Object_ptr obj)
   : KHelpCenter::HelpBrowserFactory_skel(obj) {}

  virtual KHelpCenter::HelpBrowser_ptr create();  
};

#endif
