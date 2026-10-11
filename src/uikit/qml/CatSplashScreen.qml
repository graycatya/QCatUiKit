// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QCatUiKit

CatPopup {
    id: control
    parent: T.Overlay.overlay
    property string title
    property string subtitle
    property url iconSource
    property real progress: 0
    property bool indeterminate: true
    width: parent ? parent.width : 480
    height: parent ? parent.height : 320
    padding: 32
    closePolicy: T.Popup.NoAutoClose
    contentItem: Item {
        Column {
            anchors.centerIn: parent
            width: Math.min(360, parent.width)
            spacing: 16
            Image {
                source: control.iconSource
                width: 64
                height: 64
                anchors.horizontalCenter: parent.horizontalCenter
                fillMode: Image.PreserveAspectFit
                visible: source.toString().length > 0
            }
            CatLabel {
                theme: control.theme
                width: parent.width
                text: control.title
                font.pixelSize: 24
                font.bold: true
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
            }
            CatLabel {
                theme: control.theme
                width: parent.width
                text: control.subtitle
                wrapMode: Text.Wrap
                color: control.tokens.textSecondary
                horizontalAlignment: Text.AlignHCenter
            }
            CatProgressBar {
                theme: control.theme
                width: parent.width
                value: control.progress
                indeterminate: control.indeterminate
            }
        }
    }
    background: Rectangle {
        color: control.tokens.background
    }
}
