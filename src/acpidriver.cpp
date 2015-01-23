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

// KThinkBat
#include "acpidriver.h"
#include "driverdata.h"
#include "debug.h"

AcpiDriver::AcpiDriver(const QString& procAcpiBatPrefix)
: BatteryDriver("acpi")
, m_procAcpiBatPrefix(procAcpiBatPrefix + "/")
, m_valid(false) {

    debug("This is AcpiDriver, $Id$. Using proc filesystem prefix: " + m_procAcpiBatPrefix);
    reset();
}

void
AcpiDriver::read() {
    debug("About to read ACPI values.");
    m_valid = parseProcACPI();
    if (!m_valid) {
        debug("Reading of ACPI values from prefix '" + m_procAcpiBatPrefix + "' failed. This driver is invalid.");
        reset();
    }
    else {
        debug("Read values: " + m_driverData.dump());
    }
}

void
AcpiDriver::reset() {
    debug("Reseting driver data...");
    BatteryDriver::reset();
    m_valid = false;
}

bool
AcpiDriver::parseProcACPI() {

    // not evalueated at the current state
    m_driverData.cycle_count = 0;

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

    QFile file( m_procAcpiBatPrefix + "info" );
    if (!file.exists() || !file.open(IO_ReadOnly)) {
        debug(QString("KThinkBat: could not open %s").arg(m_procAcpiBatPrefix + "info"));
        // this is nothing unexpected, so say it only once
        static bool sayTheProblem = false;
        if (sayTheProblem) {
            qDebug( "KThinkBat: could not open %s", file.name().latin1() );
            sayTheProblem = false;
        }
        reset();
        return false;
    }

    // we assume, that no battery is installed rather than beeing optimistic, see Ticket #11
    m_driverData.battery_installed = false;

    QTextStream stream( (QIODevice*) &file );
    while (!stream.atEnd()) {
        // parse output line-wise
        line = stream.readLine();
        if (-1 != rxInstalled.search( line ) && "yes" == rxInstalled.cap(1)) {
            m_driverData.battery_installed = true;
            debug("Battery is installed.");
        }
        if (-1 != rxCap.search(line)) {
            cap = rxCap.cap(1);
            m_driverData.power_unit = rxCap.cap(2);
            debug("Power unit: " + m_driverData.power_unit);
        }
        if (-1 != rxDesignCap.search(line)) {
            designCap = rxDesignCap.cap(1);
            trace("Design capacity: " + designCap);
        }
        if (-1 != rxDesignCapLow.search(line)) {
            criticalCap = rxDesignCapLow.cap(1);
            trace("Critical capacity: " + criticalCap);
        }
    }
    file.close();

    if (!m_driverData.battery_installed) {
        reset();
        m_driverData.battery_installed = false;
        // We return true, as no other backend should detect the fact again,
        // that there is no battery.
        debug("Battery is not installed but driver is valid.");
        return true;
    }

    file.setName(m_procAcpiBatPrefix + "state");
    if (!file.exists() || !file.open(IO_ReadOnly)) {
        debug(QString("KThinkBat: could not open '%s'.").arg(m_procAcpiBatPrefix + "state"));
        static bool sayTheProblem2 = false;
        if( sayTheProblem2 ) {
            qDebug( "KThinkBat: could not open %s", file.name().latin1() );
            sayTheProblem2 = false;
        }
        reset();
        return false;
    }
    stream.setDevice( &file );

    // Pattern for /proc/acpi/battery/BATx/state
    QRegExp rxCur("^remaining capacity:\\s*(\\d{1,5})\\s*m" + m_driverData.power_unit + "h");
    QRegExp rxOffline("^charging state:\\s*(discharging|charging|charged|not installed)");
    QRegExp rxMWH("^present rate:\\s*(\\d{1,5})\\s*m" + m_driverData.power_unit );

    while( ! stream.atEnd() ) {
        line = stream.readLine();
        if( -1 != rxCur.search( line ) ) {
            cur = rxCur.cap(1);
            //KMessageBox::information(0, "found cur: "+cur);
        }
        if( -1 != rxOffline.search( line ) ) {
            m_driverData.state = rxOffline.cap(1);
            // we check again for a not installed battery, see Ticket #11
            if( "not installed" == m_driverData.state ) {
                reset();
                m_driverData.battery_installed = false;
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
    m_driverData.current_full = cur.toInt(&ok);
    if (!ok) { m_driverData.current_full = 0; }

    // Read last full capacity
    ok = true;
    m_driverData.last_full = cap.toInt(&ok);
    if (!ok) { m_driverData.last_full = 0; }

    // Read battery design capacity
    ok = true;
    m_driverData.design_full = designCap.toInt(&ok);
    if (!ok) { m_driverData.design_full = 0; }

    // Read battery design critical capacity
    ok = true;
    m_driverData.critical_full = criticalCap.toInt(&ok);
    if (!ok) { m_driverData.critical_full = 0; }

    // Read current power cosumption
    ok = true;
    m_driverData.current_power_consumption = mWHstring.toInt(&ok);
    if (!ok) { m_driverData.current_power_consumption = 0; }

    if(m_driverData.state == "idle") {
        m_driverData.batState = DriverData::IDLE;
    } else if(m_driverData.state == "charging") {
        m_driverData.batState = DriverData::CHARGING;
    } else if(m_driverData.state == "discharging") {
        m_driverData.batState = DriverData::DISCHARGING;
    } else if(m_driverData.state == "not present") {
        m_driverData.batState = DriverData::NOT_PRESENT;
    } else {
        m_driverData.batState = DriverData::UNKNOWN;
    }

    // Is AC connected (aka online)?
    // TODO better read /proc/acpi/ac_adapter/AC/state and evaluate "on-line"
    m_driverData.ac_connected = (m_driverData.batState != DriverData::DISCHARGING
                                || m_driverData.batState == DriverData::CHARGING
                                || m_driverData.batState == DriverData::IDLE);

    // Is battery currently charging?
    m_driverData.charging = (m_driverData.batState == DriverData::CHARGING);


    // no need to read these values as we dont use them currently
    // parseProcAcpiBatAlarm();

    calculateRemainingTime();

    trace("Finish reading of ACPI values successfully.");
    return true;
}



bool 
AcpiDriver::parseProcAcpiBatAlarm() {

    bool ok = false;
    QRegExp rxWarnCap("^alarm:\\s*(\\d{1,5})\\s*m" + m_driverData.power_unit + "h");

    // Get Alarm Fuel
    QString filename = m_procAcpiBatPrefix + "/alarm";
    QFile file(filename);
    if (!file.exists() || !file.open(IO_ReadOnly)) {
        m_driverData.critical_full = 0;
        return false;
    }

    QTextStream stream((QIODevice*) &file);
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        if (-1 != rxWarnCap.search(line)) {
            QString warnCap = rxWarnCap.cap(1);
             m_driverData.critical_full = warnCap.toInt(&ok);
        }
    }
    file.close();

    if (!ok) {
         m_driverData.critical_full = 0;
    }

    return ok;
}

bool
AcpiDriver::isValid() {
    return m_valid;
}
