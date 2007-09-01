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
#ifndef KTHINKBAT_BATGAUGE_H
#define KTHINKBAT_BATGAUGE_H

#include <qnamespace.h>
#include <qcolor.h>
#include <qsize.h>
class QString;
class QPainter;

/**
	@author Tobias Roeser <le.petit.fou@web.de>
*/
class BatGauge {

public:
    BatGauge();

    virtual ~BatGauge();

    /** Set the percentage value (between 0 and 100) to be shown. To display the value together with an unit use setPercentageValueString(). */
    void setPercentValue( int value );

    /** Sets the percentage value (between 0 and 100) and the string to be shown into the gauge. This way, you can override the value or can display string completely different than the value. If you just want set and display the value, you can use the setPercentageValue(). */
    void setPercentValueString( int value, QString string );

    /** Sets the color of the battery gauge.
    @param bgColor Background color
    @param fillColor Color used for the filled area (area depends on the percentageValue)
    @param dotColor Color of the Dot (battery pin)
    */
    void setColors( QColor bgColor, QColor fillColor, QColor dotColor );

    void drawGauge( QPainter& painter, QSize gaugePos );

    /** @deprecated Please use drawGauge() in combination with setSize(). */
    void drawGauge( QPainter& painter, QSize gaugePos, QSize gaugeSize );

    QSize getSize() { return gaugeSize; }
    void setSize( QSize gaugeSize );
    void setSize( int gaugeWidth, int gaugeHeight ) { setSize( QSize( gaugeWidth, gaugeHeight) ); }

    Qt::Orientation getOrientation() { return orientation; }
    void setOrientation( Qt::Orientation orientation );

private:
    QColor fillColor;
    QColor dotColor;
    QColor bgColor;

    /** the percent value, used to determine the filled area of the battery gauge. */
    int percentValue;
    /** the string displayed inside the battery gauge (default to the percentage value itself). */
    QString percentString;

    QSize gaugeSize;
    Qt::Orientation orientation;

};

#endif
