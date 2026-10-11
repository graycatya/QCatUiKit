// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QCatUiKit
import QtQuick

Rectangle {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property string name
    property string initials: name.trim().split(/\s+/).filter(part => part.length).slice(0, 2).map(part
                                                                                                   => Array.from(
                                                                                                          part)[0]).join(
                                  "").toLocaleUpperCase()
    property url source
    property string presence
    implicitWidth: 48
    implicitHeight: 48
    radius: Math.min(width, height) / 2
    color: tokens.surfaceHover
    border.color: tokens.border
    antialiasing: true
    Accessible.role: Accessible.Graphic
    Accessible.name: name
    CatLabel {
        theme: control.theme
        anchors.fill: parent
        text: control.initials
        color: control.tokens.primary
        font.pixelSize: Math.max(12, control.height * 0.35)
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        visible: !portrait.isImageLoaded(control.source)
    }
    Image {
        id: dimensions
        visible: false
        source: control.source
        asynchronous: true
        onStatusChanged: if (status === Image.Ready)
                             portrait.requestPaint()
    }
    Canvas {
        id: portrait
        anchors.fill: parent
        property url loadedSource
        function loadSource(): void {
            if (loadedSource.toString().length)
                unloadImage(loadedSource);
            loadedSource = control.source;
            if (loadedSource.toString().length)
                loadImage(loadedSource);
            requestPaint();
        }
        Component.onCompleted: loadSource()
        onImageLoaded: requestPaint()
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        onPaint: {
            const context = getContext("2d");
            context.reset();
            context.clearRect(0, 0, width, height);
            if (!isImageLoaded(control.source) || dimensions.sourceSize.width <= 0
                    || dimensions.sourceSize.height <= 0)
                return;
            const scale = Math.max(width / dimensions.sourceSize.width, height
                                   / dimensions.sourceSize.height);

            const imageWidth = dimensions.sourceSize.width * scale;
            const imageHeight = dimensions.sourceSize.height * scale;
            context.beginPath();
            context.arc(width / 2, height / 2, Math.min(width, height) / 2, 0, 2 * Math.PI);
            context.clip();
            context.drawImage(control.source, (width - imageWidth) / 2, (height - imageHeight) / 2, imageWidth,
                              imageHeight);
        }
    }
    onSourceChanged: portrait.loadSource()
    Rectangle {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        width: Math.max(10, control.width / 4)
        height: width
        radius: width / 2
        border.width: 2
        border.color: control.tokens.surface
        color: control.presence === "online" ? control.tokens.success : control.presence === "busy"
                                               ? control.tokens.danger :
                                                 control.tokens.textSecondary
        visible: control.presence.length > 0
        antialiasing: true
    }
}
