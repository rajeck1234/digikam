/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-12-28
 * Description : a class to manage Raw to Png conversion using threads
 *
 * SPDX-FileCopyrightText: 2011-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2014 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "rawtopngconverterthread.h"

// Qt includes

#include <QFileInfo>
#include <QImage>
#include <QPointer>

// Local includes

#include "digikam_debug.h"
#include "drawdecoder.h"

class Q_DECL_HIDDEN Mytask : public ActionJob
{
    Q_OBJECT

public:

    Mytask(QObject* const parent = nullptr)
        : ActionJob(parent)
    {
    }

    DRawDecoderSettings settings;
    QString             errString;
    QUrl                fileUrl;

protected:

    void run() override
    {
        Q_EMIT signalStarted();

        QImage image;

        if (m_cancel) return;

        Q_EMIT signalProgress(20);

        DRawDecoder* const rawProcessor = new DRawDecoder;

        if (m_cancel)
        {
            delete rawProcessor;
            return;
        }

        Q_EMIT signalProgress(30);

        QFileInfo input(fileUrl.toLocalFile());
        QString   fullFilePath(input.baseName() + QString::fromLatin1(".full.png"));
        QFileInfo fullOutput(fullFilePath);

        if (m_cancel)
        {
            delete rawProcessor;
            return;
        }

        Q_EMIT signalProgress(40);

        if (!rawProcessor->loadFullImage(image, fileUrl.toLocalFile(), settings))
        {
            errString = QString::fromLatin1("raw2png: Loading full RAW image failed. Aborted...");
            delete rawProcessor;
            return;
        }

        if (m_cancel)
        {
            delete rawProcessor;
            return;
        }

        Q_EMIT signalProgress(60);

        qCDebug(DIGIKAM_TESTS_LOG) << "raw2png: Saving full RAW image to "
                 << fullOutput.fileName() << " size ("
                 << image.width() << "x" << image.height()
                 << ")";

        if (m_cancel)
        {
            delete rawProcessor;
            return;
        }

        Q_EMIT signalProgress(80);

        image.save(fullFilePath, "PNG");

        Q_EMIT signalDone();

        delete rawProcessor;
    }
};

// ----------------------------------------------------------------------------------------------------

RAWToPNGConverterThread::RAWToPNGConverterThread(QObject* const parent)
    : ActionThreadBase(parent)
{
    setObjectName(QLatin1String("RAWToPNGConverterThread"));
}

RAWToPNGConverterThread::~RAWToPNGConverterThread()
{
}

void RAWToPNGConverterThread::convertRAWtoPNG(const QList<QUrl>& list, const DRawDecoderSettings& settings, int priority)
{
    ActionJobCollection collection;

    Q_FOREACH (const QUrl& url, list)
    {
        Mytask* const job = new Mytask();
        job->fileUrl      = url;
        job->settings     = settings;

        connect(job, SIGNAL(signalStarted()),
                this, SLOT(slotJobStarted()));

        connect(job, SIGNAL(signalProgress(int)),
                this, SLOT(slotJobProgress(int)));

        connect(job, SIGNAL(signalDone()),
                this, SLOT(slotJobDone()));

        collection.insert(job, priority);

        qCDebug(DIGIKAM_TESTS_LOG) << "Appending file to process " << url;
    }

    appendJobs(collection);
}

void RAWToPNGConverterThread::slotJobDone()
{
    Mytask* const task = dynamic_cast<Mytask*>(sender());

    if (!task)
    {
        return;
    }

    if (task->errString.isEmpty())
    {
        Q_EMIT finished(task->fileUrl);
    }
    else
    {
        Q_EMIT failed(task->fileUrl, task->errString);
    }
}

void RAWToPNGConverterThread::slotJobProgress(int p)
{
    Mytask* const task = dynamic_cast<Mytask*>(sender());

    if (!task)
    {
        return;
    }

    Q_EMIT progress(task->fileUrl, p);
}

void RAWToPNGConverterThread::slotJobStarted()
{
    Mytask* const task = dynamic_cast<Mytask*>(sender());

    if (!task)
    {
        return;
    }

    Q_EMIT starting(task->fileUrl);
}

#include "rawtopngconverterthread.moc"
