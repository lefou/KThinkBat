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
#ifndef KTHINKBAT_BATINFO_H
#define KTHINKBAT_BATINFO_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "batinfobase.h"

#include <qstring.h>
#include <qdatetime.h>

/**
    @author Tobias Roeser <le.petit.fou@web.de>
*/
class BatInfo : public BatInfoBase {

public:
    /** Constructor.
        @param number The number of the battery, starting with 1
    */
    BatInfo( int number = 1 );
    virtual ~BatInfo();

    /** Get the critcal fuel of the battery. */
    virtual float getCriticalFuel();

    /** Get the current capacity of the battery. */
    virtual float getCurFuel();

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

    /** Returns true if the battery is installed. */
    virtual bool isInstalled();

    /** Returns true if the battery is online, means AC connected. */
    virtual bool isOnline();

    /** Returns true if the battery is charging. */
    virtual bool isCharging();

    /** Returns true if the battery is discarging. */
    virtual bool isDischarging();

    /** Return true if the battery is full. */
    virtual bool isFull();

    /** Return true if the battery is idle, thus neighter charging nor discharging. */
    virtual bool isIdle();

    /** Get the current battery state. */
    virtual QString getState();

    /** Get battery info from /proc/acpi interface. */
    bool parseProcACPI();
    bool parseProcAcpiBatAlarm();

    /** Get battery info form tp_smapi sysfs interface. */
    bool parseSysfsTP();

    /** Set the number of the battery in the system (1 or 2). */
    void setBatNr(int number);

    /**
     * Returns the last successful used method to retriev battery 
     * information. This could be e.g. "ACPI" or "SMAPI".
     */
    virtual QString getLastSuccessfulReadMethod();

    /**
     * Get the rechange cycle count of the battery or -1 if not available.
     */
    virtual int getCycleCount();

    virtual void refresh();

    virtual void reset();

protected:
    void calculateRemainingTime();

    QString getAcpiFilePrefix();

    QString getSmapiFilePrefix();

private:
    float m_lastFuel;
    float m_designFuel;
    float m_criticalFuel;
    float m_curFuel;
    float m_curPower;
    /** Remaining time in minutes. */
    int m_remainingTime;
    /** Cycle count of the battery. */
    int m_cycleCount;

    QTime m_remTimeForecastTimestamp;
    float m_remTimeForecastCap;

    /** Battery number, 1 or 2. */
    int m_batNr;

    bool m_batInstalled;
    bool m_batCharging;

    /** Note: On Acer, Asus and some other laptop brands we have mA(h) 
        instead of mW(h), so @c powerUnit is "W" or "A", depending on the 
        battery and laptop type. */
    QString m_powerUnit;
    QString m_batState;
    bool m_acConnected;

    QString m_lastSuccessfulReadMethod;
};

#endif // KTHINKBAT_BATINFO_H
