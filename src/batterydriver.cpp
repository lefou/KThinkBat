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
#include <qfile.h>
#include <qstring.h>
#include <qtextstream.h>

// // KThinkBat
#include "batterydriver.h"
#include "driverdata.h"
#include "debug.h"

BatteryDriver::BatteryDriver(const QString& driverName)
: m_driverData(DriverData()) 
, m_driverName(driverName) {
    debug("This is BatteryDriver '" + m_driverName + "', $Id$");
    reset();
}

float
BatteryDriver::getCriticalFuel() {
    return m_driverData.critical_full;
}

float
BatteryDriver::getCurFuel() {
    return m_driverData.current_full;
}

int
BatteryDriver::getCycleCount() {
    return m_driverData.cycle_count;
}

float
BatteryDriver::getDesignFuel() {
    return m_driverData.design_full;
}

float
BatteryDriver::getLastFuel() {
    return m_driverData.last_full;
}

float
BatteryDriver::getPowerConsumption() {
    return m_driverData.current_power_consumption;
}

QString
BatteryDriver::getPowerUnit() {
    return m_driverData.power_unit;
}

int
BatteryDriver::getRemainingTimeInMin() {
    return m_driverData.remaining_minutes;
}

QString
BatteryDriver::getState() {
    return m_driverData.state;
}

bool
BatteryDriver::isCharging() {
    return m_driverData.charging;
}

bool
BatteryDriver::isInstalled() {
    return m_driverData.battery_installed;
}

bool
BatteryDriver::isOnline() {
    return m_driverData.ac_connected;
}

void
BatteryDriver::read() {
    // Default impl. does nothing.
    debug("Possible programming error! The called method BatteryDriver::read() from does nothing.");
}

void
BatteryDriver::reset() {
    m_driverData.reset();
    m_remTimeForecastCap = 0;
}

bool 
BatteryDriver::isDischarging() { 
    return isInstalled() && !isOnline() && !isCharging();
}

bool
BatteryDriver::isValid() {
    return false;
}

QString
BatteryDriver::name() {
    return m_driverName;
}

void
BatteryDriver::calculateRemainingTime() {

    int remainingTime = 0;

    // Calculate remaining time
    if( isDischarging() ) {
        // discharging
        if( getCurFuel() > 0 && getPowerConsumption() > 0 ) {
            debug("Calculating remaining time based on backend reported power consumption.");
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

    m_driverData.remaining_minutes = remainingTime;
}

int
BatteryDriver::readNumber(const QString& filePath, int defaultValue) {

    QFile file(filePath);
    bool check = false;
    int result;

    if (file.exists() && file.open(IO_ReadOnly)) {
        QTextStream stream(&file);
        result = stream.readLine().toInt(&check);
        file.close();
    }
    else {
        debug(QString.("Could not read file '%1'.").arg(filePath));
    }

    return check ? result : defaultValue;
}
