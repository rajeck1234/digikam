/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-12
 * Description : Completion Box for tags
 *
 * SPDX-FileCopyrightText: 2010       by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 1997       by Sven Radej (sven.radej@iname.com)
 * SPDX-FileCopyrightText: 1999       by Patrick Ward <PAT_WARD@HP-USA-om5.om.hp.com>
 * SPDX-FileCopyrightText: 1999       by Preston Brown <pbrown at kde dot org>
 * SPDX-FileCopyrightText: 2000, 2001 by Dawit Alemayehu <adawit at kde dot org>
 * SPDX-FileCopyrightText: 2000, 2001 by Carsten Pfeiffer <pfeiffer at kde dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tagscompleter.h"

// Qt includes

#include <QStandardItemModel>
#include <QStandardItem>

// Local includes

#include "digikam_debug.h"
#include "album.h"
#include "albummodel.h"
#include "albummanager.h"
#include "albumfiltermodel.h"
#include "albumthumbnailloader.h"
#include "taggingactionfactory.h"
#include "tagscache.h"

namespace Digikam
{

enum
{
    TaggingActionRole = Qt::UserRole + 1,
    CompletionRole    = Qt::UserRole + 2
};

class Q_DECL_HIDDEN TagCompleter::Private : public TaggingActionFactory::ConstraintInterface
{
public:

    explicit Private()
        : model          (nullptr),
          supportingModel(nullptr),
          filterModel    (nullptr)
    {
    }

    ~Private() override
    {
    }

    QModelIndex indexForAlbum(int id)
    {
        if (!supportingModel)
        {
            return QModelIndex();
        }

        TAlbum* const talbum = AlbumManager::instance()->findTAlbum(id);

        return supportingModel->indexForAlbum(talbum);
    }

    bool matches(int id) override
    {
        TAlbum* const talbum = AlbumManager::instance()->findTAlbum(id);

        return filterModel->indexForAlbum(talbum).isValid();
    }

public:

    QStandardItemModel*  model;
    TaggingActionFactory factory;

    TagModel*            supportingModel;
    AlbumFilterModel*    filterModel;
};

TagCompleter::TagCompleter(QObject* const parent)
    : QCompleter(parent),
      d         (new Private)
{
    d->model = new QStandardItemModel(this);
    setModel(d->model);

    d->factory.setNameMatchMode(TaggingActionFactory::MatchContainingFragment);

    setCaseSensitivity(Qt::CaseInsensitive);
    setCompletionMode(PopupCompletion);
    setCompletionRole(CompletionRole);
    setFilterMode(Qt::MatchContains);
    setModelSorting(UnsortedModel);
    setCompletionColumn(0);

    connect(this, SIGNAL(activated(QModelIndex)),
            this, SLOT(slotActivated(QModelIndex)));

    connect(this, SIGNAL(highlighted(QModelIndex)),
            this, SLOT(slotHighlighted(QModelIndex)));
}

TagCompleter::~TagCompleter()
{
    delete d;
}

void TagCompleter::setTagFilterModel(AlbumFilterModel* const filterModel)
{
    d->filterModel = filterModel;
    d->factory.setConstraintInterface(d->filterModel ? d : nullptr);
}

void TagCompleter::setSupportingTagModel(TagModel* const model)
{
    d->supportingModel = model;
}

void TagCompleter::setContextParentTag(int parentTagId)
{
    d->factory.setParentTag(parentTagId);
}

void TagCompleter::update(const QString& fragment)
{
    if (fragment == d->factory.fragment())
    {
        return;
    }

    d->factory.setFragment(fragment);
    d->model->clear();

    QList<TaggingAction> actions = d->factory.actions();
    QList<QStandardItem*> items;

    Q_FOREACH (const TaggingAction& action, actions)
    {
        QStandardItem* const item = new QStandardItem;

        // Text, implemented by TaggingActionFactory

        item->setText(d->factory.suggestedUIString(action));

        // Action, via user data

        item->setData(QVariant::fromValue(action), TaggingActionRole);

        // Icon and completion role

        if (action.shallCreateNewTag())
        {
            item->setData(fragment, CompletionRole);
            item->setIcon(AlbumThumbnailLoader::instance()->getNewTagIcon());
        }
        else
        {
            item->setData(TagsCache::instance()->tagName(action.tagId()), CompletionRole);
            QModelIndex index = d->indexForAlbum(action.tagId());

            if (index.isValid())
            {
                item->setData(index.data(Qt::DecorationRole), Qt::DecorationRole);

                if (items.count() < d->factory.indexOfLastRecentAction())
                {
                    QFont font = item->font();
                    font.setBold(true);
                    item->setFont(font);
                }
            }
            else
            {
                item->setIcon(AlbumThumbnailLoader::instance()->getStandardTagIcon());
            }
        }

        items << item;
    }

    d->model->appendColumn(items);
}

void TagCompleter::slotActivated(const QModelIndex& index)
{
    Q_EMIT signalActivated(index.data(TaggingActionRole).value<TaggingAction>());
}

void TagCompleter::slotHighlighted(const QModelIndex& index)
{
    Q_EMIT signalHighlighted(index.data(TaggingActionRole).value<TaggingAction>());
}

} // namespace Digikam
