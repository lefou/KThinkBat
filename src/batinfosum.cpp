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
#include "batinfosum.h"

BatInfoSum::BatInfoSum(BatInfoBase* battery1, BatInfoBase* battery2) {
    m_bats.append(battery1);
    m_bats.append(battery2);

//     for(BatInfoBase* bat = bats.first(); bat; bat = bats.next()) {
//     }
}

float 
BatInfoSum::getCriticalFuel() {
    float fuel = 0;
    for(BatInfoBase* bat = m_bats.first(); bat; bat = m_bats.next()) {
        fuel += bat->getCriticalFuel();
    }
    return fuel;
}

float 
BatInfoSum::getCurFuel() {
    float fuel = 0;
    for(BatInfoBase* bat = m_bats.first(); bat; bat = m_bats.next()) {
        fuel += bat->getCurFuel();
    }
    return fuel;
}

int 
BatInfoSum::getCycleCount() {
    return 0;
}

float 
BatInfoSum::getDesignFuel() {
    float fuel = 0;
    for(BatInfoBase* bat = m_bats.first(); bat; bat = m_bats.next()) {
        fuel += bat->getDesignFuel();
    }
    return fuel;
}

float 
BatInfoSum::getLastFuel() {
    float fuel = 0;
    for(BatInfoBase* bat = m_bats.first(); bat; bat = m_bats.next()) {
        fuel += bat->getLastFuel();
    }
    return fuel;
}

float 
BatInfoSum::getPowerConsumption() {
   float power = 0;
    for(BatInfoBase* bat = m_bats.first(); bat; bat = m_bats.next()) {
        power += bat->getPowerConsumption();
    }
    return power;
}

QString 
BatInfoSum::getPowerUnit() {
    return m_bats.first()->getPowerUnit();
}

int 
BatInfoSum::getRemainingTimeInMin() {
    double remaining = 0;

    // The chance that there is only on battery is high, so we try to avoid
    // a recalculation of the remaining time if there is only one battery
    bool batCount = 0;
    for(BatInfoBase* bat = m_bats.first(); bat; bat = m_bats.next()) {
        if(bat->isInstalled()) {
            batCount++;
            remaining = bat->getRemainingTimeInMin();
        }
    }

    if(batCount > 1) {
        // if we have more than one battery, calculate the remaining time new

        remaining = 0;
        float curFuel = getCurFuel();
        float powerConsumption = getPowerConsumption();

        if (isDischarging()) {
            if(curFuel > 0 && powerConsumption > 0) {
                remaining = (curFuel / powerConsumption) * 60.0;
            }
        }
        else if (isCharging()) {
            float lastFuel = getLastFuel();
            if( powerConsumption > 0 && (lastFuel - curFuel) > 0 ) {
                remaining = ((lastFuel - curFuel) / powerConsumption) * 60.0;
            }
        }
    }

    return (int) remaining;
}

QString 
BatInfoSum::getState() {
    return "unknown";
}

bool
BatInfoSum::isCharging() {
    for(BatInfoBase* bat = m_bats.first(); bat; bat = m_bats.next()) {
        if(bat->isInstalled() && (!bat->isCharging() || !bat->isFull())) {
            return false;
        }
    }
    return true;
}

bool
BatInfoSum::isDischarging() {
    for(BatInfoBase* bat = m_bats.first(); bat; bat = m_bats.next()) {
        if(bat->isInstalled() && bat->isDischarging()) {
            return true;
        }
    }
    return false;
}

bool 
BatInfoSum::isInstalled() {
    for(BatInfoBase* bat = m_bats.first(); bat; bat = m_bats.next()) {
        if (bat->isInstalled()) {
            return true;
        }
    }
    return false;
}

bool 
BatInfoSum::isOnline() {
    bool isOnline = false;
    for(BatInfoBase* bat = m_bats.first(); bat; bat = m_bats.next()) {
        if(bat->isOnline()) {
            isOnline = true;
        }
    }
    return isOnline;
}

void 
BatInfoSum::refresh() {
    for(BatInfoBase* bat = m_bats.first(); bat; bat = m_bats.next()) {
        bat->refresh();
    }
}

void
BatInfoSum::reset() {
    for(BatInfoBase* bat = m_bats.first(); bat; bat = m_bats.next()) {
        bat->reset();
    }
}
