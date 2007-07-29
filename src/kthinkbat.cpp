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

// Qt
#include <qgroupbox.h>
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
, padding(QSize(5, 2))
, timer(NULL)
, m_batInfo1(1)
, m_batInfo2(2)
, m_batInfoSum(&m_batInfo1, &m_batInfo2)
, powerPosID(0)
, m_contextMenu(NULL)
, m_toolTipTimer(NULL)
, m_toolTip(NULL) {


    KThinkBatConfig::instance( configFile );

    neededSize = QSize( KThinkBatConfig::gaugeWidth() + (2* KThinkBatConfig::borderSize().width()), KThinkBatConfig::gaugeHeight() + (2* KThinkBatConfig::borderSize().width()) );

    // Create a popup menu to show KThinkBats specific options.
    m_contextMenu = new KPopupMenu();
    assert(m_contextMenu);
    m_contextMenu->insertTitle(i18n("KThinkBat %1").arg(VERSION));
    m_contextMenu->insertItem(i18n("&About %1").arg("KThinkBat"), this, SLOT(slotAbout()));
    m_contextMenu->insertItem(SmallIcon( "configure" ), i18n("&Configure %1...").arg("KThinkBat"), this, SLOT(slotPreferences()));
    setCustomMenu(m_contextMenu);

    // read initial battery data
    readBatteryInfoTimeout();
    // create a timer to read battery data
    timer = new QTimer(this);
    assert( timer );
    connect(timer, SIGNAL(timeout()), this, SLOT(readBatteryInfoTimeout()));
    timer->start( KThinkBatConfig::updateIntervalMsek() );

    m_toolTipTimer = new QTimer(this);
    assert(m_toolTipTimer);
    connect( m_toolTipTimer, SIGNAL(timeout()), this, SLOT(slotToolTip()));
    m_toolTip = new BatToolTip( this );
    assert(m_toolTip);

    // Trigger some translations
    i18n("charged");
    i18n("charging");
    i18n("discharging");
    i18n("idle");
    i18n("not installed");
}

KThinkBat::~KThinkBat() {

    if (timer) {
        timer->stop();
        delete timer; 
    }
    timer = NULL;

    // Save Config values
    KThinkBatConfig::writeConfig();

    if (m_contextMenu) {
        delete m_contextMenu;
        m_contextMenu = NULL;
    }
    if (m_toolTipTimer) {
        delete m_toolTipTimer;
        m_toolTipTimer = NULL;
    }
    if (m_toolTip) {
        delete m_toolTip;
        m_toolTip = NULL;
    }
}

void
KThinkBat::slotPreferences() {
    if (KConfigDialog::showDialog("KThinkBatSettings")) {
        return;
    }

    KConfigDialog* dialog = new KConfigDialog(this, "KThinkBatSettings", KThinkBatConfig::self(), KDialogBase::Plain);

    Prefs* prefs = new Prefs(this);
    assert(prefs);

    prefs->advancedAcpiGroup->setEnabled(KThinkBatConfig::overridePowerSettings());
    prefs->advancedSmapiGroup->setEnabled(KThinkBatConfig::overridePowerSettings());

    prefs->enableAcpiFrame->setEnabled(KThinkBatConfig::enableAcpi());
    prefs->enableSmapiFrame->setEnabled(KThinkBatConfig::enableSmapi());

    dialog->addPage(prefs, i18n("KThinkBat Preferences"), "configure");

    //User edited the configuration - update your local copies of the 
    //configuration data 
    connect(dialog, SIGNAL(settingsChanged()), this, SLOT(slotUpdateConfiguration()));

    dialog->show();
}

void
KThinkBat::slotUpdateConfiguration() {

    readBatteryInfoTimeout();
    update();
}

void 
KThinkBat::slotAbout() {

    KAboutData aboutData("KThinkBat", "KThinkBat", VERSION,
                         I18N_NOOP("A KDE panel applet to display the current laptop battery status."),
                         KAboutData::License_GPL_V2,
                         "(c) 2005-2007, Tobias Roeser",
                         "",
                         "https://lepetitfou.dyndns.org/kthinkbat",
                         "le.petit.fou@web.de");

    aboutData.addAuthor("Tobias Roeser", "", "le.petit.fou@web.de",
                        "https://lepetitfou.dyndns.org/kthinkbat");

    aboutData.addCredit("Luis Guillermo Sobalvarro", "Icon design and Spanish translation.", "lgsobalvarro@e-genieria.com");

    KAboutApplication about(&aboutData, this, NULL, 0);
    about.setIcon(KGlobal::instance()->iconLoader()->iconPath( "kthinkbat", -KIcon::SizeLarge));
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

QString
KThinkBat::createPowerTimeLabel(BatInfoBase* batInfo) {

    if (!batInfo) {  return ""; }

    QString label = "";

    if (KThinkBatConfig::showPowerMeter()) {
        label = batInfo->getPowerConsumptionFormated();
    }

    if (KThinkBatConfig::showRemainingTime()) {
        if (KThinkBatConfig::showPowerMeter()) {
            label += " / ";
        }
        label += batInfo->isFull() ? "full" : batInfo->getRemainingTimeFormated();
    }

    return label;
}

void 
KThinkBat::paintEvent(QPaintEvent* event) {

    // Gauge size and orientation
    m_gauge1.setOrientation(KThinkBatConfig::drawBatteryUpright() ? Qt::Vertical : Qt::Horizontal);
    m_gauge1.setSize(KThinkBatConfig::gaugeWidth(), KThinkBatConfig::gaugeHeight());
    m_gauge2.setOrientation(KThinkBatConfig::drawBatteryUpright() ? Qt::Vertical : Qt::Horizontal);
    m_gauge2.setSize(KThinkBatConfig::gaugeWidth(), KThinkBatConfig::gaugeHeight());

    QPixmap pixmap(width(), height());
    pixmap.fill(this, 0, 0);
    QPainter painter(&pixmap);
    painter.setFont(KThinkBatConfig::gaugeFont());

    // this is the (minimal) needed Space by the Applet
    QSize realNeededSpace = QSize((2 * KThinkBatConfig::borderSize().width()) + KThinkBatConfig::gaugeWidth(),
                                   (2 * KThinkBatConfig::borderSize().height()) + KThinkBatConfig::gaugeHeight());

    m_gauge1.drawGauge(painter, KThinkBatConfig::borderSize());

    QSize nextSouth = QSize(KThinkBatConfig::borderSize().width(), KThinkBatConfig::borderSize().height() + padding.height() + KThinkBatConfig::gaugeHeight());
    QSize nextEast = QSize(KThinkBatConfig::borderSize().width() + padding.width() + KThinkBatConfig::gaugeWidth(), KThinkBatConfig::borderSize().height());

    if (!KThinkBatConfig::summarizeBatteries()) {
        // If we have to draw two batteries
        if (KThinkBatConfig::powerMeterBelowGauge()) {
            m_gauge2.drawGauge(painter, nextEast);
            realNeededSpace = QSize(realNeededSpace.width() + padding.width() + KThinkBatConfig::gaugeWidth(), realNeededSpace.height());
        }
        else {
            m_gauge2.drawGauge(painter, nextSouth);
            realNeededSpace = QSize(realNeededSpace.width(), realNeededSpace.height() + padding.height()  + KThinkBatConfig::gaugeHeight());
        }
    }

    if (KThinkBatConfig::showPowerMeter() || KThinkBatConfig::showRemainingTime()) {
        // We have to draw some text below or beside the Gauge Symbol

        painter.setFont(KThinkBatConfig::powerMeterFont());

        // Position for power consumption display
        // Power consumption label: For correct rounding we add 500 mW (resp. 50 mA)
        QString powerLabel1 = createPowerTimeLabel(KThinkBatConfig::summarizeBatteries() ? (BatInfoBase*)&m_batInfoSum : (BatInfoBase*)&m_batInfo1);
        // Needed Space for Power Consumption Label
        QRect powerTextExtend1 = painter.boundingRect(0, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop, powerLabel1);

        QString powerLabel2("");
        QRect powerTextExtend2(0,0,0,0);
        QSize maxPowerExtend = QSize(powerTextExtend1.width(), powerTextExtend1.height());
    
        if (!KThinkBatConfig::summarizeBatteries()) {
            powerLabel2 = createPowerTimeLabel(&m_batInfo2);
            powerTextExtend2 = painter.boundingRect( 0, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop, powerLabel2 );
            maxPowerExtend = QSize(powerTextExtend1.width() > powerTextExtend2.width() ? powerTextExtend1.width() : powerTextExtend2.width(),
                                    powerTextExtend1.height() > powerTextExtend2.height() ? powerTextExtend1.height() : powerTextExtend2.height());
        }

        // Painting the Text
        QPen origPen = painter.pen();
        painter.setPen(KThinkBatConfig::powerMeterColor());

        QSize powerPos1;
        // left upper corner of power consumption label
        if (KThinkBatConfig::powerMeterBelowGauge()) {
            // Verbrauchsanzeige unterhalb der Gauge
            //         wastePos = QSize( KThinkBatConfig::borderSize().width(), KThinkBatConfig::borderSize().height() + gaugeSize.height() + 12 );
            powerPos1 = QSize(KThinkBatConfig::borderSize().width(), 
                              KThinkBatConfig::borderSize().height() + padding.height() + KThinkBatConfig::gaugeHeight());
            realNeededSpace = QSize(realNeededSpace.width() > maxPowerExtend.width() ? realNeededSpace.width() : ((2 * KThinkBatConfig::borderSize().width()) + maxPowerExtend.width())
                                    , realNeededSpace.height() + padding.height() + maxPowerExtend.height());
        } else {
            // Verbrauchsanzeige rechts von der Gauge
            //         wastePos = QSize( ( 3 * border.width() ) + gaugeSize.width(), border.height() );
            powerPos1 = QSize(KThinkBatConfig::borderSize().width() + padding.width() + KThinkBatConfig::gaugeWidth(),
                                KThinkBatConfig::borderSize().height() + ((KThinkBatConfig::gaugeHeight() - powerTextExtend1.height()) / 2 ));
            realNeededSpace = QSize(realNeededSpace.width() + padding.width() + maxPowerExtend.width(), realNeededSpace.height());
        }

        // Draw the Power Consumption at position @c wastePos.
        painter.drawText(powerPos1.width(), powerPos1.height(), 
                              powerTextExtend1.width(), powerTextExtend1.height(),
                              Qt::AlignTop | Qt::AlignLeft, 
                              powerLabel1);
        if (!KThinkBatConfig::summarizeBatteries()) { 
            QSize powerPos2;

            if (KThinkBatConfig::powerMeterBelowGauge()) {
                // Verbrauchsanzeige unterhalb der Gauge
//                 powerPos2 = QSize( KThinkBatConfig::borderSize().width() + padding.width() + KThinkBatConfig::gaugeWidth() > powerTextExtend1.width() ? KThinkBatConfig::gaugeWidth() : powerTextExtend1.width(),
                powerPos2 = QSize(KThinkBatConfig::borderSize().width() + padding.width() + KThinkBatConfig::gaugeWidth(),
                                   KThinkBatConfig::borderSize().height() + padding.height() + KThinkBatConfig::gaugeHeight());
            } else {
                // Verbrauchsanzeige rechts von der Gauge
                powerPos2 = QSize(KThinkBatConfig::borderSize().width() + padding.width() + KThinkBatConfig::gaugeWidth(),
                                  KThinkBatConfig::borderSize().height() + padding.height() + KThinkBatConfig::gaugeHeight() + ((KThinkBatConfig::gaugeHeight() - powerTextExtend2.height()) / 2));
            }

            painter.drawText(powerPos2.width(), powerPos2.height(), 
                             powerTextExtend2.width(), powerTextExtend2.height(),
                             Qt::AlignTop | Qt::AlignLeft, 
                             powerLabel2);
        }
        painter.setPen(origPen);

    }
    painter.end();
    bitBlt(this, 0, 0, &pixmap);

     if(neededSize != realNeededSpace) {
        // new needed Size for the applet
        neededSize = realNeededSpace;
        emit updateLayout();
    }
    else {
        // new needed Size for the applet
        neededSize = realNeededSpace;
    }
}

void 
KThinkBat::fillBatGauge(BatGauge* gauge, BatInfoBase* info) {

    // Colors
    gauge->setColors(QColor(KThinkBatConfig::batBackgroundColor()),
                     QColor(((int) info->getChargeLevel()) <= KThinkBatConfig::criticalFill() ? KThinkBatConfig::batCriticalColor() : KThinkBatConfig::batChargedColor()),
                     QColor(info->isOnline() ? KThinkBatConfig::batDotOnlineColor() : KThinkBatConfig::batBackgroundColor()));

    // Percentages
    if (info->isInstalled()) {
        if (KThinkBatConfig::gaugeContentPercent()) {
            gauge->setPercentValue((int) info->getChargeLevel());
        }
        else if (KThinkBatConfig::gaugeContentTime()) {
            gauge->setPercentValueString((int) info->getChargeLevel(), info->isFull() ? "full" : info->getRemainingTimeFormated());
        }
        else {
            gauge->setPercentValueString((int) info->getChargeLevel(), "");
        }
    }
    else {
        gauge->setPercentValue(-1);
    }
}

void 
KThinkBat::readBatteryInfoTimeout() {

    if (KThinkBatConfig::summarizeBatteries()) {
        m_batInfoSum.refresh();
        fillBatGauge(&m_gauge1, &m_batInfoSum);
    }
    else {
        m_batInfo1.refresh();
        m_batInfo2.refresh();
        fillBatGauge(&m_gauge1, &m_batInfo1);
        fillBatGauge(&m_gauge2, &m_batInfo2);
    }

    // force a repaint of the Applet
    update();

    // refresh the tooltip, if shown
    if (m_toolTip && m_toolTip->isShown()) {
        m_toolTip->setText(createToolTipText());
    }
}

QString
KThinkBat::createToolTipText() {
    QString toolTipText = "";
    BatInfo* batInfo;
    bool battery;

    for (int bat = 1; bat <= 2; ++bat) {

        if (bat == 1) { 
            batInfo = &m_batInfo1;
        }
        else if (bat == 2) {
            batInfo = &m_batInfo2;
        }
        assert(batInfo);


        QString batHeader = "<b>" + i18n("Battery %1").arg( bat );
        if (batInfo->getLastSuccessfulReadMethod() != "") {
            batHeader += " (" + batInfo->getLastSuccessfulReadMethod() + ")";
        }
        batHeader += "</b>";

        toolTipText += "<table cellspacing=\"0\" cellpadding=\"0\">";
        if (batInfo && batInfo->isInstalled()) {
            toolTipText += toolTipLine(batHeader, QString().number((int) batInfo->getChargeLevel()) + "%");

            toolTipText += toolTipLine( batInfo->isCharging() ? i18n("Current Charge Rate") : i18n("Current Consumption"), batInfo->getPowerConsumptionFormated());

            toolTipText += toolTipLine( i18n("Current Fuel"), QString().number((float)  batInfo->getCurFuel()) + " m" + batInfo->getPowerUnit());

            toolTipText += toolTipLine( i18n("Last Fuel"), QString().number((float) batInfo->getLastFuel()) + " m" + batInfo->getPowerUnit());

            toolTipText += toolTipLine( i18n("Design Fuel"), QString().number((float) batInfo->getDesignFuel()) + " m" + batInfo->getPowerUnit());

            toolTipText += toolTipLine( i18n("Critical Fuel"), QString().number((float) batInfo->getCriticalFuel()) + " m" + batInfo->getPowerUnit());

            if(batInfo->getCycleCount() > 0 ) {
                toolTipText += toolTipLine( i18n("Cycle Count"), QString().number(batInfo->getCycleCount()));
            }

            toolTipText += toolTipLine( i18n("State"), i18n(batInfo->getState()));

            toolTipText += toolTipLine( i18n("Remaining Time"), batInfo->isFull() ? i18n("full charged") : batInfo->getRemainingTimeFormated());
        }
        else {
            toolTipText += toolTipLine(batHeader, i18n("not installed"));
        } 
        toolTipText += "</table>";
    }
    return toolTipText;
}

QString
KThinkBat::toolTipLine(const QString& label, const QString& value) {
    return "<tr><td>" + label + ": </td><td>" + value + "</td></tr>";
}

void 
KThinkBat::mousePressEvent(QMouseEvent* e) {
    if (e->button() != RightButton) {
        KPanelApplet::mousePressEvent(e);
        return;
    }

    assert(m_contextMenu);
    m_contextMenu->exec( e->globalPos() );
}

void
KThinkBat::enterEvent( QEvent* e) {
    if (KThinkBatConfig::showToolTip() && m_toolTipTimer && m_toolTip && !m_toolTip->isShown()) {
        // TODO read the system time preferences for ToolTip times
        // in msek
        m_toolTip->setText(createToolTipText());
        m_toolTipTimer->start(KThinkBatConfig::toolTipTimeout());
    }
}

void
KThinkBat::leaveEvent( QEvent* e) {
    if (m_toolTipTimer) {
        m_toolTipTimer->stop();
    }
    if (m_toolTip) {
        m_toolTip->hide();
    }
}

void
KThinkBat::slotToolTip() {
    if (KThinkBatConfig::showToolTip() && m_toolTip) {
//         m_toolTip->setText(m_toolTipText);
        m_toolTip->setText(createToolTipText());
        m_toolTip->show();
    }
}
