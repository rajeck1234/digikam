/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 20013-07-03
 * Description : Tag Manager main class
 *
 * SPDX-FileCopyrightText: 2013 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_TAGS_MANAGER_H
#define DIGIKAM_TAGS_MANAGER_H

// Qt includes

#include <QPointer>
#include <QMainWindow>

// Local includes

#include "statesavingobject.h"

namespace Digikam
{

class TagModel;
class TAlbum;

class TagsManager : public QMainWindow, public StateSavingObject
{
    Q_OBJECT

public:

    TagsManager();
    ~TagsManager()                      override;

    /**
     * @brief setupUi   setup all gui elements for Tag Manager
     */
    void setupUi();

    static QPointer<TagsManager> internalPtr;
    static TagsManager* instance();
    static bool isCreated() { return !(internalPtr.isNull()); }

Q_SIGNALS:

    void signalSelectionChanged(TAlbum* album);

private Q_SLOTS:

    /**
     * @brief slotSelectionChanged - update tag properties in tagPropWidget when
     *                               different item is selected
     */
    void slotSelectionChanged();

    /**
     * Not used yet
     */
    void slotItemChanged();

    /**
     * @brief slotAddAction     - add new tag when addAction(+) is triggered
     */
    void slotAddAction();

    /**
     * @brief slotDeleteAction  - delete tag/tags when delAction is triggered
     */
    void slotDeleteAction();

    /**
     * @brief slotResetTagIcon  - connected to resetTagIcon action and
     *                            will reset icon to all selected tags
     */
    void slotResetTagIcon();

    /**
     * @brief slotEditTagTitle - view Tag Properties and set focus to title edit
     */
     void slotEditTagTitle();

    /**
     * @brief slotTitleEditReady - title edit from Tag Properties was return button pressed
     */
     void slotTitleEditReady();

    /**
     * @brief slotCreateTagAddr - connected to createTagAddr action and
     *                            will create tags from Addressbook
     */
    void slotCreateTagAddr();

    /**
     * @brief slotInvertSel     - connected to invSel action and will
     *                            invert selection of current items
     */
    void slotInvertSel();

    /**
     * @brief slotWriteToImg     - connected to wrDbImg action and will
     *                             write all metadata from database to images
     */
    void slotWriteToImg();

    /**
     * @brief slotReadFromImg     - coonected to readTags action and will
     *                              reread all images metadata into database
     */
    void slotReadFromImg();

    /**
     * @brief slotWipeAll         - connected to wipeAll action and will
     *                              wipe all tag related data from database
     *                              and reread from image's metadata
     */
    void slotWipeAll();

    /**
     * @brief slotRemoveTagsFromImg - will remove selected tags from all
     *                                images that have them.
     */
    void slotRemoveTagsFromImgs();

    /**
     * @brief slotMarkNotAssignedTags - mark all tags that are not assigned to images
     */
    void slotMarkNotAssignedTags();

protected:

    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event)   override;

    void doLoadState()                  override;
    void doSaveState()                  override;

private:

    // Disable
    explicit TagsManager(QWidget*) = delete;

    void setupActions();

    /**
     * @brief enableRootTagActions - enable or disable options when only root
     *                               tag is selected
     */
    void setHelpText(QAction* const action, const QString& text);
    void enableRootTagActions(bool value);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_TAGS_MANAGER_H
