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

#ifndef DIGIKAM_ITEM_DELEGATE_OVERLAY_H
#define DIGIKAM_ITEM_DELEGATE_OVERLAY_H

// Qt includes

#include <QAbstractItemView>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class ItemViewHoverButton;

class DIGIKAM_EXPORT ItemDelegateOverlay : public QObject
{
    Q_OBJECT

public:

    explicit ItemDelegateOverlay(QObject* const parent = nullptr);
    ~ItemDelegateOverlay() override;

    /**
     * Called when the overlay was installed and shall begin working,
     * and before it is removed and shall stop.
     * Setup your connections to view and delegate here.
     * You will be disconnected automatically on removal.
     */
    virtual void setActive(bool active);

    /**
     * Only these two methods are implemented as virtual methods.
     * For all other events, connect to the view's signals.
     * There are a few signals specifically for overlays and all
     * QAbstractItemView standard signals.
     */
    virtual void mouseMoved(QMouseEvent* e, const QRect& visualRect, const QModelIndex& index);
    virtual void paint(QPainter* p, const QStyleOptionViewItem& option, const QModelIndex& index);

    void setView(QAbstractItemView* view);
    QAbstractItemView* view()                            const;

    void setDelegate(QAbstractItemDelegate* delegate);
    QAbstractItemDelegate* delegate()                    const;

    virtual bool acceptsDelegate(QAbstractItemDelegate*) const { return true; }

Q_SIGNALS:

    void update(const QModelIndex& index);

    void requestNotification(const QModelIndex& index, const QString& message);
    void hideNotification();

protected Q_SLOTS:

    /**
     * Called when any change from the delegate occurs - when the overlay is installed,
     * when size hints, styles or fonts change
     */
    virtual void visualChange();

protected:

    /**
     * For the context that an overlay can affect multiple items:
     * Assuming the currently overlayed index is given.
     * Will an operation affect only the single item, or multiple?
     * If multiple, retrieve the affected selection.
     */
    bool               affectsMultiple(const QModelIndex& index)         const;
    QList<QModelIndex> affectedIndexes(const QModelIndex& index)         const;
    int                numberOfAffectedIndexes(const QModelIndex& index) const;

    /**
     * Utility method
     */
    bool viewHasMultiSelection()                                         const;

protected:

    QAbstractItemView*     m_view;
    QAbstractItemDelegate* m_delegate;
};

#define REQUIRE_DELEGATE(Delegate)                                                                                                           \
                                                                                                                                             \
public:                                                                                                                                      \
                                                                                                                                             \
    void setDelegate(Delegate* delegate)                                 { ItemDelegateOverlay::setDelegate(delegate);                     } \
    Delegate* delegate() const                                           { return static_cast<Delegate*>(ItemDelegateOverlay::delegate()); } \
    virtual bool acceptsDelegate(QAbstractItemDelegate*d) const override { return dynamic_cast<Delegate*>(d);                              } \
                                                                                                                                             \
private:

// -------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT AbstractWidgetDelegateOverlay : public ItemDelegateOverlay
{
    Q_OBJECT

public:

    /**
     * This class provides functionality for using a widget in an overlay.
     * You must reimplement at least createWidget to return your widget.
     * Per default it will be shown when the cursor enters an index and hidden when left.
     * Reimplement slotEntered() and mouseMove() for more fine grained control.
     */
    explicit AbstractWidgetDelegateOverlay(QObject* const parent);

    /**
     * If active is true, this will call createWidget(), initialize the widget for use,
     * and setup connections for the virtual slots.
     * If active is false, this will delete the widget and
     * disconnect all signal from model and view to this object (!)
     */
    void setActive(bool active)                     override;

protected:

    /**
     * Create your widget here. When creating the object, pass parentWidget() as parent widget.
     * Ownership of the object is passed. It will be deleted in setActive(false).
     */
    virtual QWidget* createWidget() = 0;

    /**
     * Called when the widget shall be hidden (mouse cursor left index, viewport, uninstalled etc.).
     * Default implementation hide()s m_widget.
     */
    virtual void hide();

    /**
     * Returns the widget to be used as parent for your widget created in createWidget()
     */
    QWidget* parentWidget()                           const;

    /**
     * Return true here if you want to show the overlay for the given index.
     * The default implementation returns true.
     */
    virtual bool checkIndex(const QModelIndex& index) const;

    /**
     * Called when a QEvent::Leave of the viewport is received.
     * The default implementation hide()s.
     */
    virtual void viewportLeaveEvent(QObject* obj, QEvent* event);

    /**
     * Called when a QEvent::Enter resp. QEvent::Leave event for the widget is received.
     * The default implementation does nothing.
     */
    virtual void widgetEnterEvent();
    virtual void widgetLeaveEvent();

    /**
     * A sample implementation for above methods
     */
    void widgetEnterNotifyMultiple(const QModelIndex& index);
    void widgetLeaveNotifyMultiple();
    virtual QString notifyMultipleMessage(const QModelIndex&, int number);

    /**
     * Utility method called from slotEntered
     */
    bool checkIndexOnEnter(const QModelIndex& index)  const;

protected Q_SLOTS:

    /**
     * Default implementation shows the widget iff the index is valid and checkIndex returns true.
     */
    virtual void slotEntered(const QModelIndex& index);

    /**
     * Default implementations of these three slots call hide()
     */
    virtual void slotReset();
    virtual void slotViewportEntered();
    virtual void slotRowsRemoved(const QModelIndex& parent, int start, int end);
    virtual void slotLayoutChanged();

protected:

    bool eventFilter(QObject* obj, QEvent* event)           override;

protected:

    QWidget* m_widget;

    bool     m_mouseButtonPressedOnWidget;
};

// -------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT HoverButtonDelegateOverlay : public AbstractWidgetDelegateOverlay
{
    Q_OBJECT

public:

    explicit HoverButtonDelegateOverlay(QObject* const parent);

    /**
     * Will call createButton().
     */
    void setActive(bool active)                 override;

    ItemViewHoverButton* button()                 const;

protected:

    /**
     * Create your widget here. Pass view() as parent.
     */
    virtual ItemViewHoverButton* createButton() = 0;

    /**
     * Called when a new index is entered. Reposition your button here,
     * adjust and store state.
     */
    virtual void updateButton(const QModelIndex& index) = 0;

    QWidget* createWidget()                     override;
    void visualChange()                         override;

protected Q_SLOTS:

    void slotEntered(const QModelIndex& index)  override;
    void slotReset()                            override;
};

// -------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT PersistentWidgetDelegateOverlay : public AbstractWidgetDelegateOverlay
{
    Q_OBJECT

    /**
     * This class offers additional / modified behavior:
     * When a "persistent" mode is entered, it will not move
     * by mouse hover, but stay and only move on mouse click.
     * If the overlay widget had focus, it will be restored on show.
     */

public:

    explicit PersistentWidgetDelegateOverlay(QObject* const parent);
    ~PersistentWidgetDelegateOverlay() override;

    void setActive(bool active)                                         override;

    bool isPersistent() const;

public Q_SLOTS:

    /**
     * Enters persistent mode.
     * The overlay is moved because of mouse hover.
     */
    void setPersistent(bool persistent);
    void enterPersistentMode();
    void leavePersistentMode();

    void storeFocus();

protected:

    QModelIndex index() const;

    /**
     * Most overlays reimplement this slot to get the starting point
     * for repositioning a widget etc.
     * This class instead provides showOnIndex() which you shall
     * use for this purpose.
     */
    void slotEntered(const QModelIndex& index)                          override;
    void slotReset()                                                    override;
    void slotViewportEntered()                                          override;
    void slotRowsRemoved(const QModelIndex& parent, int start, int end) override;
    void slotLayoutChanged()                                            override;
    void viewportLeaveEvent(QObject* obj, QEvent* event)                override;
    void hide()                                                         override;

    /**
     * Reimplement to set the focus on the correct subwidget.
     * Default implementation sets focus on widget()
     */
    virtual void setFocusOnWidget();

    /// see slotEntered()
    virtual void showOnIndex(const QModelIndex& index);

    void restoreFocus();

private:

    class Private;
    Private* const d;
};

// -------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT ItemDelegateOverlayContainer
{
public:

    /**
     * This is a sample implementation for
     * delegate management methods, to be inherited by a delegate.
     * Does not inherit QObject, the delegate already does.
     */

    ItemDelegateOverlayContainer()          = default;
    virtual ~ItemDelegateOverlayContainer() = default;

    QList<ItemDelegateOverlay*> overlays() const;

    void installOverlay(ItemDelegateOverlay* overlay);
    void removeOverlay(ItemDelegateOverlay* overlay);
    void setAllOverlaysActive(bool active);
    void setViewOnAllOverlays(QAbstractItemView* view);
    void removeAllOverlays();
    void mouseMoved(QMouseEvent* e, const QRect& visualRect, const QModelIndex& index);

/*
    /// Provide as signal in the delegate:
    void visualChange();
    void requestNotification(const QModelIndex& index, const QString& message);
    void hideNotification();
*/

protected:

    virtual void drawOverlays(QPainter* p, const QStyleOptionViewItem& option, const QModelIndex& index) const;

    /// Declare as slot in the derived class calling this method
    virtual void overlayDestroyed(QObject* o);

    /// Returns the delegate, typically, the derived class
    virtual QAbstractItemDelegate* asDelegate() = 0;

protected:

    QList<ItemDelegateOverlay*> m_overlays;

private:

    Q_DISABLE_COPY(ItemDelegateOverlayContainer)
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_DELEGATE_OVERLAY_H
