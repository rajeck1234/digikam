/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-11-15
 * Description : widget item delegate for setup collection view
 *
 * SPDX-FileCopyrightText: 2015-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2008 by Rafael Fernández López <ereslibre at kde dot org>
 * SPDX-FileCopyrightText: 2008      by Kevin Ottens <ervin at kde dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dwitemdelegatepool.h"

// Qt includes

#include <QMetaObject>
#include <QMetaMethod>
#include <QMetaProperty>
#include <QPair>
#include <QHash>
#include <QList>
#include <QWidget>
#include <QAbstractItemView>
#include <QApplication>
#include <QInputEvent>
#include <QAbstractProxyModel>

// Local includes

#include "digikam_debug.h"
#include "dwitemdelegate.h"
#include "dwitemdelegate_p.h"

namespace Digikam
{

class Q_DECL_HIDDEN DWItemDelegateEventListener : public QObject
{
    Q_OBJECT

public:

    explicit DWItemDelegateEventListener(DWItemDelegatePoolPrivate* const poolPrivate,
                                         QObject* const parent = nullptr)
        : QObject    (parent),
          poolPrivate(poolPrivate)
    {
    }

    bool eventFilter(QObject* watched, QEvent* event) override;

private:

    DWItemDelegatePoolPrivate* poolPrivate;
};

// -------------------------------------------------------------------------------------------

DWItemDelegatePoolPrivate::DWItemDelegatePoolPrivate(DWItemDelegate* const d)
    : delegate     (d),
      eventListener(new DWItemDelegateEventListener(this)),
      clearing     (false)
{
}

// -------------------------------------------------------------------------------------------

DWItemDelegatePool::DWItemDelegatePool(DWItemDelegate* const delegate)
    : d(new DWItemDelegatePoolPrivate(delegate))
{
}

DWItemDelegatePool::~DWItemDelegatePool()
{
    delete d->eventListener;
    delete d;
}

QList<QWidget*> DWItemDelegatePool::findWidgets(const QPersistentModelIndex& idx,
                                                const QStyleOptionViewItem& option,
                                                UpdateWidgetsEnum updateWidgets) const
{
    QList<QWidget*> result;

    if (!idx.isValid())
    {
        return result;
    }

    QModelIndex index;

    if (const QAbstractProxyModel* proxyModel = qobject_cast<const QAbstractProxyModel*>(idx.model()))
    {
        index = proxyModel->mapToSource(idx);
    }
    else
    {
        index = idx;
    }

    if (!index.isValid())
    {
        return result;
    }

    if (d->usedWidgets.contains(index))
    {
        result = d->usedWidgets[index];
    }
    else
    {
        result = d->delegate->createItemWidgets(index);
        d->allocatedWidgets << result;
        d->usedWidgets[index] = result;

        Q_FOREACH (QWidget* const widget, result)
        {
            d->widgetInIndex[widget] = index;
            widget->setParent(d->delegate->d->itemView);
            widget->installEventFilter(d->eventListener);
            widget->setVisible(true);
        }
    }

    if (updateWidgets == UpdateWidgets)
    {
        Q_FOREACH (QWidget* const widget, result)
        {
            widget->setVisible(true);
        }

        d->delegate->updateItemWidgets(result, option, idx);

        Q_FOREACH (QWidget* const widget, result)
        {
            widget->move(widget->x() + option.rect.left(), widget->y() + option.rect.top());
        }
    }

    return result;
}

QList<QWidget*> DWItemDelegatePool::invalidIndexesWidgets() const
{
    QList<QWidget*> result;

    Q_FOREACH (QWidget* const widget, d->widgetInIndex.keys())
    {
        const QAbstractProxyModel* const proxyModel = qobject_cast<const QAbstractProxyModel*>(d->delegate->d->model);
        QModelIndex index;

        if (proxyModel)
        {
            index = proxyModel->mapFromSource(d->widgetInIndex[widget]);
        }
        else
        {
            index = d->widgetInIndex[widget];
        }

        if (!index.isValid())
        {
            result << widget;
        }
    }

    return result;
}

void DWItemDelegatePool::fullClear()
{
    d->clearing = true;
    qDeleteAll(d->widgetInIndex.keyBegin(), d->widgetInIndex.keyEnd());
    d->clearing = false;
    d->allocatedWidgets.clear();
    d->usedWidgets.clear();
    d->widgetInIndex.clear();
}

bool DWItemDelegateEventListener::eventFilter(QObject* watched, QEvent* event)
{
    QWidget* const widget = static_cast<QWidget*>(watched);

    if ((event->type() == QEvent::Destroy) && !poolPrivate->clearing)
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "User of DWItemDelegate should not delete widgets created by createItemWidgets!";

        // assume the application has kept a list of widgets and tries to delete them manually
        // they have been reparented to the view in any case, so no leaking occurs

        poolPrivate->widgetInIndex.remove(widget);
        QWidget* const viewport = poolPrivate->delegate->d->itemView->viewport();
        QApplication::sendEvent(viewport, event);
    }

    if (dynamic_cast<QInputEvent*>(event) && !poolPrivate->delegate->blockedEventTypes(widget).contains(event->type()))
    {
        QWidget* const viewport = poolPrivate->delegate->d->itemView->viewport();

        switch (event->type())
        {
            case QEvent::MouseMove:
            case QEvent::MouseButtonPress:
            case QEvent::MouseButtonRelease:
            case QEvent::MouseButtonDblClick:
            {
                QMouseEvent* const mouseEvent = static_cast<QMouseEvent*>(event);

#if (QT_VERSION > QT_VERSION_CHECK(5, 99, 0))
                QMouseEvent evt(event->type(), viewport->mapFromGlobal(mouseEvent->globalPosition().toPoint()),
                                mouseEvent->button(), mouseEvent->buttons(), mouseEvent->modifiers());
#else
                QMouseEvent evt(event->type(), viewport->mapFromGlobal(mouseEvent->globalPos()),
                                mouseEvent->button(), mouseEvent->buttons(), mouseEvent->modifiers());
#endif
                QApplication::sendEvent(viewport, &evt);
                break;
            }

            case QEvent::Wheel:
            {
                QWheelEvent* const wheelEvent = static_cast<QWheelEvent*>(event);

                QWheelEvent evt(wheelEvent->position(), wheelEvent->globalPosition(),wheelEvent->pixelDelta(),
                                wheelEvent->angleDelta(), wheelEvent->buttons(), wheelEvent->modifiers(),
                                wheelEvent->phase(), wheelEvent->inverted(), wheelEvent->source());

                QApplication::sendEvent(viewport, &evt);
                break;
            }

            case QEvent::TabletMove:
            case QEvent::TabletPress:
            case QEvent::TabletRelease:
            case QEvent::TabletEnterProximity:
            case QEvent::TabletLeaveProximity:
            {
                QTabletEvent* const tabletEvent = static_cast<QTabletEvent*>(event);

#if (QT_VERSION > QT_VERSION_CHECK(5, 99, 0))
                QTabletEvent evt(event->type(),
                                 tabletEvent->pointingDevice(),
                                 QPointF(viewport->mapFromGlobal(tabletEvent->globalPosition().toPoint())),
                                 tabletEvent->globalPosition(),
                                 tabletEvent->pressure(),
                                 tabletEvent->xTilt(),
                                 tabletEvent->yTilt(),
                                 tabletEvent->tangentialPressure(),
                                 tabletEvent->rotation(),
                                 tabletEvent->z(),
                                 tabletEvent->modifiers(),
                                 tabletEvent->button(),
                                 tabletEvent->buttons()
                );
#else
                QTabletEvent evt(event->type(),
                                 QPointF(viewport->mapFromGlobal(tabletEvent->globalPos())),
                                 tabletEvent->globalPosF(),
                                 tabletEvent->deviceType(),
                                 tabletEvent->pointerType(),
                                 tabletEvent->pressure(),
                                 tabletEvent->xTilt(),
                                 tabletEvent->yTilt(),
                                 tabletEvent->tangentialPressure(),
                                 tabletEvent->rotation(),
                                 tabletEvent->z(),
                                 tabletEvent->modifiers(),
                                 tabletEvent->uniqueId(),
                                 tabletEvent->button(),
                                 tabletEvent->buttons()
                );
#endif
                QApplication::sendEvent(viewport, &evt);
                break;
            }

            default:
            {
                QApplication::sendEvent(viewport, event);
                break;
            }
        }
    }

    return QObject::eventFilter(watched, event);
}

} // namespace Digikam

#include "dwitemdelegatepool.moc"
