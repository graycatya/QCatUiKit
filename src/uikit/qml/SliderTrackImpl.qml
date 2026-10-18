// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit
import "SliderMath.js" as SliderMath

Item {
    id: track
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property bool horizontal: true
    property real thickness: 4
    property real startPosition: 0
    property real endPosition: 0
    property real from: 0
    property real to: 1
    property bool reversed: false
    property real tickInterval: 0
    property var tickValues: []
    property bool tickLabelsVisible: false
    property var formatter: null
    property real defaultPosition: -1
    property real labelStart: 0
    property real labelEnd: horizontal ? width : height
    readonly property var marks: SliderMath.ticks(from, to, tickInterval, tickValues)
    FontMetrics { id: metrics; font.pixelSize: track.tokens.fontSize - 2 }
    readonly property var labeledMarks: {
        const fontHeight = metrics.height;
        const length = horizontal ? width : height;
        const boxes = marks.map((value, index) => {
            const text = typeof formatter === "function" ? String(formatter(value)) : String(value);
            const size = horizontal ? metrics.advanceWidth(text) : fontHeight;
            const position = SliderMath.position(value, from, to);
            const coordinate = (reversed ? 1 - position : position) * length;
            const start = SliderMath.clamp(coordinate - size / 2, labelStart, labelEnd - size);
            return {index: index, start: start, end: start + size};
        }).sort((a, b) => a.start - b.start);
        if (!boxes.length) return [];
        const chosen = [boxes[0].index], last = boxes[boxes.length - 1];
        let edge = boxes[0].end;
        for (let i = 1; i + 1 < boxes.length; ++i) {
            if (boxes[i].start >= edge + 8 && boxes[i].end + 8 <= last.start) {
                chosen.push(boxes[i].index); edge = boxes[i].end;
            }
        }
        if (boxes.length > 1 && last.start >= edge + 8) chosen.push(last.index);
        return chosen;
    }
    Rectangle {
        anchors.fill: parent
        radius: track.thickness / 2
        color: track.enabled ? track.tokens.border : track.tokens.disabled
        Rectangle {
            x: track.horizontal ? Math.min(track.startPosition, track.endPosition) * parent.width : 0
            y: track.horizontal ? 0 : Math.min(track.startPosition, track.endPosition) * parent.height
            width: track.horizontal ? Math.abs(track.endPosition - track.startPosition) * parent.width : parent.width
            height: track.horizontal ? parent.height : Math.abs(track.endPosition - track.startPosition) * parent.height
            radius: track.thickness / 2
            color: track.enabled ? track.tokens.primary : track.tokens.textDisabled
        }
    }
    Repeater {
        model: track.marks
        delegate: Item {
            id: mark
            required property real modelData
            required property int index
            readonly property real position: SliderMath.position(modelData, track.from, track.to)
            readonly property real visual: track.reversed ? 1 - position : position
            x: track.horizontal ? visual * track.width : track.width / 2
            y: track.horizontal ? track.height / 2 : visual * track.height
            Rectangle {
                x: track.horizontal ? -1 : track.thickness / 2 + 3
                y: track.horizontal ? track.thickness / 2 + 3 : -1
                width: track.horizontal ? 2 : 4
                height: track.horizontal ? 4 : 2
                color: track.tokens.textDisabled
            }
            CatLabel {
                theme: track.theme
                text: typeof track.formatter === "function" ? track.formatter(mark.modelData) : String(mark.modelData)
                textFormat: Text.PlainText
                font.pixelSize: track.tokens.fontSize - 2
                color: track.enabled ? track.tokens.textSecondary : track.tokens.textDisabled
                visible: track.tickLabelsVisible && track.labeledMarks.includes(mark.index)
                x: track.horizontal ? SliderMath.clamp(-implicitWidth / 2, track.labelStart - mark.x,
                    track.labelEnd - mark.x - implicitWidth) : track.thickness / 2 + 12
                y: track.horizontal ? track.thickness / 2 + 10
                    : SliderMath.clamp(-implicitHeight / 2, track.labelStart - mark.y, track.labelEnd - mark.y - implicitHeight)
            }
        }
    }
    Rectangle {
        visible: track.defaultPosition >= 0 && track.defaultPosition <= 1
        x: track.horizontal ? track.defaultPosition * track.width - 1 : (track.width - width) / 2
        y: track.horizontal ? (track.height - height) / 2 : track.defaultPosition * track.height - 1
        width: track.horizontal ? 2 : track.thickness + 6
        height: track.horizontal ? track.thickness + 6 : 2
        color: track.tokens.textSecondary
    }
}
