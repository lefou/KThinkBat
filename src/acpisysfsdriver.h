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
#ifndef KTHINKBAT_ACPIDSYSFSRIVER_H
#define KTHINKBAT_ACPISYSFSDRIVER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt
class QString;
class QTime;

// KThinkBat
#include "batterydriver.h"

/**
    @author Tobias Roeser <le.petit.fou@web.de>
*/
class AcpiSysfsDriver : public BatteryDriver {

public:

    AcpiSysfsDriver(const QString& sysfsPrefix, const QString& batSuffix);

    virtual void read();

    virtual void reset();

    virtual bool isValid();

private:

    float myToMilly(int value);

    /** Read a value in My (10^-6) as milli (10^-3). */
    float readMyNumberAsMilli(const QString& filePath, float defaultValue);

    QString m_sysfsAcPrefix;
    QString m_sysfsBatPrefix;

    bool m_valid;
};

#endif // KTHINKBAT_ACPISYSFSDRIVER_H
