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
#include "batgauge.h"
#include "kthinkbatconfig.h"

BatGauge::BatGauge()
: fillColor( "green" )
, dotColor( "gray" )
, bgColor( "gray" )
, percentValue( 0 )
, percentString( "" ) 
, gaugeSize( 20, 10 ) 
, orientation( Qt::Horizontal ) {
}

BatGauge::~BatGauge() {
}

void
BatGauge::setPercentValue( int value ) {
    percentValue = value;
    percentString = ( value >= 0) ? QString().number( value ) : "?" ;
}

void
BatGauge::setPercentValueString( int value, QString string ) {
    percentValue = value;
    percentString = string;
}

void
BatGauge::setColors( QColor bgColor, QColor fillColor, QColor dotColor ) {
    this->bgColor = bgColor;
    this->fillColor = fillColor;
    this->dotColor = dotColor;
}

void
BatGauge::drawGauge( QPainter& painter, QSize gaugePos, QSize gaugeSize ) {
    setSize( gaugeSize );
    drawGauge( painter, gaugePos );
}

void
BatGauge::drawGauge( QPainter& painter, QSize gaugePos ) {

    QSize gaugeSize = getSize();

    // Values for Gauge and Border
    QSize offset( gaugePos.width() + 1, gaugePos.height() + 1 );
    // size of the dot
    QSize gHalfDot( KThinkBatConfig::gaugeDotWidth(), (int) ((KThinkBatConfig::gaugeDotHeight() / 2) + 0.5) );

    painter.save();

    if( Qt::Vertical == getOrientation() ) {
        // rotate the painter 90 degree counterclockwise
        painter.rotate( -90 );
        gaugeSize.transpose();
        gHalfDot.transpose();
        offset = QSize( gaugePos.width() - 3 - gaugeSize.width(), gaugePos.height() + 1 );
    }

    // substract the frame and the dot
    QSize gaugeFill(gaugeSize.width() - gHalfDot.width() - 2, gaugeSize.height() - 2 );


    // Rahmen
    QPointArray border( 9 );
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
    border.translate( offset.width() - 1, offset.height() - 1 );

    //-------------------------------------------------------------------------
    // Paint Gauge
    painter.fillRect( offset.width(), offset.height(), gaugeFill.width() + 2, gaugeFill.height(), bgColor );

    int xFill = ( percentValue > 0 ? percentValue * gaugeFill.width() / 100 : 0);
    painter.fillRect(offset.width(), offset.height(), xFill, gaugeFill.height(), fillColor );
    // Plus-Pol zeichnen
    painter.fillRect( offset.width() + gaugeFill.width() + 2, offset.height() + (gaugeFill.height() / 2) - gHalfDot.height(), gHalfDot.width(), gHalfDot.height() * 2, dotColor );

    // Paint Border
    painter.drawPolyline( border );

    // Calculate, how much space is needed by the Text string
    QRect reqTextSize = painter.boundingRect( 1,1,1,1, Qt::AlignHCenter | Qt::AlignVCenter, percentString );

    // Draw text
    painter.drawText( offset.width() + ( (gaugeFill.width() - reqTextSize.width()) / 2 )
                      , offset.height() + ( (gaugeFill.height() - reqTextSize.height()) / 2 )
                      , reqTextSize.width(), reqTextSize.height()
                      , Qt::AlignTop | Qt::AlignLeft
                      , percentString );

    painter.restore();
}

void
BatGauge::setSize( QSize gaugeSize ) {
    this->gaugeSize = gaugeSize;
}

void
BatGauge::setOrientation( Qt::Orientation orientation ) {
    this->orientation = orientation;
}
