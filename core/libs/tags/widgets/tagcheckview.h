/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-05
 * Description : tags filter view
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_TAG_CHECK_VIEW_H
#define DIGIKAM_TAG_CHECK_VIEW_H

// Qt includes

#include <QList>

// Local includes

#include "itemfiltersettings.h"
#include "tagfolderview.h"
#include "album.h"

namespace Digikam
{

class TagCheckView : public TagFolderView
{
    Q_OBJECT

public:

    enum ToggleAutoTags
    {
        NoToggleAuto = 0,
        Children,
        Parents,
        ChildrenAndParents
    };

public:

    explicit TagCheckView(QWidget* const parent, TagModel* const tagModel);

    QList<TAlbum*> getCheckedTags()          const;
    QList<TAlbum*> getPartiallyCheckedTags() const;

    ToggleAutoTags getToggleAutoTags()       const;
    void setToggleAutoTags(ToggleAutoTags toggle);

    /**
     * If this is switched on, a tag that is created
     * from _within_ this view, typically via the context menu,
     * will automatically be set checked.
     */
    void setCheckNewTags(bool checkNewTags);
    bool checkNewTags() const;

    ~TagCheckView() override;

    void doLoadState() override;
    void doSaveState() override;

Q_SIGNALS:

    /**
     * Emitted if the checked tags have changed.
     *
     * @param tags a list of selected tag ids
     */
    void checkedTagsChanged(const QList<TAlbum*>& includedTags, const QList<TAlbum*>& excludedTags);

public Q_SLOTS:

    /**
     * Resets the whole tag filter.
     */
    void slotResetCheckState();

protected:

    void addCustomContextMenuActions(ContextMenuHelper& cmh, Album* album) override;

private Q_SLOTS:

    /**
     * Called if the check state of a single item changes. Wraps this to an
     * event that is more useful for filtering tags.
     */
    void slotCheckStateChange(Album* album, Qt::CheckState state);

    void slotCreatedNewTagByContextMenu(TAlbum* tag);
    void toggleAutoActionSelected(QAction* action);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_TAG_CHECK_VIEW_H
