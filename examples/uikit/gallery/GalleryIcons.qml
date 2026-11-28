// SPDX-License-Identifier: Apache-2.0
pragma Singleton
import QtQuick

// Codepoints verified against the bundled RemixIcon font. Keep names semantic so call
// sites do not depend on font-specific numbers.
QtObject {
    // main.cpp registers the same file synchronously; this loader keeps tests and other
    // hosts working when they open the Gallery module without that entry point.
    readonly property FontLoader loader: FontLoader {
        source: "qrc:/qt/qml/QCatUiKit/Gallery/assets/remixicon.ttf"
    }
    readonly property string family: loader.status === FontLoader.Ready ? loader.name
                                                                        : "remixicon"
    readonly property string chevronLeft: "\uea64"
    readonly property string chevronRight: "\uea6e"
    readonly property string chevronDown: "\uea4e"
    readonly property string chevronUp: "\uea78"
    readonly property string check: "\ueb7b"
    readonly property string close: "\ueb99"
    readonly property string minimize: "\uf1af"
    readonly property string maximize: "\uf3dc"
    readonly property string restore: "\ueb87"
    readonly property string add: "\uea13"
    readonly property string trash: "\uec2a"
    readonly property string search: "\uf0d1"
    readonly property string refresh: "\uf064"
}
