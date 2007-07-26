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

#include "batinfobase.h"

#include <klocale.h>

#include "kthinkbatconfig.h"

QString
BatInfoBase::getPowerConsumptionFormated() {
    return BatInfoBase::formatPowerUnit(getPowerConsumption(), getPowerUnit());
}

QString
BatInfoBase::getRemainingTimeFormated() {
    return BatInfoBase::formatRemainingTime(getRemainingTimeInMin());
}

QString
BatInfoBase::getRemainingTimeInHours() {
    int min = getRemainingTimeInMin();
    int hours = min / 60;
    QString out = QString().number(hours) + ":";
    min = min - (hours * 60);
    if (min > 9) {
        out += QString().number(min);
    }
    else {
        out += "0" + QString().number(min);
    }
    return out;
}

bool 
BatInfoBase::isDischarging() {
    return isInstalled() && !isOnline() && !isCharging();
}

bool 
BatInfoBase::isFull() {
    return  isIdle() && 100.0 == getChargeLevel();
}

bool 
BatInfoBase::isIdle() {
    return isInstalled() && !isCharging() && !isDischarging();
}

QString
BatInfoBase::formatRemainingTime(int timeInMin) {
    if (!KThinkBatConfig::remainingTimeInHours()) {
        return QString().number((int) timeInMin) + " min";
    }
    int hours = timeInMin / 60;
    QString out = QString().number(hours) + ":";
    int min = timeInMin - (hours * 60);
    if (min > 9) {
        out += QString().number(min);
    }
    else {
        out += "0" + QString().number(min);
    }
    return out;
}

QString
BatInfoBase::formatPowerUnit(float power, const QString& powerUnit) {

    if (power < 0 || powerUnit.isEmpty()) {
        return i18n("nA");
    }

    QString formatString = "0";
    int precision = ("W" == powerUnit) ? KThinkBatConfig::precisionPowerUnitW() : KThinkBatConfig::precisionPowerUnitA();

    if (power > 0) {
        switch (precision) {
        case 0:
            formatString = QString().number((int)(power + 500)/1000);
            break;
        case 1:
            formatString = QString().number((float) (((int)power + 50)/100) / 10 );
            break;
        case 2:
            formatString = QString().number((float) (((int)power + 5)/10) / 100 );
            break;
        case 3:
            formatString = QString().number((float) ((int)(power + 0.5)) / 1000 );
            break;
        }
    }

    // Append zeros and point if neccessary
    if (precision >= 1 && precision <= 3) {
        int pos = formatString.find('.');
        if (pos == -1) {
            formatString += ".";
            pos = 0;
        }
        else {
            pos = formatString.length() - pos - 1;
        }
        for ( ; pos < precision; ++pos) {
            formatString += "0";
        }
    }

    return (formatString + " " + powerUnit);
}

/**
 * This implementation caclulates currently no forecast if the batInfo did not report a current power consumption.
 */
int
BatInfoBase::calculateRemainingTimeInMinutes(BatInfoBase* batInfo1, BatInfoBase* batInfo2) {
    double remaining = 0;
    
    if(!batInfo1) {
        return (int) remaining;
    }

    float curFuel = batInfo1->getCurFuel();
    float lastFuel = batInfo1->getLastFuel();
    float powerConsumption = batInfo1->getPowerConsumption();

    if(batInfo2) {
        curFuel += batInfo2->getCurFuel();
        lastFuel += batInfo2->getLastFuel();
        powerConsumption += batInfo2->getPowerConsumption();
    }

    if (batInfo1->isDischarging()) {
        if(curFuel > 0 && powerConsumption > 0) {
            remaining = (curFuel / powerConsumption) * 60.0;
        }
    }
    else if (batInfo1->isCharging()) {
        if( powerConsumption > 0 && (lastFuel - curFuel) > 0 ) {
            remaining = ((lastFuel - curFuel) / powerConsumption) * 60.0;
        }
    }

    return (int) remaining;
}

float
BatInfoBase::getChargeLevel() {
    float curFuel = getCurFuel();
    float lastFuel = getLastFuel();
    if (curFuel >= 0 && lastFuel > 0) {
        return ( 100.0 / lastFuel ) * curFuel;
    }
    return -1;
}
