// SPDX-License-Identifier: Apache-2.0
.pragma library

// Compose rotation and perspective explicitly so the camera distance also works
// on Qt 6.8, before Rotation acquired its distanceToPlane property.
function matrix(width, height, angles, distance) {
    const radians = Math.PI / 180;
    const sx = Math.sin(angles.x * radians), cx = Math.cos(angles.x * radians);
    const sy = Math.sin(angles.y * radians), cy = Math.cos(angles.y * radians);
    const sz = Math.sin(angles.z * radians), cz = Math.cos(angles.z * radians);
    const camera = Math.max(1, distance);
    const rotation = Qt.matrix4x4(
        cz * cy, cz * sy * sx - sz * cx, cz * sy * cx + sz * sx, 0,
        sz * cy, sz * sy * sx + cz * cx, sz * sy * cx - cz * sx, 0,
        -sy, cy * sx, cy * cx, 0,
        0, 0, 0, 1);
    const perspective = Qt.matrix4x4(1, 0, 0, 0, 0, 1, 0, 0,
                                   0, 0, 1, 0, 0, 0, -1 / camera, 1);
    const center = Qt.matrix4x4(1, 0, 0, width / 2, 0, 1, 0, height / 2,
                              0, 0, 1, 0, 0, 0, 0, 1);
    const uncenter = Qt.matrix4x4(1, 0, 0, -width / 2, 0, 1, 0, -height / 2,
                                0, 0, 1, 0, 0, 0, 0, 1);
    return center.times(perspective).times(rotation).times(uncenter);
}
