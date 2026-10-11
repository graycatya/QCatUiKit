// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick.Layouts
import QtQuick
import QCatUiKit

CatPopup {
    id: control
    property string title
    property string message
    property int severity: 0
    property int duration: 3500
    property string actionText
    property bool pauseOnHoverEnabled: true
    signal actionTriggered
    modal: false
    focus: false
    width: 340
    contentItem: ColumnLayout {
        spacing: 8
        CatLabel {
            theme: control.theme
            text: control.title
            font.bold: true
            visible: text.length > 0
            Layout.fillWidth: true
            wrapMode: Text.Wrap
        }
        CatAlert {
            id: alert
            theme: control.theme
            text: control.message
            severity: control.severity
            closable: true
            Layout.fillWidth: true
            onClosed: control.close()
        }
        CatButton {
            theme: control.theme
            text: control.actionText
            visible: text.length > 0
            flat: true
            onClicked: control.actionTriggered()
        }
    }
    HoverHandler {
        id: hover
        parent: control.contentItem
    }
    Timer {
        interval: Math.max(1, control.duration)
        running: control.opened && control.duration > 0 && !(control.pauseOnHoverEnabled
                                                             && hover.hovered)
        onTriggered: control.close()
    }
    onAboutToShow: alert.visible = true
}
