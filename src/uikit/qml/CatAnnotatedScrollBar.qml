// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QCatUiKit

T.Control {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property real minimum: 0
    property real maximum: 100
    property real value: 0
    property real pageStep: 10
    property var annotations: []
    signal annotationActivated(int index)
    implicitWidth: 140
    implicitHeight: 240
    function seek(candidate: real): void {
        const bounded = Math.max(minimum, Math.min(maximum, candidate));
        if (Math.abs(value - bounded) > 1e-8)
            value = bounded;
    }
    onValueChanged: if (value < minimum || value > maximum)
                        seek(value)
    CatScrollBar {
        theme: control.theme
        anchors.right: parent.right
        height: parent.height
        orientation: Qt.Vertical
        policy: T.ScrollBar.AlwaysOn
        size: control.maximum <= control.minimum ? 1 : control.pageStep / (control.maximum - control.minimum
                                                                           + control.pageStep)
        position: control.maximum <= control.minimum ? 0 : (control.value - control.minimum) / (
                                                           control.maximum - control.minimum
                                                           + control.pageStep)
        onPositionChanged: control.seek(control.minimum + position * (control.maximum
                                                                      - control.minimum
                                                                      + control.pageStep))
    }
    Repeater {
        model: control.annotations
        CatButton {
            required property var modelData
            required property int index
            theme: control.theme
            text: String(modelData.label || "")
            width: Math.max(0, control.width - 16)
            y: Math.max(0, Math.min(control.height - height, Number(modelData.position || 0) * (
                                        control.height - height)))
            flat: true
            font.pixelSize: 12
            onClicked: {
                control.seek(control.minimum + Number(modelData.position || 0) * (control.maximum
                                                                                  - control.minimum));
                control.annotationActivated(index);
            }
        }
    }
}
