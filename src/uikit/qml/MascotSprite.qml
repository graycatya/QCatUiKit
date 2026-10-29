// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit

Item {
    id: root
    required property MascotPetController controller
    property ThemePalette theme: Theme
    smooth: false
    property var retainedFrame: ({})
    property size retainedSize: Qt.size(0, 0)
    readonly property var targetFrame: controller.spriteFrame
    readonly property var frame: controller.status === MascotPetController.Loading && retainedFrame.renderer
                                 ? retainedFrame : targetFrame
    readonly property size displaySize: controller.status === MascotPetController.Loading && retainedFrame.renderer
                                       ? retainedSize : controller.logicalSize
    function rememberFrame() {
        if (controller.status === MascotPetController.Ready) {
            retainedFrame = controller.spriteFrame
            retainedSize = controller.logicalSize
        } else if (controller.status !== MascotPetController.Loading) {
            retainedFrame = ({})
            retainedSize = Qt.size(0, 0)
        }
    }
    Component.onCompleted: rememberFrame()
    Connections {
        target: root.controller
        function onStatusChanged() { root.rememberFrame() }
        function onSpriteFrameChanged() { root.rememberFrame() }
    }
    implicitWidth: displaySize.width * controller.petScale
    implicitHeight: displaySize.height * controller.petScale
    clip: true
    Binding {
        target: root.controller
        property: "playbackEnabled"
        value: root.visible && (!root.Window.window || root.Window.window.visible)
               && !(root.theme || Theme).reducedMotion
    }
    MascotRenderAdapter {
        id: adapter
        controller: root.controller
        vectorItem: lottie
    }
    AnimatedSprite {
        id: sprite
        anchors.fill: parent
        visible: root.frame.renderer === "sprite"
        z: root.controller.status === MascotPetController.Loading ? 1 : 0
        source: root.frame.renderer === "sprite" ? root.frame.source : ""
        frameWidth: root.frame.frameSize ? root.frame.frameSize.width : 1
        frameHeight: root.frame.frameSize ? root.frame.frameSize.height : 1
        frameCount: root.frame.atlasSize && root.frame.frameSize
                    ? Math.max(1, Math.floor(root.frame.atlasSize.width / frameWidth)
                                  * Math.floor(root.frame.atlasSize.height / frameHeight)) : 1
        // Only the controller advances time. The renderer samples one atlas
        // frame directly instead of moving a full-size image behind a clip.
        paused: true
        interpolate: false
        currentFrame: root.frame.renderer === "sprite"
                      ? Math.floor((root.frame.y || 0) / frameHeight)
                        * Math.floor(root.frame.atlasSize.width / frameWidth)
                        + Math.floor((root.frame.x || 0) / frameWidth) : 0
        smooth: root.smooth
        transform: Scale {
            origin.x: root.width / 2
            xScale: root.frame.mirrored ? -1 : 1
        }
    }
    // Preserve explicit image errors while the sprite renderer shares the
    // decoded source through Qt Quick's pixmap cache.
    Image {
        visible: false
        source: root.frame.renderer === "sprite" ? root.frame.source : ""
        cache: true
        onStatusChanged: {
            if (status === Image.Error)
                adapter.spriteError(source, qsTr("Cannot load sprite atlas: %1").arg(source))
        }
    }
    VectorItem {
        id: lottie
        anchors.fill: parent
        format: VectorItem.Lottie
        autoPlay: false
        animationEnabled: root.controller.playbackEnabled && !root.controller.paused
        fillMode: VectorItem.Stretch
        visible: root.frame.renderer === "lottie" || root.targetFrame.renderer === "lottie"
        transform: Scale {
            origin.x: root.width / 2
            xScale: root.frame.mirrored ? -1 : 1
        }
    }
    Image {
        id: fallback
        anchors.fill: parent
        source: root.controller.fallbackSource
        fillMode: Image.PreserveAspectFit
        visible: root.controller.status === MascotPetController.Error
                 && status === Image.Ready
    }
    Rectangle {
        anchors.fill: parent
        visible: root.controller.status === MascotPetController.Error && fallback.status !== Image.Ready
        color: (root.theme || Theme).tokens.background
        CatLabel {
            anchors.fill: parent
            anchors.margins: 8
            theme: root.theme
            text: qsTr("Character unavailable")
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
    TapHandler {
        onTapped: (eventPoint, button) => root.controller.activateRegion(eventPoint.position)
    }
}
