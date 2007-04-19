/***************************************************************************
 *   Copyright (C) 2005-2007 by Tobias Roeser   *
 *   le.petit.fou@web.de   *
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
#include "battooltip.h"

#include <qvbox.h>


BatToolTip::BatToolTip( QWidget* parent, const char* name )
: KPassivePopup( parent, name )
, text( NULL )
{
    setTimeout( 15 * 1000 );

    QHBox* hbox = new QHBox( this );
    hbox->setSpacing( 10 );

    QVBox* vbox = new QVBox( hbox );
    vbox->setSpacing( 5 );
    (void) new QLabel( "<qt><strong>KThinkBat</strong></qt>", vbox );
    text = new QLabel( vbox );

    setView( hbox );
}


BatToolTip::~BatToolTip()
{
    delete text; text = NULL;
}

void
BatToolTip::setText( const QString& text ) {
    if (this->text) {
        this->text->setText(text);
        layout();
    }
}
