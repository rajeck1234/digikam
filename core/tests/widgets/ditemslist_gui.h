/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-10-17
 * Description : test for implementation of ditemslist api
 *
 * SPDX-FileCopyrightText: 2011-2012 by A Janardhan Reddy <annapareddyjanardhanreddy at gmail dot com>
 * SPDX-FileCopyrightText: 2011-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DITEMS_LIST_GUI_H
#define DIGIKAM_DITEMS_LIST_GUI_H

// Qt includes

#include <QUrl>
#include <QDialog>

// Local includes

#include "actionthreadbase.h"

using namespace Digikam;

class ActionThread : public ActionThreadBase
{
    Q_OBJECT

public:

    explicit ActionThread(QObject* const parent);
    ~ActionThread() override;

    void rotate(const QList<QUrl>& list);

Q_SIGNALS:

    void starting(const QUrl& url);
    void finished(const QUrl& url);
    void failed(const QUrl& url, const QString& err);
    void progress(const QUrl& url, int percent);

private Q_SLOTS:

    void slotJobDone();
    void slotJobProgress(int);
    void slotJobStarted();
};

// -----------------------------------------------------

class DItemsListTest : public QDialog
{
    Q_OBJECT

public:

    explicit DItemsListTest(QWidget* const parent = nullptr);
    ~DItemsListTest() override;

private Q_SLOTS:

    void slotStart();
    void slotStarting(const QUrl&);
    void slotFinished(const QUrl&);
    void slotFailed(const QUrl&, const QString&);

private:

    class Private;
    Private* const d;
};

#endif // DIGIKAM_DITEMS_LIST_TEST_H
