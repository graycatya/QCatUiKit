// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit

Item {
    id: control
    property string componentId: "svg-image"
    readonly property VectorItem vector: preview.item as VectorItem
    readonly property string stateText: vector?.status === VectorItem.Ready ? qsTr("Ready")
                                        : vector?.status === VectorItem.Loading ? qsTr("Loading")
                                        : vector?.status === VectorItem.Error ? qsTr("Error") : qsTr("Empty")
    readonly property string exampleCode: componentId === "vector-canvas"
        ? 'CatVectorCanvas {\n    width: 320; height: 200\n    viewBox: Qt.rect(0, 0, 160, 100)\n    shapes: [{ type: "ellipse", x: 20, y: 10, width: 120, height: 80, fill: Theme.tokens.primary }]\n}'
        : componentId === "lottie-animation"
          ? 'CatLottieAnimation {\n    width: 320; height: 200\n    source: "animation.json"\n    autoPlay: true\n    loops: -1\n}'
          : 'CatSvgImage {\n    width: 320; height: 200\n    source: "illustration.svg"\n    fillMode: VectorItem.PreserveAspectFit\n}'
    implicitHeight: layout.implicitHeight
    ColumnLayout {
        id: layout
        width: parent.width
        spacing: Theme.tokens.spacing
        CatLabel {
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            text: control.vector?.backendAvailable
                  ? qsTr("Native vector rendering | SVG, animation and shapes")
                  : qsTr("This build does not include vector rendering support.")
        }
        CatPane {
            Layout.fillWidth: true
            Layout.preferredHeight: 240
            Loader {
                id: preview
                anchors.fill: parent
                anchors.margins: Theme.tokens.spacing
                sourceComponent: control.componentId === "vector-canvas" ? canvasScene
                                 : control.componentId === "lottie-animation" ? animationScene : svgScene
            }
        }
        GridLayout {
            Layout.fillWidth: true
            columns: control.width < 420 ? 2 : 4
            CatLabel { text: qsTr("Scale mode") }
            CatComboBox {
                model: [qsTr("Stretch"), qsTr("Fit"), qsTr("Crop")]
                currentIndex: 1
                onActivated: if (control.vector) control.vector.fillMode = currentIndex
            }
            CatButton { text: qsTr("Reload"); onClicked: control.vector?.reload() }
            Item { Layout.fillWidth: true; visible: control.width >= 420 }
        }
        GridLayout {
            columns: control.width < 420 ? 2 : 4
            visible: control.componentId === "lottie-animation"
            Layout.fillWidth: true
            CatButton {
                text: control.vector?.playing ? qsTr("Pause") : qsTr("Play")
                enabled: control.vector?.status === VectorItem.Ready
                onClicked: {
                    if (control.vector.playing) control.vector.pause(); else control.vector.play();
                }
            }
            CatButton { text: qsTr("Stop"); onClicked: control.vector?.stop() }
            CatSlider {
                Layout.fillWidth: true
                from: 0; to: 1
                value: control.vector?.progress ?? 0
                onMoved: if (control.vector) control.vector.progress = value
            }
            CatLabel { text: qsTr("Frame %1 / %2").arg(Math.round(control.vector?.currentFrame ?? 0)).arg(control.vector?.frameCount ?? 0) }
        }
        CatButton {
            objectName: "vectorAddShape"
            visible: control.componentId === "vector-canvas"
            text: qsTr("Add a circle")
            onClicked: control.vector?.appendShape({type: "ellipse", x: 65, y: 35, width: 30, height: 30, fill: Theme.tokens.primary})
        }
        CatLabel {
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            text: qsTr("Status: %1 | Rendered frames: %2").arg(control.stateText).arg(control.vector?.renderedFrames ?? 0)
        }
    }
    Component { id: svgScene; CatSvgImage { source: Qt.resolvedUrl("assets/vector-demo.svg") } }
    Component { id: animationScene; CatLottieAnimation { source: Qt.resolvedUrl("assets/vector-demo.json") } }
    Component {
        id: canvasScene
        CatVectorCanvas {
            viewBox: Qt.rect(0, 0, 160, 100)
            shapes: [
                {type: "rectangle", x: 8, y: 8, width: 144, height: 84, radius: 18,
                 fill: {type: "linear", x1: 0, y1: 0, x2: 160, y2: 100,
                        stops: [{offset: 0, color: "#3b82f6"}, {offset: 1, color: "#a855f7"}]}},
                {type: "path", commands: [["M", 45, 50], ["L", 70, 75], ["C", 90, 50, 110, 40, 120, 25]],
                 fill: "transparent", stroke: "white", strokeWidth: 7}
            ]
        }
    }
}
