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
#ifndef KTHINKBAT_BATTERYDRIVER_H
#define KTHINKBAT_BATTERYDRIVER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt
class QString;
#include <qdatetime.h>

// KThinkBat
#include "driverdata.h"

/**
    @author Tobias Roeser <le.petit.fou@web.de>
*/
class BatteryDriver {

public:

    BatteryDriver(const QString& driverName);

    /** The name of the battery driver. */
    QString name();

    /** Get the critcal fuel of the battery. */
    virtual float getCriticalFuel();

    /** Get the current capacity of the battery. */
    virtual float getCurFuel();

    /**
     * Get the rechrnge cycle count of the battery or -1 if not available.
     */
    virtual int getCycleCount();

    /** Get the max. design capacity of the battery. */
    virtual float getDesignFuel();

    /** Get the last fuel state of the battery. */
    virtual float getLastFuel();

    /**
     * Get the current power consumptions. The unit of this value can be retrieved 
     * with getPowerUnit().
     */
    virtual float getPowerConsumption();

    /** Get the power unit this battery uses. */
    virtual QString getPowerUnit();

    /** Get the remaining battery time in minutes. */
    virtual int getRemainingTimeInMin();

    /** Get the current battery state. */
    virtual QString getState();

    /** Returns true if the battery is charging. */
    virtual bool isCharging();

    /** Returns true if the battery is discarging. */
    virtual bool isDischarging();

    /** Returns true if the battery is installed. */
    virtual bool isInstalled();

    /** Returns true if the battery is online, means AC connected. */
    virtual bool isOnline();

    /** Refresh the battery values. */
    virtual void read();

    /** Reset all values of the battery. */
    virtual void reset();

    /** Return true, if the driver reports usable (valid) data. */
    virtual bool isValid();

protected:
    void calculateRemainingTime();

    DriverData m_driverData;

    QString m_driverName;

    QTime m_remTimeForecastTimestamp;
    float m_remTimeForecastCap;

};

#endif // KTHINKBAT_BATTERYDRIVER_H
