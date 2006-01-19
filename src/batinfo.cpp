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

#include <qstring.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qfile.h>
#include <qdir.h>
#include <qtextstream.h>

#include "batinfo.h"

BatInfo::BatInfo( int number ) 
    : batNr( number -1 )
{
    resetValues();
}

BatInfo::~BatInfo() {
}

float
BatInfo::getChargeLevel() {
    if( curFuell >= 0 && lastFuell > 0 ) {
        return ( 100.0 / lastFuell ) * curFuell;
    }
    return -1;
}

bool
BatInfo::parseProcACPI() {

    // die aktuelle Zeile beim parsen des proc-fs
    QString line = "";
    QString mWHstring = "";
    // normale Angaben: Kapzität, Max Kap., aktuelle Kap.
    QString cap = "", designCap = "", cur = "";
    // dieselben Angaben, aber in mAh statt in mWh
    QString capA = "", designCapA = "";

    // Pattern für /proc/acpi/battery/BATx/info
    // ThinkPad (mWh)
    static QRegExp rxCap("^last full capacity:\\s*(\\d{1,5})\\s*mWh");
    static QRegExp rxDesignCap("^design capacity low:\\s*(\\d{1,5})\\s*mWh");
    // Asus (mAh)
    static QRegExp rxCapA("^last full capacity:\\s*(\\d{1,5})\\s*mAh");
    static QRegExp rxDesignCapA("^design capacity low:\\s*(\\d{1,5})\\s*mAh");

    QFile file( "/proc/acpi/battery/BAT" + QString::number(batNr) + "/info" );
    if( ! file.exists() || ! file.open(IO_ReadOnly) ) {
       curFuell = -1;
       qDebug( "could not open %s", file.name().latin1() );
       return false;
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
        if( -1 != rxCapA.search( line ) ) {
            capA = rxCapA.cap(1);
            //KMessageBox::information(0, "fount cap: "+cap);
        }
        if( -1 != rxDesignCapA.search( line ) ) {
            designCapA = rxDesignCapA.cap(1);
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
        powerUnit = "A";
    }

    file.setName("/proc/acpi/battery/BAT" + QString::number( batNr ) + "/state");
    if( ! file.exists() || ! file.open(IO_ReadOnly) ) {
       curFuell = -1;
       qDebug("could not open %s", file.name().latin1() );
       return false;
    }
    stream.setDevice( &file );

    // Pattern für /proc/acpi/battery/BATx/state
    QRegExp rxCur("^remaining capacity:\\s*(\\d{1,5})\\s*m" + powerUnit + "h");
    QRegExp rxOffline("^charging state:\\s*(discharging|charging|charged)");
    QRegExp rxMWH("^present rate:\\s*(\\d{1,5})\\s*m" + powerUnit );

    while( ! stream.atEnd() ) {
        line = stream.readLine();
        if( -1 != rxCur.search( line ) ) {
            cur = rxCur.cap(1);
            //KMessageBox::information(0, "fount cur: "+cur);
        }
        if( -1 != rxOffline.search( line ) ) {
            batState = rxOffline.cap(1);
        }
        if( -1 != rxMWH.search( line ) ) {
            mWHstring = rxMWH.cap(1);
        }
    }
    file.close();

    bool ok1 = true, ok2 = true;
    int curValue = cur.toInt(&ok1);
    int capValue = cap.toInt(&ok2);

    if( ok1 ) {
        curFuell = curValue;
    }
    if( ok2 ) {
        lastFuell = capValue;
    }
    
    // current cosumption
    curPower = mWHstring.toInt(&ok2);
    if( ! ok2 ) {
        curPower = -1;
    }

    // TODO better read /proc/acpi/ac_adapter/AC/state an evaluate "on-line"
    acConnected = (batState != "discharging");

    parseProcAcpiBatAlarm();

    return true;
}

bool 
BatInfo::parseProcAcpiBatAlarm() {

    QString warnCap = "";
    QString line = "";
    bool ok = false;

    // Get Alarm Fuell
    QFile file("/proc/acpi/battery/BAT" + QString::number( batNr ) + "/alarm");
    if( ! file.exists() || ! file.open(IO_ReadOnly) ) {
       curFuell = -1;
       qDebug("could not open %s", file.name().latin1() );
       return false;
    }
    
    QTextStream stream( (QIODevice*) &file );
    
    QRegExp rxWarnCap("^alarm:\\s*(\\d{1,5})\\s*m" + powerUnit + "h");
    
    while( ! stream.atEnd() ) {
        if( -1 != rxWarnCap.search( line ) ) {
            criticalFuell = rxWarnCap.cap(1).toInt(&ok);
        }
    }
    file.close();

    if( ! ok ) {
        criticalFuell = 0;
    }

    return ok;
}

bool 
BatInfo::parseSysfsTP() {

    // critical Fuell can not be set via tp_smapi, so we decide is statically
    criticalFuell = 1000;
    powerUnit = "W";

    QString tpPath = "/sys/devices/platform/smapi/BAT" + QString::number(batNr) + "/";
    QFile file;
    QTextStream stream;
    QString line;
    QRegExp mWh( "^([-]?\\d{1,6})\\s*mWh\\s*$" );
    QRegExp mW( "^([-]?\\d{1,6})\\s*mW\\s*$" );
    bool check;
    
    if( ! QDir().exists( "/sys/devices/platform/smapi" ) ) {
        static bool sayTheProblem = true;
        if( sayTheProblem ) {
            qDebug( "There is no directory /sys/devices/platform/smapi. Do you have tp_smapi loaded?" );
            sayTheProblem = false;
        }
        return false;
    }

    file.setName( tpPath + "installed" );
    if( file.exists() && file.open(IO_ReadOnly) ) {
        stream.setDevice( &file );
        batInstalled = ( 1 == stream.readLine().toInt( &check ) ? true : false );
        file.close();
        if( ! batInstalled ) {
            resetValues();
            return true;
        }
    }
    else {
        batInstalled = false;
    }

    file.setName( tpPath + "last_full_capacity" );
    if( file.exists() && file.open(IO_ReadOnly) ) {
        stream.setDevice( &file );
        line = stream.readLine();
        if( -1 != mWh.search( line ) ) {
            lastFuell = mWh.cap(1).toInt( &check );
        }
        file.close();
    }
    else {
        lastFuell = 0;
    }

    file.setName( tpPath + "design_capacity" );
    if( file.exists() && file.open(IO_ReadOnly) ) {
        stream.setDevice( &file );
        line = stream.readLine();
        if( -1 != mWh.search( line ) ) {
            designFuell = mWh.cap(1).toInt( &check );
        }
        file.close();
    }
    else {
        designFuell = 0;
    }
    
    file.setName( tpPath + "remaining_capacity" );
    if( file.exists() && file.open(IO_ReadOnly) ) {
        stream.setDevice( &file );
        line = stream.readLine();
        if( -1 != mWh.search( line ) ) {
            curFuell = mWh.cap(1).toInt( &check );
        }
        file.close();
    }
    else {
        curFuell = -1;
    }
    
    file.setName( tpPath + "power_now" );
    if( file.exists() && file.open(IO_ReadOnly) ) {
        stream.setDevice( &file );
        line = stream.readLine();
        if( -1 != mW.search( line ) ) {
            curPower = mW.cap(1).toInt( &check );
            if( curPower < 0 ) { curPower = (0 - curPower); }
        }
        file.close();
    }
    else {
        curPower = -1;
    }

    file.setName( tpPath + "state" );
    if( file.exists() && file.open(IO_ReadOnly) ) {
        stream.setDevice( &file );
        batState = stream.readLine();
        file.close();
    }
    else {
        batState = "";
    }

    file.setName( "/sys/devices/platform/smapi/ac_connected" );
    if( file.exists() && file.open(IO_ReadOnly) ) {
        stream.setDevice( &file );
        acConnected = stream.readLine().toInt( &check );
        file.close();
    }
    else {
        acConnected = false;
    }

    parseProcAcpiBatAlarm();

    return true;
}

void 
BatInfo::resetValues() {
    lastFuell = 0;
    designFuell = 0;
    criticalFuell = 0;
    curFuell = 0;
    curPower = 0;
    batInstalled = false;
    powerUnit = "W";
    batState = "not installed";
}
