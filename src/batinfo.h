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
        @return the carge level between 0 and 100, or a negative value, if there are errors
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

    float getPowerConsumption() { return curPower; }

    QString getPowerConsumptionFormated();

    /** Get the power unit this battery uses. 
     */
    QString getPowerUnit() { return powerUnit; }

    int getRemainingTimeInMin() { return remainingTime; }
    QString getRemainingTimeInHours();

    void invalidateAll();

    /** Returns @true if the battery is installed. */
    bool isInstalled() { return batInstalled; }

    /** Returns @true if the Laptop is online, means AC connected. */
    bool isOnline() { return isInstalled() && acConnected; }

    bool isCharging() { return isInstalled() && isOnline() && batCharging; }

    bool isDischarging() { return isInstalled() && ! isOnline() && ! batCharging; }

    bool isFull() { return  isInstalled() && 100.0 == getChargeLevel() && !isCharging() && !isDischarging(); }

    /** Get the current battery state. */
    QString getState() { return batState; }

    /// Get battery info from /proc/acpi interface.
    bool parseProcACPI();
    bool parseProcAcpiBatAlarm();

    /// Get battery info form tp_smapi sysfs interface.
    bool parseSysfsTP();

    void setBatNr( int number ) { batNr = number; }

    QString getLastSuccessfulReadMethod() { return lastSuccessfulReadMethod; }

signals:
    void onlineModeChanged( bool batOnline );

protected:
    void resetValues();

    void calculateRemainingTime();

private:
    float lastFuel;
    float designFuel;
    float criticalFuel;
    float curFuel;
    float curPower;
    int remainingTime;

    int batNr;
    bool batInstalled;
    bool batCharging;

    /** Note: On Acer and Asus Laptops we have mA(h) instead of mW(h), 
        so @c powerUnit is "W" or "A", depending on the battery and 
        laptop type. */
    QString powerUnit;
    QString batState;
    bool acConnected;

    QString lastSuccessfulReadMethod;
};

#endif // KTHINKBAT_BATINFO_H
