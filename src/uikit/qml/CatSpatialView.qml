// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QCatUiKit
import QtQuick
import "SpatialTransform.js" as SpatialTransform

Item {
    id: control
    readonly property bool _qcatSpatialView: true
    property ThemePalette theme: Theme
    property bool spatialEnabled: true
    property bool pointerTrackingEnabled: true
    property real cameraDistance: 1000
    property real zoom: 1
    property point maximumTilt: Qt.point(6, 6)
    property bool forceFlat: false
    readonly property bool flatMode: forceFlat || !spatialEnabled || (theme || Theme).reducedMotion
                                     || GraphicsInfo.api === GraphicsInfo.Software
    readonly property string rendererName: GraphicsInfo.api === GraphicsInfo.Software
                                           ? "Qt Quick software" : "Qt Quick scene graph"
    default property alias surfaces: scene.data
    implicitWidth: 480
    implicitHeight: 300
    clip: true
    Item {
        id: scene
        anchors.fill: parent
        scale: control.flatMode ? 1 : control.zoom
        property real tiltX: control.flatMode || !hover.hovered || !control.pointerTrackingEnabled
                             ? 0 : (hover.point.position.y / Math.max(1, scene.height) - 0.5)
                               * control.maximumTilt.y * 2
        property real tiltY: control.flatMode || !hover.hovered || !control.pointerTrackingEnabled
                             ? 0 : (hover.point.position.x / Math.max(1, scene.width) - 0.5)
                               * control.maximumTilt.x * 2
        Behavior on tiltX {
            NumberAnimation {
                duration: control.flatMode ? 0 : (control.theme || Theme).tokens.motionDuration
            }
        }
        Behavior on tiltY {
            NumberAnimation {
                duration: control.flatMode ? 0 : (control.theme || Theme).tokens.motionDuration
            }
        }
        transform: Matrix4x4 {
            matrix: SpatialTransform.matrix(scene.width, scene.height, Qt.vector3d(scene.tiltX,
                                                                                   scene.tiltY, 0),
                                            control.cameraDistance)
        }
    }
    HoverHandler {
        id: hover
    }
    Keys.onPressed: event => {
        if (event.key === Qt.Key_Tab || event.key === Qt.Key_Escape)
            forceFlat = true;
        event.accepted = false;
    }
}
