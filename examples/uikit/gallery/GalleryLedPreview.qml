// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit

ColumnLayout {
    id: control
    objectName: "ledPreviewDemo"
    readonly property GalleryLedDemo dataController: demo
    property int mode: 0
    property real ledSize: 6
    property real outputBrightness: 1
    property bool previewsActive: true
    spacing: 12
    onVisibleChanged: demo.running = visible
    GalleryLedDemo { id: demo; objectName: "ledDemoData" }
    Connections {
        target: control.Window.window
        function onVisibilityChanged() {
            demo.running = control.visible && control.Window.window.visibility !== Window.Hidden
                           && control.Window.window.visibility !== Window.Minimized;
        }
    }
    Flow {
        Layout.fillWidth: true
        spacing: 8
        CatComboBox {
            objectName: "ledLayoutPicker"
            model: [qsTr("2D matrix"), qsTr("3D sphere")]
            currentIndex: demo.spatial ? 1 : 0
            onActivated: index => demo.spatial = index === 1
        }
        CatComboBox {
            objectName: "ledCountPicker"
            model: [qsTr("4,096 LEDs"), qsTr("10,000 LEDs"), qsTr("100,000 LEDs"), qsTr("Empty layout"), qsTr("Single LED")]
            currentIndex: [4096, 10000, 100000, 0, 1].indexOf(demo.pointCount)
            onActivated: index => demo.pointCount = [4096, 10000, 100000, 0, 1][index]
        }
        CatComboBox {
            objectName: "ledViewPicker"
            model: [qsTr("3D view"), qsTr("2D view")]
            currentIndex: control.mode
            onActivated: index => control.mode = index
        }
        CatButton {
            text: qsTr("Reset view")
            onClicked: {
                (aLoader.item as LedPreviewRenderItem)?.resetView();
                (outputLoader.item as LedPreviewRenderItem)?.resetView();
                (bLoader.item as LedPreviewRenderItem)?.resetView();
            }
        }
        CatButton {
            objectName: "ledPauseButton"
            text: demo.running ? qsTr("Pause colors") : qsTr("Start colors")
            onClicked: demo.running = !demo.running
        }
        CatButton {
            objectName: "ledRecreateButton"
            text: qsTr("Recreate previews")
            onClicked: { control.previewsActive = false; control.previewsActive = true; }
        }
    }
    GridLayout {
        columns: width < 620 ? 1 : 3
        Layout.fillWidth: true
        ColumnLayout {
            Layout.fillWidth: true
            CatLabel { text: qsTr("Point diameter: %1 px").arg(control.ledSize.toFixed(0)) }
            CatSlider { Layout.fillWidth: true; from: 1; to: 20; value: control.ledSize; onMoved: control.ledSize = value }
        }
        ColumnLayout {
            Layout.fillWidth: true
            CatLabel { text: qsTr("A/B mix: %1%").arg((demo.mix * 100).toFixed(0)) }
            CatSlider { objectName: "ledMixSlider"; Layout.fillWidth: true; from: 0; to: 1; value: demo.mix; onMoved: demo.mix = value }
        }
        ColumnLayout {
            Layout.fillWidth: true
            CatLabel { text: qsTr("Output brightness: %1%").arg((control.outputBrightness * 100).toFixed(0)) }
            CatSlider { objectName: "ledBrightnessSlider"; Layout.fillWidth: true; from: 0; to: 1; value: control.outputBrightness; onMoved: control.outputBrightness = value }
        }
    }
    GridLayout {
        Layout.fillWidth: true
        columns: width < 620 ? 1 : 3
        columnSpacing: 8
        rowSpacing: 12
        ColumnLayout {
            Layout.fillWidth: true
            CatLabel { text: qsTr("Deck A"); font.bold: true }
            Loader {
                id: aLoader
                Layout.fillWidth: true
                Layout.preferredHeight: 320
                active: control.previewsActive
                sourceComponent: LedPreview {
                    objectName: "ledDeckA"
                    source: demo.deckA
                    viewMode: control.mode === 0 ? LedPreviewRenderItem.Spatial3D : LedPreviewRenderItem.Plane2D
                    pointSize: control.ledSize
                }
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            CatLabel { text: qsTr("Output"); font.bold: true }
            Loader {
                id: outputLoader
                Layout.fillWidth: true
                Layout.preferredHeight: 320
                active: control.previewsActive
                sourceComponent: LedPreview {
                    objectName: "ledOutput"
                    source: demo.output
                    viewMode: control.mode === 0 ? LedPreviewRenderItem.Spatial3D : LedPreviewRenderItem.Plane2D
                    pointSize: control.ledSize
                    previewBrightness: control.outputBrightness
                }
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            CatLabel { text: qsTr("Deck B"); font.bold: true }
            Loader {
                id: bLoader
                Layout.fillWidth: true
                Layout.preferredHeight: 320
                active: control.previewsActive
                sourceComponent: LedPreview {
                    objectName: "ledDeckB"
                    source: demo.deckB
                    viewMode: control.mode === 0 ? LedPreviewRenderItem.Spatial3D : LedPreviewRenderItem.Plane2D
                    pointSize: control.ledSize
                }
            }
        }
    }
    CatLabel {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        color: Theme.tokens.textSecondary
        text: qsTr("Left drag: orbit in 3D · Middle drag: pan · Wheel: zoom · Double click: reset. Color Hz counts accepted CPU frames; render FPS counts LED render passes, not monitor presentation or GPU timing.")
    }
}
