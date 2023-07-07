/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-01-11
 * Description : a tool to print images
 *
 * SPDX-FileCopyrightText: 2008-2012 by Angelo Naselli <anaselli at linux dot it>
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "advprintwizard.h"

// C++ includes

#include <memory>

// Qt includes

#include <QFileInfo>
#include <QPalette>
#include <QtGlobal>
#include <QDomDocument>
#include <QContextMenuEvent>
#include <QStringView>
#include <QStandardPaths>
#include <QMenu>
#include <QIcon>
#include <QLocale>
#include <QTemporaryDir>
#include <QKeyEvent>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_globals.h"
#include "digikam_debug.h"
#include "advprintthread.h"
#include "advprintintropage.h"
#include "advprintalbumspage.h"
#include "advprintphotopage.h"
#include "advprintcaptionpage.h"
#include "advprintcroppage.h"
#include "advprintoutputpage.h"
#include "advprintfinalpage.h"
#include "templateicon.h"
#include "dwizardpage.h"
#include "dinfointerface.h"
#include "dfiledialog.h"
#include "dmetadata.h"

namespace DigikamGenericPrintCreatorPlugin
{

class Q_DECL_HIDDEN AdvPrintWizard::Private
{
public:

    explicit Private()
      : introPage    (nullptr),
        albumsPage   (nullptr),
        photoPage    (nullptr),
        captionPage  (nullptr),
        cropPage     (nullptr),
        outputPage   (nullptr),
        finalPage    (nullptr),
        settings     (nullptr),
        previewThread(nullptr),
        iface        (nullptr),
        tempPath     (nullptr)
    {
    }

    AdvPrintIntroPage*   introPage;
    AdvPrintAlbumsPage*  albumsPage;
    AdvPrintPhotoPage*   photoPage;
    AdvPrintCaptionPage* captionPage;
    AdvPrintCropPage*    cropPage;
    AdvPrintOutputPage*  outputPage;
    AdvPrintFinalPage*   finalPage;
    AdvPrintSettings*    settings;
    AdvPrintThread*      previewThread;
    DInfoInterface*      iface;

    QTemporaryDir*       tempPath;
};

AdvPrintWizard::AdvPrintWizard(QWidget* const parent, DInfoInterface* const iface)
    : DWizardDlg(parent, QLatin1String("PrintCreatorDialog")),
      d         (new Private)
{
    setWindowTitle(i18nc("@title:window", "Print Creator"));

    d->iface           = iface;
    d->settings        = new AdvPrintSettings;
    d->previewThread   = new AdvPrintThread(this);

    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group("PrintCreator");
    d->settings->readSettings(group);

    d->introPage       = new AdvPrintIntroPage(this,   i18n("Welcome to Print Creator"));
    d->albumsPage      = new AdvPrintAlbumsPage(this,  i18n("Albums Selection"));
    d->photoPage       = new AdvPrintPhotoPage(this,   i18n("Select Page Layout"));
    d->captionPage     = new AdvPrintCaptionPage(this, i18n("Caption Settings"));
    d->cropPage        = new AdvPrintCropPage(this,    i18n("Crop and Rotate Photos"));
    d->outputPage      = new AdvPrintOutputPage(this,  i18n("Images Output Settings"));
    d->finalPage       = new AdvPrintFinalPage(this,   i18n("Render Printing"));
    d->finalPage->setPhotoPage(d->photoPage);

    // -----------------------------------

    connect(button(QWizard::CancelButton), SIGNAL(clicked()),
            this, SLOT(reject()));

    connect(d->photoPage->imagesList(), SIGNAL(signalImageListChanged()),
            d->captionPage, SLOT(slotUpdateImagesList()));

    connect(d->previewThread, SIGNAL(signalPreview(QImage)),
            this, SLOT(slotPreview(QImage)));

    d->tempPath           = new QTemporaryDir();
    d->settings->tempPath = d->tempPath->path();

    installEventFilter(this);
}

AdvPrintWizard::~AdvPrintWizard()
{
    d->previewThread->cancel();

    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group("PrintCreator");
    d->settings->writeSettings(group);

    delete d->settings;
    delete d->tempPath;
    delete d;
}

DInfoInterface* AdvPrintWizard::iface() const
{
    return d->iface;
}

AdvPrintSettings* AdvPrintWizard::settings() const
{
    return d->settings;
}

int AdvPrintWizard::nextId() const
{
    if (d->settings->selMode == AdvPrintSettings::ALBUMS)
    {
        if (currentPage() == d->introPage)
        {
            return d->albumsPage->id();
        }
    }
    else
    {
        if (currentPage() == d->introPage)
        {
            return d->photoPage->id();
        }
    }

    if (d->settings->printerName == d->settings->outputName(AdvPrintSettings::FILES))
    {
        if (currentPage() == d->cropPage)
        {
            return d->outputPage->id();
        }
    }
    else
    {
        if (currentPage() == d->cropPage)
        {
            return d->finalPage->id();
        }
    }

    return DWizardDlg::nextId();
}

QList<QUrl> AdvPrintWizard::itemsList() const
{
    QList<QUrl> urls;

    for (QList<AdvPrintPhoto*>::iterator it = d->settings->photos.begin() ;
         it != d->settings->photos.end() ; ++it)
    {
        AdvPrintPhoto* const photo = static_cast<AdvPrintPhoto*>(*it);
        urls << photo->m_url;
    }

    return urls;
}

void AdvPrintWizard::setItemsList(const QList<QUrl>& fileList)
{
    QList<QUrl> list = fileList;

    for (int i = 0 ; i < d->settings->photos.count() ; ++i)
    {
        delete d->settings->photos.at(i);
    }

    d->settings->photos.clear();

    if (list.isEmpty() && d->iface)
    {
        list = d->iface->currentSelectedItems();
    }

    for (int i = 0 ; i < list.count() ; ++i)
    {
        AdvPrintPhoto* const photo = new AdvPrintPhoto(150, d->iface);
        photo->m_url               = list[i];
        photo->m_first             = true;
        d->settings->photos.append(photo);
    }

    d->cropPage->ui()->BtnCropPrev->setEnabled(false);

    if (d->settings->photos.count() == 1)
    {
        d->cropPage->ui()->BtnCropNext->setEnabled(false);
    }

    Q_EMIT currentIdChanged(d->photoPage->id());
}

void AdvPrintWizard::updateCropFrame(AdvPrintPhoto* const photo, int photoIndex)
{
    int sizeIndex              = d->photoPage->ui()->ListPhotoSizes->currentRow();
    AdvPrintPhotoSize* const s = d->settings->photosizes.at(sizeIndex);

    d->cropPage->ui()->cropFrame->init(photo,
                                       d->settings->getLayout(photoIndex, sizeIndex)->width(),
                                       d->settings->getLayout(photoIndex, sizeIndex)->height(),
                                       s->m_autoRotate,
                                       true);

    d->cropPage->ui()->LblCropPhoto->setText(i18n("Photo %1 of %2",
                                             photoIndex + 1,
                                             d->settings->photos.count()));
}

void AdvPrintWizard::previewPhotos()
{
    if (d->settings->photosizes.isEmpty())
    {
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Photo sizes is null";
        return;
    }

    // get the selected layout

    int photoCount             = d->settings->photos.count();
    int curr1                  = d->photoPage->ui()->ListPhotoSizes->currentRow();
    AdvPrintPhotoSize* const s = d->settings->photosizes.at(curr1);
    int emptySlots             = 0;
    int pageCount              = 0;
    int photosPerPage          = 0;

    if (photoCount > 0)
    {
        // how many pages?  Recall that the first layout item is the paper size

        photosPerPage = s->m_layouts.count() - 1;
        int remainder = photoCount % photosPerPage;

        if (remainder > 0)
        {
            emptySlots = photosPerPage - remainder;
        }

        pageCount     = photoCount / photosPerPage;

        if (emptySlots > 0)
        {
            pageCount++;
        }
    }

    d->photoPage->ui()->LblPhotoCount->setText(QString::number(photoCount));
    d->photoPage->ui()->LblSheetsPrinted->setText(QString::number(pageCount));
    d->photoPage->ui()->LblEmptySlots->setText(QString::number(emptySlots));

    if (photoCount > 0)
    {
        // photo previews
        // preview the first page.
        // find the first page of photos

        int page        = 0;
        int count       = 0;
        int current     = 0;
        int currentPage = 0;

        for (QList<AdvPrintPhoto*>::iterator it = d->settings->photos.begin() ;
            it != d->settings->photos.end() ; ++it)
        {
            AdvPrintPhoto* const photo = static_cast<AdvPrintPhoto*>(*it);

            photo->m_cropRegion.setRect(-1, -1, -1, -1);
            photo->m_rotation  = 0;
            QRect* const curr2 = s->m_layouts.at(count + 1);
            photo->updateCropRegion(curr2->width(),
                                    curr2->height(),
                                    s->m_autoRotate);

            count++;

            if (count >= photosPerPage)
            {
                if (page == d->settings->currentPreviewPage)
                {
                    currentPage = current;
                }

                page++;
                current += photosPerPage;
                count    = 0;
            }
        }

        // send this photo list to the painter

        AdvPrintSettings* const pwSettings = new AdvPrintSettings;
        pwSettings->photos                 = d->settings->photos;
        pwSettings->outputLayouts          = s;
        pwSettings->currentPreviewPage     = currentPage;
        pwSettings->disableCrop            = d->cropPage->ui()->m_disableCrop->isChecked();

        d->previewThread->preview(pwSettings, d->photoPage->ui()->BmpFirstPagePreview->size());
        d->previewThread->start();
    }
    else
    {
        d->photoPage->ui()->BmpFirstPagePreview->clear();
        d->photoPage->ui()->LblPreview->clear();
        d->photoPage->ui()->LblPreview->setText(i18n("Page %1 of %2", 0, 0));
        d->photoPage->manageBtnPreviewPage();
        d->photoPage->update();
    }
}

void AdvPrintWizard::slotPreview(const QImage& img)
{
    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Receive Preview" << img.size();

    d->photoPage->ui()->BmpFirstPagePreview->clear();
    d->photoPage->ui()->BmpFirstPagePreview->setPixmap(QPixmap::fromImage(img));
    d->photoPage->ui()->LblPreview->setText(i18n("Page %1 of %2",
                                                 d->settings->currentPreviewPage + 1,
                                                 d->photoPage->getPageCount()));
    d->photoPage->manageBtnPreviewPage();
    d->photoPage->update();
}

int AdvPrintWizard::normalizedInt(double n)
{
    return (int)(n + 0.5);
}

bool AdvPrintWizard::eventFilter(QObject* o, QEvent* e)
{
    if (e && (e->type() == QEvent::KeyRelease))
    {
        QKeyEvent* const k = (QKeyEvent*)e;

        if ((k->key() == Qt::Key_PageUp)   ||
            (k->key() == Qt::Key_PageDown) ||
            (k->key() == Qt::Key_Up)       ||
            (k->key() == Qt::Key_Down))
        {
            if (currentPage() == d->cropPage)
            {
                // Pass the key event to move crop frame region.

                d->cropPage->ui()->cropFrame->setFocus();
                QApplication::sendEvent(d->cropPage->ui()->cropFrame, e);

                return true; // eat event
            }
        }
    }

    return QWizard::eventFilter(o, e);
}

} // namespace DigikamGenericPrintCreatorPlugin
