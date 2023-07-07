/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-04-12
 * Description : A tool to export items to Rajce web service
 *
 * SPDX-FileCopyrightText: 2011      by Lukas Krejci <krejci.l at centrum dot cz>
 * SPDX-FileCopyrightText: 2011-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_RAJCE_COMMAND_H
#define DIGIKAM_RAJCE_COMMAND_H

// Qt includes

#include <QImage>
#include <QObject>
#include <QString>

// Local includes

#include "rajcesession.h"
#include "rajcempform.h"

class QXmlQuery;

namespace DigikamGenericRajcePlugin
{

class RajceCommand : public QObject
{
    Q_OBJECT

public:

    explicit RajceCommand(const QString& name, RajceCommandType commandType);
    ~RajceCommand() override;

public:

    void processResponse(const QString& response, RajceSession& state);

    QString            getXml()          const;
    RajceCommandType   commandType()     const;
    virtual QByteArray encode()          const;
    virtual QString    contentType()     const;

protected:

    virtual void parseResponse(QXmlQuery& query, RajceSession& state) = 0;
    virtual void cleanUpOnError(RajceSession& state)                  = 0;

    // Allow modification in const methods for lazy init to be possible
    QMap<QString, QString>& parameters() const;

    // Aooend additional xml after the "parameters"
    virtual QString additionalXml()      const;

private:

    bool parseErrorFromQuery(QXmlQuery& query, RajceSession& state);

private:

    // Disable
    explicit RajceCommand(QObject*)              = delete;
    RajceCommand(const RajceCommand&)            = delete;
    RajceCommand& operator=(const RajceCommand&) = delete;

private:

    class Private;
    Private* const d;
};

// -----------------------------------------------------------------------

class LoginCommand : public RajceCommand
{
    Q_OBJECT

public:

    explicit LoginCommand(const QString& username, const QString& password);

protected:

    void parseResponse(QXmlQuery& response, RajceSession& state) override;
    void cleanUpOnError(RajceSession& state)                     override;

private:

    // Disable
    explicit LoginCommand(QObject*) = delete;
};

// -----------------------------------------------------------------------

class OpenAlbumCommand : public RajceCommand
{
    Q_OBJECT

public:

    explicit OpenAlbumCommand(unsigned albumId, const RajceSession& state);

protected:

    void parseResponse(QXmlQuery& response, RajceSession& state) override;
    void cleanUpOnError(RajceSession& state)                     override;

private:

    // Disable
    explicit OpenAlbumCommand(QObject*) = delete;
};

// -----------------------------------------------------------------------

class CreateAlbumCommand : public RajceCommand
{
    Q_OBJECT

public:

    explicit CreateAlbumCommand(const QString& name,
                                const QString& description,
                                bool  visible,
                                const RajceSession& state);

protected:

    void parseResponse(QXmlQuery& response, RajceSession& state) override;
    void cleanUpOnError(RajceSession& state)                     override;

private:

    // Disable
    explicit CreateAlbumCommand(QObject*) = delete;
};

// -----------------------------------------------------------------------

class CloseAlbumCommand : public RajceCommand
{
    Q_OBJECT

public:

    explicit CloseAlbumCommand(const RajceSession& state);

protected:

    void parseResponse(QXmlQuery& response, RajceSession& state) override;
    void cleanUpOnError(RajceSession& state)                     override;

private:

    // Disable
    explicit CloseAlbumCommand(QObject*) = delete;
};

// -----------------------------------------------------------------------

class AlbumListCommand : public RajceCommand
{
    Q_OBJECT

public:

    explicit AlbumListCommand(const RajceSession&);

protected:

    void parseResponse(QXmlQuery& response, RajceSession& state) override;
    void cleanUpOnError(RajceSession& state)                     override;

private:

    // Disable
    explicit AlbumListCommand(QObject*) = delete;
};

// -----------------------------------------------------------------------

class AddPhotoCommand : public RajceCommand
{
    Q_OBJECT

public:

    explicit AddPhotoCommand(const    QString& tmpDir,
                             const    QString& path,
                             unsigned dimension,
                             int      jpgQuality,
                             const    RajceSession& state);
    ~AddPhotoCommand() override;

public:

    QByteArray encode()                                          const override;
    QString    contentType()                                     const override;

protected:

    void    cleanUpOnError(RajceSession& state)                        override;
    void    parseResponse(QXmlQuery& query, RajceSession& state)       override;
    QString additionalXml()                                      const override;

private:

    // Disable
    explicit AddPhotoCommand(QObject*) = delete;

private:

    class Private;
    Private* const d;
};

} // namespace DigikamGenericRajcePlugin

#endif // DIGIKAM_RAJCE_COMMAND_H
