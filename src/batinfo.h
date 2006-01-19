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
#define LTHINKBAT_BATINFO_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qstring.h>

/**
    @author Tobias Roeser <le.petit.fou@web.de>
*/
class BatInfo {
    
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

    float getLastFuell() { return lastFuell; }
    float getDesignFuell() { return designFuell; }
    float getCriticalFuell() { return criticalFuell; }
    float getCurFuell() { return curFuell; }

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

protected:
    void resetValues();

private:
    float lastFuell;
    float designFuell;
    float criticalFuell;
    float curFuell;
    float curPower;

    int batNr;
    bool batInstalled;

    QString powerUnit;
    QString batState;
    bool acConnected;
};

#endif // KTHINKBAT_BATINFO_H
