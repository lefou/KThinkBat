/***************************************************************************
 *   Copyright (C) 2005 by Tobias Roeser   *
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
    , timer(NULL)
    , wastePosBelow( true )
    , batInfo1( BatInfo() )
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
    KMessageBox::information(0, i18n("A KDE panel applet to display the current laptop battery status.\n\n(c) Copyrigth 2005 Tobias Roeser\nDistributed under the terms of the GNU General Public License v2"));
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
    return 55;
}

int KThinkBat::heightForWidth(int width) const
{
    return 30;
}

void KThinkBat::resizeEvent(QResizeEvent *e)
{
}

void KThinkBat::paintEvent(QPaintEvent* event)
{
    // TODO Gauge auslagern als Funktion, später als extra Control

    // Values for Gauge and Border
    static QSize gaugeFill(40, 18);
    static QSize gHalfDot(4, 4);
    static QSize offset(4, 4);

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

    QPixmap pixmap(width(), height());
    pixmap.fill(this, 0, 0);
    QPainter painter(&pixmap);

    //-------------------------------------------------------------------------
    // Paint Gauge
    painter.fillRect(offset.width(), offset.height(), gaugeFill.width() + 2, gaugeFill.height(), QColor( "gray"));
    // old: int showValue = (batValue <0 || batValue > 100 ) ? 0 : batValue;
    int batValue = batInfo1.getChargeLevel();
    int xFill = (batValue>0 ? batValue * gaugeFill.width() / 100 : 0);
    // TODO criticalFuell comes from batInfo, but it could also be set by user ???
    painter.fillRect(offset.width(), offset.height(), xFill, gaugeFill.height(), QColor( batInfo1.getCurFuell() <= batInfo1.getCriticalFuell() ? "red" : "green"));
    // Plus-Pol zeichnen
    painter.fillRect( offset.width() + gaugeFill.width() + 2, offset.height() + (gaugeFill.height() / 2) - gHalfDot.height(), gHalfDot.width(), gHalfDot.height() * 2, QColor( batInfo1.isOnline() ? "yellow" : "gray" ));

    // Paint Border
    painter.drawPolyline(border);

    // Prozent-Anzeige
    QString percentageString = (batValue >= 0) ? QString().number(batValue) : "?" ;
    painter.drawText( offset.width() + 12, offset.height() + gaugeFill.height() - 5, percentageString );

    //-------------------------------------------------------------------------
    // Position für Verbrauchsanzeige
    QSize wastePos;
    if( wastePosBelow ) {
        // Verbrauchsanzeige unterhalb der Gauge
        wastePos = QSize( offset.width(), offset.height() + gaugeFill.height() + 12 );
    }
    else {
        // Verbrauchsanzeige rechts von der Gauge
        wastePos = QSize( offset.width() + gaugeFill.width() + gHalfDot.width() + 12, offset.height() );
    }
    
    // Power consumption: For correct rounding we add 500 mW (resp. 50 mA)
    if( "W" == batInfo1.getPowerUnit() ) {
        // aktuellen Verbrauch in W 
        painter.drawText( wastePos.width(), wastePos.height(), QString().number((int) (batInfo1.getPowerConsumption() + 500)/1000) + " " + batInfo1.getPowerUnit() );
    }
    else {
        // aktuellen Verbrauch in A (bei Asus-Laptops) anzeigen
        painter.drawText( wastePos.width(), wastePos.height(), QString().number((float) (((int) batInfo1.getPowerConsumption() + 50)/100) / 10 )  + " " + batInfo1.getPowerUnit() );
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
    bool tpGood = batInfo1.parseSysfsTP();

    if ( ! tpGood ) {
        // Ermittle die Werte von /proc/acpi/BAT0
        batInfo1.parseProcACPI();
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
