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
#include <qpopupmenu.h>

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
, powerPosID( 0 ) {
    // Get the current application configuration handle
    ksConfig = config();

    wastePosBelow = ksConfig->readBoolEntry( "PowerMeterBelowGauge", wastePosBelow );

    // Timer der die Aktualisierung von ACPI-Werten und deren Anzeige steuert
    timeout();
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
    timer->start(intervall);

    QPopupMenu* popupMenu = new QPopupMenu();
    powerPosID = popupMenu->insertItem( tr("Power Meter below Gauge"), this, SLOT(setPowerMeterPosition()) );
    popupMenu->setItemEnabled( powerPosID, wastePosBelow );

    setCustomMenu( popupMenu );
}


KThinkBat::~KThinkBat() {
    timer->stop();
    delete timer;
    timer = NULL;
}

void KThinkBat::setPowerMeterPosition() {
    if( customMenu() ) {
        wastePosBelow = customMenu()->isItemEnabled( powerPosID );
    }
    // force an update, as we have a new layout
    update();
}

void KThinkBat::about() {
    KMessageBox::information(0, i18n("A KDE panel applet to display the current laptop battery status.\n\nCopyrigth (c) 2005-2006 Tobias Roeser\nDistributed under the terms of the GNU General Public License v2"));
    timeout();
}


void KThinkBat::help() {
    KMessageBox::information(0, i18n("A KDE panel applet to display the current laptop battery status.\n\nThere is no real help box at the moment"));
}


void KThinkBat::preferences() {
    KMessageBox::information(0, i18n("A KDE panel applet to display the current laptop battery status."));
}

int KThinkBat::widthForHeight(int height) const {
    return neededSize.width();
}

int KThinkBat::heightForWidth(int width) const {
    return neededSize.height();
}

void KThinkBat::resizeEvent(QResizeEvent *e) {}

void KThinkBat::paintEvent(QPaintEvent* event) {
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
        wastePos = QSize( border.width(), border.height() + gaugeSize.height() + 12 );
        // wastePos = QSize( border.width(), ( 2 * border.height() ) + gaugeSize.height() );
    } else {
        // Verbrauchsanzeige rechts von der Gauge
        wastePos = QSize( ( 3 * border.width() ) + gaugeSize.width(), border.height() );
        // wastePos = QSize( ( 2 * border.width() ) + gaugeSize.width(), border.height() );
    }

    // Power consumption: For correct rounding we add 500 mW (resp. 50 mA)
    if( "W" == powerUnit ) {
        // aktuellen Verbrauch in W
        // void QPainter::drawText ( int x, int y, int w, int h, int flags, const QString &, int len = -1, QRect * br = 0, QTextParag ** internal = 0 )
        //         powerTextExtend = painter.boundingRect( wastePos.width(), wastePos.height(), 1, 1
        //                                               , Qt::AlignLeft | Qt::AlignTop
        //                                               , QString().number((int) (curPower + 500)/1000) + " " + powerUnit );
        //         painter.drawText( powerTextExtend.left(), powerTextExtend.top(), powerTextExtend.width(), powerTextExtend.height()
        //                         , Qt::AlignLeft | Qt::AlignTop
        //                         , QString().number((int) (curPower + 500)/1000) + " " + powerUnit );

        painter.drawText( wastePos.width(), wastePos.height(), QString().number((int) (curPower + 500)/1000) + " " + powerUnit );

    } else {
        // aktuellen Verbrauch in A (bei Asus-Laptops) anzeigen
        painter.drawText( wastePos.width(), wastePos.height(), QString().number((float) (((int) curPower + 50)/100) / 10 )  + " " + powerUnit );
        //         powerTextExtend = painter.boundingRect( wastePos.width(), wastePos.height(), 1, gaugeSize.height()
        //                         , Qt::AlignLeft | Qt::AlignVCenter
        //                         , QString().number((float) (((int) curPower + 50)/100) / 10 ) + " " + powerUnit );
        //         painter.drawText( powerTextExtend.left(), powerTextExtend.top(), powerTextExtend.width(), powerTextExtend.height()
        //                         , Qt::AlignLeft | Qt::AlignTop
        //                         , QString().number((int) (curPower + 500)/1000) + " " + powerUnit );
    }

    // recalculate the needed Size
    if( wastePosBelow ) {
        neededSize = QSize( (2 * border.width() ) + gaugeSize.width()
                            , wastePos.height() + powerTextExtend.height() + border.height() );
    } else {
        neededSize = QSize( wastePos.width() + powerTextExtend.width() + border.width()
                            , ( 3 * border.height() ) + gaugeSize.height() );
    }

    //-------------------------------------------------------------------------
    painter.end();

    bitBlt(this, 0, 0, &pixmap);

    // full extend is
    //   width: (3 x offset) + gaugeFill + gHalfDot ( + wasteSite )
    //   height: (3 x offset) + gaugeFill ( + wasteSite )
}


void KThinkBat::timeout() {
    // 1. First try TP SMAPI on BAT0
    // 2. If that fails try ACPI /proc interface for BAT0
    bool battery1 = batInfo1.parseSysfsTP() || batInfo1.parseProcACPI();

    // 3. Now try BAT1, first TP SMAPI agian
    // 4. And, if that failed, try ACPi /proc interface for BAT1
    bool battery2 = batInfo2.parseSysfsTP() || batInfo2.parseProcACPI();

    if( battery1 && battery2 ) {
        // we have tho batteries
        float lastFuell = batInfo1.getLastFuell() + batInfo2.getLastFuell();
        float curFuell = batInfo1.getCurFuell() + batInfo2.getCurFuell();
        if( curFuell >= 0 && lastFuell > 0 ) {
            gauge1.setPercentValue( (int) (( 100.0 / lastFuell ) * curFuell ) );
        } else {
            gauge1.setPercentValue( -1 );
        }
        gauge1.setColors( QColor( batInfo1.getCurFuell() <= ( batInfo1.getCriticalFuell() + batInfo2.getCriticalFuell() ) ? "red" : "green")
                          , QColor( batInfo1.isOnline() ? "yellow" : "gray" ) );
        powerUnit = batInfo1.getPowerUnit();
        curPower = batInfo1.getPowerConsumption() + batInfo2.getPowerConsumption();

    } else if( battery1 ) {
        // we have just battery one
        gauge1.setPercentValue( (int) batInfo1.getChargeLevel() );
        gauge1.setColors( QColor( batInfo1.getCurFuell() <= batInfo1.getCriticalFuell() ? "red" : "green")
                          , QColor( batInfo1.isOnline() ? "yellow" : "gray" ) );
        powerUnit = batInfo1.getPowerUnit();
        curPower = batInfo1.getPowerConsumption();
    } else if( battery2 ) {
        // we have just battery two
        gauge1.setPercentValue( (int) batInfo2.getChargeLevel() );
        gauge1.setColors( QColor( batInfo2.getCurFuell() <= batInfo2.getCriticalFuell() ? "red" : "green")
                          , QColor( batInfo2.isOnline() ? "yellow" : "gray" ) );
        powerUnit = batInfo2.getPowerUnit();
        curPower = batInfo2.getPowerConsumption();
    } else {
        // no battery reports good values :(
        // maybe, we should colorize the background of the applet ?
    }

    // force a repaint of the Applet
    update();
}

extern "C" {
    KPanelApplet* init( QWidget *parent, const QString& configFile) {
        KGlobal::locale()->insertCatalogue("kthinkbat");
        return new KThinkBat(configFile, KPanelApplet::Normal,
//                              KPanelApplet::About | KPanelApplet::Help | KPanelApplet::Preferences,
                             KPanelApplet::About,
                             parent, "kthinkbat");
    }
}
