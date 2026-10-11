// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QtQuick.Layouts
import QCatUiKit

T.Control {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property var model: []
    property int selectedIndex: -1
    property bool paneOpen: false
    property real expandedPaneWidth: 200
    property real compactModeThresholdWidth: 600
    readonly property bool compact: width < compactModeThresholdWidth
    property Component page
    signal activated(int index)
    implicitWidth: 640
    implicitHeight: 360
    function activate(index: int): void {
        selectedIndex = index;
        paneOpen = false;
        activated(index);
    }
    contentItem: RowLayout {
        spacing: control.tokens.spacing
        CatListView {
            theme: control.theme
            model: control.model
            currentIndex: control.selectedIndex
            visible: !control.compact
            Layout.preferredWidth: control.expandedPaneWidth
            Layout.fillHeight: true
            onActivated: index => control.activate(index)
        }
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            CatButton {
                theme: control.theme
                text: "☰"
                padding: 6
                implicitWidth: control.tokens.controlHeight
                visible: control.compact
                Accessible.name: qsTr("Open navigation")
                onClicked: control.paneOpen = true
            }
            Loader {
                Layout.fillWidth: true
                Layout.fillHeight: true
                sourceComponent: control.page
            }
        }
    }
    CatDrawer {
        id: drawer
        theme: control.theme
        width: control.expandedPaneWidth
        height: parent ? parent.height : 360
        visible: control.compact && control.paneOpen
        onClosed: control.paneOpen = false
        contentItem: CatListView {
            theme: control.theme
            model: control.model
            currentIndex: control.selectedIndex
            onActivated: index => control.activate(index)
        }
    }
    onCompactChanged: if (!compact)
                          paneOpen = false
}
