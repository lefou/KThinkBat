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

#include <klocale.h>

#include "acpidriver.h"
#include "acpisysfsdriver.h"
#include "batinfo.h"
#include "batterydriver.h"
#include "kthinkbatconfig.h"
#include "debug.h"
#include "smapidriver.h"

BatInfo::BatInfo( int number ) 
: m_batNr(number)
, m_currentDriver(NULL)
, m_acpiDriver(NULL)
, m_smapiDriver(NULL)
, m_acpiSysfsDriver(NULL) {

    reset();
}

BatInfo::~BatInfo() {
    m_currentDriver = NULL;

    if(m_acpiDriver) {
        delete m_acpiDriver;
        m_acpiDriver = NULL;
 }

    if(m_smapiDriver) {
        delete m_smapiDriver;
        m_smapiDriver = NULL;
    }

    if(m_acpiSysfsDriver) {
        delete m_acpiSysfsDriver;
        m_acpiSysfsDriver = NULL;
    }
}

void 
BatInfo::reset() {
    m_currentDriver = NULL;
}

int 
BatInfo::getRemainingTimeInMin() {
    return m_currentDriver ? m_currentDriver->getRemainingTimeInMin() : 0;
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
        return KThinkBatConfig::smapiPath();
    }
    else {
        return "/sys/devices/platform/smapi";
    }
}

QString
BatInfo::getSmapiBatFilePrefix() {
    return "BAT" + QString::number(m_batNr - 1);
}

void
BatInfo::refresh() {

    debug("About to refresh the battery information.");

    bool overrideSettings = KThinkBatConfig::overridePowerSettings();
    bool enableSmapi = !overrideSettings || KThinkBatConfig::enableSmapi();
    bool enableAcpi = !overrideSettings || KThinkBatConfig::enableAcpi();
    bool enableAcpiSysfs = !overrideSettings || KThinkBatConfig::enableAcpiSysfs();

    QString acpiSysfsPefix = overrideSettings ? KThinkBatConfig::acpiSysfsPrefix() : "/sys/class/power_supply";

    bool success = false;

    // 1. First try TP SMAPI
    if(!success && enableSmapi) {
        debug("About to use SMAPI backend.");
        if(!m_smapiDriver) {
            debug("SMAPI driver is NULL. Instantiating a new driver.");
            m_smapiDriver = new SmapiDriver(getSmapiFilePrefix(), getSmapiBatFilePrefix());
        }
        m_smapiDriver->read();
        if(m_smapiDriver->isValid()) {
            m_currentDriver = m_smapiDriver;
            success = true;
        }
    }

    // 2. If that fails try ACPI /proc
    if(!success && enableAcpi) {
        debug("About to use ACPI /proc backend.");
        if(!m_acpiDriver) {
            debug("ACPI driver is NULL. Instantiating a new driver.");
            m_acpiDriver = new AcpiDriver(getAcpiFilePrefix());
        }
        m_acpiDriver->read();
        if(m_acpiDriver->isValid()) {
            m_currentDriver = m_acpiDriver;
            success = true;
        }
    }

    // 3. If that fails try ACPI /sys
    // TODO If the new kernel is stable for a longer time, raise the priority of this backend
    if(!success && enableAcpiSysfs) {
        debug("About to use new ACPI sysfs backend.");
        if(!m_acpiSysfsDriver) {
            debug("ACPI driver is NULL. Instantiating a new driver.");
            m_acpiSysfsDriver = new AcpiSysfsDriver(acpiSysfsPefix + "/BAT" + QString::number(m_batNr - 1));
        }
        m_acpiSysfsDriver->read();
        if(m_acpiSysfsDriver->isValid()) {
            m_currentDriver = m_acpiSysfsDriver;
            success = true;
        }
    }

    if (!success) {
        debug("Reading battery information was not successful.");
        reset();
    }
    else {
        debug("Reading of battery information was successful. Used driver was: " + m_currentDriver->name());
    }
}

float 
BatInfo::getCriticalFuel() {
    if(m_currentDriver && isInstalled()) {
        return m_currentDriver->getCriticalFuel();
    }
    else {
        return 0;
    }
}

float 
BatInfo::getCurFuel() {
    if(m_currentDriver && isInstalled()) {
        return m_currentDriver->getCurFuel();
    }
    else {
        return 0;
    }
}

float 
BatInfo::getDesignFuel() {
    if(m_currentDriver && isInstalled()) {
        return m_currentDriver->getDesignFuel();
    }
    return 0;
}

float 
BatInfo::getLastFuel() {
    if(m_currentDriver && isInstalled()) {
        return m_currentDriver->getLastFuel();
    }
    return 0;
}

float 
BatInfo::getPowerConsumption() {
    if(m_currentDriver && isInstalled()) {
        return m_currentDriver->getPowerConsumption();
    }
    return 0;
}

QString 
BatInfo::getPowerUnit() {
    return m_currentDriver ? m_currentDriver->getPowerUnit() : "";
}

bool 
BatInfo::isInstalled() {
    return m_currentDriver && m_currentDriver->isInstalled();
}

bool 
BatInfo::isOnline() {
    return m_currentDriver && m_currentDriver->isInstalled() && m_currentDriver->isOnline();
}

bool 
BatInfo::isCharging() {
    return m_currentDriver && m_currentDriver->isCharging();
}

bool 
BatInfo::isDischarging() { 
    return m_currentDriver && m_currentDriver->isCharging();
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
    return m_currentDriver ? m_currentDriver->getState() : "";
}

QString 
BatInfo::getLastSuccessfulReadMethod() {
    return m_currentDriver ? m_currentDriver->name() : "";
}

int 
BatInfo::getCycleCount() {
    if(m_currentDriver && isInstalled()) {
        return m_currentDriver->getCycleCount();
    }
    return 0;
}

void 
BatInfo::setBatNr(int number) { 
    m_batNr = number; 
}
