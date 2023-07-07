/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-21
 * Description : Demo test program for FacesEngine
 *
 * SPDX-FileCopyrightText: 2009-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2010 by Alex Jironkin <alexjironkin at gmail dot com>
 * SPDX-FileCopyrightText:      2010 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "demofaceitem.h"

// Qt includes

#include <QWidget>
#include <QPainter>
#include <QGraphicsOpacityEffect>
#include <QTextDocument>
#include <QGraphicsScene>
#include <QColor>
#include <QIcon>
#include <qmath.h>

// Local includes

#include "digikam_debug.h"
#include "demomarquee.h"
#include "demofancyrect.h"

namespace FaceEngineDemo
{

class Q_DECL_HIDDEN FaceItem::Private
{
public:

    explicit Private()
      : suggestionMode(false),
        sceneWidth(0),
        sceneHeight(0),
        x1(0),
        x2(0),
        y1(0),
        y2(0),
        name(),
        faceMarquee(nullptr),
        faceName(nullptr),
        nameRect(nullptr),
        origRect(),
        origScale(0.0),
        scale(0.0),
        rejectButton(nullptr),
        acceptButton(nullptr),
        suggestionRejectButton(nullptr),
        suggestionAcceptButton(nullptr)
    {
    }

    bool               suggestionMode;

    int                sceneWidth, sceneHeight;
    int                x1, x2, y1, y2;

    QString            name;
    Marquee*           faceMarquee;

    QGraphicsTextItem* faceName;
    QGraphicsRectItem* nameRect;

    QRect              origRect;
    double             origScale;
    double             scale;

    Button*            rejectButton;
    Button*            acceptButton;

    Button*            suggestionRejectButton;
    Button*            suggestionAcceptButton;
};

FaceItem::FaceItem(QGraphicsItem* const parent,
                   QGraphicsScene* const scene,
                   const QRect& rect,
                   double scale,
                   const QString& name,
                   double originalscale)
        : QGraphicsObject(parent),
          d(new Private)
{
    setAcceptHoverEvents(true);

    d->origScale     = originalscale;
    d->scale         = scale;
    d->origRect      = rect;
    d->sceneWidth    = scene->width();
    d->sceneHeight   = scene->height();

    // Scale all coordinates to fit the initial size of the scene

    d->x1 = rect.topLeft().x()     * scale;
    d->y1 = rect.topLeft().y()     * scale;
    d->x2 = rect.bottomRight().x() * scale;
    d->y2 = rect.bottomRight().y() * scale;

    // A QRect containing coordinates for the face rectangle

    QRect scaledRect;
    scaledRect.setTopLeft(QPoint(d->x1, d->y1));
    scaledRect.setBottomRight(QPoint(d->x2, d->y2));

    // marquee

    FancyRect* const fancy = new FancyRect(scaledRect);
    d->faceMarquee         = new Marquee(fancy);
    scene->addItem(d->faceMarquee);

    // Make a new QGraphicsTextItem for writing the name text, and a new QGraphicsRectItem to draw a good-looking, semi-transparent bounding box.

    d->nameRect = new QGraphicsRectItem(nullptr);
    scene->addItem(d->nameRect);

    d->faceName = new QGraphicsTextItem(name, nullptr);
    scene->addItem(d->faceName);

    // Make the bounding box for the name update itself to cover all the text whenever contents are changed

    QTextDocument* const doc = d->faceName->document();
    QTextOption o;
    o.setAlignment(Qt::AlignCenter);
    doc->setDefaultTextOption(o);

    // Get coordinates of the name relative to the scene

    QRectF r = d->faceName->mapRectToScene(d->faceName->boundingRect());

    // Draw the bounding name rectangle with the scene coordinates

    d->nameRect->setRect(r);
    QPen p(QColor(QLatin1String("white")));
    p.setWidth(3);
    d->nameRect->setPen(p);
    d->nameRect->setBrush(QBrush(QColor(QLatin1String("black"))));
    d->nameRect->setOpacity(0.6);
    d->nameRect->show();

    // Draw the name input item

    d->faceName->setDefaultTextColor(QColor(QLatin1String("white")));
    d->faceName->setFont(QFont(QLatin1String("Helvetica"), 9));
    d->faceName->setTextInteractionFlags(Qt::TextEditorInteraction);
    d->faceName->setOpacity(1);

    //---------------------

    QPixmap rejectPix  = QIcon::fromTheme(QLatin1String("window-close")).pixmap(16, 16);
    d->rejectButton    = new Button( rejectPix, rejectPix);
    scene->addItem(d->rejectButton);
    d->rejectButton->show();

    QPixmap acceptPix  = QIcon::fromTheme(QLatin1String("dialog-ok-apply")).pixmap(16, 16);
    d->acceptButton    = new Button( acceptPix, acceptPix);
    scene->addItem(d->acceptButton);
/*
    d->acceptButton->show();
*/
    d->suggestionRejectButton = new Button(rejectPix, rejectPix);
    scene->addItem(d->suggestionRejectButton);
    //d->suggestionAcceptButton->hide();

    d->suggestionAcceptButton = new Button(acceptPix, acceptPix);
    scene->addItem(d->suggestionAcceptButton);
/*
    d->suggestionRejectButton->hide();
*/
    update();

    switchToEditMode();

    d->acceptButton->hide();

    connect(d->rejectButton, &Button::clicked,
            this, &FaceItem::reject);

    connect(d->acceptButton, &Button::clicked,
            this, &FaceItem::accepted);

    connect(d->suggestionAcceptButton, &Button::clicked,
            this, &FaceItem::slotSuggestionAccepted);

    connect(d->suggestionRejectButton, &Button::clicked, this,
            &FaceItem::slotSuggestionRejected);

    connect(doc, SIGNAL(contentsChanged()),
            this, SLOT(update()));

    connect(d->faceMarquee, &Marquee::changed,
            this, &FaceItem::update);
}

FaceItem::~FaceItem()
{
    delete d;
}

QRectF FaceItem::boundingRect() const
{
    qreal adjust = 0.5;

    return QRectF(-18 - adjust,
                  -22 - adjust,
                   36 + adjust,
                   60 + adjust);
}

void FaceItem::setText(const QString& newName)
{
    d->faceName->setHtml(newName);
}

QString FaceItem::text() const
{
    return d->faceName->toPlainText().remove(QLatin1String("?"));
}

void FaceItem::update()
{
    if (text().isEmpty())
    {
        d->faceName->setDefaultTextColor(QColor("white"));
        d->nameRect->setPen(QPen(QColor("white")));
        d->acceptButton->hide();
        d->name.clear();
    }
    else
    {
        d->nameRect->setPen(QPen(QColor("yellow")));
        d->faceName->setDefaultTextColor(QColor("yellow"));

        if (!d->suggestionMode)
        {
            d->acceptButton->show();
            d->name = text();
        }
    }

    QPointF bl = d->faceMarquee->mapRectToScene(d->faceMarquee->boundingRect()).bottomLeft();
    QPointF br = d->nameRect->mapRectToScene(d->nameRect->boundingRect()).bottomRight();
    d->faceName->setPos(bl.x() + 5, bl.y() + 5);

    d->rejectButton->setPos(bl.x() - 16, bl.y() + 9);
    d->acceptButton->setPos(br.x() + 4,  bl.y() + 11);

    d->suggestionAcceptButton->setPos(br.x() + 4,  bl.y() + 11);
    d->suggestionRejectButton->setPos(br.x() + 20, bl.y() + 11);

    QRectF r      = d->faceName->mapRectToScene(d->faceName->boundingRect());
    d->nameRect->setRect(r);
    QRect newRect = this->d->faceMarquee->mapRectToScene(d->faceMarquee->boundingRect()).toRect();
    qCDebug(DIGIKAM_TESTS_LOG) << "Origscale is : " << d->origScale << " and scale is " << d->scale;

    QSize s(newRect.size());
    s.scale(newRect.width() * qSqrt(d->origScale), newRect.height() * qSqrt(d->origScale), Qt::KeepAspectRatio);
    newRect.setSize(s);
/*
    newRect.setRect(x,y,w,h);
*/
    qCDebug(DIGIKAM_TESTS_LOG) << "Orig before" << d->origRect;
/*
    d->origRect = newRect;
*/
    qCDebug(DIGIKAM_TESTS_LOG) << "Orig after" << d->origRect;
}

void FaceItem::setVisible(bool visible)
{
    d->faceMarquee->setVisible(visible);
    setControlsVisible(visible);
}

void FaceItem::setControlsVisible(bool visible)
{
    d->nameRect->setVisible(visible);
    d->faceName->setVisible(visible);

    if (d->suggestionMode)
    {
        d->suggestionAcceptButton->setVisible(visible);
        d->suggestionRejectButton->setVisible(visible);
    }
    else
    {
        d->rejectButton->setVisible(visible);
        d->acceptButton->setVisible(visible);
    }
}

void FaceItem::clearText()
{
    d->faceName->setPlainText(QString());
}

void FaceItem::clearAndHide()
{
    clearText();
    setVisible(false);
}

void FaceItem::accepted()
{
    d->acceptButton->hide();
    d->faceName->setDefaultTextColor(QColor("white"));
    d->nameRect->setPen(QPen(QColor("white")));
    Q_EMIT acceptButtonClicked(this->text(), this->originalRect());
}

QRect FaceItem::originalRect() const
{
    return d->origRect;
}

double FaceItem::originalScale() const
{
    return d->origScale;
}

void FaceItem::reject()
{
    Q_EMIT rejectButtonClicked(this->text(), this->originalRect());
    clearAndHide();
}

void FaceItem::suggest(const QString& name)
{
    qCDebug(DIGIKAM_TESTS_LOG) << "suggested name is " << name;
    d->name = name;
    this->switchToSuggestionMode();
}

void FaceItem::switchToEditMode()
{
    d->suggestionMode = false;
    d->faceName->setEnabled(true);
    d->faceName->setHtml(QLatin1String("<b>") + d->name + QLatin1String("</b>"));
    d->acceptButton->show();
    d->suggestionAcceptButton->hide();
    d->suggestionRejectButton->hide();
}

void FaceItem::switchToSuggestionMode()
{
    d->suggestionMode = true;
    d->faceName->setEnabled(false);
    d->faceName->setHtml(QLatin1String("Is this <b>") + d->name + QLatin1String("</b> ?"));
    d->acceptButton->hide();
    d->suggestionAcceptButton->show();
    d->suggestionRejectButton->show();
}

void FaceItem::slotSuggestionAccepted()
{
    switchToEditMode();
    d->faceName->setHtml(QLatin1String("<b>") + d->name + QLatin1String("</b>"));
    accepted();
    Q_EMIT suggestionAcceptButtonClicked(this->text(), this->originalRect());
}

void FaceItem::slotSuggestionRejected()
{
    switchToEditMode();
    d->faceName->setHtml(QLatin1String("<b>") + QString() + QLatin1String("</b>"));
    Q_EMIT suggestionRejectButtonClicked(this->text(), this->originalRect());
}

void FaceItem::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*)
{
}

} // namespace FaceEngineDemo
