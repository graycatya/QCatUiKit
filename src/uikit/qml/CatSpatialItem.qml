// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QCatUiKit
import QtQuick
import "SpatialTransform.js" as SpatialTransform

Item {
    id: control
    property ThemePalette theme: Theme
    property vector3d position: Qt.vector3d(0, 0, 0)
    property vector3d rotation3d: Qt.vector3d(0, 0, 0)
    property real cameraDistance: 1000
    property bool spatialEnabled: true
    readonly property var spatialHost: findSpatialHost(parent)
    readonly property real effectiveCameraDistance: Math.max(1, spatialHost
                                                             ? spatialHost.cameraDistance :
                                                               cameraDistance)
    readonly property bool flatMode: !spatialEnabled || (spatialHost && spatialHost.flatMode) || (
                                         theme || Theme).reducedMotion || GraphicsInfo.api
                                     === GraphicsInfo.Software
    function findSpatialHost(item) {
        while (item) {
            if (item._qcatSpatialView)
                return item;
            item = item.parent;
        }
        return null;
    }
    x: position.x
    y: position.y
    z: position.z
    scale: flatMode ? 1 : effectiveCameraDistance / Math.max(1, effectiveCameraDistance
                                                             - position.z)

    implicitWidth: 240
    implicitHeight: 160
    transform: Matrix4x4 {
        matrix: SpatialTransform.matrix(control.width, control.height, control.flatMode
                                        ? Qt.vector3d(0, 0, 0) : control.rotation3d,
                                        control.effectiveCameraDistance)
    }
}
