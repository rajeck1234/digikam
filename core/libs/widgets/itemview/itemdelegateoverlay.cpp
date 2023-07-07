/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-29
 * Description : Qt item view for images - delegate additions
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemdelegateoverlay.h"

// Qt includes

#include <QApplication>
#include <QEvent>
#include <QMouseEvent>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "itemviewdelegate.h"
#include "itemviewhoverbutton.h"

namespace Digikam
{

ItemDelegateOverlay::ItemDelegateOverlay(QObject* const parent)
    : QObject   (parent),
      m_view    (nullptr),
      m_delegate(nullptr)
{
}

ItemDelegateOverlay::~ItemDelegateOverlay()
{
}

void ItemDelegateOverlay::setActive(bool)
{
}

void ItemDelegateOverlay::visualChange()
{
}

void ItemDelegateOverlay::mouseMoved(QMouseEvent*, const QRect&, const QModelIndex&)
{
}

void ItemDelegateOverlay::paint(QPainter*, const QStyleOptionViewItem&, const QModelIndex&)
{
}

void ItemDelegateOverlay::setView(QAbstractItemView* view)
{
    if (m_view)
    {
        disconnect(this, SIGNAL(update(QModelIndex)),
                   m_view, SLOT(update(QModelIndex)));
    }

    m_view = view;

    if (m_view)
    {
        connect(this, SIGNAL(update(QModelIndex)),
                m_view, SLOT(update(QModelIndex)));
    }
}

QAbstractItemView* ItemDelegateOverlay::view() const
{
    return m_view;
}

void ItemDelegateOverlay::setDelegate(QAbstractItemDelegate* delegate)
{
    if (m_delegate)
    {
        disconnect(m_delegate, SIGNAL(visualChange()),
                   this, SLOT(visualChange()));
    }

    m_delegate = delegate;

    if (m_delegate)
    {
        connect(m_delegate, SIGNAL(visualChange()),
                this, SLOT(visualChange()));
    }
}

QAbstractItemDelegate* ItemDelegateOverlay::delegate() const
{
    return m_delegate;
}

bool ItemDelegateOverlay::affectsMultiple(const QModelIndex& index) const
{
    // note how selectionModel->selectedIndexes().contains() can scale badly

    QItemSelectionModel* const selectionModel = view()->selectionModel();

    if (!selectionModel->hasSelection())
    {
        return false;
    }

    if (!selectionModel->isSelected(index))
    {
        return false;
    }

    return viewHasMultiSelection();
}

bool ItemDelegateOverlay::viewHasMultiSelection() const
{
    QItemSelection selection = view()->selectionModel()->selection();

    if (selection.size() > 1)
    {
        return true;
    }

    return (selection.indexes().size() > 1);
}

QList<QModelIndex> ItemDelegateOverlay::affectedIndexes(const QModelIndex& index) const
{
    if (!affectsMultiple(index))
    {
        return QList<QModelIndex>() << index;
    }
    else
    {
        return view()->selectionModel()->selectedIndexes();
    }
}

int ItemDelegateOverlay::numberOfAffectedIndexes(const QModelIndex& index) const
{
    if (!affectsMultiple(index))
    {
        return 1;
    }

    // scales better than selectedIndexes().count()

    int count = 0;

    Q_FOREACH (const QItemSelectionRange& range, view()->selectionModel()->selection())
    {
        // cppcheck-suppress useStlAlgorithm
        count += range.height();
    }

    return count;
}

// --------------------------------------------------------------------------------------------

AbstractWidgetDelegateOverlay::AbstractWidgetDelegateOverlay(QObject* const parent)
    : ItemDelegateOverlay         (parent),
      m_widget                    (nullptr),
      m_mouseButtonPressedOnWidget(false)
{
}

void AbstractWidgetDelegateOverlay::setActive(bool active)
{
    if (active)
    {
        if (m_widget)
        {
            delete m_widget;
            m_widget = nullptr;
        }

        m_widget = createWidget();

        m_widget->setFocusPolicy(Qt::NoFocus);
        m_widget->hide(); // hide per default

        m_view->viewport()->installEventFilter(this);
        m_widget->installEventFilter(this);

        if (view()->model())
        {
            connect(m_view->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
                    this, SLOT(slotRowsRemoved(QModelIndex,int,int)));

            connect(m_view->model(), SIGNAL(layoutChanged()),
                    this, SLOT(slotLayoutChanged()));

            connect(m_view->model(), SIGNAL(modelReset()),
                    this, SLOT(slotReset()));
        }

        connect(m_view, SIGNAL(entered(QModelIndex)),
                this, SLOT(slotEntered(QModelIndex)));

        connect(m_view, SIGNAL(viewportEntered()),
                this, SLOT(slotViewportEntered()));
    }
    else
    {
        delete m_widget;
        m_widget = nullptr;

        if (m_view)
        {
            m_view->viewport()->removeEventFilter(this);

            if (view()->model())
            {
                disconnect(m_view->model(), nullptr, this, nullptr);
            }

            disconnect(m_view, SIGNAL(entered(QModelIndex)),
                       this, SLOT(slotEntered(QModelIndex)));

            disconnect(m_view, SIGNAL(viewportEntered()),
                       this, SLOT(slotViewportEntered()));
        }
    }
}

void AbstractWidgetDelegateOverlay::hide()
{
    if (m_widget)
    {
        m_widget->hide();
    }
}

QWidget* AbstractWidgetDelegateOverlay::parentWidget() const
{
    return m_view->viewport();
}

void AbstractWidgetDelegateOverlay::slotReset()
{
    hide();
}

void AbstractWidgetDelegateOverlay::slotEntered(const QModelIndex& index)
{
    hide();

    if (!checkIndexOnEnter(index))
    {
        return;
    }

    m_widget->show();
}

bool AbstractWidgetDelegateOverlay::checkIndexOnEnter(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return false;
    }

    if (QApplication::keyboardModifiers() & (Qt::ShiftModifier | Qt::ControlModifier))
    {
        return false;
    }

    if (!checkIndex(index))
    {
        return false;
    }

    return true;
}

bool AbstractWidgetDelegateOverlay::checkIndex(const QModelIndex& index) const
{
    Q_UNUSED(index);

    return true;
}

void AbstractWidgetDelegateOverlay::slotViewportEntered()
{
    hide();
}

void AbstractWidgetDelegateOverlay::slotRowsRemoved(const QModelIndex&, int, int)
{
    hide();
}

void AbstractWidgetDelegateOverlay::slotLayoutChanged()
{
    hide();
}

void AbstractWidgetDelegateOverlay::viewportLeaveEvent(QObject*, QEvent*)
{
    hide();
}

void AbstractWidgetDelegateOverlay::widgetEnterEvent()
{
}

void AbstractWidgetDelegateOverlay::widgetLeaveEvent()
{
}

void AbstractWidgetDelegateOverlay::widgetEnterNotifyMultiple(const QModelIndex& index)
{
    if (index.isValid() && affectsMultiple(index))
    {
        Q_EMIT requestNotification(index, notifyMultipleMessage(index, numberOfAffectedIndexes(index)));
    }
}

void AbstractWidgetDelegateOverlay::widgetLeaveNotifyMultiple()
{
    Q_EMIT hideNotification();
}

QString AbstractWidgetDelegateOverlay::notifyMultipleMessage(const QModelIndex&, int number)
{
    return i18ncp("@info: item overlay",
                  "Applying operation to\nthe selected picture",
                  "Applying operation to \n\"%1\" selected pictures",
                  number);
}

bool AbstractWidgetDelegateOverlay::eventFilter(QObject* obj, QEvent* event)
{
    if (m_widget && (obj == m_widget->parent()))   // events on view's viewport
    {
        switch (event->type())
        {
            case QEvent::Leave:
            {
                viewportLeaveEvent(obj, event);
                break;
            }

            case QEvent::MouseMove:
            {
                if (m_mouseButtonPressedOnWidget)
                {
                    // Don't forward mouse move events to the viewport,
                    // otherwise a rubberband selection will be shown when
                    // clicking on the selection toggle and moving the mouse
                    // above the viewport.
                    return true;
                }

                break;
            }

            case QEvent::MouseButtonRelease:
            {
                m_mouseButtonPressedOnWidget = false;
                break;
            }

            default:
            {
                break;
            }
        }
    }
    else if (obj == m_widget)
    {
        switch (event->type())
        {
            case QEvent::MouseButtonPress:
            {
                if (static_cast<QMouseEvent*>(event)->buttons() & Qt::LeftButton)
                {
                    m_mouseButtonPressedOnWidget = true;
                }

                break;
            }

            case QEvent::MouseButtonRelease:
            {
                m_mouseButtonPressedOnWidget = false;
                break;
            }

            case QEvent::Enter:
            {
                widgetEnterEvent();
                break;
            }

            case QEvent::Leave:
            {
                widgetLeaveEvent();
                break;
            }

            default:
            {
                break;
            }
        }
    }

    return ItemDelegateOverlay::eventFilter(obj, event);
}

// ------------------------------------------------------------------------------------------

HoverButtonDelegateOverlay::HoverButtonDelegateOverlay(QObject* const parent)
    : AbstractWidgetDelegateOverlay(parent)
{
}

ItemViewHoverButton* HoverButtonDelegateOverlay::button() const
{
    return static_cast<ItemViewHoverButton*>(m_widget);
}

void HoverButtonDelegateOverlay::setActive(bool active)
{
    AbstractWidgetDelegateOverlay::setActive(active);

    if (active)
    {
        button()->initIcon();
    }
}

QWidget* HoverButtonDelegateOverlay::createWidget()
{
    return createButton();
}

void HoverButtonDelegateOverlay::visualChange()
{
    if (m_widget && m_widget->isVisible())
    {
        updateButton(button()->index());
    }
}

void HoverButtonDelegateOverlay::slotReset()
{
    AbstractWidgetDelegateOverlay::slotReset();

    button()->reset();
}

void HoverButtonDelegateOverlay::slotEntered(const QModelIndex& index)
{
    AbstractWidgetDelegateOverlay::slotEntered(index);

    if (index.isValid() && checkIndex(index))
    {
        button()->setIndex(index);
        updateButton(index);
    }
    else
    {
        button()->setIndex(index);
    }
}

// -----------------------------------------------------------------------------------

class Q_DECL_HIDDEN PersistentWidgetDelegateOverlay::Private
{
public:

    explicit Private()
      : persistent  (false),
        restoreFocus(false)
    {
    }

    bool                  persistent;
    bool                  restoreFocus;

    QPersistentModelIndex index;
    QPersistentModelIndex enteredIndex;
};

PersistentWidgetDelegateOverlay::PersistentWidgetDelegateOverlay(QObject* const parent)
    : AbstractWidgetDelegateOverlay(parent),
      d                            (new Private)
{
}

PersistentWidgetDelegateOverlay::~PersistentWidgetDelegateOverlay()
{
    delete d;
}

QModelIndex PersistentWidgetDelegateOverlay::index() const
{
    return d->index;
}

void PersistentWidgetDelegateOverlay::setActive(bool active)
{
    d->persistent = false;

    AbstractWidgetDelegateOverlay::setActive(active);

    if (active)
    {
        connect(m_view->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                this, SLOT(leavePersistentMode()));

        connect(m_view, SIGNAL(viewportClicked(const QMouseEvent*)),
                this, SLOT(leavePersistentMode()));
    }
    else
    {
        if (m_view)
        {
            disconnect(m_view->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                       this, SLOT(leavePersistentMode()));

            disconnect(m_view, SIGNAL(viewportClicked(const QMouseEvent*)),
                       this, SLOT(leavePersistentMode()));
        }
    }
}

void PersistentWidgetDelegateOverlay::setPersistent(bool persistent)
{
    if (d->persistent == persistent)
    {
        return;
    }

    d->persistent = persistent;

    if      (d->persistent && d->index.isValid())
    {
        showOnIndex(d->index);
    }
    else if (!d->persistent)
    {
        if (d->enteredIndex.isValid())
        {
            slotEntered(d->enteredIndex);
        }
        else
        {
            hide();
        }
    }
}

void PersistentWidgetDelegateOverlay::enterPersistentMode()
{
    setPersistent(true);
}

void PersistentWidgetDelegateOverlay::leavePersistentMode()
{
    setPersistent(false);
}

bool PersistentWidgetDelegateOverlay::isPersistent() const
{
    return d->persistent;
}

void PersistentWidgetDelegateOverlay::slotEntered(const QModelIndex& index)
{
    d->enteredIndex = index;

    if (d->persistent && m_widget->isVisible())
    {
        return;
    }

    hide();

    if (!checkIndexOnEnter(index))
    {
        return;
    }

    m_widget->show();

    showOnIndex(index);
}

void PersistentWidgetDelegateOverlay::slotReset()
{
    setPersistent(false);
    d->restoreFocus = false;
    AbstractWidgetDelegateOverlay::slotReset();
}

void PersistentWidgetDelegateOverlay::slotViewportEntered()
{
    d->enteredIndex = QModelIndex();

    if (!d->persistent)
    {
        AbstractWidgetDelegateOverlay::slotViewportEntered();
    }
}

void PersistentWidgetDelegateOverlay::viewportLeaveEvent(QObject* obj, QEvent* event)
{
    setPersistent(false);
    d->restoreFocus = false;
    AbstractWidgetDelegateOverlay::viewportLeaveEvent(obj, event);
}

void PersistentWidgetDelegateOverlay::slotRowsRemoved(const QModelIndex& parent, int begin, int end)
{
    AbstractWidgetDelegateOverlay::slotRowsRemoved(parent, begin, end);
    setPersistent(false);
}

void PersistentWidgetDelegateOverlay::slotLayoutChanged()
{
    AbstractWidgetDelegateOverlay::slotLayoutChanged();
    setPersistent(false);
}

void PersistentWidgetDelegateOverlay::hide()
{
    if (!d->restoreFocus && m_widget->isVisible())
    {
        QWidget* const f = QApplication::focusWidget();
        d->restoreFocus  = f && m_widget->isAncestorOf(f);
    }

    AbstractWidgetDelegateOverlay::hide();
}

void PersistentWidgetDelegateOverlay::showOnIndex(const QModelIndex& index)
{
    d->index = QPersistentModelIndex(index);
    restoreFocus();
}

void PersistentWidgetDelegateOverlay::storeFocus()
{
    d->restoreFocus = true;
}

void PersistentWidgetDelegateOverlay::restoreFocus()
{
    if (d->restoreFocus)
    {
        setFocusOnWidget();
        d->restoreFocus = false;
    }
}

void PersistentWidgetDelegateOverlay::setFocusOnWidget()
{
    m_widget->setFocus();
}

// -----------------------------------------------------------------------------------

void ItemDelegateOverlayContainer::installOverlay(ItemDelegateOverlay* overlay)
{
    if (!overlay->acceptsDelegate(asDelegate()))
    {
        qCDebug(DIGIKAM_WIDGETS_LOG) << "Cannot accept delegate" << asDelegate()
                                     << "for installing" << overlay;
        return;
    }

    overlay->setDelegate(asDelegate());
    m_overlays << overlay;

    // let the view call setActive

    QObject::connect(overlay, SIGNAL(destroyed(QObject*)),
                     asDelegate(), SLOT(overlayDestroyed(QObject*)));

    QObject::connect(overlay, SIGNAL(requestNotification(QModelIndex,QString)),
                     asDelegate(), SIGNAL(requestNotification(QModelIndex,QString)));

    QObject::connect(overlay, SIGNAL(hideNotification()),
                     asDelegate(), SIGNAL(hideNotification()));
}

QList<ItemDelegateOverlay*> ItemDelegateOverlayContainer::overlays() const
{
    return m_overlays;
}

void ItemDelegateOverlayContainer::removeOverlay(ItemDelegateOverlay* overlay)
{
    overlay->setActive(false);
    overlay->setDelegate(nullptr);
    m_overlays.removeAll(overlay);
    QObject::disconnect(overlay, nullptr, asDelegate(), nullptr);
}

void ItemDelegateOverlayContainer::setAllOverlaysActive(bool active)
{
    Q_FOREACH (ItemDelegateOverlay* const overlay, m_overlays)
    {
        overlay->setActive(active);
    }
}

void ItemDelegateOverlayContainer::setViewOnAllOverlays(QAbstractItemView* view)
{
    Q_FOREACH (ItemDelegateOverlay* const overlay, m_overlays)
    {
        overlay->setView(view);
    }
}

void ItemDelegateOverlayContainer::removeAllOverlays()
{
    Q_FOREACH (ItemDelegateOverlay* const overlay, m_overlays)
    {
        overlay->setActive(false);
        overlay->setDelegate(nullptr);
        overlay->setView(nullptr);
    }

    m_overlays.clear();
}

void ItemDelegateOverlayContainer::overlayDestroyed(QObject* o)
{
    ItemDelegateOverlay* const overlay = qobject_cast<ItemDelegateOverlay*>(o);

    if (overlay)
    {
        removeOverlay(overlay);
    }
}

void ItemDelegateOverlayContainer::mouseMoved(QMouseEvent* e,
                                              const QRect& visualRect,
                                              const QModelIndex& index)
{
    Q_FOREACH (ItemDelegateOverlay* const overlay, m_overlays)
    {
        overlay->mouseMoved(e, visualRect, index);
    }
}

void ItemDelegateOverlayContainer::drawOverlays(QPainter* p,
                                                const QStyleOptionViewItem& option,
                                                const QModelIndex& index) const
{
    Q_FOREACH (ItemDelegateOverlay* const overlay, m_overlays)
    {
        overlay->paint(p, option, index);
    }
}

} // namespace Digikam
