/***************************************************************************
 *   Copyright (C) 2005-2006 by Tobias Roeser   *
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

    float getLastFuel() { return lastFuel; }
    float getDesignFuel() { return designFuel; }
    float getCriticalFuel() { return criticalFuel; }
    float getCurFuel() { return curFuel; }

    float getPowerConsumption() { return curPower; }

    QString getPowerUnit() { return powerUnit; }

    void invalidateAll();

    bool isInstalled() { return batInstalled; }
    bool isOnline() { return acConnected; }
    QString getState() { return batState; }

    /// Get battery info from /proc/acpi interface.
    bool parseProcACPI();
    bool parseProcAcpiBatAlarm();

    /// Get battery info form tp_smapi sysfs interface.
    bool parseSysfsTP();

signals:
    void onlineModeChanged( bool batOnline );

protected:
    void resetValues();

private:
    float lastFuel;
    float designFuel;
    float criticalFuel;
    float curFuel;
    float curPower;

    int batNr;
    bool batInstalled;

    /** Note: On Acer and Asus Laptops we have mA(h) instead of mW(h), 
        so @c powerUnit is "W" or "A", depending on the battery and 
        laptop type. */
    QString powerUnit;
    QString batState;
    bool acConnected;
};

#endif // KTHINKBAT_BATINFO_H
