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


#include <qlcdnumber.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <qpainter.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qtimer.h>

#include "kthinkbat.h"


KThinkBat::KThinkBat(const QString& configFile, Type type, int actions, QWidget *parent, const char *name)
    : KPanelApplet(configFile, type, actions, parent, name)
    , intervall(3000)
    , borderColor("black")
    , emptyColor("grey")
    , chargedColor("green")
    , gaugeSize( QSize( 46, 20 ) )
    , border( QSize( 3, 3 ) ) 
    , timer(NULL)
    , wastePosBelow( true )
    , batInfo1( BatInfo() )
    , batInfo2( BatInfo( 2 ) )
    , neededSize( QSize( 52, 40) )
{
    // Get the current application configuration handle
    ksConfig = config();

    wastePosBelow = ksConfig->readBoolEntry( "ConsumptionPositionBelowGauge", wastePosBelow );

    // Timer der die Aktualisierung von ACPI-Werten und deren Anzeige steuert
    timeout();
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
    timer->start(intervall);
}


KThinkBat::~KThinkBat()
{
    timer->stop();
    delete timer;
    timer = NULL;
}


void KThinkBat::about()
{
    KMessageBox::information(0, i18n("A KDE panel applet to display the current laptop battery status.\n\nCopyrigth (c) 2005-2006 Tobias Roeser\nDistributed under the terms of the GNU General Public License v2"));
    timeout();
}


void KThinkBat::help()
{
    KMessageBox::information(0, i18n("A KDE panel applet to display the current laptop battery status.\n\nThere is no real help box at the moment"));
}


void KThinkBat::preferences()
{
    KMessageBox::information(0, i18n("A KDE panel applet to display the current laptop battery status."));
}

int KThinkBat::widthForHeight(int height) const
{
    return neededSize.width();
}

int KThinkBat::heightForWidth(int width) const
{
    return neededSize.height();
}

void KThinkBat::resizeEvent(QResizeEvent *e)
{
}

void KThinkBat::paintEvent(QPaintEvent* event)
{
    // TODO Gauge auslagern als Funktion, später als extra Control

    QPixmap pixmap(width(), height());
    pixmap.fill(this, 0, 0);
    QPainter painter(&pixmap);

    gauge1.drawGauge( painter, border, gaugeSize );

    //-------------------------------------------------------------------------
    // Position for power consumtion display
    QSize wastePos;
    QRect powerTextExtend( 0, 0, 0, 0 );

    if( wastePosBelow ) {
        // Verbrauchsanzeige unterhalb der Gauge
        wastePos = QSize( border.width(), ( 2 * border.height() ) + gaugeSize.height() );
    }
    else {
        // Verbrauchsanzeige rechts von der Gauge
        wastePos = QSize( ( 2 * border.width() ) + gaugeSize.width(), border.height() );
    }
    
    // Power consumption: For correct rounding we add 500 mW (resp. 50 mA)
    if( "W" == powerUnit ) {
        // aktuellen Verbrauch in W 
        // void QPainter::drawText ( int x, int y, int w, int h, int flags, const QString &, int len = -1, QRect * br = 0, QTextParag ** internal = 0 )
        painter.drawText( wastePos.width(), wastePos.height()
                        , 1, 1
                        , Qt::AlignLeft | Qt::AlignTop
                        , QString().number((int) (curPower + 500)/1000) + " " + powerUnit
                        , -1,  &powerTextExtend );
        // painter.drawText( wastePos.width(), wastePos.height(), QString().number((int) (curPower + 500)/1000) + " " + powerUnit );

    }
    else {
        // aktuellen Verbrauch in A (bei Asus-Laptops) anzeigen
        // painter.drawText( wastePos.width(), wastePos.height(), QString().number((float) (((int) curPower + 50)/100) / 10 )  + " " + powerUnit );
        painter.drawText( wastePos.width(), wastePos.height()
                        , 1, gaugeSize.height()
                        , Qt::AlignLeft | Qt::AlignVCenter
                        , QString().number((float) (((int) curPower + 50)/100) / 10 ) + " " + powerUnit 
                        , -1, &powerTextExtend );
    }

    // recalculate the needed Size
    if( wastePosBelow ) {
        neededSize.setWidth( (2 * border.width() ) + gaugeSize.width() );
        neededSize.setHeight( wastePos.height() + powerTextExtend.height() + border.height() );
    }
    else {
        neededSize.setWidth( wastePos.width() + powerTextExtend.width() + border.width() );
        neededSize.setHeight( ( 3 * border.height() ) + gaugeSize.height() );
    }

    //-------------------------------------------------------------------------
    painter.end();

    bitBlt(this, 0, 0, &pixmap);

    // full extend is
    //   width: (3 x offset) + gaugeFill + gHalfDot ( + wasteSite )
    //   height: (3 x offset) + gaugeFill ( + wasteSite )
}


void KThinkBat::timeout()
{
    // Ermittle die Werte von /sys/devices/platform/smapi/
    bool tp1Good = batInfo1.parseSysfsTP();
    bool acpi1Good = false;

    if ( ! tp1Good ) {
        // Ermittle die Werte von /proc/acpi/BAT0
        acpi1Good = batInfo1.parseProcACPI();
    }

    // Zweite Battery
    // Ermittle die Werte von /sys/devices/platform/smapi/
    bool tp2Good = batInfo2.parseSysfsTP();
    bool acpi2Good = false;

    if ( ! tp2Good ) {
        // Ermittle die Werte von /proc/acpi/BAT1
        acpi2Good = batInfo2.parseProcACPI();
    }

    if( ( tp1Good || acpi1Good ) && ( tp2Good || acpi2Good ) ) {
        // we have tho batteries
        float lastFuell = batInfo1.getLastFuell() + batInfo2.getLastFuell();
        float curFuell = batInfo1.getCurFuell() + batInfo2.getCurFuell();
        if( curFuell >= 0 && lastFuell > 0 ) {
            gauge1.setPercentValue( (int) (( 100.0 / lastFuell ) * curFuell ) );
        }
        else {
            gauge1.setPercentValue( -1 );
        }
        gauge1.setColors( QColor( batInfo1.getCurFuell() <= ( batInfo1.getCriticalFuell() + batInfo2.getCriticalFuell() ) ? "red" : "green")
                        , QColor( batInfo1.isOnline() ? "yellow" : "gray" ) );
        powerUnit = batInfo1.getPowerUnit();
        curPower = batInfo1.getPowerConsumption() + batInfo2.getPowerConsumption();

    }
    else if( tp1Good || acpi1Good ) {
        // we have just battery one
        gauge1.setPercentValue( (int) batInfo1.getChargeLevel() );
        gauge1.setColors( QColor( batInfo1.getCurFuell() <= batInfo1.getCriticalFuell() ? "red" : "green")
                        , QColor( batInfo1.isOnline() ? "yellow" : "gray" ) );
        powerUnit = batInfo1.getPowerUnit();
        curPower = batInfo1.getPowerConsumption();
    }
    else if( tp2Good || acpi2Good ) {
        // we have just battery two
        gauge1.setPercentValue( (int) batInfo2.getChargeLevel() );
        gauge1.setColors( QColor( batInfo2.getCurFuell() <= batInfo2.getCriticalFuell() ? "red" : "green")
                        , QColor( batInfo2.isOnline() ? "yellow" : "gray" ) );
        powerUnit = batInfo2.getPowerUnit();
        curPower = batInfo2.getPowerConsumption();
    }
    else {
        // no battery reports good values :(
        // maybe, we should colorize the background of the applet ?
    }

    // Aktualisierte Interface
    update();
}

extern "C"
{
    KPanelApplet* init( QWidget *parent, const QString& configFile)
    {
        KGlobal::locale()->insertCatalogue("kthinkbat");
        return new KThinkBat(configFile, KPanelApplet::Normal,
                             KPanelApplet::About | KPanelApplet::Help | KPanelApplet::Preferences,
                             parent, "kthinkbat");
    }
}
