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

// Qt
// this is needed to avoid typedef clash with X11
#ifndef QT_CLEAN_NAMESPACE
#  define QT_CLEAN_NAMESPACE
#endif
#include <qwidget.h>

// KThinkBat
#include "idlehandler.h"

/* needed for lXext C library linkage */
extern "C" {
    #include <X11/Xproto.h>
    #include <X11/extensions/dpms.h>
    #include <X11/extensions/scrnsaver.h>
}

IdleHandler::IdleHandler(Display* display)
: m_display(display)
{
    checked = m_display != NULL;

    int dummy = 0;
    checked &= XScreenSaverQueryExtension(m_display, &dummy, &dummy);
}

IdleHandler::~IdleHandler() {
    m_display = NULL;
}
