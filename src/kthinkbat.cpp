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
#include <kaboutapplication.h>

#include "kthinkbat.h"

extern "C" {
    KPanelApplet* init( QWidget *parent, const QString& configFile) {
        KGlobal::locale()->insertCatalogue("kthinkbat");
        return new KThinkBat(configFile, KPanelApplet::Normal, 0, parent, "kthinkbat");
//                              KPanelApplet::About | KPanelApplet::Help | KPanelApplet::Preferences,
    }
}

KThinkBat::KThinkBat(const QString& configFile, Type type, int actions, QWidget *parent, const char *name)
: KPanelApplet(configFile, type, actions, parent, name)
, config( NULL )
, gaugeSize( QSize( 46, 20 ) )
, border( QSize( 3, 3 ) )
, padding( QSize( 5, 2 ) )
, timer(NULL)
, wastePosBelow( true )
, batInfo1( BatInfo( 1 ) )
, batInfo2( BatInfo( 2 ) )
, neededSize( QSize( 52, 40) )
, powerPosID( 0 ) {

    config = new KThinkBatConfig( sharedConfig() );
    assert( config );

    wastePosBelow = config->powerMeterBelowGauge();

    // Timer der die Aktualisierung von ACPI/SMAPI-Werten und deren Anzeige veranlasst.
    timeout();
    timer = new QTimer(this);
    assert( timer );
    connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
    timer->start( config->updateIntervalMsek() );

    // Create a popup menu to show KThinkBats specific options.
    contextMenu = new KPopupMenu();
    assert( contextMenu );
    contextMenu->setCheckable( true );
    contextMenu->insertTitle( i18n("KThinkBat %1").arg(VERSION) );
    contextMenu->insertItem( i18n("About KThinkBat"), this, SLOT(about()) );
    powerPosID = contextMenu->insertItem( i18n("Power Meter below Gauge"), this, SLOT(slotPowerMeterPosition()) );
    contextMenu->insertItem( i18n("Power Meter Color..."), this, SLOT(slotPowerMeterColor()) );
    contextMenu->setItemChecked( powerPosID, wastePosBelow );

    // KPanelApplet takes ownership of this menu, so we don't have to delete it.
    setCustomMenu( contextMenu );
}

KThinkBat::~KThinkBat() {

    // Save Config values
//     ksConfig->writeEntry( "PowerMeterBelowGauge", wastePosBelow );
//     ksConfig->sync();
    config->writeConfig();

    timer->stop();
    delete timer; timer = NULL;

    delete contextMenu; contextMenu = NULL;
}

void 
KThinkBat::slotPowerMeterPosition() {

    wastePosBelow = ! wastePosBelow;
    contextMenu->setItemChecked( powerPosID, wastePosBelow );
    config->setPowerMeterBelowGauge( wastePosBelow );

    // force an update, as we have a new layout
    update();
}

void 
KThinkBat::slotPowerMeterColor() {

    QColor myColor = config->powerMeterColor();
    if ( KColorDialog::Accepted == KColorDialog::getColor( myColor ) ) {
        config->setPowerMeterColor( myColor );
        update();
    }
}

void 
KThinkBat::about() {

    KAboutData aboutData( "KThinkBat", "KThinkBat", VERSION,
                          I18N_NOOP("A KDE panel applet to display the current laptop battery status."),
                          KAboutData::License_GPL_V2, 
                          "(c) 2005-2006, Tobias Roeser",
                          "", 
                          "https://lepetitfou.dyndns.org/KThinkBat",
                          "le.petit.fou@web.de" );
    
    aboutData.addAuthor( "Tobias Roeser", "", "le.petitfou@web.de",
                         "https://lepetitfou.dyndns.org/KThinkBat" );

    KAboutApplication about( &aboutData, this, NULL, 0 );
//     about.setIcon( KGlobal::instance()->iconLoader()->iconPath( "kthinkbat", -KIcon::SizeLarge ) );
    about.exec();
}


void 
KThinkBat::help() {
    KMessageBox::information(0, i18n("A KDE panel applet to display the current laptop battery status.\n\nThere is no real help box at the moment"));
}


void 
KThinkBat::preferences() {
    KMessageBox::information(0, i18n("A KDE panel applet to display the current laptop battery status."));
}

int 
KThinkBat::widthForHeight(int height) const {
    return neededSize.width();
}

int 
KThinkBat::heightForWidth(int width) const {
    return neededSize.height();
}

void 
KThinkBat::resizeEvent(QResizeEvent *e) {}

void 
KThinkBat::paintEvent(QPaintEvent* event) {
    // TODO Gauge auslagern als Funktion, später als extra Control

    QPixmap pixmap(width(), height());
    pixmap.fill(this, 0, 0);
    QPainter painter(&pixmap);

    gauge1.drawGauge( painter, border, gaugeSize );

    //-------------------------------------------------------------------------
    // Position for power consumtion display
    QSize wastePos;
    // Power consumption label: For correct rounding we add 500 mW (resp. 50 mA)
    QString powerLabel = ("W" == powerUnit) ? QString().number((int) (curPower + 500)/1000) + " " + powerUnit : QString().number((float) (((int) curPower + 50)/100) / 10 )  + " " + powerUnit;
//     powerLabel += batInfo1.isInstalled() ? "[1|" : "[0|";
//     powerLabel += batInfo2.isInstalled() ? "1]" : "0]";

    // Needed Space for Power Consumption Label
    QRect powerTextExtend = painter.boundingRect( 0, 0, 1, 1,
                                                  Qt::AlignLeft | Qt::AlignTop,
                                                  powerLabel );

    // left upper corner of power consumption label
    if( wastePosBelow ) {
        // Verbrauchsanzeige unterhalb der Gauge
        //         wastePos = QSize( border.width(), border.height() + gaugeSize.height() + 12 );
        wastePos = QSize( border.width(), border.height() + padding.height() + gaugeSize.height() );
        neededSize = QSize( (2 * border.width() ) + gaugeSize.width()
                            , wastePos.height() + powerTextExtend.height() + border.height() );
    } else {
        // Verbrauchsanzeige rechts von der Gauge
        //         wastePos = QSize( ( 3 * border.width() ) + gaugeSize.width(), border.height() );
        wastePos = QSize( border.width() + padding.width() + gaugeSize.width(), border.height() + ((gaugeSize.height() - powerTextExtend.height()) / 2 ) );
        neededSize = QSize( wastePos.width() + powerTextExtend.width() + border.width()
                            , ( 2 * border.height() ) + gaugeSize.height() );



//     painter.drawText( offset.width() + ( (gaugeFill.width() - reqTextSize.width()) / 2 )
//                     , offset.height() + ( (gaugeFill.height() - reqTextSize.height()) / 2 )


    }

    QPen origPen = painter.pen();
    painter.setPen( config->powerMeterColor() );

    // Draw the Power Consumption at position @c wastePos.
    painter.drawText( wastePos.width(), wastePos.height(), 
                          powerTextExtend.width(), powerTextExtend.height(),
                          Qt::AlignTop | Qt::AlignLeft, 
                          powerLabel );

    painter.setPen( origPen );
    //-------------------------------------------------------------------------
    painter.end();

    bitBlt(this, 0, 0, &pixmap);

    // full extend is
    //   width: (3 x offset) + gaugeFill + gHalfDot ( + wasteSite )
    //   height: (3 x offset) + gaugeFill ( + wasteSite )
}


void 
KThinkBat::timeout() {

    float lastFuel = 0;
    float curFuel = 0;
    float critFuel = 0;
    curPower = 0;
    bool batOnline = true;

    // 1. First try TP SMAPI on BAT0
    // 2. If that fails try ACPI /proc interface for BAT0
    bool battery1 = batInfo1.parseSysfsTP() || batInfo1.parseProcACPI();
    if( battery1 ) {
        lastFuel += batInfo1.getLastFuel();
        curFuel += batInfo1.getCurFuel();
        critFuel += batInfo1.getCriticalFuel();
        batOnline = batInfo1.isOnline();
        curPower += batInfo1.getPowerConsumption();
        powerUnit = batInfo1.getPowerUnit();
    }

    // 3. Now try BAT1, first TP SMAPI agian
    // 4. And, if that failed, try ACPi /proc interface for BAT1
    bool battery2 = batInfo2.parseSysfsTP() || batInfo2.parseProcACPI();
    if( battery2 ) {
        lastFuel += batInfo2.getLastFuel();
        curFuel += batInfo2.getCurFuel();
        critFuel += batInfo2.getCriticalFuel();
        batOnline = batOnline || batInfo2.isOnline();
        curPower += batInfo2.getPowerConsumption();
        powerUnit = batInfo2.getPowerUnit();
    }

    if( curFuel >= 0 && lastFuel > 0 ) {
        gauge1.setPercentValue( (int) (( 100.0 / lastFuel ) * curFuel )  );
    } else {
        gauge1.setPercentValue( -1 );
//         gauge1.setPercentValueString( -1, QString::number(lastFuel) + ":" + QString::number(curFuel) );
    }
    gauge1.setColors( QColor( config->batBackgroundColor() ),
                      QColor( curFuel <= critFuel ? config->batCriticalColor() : config->batChargedColor() ),
                      QColor( batOnline ? config->batDotOnlineColor() : config->batBackgroundColor() ) );

    // force a repaint of the Applet
    update();
}

