// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QCatUiKit

CatTeachingTip {
    id: control
    dim: true
    modal: true
    closePolicy: T.Popup.CloseOnEscape | T.Popup.CloseOnPressOutside
    T.Overlay.modal: Rectangle {
        color: Qt.rgba(0, 0, 0, 0.2)
    }
}
