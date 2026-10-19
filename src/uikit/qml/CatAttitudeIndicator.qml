// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Shapes
import QCatUiKit

Item {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property real pitchDegrees: 0
    property real rollDegrees: 0
    property bool attitudeValid: false
    readonly property bool displayValid: attitudeValid && Number.isFinite(pitchDegrees)
        && Math.abs(pitchDegrees) <= 90 && Number.isFinite(rollDegrees)
        && Number.isFinite(visiblePitchSpanDegrees) && visiblePitchSpanDegrees >= 10
        && visiblePitchSpanDegrees <= 180
    property real visiblePitchSpanDegrees: 60
    property bool pitchLadderVisible: true
    property bool rollScaleVisible: true
    property bool readoutVisible: true
    property color skyColor: "#2775a5"
    property color groundColor: "#80543b"
    property color aircraftColor: "#ffe278"
    property color scaleColor: "#ffffff"
    implicitWidth: 280
    implicitHeight: 280
    Accessible.role: Accessible.Indicator
    Accessible.name: qsTr("Aircraft attitude")
    Accessible.description: displayValid
        ? qsTr("Pitch %1°, roll %2°").arg(pitchDegrees.toFixed(1)).arg(rollDegrees.toFixed(1))
        : qsTr("Attitude unavailable")

    Item {
        id: dial
        anchors.centerIn: parent
        width: Math.max(0, Math.min(control.width, control.height))
        height: width
        readonly property real radius: width / 2
        readonly property real textSize: Math.max(9, Math.min(control.tokens.fontSize, width / 20))
        // Every moving primitive is bounded analytically by the circular aperture.
        // No offscreen texture, ShaderEffect, or rectangular clip masquerading as a circle.
        Item {
            id: moving
            anchors.fill: parent
            visible: control.displayValid
            rotation: control.displayValid ? -(((control.rollDegrees + 180) % 360 + 360) % 360 - 180) : 0
            readonly property real pixelsPerDegree: height / (control.displayValid ? control.visiblePitchSpanDegrees : 60)
            readonly property real horizonY: dial.radius + (control.displayValid ? control.pitchDegrees : 0) * pixelsPerDegree
            readonly property real halfChord: Math.sqrt(Math.max(0, dial.radius * dial.radius
                - Math.pow(horizonY - dial.radius, 2)))
            Rectangle { anchors.fill: parent; radius: width / 2; color: control.skyColor; antialiasing: true }
            Rectangle {
                anchors.fill: parent; radius: width / 2
                color: control.groundColor; antialiasing: true
                visible: moving.horizonY <= 0
            }
            Shape {
                anchors.fill: parent
                visible: moving.horizonY > 0 && moving.horizonY < height
                preferredRendererType: Shape.CurveRenderer
                ShapePath {
                    strokeWidth: -1
                    fillColor: control.groundColor
                    startX: dial.radius - moving.halfChord
                    startY: Math.max(0, Math.min(dial.height, moving.horizonY))
                    PathLine { x: dial.radius + moving.halfChord; y: Math.max(0, Math.min(dial.height, moving.horizonY)) }
                    PathArc {
                        x: dial.radius - moving.halfChord
                        y: Math.max(0, Math.min(dial.height, moving.horizonY))
                        radiusX: dial.radius; radiusY: dial.radius
                        direction: PathArc.Clockwise
                        useLargeArc: moving.horizonY < dial.radius
                    }
                }
            }
            Rectangle {
                x: dial.radius - moving.halfChord + 1
                y: moving.horizonY - height / 2
                width: Math.max(0, moving.halfChord * 2 - 2); height: 2
                color: control.scaleColor
                visible: moving.horizonY > 1 && moving.horizonY < dial.height - 1
            }
            Repeater {
                model: 37
                delegate: Item {
                    id: tick
                    required property int index
                    readonly property int angle: (index - 18) * 5
                    readonly property bool major: angle % 10 === 0
                    readonly property real offset: ((control.displayValid ? control.pitchDegrees : 0) - angle) * moving.pixelsPerDegree
                    readonly property real halfWidth: dial.width * (major ? .14 : .08)
                    readonly property real labelExtent: halfWidth + dial.textSize * 2.8
                    x: dial.radius
                    y: dial.radius + offset
                    visible: control.pitchLadderVisible && angle !== 0
                        && Math.pow(Math.abs(offset) + dial.textSize, 2) + labelExtent * labelExtent < Math.pow(dial.radius - 6, 2)
                    Rectangle {
                        x: -tick.halfWidth; y: -height / 2
                        width: tick.halfWidth * 2; height: tick.major ? 2 : 1
                        color: control.scaleColor
                    }
                    Text {
                        x: -tick.halfWidth - width - 6; y: -height / 2
                        text: tick.angle; visible: tick.major
                        color: control.scaleColor; font.pixelSize: dial.textSize
                    }
                    Text {
                        x: tick.halfWidth + 6; y: -height / 2
                        text: tick.angle; visible: tick.major
                        color: control.scaleColor; font.pixelSize: dial.textSize
                    }
                }
            }
        }
        Repeater {
            model: [-60, -45, -30, -20, -10, 0, 10, 20, 30, 45, 60]
            delegate: Rectangle {
                required property int modelData
                readonly property real a: modelData * Math.PI / 180
                readonly property real r: dial.radius - height / 2 - 7
                x: dial.radius + Math.sin(a) * r - width / 2
                y: dial.radius - Math.cos(a) * r - height / 2
                width: 2; height: Math.abs(modelData) % 30 === 0 ? dial.width * .045 : dial.width * .025
                rotation: modelData
                color: control.scaleColor
                visible: control.rollScaleVisible && control.displayValid
                antialiasing: true
            }
        }
        Item {
            anchors.fill: parent
            rotation: moving.rotation
            visible: control.rollScaleVisible && control.displayValid
            Shape {
                anchors.fill: parent
                preferredRendererType: Shape.CurveRenderer
                ShapePath {
                    fillColor: control.aircraftColor; strokeWidth: -1
                    startX: dial.radius; startY: dial.width * .09
                    PathLine { x: dial.radius - dial.width * .022; y: dial.width * .13 }
                    PathLine { x: dial.radius + dial.width * .022; y: dial.width * .13 }
                    PathLine { x: dial.radius; y: dial.width * .09 }
                }
            }
        }
        Shape {
            anchors.fill: parent
            visible: control.displayValid
            preferredRendererType: Shape.CurveRenderer
            ShapePath {
                fillColor: "transparent"; strokeColor: control.aircraftColor
                strokeWidth: Math.max(2, dial.width * .012)
                capStyle: ShapePath.RoundCap; joinStyle: ShapePath.RoundJoin
                startX: dial.width * .27; startY: dial.radius
                PathLine { x: dial.width * .42; y: dial.radius }
                PathLine { x: dial.width * .46; y: dial.width * .54 }
                PathLine { x: dial.radius; y: dial.radius }
                PathLine { x: dial.width * .54; y: dial.width * .54 }
                PathLine { x: dial.width * .58; y: dial.radius }
                PathLine { x: dial.width * .73; y: dial.radius }
            }
        }
        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            y: dial.height * .76
            visible: control.readoutVisible && control.displayValid
            width: Math.min(dial.width * .72, readout.implicitWidth + 12)
            height: readout.implicitHeight + 6
            radius: 4; color: "#99000000"
            Text {
                id: readout
                anchors.centerIn: parent
                width: Math.max(0, parent.width - 12)
                text: qsTr("P %1°  R %2°").arg(control.pitchDegrees.toFixed(1)).arg(control.rollDegrees.toFixed(1))
                font.pixelSize: dial.textSize; color: control.scaleColor
                horizontalAlignment: Text.AlignHCenter; elide: Text.ElideRight
            }
        }
        Rectangle {
            anchors.fill: parent; radius: width / 2
            color: control.tokens.surface
            visible: !control.displayValid; antialiasing: true
            Text {
                anchors.centerIn: parent; width: parent.width * .7
                text: qsTr("Attitude unavailable")
                color: control.tokens.textSecondary; font.pixelSize: control.tokens.fontSize
                horizontalAlignment: Text.AlignHCenter; wrapMode: Text.Wrap
            }
        }
        Rectangle {
            anchors.fill: parent; radius: width / 2; color: "transparent"
            border.width: 2; border.color: control.tokens.border; antialiasing: true
        }
    }
}
