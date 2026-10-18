// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QCatUiKit
import "SliderMath.js" as SliderMath

T.RangeSlider {
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
    property real minimumSpan: 0
    property real maximumSpan: Infinity
    // Negative disables the fixed-span constraint.
    property real fixedSpan: -1
    property bool rangeDragEnabled: false
    readonly property bool rangeDragging: dragArea.pressed
    property real defaultFirstValue: from
    property real defaultSecondValue: to
    signal moved()
    signal valuesCommitted(real firstValue, real secondValue)

    function textFromValue(number: real): string {
        return SliderMath.format(number, locale, decimals, suffix, valueFormatter);
    }
    function constrain(anchor: int): void {
        if (!interaction.ready || interaction.adjusting) return;
        const pair = SliderMath.constrain(first.value, second.value, from, to,
            minimumSpan, maximumSpan, fixedSpan, anchor);
        interaction.adjusting = true;
        if (first.value !== pair[0] || second.value !== pair[1]) setValues(pair[0], pair[1]);
        interaction.adjusting = false;
    }
    function normalize(): void { constrain(0); }
    function scheduleNormalize(): void { if (interaction.ready && !interaction.adjusting) normalizeTimer.restart(); }
    function handleMoved(anchor: int): void { constrain(anchor); interaction.edited = true; moved(); }
    function finishEdit(): void {
        if (interaction.edited) { interaction.edited = false; valuesCommitted(first.value, second.value); }
    }
    function reset(): void {
        const pair = SliderMath.constrain(defaultFirstValue, defaultSecondValue, from, to,
            minimumSpan, maximumSpan, fixedSpan, 0);
        if (first.value === pair[0] && second.value === pair[1]) return;
        setValues(pair[0], pair[1]); interaction.edited = true; moved(); finishEdit();
    }
    function visual(position: real): real { return horizontal ? (mirrored ? 1 - position : position) : 1 - position; }
    function translateRange(coordinate: real, finalMove: bool): void {
        const extent = horizontal ? availableWidth - first.handle.width : availableHeight - first.handle.height;
        if (extent <= 0) return;
        let delta = (coordinate - interaction.startCoordinate) / extent * (to - from)
            * ((horizontal ? mirrored : true) ? -1 : 1);
        if (stepSize > 0 && (snapMode === T.RangeSlider.SnapAlways || (finalMove && snapMode === T.RangeSlider.SnapOnRelease)))
            delta = Math.round(delta / stepSize) * stepSize;
        delta = SliderMath.clamp(delta, Math.min(from, to) - Math.min(interaction.startFirst, interaction.startSecond),
            Math.max(from, to) - Math.max(interaction.startFirst, interaction.startSecond));
        interaction.previewFirst = interaction.startFirst + delta;
        interaction.previewSecond = interaction.startSecond + delta;
        if (live || finalMove) {
            if (first.value !== interaction.previewFirst || second.value !== interaction.previewSecond) {
                setValues(interaction.previewFirst, interaction.previewSecond);
                interaction.edited = true; moved();
            }
        }
    }
    QtObject {
        id: interaction
        property bool ready: false
        property bool adjusting: false
        property bool edited: false
        property bool dragActive: false
        property real startCoordinate: 0
        property real startFirst: 0
        property real startSecond: 1
        property real previewFirst: 0
        property real previewSecond: 1
        readonly property real firstVisual: interaction.dragActive && control.rangeDragging && !control.live
            ? control.visual(SliderMath.position(previewFirst, control.from, control.to)) : control.first.visualPosition
        readonly property real secondVisual: interaction.dragActive && control.rangeDragging && !control.live
            ? control.visual(SliderMath.position(previewSecond, control.from, control.to)) : control.second.visualPosition
    }
    // Owned by the control: queued work is canceled when a Loader destroys it.
    Timer { id: normalizeTimer; interval: 0; onTriggered: control.normalize() }
    Component.onCompleted: { interaction.ready = true; normalize(); }
    onFromChanged: scheduleNormalize()
    onToChanged: scheduleNormalize()
    onMinimumSpanChanged: scheduleNormalize()
    onMaximumSpanChanged: scheduleNormalize()
    onFixedSpanChanged: scheduleNormalize()
    first.onValueChanged: scheduleNormalize()
    second.onValueChanged: scheduleNormalize()
    first.onMoved: handleMoved(1)
    second.onMoved: handleMoved(2)
    first.onPressedChanged: if (!first.pressed && !second.pressed) finishEdit()
    second.onPressedChanged: if (!first.pressed && !second.pressed) finishEdit()
    onActiveFocusChanged: if (!activeFocus && !first.pressed && !second.pressed && !rangeDragging) finishEdit()
    Keys.onReleased: event => { if (!event.isAutoRepeat) finishEdit(); event.accepted = false; }

    implicitWidth: horizontal ? 200 : Math.max(tokens.controlHeight, first.implicitHandleWidth + leftPadding + rightPadding)
    implicitHeight: horizontal ? Math.max(tokens.controlHeight, first.implicitHandleHeight + topPadding + bottomPadding) : 200
    padding: tokens.padding / 2
    bottomPadding: horizontal && tickLabelsVisible ? tokens.padding / 2 + tokens.fontSize + 14 : undefined
    rightPadding: !horizontal && tickLabelsVisible ? tokens.padding / 2 + 64 : undefined
    hoverEnabled: tokens.hoverEnabled
    focusPolicy: Qt.StrongFocus
    background: SliderTrackImpl {
        theme: control.theme
        horizontal: control.horizontal
        thickness: Math.max(1, control.trackThickness)
        x: horizontal ? control.leftPadding + control.first.handle.width / 2 : control.leftPadding + (control.availableWidth - width) / 2
        y: horizontal ? control.topPadding + (control.availableHeight - height) / 2 : control.topPadding + control.first.handle.height / 2
        width: horizontal ? Math.max(0, control.availableWidth - control.first.handle.width) : thickness
        height: horizontal ? thickness : Math.max(0, control.availableHeight - control.first.handle.height)
        from: control.from; to: control.to
        reversed: horizontal ? control.mirrored : true
        startPosition: interaction.firstVisual
        endPosition: interaction.secondVisual
        tickInterval: control.tickInterval
        tickValues: control.tickValues
        tickLabelsVisible: control.tickLabelsVisible
        formatter: control.textFromValue
        labelStart: horizontal ? -x : -y
        labelEnd: horizontal ? control.width - x : control.height - y
    }
    first.handle: SliderHandleImpl {
        theme: control.theme
        diameter: control.handleDiameter
        x: control.horizontal ? control.leftPadding + interaction.firstVisual * (control.availableWidth - width)
            : control.leftPadding + (control.availableWidth - width) / 2
        y: control.horizontal ? control.topPadding + (control.availableHeight - height) / 2
            : control.topPadding + interaction.firstVisual * (control.availableHeight - height)
        down: control.first.pressed || control.rangeDragging
        highlighted: control.first.hovered
        tipVisible: control.valueToolTipVisible && (down || control.first.hovered || activeFocus)
        tipText: control.textFromValue(control.rangeDragging ? interaction.previewFirst
            : control.from + control.first.position * (control.to - control.from))
        Accessible.name: control.Accessible.name + " " + qsTr("Start value")
    }
    second.handle: SliderHandleImpl {
        theme: control.theme
        diameter: control.handleDiameter
        x: control.horizontal ? control.leftPadding + interaction.secondVisual * (control.availableWidth - width)
            : control.leftPadding + (control.availableWidth - width) / 2
        y: control.horizontal ? control.topPadding + (control.availableHeight - height) / 2
            : control.topPadding + interaction.secondVisual * (control.availableHeight - height)
        down: control.second.pressed || control.rangeDragging
        highlighted: control.second.hovered
        tipVisible: control.valueToolTipVisible && (down || control.second.hovered || activeFocus)
        tipText: control.textFromValue(control.rangeDragging ? interaction.previewSecond
            : control.from + control.second.position * (control.to - control.from))
        Accessible.name: control.Accessible.name + " " + qsTr("End value")
    }
    // Only the interior of the selected interval captures whole-range drags.
    // The native RangeSlider keeps ownership of both handles and their keyboard input.
    MouseArea {
        id: dragArea
        objectName: "rangeDragArea"
        enabled: control.rangeDragEnabled && control.enabled
        x: control.horizontal ? Math.min(control.first.handle.x, control.second.handle.x) + control.first.handle.width : control.leftPadding
        y: control.horizontal ? control.topPadding : Math.min(control.first.handle.y, control.second.handle.y) + control.first.handle.height
        width: control.horizontal ? Math.max(0, Math.abs(control.second.handle.x - control.first.handle.x) - control.first.handle.width) : control.availableWidth
        height: control.horizontal ? control.availableHeight : Math.max(0, Math.abs(control.second.handle.y - control.first.handle.y) - control.first.handle.height)
        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor
        preventStealing: true
        onPressed: mouse => {
            control.first.handle.forceActiveFocus(Qt.MouseFocusReason);
            const point = mapToItem(control, mouse.x, mouse.y);
            interaction.startCoordinate = control.horizontal ? point.x : point.y;
            interaction.startFirst = control.first.value; interaction.startSecond = control.second.value;
            interaction.previewFirst = interaction.startFirst; interaction.previewSecond = interaction.startSecond;
            interaction.dragActive = true;
        }
        onPositionChanged: mouse => {
            if (!pressed) return;
            const point = mapToItem(control, mouse.x, mouse.y);
            control.translateRange(control.horizontal ? point.x : point.y, false);
        }
        onReleased: mouse => {
            const point = mapToItem(control, mouse.x, mouse.y);
            control.translateRange(control.horizontal ? point.x : point.y, true);
            interaction.dragActive = false;
            control.finishEdit();
        }
        onCanceled: { interaction.dragActive = false; control.finishEdit(); }
    }
}
