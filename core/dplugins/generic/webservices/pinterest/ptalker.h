/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-05-20
 * Description : a tool to export images to Pinterest web service
 *
 * SPDX-FileCopyrightText: 2018      by Tarek Talaat <tarektalaat93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_P_TALKER_H
#define DIGIKAM_P_TALKER_H

// Qt includes

#include <QList>
#include <QPair>
#include <QString>
#include <QNetworkReply>

// Local includes

#include "pitem.h"
#include "dmetadata.h"

using namespace Digikam;

namespace DigikamGenericPinterestPlugin
{

class PTalker : public QObject
{
    Q_OBJECT

public:

    explicit PTalker(QWidget* const parent);
    ~PTalker() override;

public:

    void link();
    void unLink();
    void getUserName();
    bool authenticated();
    void cancel();
    bool addPin(const QString& imgPath,
                const QString& boradID,
                bool rescale, int maxDim, int imageQuality);
    void listBoards(const QString& path = QString());
    void createBoard(QString& boardName);
    void setAccessToken(const QString& token);
    QMap<QString, QString> ParseUrlParameters(const QString& url);
    void getToken(const QString& code);
    void readSettings();
    void writeSettings();

Q_SIGNALS:

    void signalBusy(bool val);
    void signalLinkingSucceeded();
    void signalLinkingFailed();
    void signalNetworkError();
    void signalSetUserName(const QString& msg);
    void signalListBoardsFailed(const QString& msg);
    void signalListBoardsDone(const QList<QPair<QString, QString> >& list);
    void signalCreateBoardFailed(const QString& msg);
    void signalCreateBoardSucceeded();
    void signalAddPinFailed(const QString& msg);
    void signalAddPinSucceeded();
    void pinterestLinkingSucceeded();
    void pinterestLinkingFailed();

private Q_SLOTS:

    void slotLinkingFailed();
    void slotLinkingSucceeded();
    void slotCatchUrl(const QUrl& url);
    void slotFinished(QNetworkReply* reply);

private:

    void parseResponseUserName(const QByteArray& data);
    void parseResponseListBoards(const QByteArray& data);
    void parseResponseCreateBoard(const QByteArray& data);
    void parseResponseAddPin(const QByteArray& data);
    void parseResponseAccessToken(const QByteArray& data);

private:

    class Private;
    Private* const d;
};

} // namespace DigikamGenericPinterestPlugin

#endif // DIGIKAM_P_TALKER_H
