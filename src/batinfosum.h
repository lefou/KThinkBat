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
#ifndef KTHINKBAT_BATINFOSUM_H
#define KTHINKBAT_BATINFOSUM_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "batinfobase.h"

#include <qptrlist.h>

/**
    @author Tobias Roeser <le.petit.fou@web.de>
*/
class BatInfoSum : public BatInfoBase {

public:
    BatInfoSum(BatInfoBase* battery1, BatInfoBase* battery2);

    virtual float getCriticalFuel();

    virtual float getCurFuel();

    virtual int getCycleCount();

    virtual float getDesignFuel();

    virtual float getLastFuel();

    virtual float getPowerConsumption();

    virtual QString getPowerUnit();

    virtual int getRemainingTimeInMin();

    virtual QString getState();

    virtual bool isDischarging();

    virtual bool isCharging();

    virtual bool isInstalled();

    virtual bool isOnline();

    virtual void refresh();

    virtual void reset();

private:
    QPtrList<BatInfoBase> m_bats;

};

#endif // KTHINKBAT_BATINFOSUM_H
