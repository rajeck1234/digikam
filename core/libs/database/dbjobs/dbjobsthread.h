/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-06-01
 * Description : DB Jobs thread for listing and scanning
 *
 * SPDX-FileCopyrightText: 2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DB_JOBS_THREAD_H
#define DIGIKAM_DB_JOBS_THREAD_H

// Local includes

#include "dbengineparameters.h"
#include "dbjobinfo.h"
#include "dbjob.h"
#include "haariface.h"
#include "itemlisterrecord.h"
#include "actionthreadbase.h"
#include "digikam_export.h"

namespace Digikam
{

class DBJob;

class DIGIKAM_DATABASE_EXPORT DBJobsThread : public ActionThreadBase
{
    Q_OBJECT

public:

    explicit DBJobsThread(QObject* const parent);
    ~DBJobsThread() override;

    /**
     * @brief hasErrors: a method to check for jobs errors
     * @return bool: true if the error list is not empty
     */
    bool hasErrors();

    /**
     * @brief A method to get all errors reported from jobs
     * @return String list with errors
     */
    QList<QString>& errorsList();

protected:

    /**
     * @brief Connects the signals of job to the signals of the thread
     * @param j: Job that wanted to be connected
     */
    void connectFinishAndErrorSignals(DBJob* const j);

public Q_SLOTS:

    /**
     * @brief Appends the error string to m_errorsList
     * @param errString: error string reported from the job
     */
    void error(const QString& errString);

Q_SIGNALS:

    void finished();
    void data(const QList<ItemListerRecord>& records);

private:

    QStringList m_errorsList;
};

// ---------------------------------------------

class DIGIKAM_DATABASE_EXPORT AlbumsDBJobsThread : public DBJobsThread
{
    Q_OBJECT

public:

    explicit AlbumsDBJobsThread(QObject* const parent);
    ~AlbumsDBJobsThread() override;

    /**
     * @brief Starts PAlbums listing and scanning job(s)
     * @param info: represents the albums job info
     */
    void albumsListing(const AlbumsDBJobInfo& info);

Q_SIGNALS:

    void foldersData(const QHash<int, int>&);
    void faceFoldersData(const QMap<QString, QHash<int, int> >&);
};

// ---------------------------------------------

class DIGIKAM_DATABASE_EXPORT TagsDBJobsThread : public DBJobsThread
{
    Q_OBJECT

public:

    explicit TagsDBJobsThread(QObject* const parent);
    ~TagsDBJobsThread() override;

    /**
     * @brief Starts tags listing and scanning job(s)
     * @param info: represents the tags job info
     */
    void tagsListing(const TagsDBJobInfo& info);

Q_SIGNALS:

    void foldersData(const QHash<int, int>&);
    void faceFoldersData(const QMap<QString, QHash<int, int> >&);
};

// ---------------------------------------------

class DIGIKAM_DATABASE_EXPORT DatesDBJobsThread : public DBJobsThread
{
    Q_OBJECT

public:

    explicit DatesDBJobsThread(QObject* const parent);
    ~DatesDBJobsThread() override;

    /**
     * @brief Starts dates listing and scanning
     * @param info: represents the dates job info
     */
    void datesListing(const DatesDBJobInfo& info);

Q_SIGNALS:

    void foldersData(const QHash<QDateTime, int>&);
};

// ---------------------------------------------

class DIGIKAM_DATABASE_EXPORT SearchesDBJobsThread : public DBJobsThread
{
    Q_OBJECT

public:

    explicit SearchesDBJobsThread(QObject* const parent);
    ~SearchesDBJobsThread() override;

    /**
     * @brief Starts searches listing and scanning
     * @param info: represents the searches job info
     */
    void searchesListing(const SearchesDBJobInfo& info);

public Q_SLOTS:

    void slotImageProcessed();
    void slotDuplicatesResults(const HaarIface::DuplicatesResultsMap&);

Q_SIGNALS:

    void signalProgress(int percentage);

private:
    HaarIface::DuplicatesResultsMap m_results;
    QScopedPointer<HaarIface>       m_haarIface;
    bool                            m_isAlbumUpdate;
    int                             m_processedImages;
    int                             m_totalImages2Scan;
};

// ---------------------------------------------

class DIGIKAM_DATABASE_EXPORT GPSDBJobsThread : public DBJobsThread
{
    Q_OBJECT

public:

    explicit GPSDBJobsThread(QObject* const parent);
    ~GPSDBJobsThread() override;

    /**
     * @brief Starts GPS listing and scanning
     * @param info: represents the GPS job info
     */
    void GPSListing(const GPSDBJobInfo& info);

Q_SIGNALS:

    void directQueryData(const QList<QVariant>& data);
};

} // namespace Digikam

#endif // DIGIKAM_DB_JOBS_THREAD_H
