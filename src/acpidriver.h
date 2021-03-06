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
#ifndef KTHINKBAT_ACPIDRIVER_H
#define KTHINKBAT_ACPIDRIVER_H

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
class AcpiDriver : public BatteryDriver {

public:

    AcpiDriver(const QString& procAcpiBatPrefix);

    virtual void read();

    virtual void reset();

    virtual bool isValid();

private:
    /** Parse the proc interface of the Linux Kernel (in most cases /proc/acpi/battery) and try to read the battery information. If this is successfull, return with true, if this failes or any other indication for a invalid battery/interface is determined, this method will return false.

    @return true if the parsing was successful, else false.
    */
    bool parseProcACPI();

    bool parseProcAcpiBatAlarm();

    QString m_procAcpiBatPrefix;

    bool m_valid;
};

#endif // KTHINKBAT_ACPIDRIVER_H
