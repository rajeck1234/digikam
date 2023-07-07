/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-06-01
 * Description : DB Jobs for listing and scanning
 *
 * SPDX-FileCopyrightText: 2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DB_JOB_H
#define DIGIKAM_DB_JOB_H

// Local includes

#include "dbjobinfo.h"
#include "itemlisterrecord.h"
#include "duplicatesprogressobserver.h"
#include "actionthreadbase.h"
#include "digikam_export.h"

namespace Digikam
{

class DuplicatesProgressObserver;

class DIGIKAM_DATABASE_EXPORT DBJob : public ActionJob
{
    Q_OBJECT

protected:

    explicit DBJob();
    ~DBJob()        override;

Q_SIGNALS:

    void data(const QList<ItemListerRecord>& records);
    void error(const QString& err);

private:

    // Disable
    explicit DBJob(QObject*) = delete;
};

// ----------------------------------------------

class DIGIKAM_DATABASE_EXPORT AlbumsJob : public DBJob
{
    Q_OBJECT

public:

    explicit AlbumsJob(const AlbumsDBJobInfo& jobInfo);
    ~AlbumsJob()    override;

protected:

    void run()      override;

Q_SIGNALS:

    void foldersData(const QHash<int, int>&);

private:

    AlbumsDBJobInfo m_jobInfo;

private:

    // Disable
    AlbumsJob(QObject*);
};

// ----------------------------------------------

class DIGIKAM_DATABASE_EXPORT DatesJob : public DBJob
{
    Q_OBJECT

public:

    explicit DatesJob(const DatesDBJobInfo& jobInfo);
    ~DatesJob()     override;

protected:

    void run()      override;

Q_SIGNALS:

    void foldersData(const QHash<QDateTime, int>& datesStatMap);

private:

    DatesDBJobInfo m_jobInfo;

private:

    // Disable
    DatesJob(QObject*) = delete;
};

// ----------------------------------------------

class DIGIKAM_DATABASE_EXPORT GPSJob : public DBJob
{
    Q_OBJECT

public:

    explicit GPSJob(const GPSDBJobInfo& jobInfo);
    ~GPSJob()       override;

protected:

    void run()      override;

Q_SIGNALS:

    void directQueryData(const QList<QVariant>& data);

private:

    GPSDBJobInfo m_jobInfo;

private:

    // Disable
    GPSJob(QObject*);
};

// ----------------------------------------------

class DIGIKAM_DATABASE_EXPORT TagsJob : public DBJob
{
    Q_OBJECT

public:

    explicit TagsJob(const TagsDBJobInfo& jobInfo);
    ~TagsJob()      override;

protected:

    void run()      override;

Q_SIGNALS:

    void foldersData(const QHash<int, int>& data);
    void faceFoldersData(const QMap<QString, QHash<int, int> >& data);

private:

    TagsDBJobInfo m_jobInfo;

private:

    // Disable
    TagsJob(QObject*);
};

// ----------------------------------------------

class DIGIKAM_DATABASE_EXPORT SearchesJob : public DBJob
{
    Q_OBJECT

public:

    explicit SearchesJob(const SearchesDBJobInfo& jobInfo);
    SearchesJob(const SearchesDBJobInfo& jobInfo,
                const QSet<qlonglong>::const_iterator& begin,
                const QSet<qlonglong>::const_iterator& end,
                HaarIface* iface);

    ~SearchesJob()  override;

    bool isCanceled() const;

Q_SIGNALS:

    void signalImageProcessed();
    void signalDuplicatesResults(const HaarIface::DuplicatesResultsMap&);

protected:

    void run()      override;

private:

    SearchesDBJobInfo                m_jobInfo;
    QSet<qlonglong>::const_iterator  m_begin;
    QSet<qlonglong>::const_iterator  m_end;
    HaarIface*                       m_iface;

private:

    // Disable
    SearchesJob(QObject*);

    void runSearches();
    void runFindDuplicates();
};

} // namespace Digikam

#endif // DIGIKAM_DB_JOB_H
