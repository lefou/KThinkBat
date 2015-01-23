/***************************************************************************
 *   Copyright (C) 2005,2006,2007 by Siraj Razick <siraj@kdemail.net>      *
 *   Copyright (C) 2007 by Riccardo Iaconelli <ruphy@fsfe.org>             *
 *   Copyright (C) 2007 by Sebastian Kuegler <sebas@kde.org>               *
 *   Copyright (C) 2007 by Tobias Roeser <le.petit.fou@web.de>             *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "kthinkbat.h"

// Qt
#include <QApplication>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QFont>
#include <QGraphicsSceneHoverEvent>

// KDE
#include <KDebug>
#include <KIcon>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KDialog>
#include <KColorScheme>
#include <KGlobalSettings>

#include <plasma/svg.h>
#include <plasma/theme.h>

KThinkBat::KThinkBat(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_battery_percent_label(0),
      m_battery_percent(0),
      m_dialog(0),
      m_isHovered(0),
      m_hasBattery(0)
{
    kDebug() << "Loading applet kthinkbat";
    setAcceptsHoverEvents(true);
    setHasConfigurationInterface(true);

    KConfigGroup cg = config();
    m_showBatteryString = cg.readEntry("showBatteryString", true);
    m_drawBackground = cg.readEntry("drawBackground", true);
    m_pixelSize = cg.readEntry("size", 200);
    m_smallPixelSize = 22;
    m_theme = new Plasma::Svg("widgets/kthinkbat", this);
    m_theme->setContentType(Plasma::Svg::SingleImage);
    m_theme->resize(m_pixelSize, m_pixelSize);

    m_font = QApplication::font();
    m_font.setWeight(QFont::Bold);

    m_textColor = KColorScheme(QPalette::Active, KColorScheme::View).foreground().color();
    m_boxColor = KColorScheme(QPalette::Active, KColorScheme::View).background().color();

    m_boxAlpha = 92;
    m_boxHoverAlpha = 192;
    m_boxColor.setAlpha(m_boxAlpha);

    dataEngine("powermanagement")->connectSource(I18N_NOOP("Battery"), this);
    dataEngine("powermanagement")->connectSource(I18N_NOOP("AC Adapter"), this);

    updated(I18N_NOOP("Battery"), dataEngine("powermanagement")->query(I18N_NOOP("Battery")));
    updated(I18N_NOOP("AC Adapter"), dataEngine("powermanagement")->query(I18N_NOOP("AC Adapter")));

    setAcceptsHoverEvents(true);
    if (formFactor() == Plasma::Vertical ||
        formFactor() == Plasma::Horizontal) {
        kDebug() << "Init:Small FormFactor";
    } else {
        kDebug() << "Init:Huge FormFactor";
    }
}

QSizeF 
KThinkBat::contentSizeHint() const
{
    return m_size;
}

void 
KThinkBat::constraintsUpdated(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        int pixelSize = 0;
        if (formFactor() == Plasma::Vertical ||
                formFactor() == Plasma::Horizontal) {
            kDebug() << "Small FormFactor";
            setDrawStandardBackground(true);
            pixelSize = m_smallPixelSize;
        } else {
            kDebug() << "Huge FormFactor";
            setDrawStandardBackground(m_drawBackground);
            pixelSize = m_pixelSize;
        }
        m_theme->resize(QSize(pixelSize, pixelSize));
        updateGeometry();
        prepareGeometryChange();
        m_size = m_theme->size();
        m_font.setPointSize((int)(pixelSize/10));
        updateGeometry();
    }
}

void 
KThinkBat::updated(const QString& source, const Plasma::DataEngine::Data &data)
{
    kDebug() << "Applet::updated() ---------------------------- " << source;
    if (source == I18N_NOOP("Battery")) {
        m_hasBattery = data[I18N_NOOP("has Battery")].toBool();
        if (!data[I18N_NOOP("Plugged in")].toBool()) {
           m_hasBattery = false; 
        }
        m_battery_percent = data[I18N_NOOP("Percent")].toInt();
        m_battery_percent_label = data[I18N_NOOP("Percent")].toString();
        m_battery_percent_label.append("%");
        if (!m_hasBattery) {
            m_battery_percent_label = I18N_NOOP("No Battery");
        }
        kDebug() << "Applet::Battery::updated " << m_battery_percent;
    } else if (source == I18N_NOOP("AC Adapter")) {
        m_acadapter_plugged = data[I18N_NOOP("Plugged in")].toBool();
        kDebug() << "Applet::AC Adapter updated: " << m_acadapter_plugged; 
    } else {
        kDebug() << "Applet::Dunno what to do with " << source;
    }
    update();
}

void 
KThinkBat::showConfigurationInterface()
{
     if (m_dialog == 0) {
        m_dialog = new KDialog;
        m_dialog->setCaption(i18n("Configure Battery Monitor"));

        ui.setupUi(m_dialog->mainWidget());
        m_dialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );

        connect( m_dialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()) );
        connect( m_dialog, SIGNAL(okClicked()), this, SLOT(configAccepted()) );

    }
    ui.spinSize->setValue((int)m_size.width());
    ui.showBatteryStringCheckBox->setChecked(m_showBatteryString ? Qt::Checked : Qt::Unchecked);
    ui.drawBackgroundCheckBox->setChecked(m_drawBackground ? Qt::Checked : Qt::Unchecked); 
    m_dialog->show();
}

void 
KThinkBat::configAccepted()
{
    KConfigGroup cg = config();
    m_showBatteryString = ui.showBatteryStringCheckBox->checkState() == Qt::Checked;
    cg.writeEntry("showBatteryString", m_showBatteryString);

    m_drawBackground = ui.drawBackgroundCheckBox->checkState() == Qt::Checked;
    cg.writeEntry("drawBackground", m_drawBackground);
    cg.writeEntry("size", ui.spinSize->value());

    kDebug() << "Resize to: " << ui.spinSize->value();
    m_pixelSize = ui.spinSize->value();
    m_theme->resize(m_pixelSize, m_pixelSize);

    dataEngine("powermanagement")->disconnectSource(I18N_NOOP("Battery"), this);
    dataEngine("powermanagement")->connectSource(I18N_NOOP("Battery"), this);
    dataEngine("powermanagement")->disconnectSource(I18N_NOOP("AC Adapter"), this);
    dataEngine("powermanagement")->connectSource(I18N_NOOP("AC Adapter"), this);

    constraintsUpdated(Plasma::AllConstraints);
    cg.config()->sync();
}

void 
KThinkBat::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED( event );
    m_isHovered = true;
    update();
}

void 
KThinkBat::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED( event );
    m_isHovered = false;
    update();
}

KThinkBat::~KThinkBat()
{
}

void 
KThinkBat::paintLabel(QPainter *p, const QString& labelText)
{
    // Store font size, we want to restore it shortly
    int original_font_size = m_font.pointSize();

    // Fonts smaller than 4 points become unreadable, so the minimum size is 6
    if (original_font_size < KGlobalSettings::smallestReadableFont().pointSize()) {
        m_font.setPointSize(KGlobalSettings::smallestReadableFont().pointSize());
    }
    QFontMetrics fm(m_font);


    int text_width = fm.width(labelText);

    // Longer texts get smaller fonts
    if (labelText.length() > 4) {
        if (original_font_size/1.5 < KGlobalSettings::smallestReadableFont().pointSize()) {
            m_font.setPointSize((int)(KGlobalSettings::smallestReadableFont().pointSize()));
        } else {
            m_font.setPointSize((int)(original_font_size/1.5));
        }
        fm = QFontMetrics(m_font);
        text_width = (int)(fm.width(labelText) * 1.2);
    } else {
        // Smaller texts get a wider box
        text_width = (int)(text_width * 1.4);
    }
    p->setFont(m_font);

    // Let's find a good position for painting the background
    QRect text_rect = QRect((int)((contentSize().width()-fm.width(labelText))/2),
                            (int)(((contentSize().height() - (int)fm.height())/2*0.9)),
                            text_width, 
                            (int)(fm.height()*1.2));
    // Poor man's highlighting
    if (m_isHovered) {
        m_boxColor.setAlpha(m_boxHoverAlpha);
    }
    p->setBrush(m_boxColor);
    
    // Find sensible proportions for the rounded corners
    float round_prop = text_rect.width() / text_rect.height();

    // Tweak the rounding edge a bit with the proportions of the textbox
    int round_radius = 35;
    p->drawRoundRect(text_rect, (int)(round_radius/round_prop), round_radius);

    p->setBrush(m_textColor);
    p->drawText(text_rect, Qt::AlignCenter, labelText);

    // Reset font and box
    m_font.setPointSize(original_font_size);
    m_boxColor.setAlpha(m_boxAlpha);
}

void 
KThinkBat::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect 
&contentsRect)
{
    Q_UNUSED( option );
    //Q_UNUSED( contentsRect );

    bool showString = true;

    if (formFactor() == Plasma::Vertical ||
        formFactor() == Plasma::Horizontal) {
        showString = false;
    };

    p->setRenderHint(QPainter::SmoothPixmapTransform);
    p->setRenderHint(QPainter::Antialiasing);

    if (!m_hasBattery) {
        m_theme->paint(p, contentsRect, "AcAdapter");
        if (formFactor() == Plasma::Planar ||
            formFactor() == Plasma::MediaCenter) {

            // Show that there's no battery
            paintLabel(p, m_battery_percent_label);
        }
        return;
    }

    if (m_theme->elementExists("Battery")) {
        m_theme->paint(p, contentsRect, "Battery");
    } else {
        kDebug() << "Battery does not exist in SVG";
    }

    // Now let's find out which fillstate to show
    QString fill_element = QString();

    if (m_battery_percent > 95) {
        fill_element = "Fill100";
    } else if (m_battery_percent > 90) {
        fill_element = "Fill90";
    } else if (m_battery_percent > 80) {
        fill_element = "Fill80";
    } else if (m_battery_percent > 70) {
        fill_element = "Fill70";
    } else if (m_battery_percent > 55) {
        fill_element = "Fill60";
    } else if (m_battery_percent > 40) {
        fill_element = "Fill50";
    } else if (m_battery_percent > 30) {
        fill_element = "Fill40";
    } else if (m_battery_percent > 20) {
        fill_element = "Fill30";
    } else if (m_battery_percent > 10) {
        fill_element = "Fill20";
    } else if (m_battery_percent >= 5) {
        fill_element = "Fill10";
    }

    if (!fill_element.isEmpty()) {
        if (m_theme->elementExists(fill_element)) {
            m_theme->paint(p, contentsRect, fill_element);
        } else {
            kDebug() << fill_element << " does not exist in svg";
        }
    }

    if (m_acadapter_plugged) {
        m_theme->paint(p, contentsRect, "AcAdapter");
    }

    // Only show batterystring when we're huge
    if (formFactor() == Plasma::Planar ||
        formFactor() == Plasma::MediaCenter) {
        showString = m_showBatteryString;
    }

    // For small FormFactors, we're drawing a shadow,
    // but no text.
    if (formFactor() == Plasma::Vertical ||
        formFactor() == Plasma::Horizontal) {
        m_theme->paint(p, contentsRect, "Shadow");
        showString = false;
    }

    if (formFactor() == Plasma::Planar ||
        formFactor() == Plasma::MediaCenter) {
        if (showString || m_isHovered) {
            // Show the charge percentage with a box 
            // on top of the battery, but only for plasmoids bigger than ....
            if (m_pixelSize > 36) {
                paintLabel(p, m_battery_percent_label);
            }
        }
    }
}

#include "kthinkbat.moc"
