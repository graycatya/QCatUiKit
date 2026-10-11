// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit
import QtQuick.Controls.Basic as B

ListView {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property string currentUser
    property bool followTail: true
    implicitWidth: 360
    implicitHeight: 300
    clip: true
    spacing: tokens.spacing
    boundsBehavior: Flickable.StopAtBounds
    onCountChanged: if (followTail)
                        Qt.callLater(positionViewAtEnd)
    delegate: Item {
        id: entry
        required property string sender
        required property string username
        required property var content
        required property int type
        required property int status
        required property int index
        width: ListView.view.width
        height: bubble.implicitHeight
        CatPane {
            id: bubble
            theme: control.theme
            width: Math.min(entry.width * 0.85, 480)
            x: entry.sender === control.currentUser ? entry.width - width : 0
            contentItem: Column {
                spacing: control.tokens.spacing
                CatLabel {
                    theme: control.theme
                    text: entry.username || entry.sender
                    color: control.tokens.textSecondary
                }
                CatLabel {
                    theme: control.theme
                    width: parent.width
                    text: entry.type === ChatModel.Text ? String(entry.content) : qsTr(
                                                              "Attachment: %1").arg(String(
                                                                                        entry.content))
                    wrapMode: Text.Wrap
                }
                CatLabel {
                    theme: control.theme
                    text: entry.status === ChatModel.Failed ? qsTr("Failed") : entry.status
                                                              === ChatModel.Pending ? qsTr(
                                                                                          "Pending") :
                                                                                      qsTr("Sent")
                    color: control.tokens.textSecondary
                }
            }
        }
    }
    B.ScrollBar.vertical: B.ScrollBar {}
}
