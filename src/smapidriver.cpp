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
#include <qstring.h>
#include <qregexp.h>
#include <qfile.h>
#include <qdatetime.h>
#include <qdir.h>

// KThinkBat
#include "driverdata.h"
#include "smapidriver.h"
#include "debug.h"

SmapiDriver::SmapiDriver(const QString& smapiPrefix, const QString& batSuffix)
: BatteryDriver("smapi")
, m_smapiPrefix(smapiPrefix + "/")
, m_smapiBatPrefix(smapiPrefix + "/" + batSuffix + "/")
, m_valid(false) {

    debug("This is AcpiDriver, $Id$. Using sysfs prefix: " + m_smapiBatPrefix);
    reset();
}

void
SmapiDriver::read() {
    debug("About to read SMAPI values.");
    m_valid = parseSysfsTP();
    if (!m_valid) {
        debug(QString("Reading of SMAPI values from prefix '%1' failed. This driver is invalid.").arg(m_smapiBatPrefix));
        reset();
    }
    else {
        debug("Read values: " + m_driverData.dump());
    }
}

void
SmapiDriver::reset() {
    BatteryDriver::reset();
    m_valid = false;
}

bool
SmapiDriver::isValid() {
    return m_valid;
}

bool
SmapiDriver::parseSysfsTP() {

    m_driverData.power_unit = "W";
    QFile file;
    QTextStream stream;
    QString line;
    QRegExp mWh( "^([-]?\\d{1,6})(\\s*mWh)?\\s*$" );
    QRegExp mW( "^([-]?\\d{1,6})(\\s*mW)?\\s*$" );
    bool check;

    if (!QDir().exists(m_smapiBatPrefix)) {
        debug(QString("Prefix directory '%1' does not exists.").arg(m_smapiBatPrefix));
        static bool sayTheProblem = false;
        if (sayTheProblem) {
            qDebug( "KThinkBat: There is no directory %s. Do you have tp_smapi loaded?", m_smapiBatPrefix );
            sayTheProblem = false;
        }
        return false;
    }

    file.setName( m_smapiBatPrefix + "installed" );
    if (file.exists() && file.open(IO_ReadOnly)) {
        stream.setDevice( &file );
        m_driverData.battery_installed = ( 1 == stream.readLine().toInt( &check ) ? true : false );
        file.close();
        if(!m_driverData.battery_installed) {
            reset();
            return true;
        }
    }
    else {
        reset();
        return false;
//         batInstalled = false;
    }

    file.setName( m_smapiBatPrefix + "last_full_capacity" );
    if (file.exists() && file.open(IO_ReadOnly)) {
        stream.setDevice( &file );
        line = stream.readLine();
        if (-1 != mWh.search(line)) {
            m_driverData.last_full = mWh.cap(1).toInt( &check );
            if (!check) { m_driverData.last_full = 0; }
        }
        file.close();
    }
    else {
        m_driverData.design_full = 0;
    }

    file.setName( m_smapiBatPrefix + "design_capacity" );
    if (file.exists() && file.open(IO_ReadOnly)) {
        stream.setDevice( &file );
        line = stream.readLine();
        if (-1 != mWh.search(line)) {
            m_driverData.design_full = mWh.cap(1).toInt( &check );
            if (!check) { m_driverData.design_full = 0; }
        }
        file.close();
    }
    else {
        m_driverData.design_full = 0;
    }
    
    file.setName( m_smapiBatPrefix + "remaining_capacity" );
    if (file.exists() && file.open(IO_ReadOnly)) {
        stream.setDevice( &file );
        line = stream.readLine();
        if (-1 != mWh.search(line)) {
            m_driverData.current_full = mWh.cap(1).toInt(&check);
            if (!check) { m_driverData.current_full = 0; }
        }
        file.close();
    }
    else {
        m_driverData.current_full = -1;
    }
    
    file.setName(m_smapiBatPrefix + "power_now");
    if (file.exists() && file.open(IO_ReadOnly)) {
        stream.setDevice( &file );
        line = stream.readLine();
        if (-1 != mW.search(line)) {
            m_driverData.current_power_consumption = mW.cap(1).toInt( &check );
            if (!check) { m_driverData.current_power_consumption = 0; }
            else if (m_driverData.current_power_consumption < 0) { 
                m_driverData.current_power_consumption = (0 - m_driverData.current_power_consumption); }
        }
        file.close();
    }
    else {
        m_driverData.current_power_consumption = -1;
    }

    file.setName(m_smapiBatPrefix + "state");
    if (file.exists() && file.open(IO_ReadOnly)) {
        stream.setDevice(&file);
        m_driverData.state = stream.readLine();
        file.close();
    }
    else {
        m_driverData.state = "";
    }

    file.setName(m_smapiBatPrefix + "cycle_count");
    if (file.exists() && file.open(IO_ReadOnly)) {
        stream.setDevice(&file);
        m_driverData.cycle_count = stream.readLine().toInt(&check);
        if(!check) {
            m_driverData.cycle_count = 0;
        }
        file.close();
    }
    else {
        m_driverData.cycle_count = 0;
    }

    m_driverData.charging = (m_driverData.state == "charging");

    bool oldAcCon = isOnline();
    file.setName(m_smapiPrefix + "/ac_connected" );
    if (file.exists() && file.open(IO_ReadOnly)) {
        stream.setDevice( &file );
        m_driverData.ac_connected = stream.readLine().toInt(&check);
        file.close();
    }
    else {
        m_driverData.ac_connected = false;
    }
//     if (oldAcCon != m_driverData.ac_connected) {
//        emit onlineModeChanged(m_driverData.ac_connected);
//     }

    if (isCharging()) {
        file.setName( m_smapiBatPrefix + "remaining_charging_time" );
        if (file.exists() && file.open(IO_ReadOnly)) {
            stream.setDevice(&file);
            m_driverData.remaining_minutes = stream.readLine().toInt(&check);
            if (!check) { m_driverData.remaining_minutes = 0; }
            file.close();
        }
        else {
            calculateRemainingTime();
        }
    }
    else if (isDischarging()) {
        file.setName( m_smapiBatPrefix + "remaining_running_time" );
        if (file.exists() && file.open(IO_ReadOnly)) {
            stream.setDevice( &file );
            m_driverData.remaining_minutes = stream.readLine().toInt( &check );
            if (!check) { m_driverData.remaining_minutes = 0; }
            file.close();
        }
        else {
            calculateRemainingTime();
        }
    }
    else {
        m_driverData.remaining_minutes = 0;
    }

    // critical Fuel can not be set via tp_smapi, so we try to read /proc/acpi for that
//     parseProcAcpiBatAlarm();

    return true;
}
