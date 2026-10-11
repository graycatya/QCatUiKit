// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit

CatLabel {
    id: control
    property string fullText
    property bool running: true
    property int interval: 80
    property int revealed: 0
    readonly property var characters: Array.from(fullText)
    text: (theme || Theme).reducedMotion ? fullText : characters.slice(0, revealed).join("")
    onFullTextChanged: revealed = 0
    function restart(): void {
        revealed = 0;
    }
    Timer {
        interval: Math.max(1, control.interval)
        repeat: true
        running: control.running && control.visible && !(control.theme || Theme).reducedMotion
                 && control.revealed < control.characters.length
        onTriggered: control.revealed += 1
    }
}
