/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-09-07
 * Description : a pop-up menu implementation to display a
 *               hierarchical view of digiKam tags.
 *
 * SPDX-FileCopyrightText: 2004      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * Parts of the drawing code are inspired by from Trolltech ASA implementation.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tagspopupmenu.h"

// Qt includes

#include <QWidgetAction>
#include <QPainter>
#include <QPixmap>
#include <QSet>
#include <QFont>
#include <QString>
#include <QStyle>
#include <QStyleOptionButton>
#include <QStyleOptionMenuItem>
#include <QStyleOptionViewItem>
#include <QApplication>
#include <QActionGroup>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "album.h"
#include "coredb.h"
#include "albummanager.h"
#include "albumthumbnailloader.h"
#include "tageditdlg.h"
#include "tagscache.h"

namespace Digikam
{

class Q_DECL_HIDDEN TagToggleAction : public QWidgetAction
{
    Q_OBJECT

public:

    TagToggleAction(const QString& text, QObject* const parent);
    TagToggleAction(const QIcon& icon, const QString& text, QObject* const parent);
    QWidget* createWidget(QWidget* parent) override;

    void setSpecialChecked(bool checked);
    bool isChecked()        const;

    void setCheckBoxHidden(bool hidden);
    bool isCheckBoxHidden() const;

private:

    bool m_checked;
    bool m_checkBoxHidden;
};

// ------------------------------------------------------------------------

class Q_DECL_HIDDEN TagToggleMenuWidget : public QWidget
{
    Q_OBJECT

public:

    TagToggleMenuWidget(QMenu* const parent, TagToggleAction* const action);

protected:

    QSize sizeHint()                                        const override;
    void paintEvent(QPaintEvent*) override;

private:

    void initMenuStyleOption(QStyleOptionMenuItem* option)  const;
    void initViewStyleOption(QStyleOptionViewItem* option)  const;
    QSize menuItemSize(QStyleOptionMenuItem* opt)           const;
    QRect checkIndicatorSize(QStyleOption* option)          const;

private:

    QMenu*           m_menu;
    TagToggleAction* m_action;
};

// ------------------------------------------------------------------------

TagToggleMenuWidget::TagToggleMenuWidget(QMenu* const parent, TagToggleAction* const action)
    : QWidget (parent),
      m_menu  (parent),
      m_action(action)
{
    setMouseTracking(style()->styleHint(QStyle::SH_Menu_MouseTracking, nullptr, this));
}

QSize TagToggleMenuWidget::sizeHint() const
{
    // init style option for menu item

    QStyleOptionMenuItem opt;
    initMenuStyleOption(&opt);

    // get the individual sizes

    QSize menuSize   = menuItemSize(&opt);
    QRect checkRect  = checkIndicatorSize(&opt);
    const int margin = style()->pixelMetric(QStyle::PM_FocusFrameHMargin, nullptr, this) + 1;

    // return widget size

    int width        = margin + checkRect.width() + menuSize.width() + margin;
    QSize size(width, qMax(checkRect.height(), menuSize.height()));

    return size;
}

void TagToggleMenuWidget::paintEvent(QPaintEvent*)
{
    // init style option for menu item

    QStyleOptionMenuItem menuOpt;
    initMenuStyleOption(&menuOpt);

    // init style option for check indicator

    QStyleOptionViewItem viewOpt;
    initViewStyleOption(&viewOpt);

    // get a suitable margin

    const int margin      = style()->pixelMetric(QStyle::PM_FocusFrameHMargin,     nullptr, this);
    const int frameMargin = style()->pixelMetric(QStyle::PM_MenuDesktopFrameWidth, nullptr, this);

    // create painter

    QPainter p(this);

    /**
     * the menu rect should not go beyond the parent menu in width
     * move by margin and free room for menu frame
     */
    if (menuOpt.direction == Qt::RightToLeft)
    {
        // right-to-left untested

        viewOpt.rect.translate(-margin, 0);
        menuOpt.rect.translate(-margin, 0);
        menuOpt.menuRect.adjust(margin, 0, 0, 0);
    }
    else
    {
        viewOpt.rect.translate(margin, 0);
        menuOpt.rect.translate(margin, 0);
        menuOpt.menuRect.adjust(0, 0, -margin, 0);
    }

    // clear the background of the check indicator

    QStyleOptionMenuItem clearOpt(menuOpt);
    clearOpt.state        = QStyle::State_None;
    clearOpt.menuItemType = QStyleOptionMenuItem::EmptyArea;
    clearOpt.checkType    = QStyleOptionMenuItem::NotCheckable;
    clearOpt.rect         = viewOpt.rect;
    style()->drawControl(QStyle::CE_MenuEmptyArea, &menuOpt, &p, this);

    // draw a check indicator like the one used in a treeview

    QRect checkRect       = checkIndicatorSize(&menuOpt);
    viewOpt.rect          = checkRect;

    if (!m_action->isCheckBoxHidden())
    {
        style()->drawPrimitive(QStyle::PE_IndicatorItemViewItemCheck, &viewOpt, &p, this);
    }

    // move by size of check indicator

    if (menuOpt.direction == Qt::RightToLeft)
    {
        menuOpt.rect.translate( - checkRect.width() - margin, 0);
        menuOpt.rect.adjust( checkRect.width() + margin, 0, 0, 0);
    }
    else
    {
        menuOpt.rect.translate(checkRect.right() + margin, 0);
        menuOpt.rect.adjust(0, 0, - checkRect.right() - margin, 0);
    }

    // draw a full menu item - icon, text and menu indicator

    style()->drawControl(QStyle::CE_MenuItem, &menuOpt, &p, this);

    // draw the frame on the right

    if (frameMargin)
    {
        QRegion borderReg;
        borderReg         += QRect(width() - frameMargin, 0, frameMargin, height()); // right
        p.setClipRegion(borderReg);
        QStyleOptionFrame frame;
        frame.rect         = rect();
        frame.palette      = palette();
        frame.state        = QStyle::State_None;
        frame.lineWidth    = style()->pixelMetric(QStyle::PM_MenuPanelWidth);
        frame.midLineWidth = 0;
        style()->drawPrimitive(QStyle::PE_FrameMenu, &frame, &p, this);
    }
}

void TagToggleMenuWidget::initMenuStyleOption(QStyleOptionMenuItem* option) const
{
    // set basic option from widget properties

    option->initFrom(this);

    // set menu item state

    option->state  = QStyle::State_None;
    option->state |= QStyle::State_Enabled;

    if (m_menu->activeAction() == m_action) // if hovered etc.
    {
        option->state |= QStyle::State_Selected;
    }
/*
    if (mouseDown)
    {
        option->state |= QStyle::State_Sunken;
    }
*/

    // We have a special case here: menu items which are checked are not selectable,
    // it is an "Assign Tags" menu. To signal this, we change the pallette.
    // But only if there is no submenu...

    if (m_action->isChecked() && !m_action->menu())
    {
        option->palette.setCurrentColorGroup(QPalette::Disabled);
        option->state &= ~QStyle::State_Enabled;
    }

    // set options from m_action

    option->font                  = m_action->font();
    option->icon                  = m_action->icon();
    option->text                  = m_action->text();

    // we do the check mark ourselves

    option->checked               = false;
    option->menuHasCheckableItems = false;
    option->checkType             = QStyleOptionMenuItem::NotCheckable;

    // Don't forget the submenu indicator

    if (m_action->menu())
    {
        option->menuItemType = QStyleOptionMenuItem::SubMenu;
    }
    else
    {
        option->menuItemType = QStyleOptionMenuItem::Normal;
    }

    // seems QMenu does it like this

    option->maxIconWidth = style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, this);
    option->rect         = rect();
    option->menuRect     = parentWidget()->rect();
}

void TagToggleMenuWidget::initViewStyleOption(QStyleOptionViewItem* option) const
{
    // set basic option from widget properties

    option->initFrom(this);

    // set check state

    if (m_action->isChecked())
    {
        option->state |= QStyle::State_On;
    }
    else
    {
        option->state |= QStyle::State_Off;
    }
}

QSize TagToggleMenuWidget::menuItemSize(QStyleOptionMenuItem* opt) const
{
    QSize size;

    QFontMetrics fm(fontMetrics());
    size.setWidth(fm.horizontalAdvance(m_action->text()));
    size.setHeight(fm.height());

    if (!m_action->icon().isNull())
    {
        if (size.height() < opt->maxIconWidth)
        {
            size.setHeight(opt->maxIconWidth);
        }
    }

    return style()->sizeFromContents(QStyle::CT_MenuItem, opt, size, this);
}

QRect TagToggleMenuWidget::checkIndicatorSize(QStyleOption* option) const
{
    if (m_action->isCheckBoxHidden())
    {
        return QRect();
    }

    QStyleOptionButton opt;
    opt.QStyleOption::operator=(*option);
/*
    opt.rect = bounding;
*/
    return style()->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &opt, this);
}

// ------------------------------------------------------------------------

TagToggleAction::TagToggleAction(const QString& text, QObject* const parent)
    : QWidgetAction   (parent),
      m_checked       (false),
      m_checkBoxHidden(false)
{
    setText(text);
    setCheckable(true);
}

TagToggleAction::TagToggleAction(const QIcon& icon, const QString& text, QObject* const parent)
    : QWidgetAction   (parent),
      m_checked       (false),
      m_checkBoxHidden(false)
{
    setIcon(icon);
    setText(text);
    setCheckable(true);
}

QWidget* TagToggleAction::createWidget(QWidget* parent)
{
    QMenu* const menu = qobject_cast<QMenu*>(parent);

    if (menu)
    {
        return (new TagToggleMenuWidget(menu, this));
    }
    else
    {
        return nullptr;
    }
}

void TagToggleAction::setSpecialChecked(bool checked)
{
    // something is resetting the checked property when there is a submenu.
    // Use this to store "checked" anyway.
    // Note: the method isChecked() is not virtual.

    m_checked = checked;
    setChecked(checked);
}

bool TagToggleAction::isChecked() const
{
    return (m_checked || QWidgetAction::isChecked());
}

void TagToggleAction::setCheckBoxHidden(bool hidden)
{
    m_checkBoxHidden = hidden;
}

bool TagToggleAction::isCheckBoxHidden() const
{
    return m_checkBoxHidden;
}

// ------------------------------------------------------------------------

class Q_DECL_HIDDEN TagsPopupMenu::Private
{
public:

    explicit Private()
      : addTagActions   (nullptr),
        toggleTagActions(nullptr),
        mode            (ASSIGN)
    {
    }

    QPixmap              addTagPix;
    QPixmap              recentTagPix;
    QPixmap              tagViewPix;

    QSet<int>            assignedTags;
    QSet<int>            parentAssignedTags;
    QList<qlonglong>     selectedImageIDs;

    QActionGroup*        addTagActions;
    QActionGroup*        toggleTagActions;

    TagsPopupMenu::Mode  mode;
};

TagsPopupMenu::TagsPopupMenu(qlonglong selectedImageId, Mode mode, QWidget* const parent)
    : QMenu(parent),
      d    (new Private)
{
    d->selectedImageIDs << selectedImageId;
    setup(mode);
}

TagsPopupMenu::TagsPopupMenu(const QList<qlonglong>& selectedImageIds, Mode mode, QWidget* const parent)
    : QMenu(parent),
      d    (new Private)
{
    d->selectedImageIDs = selectedImageIds;
    setup(mode);
}

void TagsPopupMenu::setup(Mode mode)
{
    d->mode                 = mode;
    d->addTagPix            = QIcon::fromTheme(QLatin1String("tag")).pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize));
    d->recentTagPix         = QIcon::fromTheme(QLatin1String("tag-assigned")).pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize));
    d->tagViewPix           = QIcon::fromTheme(QLatin1String("edit-text-frame-update")).pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize));
    d->addTagActions        = new QActionGroup(this);
    d->toggleTagActions     = new QActionGroup(this);

    setSeparatorsCollapsible(true);

    connect(d->addTagActions, SIGNAL(triggered(QAction*)),
            this, SLOT(slotAddTag(QAction*)));

    connect(d->toggleTagActions, SIGNAL(triggered(QAction*)),
            this, SLOT(slotToggleTag(QAction*)));

    connect(this, SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShow()));

    AlbumThumbnailLoader* const loader = AlbumThumbnailLoader::instance();

    connect(loader, SIGNAL(signalThumbnail(Album*,QPixmap)),
            this, SLOT(slotTagThumbnail(Album*,QPixmap)));

    // we are not interested in signalThumbnailFailed
}

TagsPopupMenu::~TagsPopupMenu()
{
    delete d;
}

void TagsPopupMenu::clearPopup()
{
    d->assignedTags.clear();
    d->parentAssignedTags.clear();
    clear();
}

void TagsPopupMenu::slotAboutToShow()
{
    clearPopup();

    AlbumManager* const man = AlbumManager::instance();

    if      ((d->mode == REMOVE) || (d->mode == DISPLAY))
    {
        if (d->selectedImageIDs.isEmpty())
        {
            menuAction()->setEnabled(false);
            return;
        }

        const auto list = CoreDbAccess().db()->getItemCommonTagIDs(d->selectedImageIDs);
        d->assignedTags = QSet<int>(list.begin(), list.end());

        if (d->assignedTags.isEmpty())
        {
            menuAction()->setEnabled(false);
            return;
        }

        // also add the parents of the assigned tags

        bool hasValidTag = false;

        for (QSet<int>::const_iterator it = d->assignedTags.constBegin() ;
             it != d->assignedTags.constEnd() ; ++it)
        {
            TAlbum* const album = man->findTAlbum(*it);

            if (!album || album->isInternalTag())
            {
                continue;
            }

            hasValidTag = true;
            Album* a    = album->parent();

            while (a)
            {
                d->parentAssignedTags << a->id();
                a = a->parent();
            }
        }

        if (!hasValidTag)
        {
            menuAction()->setEnabled(false);

            return;
        }
    }
    else if (d->mode == ASSIGN)
    {
        if (d->selectedImageIDs.count() == 1)
        {
            const auto list = CoreDbAccess().db()->getItemCommonTagIDs(d->selectedImageIDs);
            d->assignedTags = QSet<int>(list.begin(), list.end());
        }
    }
    else if (d->mode == RECENTLYASSIGNED)
    {
        AlbumList recentTags = man->getRecentlyAssignedTags();

        if (recentTags.isEmpty())
        {
            addSection(d->recentTagPix, i18n("No Recently Assigned Tags"));
        }
        else
        {
            addSection(d->recentTagPix, i18n("Recently Assigned Tags"));

            for (AlbumList::const_iterator it = recentTags.constBegin() ;
                 it != recentTags.constEnd() ; ++it)
            {
                TAlbum* const album = static_cast<TAlbum*>(*it);

                if (album)
                {
                    TAlbum* const parent = dynamic_cast<TAlbum*> (album->parent());

                    if (parent)
                    {
                        QString p                     = parent->prettyUrl().section(QLatin1Char('/'), 1, -1);
                        p.replace(QLatin1Char('/'), QLatin1String(" / "));

                        if (!p.isEmpty())
                        {
                            p                         = QLatin1String(" (") + p + QLatin1Char(')');
                        }

                        QString t                     = album->title() + p;
                        t.replace(QLatin1Char('&'), QLatin1String("&&"));
                        TagToggleAction* const action = new TagToggleAction(t, d->toggleTagActions);
                        action->setData(album->id());
                        action->setCheckBoxHidden(true);
                        setAlbumIcon(action, album);
                        addAction(action);
                    }
                    else
                    {
                        qCDebug(DIGIKAM_GENERAL_LOG) << "Tag" << album
                                                     << "do not have a valid parent";
                    }
                }
            }
        }
    }

    if (((d->mode == REMOVE) || (d->mode == DISPLAY)) && (d->assignedTags.count() < 10))
    {
        buildFlatMenu(this);
    }
    else
    {
        TAlbum* const album = man->findTAlbum(0);

        if (!album)
        {
            return;
        }

        iterateAndBuildMenu(this, album);

        if ((d->mode == ASSIGN) || (d->mode == RECENTLYASSIGNED))
        {
            addSeparator();
            TagToggleAction* const addTag = new TagToggleAction(d->addTagPix, i18n("Add New Tag..."), d->addTagActions);
            addTag->setData(0);   // root id
            addTag->setCheckBoxHidden(true);
            addAction(addTag);

            addSeparator();
            TagToggleAction* const moreTag = new TagToggleAction(d->tagViewPix, i18n("More Tags..."), d->addTagActions);
            moreTag->setData(-1); // special id to query tag view
            moreTag->setCheckBoxHidden(true);
            addAction(moreTag);
        }
    }
}

/// for qSort
bool lessThanByTitle(const Album* first, const Album* second)
{
    return first->title() < second->title();
}

void TagsPopupMenu::iterateAndBuildMenu(QMenu* menu, TAlbum* album)
{
    QList<Album*> sortedTags;

    for (Album* a = album->firstChild() ; a ; a = a->next())
    {
        sortedTags << a;
    }

    std::stable_sort(sortedTags.begin(), sortedTags.end(), lessThanByTitle);

    for (QList<Album*>::const_iterator it = sortedTags.constBegin() ;
         it != sortedTags.constEnd() ; ++it)
    {
        TAlbum* const a = (TAlbum*)(*it);

        if (a->isInternalTag())
        {
            continue;
        }

        if      (d->mode == RECENTLYASSIGNED)
        {
            continue;
        }
        else if ((d->mode == REMOVE) || (d->mode == DISPLAY))
        {
            if (!d->assignedTags.contains(a->id()) && !d->parentAssignedTags.contains(a->id()))
            {
                continue;
            }
        }

        QString t = a->title();
        t.replace(QLatin1Char('&'), QLatin1String("&&"));

        TagToggleAction* action = nullptr;

        if (d->mode == ASSIGN)
        {
            action = new TagToggleAction(t, d->toggleTagActions);

            if (d->assignedTags.contains(a->id()))
            {
                action->setSpecialChecked(true);
            }
        }
        else     // REMOVE or DISPLAY mode
        {
            action = new TagToggleAction(t, d->toggleTagActions);
            action->setCheckBoxHidden(true);
        }

        action->setData(a->id());
        menu->addAction(action);

        // get icon

        setAlbumIcon(action, a);

        if (a->firstChild())
        {
            if (((d->mode != REMOVE) && (d->mode != DISPLAY)) ||
                d->parentAssignedTags.contains(a->id()))
            {
                action->setMenu(buildSubMenu(a->id()));
            }
        }
    }
}

QMenu* TagsPopupMenu::buildSubMenu(int tagid)
{
    AlbumManager* const man = AlbumManager::instance();
    TAlbum* const album     = man->findTAlbum(tagid);

    if (!album)
    {
        return nullptr;
    }

    QMenu* const popup = new QMenu(this);
    popup->setSeparatorsCollapsible(true);

    if      ((d->mode == ASSIGN) && !d->assignedTags.contains(album->id()))
    {
        TagToggleAction* const action = new TagToggleAction(i18n("Assign this Tag"), d->toggleTagActions);
        action->setData(album->id());
        action->setCheckBoxHidden(true);
        setAlbumIcon(action, album);
        popup->addAction(action);
        popup->addSeparator();
    }
    else if ((d->mode == REMOVE) && d->assignedTags.contains(tagid))
    {
        TagToggleAction* const action = new TagToggleAction(i18n("Remove this Tag"), d->toggleTagActions);
        action->setData(album->id());
        action->setCheckBoxHidden(true);
        setAlbumIcon(action, album);
        popup->addAction(action);
        popup->addSeparator();
        d->toggleTagActions->addAction(action);
    }
    else if (d->mode == DISPLAY)
    {
        TagToggleAction* const action = new TagToggleAction(i18n("Go to this Tag"), d->toggleTagActions);
        action->setData(album->id());
        action->setCheckBoxHidden(true);
        setAlbumIcon(action, album);
        popup->addAction(action);
        popup->addSeparator();
        d->toggleTagActions->addAction(action);
    }

    iterateAndBuildMenu(popup, album);

    if (d->mode == ASSIGN)
    {
        popup->addSeparator();

        TagToggleAction* const action = new TagToggleAction(d->addTagPix, i18n("Add New Tag..."), d->addTagActions);
        action->setData(album->id());
        action->setCheckBoxHidden(true);
        popup->addAction(action);
    }

    return popup;
}

void TagsPopupMenu::buildFlatMenu(QMenu* menu)
{
    QList<int> ids;
    QStringList shortenedPaths = TagsCache::instance()->shortenedTagPaths(d->assignedTags.values(), &ids,
                                                        TagsCache::NoLeadingSlash, TagsCache::NoHiddenTags);

    for (int i = 0 ; i < shortenedPaths.size() ; ++i)
    {
        QString t       = shortenedPaths.at(i);
        t.replace(QLatin1Char('&'), QLatin1String("&&"));
        TAlbum* const a = AlbumManager::instance()->findTAlbum(ids.at(i));

        if (!a)
        {
            continue;
        }

        TagToggleAction* const action = new TagToggleAction(t, d->toggleTagActions);

        if (d->mode == ASSIGN)
        {
            if (d->assignedTags.contains(a->id()))
            {
                action->setSpecialChecked(true);
            }
        }
        else     // REMOVE or DISPLAY mode
        {
            action->setCheckBoxHidden(true);
        }

        action->setData(a->id());
        menu->addAction(action);

        // get icon

        setAlbumIcon(action, a);
    }
}

void TagsPopupMenu::setAlbumIcon(QAction* action, TAlbum* album)
{
    AlbumThumbnailLoader* const loader = AlbumThumbnailLoader::instance();
    QPixmap pix;

    if (!loader->getTagThumbnail(album, pix))
    {
        if (pix.isNull())
        {
            action->setIcon(loader->getStandardTagIcon(album));
        }
        else
        {
            action->setIcon(pix);
        }
    }
    else
    {
        // for the time while loading, set standard icon
        // usually this code path will not be used, as icons are cached

        action->setIcon(loader->getStandardTagIcon(album));
    }
}

void TagsPopupMenu::slotToggleTag(QAction* action)
{
    int tagID = action->data().toInt();

    Q_EMIT signalTagActivated(tagID);
}

void TagsPopupMenu::slotAddTag(QAction* action)
{
    int tagID               = action->data().toInt();
    AlbumManager* const man = AlbumManager::instance();

    if (tagID == -1)
    {
        Q_EMIT signalPopupTagsView();
        return;
    }

    TAlbum* const parent    = man->findTAlbum(tagID);

    if (!parent)
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to find album with id " << tagID;
        return;
    }

    QString      title, icon;
    QKeySequence ks;

    if (!TagEditDlg::tagCreate(qApp->activeWindow(), parent, title, icon, ks))
    {
        return;
    }

    QMap<QString, QString> errMap;
    AlbumList tList = TagEditDlg::createTAlbum(parent, title, icon, ks, errMap);
    TagEditDlg::showtagsListCreationError(qApp->activeWindow(), errMap);

    for (AlbumList::const_iterator it = tList.constBegin() ;
         it != tList.constEnd() ; ++it)
    {
        Q_EMIT signalTagActivated((*it)->id());
    }
}

void TagsPopupMenu::slotTagThumbnail(Album* album, const QPixmap& pix)
{
    QList<QAction*> actionList = actions();

    Q_FOREACH (QAction* const action, actionList)
    {
        if (action->data().toInt() == album->id())
        {    // cppcheck-suppress useStlAlgorithm
            action->setIcon(pix);

            return;
        }
    }
}

} // namespace Digikam

#include "tagspopupmenu.moc"
