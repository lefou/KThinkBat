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
    , batValue( -1 )
    , intervall(3000)
    , mWH( 0 )
    , online( true )
    , critical( false )
    , borderColor("black")
    , emptyColor("grey")
    , chargedColor("green")
    , timer(NULL)
    , unity("W")
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
    painter.fillRect(offset.width(), offset.height(), xFill, gaugeFill.height(), QColor( critical ? "red" : "green"));
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
    batInfo1.parseProcACPI();
    return;

    // ALT - jetzt ausgelagert in BatInfo

    // die aktuelle Zeile beim parsen des proc-fs
    QString line = "";
    QString mWHstring = "";
    // normale Angaben: Kapzität, Max Kap., aktuelle Kap.
    QString cap = "", designCap = "", cur = "", warnCap = "";
    // dieselben Angaben, aber in mAh statt in mWh
    QString capA = "", designCapA = "", warnCapA = "";

    // Pattern für /proc/acpi/battery/BAT0/info
    // ThinkPad (mWh)
    static QRegExp rxCap("^last full capacity:\\s*(\\d{1,5})\\s*mWh");
    static QRegExp rxDesignCap("^design capacity low:\\s*(\\d{1,5})\\s*mWh");
    static QRegExp rxWarnCap("^design capacity warning:\\s*(\\d{1,5})\\s*mWh");
    // Asus (mAh)
    static QRegExp rxCapA("^last full capacity:\\s*(\\d{1,5})\\s*mAh");
    static QRegExp rxDesignCapA("^design capacity low:\\s*(\\d{1,5})\\s*mAh");
    static QRegExp rxWarnCapA("^design capacity warning:\\s*(\\d{1,5})\\s*mAh");

    QFile file("/proc/acpi/battery/BAT0/info");
    if( ! file.exists() || ! file.open(IO_ReadOnly) ) {
       batValue = -1;
       qDebug("could not open \"/proc/acpi/battery/BAT0/info\"");
       return;
    }
    QTextStream stream( (QIODevice*) &file );

    while( ! stream.atEnd() ) {
        line = stream.readLine();
        if( -1 != rxCap.search( line ) ) {
            cap = rxCap.cap(1);
            //KMessageBox::information(0, "fount cap: "+cap);
        }
        if( -1 != rxDesignCap.search( line ) ) {
            designCap = rxDesignCap.cap(1);
        }
        if( -1 != rxWarnCap.search( line ) ) {
            warnCap = rxWarnCap.cap(1);
        }
        if( -1 != rxCapA.search( line ) ) {
            capA = rxCapA.cap(1);
            //KMessageBox::information(0, "fount cap: "+cap);
        }
        if( -1 != rxDesignCapA.search( line ) ) {
            designCapA = rxDesignCapA.cap(1);
        }
        if( -1 != rxWarnCapA.search( line ) ) {
            warnCapA = rxWarnCapA.cap(1);
        }
    }
    file.close();

    // Falls keine Angaben in mWh, dafür aber welche in mAh, 
    // dann verwenden wir A als Einheit
    if( cap.isEmpty() && ! capA.isEmpty() ) {
        // no relevant values found, (maybe) wa are using mAh instead of mWh
        // so copy this values and set default unity
        cap = capA;
        designCap = designCapA;
        warnCap = warnCapA;
        unity = "A";
    }

    file.setName("/proc/acpi/battery/BAT0/state");
    if( ! file.exists() || ! file.open(IO_ReadOnly) ) {
       batValue = -1;
       qDebug("could not open \"/proc/acpi/battery/BAT0/state\"");
       return;
    }
    stream.setDevice( &file );

    // Pattern für /proc/acpi/battery/BAT0/state
    QRegExp rxCur("^remaining capacity:\\s*(\\d{1,5})\\s*m"+unity+"h");
    QRegExp rxOffline("^charging state:\\s*(discharging|charging|charged)");
    QRegExp rxMWH("^present rate:\\s*(\\d{1,5})\\s*m"+unity);

    while( ! stream.atEnd() ) {
        line = stream.readLine();
        if( -1 != rxCur.search( line ) ) {
            cur = rxCur.cap(1);
            //KMessageBox::information(0, "fount cur: "+cur);
        }
        if( -1 != rxOffline.search( line ) ) {
            online = (rxOffline.cap(1) != "discharging");
            state = rxOffline.cap(1);
        }
        if( -1 != rxMWH.search( line ) ) {
            mWHstring = rxMWH.cap(1);
        }
    }
    file.close();

    bool ok1 = true, ok2 = true;
    int curValue = cur.toInt(&ok1);
    int capValue = cap.toInt(&ok2);

    if( ok1 && ok2 ) {
        batValue = (int) (( 100.0 / (float) capValue ) * (float) curValue);
    }
    else {
        // KMessageBox::information(0, "Fehler beim parsen: "+ cap +", "+designCap+", "+cur);
        qDebug( "Fehler beim konvertieren (current capacity: = " + cur + " and design capacity = " + cap + ")");
        // reset value, to reflect unawareness in display
        batValue = -1;
    }

    // if we got a warn value and the cur capacity is below warn, the set critical to true
    int warnValue = warnCap.toInt(&ok2);
    critical = ok1 && ok2 && ! online && capValue < warnValue;

    // current cosumption
    mWH = mWHstring.toInt(&ok2);

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
