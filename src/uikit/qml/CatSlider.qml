// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QCatUiKit
import "SliderMath.js" as SliderMath

T.Slider {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property real trackThickness: 4
    property real handleDiameter: 22
    property real tickInterval: 0
    property var tickValues: []
    property bool tickLabelsVisible: false
    property bool valueToolTipVisible: false
    property int decimals: 0
    property string suffix: ""
    property var valueFormatter: null
    property real fillOrigin: from
    property real defaultValue: from
    property bool defaultIndicatorVisible: false
    signal valueCommitted(real value)

    function textFromValue(number: real): string {
        return SliderMath.format(number, locale, decimals, suffix, valueFormatter);
    }
    function editValue(number: real): void {
        const next = SliderMath.clamp(number, Math.min(from, to), Math.max(from, to));
        if (!isFinite(next) || value === next) return;
        interaction.inputValue = next;
        interaction.applyingInput = true;
        moved(); finishEdit();
        interaction.applyingInput = false;
    }
    function reset(): void { editValue(defaultValue); }
    function finishEdit(): void {
        if (interaction.edited) { interaction.edited = false; valueCommitted(value); }
    }
    QtObject {
        id: interaction
        property bool edited: false
        property bool applyingInput: false
        property real inputValue: 0
    }
    // Restore a caller's value binding after the user updates its backing model
    // in onMoved. Without an original binding the entered value is retained.
    Binding {
        target: control; property: "value"
        value: interaction.inputValue
        when: interaction.applyingInput
        restoreMode: Binding.RestoreBinding
    }
    onMoved: interaction.edited = true
    onPressedChanged: if (!pressed) finishEdit()
    onActiveFocusChanged: if (!activeFocus && !pressed) finishEdit()
    Keys.onReleased: event => { if (!event.isAutoRepeat) finishEdit(); event.accepted = false; }

    implicitWidth: horizontal ? 200 : Math.max(tokens.controlHeight, implicitHandleWidth + leftPadding + rightPadding)
    implicitHeight: horizontal ? Math.max(tokens.controlHeight, implicitHandleHeight + topPadding + bottomPadding) : 200
    padding: tokens.padding / 2
    bottomPadding: horizontal && tickLabelsVisible ? tokens.padding / 2 + tokens.fontSize + 14 : undefined
    rightPadding: !horizontal && tickLabelsVisible ? tokens.padding / 2 + 64 : undefined
    hoverEnabled: tokens.hoverEnabled
    focusPolicy: Qt.StrongFocus
    background: SliderTrackImpl {
        theme: control.theme
        horizontal: control.horizontal
        thickness: Math.max(1, control.trackThickness)
        x: horizontal ? control.leftPadding + control.handle.width / 2 : control.leftPadding + (control.availableWidth - width) / 2
        y: horizontal ? control.topPadding + (control.availableHeight - height) / 2 : control.topPadding + control.handle.height / 2
        width: horizontal ? Math.max(0, control.availableWidth - control.handle.width) : thickness
        height: horizontal ? thickness : Math.max(0, control.availableHeight - control.handle.height)
        from: control.from; to: control.to
        reversed: horizontal ? control.mirrored : true
        readonly property real origin: SliderMath.position(control.fillOrigin, from, to)
        readonly property real defaultMark: SliderMath.position(control.defaultValue, from, to)
        startPosition: reversed ? 1 - origin : origin
        endPosition: control.visualPosition
        defaultPosition: control.defaultIndicatorVisible ? (reversed ? 1 - defaultMark : defaultMark) : -1
        tickInterval: control.tickInterval
        tickValues: control.tickValues
        tickLabelsVisible: control.tickLabelsVisible
        formatter: control.textFromValue
        labelStart: horizontal ? -x : -y
        labelEnd: horizontal ? control.width - x : control.height - y
    }
    handle: SliderHandleImpl {
        theme: control.theme
        diameter: control.handleDiameter
        x: control.horizontal ? control.leftPadding + control.visualPosition * (control.availableWidth - width)
            : control.leftPadding + (control.availableWidth - width) / 2
        y: control.horizontal ? control.topPadding + (control.availableHeight - height) / 2
            : control.topPadding + control.visualPosition * (control.availableHeight - height)
        down: control.pressed
        highlighted: control.hovered
        focused: control.visualFocus
        tipVisible: control.valueToolTipVisible && (control.pressed || control.hovered || control.visualFocus)
        tipText: control.textFromValue(control.live ? control.value : control.valueAt(control.position))
    }
}
