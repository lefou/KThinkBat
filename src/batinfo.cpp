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

#include <qstring.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qfile.h>
#include <qdir.h>
#include <qtextstream.h>

#include <klocale.h>

#include "batinfo.h"
#include "kthinkbatconfig.h"

BatInfo::BatInfo( int number ) 
: m_batNr(number) {

    reset();
}

BatInfo::~BatInfo() {
}

void 
BatInfo::reset() {
    m_lastFuel = 0;
    m_designFuel = 0;
    m_criticalFuel = 0;
    m_curFuel = 0;
    m_curPower = 0;
    m_remainingTime = 0;
    m_cycleCount = 0;
    m_batInstalled = false;
    m_batCharging = false;
    m_powerUnit = "W";
    m_batState = "not installed";
    m_lastSuccessfulReadMethod = "";
    m_remTimeForecastCap = 0;
}

bool
BatInfo::parseProcACPI() {

    QString filePrefix = getAcpiFilePrefix() + "/";

    // not evalueated at the current state
    m_cycleCount = 0;

    // the currently read line
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

    QFile file( filePrefix + "info" );
    if (!file.exists() || !file.open(IO_ReadOnly)) {
        // this is nothing unexpected, so say it only once
        static bool sayTheProblem = true;
        if (sayTheProblem) {
            qDebug( "KThinkBat: could not open %s", file.name().latin1() );
            sayTheProblem = false;
        }
        reset();
        return false;
    }

    // we assume, that no battery is installed rather than beeing optimistic, see Ticket #11
    m_batInstalled = false;

    QTextStream stream( (QIODevice*) &file );
    while (!stream.atEnd()) {
        // parse output line-wise
        line = stream.readLine();
        if (-1 != rxInstalled.search( line ) && "yes" == rxInstalled.cap(1)) {
            m_batInstalled = true;
        }
        if (-1 != rxCap.search(line)) {
            cap = rxCap.cap(1);
            m_powerUnit = rxCap.cap(2);
        }
        if (-1 != rxDesignCap.search(line)) {
            designCap = rxDesignCap.cap(1);
        }
        if (-1 != rxDesignCapLow.search(line)) {
            criticalCap = rxDesignCapLow.cap(1);
        }
    }
    file.close();

    if (!m_batInstalled) {
        reset();
        m_batInstalled = false;
        // we return true, as no other backend should detect the fact, that there is no battery again.
        return true;
    }

    file.setName(filePrefix + "state");
    if (!file.exists() || !file.open(IO_ReadOnly)) {
        static bool sayTheProblem2 = true;
        if( sayTheProblem2 ) {
            qDebug( "KThinkBat: could not open %s", file.name().latin1() );
            sayTheProblem2 = false;
        }
        reset();
        return false;
    }
    stream.setDevice( &file );

    // Pattern for /proc/acpi/battery/BATx/state
    QRegExp rxCur("^remaining capacity:\\s*(\\d{1,5})\\s*m" + m_powerUnit + "h");
    QRegExp rxOffline("^charging state:\\s*(discharging|charging|charged|not installed)");
    QRegExp rxMWH("^present rate:\\s*(\\d{1,5})\\s*m" + m_powerUnit );

    while( ! stream.atEnd() ) {
        line = stream.readLine();
        if( -1 != rxCur.search( line ) ) {
            cur = rxCur.cap(1);
            //KMessageBox::information(0, "found cur: "+cur);
        }
        if( -1 != rxOffline.search( line ) ) {
            m_batState = rxOffline.cap(1);
            // we check again for a not installed battery, see Ticket #11
            if( "not installed" == m_batState ) {
                reset();
                m_batInstalled = false;
                // we return true, as we know that no battery is installed and
                // no other backend should detect the fact again.
                return true;
            }
        }
        if( -1 != rxMWH.search( line ) ) {
            mWHstring = rxMWH.cap(1);
        }
    }
    file.close();

    // Read current capacity
    bool ok = true;
    m_curFuel = cur.toInt(&ok);
    if (!ok) { m_curFuel = 0; }

    // Read last full capacity
    ok = true;
    m_lastFuel = cap.toInt(&ok);
    if (!ok) { m_lastFuel = 0; }

    // Read battery design capacity
    ok = true;
    m_designFuel = designCap.toInt(&ok);
    if (!ok) { m_designFuel = 0; }

    // Read battery disign critical capacity
    ok = true;
    m_criticalFuel = criticalCap.toInt(&ok);
    if (!ok) { m_criticalFuel = 0; }

    // current cosumption
    ok = true;
    m_curPower = mWHstring.toInt(&ok);
    if (!ok) { m_curPower = 0; }

    // TODO better read /proc/acpi/ac_adapter/AC/state and evaluate "on-line"
    bool oldAcCon = isOnline();
    m_acConnected = (m_batState != "discharging");
    if( oldAcCon != m_acConnected ) {
        emit onlineModeChanged( m_acConnected );
    }

    // no need to read these values as we dont use them currently
    // parseProcAcpiBatAlarm();

    calculateRemainingTime();

    m_lastSuccessfulReadMethod = "ACPI";
    return true;
}


void
BatInfo::calculateRemainingTime() {

    int remainingTime = 0;

    // Calculate remaining time
    if( isDischarging() ) {
        // discharging
        if( getCurFuel() > 0 && getPowerConsumption() > 0 ) {
            double remain = getCurFuel() / getPowerConsumption();
            remainingTime = (int) (remain * 60.0);
        }
        else {
            // the laptop provides no (usable) current power
            // consumption values, which means we can not easily calculate the remaining
            // time. See Ticket #13

            // Calculate remaining time the hard way without using the current power 
            // consumption which means we have to  the short history of this battery and 
            // make a forecast.
            // TODO find good shapshots for reliable forcasts.

            // 1. make a new pair (timestamp, capacity)
            // 2. if an older pair is knows calculate the delta (time gone, capacity gone) and make a forcast based on last delta
            // 3. repeat with 1.

            // Idea: remember more than one pair to straigten out consumption pitches

            // FIXME prove of concept code
            if(m_remTimeForecastCap <= 0 ) {
                // Take a new shapshot
                m_remTimeForecastTimestamp = QTime::currentTime();
                m_remTimeForecastCap = getCurFuel();
            }
            else {
                int secsGone = m_remTimeForecastTimestamp.secsTo(QTime::currentTime());
                float capGone = m_remTimeForecastCap - getCurFuel();
                if( secsGone > 1 && capGone > 0 ) {
                    float secsPerCap = ((float) secsGone) / capGone;
                    remainingTime = (int) ((getCurFuel() * secsPerCap) / 60);
                }
            }
        }
    }
    else {
        // not charging
        m_remTimeForecastCap = 0;
        if (isCharging()) {
            if (getPowerConsumption() > 0 && (getLastFuel() - getCurFuel()) > 0) {
                double remain = (getLastFuel() - getCurFuel()) / getPowerConsumption();
                remainingTime = (int) (remain * 60.0);
            }
        }
    }

    this->m_remainingTime = remainingTime;
}

bool 
BatInfo::parseProcAcpiBatAlarm() {

    bool ok = false;
    QRegExp rxWarnCap("^alarm:\\s*(\\d{1,5})\\s*m" + m_powerUnit + "h");

    // Get Alarm Fuel
    QString filename = getAcpiFilePrefix() + "/alarm";
    QFile file(filename);
    if (!file.exists() || !file.open(IO_ReadOnly)) {
        m_criticalFuel = 0;
        return false;
    }

    QTextStream stream((QIODevice*) &file);
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        if (-1 != rxWarnCap.search(line)) {
            QString warnCap = rxWarnCap.cap(1);
            m_criticalFuel = warnCap.toInt(&ok);
        }
    }
    file.close();

    if (!ok) {
        m_criticalFuel = 0;
    }

    return ok;
}

bool 
BatInfo::parseSysfsTP() {

    m_powerUnit = "W";
    const QString tpPath = getSmapiFilePrefix() + "/";
    QFile file;
    QTextStream stream;
    QString line;
    QRegExp mWh( "^([-]?\\d{1,6})(\\s*mWh)?\\s*$" );
    QRegExp mW( "^([-]?\\d{1,6})(\\s*mW)?\\s*$" );
    bool check;

    if (!QDir().exists(KThinkBatConfig::smapiPath())) {
        static bool sayTheProblem = true;
        if (sayTheProblem) {
            qDebug( "KThinkBat: There is no directory %s. Do you have tp_smapi loaded?", QString(KThinkBatConfig::smapiPath()).latin1() );
            sayTheProblem = false;
        }
        return false;
    }

    file.setName( tpPath + "installed" );
    if (file.exists() && file.open(IO_ReadOnly)) {
        stream.setDevice( &file );
        m_batInstalled = ( 1 == stream.readLine().toInt( &check ) ? true : false );
        file.close();
        if(!m_batInstalled) {
            reset();
            return true;
        }
    }
    else {
        reset();
        return false;
//         batInstalled = false;
    }

    file.setName( tpPath + "last_full_capacity" );
    if (file.exists() && file.open(IO_ReadOnly)) {
        stream.setDevice( &file );
        line = stream.readLine();
        if (-1 != mWh.search(line)) {
            m_lastFuel = mWh.cap(1).toInt( &check );
            if (!check) { m_lastFuel = 0; }
        }
        file.close();
    }
    else {
        m_lastFuel = 0;
    }

    file.setName( tpPath + "design_capacity" );
    if (file.exists() && file.open(IO_ReadOnly)) {
        stream.setDevice( &file );
        line = stream.readLine();
        if (-1 != mWh.search(line)) {
            m_designFuel = mWh.cap(1).toInt( &check );
            if (!check) { m_designFuel = 0; }
        }
        file.close();
    }
    else {
        m_designFuel = 0;
    }
    
    file.setName( tpPath + "remaining_capacity" );
    if (file.exists() && file.open(IO_ReadOnly)) {
        stream.setDevice( &file );
        line = stream.readLine();
        if (-1 != mWh.search(line)) {
            m_curFuel = mWh.cap(1).toInt(&check);
            if (!check) { m_curFuel = 0; }
        }
        file.close();
    }
    else {
        m_curFuel = -1;
    }
    
    file.setName(tpPath + "power_now");
    if (file.exists() && file.open(IO_ReadOnly)) {
        stream.setDevice( &file );
        line = stream.readLine();
        if (-1 != mW.search(line)) {
            m_curPower = mW.cap(1).toInt( &check );
            if (!check) { m_curPower = 0; }
            else if (m_curPower < 0) { m_curPower = (0 - m_curPower); }
        }
        file.close();
    }
    else {
        m_curPower = -1;
    }

    file.setName(tpPath + "state");
    if (file.exists() && file.open(IO_ReadOnly)) {
        stream.setDevice(&file);
        m_batState = stream.readLine();
        file.close();
    }
    else {
        m_batState = "";
    }

    file.setName(tpPath + "cycle_count");
    if (file.exists() && file.open(IO_ReadOnly)) {
        stream.setDevice(&file);
        m_cycleCount = stream.readLine().toInt(&check);
        if(!check) {
            m_cycleCount = 0;
        }
        file.close();
    }
    else {
        m_cycleCount = 0;
    }

    m_batCharging = (m_batState == "charging");

    bool oldAcCon = isOnline();
    file.setName( KThinkBatConfig::smapiPath() + "/ac_connected" );
    if (file.exists() && file.open(IO_ReadOnly)) {
        stream.setDevice( &file );
        m_acConnected = stream.readLine().toInt(&check);
        file.close();
    }
    else {
        m_acConnected = false;
    }
    if (oldAcCon != m_acConnected) {
       emit onlineModeChanged(m_acConnected);
    }

    if (isCharging()) {
        file.setName( tpPath + "remaining_charging_time" );
        if (file.exists() && file.open(IO_ReadOnly)) {
            stream.setDevice(&file);
            m_remainingTime = stream.readLine().toInt(&check);
            if (!check) { m_remainingTime = 0; }
            file.close();
        }
        else {
            calculateRemainingTime();
        }
    }
    else if (isDischarging()) {
        file.setName( tpPath + "remaining_running_time" );
        if (file.exists() && file.open(IO_ReadOnly)) {
            stream.setDevice( &file );
            m_remainingTime = stream.readLine().toInt( &check );
            if (!check) { m_remainingTime = 0; }
            file.close();
        }
        else {
            calculateRemainingTime();
        }
    }
    else {
        m_remainingTime = 0;
    }

    // critical Fuel can not be set via tp_smapi, so we try to read /proc/acpi for that
//     parseProcAcpiBatAlarm();

    m_lastSuccessfulReadMethod = "SMAPI";
    return true;
}

int 
BatInfo::getRemainingTimeInMin() {
    return m_remainingTime;
}

QString
BatInfo::getAcpiFilePrefix() {
    if(KThinkBatConfig::overridePowerSettings()) {
        return KThinkBatConfig::acpiBatteryPath() + "/"
            + ((1 == m_batNr) ? KThinkBatConfig::acpiBat1Dir() : KThinkBatConfig::acpiBat2Dir());
    }
    else {
        return "/proc/acpi/battery/BAT" + QString::number(m_batNr -1);
    }
}

QString
BatInfo::getSmapiFilePrefix() {
    if(KThinkBatConfig::overridePowerSettings()) {
        return KThinkBatConfig::smapiPath() + "/BAT" + QString::number(m_batNr-1);
    }
    else {
        return "/sys/devices/platform/smapi/BAT" + QString::number(m_batNr - 1);
    }

}

void
BatInfo::refresh() {

    bool overrideSettings = KThinkBatConfig::overridePowerSettings();
    bool enableSmapi = !overrideSettings || KThinkBatConfig::enableSmapi();
    bool enableAcpi = !overrideSettings || KThinkBatConfig::enableAcpi();

    // 1. First try TP SMAPI
    // 2. If that fails try ACPI /proc
    bool success = (enableSmapi && parseSysfsTP()) || (enableAcpi && parseProcACPI());
    if (!success) {
        reset();
    }
}

float 
BatInfo::getCriticalFuel() {
    return m_criticalFuel;
}

float 
BatInfo::getCurFuel() {
    return m_curFuel;
}

float 
BatInfo::getDesignFuel() {
    return m_designFuel;
}

float 
BatInfo::getLastFuel() {
    return m_lastFuel;
}

float 
BatInfo::getPowerConsumption() {
    return m_curPower;
}

QString 
BatInfo::getPowerUnit() {
    return m_powerUnit;
}

bool 
BatInfo::isInstalled() {
    return m_batInstalled;
}

bool 
BatInfo::isOnline() {
    return isInstalled() && m_acConnected;
}

bool 
BatInfo::isCharging() {
    return isInstalled() && isOnline() && m_batCharging;
}

bool 
BatInfo::isDischarging() { 
    return isInstalled() && !isOnline() && !m_batCharging;
}

bool 
BatInfo::isFull() { 
    return isIdle() && 100.0 == getChargeLevel(); 
}

bool 
BatInfo::isIdle() { 
    return isInstalled() && !isCharging() && !isDischarging(); 
}

QString 
BatInfo::getState() {
    return m_batState;
}

QString 
BatInfo::getLastSuccessfulReadMethod() {
    return m_lastSuccessfulReadMethod;
}

int 
BatInfo::getCycleCount() {
    return m_cycleCount;
}

void 
BatInfo::setBatNr(int number) { 
    m_batNr = number; 
}
