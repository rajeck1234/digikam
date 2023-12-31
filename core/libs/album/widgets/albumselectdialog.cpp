/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-16
 * Description : a dialog to select a target album to download
 *               pictures from camera
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2023 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "albumselectdialog.h"

// Qt includes

#include <QLabel>
#include <QGridLayout>
#include <QPixmap>
#include <QPointer>
#include <QStandardPaths>
#include <QApplication>
#include <QStyle>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "album.h"
#include "albummanager.h"
#include "albumselectwidget.h"
#include "albumthumbnailloader.h"
#include "collectionmanager.h"

namespace Digikam
{

class Q_DECL_HIDDEN AlbumSelectDialog::Private
{

public:

    explicit Private()
      : buttons     (nullptr),
        albumSel    (nullptr),
        searchBar   (nullptr)
    {
    }

    QDialogButtonBox*  buttons;

    AlbumSelectWidget* albumSel;

    SearchTextBar*     searchBar;
};

AlbumSelectDialog::AlbumSelectDialog(QWidget* const parent, PAlbum* const albumToSelect, const QString& header)
    : QDialog(parent),
      d      (new Private)
{
    setWindowTitle(i18nc("@title:window", "Select Album"));

    d->buttons = new QDialogButtonBox(QDialogButtonBox::Help | QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);

    // -------------------------------------------------------------

    QWidget* const page     = new QWidget(this);
    QGridLayout* const grid = new QGridLayout(page);
    QLabel* const logo      = new QLabel(page);
    logo->setPixmap(QIcon::fromTheme(QLatin1String("digikam")).pixmap(QSize(48,48)));

    QLabel* const message   = new QLabel(page);
    message->setWordWrap(true);

    if (!header.isEmpty())
    {
        message->setText(header);
    }

    d->albumSel = new AlbumSelectWidget(page, albumToSelect, true);

    grid->addWidget(logo,        0, 0, 1, 1);
    grid->addWidget(message,     1, 0, 1, 1);
    grid->addWidget(d->albumSel, 0, 1, 3, 1);
    grid->setColumnStretch(0, 2);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(2, 10);
    grid->setContentsMargins(QMargins());
    grid->setSpacing(qMin(QApplication::style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing),
                          QApplication::style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing)));

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(page);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    // -------------------------------------------------------------

    connect(d->albumSel, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(d->albumSel, SIGNAL(completerActivated()),
            this, SLOT(accept()));

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));

    connect(d->buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));

    // -------------------------------------------------------------

    resize(500, 500);
    slotSelectionChanged();
}

AlbumSelectDialog::~AlbumSelectDialog()
{
    delete d;
}

void AlbumSelectDialog::slotSelectionChanged()
{
    PAlbum* const currentAlbum = d->albumSel->currentAlbum();

    if (!currentAlbum || (currentAlbum->isRoot()))
    {
        d->buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
        return;
    }

    d->buttons->button(QDialogButtonBox::Ok)->setEnabled(true);
}

PAlbum* AlbumSelectDialog::selectAlbum(QWidget* const parent, PAlbum* const albumToSelect, const QString& header)
{
    QPointer<AlbumSelectDialog> dlg = new AlbumSelectDialog(parent, albumToSelect, header);

    if (dlg->exec() != QDialog::Accepted)
    {
        delete dlg;
        return nullptr;
    }

    PAlbum* const selectedAlbum = dlg->d->albumSel->currentAlbum();

    if (!selectedAlbum || (selectedAlbum->isRoot()))
    {
        delete dlg;
        return nullptr;
    }

    delete dlg;

    return selectedAlbum;
}

void AlbumSelectDialog::slotHelp()
{
    openOnlineDocumentation(QLatin1String("main_window"), QLatin1String("albums_view"));
}

} // namespace Digikam
