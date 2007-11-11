/***************************************************************************
 *   Copyright (C) 2005-2007 by Tobias Roeser   *
 *   le.petit.fou@web.de   *
 *   $Id$   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef KTHINKBAT_DEBUG_H
#define KTHINKBAT_DEBUG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define USE_LOG 1
#if USE_LOG
#  include <kdebug.h>
#  define debug(x) \
    kdDebug(1) << "DEBUG [" << __FILE__ << ":" << __LINE__ << "] " << (x) << endl;
#  define trace(x) \
    kdDebug(1) << "TRACE [" << __FILE__ << ":" << __LINE__ << "] " << (x) << endl;
#else
#  define debug(x) // debug(x);
#  define trace(x) // trace(x);
#endif


#endif // KTHINKBAT_DEBUG_H
