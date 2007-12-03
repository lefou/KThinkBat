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
#include "acpisysfsdriver.h"
#include "driverdata.h"
#include "debug.h"

AcpiSysfsDriver::AcpiSysfsDriver(const QString& sysfsPrefix, const QString& batSuffix)
: BatteryDriver("acpi_new")
, m_sysfsAcPrefix(sysfsPrefix + "/AC/")
, m_sysfsBatPrefix(sysfsPrefix + "/" + batSuffix + "/")
, m_valid(false) {

    debug("This is AcpiSysfsDriver, $Id$. Using sysfs prefix: " + m_sysfsBatPrefix);
    reset();
}

void
AcpiSysfsDriver::read() {
    debug("About to read ACPI values.");
    QString energyOrChargePrefix = m_sysfsBatPrefix + "energy";

    m_driverData.current_full = readMyNumberAsMilli(energyOrChargePrefix + "_now", -1);

    if(-1 == m_driverData.current_full) {
        // Values in A
        energyOrChargePrefix = m_sysfsBatPrefix + "charge";
        m_driverData.current_full = readMyNumberAsMilli(energyOrChargePrefix + "_now", -1);
        if(-1 == m_driverData.current_full) {
            // No value for current energy available
            m_valid = false;
        }
        else {
            m_driverData.power_unit = "A";
            m_valid = true;
        }
    }
    else {
        // Values in W
        m_driverData.power_unit = "W";
        m_valid = true;
    }

    if (!m_valid) {
        debug("Reading of ACPI values from prefix '" + m_sysfsBatPrefix + "' failed. This driver is invalid.");
        reset();
        m_valid = false;
        return;
    }

    m_driverData.last_full = readMyNumberAsMilli(energyOrChargePrefix + "_full", 0);
    m_driverData.design_full = readMyNumberAsMilli(energyOrChargePrefix + "_full_design", 0);

    m_driverData.current_power_consumption = readMyNumberAsMilli(m_sysfsBatPrefix + "current_now", 0);

    m_driverData.battery_installed = (1 == readNumber(m_sysfsBatPrefix + "present", 0));

    m_driverData.ac_connected = (1 == readNumber(m_sysfsAcPrefix + "online", 0));

    m_driverData.state = readString(m_sysfsBatPrefix + "status", "");

    debug("Read values: " + m_driverData.dump());
}

void
AcpiSysfsDriver::reset() {
    debug("Reseting driver data...");
    BatteryDriver::reset();
    m_valid = false;
}

bool
AcpiSysfsDriver::isValid() {
    return m_valid;
}

float
AcpiSysfsDriver::myToMilly(int value) {
    if(0 != value) {
        return ((float) value) / 1000;
    }
    else {
        return 0.0;
    }
}

float
AcpiSysfsDriver::readMyNumberAsMilli(const QString& filePath, float defaultValue) {
    debug(QString("Reading value from '%1' with default '%2'").arg(filePath).arg(defaultValue));
    float ret = myToMilly(readNumber(filePath, defaultValue * 1000));
    return ret;
}

