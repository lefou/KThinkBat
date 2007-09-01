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
#ifndef KTHINKBAT_BATTOOLTIP_H
#define KTHINKBAT_BATTOOLTIP_H

// Qt
class QLabel;
class QString;

// KDE
#include <kpassivepopup.h>

/**
	@author Tobias Roeser <le.petit.fou@web.de>
*/
class BatToolTip : public KPassivePopup {
  Q_OBJECT

public:
    BatToolTip( QWidget* parent = 0, const char* name = 0 );
    virtual ~BatToolTip();

public slots:
    /** Sets the tooltip to @param text */
    void setText( const QString &text);

private:
    QLabel* text;

};

#endif
