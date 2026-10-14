// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QCatUiKit
import QtQuick

CatTextField {
    id: control
    property var locale: Qt.locale()
    property real value: 0
    property real minimum: -Infinity
    property real maximum: Infinity
    property real smallChange: 1
    property real largeChange: 10
    property bool acceptsExpression: false
    property int decimals: 2
    signal valueEdited(real value)
    rightPadding: tokens.padding + tokens.controlHeight
    inputMethodHints: Qt.ImhFormattedNumbersOnly
    function commitValue(candidate: real): bool {
        if (!isFinite(candidate))
            return false;
        invalid = false;
        value = Math.max(minimum, Math.min(maximum, candidate));
        text = Number(value).toLocaleString(locale, 'f', decimals);
        valueEdited(value);
        return true;
    }
    function parseExpression(source: string): real {
        const compact = source.replace(/\s/g, "");
        if (!/^[0-9.+*/()-]+$/.test(compact))
            return NaN;
        let offset = 0;
        function atom() {
            if (compact[offset] === '+') {
                offset++;
                return atom();
            }
            if (compact[offset] === '-') {
                offset++;
                return -atom();
            }
            if (compact[offset] === '(') {
                offset++;
                const n = sum();
                if (compact[offset++] !== ')')
                    return NaN;
                return n;
            }
            const match = compact.slice(offset).match(/^(?:\d+(?:\.\d*)?|\.\d+)/);
            if (!match)
                return NaN;
            offset += match[0].length;
            return Number(match[0]);
        }
        function product() {
            let n = atom();
            while (compact[offset] === '*' || compact[offset] === '/') {
                const op = compact[offset++];
                const rhs = atom();
                n = op === '*' ? n * rhs : n / rhs;
            }
            return n;
        }
        function sum() {
            let n = product();
            while (compact[offset] === '+' || compact[offset] === '-') {
                const op = compact[offset++];
                const rhs = product();
                n = op === '+' ? n + rhs : n - rhs;
            }
            return n;
        }
        const result = sum();
        return offset === compact.length ? result : NaN;
    }
    function commitText(): bool {
        let candidate = NaN;
        try {
            candidate = acceptsExpression ? parseExpression(text) : Number.fromLocaleString(locale,
                                                                                            text);
        } catch (error) {
            invalid = true;
            return false;
        }
        const valid = commitValue(candidate);
        invalid = !valid;
        return valid;
    }
    function synchronize(): void {
        invalid = false;
        text = Number(value).toLocaleString(locale, 'f', decimals);
    }
    onValueChanged: {
        const bounded = Math.max(minimum, Math.min(maximum, value));
        if (bounded !== value && isFinite(bounded))
            value = bounded;
        synchronize();
    }
    onMinimumChanged: if (value < minimum)
                          value = minimum
    onMaximumChanged: if (value > maximum)
                          value = maximum
    onDecimalsChanged: synchronize()
    onLocaleChanged: synchronize()
    Component.onCompleted: synchronize()
    onEditingFinished: {
        if (invalid || text !== Number(value).toLocaleString(locale, 'f', decimals))
            commitText();
    }
    Keys.onUpPressed: event => {
        commitValue(value + smallChange);
        event.accepted = true;
    }
    Keys.onDownPressed: event => {
        commitValue(value - smallChange);
        event.accepted = true;
    }
    Keys.onPressed: event => {
        if (event.key === Qt.Key_PageUp || event.key === Qt.Key_PageDown) {
            commitValue(value + (event.key === Qt.Key_PageUp ? largeChange : -largeChange));
            event.accepted = true;
        }
    }
    Column {
        anchors.right: parent.right
        width: control.tokens.controlHeight
        height: parent.height
        CatRepeatButton {
            theme: control.theme
            width: parent.width
            implicitWidth: width
            height: parent.height / 2
            flat: true
            padding: 4
            topPadding: 0
            bottomPadding: 0
            font.pixelSize: 14
            text: "+"
            enabled: control.value < control.maximum
            Accessible.name: qsTr("Increase")
            onClicked: control.commitValue(control.value + control.smallChange)
        }
        CatRepeatButton {
            theme: control.theme
            width: parent.width
            implicitWidth: width
            height: parent.height / 2
            flat: true
            padding: 4
            topPadding: 0
            bottomPadding: 0
            font.pixelSize: 14
            text: "−"
            enabled: control.value > control.minimum
            Accessible.name: qsTr("Decrease")
            onClicked: control.commitValue(control.value - control.smallChange)
        }
    }
}
