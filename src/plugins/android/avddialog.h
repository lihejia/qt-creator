/****************************************************************************
**
** Copyright (C) 2016 BogDan Vatra <bog_dan_ro@yahoo.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#pragma once
#include "androidconfigurations.h"
#include "androidconfigurations.h"

#include <QDialog>
#include <QTimer>

QT_BEGIN_NAMESPACE
class QComboBox;
class QCheckBox;
class QDialogButtonBox;
class QLineEdit;
class QSpinBox;
QT_END_NAMESPACE

namespace Utils { class InfoLabel; }

namespace Android {
class AndroidConfig;
class SdkPlatform;

namespace Internal {
class AndroidSdkManager;
class AvdDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AvdDialog(const AndroidConfig &config, QWidget *parent = nullptr);
    int exec() override;

    enum DeviceType { Phone, Tablet, Automotive, TV, Wear, PhoneOrTablet };

    ProjectExplorer::IDevice::Ptr device() const;

    const SystemImage *systemImage() const;
    QString name() const;
    QString abi() const;
    QString deviceDefinition() const;
    int sdcardSize() const;
    bool isValid() const;

private:
    void parseDeviceDefinitionsList();
    void updateDeviceDefinitionComboBox();
    void updateApiLevelComboBox();
    bool eventFilter(QObject *obj, QEvent *event) override;

    static AvdDialog::DeviceType tagToDeviceType(const QString &type_tag);

    struct DeviceDefinitionStruct
    {
        QString name_id;
        QString type_str;
        DeviceType deviceType;
    };

    CreateAvdInfo m_createdAvdInfo;
    QTimer m_hideTipTimer;
    QRegularExpression m_allowedNameChars;
    QList<DeviceDefinitionStruct> m_deviceDefinitionsList;
    const AndroidConfig &m_androidConfig;
    AndroidSdkManager m_sdkManager;
    QMap<AvdDialog::DeviceType, QString> m_deviceTypeToStringMap;

    QComboBox *m_abiComboBox;
    QSpinBox *m_sdcardSizeSpinBox;
    QLineEdit *m_nameLineEdit;
    QComboBox *m_targetApiComboBox;
    QComboBox *m_deviceDefinitionComboBox;
    Utils::InfoLabel *m_warningText;
    QComboBox *m_deviceDefinitionTypeComboBox;
    QCheckBox *m_overwriteCheckBox;
    QDialogButtonBox *m_buttonBox;
};

} // Internal
} // Android
