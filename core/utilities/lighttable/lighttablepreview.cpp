/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-21-12
 * Description : digiKam light table preview item.
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "lighttablepreview.h"

// Qt includes

#include <QList>
#include <QPainter>
#include <QString>
#include <QFontMetrics>
#include <QPixmap>
#include <QPalette>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "coredb.h"
#include "ddragobjects.h"
#include "dimg.h"
#include "dimgpreviewitem.h"

namespace Digikam
{

LightTablePreview::LightTablePreview(QWidget* const parent)
    : ItemPreviewView(parent, ItemPreviewView::LightTablePreview)
{
    setDragAndDropEnabled(true);
    showDragAndDropMessage();
}

LightTablePreview::~LightTablePreview()
{
}

void LightTablePreview::setDragAndDropEnabled(bool b)
{
    setAcceptDrops(b);
    viewport()->setAcceptDrops(b);
}

void LightTablePreview::showDragAndDropMessage()
{
    if (acceptDrops())
    {
        QString msg    = i18n("Drag and drop an image here");
        QFontMetrics fontMt(font());
        QRect fontRect = fontMt.boundingRect(0, 0, width(), height(), 0, msg);
        QPixmap pix(fontRect.size());
        pix.fill(qApp->palette().color(QPalette::Base));
        QPainter p(&pix);
        p.setPen(QPen(qApp->palette().color(QPalette::Text)));
        p.drawText(0, 0, pix.width(), pix.height(),
                   Qt::AlignCenter | Qt::TextWordWrap,
                   msg);
        p.end();
        previewItem()->setImage(DImg(pix.toImage()));
    }
}

void LightTablePreview::dragEnterEvent(QDragEnterEvent* e)
{
    if (dragEventWrapper(e->mimeData()))
    {
        e->accept();
    }
}

void LightTablePreview::dragMoveEvent(QDragMoveEvent* e)
{
    if (dragEventWrapper(e->mimeData()))
    {
        e->accept();
    }
}

bool LightTablePreview::dragEventWrapper(const QMimeData* data) const
{
    if (acceptDrops())
    {
        int              albumID;
        QList<int>       albumIDs;
        QList<qlonglong> imageIDs;
        QList<QUrl>      urls;

        if (DItemDrag::decode(data, urls, albumIDs, imageIDs) ||
            DAlbumDrag::decode(data, urls, albumID)           ||
            DTagListDrag::canDecode(data))
        {
            return true;
        }
    }

    return false;
}

void LightTablePreview::dropEvent(QDropEvent* e)
{
    if (acceptDrops())
    {
        int              albumID;
        QList<int>       albumIDs;
        QList<qlonglong> imageIDs;
        QList<QUrl>      urls;

        if      (DItemDrag::decode(e->mimeData(), urls, albumIDs, imageIDs))
        {
            Q_EMIT signalDroppedItems(ItemInfoList(imageIDs));
            e->accept();
            return;
        }
        else if (DAlbumDrag::decode(e->mimeData(), urls, albumID))
        {
            QList<qlonglong> itemIDs = CoreDbAccess().db()->getItemIDsInAlbum(albumID);

            Q_EMIT signalDroppedItems(ItemInfoList(itemIDs));
            e->accept();
            return;
        }
        else if (DTagListDrag::canDecode(e->mimeData()))
        {
            QList<int> tagIDs;

            if (!DTagListDrag::decode(e->mimeData(), tagIDs))
            {
                return;
            }

            QList<qlonglong> itemIDs = CoreDbAccess().db()->getItemIDsInTag(tagIDs.first(), true);
            ItemInfoList imageInfoList;

            Q_EMIT signalDroppedItems(ItemInfoList(itemIDs));
            e->accept();

            return;
        }
    }
}

} // namespace Digikam
