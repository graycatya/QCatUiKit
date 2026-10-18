// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick.Layouts
import QtQuick
import QCatUiKit

CatButton {
    id: control
    property color selectedColor: tokens.primary
    property bool alphaEnabled: false
    signal colorSelected(color value)
    text: selectedColor.toString()
    leftPadding: tokens.padding + 26
    Rectangle {
        width: 18
        height: 18
        radius: 4
        x: control.mirrored ? control.width - width - control.tokens.padding :
                              control.tokens.padding
        anchors.verticalCenter: parent.verticalCenter
        color: control.selectedColor
        border.color: control.tokens.border
    }
    function chooseColor(candidate: color): void {
        selectedColor = candidate;
        colorSelected(candidate);
    }
    onClicked: picker.open()
    CatPopup {
        id: picker
        theme: control.theme
        y: control.height + 4
        width: 280
        modal: false
        contentItem: ColumnLayout {
            spacing: 8
            Flow {
                Layout.fillWidth: true
                spacing: 8
                Repeater {
                    model: ["#1677ff", "#6366f1", "#008577", "#c24178", "#b75b12", "#ed4264",
                        "#111827", "#ffffff"]
                    CatButton {
                        id: swatch
                        required property string modelData
                        width: 48
                        implicitWidth: width
                        height: 32
                        Accessible.name: modelData
                        background: Rectangle {
                            radius: 4
                            color: swatch.modelData
                            border.color: control.tokens.border
                        }
                        onClicked: control.chooseColor(modelData)
                    }
                }
            }
            RowLayout {
                Layout.fillWidth: true
                CatLabel {
                    theme: control.theme
                    text: qsTr("Red")
                    Layout.preferredWidth: 60
                    elide: Text.ElideRight
                }
                CatSlider {
                    theme: control.theme
                    Layout.fillWidth: true
                    stepSize: 1 / 255
                    valueToolTipVisible: true
                    valueFormatter: n => String(Math.round(n * 255))
                    value: control.selectedColor.r
                    Accessible.name: qsTr("Red")
                    onMoved: control.chooseColor(Qt.rgba(value, control.selectedColor.g,
                                                         control.selectedColor.b,
                                                         control.selectedColor.a))
                }
            }
            RowLayout {
                Layout.fillWidth: true
                CatLabel {
                    theme: control.theme
                    text: qsTr("Green")
                    Layout.preferredWidth: 60
                    elide: Text.ElideRight
                }
                CatSlider {
                    theme: control.theme
                    Layout.fillWidth: true
                    stepSize: 1 / 255
                    valueToolTipVisible: true
                    valueFormatter: n => String(Math.round(n * 255))
                    value: control.selectedColor.g
                    Accessible.name: qsTr("Green")
                    onMoved: control.chooseColor(Qt.rgba(control.selectedColor.r, value,
                                                         control.selectedColor.b,
                                                         control.selectedColor.a))
                }
            }
            RowLayout {
                Layout.fillWidth: true
                CatLabel {
                    theme: control.theme
                    text: qsTr("Blue")
                    Layout.preferredWidth: 60
                    elide: Text.ElideRight
                }
                CatSlider {
                    theme: control.theme
                    Layout.fillWidth: true
                    stepSize: 1 / 255
                    valueToolTipVisible: true
                    valueFormatter: n => String(Math.round(n * 255))
                    value: control.selectedColor.b
                    Accessible.name: qsTr("Blue")
                    onMoved: control.chooseColor(Qt.rgba(control.selectedColor.r,
                                                         control.selectedColor.g, value,
                                                         control.selectedColor.a))
                }
            }
            RowLayout {
                Layout.fillWidth: true
                visible: control.alphaEnabled
                CatLabel {
                    theme: control.theme
                    text: qsTr("Opacity")
                    Layout.preferredWidth: 60
                    elide: Text.ElideRight
                }
                CatSlider {
                    theme: control.theme
                    Layout.fillWidth: true
                    stepSize: 1 / 255
                    valueToolTipVisible: true
                    valueFormatter: n => Math.round(n * 100) + "%"
                    value: control.selectedColor.a
                    Accessible.name: qsTr("Opacity")
                    onMoved: control.chooseColor(Qt.rgba(control.selectedColor.r,
                                                         control.selectedColor.g,
                                                         control.selectedColor.b, value))
                }
            }
            CatTextField {
                objectName: "colorHexInput"
                Layout.fillWidth: true
                theme: control.theme
                text: control.selectedColor.toString()
                validator: RegularExpressionValidator {
                    regularExpression: /#[0-9a-fA-F]{6}(?:[0-9a-fA-F]{2})?/
                }
                Accessible.name: qsTr("Hex color")
                onEditingFinished: if (acceptableInput && text.toLowerCase() !== control.selectedColor.toString())
                                control.chooseColor(text)
            }
        }
    }
}
