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

// Qt
#include <qstring.h>

// KThinkBat
#include "driverdata.h"


DriverData::DriverData() {
    reset();
}

void
DriverData::reset() {
    battery_installed = false;
    ac_connected = false;
    charging = false;
    critical_full = 0;
    current_full = 0;
    design_full = 0;
    last_full = 0;
    current_power_consumption = 9;
    power_unit = "";
    state = "";
    cycle_count = 0;
    remaining_minutes = 0;
}

QString
DriverData::dump() {

    QString partA = QString("battery_installed = %1, ac_connected = %2, charging = %3, critical_full = %4, current_full = %5, design_full = %6, last_full = %7, current_power_consumption = %8, power_unit = %9").arg(battery_installed).arg(ac_connected).arg(charging).arg(critical_full).arg(current_full).arg(design_full).arg(last_full).arg(current_power_consumption);

    return QString("%1, power_unit = '%2', state = '%3', cycle_count = %4, remaining_minutes = %5").arg(partA).arg(power_unit).arg(state).arg(cycle_count).arg(remaining_minutes);
}
