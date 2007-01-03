/***************************************************************************
 *   Copyright (C) 2005-2007 by Tobias Roeser   *
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
#include "kthinkbatconfig.h"

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

    // the current line while parsing the proc-fs
    QString line = "";
    QString mWHstring = "";
    QString cur = "";
    QString cap = "";
    QString designCap = "";
    QString criticalCap = "";

    // Pattern für /proc/acpi/battery/BATx/info
    QRegExp rxInstalled( "^present:\\s*(yes)" );
    QRegExp rxCap("^last full capacity:\\s*(\\d{1,5})\\s*m(A|W)h");
    QRegExp rxDesignCap("^design capacity:\\s*(\\d{1,5})\\s*m(A|W)h");
    QRegExp rxDesignCapLow("^design capacity warning:\\s*(\\d{1,5})\\s*m(A|W)h");

    QString filename = KThinkBatConfig::acpiBatteryPath() + "/BAT" + QString::number(batNr) + "/info";
    QFile file( filename );
    if( ! file.exists() || ! file.open(IO_ReadOnly) ) {
        // this is nothing unexpected, so say it only once
        static bool sayTheProblem = true;
        if( sayTheProblem ) {
            qDebug( "KThinkBat: could not open %s", file.name().latin1() );
            sayTheProblem = false;
        }
        resetValues();
        return false;
    }

    // we assume, that no battery is installed rather than beeing optimistic, see Ticket #11
    batInstalled = false;

    QTextStream stream( (QIODevice*) &file );
    while( ! stream.atEnd() ) {
        // parse output line-wise
        line = stream.readLine();
        if( -1 != rxInstalled.search( line ) && "yes" == rxInstalled.cap(1) ) {
            batInstalled = true;
        }
        if( -1 != rxCap.search( line ) ) {
            cap = rxCap.cap(1);
            powerUnit = rxCap.cap(2);
        }
        if( -1 != rxDesignCap.search( line ) ) {
            designCap = rxDesignCap.cap(1);
        }
        if( -1 != rxDesignCapLow.search( line ) ) {
            criticalCap = rxDesignCapLow.cap(1);
        }
    }
    file.close();

    if( ! batInstalled ) {
        resetValues();
        batInstalled = false;
        // we return true, as no other backend should detect the fact, that there is no battery again.
        return true;
    }

    filename = KThinkBatConfig::acpiBatteryPath() + "/BAT" + QString::number(batNr) + "/state";
    file.setName( filename );
    if( ! file.exists() || ! file.open(IO_ReadOnly) ) {
        static bool sayTheProblem2 = true;
        if( sayTheProblem2 ) {
            qDebug( "KThinkBat: could not open %s", file.name().latin1() );
            sayTheProblem2 = false;
        }
        resetValues();
        return false;
    }
    stream.setDevice( &file );

    // Pattern for /proc/acpi/battery/BATx/state
    QRegExp rxCur("^remaining capacity:\\s*(\\d{1,5})\\s*m" + powerUnit + "h");
    QRegExp rxOffline("^charging state:\\s*(discharging|charging|charged|not installed)");
    QRegExp rxMWH("^present rate:\\s*(\\d{1,5})\\s*m" + powerUnit );

    while( ! stream.atEnd() ) {
        line = stream.readLine();
        if( -1 != rxCur.search( line ) ) {
            cur = rxCur.cap(1);
            //KMessageBox::information(0, "fount cur: "+cur);
        }
        if( -1 != rxOffline.search( line ) ) {
            batState = rxOffline.cap(1);
            // we check again for a not installed battery, see Ticket #11
            if( "not installed" == batState ) {
                resetValues();
                batInstalled = false;
                // we return true, as no other backend should detect the fact, that there is no battery again.
                return true;
            }
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

    ok = true;
    designFuel = designCap.toInt(&ok);
    if( ! ok ) { designFuel = 0; }

    ok = true;
    criticalFuel = criticalCap.toInt(&ok);
    if( ! ok ) { criticalFuel = 0; }

    // current cosumption
    ok = true;
    curPower = mWHstring.toInt(&ok);
    if( ! ok ) { curPower = 0; }

    // TODO better read /proc/acpi/ac_adapter/AC/state and evaluate "on-line"
    bool oldAcCon = isOnline();
    acConnected = (batState != "discharging");
    if( oldAcCon != acConnected ) {
        emit onlineModeChanged( acConnected );
    }

//     parseProcAcpiBatAlarm();

    calculateRemainingTime();

    lastSuccessfulReadMethod = "ACPI";
    return true;
}

void
BatInfo::calculateRemainingTime() {

    // Calculate remaining time
    if( isDischarging() ) {
        if( getCurFuel() > 0 && getPowerConsumption() > 0 ) {
            double remain = getCurFuel() / getPowerConsumption();
            remainingTime = (int) (remain * 60.0);
        }
        else {
            remainingTime = 0;
        }
    }
    else if( isCharging() ) {
        if( getPowerConsumption() > 0 && (getLastFuel() - getCurFuel()) > 0 ) {
            double remain = (getLastFuel() - getCurFuel()) / getPowerConsumption();
            remainingTime = (int) (remain * 60.0);
        }
        else {
            remainingTime = 0;
        }
    }
    else {
        remainingTime = 0;
    }
}

bool 
BatInfo::parseProcAcpiBatAlarm() {

    bool ok = false;
    QRegExp rxWarnCap("^alarm:\\s*(\\d{1,5})\\s*m" + powerUnit + "h");

    // Get Alarm Fuel
    QString filename = KThinkBatConfig::acpiBatteryPath() + "/BAT" + QString::number( batNr ) + "/alarm";
    QFile file( filename );
    if( ! file.exists() || ! file.open(IO_ReadOnly) ) {
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

    QString tpPath = KThinkBatConfig::smapiPath() + "/BAT" + QString::number(batNr) + "/";
    QFile file;
    QTextStream stream;
    QString line;
    QRegExp mWh( "^([-]?\\d{1,6})(\\s*mWh)?\\s*$" );
    QRegExp mW( "^([-]?\\d{1,6})(\\s*mW)?\\s*$" );
    bool check;

    if( ! QDir().exists( KThinkBatConfig::smapiPath() ) ) {
        static bool sayTheProblem = true;
        if( sayTheProblem ) {
            qDebug( "KThinkBat: There is no directory %s. Do you have tp_smapi loaded?", QString(KThinkBatConfig::smapiPath()).latin1() );
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
            if( !check ) { lastFuel = 0; }
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
            if( !check ) { designFuel = 0; }
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
            if( !check ) { curFuel = 0; }
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
            if( !check ) { curPower = 0; }
            else if( curPower < 0 ) { curPower = (0 - curPower); }
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

    batCharging = ( batState == "charging" );

    bool oldAcCon = isOnline();
    file.setName( KThinkBatConfig::smapiPath() + "/ac_connected" );
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

    if( isCharging() ) {
        file.setName( tpPath + "remaining_charging_time" );
        if( file.exists() && file.open(IO_ReadOnly) ) {
            stream.setDevice( &file );
            remainingTime = stream.readLine().toInt( &check );
            if( !check ) { remainingTime = 0; }
            file.close();
        }
        else {
            calculateRemainingTime();
        }
    }
    else if( isDischarging() ) {
        file.setName( tpPath + "remaining_running_time" );
        if( file.exists() && file.open(IO_ReadOnly) ) {
            stream.setDevice( &file );
            remainingTime = stream.readLine().toInt( &check );
            if( !check ) { remainingTime = 0; }
            file.close();
        }
        else {
            calculateRemainingTime();
        }
    }
    else {
        remainingTime = 0;
    }

    // critical Fuel can not be set via tp_smapi, so we try to read /proc/acpi for that
//     parseProcAcpiBatAlarm();

    lastSuccessfulReadMethod = "SMAPI";
    return true;
}

void 
BatInfo::resetValues() {
    lastFuel = 0;
    designFuel = 0;
    criticalFuel = 0;
    curFuel = 0;
    curPower = 0;
    remainingTime = 0;
    batInstalled = false;
    batCharging = false;
    powerUnit = "W";
    batState = "not installed";
    lastSuccessfulReadMethod = "";
}

QString
BatInfo::getPowerConsumptionFormated() {

    QString formatString = "";

    if( "W" == getPowerUnit() ) {
            formatString = QString().number((int) (curPower + 500)/1000);
        }
        else {
            formatString = QString().number((float) (((int) curPower + 50)/100) / 10 );
        }

    return (formatString + " " + getPowerUnit());
}

QString 
BatInfo::getRemainingTimeInHours() {
    int min = getRemainingTimeInMin();
    int hours = min / 60;
    QString out = QString().number(hours) + ":";
    min = min - (hours * 60);
    if( min > 9 ) {
        out += QString().number(min);
    }
    else {
        out += "0" + QString().number(min);
    }
    return out;
}
