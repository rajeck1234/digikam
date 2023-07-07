/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a MediaWiki C++ interface
 *
 * SPDX-FileCopyrightText: 2011-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Alexandre Mendes <alex dot mendes1988 at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Hormiere Guillaume <hormiere dot guillaume at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Manuel Campomanes <campomanes dot manuel at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QObject>
#include <QTest>

// KDE includes

#include <kjob.h>

// Local includes

#include "mediawiki_iface.h"
#include "mediawiki_upload.h"
#include "fakeserver/fakeserver.h"
#include "dtestdatadir.h"
#include "digikam_debug.h"

using MediaWiki::Iface;
using MediaWiki::Upload;

Q_DECLARE_METATYPE(FakeServer::Request)
Q_DECLARE_METATYPE(Upload*)

class Q_DECL_HIDDEN UploadTest : public QObject
{
    Q_OBJECT

public:

    explicit UploadTest(QObject* const parent = nullptr)
    {
        Q_UNUSED(parent);
        uploadCount = 0;
        m_file      = nullptr;
        m_mediaWiki = nullptr;
    }

public Q_SLOTS:

    void uploadHandle(KJob*)
    {
        uploadCount++;
    }

private Q_SLOTS:

    void initTestCase()
    {
        uploadCount          = 0;
        this->m_mediaWiki    = new Iface(QUrl(QStringLiteral("http://127.0.0.1:12566")));       // krazy:exclude=insecurenet
        this->m_infoScenario = QStringLiteral("<api><query><pages><page pageid=\"27697087\" ns=\"0\" title=\"API\" touched=\"2010-11-25T13:59:03Z\" lastrevid=\"367741756\" counter=\"0\" length=\"70\" redirect=\"\" starttimestamp=\"2010-11-25T16:14:51Z\" edittoken=\"cecded1f35005d22904a35cc7b736e18%2B\" talkid=\"5477418\" fullurl=\"https://en.wikipedia.org/wiki/API\" editurl=\"https://en.wikipedia.org/w/index.php?title=API&action=edit\" ><protection /></page></pages></query></api>");

        const QString filePath = DTestDataDir::TestData(QString::fromUtf8("core/tests/mediawiki"))
                                   .root().path() + QLatin1Char('/') + QLatin1String("uploadtest_image.jpg");
        qCDebug(DIGIKAM_TESTS_LOG) << "Test Data File:" << filePath;

        this->m_file         = new QFile(filePath);
        this->m_file->open(QIODevice::ReadOnly);
    }

    void uploadSetters()
    {
        QFETCH(QString, request);
        QFETCH(QString, senario);
        QFETCH(Upload*, job);

        uploadCount = 0;
        FakeServer fakeserver;
        fakeserver.setScenario(m_infoScenario);
        fakeserver.addScenario(senario);
        fakeserver.startAndWait();

        connect(job, SIGNAL(result(KJob*)),
                this, SLOT(uploadHandle(KJob*)));

        job->exec();
        FakeServer::Request serverrequest = fakeserver.getRequest()[1];
        QCOMPARE(serverrequest.type, QStringLiteral("POST"));
        QCOMPARE(serverrequest.value, request);
        QCOMPARE(job->error(), (int)Upload::NoError);
        QCOMPARE(this->uploadCount, 1);
    }

    void uploadSetters_data()
    {
        QTest::addColumn<QString>("request");
        QTest::addColumn<QString>("senario");
        QTest::addColumn<Upload*>("job");

        Upload* const e1 = new Upload( *m_mediaWiki, nullptr);
        e1->setFile(this->m_file);
        e1->setFilename(QStringLiteral("Test.jpeg"));
        e1->setComment(QStringLiteral("Test"));
        e1->setText(QStringLiteral("{{Information|Description=Ajout du logo de l'IUP ISI, Toulouse.|"
                                   "Source=http://www.iupisi.ups-tlse.fr/"                                  // krazy:exclude=insecurenet
                                   "|Date=1992-01-01|Author=iup|Permission={{PD-EEA}}|other_versions=}}"));
        QTest::newRow("Text")
                << QStringLiteral("/?action=upload&format=xml")
                << QStringLiteral("<api><upload result=\"Success\" pageid=\"12\" title=\"Talk:Main Page\" oldrevid=\"465\" newrevid=\"471\" /></api>")
                << e1;
    }

    void error()
    {
        QFETCH(QString, scenario);
        QFETCH(int, error);

        uploadCount = 0;
        Iface MediaWiki(QUrl(QStringLiteral("http://127.0.0.1:12566")));        // krazy:exclude=insecurenet
        FakeServer fakeserver;

        if (scenario != QStringLiteral("error server"))
        {
            fakeserver.setScenario(m_infoScenario);
            fakeserver.addScenario(scenario);
            fakeserver.startAndWait();
        }

        Upload* const job = new Upload(MediaWiki, nullptr);
        job->setFile(this->m_file);
        job->setFilename(QStringLiteral("Test.jpeg"));
        job->setComment(QStringLiteral("Test"));
        job->setText(QStringLiteral("{{Information|Description=Ajout du logo de l'IUP ISI, Toulouse.|"
                                    "Source=http://www.iupisi.ups-tlse.fr/"         // krazy:exclude=insecurenet
                                    "|Date=1992-01-01|Author=iup|Permission={{PD-EEA}}|other_versions=}}"));

        connect(job,  SIGNAL(result(KJob*)),
                this, SLOT(uploadHandle(KJob*)));

        job->exec();

        if (scenario != QStringLiteral("error server"))
        {
            QList<FakeServer::Request> requests = fakeserver.getRequest();
            QCOMPARE(requests.size(), 2);
        }

        QCOMPARE(job->error(), error);
        QCOMPARE(uploadCount, 1);

        if (scenario != QStringLiteral("error server"))
        {
            QVERIFY(fakeserver.isAllScenarioDone());
        }
    }

    void error_data()
    {
        QTest::addColumn<QString>("scenario");
        QTest::addColumn<int>("error");

        QTest::newRow("UploadDisabled")
                << QStringLiteral("<api><upload result=\"Failure\"><error code=\"upload-disabled\" info=\"\" /></upload> </api>")
                << int(Upload::UploadDisabled);

        QTest::newRow("InvalidSessionKey")
                << QStringLiteral("<api><upload result=\"Failure\"><error code=\"invalid-session-key\" info=\"\" /></upload> </api>")
                << int(Upload::InvalidSessionKey);

        QTest::newRow("BadAccess")
                << QStringLiteral("<api><upload result=\"Failure\"><error code=\"bad-access-groups\" info=\"\" /></upload> </api>")
                << int(Upload::BadAccess);

        QTest::newRow("ParamMissing")
                << QStringLiteral("<api><upload result=\"Failure\"><error code=\"missing-param\" info=\"\" /></upload> </api>")
                << int(Upload::ParamMissing);

        QTest::newRow("MustBeLoggedIn")
                << QStringLiteral("<api><upload result=\"Failure\"><error code=\"mustbeloggedin\" info=\"\" /></upload> </api>")
                << int(Upload::MustBeLoggedIn);

        QTest::newRow("FetchFileError")
                << QStringLiteral("<api><upload result=\"Failure\"><error code=\"fetchfile-error\" info=\"\" /></upload> </api>")
                << int(Upload::FetchFileError);

        QTest::newRow("NoModule")
                << QStringLiteral("<api><upload result=\"Failure\"><error code=\"no-module\" info=\"\" /></upload> </api>")
                << int(Upload::NoModule);

        QTest::newRow("EmptyFile")
                << QStringLiteral("<api><upload result=\"Failure\"><error code=\"emptyfile\" info=\"\" /></upload> </api>")
                << int(Upload::EmptyFile);

        QTest::newRow("ExtensionMissing")
                << QStringLiteral("<api><upload result=\"Failure\"><error code=\"filetype-missing\" info=\"\" /></upload> </api>")
                << int(Upload::ExtensionMissing);

        QTest::newRow("TooShortFilename")
                << QStringLiteral("<api><upload result=\"Failure\"><error code=\"filenametooshort\" info=\"\" /></upload> </api>")
                << int(Upload::TooShortFilename);

        QTest::newRow("OverWriting")
                << QStringLiteral("<api><upload result=\"Failure\"><error code=\"overwrite\" info=\"\" /></upload> </api>")
                << int(Upload::OverWriting);

        QTest::newRow("StashFailed")
                << QStringLiteral("<api><upload result=\"Failure\"><error code=\"stashfailed\" info=\"\" /></upload> </api>")
                << int(Upload::StashFailed);

        QTest::newRow("InternalError")
                << QStringLiteral("<api><upload result=\"Failure\"><error code=\"internal-error\" info=\"\" /></upload> </api>")
                << int(Upload::InternalError);
    }

    void cleanupTestCase()
    {
        delete this->m_mediaWiki;
        delete this->m_file;
    }

private:

    int        uploadCount;
    QString    request;
    QString    m_infoScenario;
    QIODevice* m_file;
    Iface*     m_mediaWiki;
};

QTEST_MAIN(UploadTest)

#include "upload_utest.moc"
