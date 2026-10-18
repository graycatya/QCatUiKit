// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit

ColumnLayout {
    id: control
    signal selectComponent(string id)
    spacing: 24
    CatPane {
        Layout.fillWidth: true
        contentItem: ColumnLayout {
            spacing: 8
            CatLabel {
                text: qsTr("How the chart APIs fit together")
                font.bold: true
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
            CatLabel {
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                color: Theme.tokens.textSecondary
                text: qsTr("CatChart is the drawing container. CatChartAxis defines coordinates and scales. CatChartSeries / CatChartSeries3D provide data and styles. Explore them together in the examples below.")
            }
        }
    }
    Repeater {
        model: GalleryChartCatalog.groups
        ColumnLayout {
            id: section
            required property var modelData
            Layout.fillWidth: true
            spacing: 12
            CatLabel {
                text: section.modelData.title
                font.pixelSize: 20
                font.bold: true
                Layout.fillWidth: true
            }
            GridLayout {
                Layout.fillWidth: true
                columns: control.width >= 840 ? 3 : control.width >= 500 ? 2 : 1
                columnSpacing: 16
                rowSpacing: 16
                uniformCellWidths: true
                Repeater {
                    model: GalleryChartCatalog.entries.filter(entry => entry.group === section.modelData.id)
                    GalleryEntryCard {
                        required property var modelData
                        objectName: "chartEntry_" + modelData.id
                        entry: modelData
                        Layout.fillWidth: true
                        onClicked: control.selectComponent(modelData.id)
                    }
                }
            }
        }
    }
}
