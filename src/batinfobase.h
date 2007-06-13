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

#ifndef KTHINKBATBASE_BATINFO_H
#define KTHINKBATBASE_BATINFO_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qobject.h>
#include <qstring.h>

/**
    @author Tobias Roeser <le.petit.fou@web.de>
*/
class BatInfoBase : public QObject {
    Q_OBJECT

public:

    /** Get the current charge level.
        @return the charge level between 0 and 100, or a negative value, if there are errors
    */
    virtual float getChargeLevel();

    /** Get the critcal fuel of the battery. */
    virtual float getCriticalFuel() = 0;

    /** Get the current capacity of the battery. */
    virtual float getCurFuel() = 0;

    /**
     * Get the rechrnge cycle count of the battery or -1 if not available.
     */
    virtual int getCycleCount() = 0;

    /** Get the max. design capacity of the battery. */
    virtual float getDesignFuel() = 0;

    /** Get the last fuel state of the battery. */
    virtual float getLastFuel() = 0;

    /**
     * Get the current power consumptions. The unit of this value can be retrieved 
     * with getPowerUnit().
     */
    virtual float getPowerConsumption() = 0;

    /** Get the current power consumption formated in the preferred format. */
    virtual QString getPowerConsumptionFormated();

    /** Get the power unit this battery uses. */
    virtual QString getPowerUnit() = 0;

    /** Get the remaining battery time in minutes. */
    virtual int getRemainingTimeInMin() = 0;

    /** Get the remaining time as string in h:mm format. */
    virtual QString getRemainingTimeInHours();

    /** Get the remaining time as string in the preferred (configurred) format. */
    virtual QString getRemainingTimeFormated();

    /** Get the current battery state. */
    virtual QString getState() = 0;

    /** Returns true if the battery is charging. */
    virtual bool isCharging() = 0;

    /** Returns true if the battery is discarging. */
    virtual bool isDischarging();

    /** Return true if the battery is full. */
    virtual bool isFull();

    /** Return true if the battery is idle, thus neighter charging nor discharging. */
    virtual bool isIdle();

    /** Returns true if the battery is installed. */
    virtual bool isInstalled() = 0;

    /** Returns true if the battery is online, means AC connected. */
    virtual bool isOnline() = 0;

    /** Refresh the battery values. */
    virtual void refresh() = 0;

    /** Reset all values of the battery. */
    virtual void reset() = 0;

    /**
     * Format the given power and unit into a representable string 
     * with the right pricision according on configruation.
     */
    static QString formatPowerUnit(float power, const QString& powerUnit);

    /** Format the given time into a representable string according to configuration. */
    static QString formatRemainingTime(int timInMin);

    /** Calculates the remaining time in minutes for the given (one or two) batterie(s). */
    static int calculateRemainingTimeInMinutes(BatInfoBase* batInfo1, BatInfoBase* batInfo2 = 0L);

signals:
    /** Signal that is emitted if the online Mode of the battery has changed. */
    void onlineModeChanged(bool batOnline);

};

#endif // KTHINKBATBASE_BATINFO_H
