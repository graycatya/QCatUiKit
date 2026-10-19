// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QCatUiKit

CatPane {
    id: control
    property bool controlEnabled: false
    property bool controlAllowed: false
    property bool sampleFresh: false
    property int controllerState: TiltController.Disabled
    property real rollInput: 0
    property real pitchInput: 0
    property real fullScaleAngleDegrees: 25
    property real deadzoneDegrees: 2
    property real filterTimeConstantMs: 100
    property real expo: .2
    property string errorText: ""
    readonly property bool holding: holdButton.held
    readonly property bool canEngage: enabled && visible && controlEnabled && controlAllowed
        && sampleFresh && (controllerState === TiltController.Ready || controllerState === TiltController.Active)
    signal enabledRequested(bool value)
    signal calibrateRequested()
    signal engageRequested()
    signal disengageRequested()
    signal fullScaleAngleEdited(real degrees)
    signal deadzoneEdited(real degrees)
    signal filterTimeConstantEdited(real milliseconds)
    signal expoEdited(real value)
    signal resetCalibrationRequested()
    implicitWidth: 380
    onCanEngageChanged: { if (!canEngage) holdButton.releaseControl(); }
    Component.onDestruction: { if (holding) disengageRequested(); }
    Connections {
        target: control.Window.window
        function onActiveChanged() { if (!control.Window.window.active) holdButton.releaseControl(); }
    }
    Connections {
        target: Application
        function onStateChanged() { if (Application.state !== Qt.ApplicationActive) holdButton.releaseControl(); }
    }
    contentItem: ColumnLayout {
        spacing: control.tokens.spacing
        CatSwitch {
            objectName: "tiltEnableSwitch"
            theme: control.theme
            Layout.fillWidth: true
            text: qsTr("Enable tilt input")
            checked: control.controlEnabled
            onToggled: control.enabledRequested(checked)
        }
        CatLabel {
            theme: control.theme; Layout.fillWidth: true; wrapMode: Text.Wrap
            text: {
                switch (control.controllerState) {
                case TiltController.Uncalibrated: return qsTr("Calibration required");
                case TiltController.Calibrating: return qsTr("Keep the phone still to calibrate…");
                case TiltController.Ready: return qsTr("Ready — preview the input before engaging");
                case TiltController.Active: return qsTr("Control input active");
                case TiltController.Fault: return qsTr("Input fault — reset and recalibrate");
                default: return qsTr("Tilt input disabled");
                }
            }
        }
        CatLabel {
            theme: control.theme; Layout.fillWidth: true; wrapMode: Text.Wrap
            text: control.errorText; visible: text.length > 0; color: control.tokens.danger
        }
        Flow {
            Layout.fillWidth: true; spacing: control.tokens.spacing
            CatButton {
                objectName: "tiltCalibrateButton"
                theme: control.theme; text: qsTr("Calibrate")
                enabled: control.controlEnabled && control.sampleFresh
                    && (control.controllerState === TiltController.Ready || control.controllerState === TiltController.Uncalibrated)
                onClicked: control.calibrateRequested()
            }
            CatButton {
                objectName: "tiltResetButton"
                theme: control.theme; text: qsTr("Reset calibration")
                enabled: control.controlEnabled && control.controllerState !== TiltController.Active
                onClicked: control.resetCalibrationRequested()
            }
        }
        CatLabel { theme: control.theme; text: qsTr("Full-scale angle (smaller is more sensitive)"); Layout.fillWidth: true; wrapMode: Text.Wrap }
        CatSliderField {
            objectName: "tiltFullScaleField"
            theme: control.theme; Layout.fillWidth: true
            from: 5; to: 60
            value: control.fullScaleAngleDegrees; stepSize: 1; decimals: 0; suffix: "°"
            enabled: control.controllerState !== TiltController.Active
            Accessible.name: qsTr("Full-scale angle")
            onValueEdited: value => control.fullScaleAngleEdited(value)
        }
        CatExpander {
            theme: control.theme; Layout.fillWidth: true
            headerText: qsTr("Input tuning")
            ColumnLayout {
                width: parent.width
                CatLabel { theme: control.theme; text: qsTr("Deadzone") }
                CatSliderField {
                    theme: control.theme; Layout.fillWidth: true
                    from: 0; to: Math.max(0, control.fullScaleAngleDegrees)
                    value: control.deadzoneDegrees; stepSize: .5; decimals: 1; suffix: "°"
                    enabled: control.controllerState !== TiltController.Active
                    Accessible.name: qsTr("Deadzone")
                    onValueEdited: value => control.deadzoneEdited(value)
                }
                CatLabel { theme: control.theme; text: qsTr("Filter time constant") }
                CatSliderField {
                    theme: control.theme; Layout.fillWidth: true
                    from: 0; to: 1000; value: control.filterTimeConstantMs; stepSize: 10; decimals: 0; suffix: "ms"
                    enabled: control.controllerState !== TiltController.Active
                    Accessible.name: qsTr("Filter time constant")
                    onValueEdited: value => control.filterTimeConstantEdited(value)
                }
                CatLabel { theme: control.theme; text: qsTr("Response curve") }
                CatSliderField {
                    theme: control.theme; Layout.fillWidth: true
                    from: 0; to: 1; value: control.expo; stepSize: .05; decimals: 2
                    enabled: control.controllerState !== TiltController.Active
                    Accessible.name: qsTr("Response curve")
                    onValueEdited: value => control.expoEdited(value)
                }
            }
        }
        CatLabel {
            theme: control.theme; text: qsTr("Phone control input")
            Layout.fillWidth: true; wrapMode: Text.Wrap
        }
        Repeater {
            model: [qsTr("Roll / right"), qsTr("Pitch / forward")]
            delegate: ColumnLayout {
                id: axis
                required property int index
                required property string modelData
                readonly property real input: index === 0 ? control.rollInput : control.pitchInput
                readonly property real bounded: Number.isFinite(input) ? Math.max(-1, Math.min(1, input)) : 0
                Layout.fillWidth: true
                CatLabel {
                    theme: control.theme; Layout.fillWidth: true; wrapMode: Text.Wrap
                    text: axis.modelData + ": " + (control.sampleFresh ? axis.bounded.toFixed(2) : "—")
                }
                Rectangle {
                    Layout.fillWidth: true; implicitHeight: 12; radius: 6; color: control.tokens.surfaceHover
                    Rectangle {
                        x: parent.width * (axis.bounded < 0 ? (1 + axis.bounded) / 2 : .5)
                        width: Math.abs(axis.bounded) * parent.width / 2; height: parent.height; radius: 6
                        color: control.tokens.primary; visible: control.sampleFresh
                    }
                    Rectangle { x: parent.width / 2; width: 1; height: parent.height; color: control.tokens.textSecondary }
                }
            }
        }
        CatButton {
            id: holdButton
            objectName: "tiltHoldButton"
            property bool held: false
            function releaseControl(): void {
                if (!held) return;
                held = false;
                control.disengageRequested();
            }
            theme: control.theme; Layout.fillWidth: true
            text: held ? qsTr("Release to stop control") : qsTr("Hold to control")
            primary: held; enabled: control.canEngage
            onPressed: { held = true; control.engageRequested(); }
            onReleased: releaseControl()
            onCanceled: releaseControl()
            onPressedChanged: { if (!pressed) releaseControl(); }
        }
    }
}
