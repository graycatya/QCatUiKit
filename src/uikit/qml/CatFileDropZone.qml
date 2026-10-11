// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick.Layouts
import QtQuick
import QCatUiKit

CatPane {
    id: control
    property string title: qsTr("Drop files here")
    property string description
    property string hintText
    property string browseText: qsTr("Browse")
    property string errorMessage
    property var acceptedExtensions: []
    readonly property alias dragActive: drop.containsDrag
    signal filesDropped(var urls)
    signal browseRequested
    implicitWidth: 360
    function acceptsUrls(urls: var): bool {
        if (!urls || !urls.length)
            return false;
        return urls.every(url => {
            const text = url.toString();
            return text.startsWith("file:") && (!acceptedExtensions.length
                                                || acceptedExtensions.some(extension
                                                                           => text.toLowerCase(
                                                                                  ).endsWith(
                                                                                  extension.toLowerCase(
                                                                                      ))));
        });
    }
    background: SurfaceImpl {
        theme: control.theme
        hovered: control.dragActive
        focused: control.dragActive
        invalid: control.errorMessage.length > 0
    }
    contentItem: ColumnLayout {
        spacing: 12
        CatLabel {
            theme: control.theme
            text: control.title
            font.bold: true
            Layout.fillWidth: true
            wrapMode: Text.Wrap
        }
        CatLabel {
            theme: control.theme
            text: control.description
            visible: text.length > 0
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            color: control.tokens.textSecondary
        }
        CatButton {
            theme: control.theme
            text: control.browseText
            onClicked: control.browseRequested()
        }
        CatLabel {
            theme: control.theme
            text: control.errorMessage || control.hintText
            visible: text.length > 0
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            color: control.errorMessage.length ? control.tokens.danger :
                                                 control.tokens.textSecondary
        }
    }
    DropArea {
        id: drop
        parent: control
        anchors.fill: parent
        onEntered: drag => {
            drag.accepted = control.enabled && control.acceptsUrls(drag.urls);
        }
        onDropped: event => {
            if (control.enabled && control.acceptsUrls(event.urls)) {
                control.filesDropped(event.urls);
                event.acceptProposedAction();
            }
        }
    }
}
