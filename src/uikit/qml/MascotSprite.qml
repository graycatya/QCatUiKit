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
    Item {
        visible: root.frame.renderer === "sprite"
        z: root.controller.status === MascotPetController.Loading ? 1 : 0
        width: root.frame.frameSize ? root.frame.frameSize.width : 1
        height: root.frame.frameSize ? root.frame.frameSize.height : 1
        transform: [
            Scale {
                origin.x: root.frame.mirrored ? (root.frame.frameSize ? root.frame.frameSize.width / 2 : 0) : 0
                xScale: root.frame.mirrored ? -1 : 1
            },
            Scale {
                xScale: root.width / (root.frame.frameSize ? root.frame.frameSize.width : 1)
                yScale: root.height / (root.frame.frameSize ? root.frame.frameSize.height : 1)
            }
        ]
        Image {
            x: -(root.frame.x || 0)
            y: -(root.frame.y || 0)
            width: root.frame.atlasSize ? root.frame.atlasSize.width : 0
            height: root.frame.atlasSize ? root.frame.atlasSize.height : 0
            source: root.frame.renderer === "sprite" ? root.frame.source : ""
            smooth: root.smooth
            asynchronous: false
            cache: true
            onStatusChanged: {
                if (status === Image.Error)
                    adapter.spriteError(source, qsTr("Cannot load sprite atlas: %1").arg(source))
            }
            visible: root.controller.status === MascotPetController.Ready
                     || root.controller.status === MascotPetController.Loading
        }
    }
    CatLottieAnimation {
        id: lottie
        anchors.fill: parent
        theme: root.theme
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
