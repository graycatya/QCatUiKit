// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit

ColumnLayout {
    id: control
    objectName: "attitudePreview"
    property real aircraftPitch: 12
    property real aircraftRoll: 25
    property bool telemetryValid: true
    readonly property CatAttitudeIndicator indicator: attitude
    spacing: Theme.tokens.spacing
    CatLabel {
        Layout.fillWidth: true; wrapMode: Text.Wrap
        text: qsTr("Simulated aircraft telemetry. Phone input does not drive this instrument.")
        color: Theme.tokens.textSecondary
    }
    CatAttitudeIndicator {
        id: attitude
        objectName: "demoAttitudeIndicator"
        Layout.alignment: Qt.AlignHCenter
        Layout.preferredWidth: Math.min(320, control.width)
        Layout.preferredHeight: Layout.preferredWidth
        pitchDegrees: control.aircraftPitch
        rollDegrees: control.aircraftRoll
        attitudeValid: control.telemetryValid
    }
    CatLabel { text: qsTr("Aircraft pitch") }
    CatSliderField {
        objectName: "aircraftPitchField"
        Layout.fillWidth: true
        from: -90; to: 90; stepSize: 1; decimals: 0; suffix: "°"
        value: control.aircraftPitch
        Accessible.name: qsTr("Aircraft pitch")
        onValueEdited: value => control.aircraftPitch = value
    }
    CatLabel { text: qsTr("Aircraft roll") }
    CatSliderField {
        objectName: "aircraftRollField"
        Layout.fillWidth: true
        from: -180; to: 180; stepSize: 1; decimals: 0; suffix: "°"
        value: control.aircraftRoll
        Accessible.name: qsTr("Aircraft roll")
        onValueEdited: value => control.aircraftRoll = value
    }
    Flow {
        Layout.fillWidth: true; spacing: Theme.tokens.spacing
        CatSwitch { text: qsTr("Valid telemetry"); checked: control.telemetryValid; onToggled: control.telemetryValid = checked }
        CatCheckBox { text: qsTr("Pitch ladder"); checked: attitude.pitchLadderVisible; onToggled: attitude.pitchLadderVisible = checked }
        CatCheckBox { text: qsTr("Roll scale"); checked: attitude.rollScaleVisible; onToggled: attitude.rollScaleVisible = checked }
        CatCheckBox { text: qsTr("Angle readout"); checked: attitude.readoutVisible; onToggled: attitude.readoutVisible = checked }
    }
    CatButton { text: qsTr("Level aircraft"); onClicked: { control.aircraftPitch = 0; control.aircraftRoll = 0; } }
}
