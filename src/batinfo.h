/***************************************************************************
 *   Copyright (C) 2005 by Tobias Roeser   *
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

class BatInfo {

public:
    BatInfo();
    virtual ~BatInfo();

    /// Charge level is between 0 and 100, or negative, if there are errors
    float getChargeLevel();

    float getLastFuell() { return lastFuell; }
    float getDesignFuell() { return designFuell; }

    float getPowerConsumption() { return curPower; }

    QString getPowerUnit() { return powerUnit; }

    void invalidateAll();

    bool isOnline() { return batState == "charging" || batState == "charged"; }
    QString getState() { return batState; }

    bool parseProcACPI();

protected:

private:
    float lastFuell;
    float designFuell;
    float criticalFuell;
    float curFuell;
    float curPower;

    int batNr;

    QString powerUnit;
    QString batState;
};

#endif // KTHINKBAT_BATINFO_H
