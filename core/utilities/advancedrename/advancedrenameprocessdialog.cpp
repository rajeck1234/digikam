/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-11-14
 * Description : process dialog for renaming files
 *
 * SPDX-FileCopyrightText: 2010-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "advancedrenameprocessdialog.h"

// Qt includes

#include <QDialogButtonBox>
#include <QAbstractButton>
#include <QCloseEvent>
#include <QMessageBox>
#include <QPixmap>
#include <QPointer>
#include <QTimer>
#include <QDir>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dio.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

class Q_DECL_HIDDEN AdvancedRenameProcessDialog::Private
{
public:

    explicit Private()
        : thumbLoadThread(nullptr),
          overwrite      (false),
          cancel         (false)
    {
    }

    ThumbnailLoadThread* thumbLoadThread;

    NewNameInfo          currentInfo;
    NewNamesList         newNameList;
    NewNamesList         failedList;

    bool                 overwrite;
    bool                 cancel;

    QPixmap              thumbPixmap;
    QString              thumbPath;
    QString              infoLabel;
};

AdvancedRenameProcessDialog::AdvancedRenameProcessDialog(const NewNamesList& list, QWidget* const parent)
    : DProgressDlg(parent),
      d           (new Private)
{
    d->newNameList     = list;
    d->thumbLoadThread = new ThumbnailLoadThread;
    d->infoLabel       = i18n("<b>Renaming images. Please wait...</b>");

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotGotThumbnail(LoadingDescription,QPixmap)));

    connect(DIO::instance(), SIGNAL(signalRenameFailed(QUrl)),
            this, SLOT(slotRenameFailed(QUrl)));

    connect(DIO::instance(), SIGNAL(signalRenameFinished()),
            this, SLOT(slotRenameFinished()));

    setValue(0);
    setModal(true);
    setLabel(d->infoLabel);
    setButtonText(i18nc("@action:button", "&Abort"));
    setTitle(i18n("Processing..."));
    setWindowTitle(i18nc("@title:window", "Renaming Images"));

    getNextThumbnail();
    setMaximum(d->newNameList.count());
    QTimer::singleShot(500, this, SLOT(slotRenameImages()));
}

AdvancedRenameProcessDialog::~AdvancedRenameProcessDialog()
{
    delete d->thumbLoadThread;
    delete d;
}

void AdvancedRenameProcessDialog::slotRenameImages()
{
    if (d->newNameList.isEmpty())
    {
        slotCancel();
        return;
    }

    processOne();
}

void AdvancedRenameProcessDialog::processOne()
{
    if (d->cancel || d->newNameList.isEmpty())
    {
        return;
    }

    d->currentInfo = d->newNameList.takeFirst();

    addedAction(d->thumbPixmap,
                QDir::toNativeSeparators(d->thumbPath));

    DIO::rename(d->currentInfo.first,
                d->currentInfo.second, d->overwrite);

    getNextThumbnail();
}

void AdvancedRenameProcessDialog::complete()
{
    done(QDialogButtonBox::Cancel);
}

void AdvancedRenameProcessDialog::slotGotThumbnail(const LoadingDescription& desc, const QPixmap& pix)
{
    d->thumbPixmap = pix;
    d->thumbPath   = desc.filePath;
}

void AdvancedRenameProcessDialog::slotCancel()
{
    abort();
    done(QDialogButtonBox::Cancel);
}

void AdvancedRenameProcessDialog::slotRenameFinished()
{
    if (d->cancel)
    {
        return;
    }

    advance(1);

    if (d->newNameList.isEmpty())
    {
        if (!d->failedList.isEmpty())
        {
            QPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::Warning,
                     i18nc("@title:window", "Renaming Images"),
                     i18np("An error occurred while renaming %1 image.\n"
                           "Do you want to rename this image again or "
                           "rename this image by overwriting?",
                           "An error occurred while renaming %1 images.\n"
                           "Do you want to rename these images again or "
                           "rename these images by overwriting?",
                           d->failedList.count()),
                     QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, this);

            msgBox->button(QMessageBox::Yes)->setText(i18nc("@action:button", "Rename Again"));
            msgBox->button(QMessageBox::Yes)->setIcon(QIcon::fromTheme(QLatin1String("document-edit")));
            msgBox->button(QMessageBox::No)->setText(i18nc("@action:button", "Overwrite"));
            msgBox->button(QMessageBox::No)->setIcon(QIcon::fromTheme(QLatin1String("edit-copy")));

            int result = msgBox->exec();
            delete msgBox;

            if      (result == QMessageBox::Cancel)
            {
                d->failedList.clear();
                complete();
            }
            else if (result == QMessageBox::No)
            {
                d->newNameList = d->failedList;
                d->failedList.clear();
                d->overwrite   = true;

                setValue(0);
                getNextThumbnail();
                setLabel(d->infoLabel);
                setMaximum(d->newNameList.count());
                QTimer::singleShot(500, this, SLOT(slotRenameImages()));
            }
            else
            {
                complete();
            }
        }
        else
        {
            complete();
        }
    }
    else
    {
        processOne();
    }
}

void AdvancedRenameProcessDialog::slotRenameFailed(const QUrl& url)
{
    if (d->cancel || (d->currentInfo.first != url))
    {
        return;
    }

    d->failedList << d->currentInfo;

    setLabel(i18n("<b>Renaming images. Errors: %1</b>",
                  d->failedList.count()));
}

void AdvancedRenameProcessDialog::closeEvent(QCloseEvent* e)
{
    abort();
    e->accept();
}

void AdvancedRenameProcessDialog::abort()
{
    d->cancel = true;
    d->failedList.clear();
}

QList<QUrl> AdvancedRenameProcessDialog::failedUrls() const
{
    QList<QUrl> failedUrls;

    Q_FOREACH (const NewNameInfo& info, d->failedList)
    {
        failedUrls << info.first;
    }

    return failedUrls;
}

void AdvancedRenameProcessDialog::getNextThumbnail()
{
    if (!d->newNameList.isEmpty())
    {
        QString path = d->newNameList.first().first.toLocalFile();
        d->thumbLoadThread->find(ThumbnailIdentifier(path));
    }
}

} // namespace Digikam
