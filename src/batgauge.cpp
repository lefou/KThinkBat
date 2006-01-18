/***************************************************************************
 *   Copyright (C) 2005-2006 by Tobias Roeser   *
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
#include "batgauge.h"

BatGauge::BatGauge()
    : fillColor( QColor( "green" ) )
    , dotColor( QColor( "gray" ) )
    , percentValue( 0 )
{
}


BatGauge::~BatGauge()
{
}

void 
BatGauge::setPercentValue( int value ) {
    percentValue = value;
}

void
BatGauge::setColors( QColor fillColor, QColor dotColor ) {
    this->fillColor = fillColor;
    this->dotColor = dotColor;
}

void
BatGauge::drawGauge( QPainter& painter, QSize gaugePos, QSize gaugeSize ) {

    // Values for Gauge and Border
    QSize offset( gaugePos.width() + 1, gaugePos.height() + 1 );
    // size of the dot
    QSize gHalfDot(4, 4);
    // substract the frame and the dot
    QSize gaugeFill(gaugeSize.width() - gHalfDot.width() - 2, gaugeSize.height() - 2 );

    // Rahmen
    QPointArray border(9);
    border.putPoints( 0, 9
        , 0, 0
        , gaugeFill.width() + 2, 0
        , gaugeFill.width() + 2, (gaugeFill.height() / 2) - gHalfDot.height()
        , gaugeFill.width() + 2 + gHalfDot.width(), (gaugeFill.height() / 2) - gHalfDot.height()
        , gaugeFill.width() + 2 + gHalfDot.width(), (gaugeFill.height() / 2) + gHalfDot.height()
        , gaugeFill.width() + 2, (gaugeFill.height() / 2) + gHalfDot.height()
        , gaugeFill.width() + 2, gaugeFill.height()
        , 0 , gaugeFill.height()
        , 0 , 0);
    border.translate(offset.width() - 1, offset.height() - 1);

    //-------------------------------------------------------------------------
    // Paint Gauge
    painter.fillRect(offset.width(), offset.height(), gaugeFill.width() + 2, gaugeFill.height(), QColor( "gray"));

    int xFill = ( percentValue > 0 ? percentValue * gaugeFill.width() / 100 : 0);
    painter.fillRect(offset.width(), offset.height(), xFill, gaugeFill.height(), fillColor );
    // Plus-Pol zeichnen
    painter.fillRect( offset.width() + gaugeFill.width() + 2, offset.height() + (gaugeFill.height() / 2) - gHalfDot.height(), gHalfDot.width(), gHalfDot.height() * 2, dotColor );

    // Paint Border
    painter.drawPolyline(border);

    // Prozent-Anzeige
    QString percentageString = ( percentValue >= 0) ? QString().number( percentValue ) : "?" ;
    painter.drawText( offset.width() + 12, offset.height() + gaugeFill.height() - 5, percentageString );
}
