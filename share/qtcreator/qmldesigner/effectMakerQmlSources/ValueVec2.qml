// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtQuickDesignerTheme
import StudioControls as StudioControls
import StudioTheme 1.0 as StudioTheme
import EffectMakerBackend

Column {
    width: parent.width
    spacing: 1

    Row {
        width: parent.width
        spacing: 5

        StudioControls.RealSpinBox {
            id: vX

            width: 40
            actionIndicatorVisible: false
            spinBoxIndicatorVisible: false
            inputHAlignment: Qt.AlignHCenter
            realFrom: uniformMinValue.x
            realTo: uniformMaxValue.x
            realValue: uniformValue.x
            realStepSize: .01
            decimals: 2
            onRealValueModified: uniformValue.x = realValue
        }

        StudioControls.Slider {
            id: sliderX

            width: parent.width - 80
            labels: false
            decimals: 2
            actionIndicatorVisible: false
            from: uniformMinValue.x
            to: uniformMaxValue.x
            value: uniformValue.x
            onMoved: {
                uniformValue.x = value
                vX.realValue = value // binding isn't working for this property so update it
            }
        }
    }

    Row {
        width: parent.width
        spacing: 5

        StudioControls.RealSpinBox {
            id: vY

            width: 40
            actionIndicatorVisible: false
            spinBoxIndicatorVisible: false
            inputHAlignment: Qt.AlignHCenter
            realFrom: uniformMinValue.y
            realTo: uniformMaxValue.y
            realValue: uniformValue.y
            realStepSize: .01
            decimals: 2
            onRealValueModified: uniformValue.y = realValue
        }

        StudioControls.Slider {
            id: sliderY

            width: parent.width - 80
            labels: false
            decimals: 2
            actionIndicatorVisible: false
            from: uniformMinValue.y
            to: uniformMaxValue.y
            value: uniformValue.y
            onMoved: {
                uniformValue.y = value
                vY.realValue = value // binding isn't working for this property so update it
            }
        }
    }
}