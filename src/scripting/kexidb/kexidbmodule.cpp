/***************************************************************************
 * kexidbmodule.cpp
 * This file is part of the KDE project
 * copyright (C)2004-2005 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 ***************************************************************************/

#include "kexidbmodule.h"
#include "kexidbdrivermanager.h"

#include <kdebug.h>

using namespace Kross::KexiDB;

KexiDBModule::KexiDBModule()
    : Kross::Api::Module<KexiDBModule>("KexiDB")
{
    addChild( new KexiDBDriverManager() );
}

KexiDBModule::~KexiDBModule()
{
}


const QString KexiDBModule::getClassName() const
{
    return "Kross::KexiDB::KexiDBModule";
}

const QString KexiDBModule::getDescription() const
{
    return QString("The KexiDB object provides a wrapper for the Kexi::KexiDB "
                "library and allows using the functionality from within "
                "scripting languages.");
}

