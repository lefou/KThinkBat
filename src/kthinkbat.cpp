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

// Qt
#include <qpainter.h>
#include <qfile.h>
#include <qtimer.h>

// KDE
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kaboutapplication.h>
#include <kconfigdialog.h>

// KThinkkBat
#include "kthinkbat.h"
#include "prefs.h"

extern "C" {
    KPanelApplet* init( QWidget *parent, const QString& configFile) {
        KGlobal::locale()->insertCatalogue("kthinkbat");
        return new KThinkBat( configFile, KPanelApplet::Normal, 0, parent, "kthinkbat" );
    }
}

KThinkBat::KThinkBat(const QString& configFile, Type type, int actions, QWidget *parent, const char *name)
: KPanelApplet(configFile, type, actions, parent, name)
// , config( NULL )
, padding( QSize( 5, 2 ) )
, timer(NULL)
, batInfo1( BatInfo( 1 ) )
, batInfo2( BatInfo( 2 ) )
, powerPosID( 0 ) {

    KThinkBatConfig::instance( configFile );

//     config = new KThinkBatConfig( sharedConfig() );
//     config = KThinkBatConfig::self();
//     assert( config );

    neededSize = QSize( KThinkBatConfig::gaugeWidth() + (2* KThinkBatConfig::borderSize().width()), KThinkBatConfig::gaugeHeight() + (2* KThinkBatConfig::borderSize().width()) );

    // Create a popup menu to show KThinkBats specific options.
    contextMenu = new KPopupMenu();
    assert( contextMenu );
//     contextMenu->setCheckable( true );
    contextMenu->insertTitle( i18n("KThinkBat %1").arg(VERSION) );
    contextMenu->insertItem( i18n("About %1").arg("KThinkBat"), this, SLOT(slotAbout()) );

//     powerPosID = contextMenu->insertItem( i18n("Power Meter below Gauge"), this, SLOT(slotPowerMeterPosition()) );
//     contextMenu->setItemChecked( powerPosID, KThinkBatConfig::powerMeterBelowGauge() );

//     contextMenu->insertItem( i18n("Power Meter Color..."), this, SLOT(slotPowerMeterColor()) );

    contextMenu->insertItem( i18n("Configure %1...").arg("KThinkBat"), this, SLOT(slotPreferences()) );

    // KPanelApplet takes ownership of this menu, so we don't have to delete it.
    setCustomMenu( contextMenu );

    // Timer der die Aktualisierung von ACPI/SMAPI-Werten und deren Anzeige veranlasst.
    timeout();
    timer = new QTimer(this);
    assert( timer );
    connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
    timer->start( KThinkBatConfig::updateIntervalMsek() );
}

KThinkBat::~KThinkBat() {

    timer->stop();
    delete timer; timer = NULL;

    // Save Config values
    KThinkBatConfig::writeConfig();

    delete contextMenu; contextMenu = NULL;
}

// void 
// KThinkBat::slotPowerMeterPosition() {
// 
//     KThinkBatConfig::setPowerMeterBelowGauge( ! KThinkBatConfig::powerMeterBelowGauge() );
//     contextMenu->setItemChecked( powerPosID, KThinkBatConfig::powerMeterBelowGauge() );
// 
//     // force an update, as we have a new layout
//     update();
// }

// void 
// KThinkBat::slotPowerMeterColor() {
// 
//     QColor myColor = KThinkBatConfig::powerMeterColor();
//     if ( KColorDialog::Accepted == KColorDialog::getColor( myColor ) ) {
//         KThinkBatConfig::setPowerMeterColor( myColor );
//         update();
//     }
// }

void
KThinkBat::slotPreferences() {
    if ( KConfigDialog::showDialog( "KThinkBatSettings" ) ) {
        return;
    }

    KConfigDialog* dialog = new KConfigDialog( this, "KThinkBatSettings", KThinkBatConfig::self() );

    Prefs* prefs = new Prefs( this );
    assert( prefs );
 
    dialog->addPage( prefs, i18n("KThinkBat Preferences"), "general" ); 
 
    //User edited the configuration - update your local copies of the 
    //configuration data 
    connect( dialog, SIGNAL(settingsChanged()), this, SLOT(slotUpdateConfiguration()) ); 
     
    dialog->show();
}

void
KThinkBat::slotUpdateConfiguration() {
    timeout();
    update();
}

void 
KThinkBat::slotAbout() {

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
    // this is the needed Space
    QSize realNeededSpace = QSize( (2 * KThinkBatConfig::borderSize().width()) + KThinkBatConfig::gaugeWidth(),
                                   (2 * KThinkBatConfig::borderSize().height()) + KThinkBatConfig::gaugeHeight() );

    gauge1.drawGauge( painter, KThinkBatConfig::borderSize(), QSize( KThinkBatConfig::gaugeWidth(), KThinkBatConfig::gaugeHeight() ) );

    QSize nextSouth = QSize( KThinkBatConfig::borderSize().width(), KThinkBatConfig::borderSize().height() + padding.height() + KThinkBatConfig::gaugeHeight() );
    QSize nextEast = QSize( KThinkBatConfig::borderSize().width() + padding.width() + KThinkBatConfig::gaugeWidth(), KThinkBatConfig::borderSize().height() );


    if( ! KThinkBatConfig::summarizeBatteries() ) {
        if( KThinkBatConfig::powerMeterBelowGauge() ) {
            gauge2.drawGauge( painter, nextEast, QSize( KThinkBatConfig::gaugeWidth(), KThinkBatConfig::gaugeHeight() ) );
            realNeededSpace = QSize( realNeededSpace.width() + padding.width() + KThinkBatConfig::gaugeWidth(), realNeededSpace.height() );
        }
        else {
            gauge2.drawGauge( painter, nextSouth, QSize( KThinkBatConfig::gaugeWidth(), KThinkBatConfig::gaugeHeight() ) );
            realNeededSpace = QSize( realNeededSpace.width(), realNeededSpace.height() + padding.height()  + KThinkBatConfig::gaugeHeight() );
        }
    }

    //-------------------------------------------------------------------------
    // Position for power consumtion display
    // Power consumption label: For correct rounding we add 500 mW (resp. 50 mA)
    QString powerLabel1 = ("W" == powerUnit1) ? QString().number((int) (curPower1 + 500)/1000) + " " + powerUnit1 : QString().number((float) (((int) curPower1 + 50)/100) / 10 )  + " " + powerUnit1;
    // Needed Space for Power Consumption Label
    QRect powerTextExtend1 = painter.boundingRect( 0, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop, powerLabel1 );

    QString powerLabel2("");
    QRect powerTextExtend2(0,0,0,0);
    QSize maxPowerExtend = QSize( powerTextExtend1.width(), powerTextExtend1.height() );
    
    if( ! KThinkBatConfig::summarizeBatteries() ) {
        powerLabel2 = ("W" == powerUnit2) ? QString().number((int) (curPower2 + 500)/1000) + " " + powerUnit2 : QString().number((float) (((int) curPower2 + 50)/100) / 10 )  + " " + powerUnit2;
        powerTextExtend2 = painter.boundingRect( 0, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop, powerLabel2 );
        maxPowerExtend = QSize( powerTextExtend1.width() > powerTextExtend2.width() ? powerTextExtend1.width() : powerTextExtend2.width(),
                                powerTextExtend1.height() > powerTextExtend2.height() ? powerTextExtend1.height() : powerTextExtend2.height() );
    }

    QSize powerPos1;
    // left upper corner of power consumption label
    if( KThinkBatConfig::powerMeterBelowGauge() ) {
        // Verbrauchsanzeige unterhalb der Gauge
        //         wastePos = QSize( KThinkBatConfig::borderSize().width(), KThinkBatConfig::borderSize().height() + gaugeSize.height() + 12 );
        powerPos1 = QSize( KThinkBatConfig::borderSize().width(), KThinkBatConfig::borderSize().height() + padding.height() + KThinkBatConfig::gaugeHeight() );
//         neededSize = QSize( (2 * KThinkBatConfig::borderSize().width() ) + KThinkBatConfig::gaugeWidth()
//                             , wastePos.height() + powerTextExtend1.height() + KThinkBatConfig::borderSize().height() );
        realNeededSpace = QSize( realNeededSpace.width(), realNeededSpace.height() + padding.height() + maxPowerExtend.height() );
    } else {
        // Verbrauchsanzeige rechts von der Gauge
        //         wastePos = QSize( ( 3 * border.width() ) + gaugeSize.width(), border.height() );
        powerPos1 = QSize( KThinkBatConfig::borderSize().width() + padding.width() + KThinkBatConfig::gaugeWidth(), KThinkBatConfig::borderSize().height() + ((KThinkBatConfig::gaugeHeight() - powerTextExtend1.height()) / 2 ) );
//         neededSize = QSize( wastePos.width() + powerTextExtend1.width() + KThinkBatConfig::borderSize().width()
//                             , ( 2 * KThinkBatConfig::borderSize().height() ) + KThinkBatConfig::gaugeHeight() );
        realNeededSpace = QSize( realNeededSpace.width() + padding.width() + maxPowerExtend.width(), realNeededSpace.height() );
    }

    // Painting the Text
    QPen origPen = painter.pen();
    painter.setPen( KThinkBatConfig::powerMeterColor() );
    // Draw the Power Consumption at position @c wastePos.
    painter.drawText( powerPos1.width(), powerPos1.height(), 
                          powerTextExtend1.width(), powerTextExtend1.height(),
                          Qt::AlignTop | Qt::AlignLeft, 
                          powerLabel1 );
    if( ! KThinkBatConfig::summarizeBatteries() ) { 
        QSize powerPos2;
        powerPos2 = QSize( KThinkBatConfig::borderSize().width() + padding.width() + KThinkBatConfig::gaugeWidth(),
                           KThinkBatConfig::borderSize().height() + padding.height() + KThinkBatConfig::gaugeHeight() );

        painter.drawText( powerPos2.width(), powerPos2.height(), 
                          powerTextExtend2.width(), powerTextExtend2.height(),
                          Qt::AlignTop | Qt::AlignLeft, 
                          powerLabel2 );
    }
    painter.setPen( origPen );

    //-------------------------------------------------------------------------
    painter.end();

    bitBlt(this, 0, 0, &pixmap);

    // new needed Size for the applet
    neededSize = realNeededSpace;

    // full extend is
    //   width: (3 x offset) + gaugeFill + gHalfDot ( + wasteSite )
    //   height: (3 x offset) + gaugeFill ( + wasteSite )
}


void 
KThinkBat::timeout() {

    float lastFuel = 0;
    float curFuel = 0;
//     float critFuel = 0;
    curPower1 = 0;
    bool batOnline = true;

    // 1. First try TP SMAPI on BAT0
    // 2. If that fails try ACPI /proc interface for BAT0
    bool battery1 = batInfo1.parseSysfsTP() || batInfo1.parseProcACPI();
    if( battery1 ) {
        if( ! KThinkBatConfig::summarizeBatteries() ) {
            gauge1.setPercentValue( (int) batInfo1.getChargeLevel() );
            gauge1.setColors( QColor( KThinkBatConfig::batBackgroundColor() ),
                              QColor( ((int) batInfo1.getChargeLevel()) <= KThinkBatConfig::criticalFill() ? KThinkBatConfig::batCriticalColor() : KThinkBatConfig::batChargedColor() ),
                              QColor( batInfo1.isOnline() ? KThinkBatConfig::batDotOnlineColor() : KThinkBatConfig::batBackgroundColor() ) );
            curPower1 = batInfo1.getPowerConsumption();
            powerUnit1 = batInfo1.getPowerUnit();
        }
        else {
            lastFuel += batInfo1.getLastFuel();
            curFuel += batInfo1.getCurFuel();
//             critFuel += batInfo1.getCriticalFuel();
            batOnline = batInfo1.isOnline();
            curPower1 += batInfo1.getPowerConsumption();
            powerUnit1 = batInfo1.getPowerUnit();
        }
    }

    // 3. Now try BAT1, first TP SMAPI agian
    // 4. And, if that failed, try ACPi /proc interface for BAT1
    bool battery2 = batInfo2.parseSysfsTP() || batInfo2.parseProcACPI();
    if( battery2 ) {
        if( ! KThinkBatConfig::summarizeBatteries() ) {
            gauge2.setPercentValue( (int) batInfo2.getChargeLevel() );
            gauge2.setColors( QColor( KThinkBatConfig::batBackgroundColor() ),
                              QColor( ((int) batInfo2.getChargeLevel()) <= KThinkBatConfig::criticalFill() ? KThinkBatConfig::batCriticalColor() : KThinkBatConfig::batChargedColor() ),
                              QColor( batInfo2.isOnline() ? KThinkBatConfig::batDotOnlineColor() : KThinkBatConfig::batBackgroundColor() ) );
            curPower2 = batInfo1.getPowerConsumption();
            powerUnit2 = batInfo1.getPowerUnit();
        }
        else {
            lastFuel += batInfo2.getLastFuel();
            curFuel += batInfo2.getCurFuel();
//             critFuel += batInfo2.getCriticalFuel();
            batOnline = batOnline || batInfo2.isOnline();
            curPower1 += batInfo2.getPowerConsumption();
            powerUnit1 = batInfo2.getPowerUnit();
        }
    }

    if( KThinkBatConfig::summarizeBatteries() ) {

        int percent = -1;
        if( curFuel >= 0 && lastFuel > 0 ) {
            percent =  (int) (( 100.0 / lastFuel ) * curFuel );
        } 
        gauge1.setPercentValue( percent );
        gauge1.setColors( QColor( KThinkBatConfig::batBackgroundColor() ),
                          QColor( percent <= KThinkBatConfig::criticalFill() ? KThinkBatConfig::batCriticalColor() : KThinkBatConfig::batChargedColor() ),
                          QColor( batOnline ? KThinkBatConfig::batDotOnlineColor() : KThinkBatConfig::batBackgroundColor() ) );
    }
    // force a repaint of the Applet
    update();
}

