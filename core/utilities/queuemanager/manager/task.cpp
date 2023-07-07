/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-06
 * Description : Thread actions task.
 *
 * SPDX-FileCopyrightText: 2009-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Pankaj Kumar <me at panks dot me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "task.h"

// Qt includes

#include <QFileInfo>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "dimg.h"
#include "dmetadata.h"
#include "iteminfo.h"
#include "batchtool.h"
#include "batchtoolsfactory.h"
#include "dfileoperations.h"

namespace Digikam
{

class Q_DECL_HIDDEN Task::Private
{
public:

    explicit Private()
      : cancel(false),
        tool  (nullptr)
    {
    }

    bool               cancel;

    BatchTool*         tool;

    QueueSettings      settings;
    AssignedBatchTools tools;
};

// -------------------------------------------------------

Task::Task()
    : ActionJob(),
      d        (new Private)
{
}

Task::~Task()
{
    slotCancel();
    delete d;
}

void Task::setSettings(const QueueSettings& settings)
{
    d->settings = settings;
}

void Task::setItem(const AssignedBatchTools& tools)
{
    d->tools = tools;
}

void Task::slotCancel()
{
    if (d->tool)
    {
        d->tool->cancel();
    }

    d->cancel = true;
}

void Task::removeTempFiles(const QList<QUrl>& tmpList)
{
    Q_FOREACH (const QUrl& url, tmpList)
    {
        QString tmpPath(url.toLocalFile());
        QFile::remove(tmpPath);

        tmpPath = DMetadata::sidecarPath(tmpPath);

        if (QFile::exists(tmpPath))
        {
            QFile::remove(tmpPath);
        }
    }
}

void Task::emitActionData(ActionData::ActionStatus st,
                          const QString& mess,
                          const QUrl& dest,
                          bool noWrite)
{
    ActionData ad;
    ad.fileUrl = d->tools.m_itemUrl;
    ad.status  = st;
    ad.message = mess;
    ad.destUrl = dest;
    ad.noWrite = noWrite;

    Q_EMIT signalFinished(ad);
}

void Task::run()
{
    if (d->cancel)
    {
        return;
    }

    emitActionData(ActionData::BatchStarted);

    // Loop with all batch tools operations to apply on item.

    bool        success = false;
    int         index   = 0;
    QUrl        outUrl  = d->tools.m_itemUrl;
    QUrl        workUrl = !d->settings.useOrgAlbum ? d->settings.workingUrl
                                                   : d->tools.m_itemUrl.adjusted(QUrl::RemoveFilename);
    workUrl             = workUrl.adjusted(QUrl::StripTrailingSlash);
    QUrl        inUrl;
    QList<QUrl> tmp2del;
    DImg        tmpImage;
    QString     errMsg;

    // ItemInfo must be tread-safe.

    ItemInfo source     = ItemInfo::fromUrl(d->tools.m_itemUrl);
    bool isMetadataTool = false;
    bool timeAdjust     = false;

    Q_FOREACH (const BatchToolSet& set, d->tools.m_toolsList)
    {
        BatchTool* const tool = BatchToolsFactory::instance()->findTool(set.name, set.group);

        if (!tool)
        {
            emitActionData(ActionData::BatchFailed, i18n("Failed to find tool..."));
            removeTempFiles(tmp2del);

            Q_EMIT signalDone();

            return;
        }

        d->tool               = tool->clone();
        d->tool->setToolIcon(tool->toolIcon());
        d->tool->setToolTitle(tool->toolTitle());
        d->tool->setToolDescription(tool->toolDescription());

        // Only true if it is also the last tool

        isMetadataTool = (set.group == BatchTool::MetadataTool);
        timeAdjust    |= (set.name == QLatin1String("TimeAdjust"));
        inUrl          = outUrl;
        index          = set.index + 1;

        qCDebug(DIGIKAM_GENERAL_LOG) << "Tool : index= " << index
                 << " :: name= "     << set.name
                 << " :: group= "    << set.group
                 << " :: wurl= "     << workUrl;

        d->tool->setImageData(tmpImage);
        d->tool->setItemInfo(source);
        d->tool->setInputUrl(inUrl);
        d->tool->setWorkingUrl(workUrl);
        d->tool->setSettings(set.settings);
        d->tool->setIOFileSettings(d->settings.ioFileSettings);
        d->tool->setRawLoadingRules(d->settings.rawLoadingRule);
        d->tool->setDRawDecoderSettings(d->settings.rawDecodingSettings);
        d->tool->setResetExifOrientationAllowed(d->settings.exifSetOrientation);

        if      (index == d->tools.m_toolsList.count())
        {
            d->tool->setLastChainedTool(true);
        }
        else if (d->tools.m_toolsList[index].group == BatchTool::CustomTool)
        {
            // If the next tool is under the custom group (user script)
            // treat as the last chained tool, i.e. save image to file

            d->tool->setLastChainedTool(true);
        }
        else
        {
            d->tool->setLastChainedTool(false);
        }

        d->tool->setSaveAsNewVersion(d->settings.saveAsNewVersion);
        d->tool->setOutputUrlFromInputUrl();
        d->tool->setBranchHistory(true);

        outUrl   = d->tool->outputUrl();
        success  = d->tool->apply();
        tmpImage = d->tool->imageData();
        errMsg   = d->tool->errorDescription();
        tmp2del.append(outUrl);

        delete d->tool;
        d->tool = nullptr;

        if      (d->cancel)
        {
            emitActionData(ActionData::BatchCanceled);
            removeTempFiles(tmp2del);

            Q_EMIT signalDone();

            return;
        }
        else if (!success)
        {
            emitActionData(ActionData::BatchFailed, errMsg);
            break;
        }
    }

    // Clean up all tmp url.

    // We don't remove last output tmp url.

    tmp2del.removeAll(outUrl);
    removeTempFiles(tmp2del);

    // Move processed temp file to target

    QString renameMess;
    QUrl dest = workUrl;
    dest.setPath(dest.path() + QLatin1Char('/') + d->tools.m_destFileName);

    if (QFileInfo::exists(dest.toLocalFile()))
    {
        if      (d->settings.conflictRule == FileSaveConflictBox::OVERWRITE)
        {
            renameMess = i18n("(overwritten)");
        }
        else if (d->settings.conflictRule == FileSaveConflictBox::DIFFNAME)
        {
            dest       = DFileOperations::getUniqueFileUrl(dest);
            renameMess = i18n("(renamed to %1)", dest.fileName());
        }
        else
        {
            emitActionData(ActionData::BatchSkipped,
                           i18n("Item exists and was skipped"), dest);

            removeTempFiles(QList<QUrl>() << outUrl);

            Q_EMIT signalDone();

            return;
        }
    }

    if (QFileInfo(outUrl.toLocalFile()).size() == 0)
    {
        removeTempFiles(QList<QUrl>() << outUrl);
        dest.clear();
    }

    if (!dest.isEmpty())
    {
        if (DMetadata::hasSidecar(outUrl.toLocalFile()))
        {
            if (!DFileOperations::localFileRename(d->tools.m_itemUrl.toLocalFile(),
                                                  DMetadata::sidecarPath(outUrl.toLocalFile()),
                                                  DMetadata::sidecarPath(dest.toLocalFile()),
                                                  timeAdjust))
            {
                emitActionData(ActionData::BatchFailed, i18n("Failed to create sidecar file..."), dest);
            }
        }

        if (DFileOperations::localFileRename(d->tools.m_itemUrl.toLocalFile(),
                                             outUrl.toLocalFile(),
                                             dest.toLocalFile(),
                                             timeAdjust))
        {
            emitActionData(ActionData::BatchDone, i18n("Item processed successfully %1", renameMess),
                           dest, isMetadataTool);
        }
        else
        {
            emitActionData(ActionData::BatchFailed, i18n("Failed to create file..."), dest);
        }
    }
    else
    {
        emitActionData(ActionData::BatchFailed, i18n("Failed to create file..."), dest);
    }

    Q_EMIT signalDone();
}

} // namespace Digikam
