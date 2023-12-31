// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick 2.1
import QtQuick.Window 2.1
import QtQuick.Controls 1.1
import QtQuick.Controls.Private 1.0

/*!
    \qmltype MenuStyle
    \internal
    \ingroup menusstyling
    \inqmlmodule QtQuick.Controls.Styles
*/

Style {
    id: styleRoot

    property string __menuItemType: "menuitem"
    property real maxPopupHeight: 600 // ### FIXME Screen.desktopAvailableHeight * 0.99

    property Component frame: Rectangle {
        width: (parent ? parent.contentWidth : 0) + 2
        height: (parent ? parent.contentHeight : 0) + 2

        color: "lightgray"
        border { width: 1; color: "darkgray" }

        property int subMenuOverlap: -1
        property real maxHeight: maxPopupHeight
        property int margin: 1
    }

    property Component menuItem: Rectangle {
        x: 1
        y: 1
        implicitWidth: Math.max((parent ? parent.width : 0),
                                18 + text.paintedWidth + (rightDecoration.visible ? rightDecoration.width + 40 : 12))
        implicitHeight: isSeparator ? text.font.pixelSize / 2 : !!scrollerDirection ? text.font.pixelSize * 0.75 : text.paintedHeight + 4
        color: selected && enabled ? "" : backgroundColor
        gradient: selected && enabled ? selectedGradient : undefined
        border.width: 1
        border.color: selected && enabled ? Qt.darker(selectedColor, 1) : color
        readonly property int leftMargin: __menuItemType === "menuitem" ? 18 : 0

        readonly property color backgroundColor: "#dcdcdc"
        readonly property color selectedColor: "#49d"
        Gradient {
            id: selectedGradient
            GradientStop {color: Qt.lighter(selectedColor, 1.3)  ; position: -0.2}
            GradientStop {color: selectedColor; position: 1.4}
        }
        antialiasing: true

        SystemPalette {
            id: syspal
            colorGroup: enabled ? SystemPalette.Active : SystemPalette.Disabled
        }

        readonly property string itemText: parent ? parent.text : ""
        readonly property bool mirrored: Qt.application.layoutDirection === Qt.RightToLeft

        Loader {
            id: checkMark
            x: mirrored ? parent.width - width - 4 : 4
            y: 6
            active: __menuItemType === "menuitem" && !!menuItem && !!menuItem["checkable"]
            sourceComponent: exclusive ? exclusiveCheckMark : nonExclusiveCheckMark

            readonly property bool checked: !!menuItem && !!menuItem.checked
            readonly property bool exclusive: !!menuItem && !!menuItem["exclusiveGroup"]

            Component {
                id: nonExclusiveCheckMark
                BorderImage {
                    width: 12
                    height: 12
                    source: "images/editbox.png"
                    border.top: 6
                    border.bottom: 6
                    border.left: 6
                    border.right: 6

                    Rectangle {
                        antialiasing: true
                        visible: checkMark.checked
                        color: "#666"
                        radius: 1
                        anchors.margins: 4
                        anchors.fill: parent
                        anchors.topMargin: 3
                        anchors.bottomMargin: 5
                        border.color: "#222"
                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: 1
                            color: "transparent"
                            border.color: "#33ffffff"
                        }
                    }
                }
            }

            Component {
                id: exclusiveCheckMark
                Rectangle {
                    x: 1
                    width: 10
                    height: 10
                    color: "white"
                    border.color: "gray"
                    antialiasing: true
                    radius: height/2

                    Rectangle {
                        anchors.centerIn: parent
                        visible: checkMark.checked
                        width: 4
                        height: 4
                        color: "#666"
                        border.color: "#222"
                        antialiasing: true
                        radius: height/2
                    }
                }
            }
        }

        Text {
            id: text
            visible: !isSeparator
            text: StyleHelpers.stylizeMnemonics(itemText)
            readonly property real offset: __menuItemType === "menuitem" ? 24 : 6
            x: mirrored ? parent.width - width - offset : offset
            anchors.verticalCenter: parent.verticalCenter
            renderType: Text.NativeRendering
            color: selected && enabled ? "white" : syspal.text
        }

        Text {
            id: rightDecoration
            readonly property string shortcut: !!menuItem && menuItem["shortcut"] || ""
            visible: isSubmenu || shortcut !== ""
            text: isSubmenu ? mirrored ? "\u25c2" : "\u25b8" // BLACK LEFT/RIGHT-POINTING SMALL TRIANGLE
                            : shortcut
            LayoutMirroring.enabled: mirrored
            anchors {
                right: parent.right
                rightMargin: 6
                baseline: isSubmenu ? undefined : text.baseline
            }
            font.pixelSize: isSubmenu ? text.font.pixelSize : text.font.pixelSize * 0.9
            color: text.color
            renderType: Text.NativeRendering
            style: selected || !isSubmenu ? Text.Normal : Text.Raised; styleColor: Qt.lighter(color, 4)
        }

        Image {
            id: scrollerDecoration
            visible: !!scrollerDirection
            anchors.centerIn: parent
            source: scrollerDirection === "up" ? "images/arrow-up.png" : "images/arrow-down.png"
        }

        Rectangle {
            visible: isSeparator
            width: parent.width - 2
            height: 1
            x: 1
            anchors.verticalCenter: parent.verticalCenter
            color: "darkgray"
        }
    }

    property Component scrollerStyle: Style {
        padding { left: 0; right: 0; top: 0; bottom: 0 }
        property bool scrollToClickedPosition: false
        property Component frame: Item { visible: false }
        property Component corner: Item { visible: false }
        property Component __scrollbar: Item { visible: false }
        property bool useScrollers: true
    }
}
