// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "androidconfigurations.h"

#include <QNetworkReply>
#include <QObject>
#include <QProgressDialog>

namespace Utils {
class Archive;
class FilePath;
}

namespace Android::Internal {

class AndroidSdkDownloader : public QObject
{
    Q_OBJECT

public:
    AndroidSdkDownloader();
    ~AndroidSdkDownloader();
    void downloadAndExtractSdk();
    static QString dialogTitle();

    void cancel();

signals:
    void sdkPackageWriteFinished();
    void sdkExtracted();
    void sdkDownloaderError(const QString &error);

private:
    void cancelWithError(const QString &error);
    void logError(const QString &error);

    void downloadFinished();
#if QT_CONFIG(ssl)
    void sslErrors(const QList<QSslError> &errors);
#endif

    QNetworkReply *m_reply = nullptr;
    Utils::FilePath m_sdkFilename;
    QProgressDialog *m_progressDialog = nullptr;
    AndroidConfig &m_androidConfig;
    std::unique_ptr<Utils::Archive> m_archive;
};

} // namespace Android::Internal
