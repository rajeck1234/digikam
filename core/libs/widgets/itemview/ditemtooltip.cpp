/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-12-10
 * Description : tool tip widget for iconview, thumbbar, and folderview items
 *
 * SPDX-FileCopyrightText: 2008-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "ditemtooltip.h"

// Qt includes

#include <QApplication>
#include <QDateTime>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QStyle>
#include <QStyleHintReturnMask>
#include <QStyleOptionFrame>
#include <QStylePainter>
#include <QToolTip>
#include <QVBoxLayout>
#include <QTextDocument>
#include <QByteArray>
#include <QBuffer>
#include <QScreen>
#include <QWindow>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

DToolTipStyleSheet::DToolTipStyleSheet(const QFont& font)
    : maxStringLength(30),
      unavailable    (i18n("unavailable"))
{
    // Note: rich-text doc https://doc.qt.io/qt-5/richtext-html-subset.html

    // Fix layout following bug #376438

    QString dir = QLatin1String("dir=\"rtl\"");

    if (qApp->layoutDirection() == Qt::LeftToRight)
    {
        dir = QLatin1String("dir=\"ltr\"");
    }

    QString fontSize = (font.pointSize() == -1) ? QString::fromUtf8("font-size: %1px;").arg(font.pixelSize())
                                                : QString::fromUtf8("font-size: %1pt;").arg(font.pointSize());

    tipHeader        = QLatin1String("<qt><table cellspacing=\"0\" cellpadding=\"0\" width=\"250\" border=\"0\">");
    tipFooter        = QLatin1String("</table></qt>");

    headBeg          = QString::fromLatin1("<tr bgcolor=\"%1\"><td colspan=\"2\">"
                                      "<nobr><p style=\"color:%2; font-family:%3; %4; %5\"><center><b>")
                       .arg(qApp->palette().color(QPalette::Base).name())
                       .arg(qApp->palette().color(QPalette::Text).name())
                       .arg(font.family())
                       .arg(fontSize)
                       .arg(dir);

    headEnd          = QLatin1String("</b></center></p></nobr></td></tr>");

    cellBeg          = QString::fromLatin1("<tr><td><nobr><p style=\"color:%1; font-family:%2; %3; %4\">")
                       .arg(qApp->palette().color(QPalette::ToolTipText).name())
                       .arg(font.family())
                       .arg(fontSize)
                       .arg(dir);

    cellMid          = QString::fromLatin1("</p></nobr></td><td><nobr><p style=\"color:%1; font-family:%2; %3; %4\">")
                       .arg(qApp->palette().color(QPalette::ToolTipText).name())
                       .arg(font.family())
                       .arg(fontSize)
                       .arg(dir);

    cellEnd          = QLatin1String("</p></nobr></td></tr>");

    cellSpecBeg      = QString::fromLatin1("<tr><td><nobr><p style=\"color:%1; font-family:%2; %3; %4\">")
                       .arg(qApp->palette().color(QPalette::ToolTipText).name())
                       .arg(font.family())
                       .arg(fontSize)
                       .arg(dir);

    cellSpecMid      = QString::fromLatin1("</p></nobr></td><td><nobr><p style=\"color:%1; font-family:%2; %3; %4\"><i>")
                       .arg(qApp->palette().color(QPalette::ToolTipText).name())
                       .arg(font.family())
                       .arg(fontSize)
                       .arg(dir);

    cellSpecEnd      = QLatin1String("</i></p></nobr></td></tr>");
}

QString DToolTipStyleSheet::breakString(const QString& input) const
{
    QString str = input.simplified();
    str         = str.toHtmlEscaped();

    if (str.length() <= maxStringLength)
    {
        return str;
    }

    QString br;

    int i     = 0;
    int count = 0;

    while (i < str.length())
    {
        if ((count >= maxStringLength) && str.at(i).isSpace())
        {
            count = 0;
            br.append(QLatin1String("<br/>"));
        }
        else
        {
            br.append(str.at(i));
        }

        ++i;
        ++count;
    }

    return br;
}

QString DToolTipStyleSheet::elidedText(const QString& str, Qt::TextElideMode elideMode) const
{
    if (str.length() <= maxStringLength)
    {
        return str;
    }

    switch (elideMode)
    {
        case Qt::ElideLeft:
        {
            return QLatin1String("...") + str.right(maxStringLength-3);
        }

        case Qt::ElideRight:
        {
            return str.left(maxStringLength-3) + QLatin1String("...");
        }

        case Qt::ElideMiddle:
        {
            return str.left(maxStringLength / 2 - 2) + QLatin1String("...") + str.right(maxStringLength / 2 - 1);
        }

        case Qt::ElideNone:
        {
            return str.left(maxStringLength);
        }

        default:
        {
            return str;
        }
    }
}

QString DToolTipStyleSheet::imageAsBase64(const QImage& img) const
{
    QByteArray byteArray;
    QBuffer    buffer(&byteArray);
    img.save(&buffer, "PNG");
    QString    iconBase64 = QString::fromLatin1(byteArray.toBase64().data());

    return QString::fromLatin1("<img src=\"data:image/png;base64,%1\">").arg(iconBase64);
}

// --------------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN DItemToolTip::Private
{
public:

    explicit Private()
      : tipBorder(5),
        corner   (0)
    {
    }

    const int   tipBorder;
    int         corner;
    QPixmap     corners[4];
};

DItemToolTip::DItemToolTip(QWidget* const parent)
    : QLabel(parent, Qt::ToolTip),
      d     (new Private)
{
    hide();

    setForegroundRole(QPalette::ToolTipText);
    setBackgroundRole(QPalette::ToolTipBase);
    ensurePolished();
    const int fwidth = qMax(d->tipBorder, 1 + style()->pixelMetric(QStyle::PM_ToolTipLabelFrameWidth, nullptr, this));
    setContentsMargins(fwidth, fwidth, fwidth, fwidth);
    setWindowOpacity(style()->styleHint(QStyle::SH_ToolTipLabel_Opacity, nullptr, this) / 255.0);
    setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    setFrameStyle(QFrame::StyledPanel);
/*
    Old-style box:
    setFrameStyle(QFrame::Plain | QFrame::Box);
    setLineWidth(1);
*/
    renderArrows();
}

DItemToolTip::~DItemToolTip()
{
    delete d;
}

void DItemToolTip::updateToolTip()
{
    renderArrows();

    QString contents = tipContents();
/*
    setWordWrap(Qt::mightBeRichText(contents));
*/
    setText(contents);
    resize(sizeHint());
}

bool DItemToolTip::toolTipIsEmpty() const
{
    return(text().isEmpty());
}

void DItemToolTip::reposition()
{
    QRect rect = repositionRect();

    if (rect.isNull())
    {
        return;
    }

    QPoint pos = rect.center();

    // d->corner:
    // 0: upperleft
    // 1: upperright
    // 2: lowerleft
    // 3: lowerright

    d->corner = 0;

    // should the tooltip be shown to the left or to the right of the ivi ?

    QScreen* screen = qApp->primaryScreen();

    if (QWidget* const widget = nativeParentWidget())
    {
        if (QWindow* const window = widget->windowHandle())
        {
            screen = window->screen();
        }
    }

    QRect desk = screen->geometry();

    if ((rect.center().x() + width()) > desk.right())
    {
        // to the left

        if (pos.x() - width() < desk.left())
        {
            pos.setX(0);
            d->corner = 4;
        }
        else
        {
            pos.setX(pos.x() - width());
            d->corner = 1;
        }
    }

    // should the tooltip be shown above or below the ivi ?

    if (rect.bottom() + height() > desk.bottom())
    {
        // above

        int top = rect.top() - height() - 5;

        if (top < desk.top())
        {
            top = desk.top();

            // recalculate x

            if      (d->corner == 0)
            {
                pos.setX(rect.right() + 5);
            }
            else if (d->corner == 1)
            {
                pos.setX(rect.left() - width() - 5);
            }
        }

        pos.setY(top);
        d->corner += 2;
    }
    else
    {
        pos.setY(rect.bottom() + 5);
    }

    move(pos);
}

void DItemToolTip::renderArrows()
{
    int w = d->tipBorder;

    // -- left top arrow -------------------------------------

    QPixmap& pix0 = d->corners[0];
    pix0          = QPixmap(w, w);
    pix0.fill(Qt::transparent);

    QPainter p0(&pix0);
    p0.setPen(QPen(qApp->palette().color(QPalette::Text), 1));

    for (int j = 0 ; j < w ; ++j)
    {
        p0.drawLine(0, j, w-j-1, j);
    }

    p0.end();

    // -- right top arrow ------------------------------------

    QPixmap& pix1 = d->corners[1];
    pix1          = QPixmap(w, w);
    pix1.fill(Qt::transparent);

    QPainter p1(&pix1);
    p1.setPen(QPen(qApp->palette().color(QPalette::Text), 1));

    for (int j = 0 ; j < w ; ++j)
    {
        p1.drawLine(j, j, w-1, j);
    }

    p1.end();

    // -- left bottom arrow ----------------------------------

    QPixmap& pix2 = d->corners[2];
    pix2          = QPixmap(w, w);
    pix2.fill(Qt::transparent);

    QPainter p2(&pix2);
    p2.setPen(QPen(qApp->palette().color(QPalette::Text), 1));

    for (int j = 0 ; j < w ; ++j)
    {
        p2.drawLine(0, j, j, j);
    }

    p2.end();

    // -- right bottom arrow ---------------------------------

    QPixmap& pix3 = d->corners[3];
    pix3          = QPixmap(w, w);
    pix3.fill(Qt::transparent);

    QPainter p3(&pix3);
    p3.setPen(QPen(qApp->palette().color(QPalette::Text), 1));

    for (int j = 0 ; j < w ; ++j)
    {
        p3.drawLine(w-j-1, j, w-1, j);
    }

    p3.end();
}

bool DItemToolTip::event(QEvent* e)
{
    switch (e->type())
    {
        case QEvent::Leave:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::FocusIn:
        case QEvent::FocusOut:
        case QEvent::Wheel:
        {
            hide();
            break;
        }

        default:
        {
            break;
        }
    }

    return QLabel::event(e);
}

void DItemToolTip::resizeEvent(QResizeEvent* e)
{
    QStyleHintReturnMask frameMask;
    QStyleOption option;
    option.initFrom(this);

    if (style()->styleHint(QStyle::SH_ToolTip_Mask, &option, this, &frameMask))
    {
        setMask(frameMask.region);
    }

    update();
    QLabel::resizeEvent(e);
}

void DItemToolTip::paintEvent(QPaintEvent* e)
{
    {
        QStylePainter p(this);
        QStyleOptionFrame opt;
        opt.initFrom(this);
        p.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
    }

    QLabel::paintEvent(e);

    QPainter p(this);

    if (d->corner >= 4)
    {
        return;
    }

    QPixmap& pix = d->corners[d->corner];

    switch (d->corner)
    {
        case 0:
        {
            p.drawPixmap(3, 3, pix);
            break;
        }

        case 1:
        {
            p.drawPixmap(width() - pix.width() - 3, 3, pix);
            break;
        }

        case 2:
        {
            p.drawPixmap(3, height() - pix.height() - 3, pix);
            break;
        }

        case 3:
        {
            p.drawPixmap(width() - pix.width() - 3, height() - pix.height() - 3, pix);
            break;
        }
    }
}

} // namespace Digikam
