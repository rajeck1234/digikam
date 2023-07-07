/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 11.09.2015
 *
 * SPDX-FileCopyrightText: 2012 by Marcel Wiesweg <marcel dot wiesweg at uk-essen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "taggingactionfactory.h"

// Qt includes

#include <QDebug>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "tagscache.h"
#include "coredbaccess.h"
#include "coredb.h"

namespace Digikam
{

class Q_DECL_HIDDEN TaggingActionFactory::Private
{
public:

    explicit Private()
        : parentTagId        (0),               ///< 0 means toplevel tag
          nameMatchMode      (MatchStartingWithFragment),
          constraintInterface(nullptr),
          defaultIndex       (-1),
          recentIndex        (0),
          valid              (false)
    {
    }

public:

    QString              fragment;
    int                  parentTagId;
    NameMatchMode        nameMatchMode;
    ConstraintInterface* constraintInterface;

    int                  defaultIndex;  ///< use if valid is true
    int                  recentIndex;   ///< use if valid is true

    QList<TaggingAction> actions;       ///< use if valid is true
    bool                 valid;

    TaggingAction        defaultAction; ///< independent from valid

public:

    void invalidate()
    {
        valid         = false;
        actions.clear();
        defaultAction = TaggingAction();
    }
};

TaggingActionFactory::TaggingActionFactory()
    : d(new Private)
{
}

TaggingActionFactory::~TaggingActionFactory()
{
    delete d;
}

QString TaggingActionFactory::fragment() const
{
    return d->fragment;
}

void TaggingActionFactory::setFragment(const QString& fragment)
{
    if (fragment == d->fragment)
    {
        return;
    }

    d->fragment = fragment;
    d->invalidate();
}

int TaggingActionFactory::parentTagId() const
{
    return d->parentTagId;
}

void TaggingActionFactory::setParentTag(int parentTagId)
{
    if (parentTagId == d->parentTagId)
    {
        return;
    }

    d->parentTagId = parentTagId;
    d->invalidate();
}

void TaggingActionFactory::setConstraintInterface(ConstraintInterface* const iface)
{
    if (d->constraintInterface == iface)
    {
        return;
    }

    d->constraintInterface = iface;
    d->invalidate();
}

TaggingActionFactory::ConstraintInterface* TaggingActionFactory::constraintInterface() const
{
    return d->constraintInterface;
}

void TaggingActionFactory::setNameMatchMode(NameMatchMode mode)
{
    if (d->nameMatchMode == mode)
    {
        return;
    }

    d->nameMatchMode = mode;
    d->invalidate();
}

TaggingActionFactory::NameMatchMode TaggingActionFactory::nameMatchMode() const
{
    return d->nameMatchMode;
}

void TaggingActionFactory::reset()
{
    d->fragment.clear();
    d->parentTagId         = 0;
    d->defaultIndex        = -1;
    d->nameMatchMode       = MatchStartingWithFragment;
    d->constraintInterface = nullptr;
    d->invalidate();
}

QList<TaggingAction> TaggingActionFactory::actions() const
{
    if (d->valid)
    {
        return d->actions;
    }

    QList<TaggingAction> actions;
    int defaultActionIndex = 0;
    d->recentIndex         = 0;

    // We use this action to find the right entry to select

    TaggingAction defaultAction = defaultTaggingAction();
    TaggingAction newUnderParent;

    if (d->parentTagId)
    {
        newUnderParent = TaggingAction(d->fragment, d->parentTagId);
    }

    TaggingAction newToplevel(d->fragment, 0);

    QList<int> completionEntries;

    if (d->nameMatchMode == MatchStartingWithFragment)
    {
        completionEntries = TagsCache::instance()->tagsStartingWith(d->fragment);
    }
    else
    {
        completionEntries = TagsCache::instance()->tagsContaining(d->fragment);
    }

    QList<TaggingAction> assignActions;

    // order the matched tags by chronological order, based on
    // recently assigned tags

    QList<int> recentTagIDs = CoreDbAccess().db()->getRecentlyAssignedTags();

    QListIterator<int> recent_iter(recentTagIDs);
    recent_iter.toBack();

    while (recent_iter.hasPrevious())
    {
        int pos = completionEntries.indexOf(recent_iter.previous());

        if (pos > 0)
        {
            completionEntries.move(pos, 0);
            ++d->recentIndex;
        }
    }

    Q_FOREACH (int id, completionEntries)
    {
        if (d->constraintInterface && !d->constraintInterface->matches(id))
        {
            continue;
        }

        assignActions << TaggingAction(id);
    }

    if (defaultAction.shallCreateNewTag())
    {
        // If it is the default action, we place the "Create Tag" entry at the top of the list.

        if (newUnderParent.isValid() && newUnderParent == defaultAction)
        {
            // Case A
            // a tag is currently selected in the listbox, we have the choice of toplevel and underparent for a new tag
            // the entire text currently written by the user doesn't exist as a tag. However, it might be a part of a tag

            Q_FOREACH (const TaggingAction& assignAction, assignActions)
            {
                actions << assignAction;
            }

            actions << newUnderParent;
            actions << newToplevel;
        }
        else
/*
        if (createItemTopLevel && (createItemTopLevel->action() == defaultAction))
*/
        {
            // Case B
            // no tag is currently selected in the listbox, only toplevel choice for a new tag
            // the entire text currently written by the user doesn't exist as a tag. However, it might be a part of a tag

            Q_FOREACH (const TaggingAction& assignAction, assignActions)
            {
                actions << assignAction;
            }

            actions << newToplevel;
        }
    }
    else
    {
        // Case C
        // the entire text currently written by the user exists as a tag

        Q_FOREACH (const TaggingAction& assignAction, assignActions)
        {
            actions << assignAction;

            if (assignAction == defaultAction)
            {
                defaultActionIndex = actions.size()-1;
            }
        }

        actions << newUnderParent;
        actions << newToplevel;
    }

    Private* const ncd = const_cast<Private*>(d);
    ncd->valid         = true;
    ncd->actions       = actions;
    ncd->defaultIndex  = defaultActionIndex;

    return actions;
}

int TaggingActionFactory::indexOfDefaultAction() const
{
    if (!d->valid)
    {
        actions();
    }

    return d->defaultIndex;
}

int TaggingActionFactory::indexOfLastRecentAction() const
{
    if (!d->valid)
    {
        return 0;
    }

    return d->recentIndex;
}

/*
AddTagsCompletionBoxItem* AddTagsCompletionBox::Private::createItemForExistingTag(TAlbum* talbum, bool uniqueName)
{
    if (!talbum || talbum->isRoot())
    {
        return 0;
    }

    AddTagsCompletionBoxItem* const item = new AddTagsCompletionBoxItem;
    TAlbum* const parent                 = static_cast<TAlbum*>(talbum->parent());

    if (parent->isRoot() || uniqueName)
    {
        item->setText(talbum->title());
    }
    else
    {
        item->setText(i18nc("<tag name> in <tag path>", "%1 in %2",
                            talbum->title(), parent->tagPath(false)));
    }

    if (model || filterModel)
    {
        QModelIndex index;

        if (filterModel)
        {
            index = filterModel->indexForAlbum(talbum);
        }
        else if (model)
        {
            index = model->indexForAlbum(talbum);
        }

        item->setData(Qt::DecorationRole, index.data(Qt::DecorationRole));
    }

    item->setData(CompletionTextRole, talbum->title());
    item->setAction(TaggingAction(talbum->id()));

    return item;
}

AddTagsCompletionBoxItem* AddTagsCompletionBox::Private::createItemForNewTag(const QString& newName, TAlbum* parent)
{
    int parentTagId = parent ? parent->id() : 0;

    // If tag exists, do not add an entry to create it

    if (TagsCache::instance()->tagForName(newName, parentTagId))
    {
        return 0;
    }

    AddTagsCompletionBoxItem* const item = new AddTagsCompletionBoxItem;

    item->setData(Qt::DecorationRole, AlbumThumbnailLoader::instance()->getNewTagIcon());
    item->setData(CompletionTextRole, newName);
    item->setAction(TaggingAction(newName, parentTagId));

    return item;
}
*/

QString TaggingActionFactory::suggestedUIString(const TaggingAction& action) const
{
    if (!action.isValid())
    {
        return QString();
    }

    if (action.shallAssignTag())
    {
        QString tagName     = TagsCache::instance()->tagName(action.tagId());
        int tagsParentTagId = TagsCache::instance()->parentTag(action.tagId());

        // check if it is a toplevel tag or there is only one tag with this name - then simply the tag name is sufficient

        if ((tagsParentTagId == 0) || (TagsCache::instance()->tagsForName(tagName).size() == 1))
        {
            return tagName;
        }
        else
        {
            return i18nc("<tag name> in <tag path>", "%1 in %2",
                         tagName, TagsCache::instance()->tagPath(tagsParentTagId, TagsCache::NoLeadingSlash));
        }
    }
    else // shallCreateNewTag
    {
        if (action.parentTagId())
        {
            return i18nc("Create New Tag <tag name> in <parent tag path>", "Create \"%1\" in %2",
                         action.newTagName(), TagsCache::instance()->tagPath(action.parentTagId(), TagsCache::NoLeadingSlash));
        }
        else
        {
            return i18n("Create \"%1\"", action.newTagName());
        }
    }
}

TaggingAction TaggingActionFactory::defaultTaggingAction() const
{
    if (d->defaultAction.isValid())
    {
        return d->defaultAction;
    }

    if (d->fragment.isEmpty())
    {
        return TaggingAction();
    }

    Private* const ncd = const_cast<Private*>(d);
    ncd->defaultAction = defaultTaggingAction(d->fragment, d->parentTagId);

    return d->defaultAction;
}

TaggingAction TaggingActionFactory::defaultTaggingAction(const QString& tagName, int parentTagId)
{
    // We now take the presumably best action, without autocompletion popup.
    // 1. Tag exists?
    //    a) Single tag? Assign.
    //    b) Multiple tags? 1. Existing tag under parent. 2. Toplevel tag 3. Alphabetically lowest tag
    // 2. Create tag under parent. No parent selected? Toplevel

    if (tagName.isEmpty())
    {
        return TaggingAction();
    }

    QList<int> tagIds = TagsCache::instance()->tagsForName(tagName);

    if (!tagIds.isEmpty())
    {
        if (tagIds.count() == 1)
        {
            return TaggingAction(tagIds.first());
        }
        else
        {
            int tagId = 0;

            if (parentTagId)
            {
                tagId = TagsCache::instance()->tagForName(tagName, parentTagId);
            }

            if (!tagId)
            {
                tagId = TagsCache::instance()->tagForName(tagName);    // toplevel tag
            }

            if (!tagId)
            {
                // sort lexically

                QMap<QString, int> map;

                Q_FOREACH (int id, tagIds)
                {
                    map[TagsCache::instance()->tagPath(id, TagsCache::NoLeadingSlash)] = id;
                }

                tagId = map.begin().value();
            }

            return TaggingAction(tagId);
        }
    }
    else
    {
        return TaggingAction(tagName, parentTagId);
    }
}

} // namespace Digikam
