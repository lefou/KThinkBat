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


#ifndef KTHINKBAT_H
#define KTHINKBAT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kpanelapplet.h>
#include <qstring.h>
#include <kconfig.h>
#include <qtimer.h>

#include "batinfo.h"

class KThinkBat : public KPanelApplet
{
    Q_OBJECT
    
public:
    /**
     * Construct a @ref KPanelApplet just like any other widget.
     *
     * @param configFile The configFile handed over in the factory function.
     * @param Type The applet @ref type().
     * @param actions Standard RMB menu actions supported by the applet (see @ref action() ).
     * @param parent The pointer to the parent widget handed over in the factory function.
     * @param name A Qt object name for your applet.
     **/
    KThinkBat(const QString& configFile, Type t = Normal, int actions = 0,
        QWidget *parent = 0, const char *name = 0);
    /** destructor */
    ~KThinkBat();
    
    /**
     * Retrieve a suggested width for a given height.
     *
     * Every applet should reimplement this function.
     *
     * Depending on the panel orientation the height (horizontal panel) or the
     * width (vertical panel) of the applets is fixed.
     * The exact values of the fixed size component depend on the panel size.
     *
     * On a horizontal panel the applet height is fixed, the panel will
     * call @ref widthForHeight(int height) with @p height
     * equal to 'the fixed applet height'
     * when laying out the applets.
     *
     * The applet can now choose the other size component (width)
     * based on the given height.
     *
     * The width you return is granted.
     **/
    virtual int widthForHeight(int height) const;
    /**
     * @return A suggested height for a given width.
     *
     * Every applet should reimplement this function.
     *
     * Depending on the panel orientation the height (horizontal panel) or the
     * width (vertical panel) of the applets is fixed.
     * The exact values of the fixed size component depend on the panel size.
     *
     * On a vertical panel the applet width is fixed, the panel will
     * call @ref heightForWidth(int width) with @p width
     * equal to 'the fixed applet width'
     * when laying out the applets.
     *
     * The applet can now choose the other size component (height)
     * based on the given width.
     *
     * The height you return is granted.
     **/
    virtual int heightForWidth(int width) const;
    /**
     * Is called when the user selects "About" from the applets RMB menu.
     * Reimplement this function to launch a about dialog.
     *
     * Note that this is called only when your applet supports the About action.
     * See @ref Action and @ref KPanelApplet().
     **/
    virtual void about();
    /**
     * Is called when the user selects "Help" from the applets RMB menu.
     * Reimplement this function to launch a manual or help page.
     *
     * Note that this is called only when your applet supports the Help action.
     * See @ref Action and @ref KPanelApplet().
     **/
    virtual void help();
    /**
     * Is called when the user selects "Preferences" from the applets RMB menu.
     * Reimplement this function to launch a preferences dialog or kcontrol module.
     *
     * Note that this is called only when your applet supports the preferences action.
     * See @ref Action and @ref KPanelApplet().
     **/
    virtual void preferences();

public slots:
    void timeout();
    
protected:
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent* event);


private:
    KConfig *ksConfig;
    
    /// Ladestand des ersten Akkus in Prozent. -1 if unaware.
    int batValue;
    /// Aktualisierungs-Interval fr die ACPI-Werte
    int intervall;
    /// aktueller Verbrauch in mW (oder unity)
    int mWH;
    /// \c true, wenn online
    bool online;
    /// \c true, wenn nur noch sehr wenig Ladung
    bool critical;
    /// Betriebs-zustand des Akkus
    QString state;

    QColor borderColor, emptyColor, chargedColor;

    QTimer* timer;

    QString unity;

    /// Anzeige des Verbrauchs unterhalb der Gauge anzeigen (oder rechts davon)
    bool wastePosBelow;

    BatInfo batInfo1;

};

#endif
