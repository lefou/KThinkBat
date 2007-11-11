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
#ifndef KTHINKBAT_DRIVERDATA_H
#define KTHINKBAT_DRIVERDATA_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qstring.h>

/**
    @author Tobias Roeser <le.petit.fou@web.de>
*/
class DriverData {

public:

    DriverData();

    virtual void reset();

    virtual QString dump();

    bool battery_installed;
    bool ac_connected;
    bool charging;

    float critical_full;
    float current_full;
    float design_full;
    float last_full;
    float current_power_consumption;

    QString power_unit;
    QString state;

    int cycle_count;
    int remaining_minutes;

};

#endif // KTHINKBAT_DRIVERDATA_H
