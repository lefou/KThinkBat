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
#include <qvbox.h>
#include <qlabel.h>

// KDE
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kaboutapplication.h>
#include <kconfigdialog.h>
#include <kiconloader.h>

// other libs 
#include <assert.h>

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
, timer( NULL )
, batInfo1( 1 )
, batInfo2( 2 )
, powerPosID( 0 )
, contextMenu( NULL )
, toolTipTimer( NULL )
, toolTip( NULL ) 
, toolTipText("") {

    KThinkBatConfig::instance( configFile );

    neededSize = QSize( KThinkBatConfig::gaugeWidth() + (2* KThinkBatConfig::borderSize().width()), KThinkBatConfig::gaugeHeight() + (2* KThinkBatConfig::borderSize().width()) );

    // Create a popup menu to show KThinkBats specific options.
    contextMenu = new KPopupMenu();
    assert( contextMenu );
    contextMenu->insertTitle( i18n("KThinkBat %1").arg(VERSION) );
    contextMenu->insertItem( i18n("&About %1").arg("KThinkBat"), this, SLOT(slotAbout()) );
    contextMenu->insertItem( SmallIcon( "configure" ), i18n("&Configure %1...").arg("KThinkBat"), this, SLOT(slotPreferences()) );
    setCustomMenu( contextMenu );

    // Timer der die Aktualisierung von ACPI/SMAPI-Werten und deren Anzeige veranlasst.
    timeout();
    timer = new QTimer(this);
    assert( timer );
    connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
    timer->start( KThinkBatConfig::updateIntervalMsek() );

    toolTipTimer = new QTimer(this);
    assert( toolTipTimer );
    connect( toolTipTimer, SIGNAL(timeout()), this, SLOT(slotToolTip()));
    toolTip = new BatToolTip( this );
    assert( toolTip );
}

KThinkBat::~KThinkBat() {

    if( timer ) {
        timer->stop();
        delete timer; 
    }
    timer = NULL;

    // Save Config values
    KThinkBatConfig::writeConfig();

    if( contextMenu ) delete contextMenu; contextMenu = NULL;
    if( toolTipTimer ) delete toolTipTimer; toolTipTimer = NULL;
    if( toolTip ) delete toolTip; toolTip = NULL;
}

void
KThinkBat::slotPreferences() {
    if ( KConfigDialog::showDialog( "KThinkBatSettings" ) ) {
        return;
    }

    KConfigDialog* dialog = new KConfigDialog( this, "KThinkBatSettings", KThinkBatConfig::self(), KDialogBase::Plain );

    Prefs* prefs = new Prefs( this );
    assert( prefs );
 
    dialog->addPage( prefs, i18n("KThinkBat Preferences"), "configure" ); 
 
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

    aboutData.addAuthor( "Tobias Roeser", "", "le.petit.fou@web.de",
                         "https://lepetitfou.dyndns.org/KThinkBat" );

    KAboutApplication about( &aboutData, this, NULL, 0 );
    about.setIcon( KGlobal::instance()->iconLoader()->iconPath( "kthinkbat", -KIcon::SizeLarge ) );
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

// void 
// KThinkBat::resizeEvent(QResizeEvent *e) {}

void 
KThinkBat::paintEvent(QPaintEvent* event) {

    QPixmap pixmap(width(), height());
    pixmap.fill(this, 0, 0);
    QPainter painter(&pixmap);
    painter.setFont( KThinkBatConfig::gaugeFont() );

    // this is the (minimal) needed Space by the Applet
    QSize realNeededSpace = QSize( (2 * KThinkBatConfig::borderSize().width()) + KThinkBatConfig::gaugeWidth(),
                                   (2 * KThinkBatConfig::borderSize().height()) + KThinkBatConfig::gaugeHeight() );

    gauge1.drawGauge( painter, KThinkBatConfig::borderSize(), QSize( KThinkBatConfig::gaugeWidth(), KThinkBatConfig::gaugeHeight() ) );

    QSize nextSouth = QSize( KThinkBatConfig::borderSize().width(), KThinkBatConfig::borderSize().height() + padding.height() + KThinkBatConfig::gaugeHeight() );
    QSize nextEast = QSize( KThinkBatConfig::borderSize().width() + padding.width() + KThinkBatConfig::gaugeWidth(), KThinkBatConfig::borderSize().height() );

    if( ! KThinkBatConfig::summarizeBatteries() ) {
        // If we have to draw two batteries
        if( KThinkBatConfig::powerMeterBelowGauge() ) {
            gauge2.drawGauge( painter, nextEast, QSize( KThinkBatConfig::gaugeWidth(), KThinkBatConfig::gaugeHeight() ) );
            realNeededSpace = QSize( realNeededSpace.width() + padding.width() + KThinkBatConfig::gaugeWidth(), realNeededSpace.height() );
        }
        else {
            gauge2.drawGauge( painter, nextSouth, QSize( KThinkBatConfig::gaugeWidth(), KThinkBatConfig::gaugeHeight() ) );
            realNeededSpace = QSize( realNeededSpace.width(), realNeededSpace.height() + padding.height()  + KThinkBatConfig::gaugeHeight() );
        }
    }

    if( KThinkBatConfig::showPowerMeter() ) {
        // We have to draw some text below or beside the Gauge Symbol

        painter.setFont( KThinkBatConfig::powerMeterFont() );

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

        // Painting the Text
        QPen origPen = painter.pen();
        painter.setPen( KThinkBatConfig::powerMeterColor() );

        QSize powerPos1;
        // left upper corner of power consumption label
        if( KThinkBatConfig::powerMeterBelowGauge() ) {
            // Verbrauchsanzeige unterhalb der Gauge
            //         wastePos = QSize( KThinkBatConfig::borderSize().width(), KThinkBatConfig::borderSize().height() + gaugeSize.height() + 12 );
            powerPos1 = QSize( KThinkBatConfig::borderSize().width(), 
                               KThinkBatConfig::borderSize().height() + padding.height() + KThinkBatConfig::gaugeHeight() );
            realNeededSpace = QSize( realNeededSpace.width(), realNeededSpace.height() + padding.height() + maxPowerExtend.height() );
        } else {
            // Verbrauchsanzeige rechts von der Gauge
            //         wastePos = QSize( ( 3 * border.width() ) + gaugeSize.width(), border.height() );
            powerPos1 = QSize( KThinkBatConfig::borderSize().width() + padding.width() + KThinkBatConfig::gaugeWidth(),
                                KThinkBatConfig::borderSize().height() + ((KThinkBatConfig::gaugeHeight() - powerTextExtend1.height()) / 2 ) );
            realNeededSpace = QSize( realNeededSpace.width() + padding.width() + maxPowerExtend.width(), realNeededSpace.height() );
        }

        // Draw the Power Consumption at position @c wastePos.
        painter.drawText( powerPos1.width(), powerPos1.height(), 
                              powerTextExtend1.width(), powerTextExtend1.height(),
                              Qt::AlignTop | Qt::AlignLeft, 
                              powerLabel1 );
        if( ! KThinkBatConfig::summarizeBatteries() ) { 
            QSize powerPos2;

            if( KThinkBatConfig::powerMeterBelowGauge() ) {
                // Verbrauchsanzeige unterhalb der Gauge
                powerPos2 = QSize( KThinkBatConfig::borderSize().width() + padding.width() + KThinkBatConfig::gaugeWidth(),
                                   KThinkBatConfig::borderSize().height() + padding.height() + KThinkBatConfig::gaugeHeight() );
            } else {
                // Verbrauchsanzeige rechts von der Gauge
                powerPos2 = QSize( KThinkBatConfig::borderSize().width() + padding.width() + KThinkBatConfig::gaugeWidth(),
                                   KThinkBatConfig::borderSize().height() + padding.height() + KThinkBatConfig::gaugeHeight() + ((KThinkBatConfig::gaugeHeight() - powerTextExtend2.height()) / 2 ) );
            }

            painter.drawText( powerPos2.width(), powerPos2.height(), 
                              powerTextExtend2.width(), powerTextExtend2.height(),
                              Qt::AlignTop | Qt::AlignLeft, 
                              powerLabel2 );
        }
        painter.setPen( origPen );

    }
    painter.end();
    bitBlt(this, 0, 0, &pixmap);

//      if( neededSize != realNeededSpace ) {
//         // new needed Size for the applet
//         neededSize = realNeededSpace;
//         resize( realNeededSpace );
//         repaint();
//     }
//     else {
        // new needed Size for the applet
        neededSize = realNeededSpace;
//     }
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
            curPower2 = batInfo2.getPowerConsumption();
            powerUnit2 = batInfo2.getPowerUnit();
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

//     QSize oldSize = neededSize;

    // force a repaint of the Applet
    update();

    // This is some kind of hack to force a resize
//     if( oldSize != neededSize ) {
//         resize( neededSize );
//         update();
//     }

    if( toolTip && toolTip->isShown() ) {
        toolTip->setText( createToolTipText( battery1, battery2 ) );
    }
}

QString
KThinkBat::createToolTipText( bool battery1, bool battery2 ) {
    toolTipText = "";
    BatInfo* batInfo;
    bool battery;

    for( int bat = 1; bat <= 2; ++bat ) {

        if( bat == 1 ) { batInfo = &batInfo1; battery = battery1; }
        else if( bat == 2 ) { batInfo = &batInfo2; battery = battery2; }
        else { break; }
        assert( batInfo );

        toolTipText += "<table cellspacing=\"0\" cellpadding=\"0\">";
        toolTipText += "<tr><td><b>" + i18n("Battery %1: ").arg( bat ) + "</b></td>";

        if( batInfo && battery && batInfo->isInstalled() ) {
            toolTipText += "<td>" + QString().number((int) batInfo->getChargeLevel()) + "%</td></tr>";
            toolTipText += "<tr><td>" + i18n("Current Fuel: ") + "</td><td>" + QString().number((float) batInfo->getCurFuel()) + " m" + batInfo->getPowerUnit() + "h</td></tr>";
            toolTipText += "<tr><td>" + i18n("Crit Fuel: ") + "</td><td>" + QString().number((float) batInfo->getCriticalFuel()) + " m" + batInfo->getPowerUnit() + "h</td></tr>";
            toolTipText += "<tr><td>" + i18n("Last Fuel: ") + "</td><td>" + QString().number((float) batInfo->getLastFuel()) + " m" + batInfo->getPowerUnit() + "h</td></tr>";
            toolTipText += "<tr><td>" + i18n("Design Fuel: ") + "</td><td>" + QString().number((float) batInfo->getDesignFuel()) + " m" + batInfo->getPowerUnit() + "h</td></tr>";
            toolTipText += "<tr><td>" + i18n("State: ") + "</td><td>" + i18n(batInfo->getState()) + "</td></tr>";
            toolTipText += "<tr><td>" + i18n("Remaining Time: ") + "</td><td>" + QString().number((int) batInfo->getRemainingTimeInMin()) + " min</td></tr>";
        }
        else {
            toolTipText += "<td>" + i18n("not installed") + "</td></tr>";
        } 
        toolTipText += "</table>";
    }
    return toolTipText;
}

void 
KThinkBat::mousePressEvent(QMouseEvent* e) {
    if ( e->button() != RightButton ) {
        KPanelApplet::mousePressEvent( e );
        return;
    }

    assert(contextMenu);
    contextMenu->exec( e->globalPos() );
}

void
KThinkBat::enterEvent( QEvent* e) {
    if( KThinkBatConfig::showToolTip() && toolTipTimer && toolTip && ! toolTip->isShown() ) {
        // FIXME read the system time preferences for ToolTip times
        // in msek
        toolTip->setText( createToolTipText() );
        toolTipTimer->start( 500 );
    }
}

void
KThinkBat::leaveEvent( QEvent* e) {
    if( toolTipTimer ) {
        toolTipTimer->stop();
    }
    if( toolTip ) {
        toolTip->hide();
    }
}

void
KThinkBat::slotToolTip() {
    if( KThinkBatConfig::showToolTip() && toolTip ) {
        toolTip->setText( toolTipText );
        toolTip->show();
    }
}
