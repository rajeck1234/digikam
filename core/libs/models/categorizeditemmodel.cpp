/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-12-02
 * Description : Generic, standard item based model for DCategorizedView
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "categorizeditemmodel.h"

// Qt includes

#include <QAction>
#include <QMenu>
#include <QWidget>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

CategorizedItemModel::CategorizedItemModel(QObject* const parent)
    : QStandardItemModel(parent)
{
}

QStandardItem* CategorizedItemModel::addItem(const QString& text, const QVariant& category, const QVariant& categorySorting)
{
    QStandardItem* const item = new QStandardItem(text);
    item->setData(category, DCategorizedSortFilterProxyModel::CategoryDisplayRole);
    item->setData(categorySorting.isNull() ? category : categorySorting, DCategorizedSortFilterProxyModel::CategorySortRole);
    item->setData(rowCount(), ItemOrderRole);
    appendRow(item);

    return item;
}

QStandardItem* CategorizedItemModel::addItem(const QString& text, const QIcon& decoration,
                                             const QVariant& category, const QVariant& categorySorting)
{
    QStandardItem* const item = addItem(text, category, categorySorting);
    item->setIcon(decoration);

    return item;
}

DCategorizedSortFilterProxyModel* CategorizedItemModel::createFilterModel()
{
    DCategorizedSortFilterProxyModel* const filterModel = new DCategorizedSortFilterProxyModel(this);
    filterModel->setCategorizedModel(true);
    filterModel->setSortRole(ItemOrderRole);
    filterModel->setSourceModel(this);

    return filterModel;
}

// -------------------------------------------------------------------------------------------------------------------------

static QString adjustedActionText(const QAction* const action)
{
    QString text = action->text();
    text.remove(QLatin1Char('&'));
    text.remove(QRegularExpression(QString::fromUtf8("[ \u00A0]?(\\.\\.\\.|…)$")));
    int slashPos = -1;

    while ((slashPos = text.indexOf(QLatin1Char('/'), slashPos + 1)) != -1)
    {
        if ((slashPos == 0) || (slashPos == (text.length()-1)))
        {
            continue;
        }

        if (text.at(slashPos - 1).isSpace() || text.at(slashPos + 1).isSpace())
        {
            continue;
        }

        text.replace(slashPos, 1, QLatin1Char(','));
        text.insert(slashPos+1, QLatin1Char(' '));
    }

    return text;
}

class Q_DECL_HIDDEN ActionEnumerator
{
public:

    explicit ActionEnumerator(const QList<QAction*>& whitelist)
        : whitelist(whitelist)
    {
    }

    void enumerate(QWidget* const w)
    {
        // recurse

        enumerateActions(w, nullptr);
    }

    void addTo(ActionItemModel* const model, ActionItemModel::MenuCategoryMode mode)
    {
        int categorySortStartIndex = model->rowCount();

        Q_FOREACH (QAction* const a, actions)
        {
            QAction* categoryAction = nullptr;

            if (mode & ActionItemModel::ToplevelMenuCategory)
            {
                for (QAction* p = a ; p ; p = parents.value(p))
                {
                    categoryAction = p;
                }
            }
            else
            {
                categoryAction = parents.value(a);
            }

            if (!categoryAction)
            {
                continue;
            }

            QVariant categorySortValue;

            if (mode & ActionItemModel::SortCategoriesByInsertionOrder)
            {
                categorySortValue = categorySortStartIndex++;
            }

            model->addAction(a, adjustedActionText(categoryAction), categorySortValue);
        }
    }

protected:

    const QList<QAction*>&   whitelist;
    QList<QAction*>          actions;
    QMap<QAction*, QAction*> parents;
    QList<QAction*>          parentsInOrder;

    void enumerateActions(const QWidget* const w, QAction* const widgetAction)
    {
        Q_FOREACH (QAction* const a, w->actions())
        {
            if (a->menu())
            {
                enumerateActions(a->menu(), a->menu()->menuAction());
            }
            else if (whitelist.isEmpty() || whitelist.contains(a))
            {
                actions << a;
            }

            parents[a] = widgetAction;

            if (!parentsInOrder.contains(widgetAction))
            {
                parentsInOrder << widgetAction;
            }
        }
    }
};

// -------------------------------------------------------------------------------------------------------------------------

ActionItemModel::ActionItemModel(QObject* const parent)
    : CategorizedItemModel(parent),
      m_mode              (ToplevelMenuCategory | SortCategoriesAlphabetically),
      m_filterModel       (nullptr)
{
}

void ActionItemModel::setMode(MenuCategoryMode mode)
{
    m_mode = mode;
}

ActionItemModel::MenuCategoryMode ActionItemModel::mode() const
{
    return m_mode;
}

QStandardItem* ActionItemModel::addAction(QAction* action, const QString& category, const QVariant& categorySorting)
{
    if (!action || action->menu())
    {
        return nullptr;
    }

    QStandardItem* const item = addItem(QString(), category, categorySorting);
    item->setEditable(false);
    setPropertiesFromAction(item, action);

    connect(action, SIGNAL(changed()),
            this, SLOT(slotActionChanged()));

    return item;
}

DCategorizedSortFilterProxyModel* ActionItemModel::createFilterModel()
{
    DCategorizedSortFilterProxyModel* const filterModel = new ActionSortFilterProxyModel(this);
    filterModel->setCategorizedModel(true);
    filterModel->setSortRole(ItemOrderRole);
    filterModel->setSourceModel(this);
    m_filterModel = filterModel;

    return filterModel;
}

void ActionItemModel::setPropertiesFromAction(QStandardItem* item, QAction* action)
{
    item->setText(adjustedActionText(action));
    item->setIcon(action->icon());
    item->setEnabled(action->isEnabled());
    item->setCheckable(action->isCheckable());

    if (action->toolTip() != action->text())
    {
        item->setToolTip(action->toolTip());
    }

    item->setWhatsThis(action->whatsThis());
    item->setData(QVariant::fromValue(static_cast<QObject*>(action)), ItemActionRole);
}

void ActionItemModel::addActions(QWidget* w)
{
    addActions(w, QList<QAction*>());
}

void ActionItemModel::addActions(QWidget* w, const QList<QAction*>& actionWhiteList)
{
    ActionEnumerator enumerator(actionWhiteList);
    enumerator.enumerate(w);
    enumerator.addTo(this, m_mode);
}

QAction* ActionItemModel::actionForIndex(const QModelIndex& index)
{
    return static_cast<QAction*>(index.data(ItemActionRole).value<QObject*>());
}

QStandardItem* ActionItemModel::itemForAction(QAction* action) const
{
    if (!action)
    {
        return nullptr;
    }

    for (int i = 0 ; i < rowCount() ; ++i)
    {
        QStandardItem* const it = item(i);

        if (it && (static_cast<QAction*>(it->data(ItemActionRole).value<QObject*>()) == action))
        {
            return it;
        }
    }

    return nullptr;
}

QModelIndex ActionItemModel::indexForAction(QAction *action) const
{
    return indexFromItem(itemForAction(action));
}

void ActionItemModel::hover(const QModelIndex& index)
{
    QAction* const action = actionForIndex(index);

    if (action)
    {
        action->hover();
    }
}

void ActionItemModel::toggle(const QModelIndex& index)
{
    QAction* const action = actionForIndex(index);

    if (action)
    {
        action->toggle();
    }
}

void ActionItemModel::trigger(const QModelIndex& index)
{
    QAction* const action = actionForIndex(index);

    if (action && action->isEnabled())
    {
        action->trigger();
    }
}

void ActionItemModel::slotActionChanged()
{
    QAction* const action     = qobject_cast<QAction*>(sender());
    QStandardItem* const item = itemForAction(action);

    if (item)
    {
        setPropertiesFromAction(item, action);
    }

    if (m_filterModel)
    {
        m_filterModel->invalidate();
    }
}

} // namespace Digikam
