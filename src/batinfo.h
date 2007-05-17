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

#include <qobject.h>
#include <qstring.h>
#include <qdatetime.h>

/**
    @author Tobias Roeser <le.petit.fou@web.de>
*/
class BatInfo : public QObject {
    Q_OBJECT

public:
    /** Constructor.
        @param number The number of the battery, starting with 1
    */
    BatInfo( int number = 1 );
    virtual ~BatInfo();

    /** Get the current charge level.
        @return the charge level between 0 and 100, or a negative value, if there are errors
    */
    float getChargeLevel();

    /** Get the critcal fuel of the battery. */
    float getCriticalFuel() { return criticalFuel; }

    /** Get the current capacity of the battery. */
    float getCurFuel() { return curFuel; }

    /** Get the max. design capacity of the battery. */
    float getDesignFuel() { return designFuel; }

    /** Get the last fuel state of the battery. */
    float getLastFuel() { return lastFuel; }

    /**
     * Get the current power consumptions. The unit of this value can be retrieved 
     * with getPowerUnit().
     */
    float getPowerConsumption() { return curPower; }

    QString getPowerConsumptionFormated();

    /** Get the power unit this battery uses. */
    QString getPowerUnit() { return powerUnit; }

    /** Get the remaining battery time in minutes. */
    int getRemainingTimeInMin();

    /** Get the remaining time as string in h:mm format. */
    QString getRemainingTimeInHours();

    /** Get the remaining time as string in the preferred (configurred) format. */
    QString getRemainingTimeFormated();

    /** Returns true if the battery is installed. */
    bool isInstalled() { return batInstalled; }

    /** Returns true if the battery is online, means AC connected. */
    bool isOnline() { return isInstalled() && acConnected; }

    /** Returns true if the battery is charging. */
    bool isCharging() { return isInstalled() && isOnline() && batCharging; }

    /** Returns true if the battery is discarging. */
    bool isDischarging() { return isInstalled() && ! isOnline() && ! batCharging; }

    /** Return true if the battery is full. */
    bool isFull() { return  isIdle() && 100.0 == getChargeLevel(); }

    /** Return true if the battery is idle, thus neighter charging nor discharging. */
    bool isIdle() { return isInstalled() && !isCharging() && !isDischarging(); }

    /** Get the current battery state. */
    QString getState() { return batState; }

    /** Get battery info from /proc/acpi interface. */
    bool parseProcACPI();
    bool parseProcAcpiBatAlarm();

    /** Get battery info form tp_smapi sysfs interface. */
    bool parseSysfsTP();

    /** Set the number of the battery in the system (1 or 2). */
    void setBatNr(int number) { batNr = number; }

    /**
     * Returns the last successful used method to retriev battery 
     * information. This could be e.g. "ACPI" or "SMAPI".
     */
    QString getLastSuccessfulReadMethod() { return lastSuccessfulReadMethod; }

    /** Format the given time into a representable string according to configuration. */
    static QString formatRemainingTime(int timInMin);

    /**
     * Format the given power and unit into a representable string 
     * with the right pricision according on configruation.
     */
    static QString formatPowerUnit(float power, const QString& powerUnit);

    /**
     * Get the rechange cycle count of the battery or -1 if not available.
     */
    int getCycleCount() { return cycleCount; }

    static int calculateRemainingTimeInMinutes(BatInfo* batInfo1, BatInfo* batInfo2 = 0L);

signals:
    void onlineModeChanged(bool batOnline);

protected:
    void resetValues();

    void calculateRemainingTime();

private:
    float lastFuel;
    float designFuel;
    float criticalFuel;
    float curFuel;
    float curPower;
    /** Remaining time in minutes. */
    int remainingTime;
    /** Cycle count of the battery. */
    int cycleCount;

    QTime remTimeForecastTimestamp;
    float remTimeForecastCap;

    /** Battery number, 1 or 2. */
    int batNr;

    bool batInstalled;
    bool batCharging;

    /** Note: On Acer, Asus and some other laptop brands we have mA(h) 
        instead of mW(h), so @c powerUnit is "W" or "A", depending on the 
        battery and laptop type. */
    QString powerUnit;
    QString batState;
    bool acConnected;

    QString lastSuccessfulReadMethod;
};

#endif // KTHINKBAT_BATINFO_H
