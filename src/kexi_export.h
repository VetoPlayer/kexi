/* This file is part of the KDE project
   Copyright (C) 2003-2004 Jaroslaw Staniek <js@iidea.pl>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef _KEXI_EXPORT_H_
#define _KEXI_EXPORT_H_

#ifdef __cplusplus
# include <kdeversion.h> /* this will also include <kdelibs_export.h>, if available */
#endif 
/* KDE_EXPORT will be defined multiple times without this on kdelibs 3.3 (tested on 3.3.1) */
#include <kdemacros.h>

/* workaround for KDElibs < 3.2 on !win32 */
#ifndef KDE_EXPORT
# define KDE_EXPORT
#endif

/* TODO: #include <koffice_export.h> */

#ifdef MAKE_KEXICORE_LIB
# define KEXICORE_EXPORT KDE_EXPORT
#elif defined(KDE_MAKE_LIB)
# define KEXICORE_EXPORT KDE_IMPORT
#else
# define KEXICORE_EXPORT 
#endif

#ifdef MAKE_KEXIMAIN_LIB
# define KEXIMAIN_EXPORT KDE_EXPORT
#elif defined(KDE_MAKE_LIB)
# define KEXIMAIN_EXPORT KDE_IMPORT
#else
# define KEXIMAIN_EXPORT 
#endif

#ifdef MAKE_KEXI_DB_LIB
# define KEXI_DB_EXPORT KDE_EXPORT
#elif defined(KDE_MAKE_LIB)
# define KEXI_DB_EXPORT KDE_IMPORT
#else
# define KEXI_DB_EXPORT 
#endif

#ifdef MAKE_KEXITABLEFILTERS_LIB
# define KEXITABLEFILTERS_EXPORT  KDE_EXPORT
#elif defined(KDE_MAKE_LIB)
# define KEXITABLEFILTERS_EXPORT  KDE_IMPORT
#else
# define KEXITABLEFILTERS_EXPORT //for apps
#endif

#ifdef MAKE_KEXIDATATABLE_LIB
# define KEXIDATATABLE_EXPORT  KDE_EXPORT
#elif defined(KDE_MAKE_LIB)
# define KEXIDATATABLE_EXPORT  KDE_IMPORT
#else
# define KEXIDATATABLE_EXPORT //for apps
#endif

#ifndef KEXIEXTWIDGETS_EXPORT //tmp

#ifdef MAKE_KEXIEXTWIDGETS_LIB
# define KEXIEXTWIDGETS_EXPORT  KDE_EXPORT
#elif defined(KDE_MAKE_LIB)
# define KEXIEXTWIDGETS_EXPORT  KDE_IMPORT
#else
# define KEXIEXTWIDGETS_EXPORT //for apps
#endif

#endif

#ifdef MAKE_KFORMEDITOR_LIB
# define KFORMEDITOR_EXPORT  KDE_EXPORT
#elif defined(KDE_MAKE_LIB)
# define KFORMEDITOR_EXPORT  KDE_IMPORT
#else
# define KFORMEDITOR_EXPORT //for apps
#endif

#ifdef MAKE_KEXIPRJWIZARD_LIB
# define KEXIPRJWIZARD_EXPORT  KDE_EXPORT
#elif defined(KDE_MAKE_LIB)
# define KEXIPRJWIZARD_EXPORT  KDE_IMPORT
#else
# define KEXIPRJWIZARD_EXPORT //for apps
#endif

#ifdef MAKE_KEXIFILTER_LIB
# define KEXIFILTER_EXPORT  KDE_EXPORT
#elif defined(KDE_MAKE_LIB)
# define KEXIFILTER_EXPORT  KDE_IMPORT
#else
# define KEXIFILTER_EXPORT //for apps
#endif

#ifdef MAKE_KEXIWIDGETS_LIB
# define KEXIWIDGETS_EXPORT  KDE_EXPORT
#elif defined(KDE_MAKE_LIB)
# define KEXIWIDGETS_EXPORT  KDE_IMPORT
#else
# define KEXIWIDGETS_EXPORT //for apps
#endif

#ifdef MAKE_KEXIUUID_LIB
# define KEXIUUID_EXPORT  KDE_EXPORT
#else
# define KEXIUUID_EXPORT //for apps
#endif

#ifdef MAKE_KEXIPROPERTYEDITOR_LIB
# define KEXIPROPERTYEDITOR_EXPORT  KDE_EXPORT
#elif defined(KDE_MAKE_LIB)
# define KEXIPROPERTYEDITOR_EXPORT  KDE_IMPORT
#else
# define KEXIPROPERTYEDITOR_EXPORT //for apps
#endif

#ifdef MAKE_KEXIRELATIONSVIEW_LIB
# define KEXIRELATIONSVIEW_EXPORT  KDE_EXPORT
#elif defined(KDE_MAKE_LIB)
# define KEXIRELATIONSVIEW_EXPORT  KDE_IMPORT
#else
# define KEXIRELATIONSVIEW_EXPORT //for apps
#endif

#ifdef MAKE_KEXIMIGR_LIB
# define KEXIMIGR_EXPORT  KDE_EXPORT
#elif defined(KDE_MAKE_LIB)
# define KEXIMIGR_EXPORT  KDE_IMPORT
#else
# define KEXIMIGR_EXPORT //for apps
#endif

#ifdef MAKE_KEXIGUIUTILS_LIB
# define KEXIGUIUTILS_EXPORT KDE_EXPORT
#elif defined(KDE_MAKE_LIB)
# define KEXIGUIUTILS_EXPORT KDE_IMPORT
#else
# define KEXIGUIUTILS_EXPORT //for apps
#endif

#ifdef MAKE_KROSS_MAIN_LIB
# define KROSS_MAIN_EXPORT KDE_EXPORT
#elif defined(KDE_MAKE_LIB)
# define KROSS_MAIN_EXPORT KDE_IMPORT
#else
# define KROSS_MAIN_EXPORT //for apps
#endif

/* -- compile-time settings -- */
#if defined(Q_WS_WIN) || defined(KEXI_OPTIONS)
/* defined in a .pro file or 'KEXI_OPTIONS' env. variable */
#else


/* unfinished features visibility */
/* -- undefined because people asked why these doesn't work: --
# define KEXI_SHOW_UNIMPLEMENTED 
# define KEXI_STARTUP_SHOW_TEMPLATES 
# define KEXI_STARTUP_SHOW_RECENT*/
# define KEXI_SERVER_SUPPORT
# define KEXI_FORMS_SUPPORT
# define KEXI_REPORTS_SUPPORT
# define KEXI_NO_MIGRATION
#endif

/* additional default options */
# define KDE_CXXFLAGS
# define KEXI_NO_CTXT_HELP


#endif //KEXI_EXPORT_H
