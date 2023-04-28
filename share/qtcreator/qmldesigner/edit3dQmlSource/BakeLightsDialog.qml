// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0 WITH Qt-GPL-exception-1.0

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuickDesignerTheme
import HelperWidgets
import StudioControls as StudioControls
import StudioTheme as StudioTheme

Rectangle {
    id: root

    color: StudioTheme.Values.themePanelBackground

    Column {
        id: col
        padding: 5
        leftPadding: 10
        spacing: 5

        Text {
            id: title
            text: qsTr("Baking lights for 3D view: %1").arg(sceneId)
            font.bold: true
            font.pixelSize: StudioTheme.Values.myFontSize
            color: StudioTheme.Values.themeTextColor
        }

        Rectangle {
            width: root.width - 16
            height: root.height - title.height - button.height - 20

            color: StudioTheme.Values.themePanelBackground
            border.color: StudioTheme.Values.themeControlOutline
            border.width: StudioTheme.Values.border

            ScrollView {
                id: scrollView

                anchors.fill: parent
                anchors.margins: 4

                clip: true

                Behavior on contentY {
                    PropertyAnimation {
                        easing.type: Easing.InOutQuad
                    }
                }

                 Text {
                     id: progressText
                     width: scrollView.width
                     font.pixelSize: StudioTheme.Values.myFontSize
                     color: StudioTheme.Values.themeTextColor
                 }

                 function ensureVisible()
                 {
                     let newPos = scrollView.contentHeight - scrollView.height
                     scrollView.contentY = newPos < 0 ? 0 : newPos
                 }
            }

        }

        Connections {
            target: rootView
            function onProgress(msg) {
                progressText.text += progressText.text === "" ? msg : "\n" + msg
                scrollView.ensureVisible()
            }

            function onFinished() {
                button.text = qsTr("Close")
            }
        }

        Button {
            id: button
            text: qsTr("Cancel")
            anchors.right: parent.right
            anchors.margins: StudioTheme.Values.dialogButtonPadding

            onClicked: rootView.cancel()
        }
    }
}