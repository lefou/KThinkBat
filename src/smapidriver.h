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
#ifndef KTHINKBAT_SMAPIDRIVER_H
#define KTHINKBAT_SMAPIDRIVER_H

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
class SmapiDriver : public BatteryDriver {

public:

    SmapiDriver(const QString& smapiPrefix, const QString& batSuffix);

    virtual void read();

    virtual void reset();

    virtual bool isValid();

private:

    bool parseSysfsTP();

    QString m_smapiPrefix;
    QString m_smapiBatPrefix;

    bool m_valid;
};

#endif // KTHINKBAT_SMAPIDRIVER_H
