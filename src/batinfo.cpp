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

#include <qstring.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qfile.h>

#include "batinfo.h"

BatInfo::BatInfo() 
    : lastFuell( -1 )
    , designFuell( -1 )
    , criticalFuell( -1 )
    , curFuell( -1 )
    , curPower( -1 )
    , batNr( 0 )
    , powerUnit( "W" )
    , batState( "?" )
{
}

BatInfo::~BatInfo() {
}

float
BatInfo::getChargeLevel() {
    if( curFuell >= 0 && lastFuell > 0 ) {
        return ( 100.0 / lastFuell ) * curFuell;
    }
    return -1;
}

bool
BatInfo::parseProcACPI() {

    // die aktuelle Zeile beim parsen des proc-fs
    QString line = "";
    QString mWHstring = "";
    // normale Angaben: Kapzität, Max Kap., aktuelle Kap.
    QString cap = "", designCap = "", cur = "", warnCap = "";
    // dieselben Angaben, aber in mAh statt in mWh
    QString capA = "", designCapA = "", warnCapA = "";

    // Pattern für /proc/acpi/battery/BATx/info
    // ThinkPad (mWh)
    static QRegExp rxCap("^last full capacity:\\s*(\\d{1,5})\\s*mWh");
    static QRegExp rxDesignCap("^design capacity low:\\s*(\\d{1,5})\\s*mWh");
    static QRegExp rxWarnCap("^design capacity warning:\\s*(\\d{1,5})\\s*mWh");
    // Asus (mAh)
    static QRegExp rxCapA("^last full capacity:\\s*(\\d{1,5})\\s*mAh");
    static QRegExp rxDesignCapA("^design capacity low:\\s*(\\d{1,5})\\s*mAh");
    static QRegExp rxWarnCapA("^design capacity warning:\\s*(\\d{1,5})\\s*mAh");

    QFile file( "/proc/acpi/battery/BAT" + QString::number(batNr) + "/info" );
    if( ! file.exists() || ! file.open(IO_ReadOnly) ) {
       curFuell = -1;
       qDebug( "could not open %s", file.name().latin1() );
       return false;
    }
    QTextStream stream( (QIODevice*) &file );

    while( ! stream.atEnd() ) {
        line = stream.readLine();
        if( -1 != rxCap.search( line ) ) {
            cap = rxCap.cap(1);
            //KMessageBox::information(0, "fount cap: "+cap);
        }
        if( -1 != rxDesignCap.search( line ) ) {
            designCap = rxDesignCap.cap(1);
        }
        if( -1 != rxWarnCap.search( line ) ) {
            warnCap = rxWarnCap.cap(1);
        }
        if( -1 != rxCapA.search( line ) ) {
            capA = rxCapA.cap(1);
            //KMessageBox::information(0, "fount cap: "+cap);
        }
        if( -1 != rxDesignCapA.search( line ) ) {
            designCapA = rxDesignCapA.cap(1);
        }
        if( -1 != rxWarnCapA.search( line ) ) {
            warnCapA = rxWarnCapA.cap(1);
        }
    }
    file.close();

    // Falls keine Angaben in mWh, dafür aber welche in mAh, 
    // dann verwenden wir A als Einheit
    if( cap.isEmpty() && ! capA.isEmpty() ) {
        // no relevant values found, (maybe) wa are using mAh instead of mWh
        // so copy this values and set default unity
        cap = capA;
        designCap = designCapA;
        warnCap = warnCapA;
        powerUnit = "A";
    }

    file.setName("/proc/acpi/battery/BAT" + QString::number( batNr ) + "/state");
    if( ! file.exists() || ! file.open(IO_ReadOnly) ) {
       curFuell = -1;
       qDebug("could not open %s", file.name().latin1() );
       return false;
    }
    stream.setDevice( &file );

    // Pattern für /proc/acpi/battery/BATx/state
    QRegExp rxCur("^remaining capacity:\\s*(\\d{1,5})\\s*m" + powerUnit + "h");
    QRegExp rxOffline("^charging state:\\s*(discharging|charging|charged)");
    QRegExp rxMWH("^present rate:\\s*(\\d{1,5})\\s*m" + powerUnit );

    while( ! stream.atEnd() ) {
        line = stream.readLine();
        if( -1 != rxCur.search( line ) ) {
            cur = rxCur.cap(1);
            //KMessageBox::information(0, "fount cur: "+cur);
        }
        if( -1 != rxOffline.search( line ) ) {
            batState = rxOffline.cap(1);
        }
        if( -1 != rxMWH.search( line ) ) {
            mWHstring = rxMWH.cap(1);
        }
    }
    file.close();

    bool ok1 = true, ok2 = true;
    int curValue = cur.toInt(&ok1);
    int capValue = cap.toInt(&ok2);

    if( ok1 ) {
        curFuell = curValue;
    }
    if( ok2 ) {
        lastFuell = capValue;
    }

    // if we got a warn value and the cur capacity is below warn, the set critical to true
    criticalFuell = warnCap.toInt(&ok2);
    
    // current cosumption
    curPower = mWHstring.toInt(&ok2);
    if( ! ok2 ) {
        curPower = -1;
    }

    return true;
}
