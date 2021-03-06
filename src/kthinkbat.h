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
#ifndef KTHINKBAT_KTHINKBAT_H
#define KTHINKBAT_KTHINKBAT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt
#include <qvaluevector.h>
class QString;
class QTimer;
class QPainter;
class QColor;

// KDE
#include <kpanelapplet.h>
class KPopupMenu;
class KConfig;

// KThinkBat
#include "batinfo.h"
#include "batinfosum.h"
#include "batgauge.h"
class KThinkBatConfig;
class BatToolTip;

/**
 * A KDE panel applet to visually display the laptop battery state.
 *
 * @author Tobias Roeser <le.petit.fou@web.de>
 */
class KThinkBat : public KPanelApplet {
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

    /** Destructor of the panel applet. */
    virtual ~KThinkBat();

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
     * Is called when the user selects "Help" from the applets RMB menu.
     * Reimplement this function to launch a manual or help page.
     *
     * Note that this is called only when your applet supports the Help action.
     * See @ref Action and @ref KPanelApplet().
     **/
    virtual void help();

public slots:
    /** Called by the update timer to reread the laptop battery information. */
    void readBatteryInfoTimeout();

    void slotPreferences();

    /** Called, when Configuration is changed. Forces an update of the widget to reflect changes. */
    void slotUpdateConfiguration();

    /// Popup a passive Message about the current batteries
    void slotToolTip();

    /**
     * Is called when the user selects "About" from the menu.
     **/
    void slotAbout();

protected:
    /** (Re-)Paint the Applet Content. */
    void paintEvent(QPaintEvent* event);

    /** Handle mouse clicks. */
    void mousePressEvent(QMouseEvent* e);

    /** Handle the mouse enter event. Needed to start a timer and popup a custom tooltip after timeout. */
    void enterEvent(QEvent* e);

    /** Handle the mouse leave event. Needed to let disappear the tooltip. */
    void leaveEvent(QEvent* e);

    /** Construct the (HTML) tooltip text. */
    QString createToolTipText();

    QString createPowerTimeLabel(BatInfoBase* batInfo);

    void fillBatGauge(BatGauge* gauge, BatInfoBase* info);

private:

    /** Add a formated label value pair line to the tooltip. */
    QString toolTipLine(const QString& label, const QString& value);

    /// The space between gauge and power consuption label
    QSize padding;

    /// The timer, that controls the update of the battery values
    QTimer* timer;

    BatInfo m_batInfo1;
    BatInfo m_batInfo2;
    BatInfoSum m_batInfoSum;

    BatGauge m_gauge1;
    BatGauge m_gauge2;

    /** The (maximal) needed size of the panel applet. */
    QSize neededSize;

    int powerPosID;

    KPopupMenu* m_contextMenu;
    QTimer* m_toolTipTimer;

    BatToolTip* m_toolTip;

};

#endif
