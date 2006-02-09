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
: batNr( number - 1 ) {

    resetValues();
}

BatInfo::~BatInfo() {
}

float
BatInfo::getChargeLevel() {
    if( curFuel >= 0 && lastFuel > 0 ) {
        return ( 100.0 / lastFuel ) * curFuel;
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
    QRegExp rxInstalled( "^present:\\s*(yes)" );
    QRegExp rxCap("^last full capacity:\\s*(\\d{1,5})\\s*mWh");
    QRegExp rxDesignCap("^design capacity low:\\s*(\\d{1,5})\\s*mWh");
    // Asus/Acer (mAh)
    QRegExp rxCapA("^last full capacity:\\s*(\\d{1,5})\\s*mAh");
    QRegExp rxDesignCapA("^design capacity low:\\s*(\\d{1,5})\\s*mAh");

    QString filename = "/proc/acpi/battery/BAT" + QString::number(batNr) + "/info";
    QFile file( filename );
    if( ! file.exists() || ! file.open(IO_ReadOnly) ) {
        qDebug( "could not open %s", file.name().latin1() );
        resetValues();
        return false;
    }

    batInstalled = true;

    QTextStream stream( (QIODevice*) &file );
    while( ! stream.atEnd() ) {
        line = stream.readLine();
        if( -1 != rxInstalled.search( line ) ) {
            batInstalled = rxInstalled.cap(1) == "yes";
            //KMessageBox::information(0, "fount cap: "+cap);
        }
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

    if( ! batInstalled ) {
        resetValues();
        batInstalled = false;
        return false;
    }   

    // Falls keine Angaben in mWh, dafür aber welche in mAh, 
    // dann verwenden wir A als Einheit
    if( cap.isEmpty() && ! capA.isEmpty() ) {
//     if( ! capA.isEmpty() ) {
        cap = capA;
        designCap = designCapA;
        powerUnit = "A";
    }
    else {
        powerUnit = "W";
    }

    filename = "/proc/acpi/battery/BAT" + QString::number(batNr) + "/state";
    file.setName( filename );
    if( ! file.exists() || ! file.open(IO_ReadOnly) ) {
        qDebug("could not open %s", file.name().latin1() );
        resetValues();
        return false;
    }
    stream.setDevice( &file );

    // Pattern for /proc/acpi/battery/BATx/state
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

    bool ok = true;
    curFuel = cur.toInt(&ok);
    if( ! ok ) { curFuel = 0; }

    ok = true;
    lastFuel = cap.toInt(&ok);
    if( ! ok ) { lastFuel = 0; }

    // current cosumption
    ok = true;
    curPower = mWHstring.toInt(&ok);
    if( ! ok ) { curPower = 0; }

    // TODO better read /proc/acpi/ac_adapter/AC/state an evaluate "on-line"
    bool oldAcCon = isOnline();
    acConnected = (batState != "discharging");
    if( oldAcCon != acConnected ) {
        emit onlineModeChanged( acConnected );
    }

//     parseProcAcpiBatAlarm();

    return true;
}

bool 
BatInfo::parseProcAcpiBatAlarm() {

    bool ok = false;
    QRegExp rxWarnCap("^alarm:\\s*(\\d{1,5})\\s*m" + powerUnit + "h");

    // Get Alarm Fuel
    QString filename = "/proc/acpi/battery/BAT" + QString::number( batNr ) + "/alarm";
    QFile file( filename );
    if( ! file.exists() || ! file.open(IO_ReadOnly) ) {
        qDebug("could not open %s", file.name().latin1() );
        criticalFuel = 0;
        return false;
    }

    QTextStream stream( (QIODevice*) &file );
    while( ! stream.atEnd() ) {
        QString line = stream.readLine();
        if( -1 != rxWarnCap.search( line ) ) {
            QString warnCap = rxWarnCap.cap(1);
            criticalFuel = warnCap.toInt(&ok);
        }
    }
    file.close();

    if( ! ok ) {
        criticalFuel = 0;
    }

    return ok;
}

bool 
BatInfo::parseSysfsTP() {

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
        resetValues();
        return false;
//         batInstalled = false;
    }

    file.setName( tpPath + "last_full_capacity" );
    if( file.exists() && file.open(IO_ReadOnly) ) {
        stream.setDevice( &file );
        line = stream.readLine();
        if( -1 != mWh.search( line ) ) {
            lastFuel = mWh.cap(1).toInt( &check );
        }
        file.close();
    }
    else {
        lastFuel = 0;
    }

    file.setName( tpPath + "design_capacity" );
    if( file.exists() && file.open(IO_ReadOnly) ) {
        stream.setDevice( &file );
        line = stream.readLine();
        if( -1 != mWh.search( line ) ) {
            designFuel = mWh.cap(1).toInt( &check );
        }
        file.close();
    }
    else {
        designFuel = 0;
    }
    
    file.setName( tpPath + "remaining_capacity" );
    if( file.exists() && file.open(IO_ReadOnly) ) {
        stream.setDevice( &file );
        line = stream.readLine();
        if( -1 != mWh.search( line ) ) {
            curFuel = mWh.cap(1).toInt( &check );
        }
        file.close();
    }
    else {
        curFuel = -1;
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

    bool oldAcCon = isOnline();
    file.setName( "/sys/devices/platform/smapi/ac_connected" );
    if( file.exists() && file.open(IO_ReadOnly) ) {
        stream.setDevice( &file );
        acConnected = stream.readLine().toInt( &check );
        file.close();
    }
    else {
        acConnected = false;
    }
    if( oldAcCon != acConnected ) {
       emit onlineModeChanged( acConnected );
    }

    // critical Fuel can not be set via tp_smapi, so we try to read /proc/acpi for that
//     parseProcAcpiBatAlarm();

    return true;
}

void 
BatInfo::resetValues() {
    lastFuel = 0;
    designFuel = 0;
    criticalFuel = 0;
    curFuel = 0;
    curPower = 0;
    batInstalled = false;
    powerUnit = "W";
    batState = "not installed";
}
