// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit

ColumnLayout {
    id: control
    objectName: "tiltPreview"
    property real phoneRoll: 0
    property real phonePitch: 0
    property bool pauseInput: false
    property bool allowControl: true
    readonly property TiltController controller: tilt
    spacing: Theme.tokens.spacing
    function submitSimulatedOrientation(): void {
        // Ry(roll) * Rx(-pitch), device -> reference; this is simulated PHONE data.
        const r = phoneRoll * Math.PI / 360, p = -phonePitch * Math.PI / 360;
        simulation.timestampUs += 20000;
        tilt.submitOrientation(Qt.quaternion(Math.cos(r) * Math.cos(p), Math.cos(r) * Math.sin(p),
            Math.sin(r) * Math.cos(p), -Math.sin(r) * Math.sin(p)), simulation.timestampUs);
    }
    TiltController { id: tilt; controlAllowed: control.visible && control.allowControl }
    Timer {
        id: simulation
        property double timestampUs: 0
        interval: 20; repeat: true
        running: control.visible && tilt.enabled && !control.pauseInput && tilt.state !== TiltController.Fault
        onTriggered: control.submitSimulatedOrientation()
    }
    CatLabel {
        Layout.fillWidth: true; wrapMode: Text.Wrap; color: Theme.tokens.textSecondary
        text: qsTr("Phone input simulator. No sensor access or flight-control messages. Enable, calibrate while still, then hold to control.")
    }
    CatLabel { text: qsTr("Phone tilt right") }
    CatSliderField {
        objectName: "phoneRollField"
        Layout.fillWidth: true
        from: -60; to: 60; stepSize: 1; decimals: 0; suffix: "°"
        value: control.phoneRoll; Accessible.name: qsTr("Phone tilt right")
        onValueEdited: value => control.phoneRoll = value
    }
    CatLabel { text: qsTr("Phone tilt forward") }
    CatSliderField {
        objectName: "phonePitchField"
        Layout.fillWidth: true
        from: -60; to: 60; stepSize: 1; decimals: 0; suffix: "°"
        value: control.phonePitch; Accessible.name: qsTr("Phone tilt forward")
        onValueEdited: value => control.phonePitch = value
    }
    Flow {
        Layout.fillWidth: true; spacing: Theme.tokens.spacing
        CatCheckBox { text: qsTr("Pause input (test timeout)"); checked: control.pauseInput; onToggled: control.pauseInput = checked }
        CatCheckBox { text: qsTr("Allow simulated control"); checked: control.allowControl; onToggled: control.allowControl = checked }
        CatButton { text: qsTr("Center phone"); onClicked: { control.phoneRoll = 0; control.phonePitch = 0; } }
    }
    CatTiltControlPanel {
        objectName: "demoTiltControlPanel"
        Layout.fillWidth: true
        controlEnabled: tilt.enabled; controlAllowed: tilt.controlAllowed
        controllerState: tilt.state; sampleFresh: tilt.sampleFresh
        rollInput: tilt.rollInput; pitchInput: tilt.pitchInput
        fullScaleAngleDegrees: tilt.fullScaleAngleDegrees; deadzoneDegrees: tilt.deadzoneDegrees
        filterTimeConstantMs: tilt.filterTimeConstantMs; expo: tilt.expo
        errorText: tilt.error
        onEnabledRequested: value => tilt.enabled = value
        onCalibrateRequested: tilt.calibrate()
        onResetCalibrationRequested: tilt.resetCalibration()
        onEngageRequested: tilt.engage()
        onDisengageRequested: tilt.disengage()
        onFullScaleAngleEdited: degrees => tilt.fullScaleAngleDegrees = degrees
        onDeadzoneEdited: degrees => tilt.deadzoneDegrees = degrees
        onFilterTimeConstantEdited: milliseconds => tilt.filterTimeConstantMs = milliseconds
        onExpoEdited: value => tilt.expo = value
    }
    onVisibleChanged: { if (!visible) tilt.enabled = false; }
}
