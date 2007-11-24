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

AcpiSysfsDriver::AcpiSysfsDriver(const QString& sysfsPrefix)
: BatteryDriver("acpi_new")
, m_sysfsPrefix(sysfsPrefix + "/")
, m_valid(false) {

    debug("This is NewAcpiDriver, $Id$. Using sysfs prefix: " + m_sysfsPrefix);
    reset();
}

void
AcpiSysfsDriver::read() {
    debug("About to read ACPI values.");
    QString energyOrChargePrefix = m_sysfsPrefix + "energy";

    m_driverData.current_full = readMyNumberAsMilli(energyOrChargePrefix + "_now", -1);

    if(-1 == m_driverData.current_full) {
        // Values in A
        energyOrChargePrefix = m_sysfsPrefix + "charge";
        m_driverData.current_full = readMyNumberAsMilli(energyOrChargePrefix + "_now", -1);
        if(-1 == m_driverData.current_full) {
            // No value for current energy available
            m_valid = false;
        }
        m_driverData.power_unit = "A";
    }
    else {
        // Values in W
        m_driverData.power_unit = "W";
    }

    if (!m_valid) {
        debug("Reading of ACPI values from prefix '" + m_sysfsPrefix + "' failed. This driver is invalid.");
        reset();
        m_valid = false;
        return;
    }
    else {
        m_valid = true;
    }

    m_driverData.last_full = readMyNumberAsMilli(energyOrChargePrefix + "_full", 0);
    m_driverData.design_full = readMyNumberAsMilli(energyOrChargePrefix + "_full_design", 0);

    m_driverData.current_power_consumption = readMyNumberAsMilli(m_sysfsPrefix + "current_now", 0);

    m_driverData.battery_installed = (1 == readNumber(m_sysfsPrefix + "present", 0));

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
    return myToMilly(readNumber(filePath, defaultValue * 1000));
}

