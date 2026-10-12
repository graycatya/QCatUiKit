// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit

CatPane {
    id: control
    required property GalleryDataDemo demo
    Layout.fillWidth: true
    function countText(value: real): string {
        return value.toLocaleString(Qt.locale(GalleryLanguage.localeName), "f", 0);
    }
    contentItem: ColumnLayout {
        spacing: 12
        CatLabel { text: qsTr("Continuous stress test"); font.bold: true }
        Flow {
            Layout.fillWidth: true
            spacing: 12
            ColumnLayout {
                width: control.availableWidth < 560 ? control.availableWidth : (control.availableWidth - 24) / 3
                CatLabel { text: qsTr("Maximum operations per batch"); wrapMode: Text.Wrap; Layout.fillWidth: true }
                CatSpinBox {
                    objectName: "dataBatchSize"
                    Layout.fillWidth: true
                    from: 1; to: control.demo.maximumBatchSize
                    value: control.demo.batchSize
                    editable: true; enabled: !control.demo.running
                    onValueModified: control.demo.batchSize = value
                    Accessible.name: qsTr("Maximum operations per batch")
                }
            }
            ColumnLayout {
                width: control.availableWidth < 560 ? control.availableWidth : (control.availableWidth - 24) / 3
                CatLabel { text: qsTr("Data limit"); wrapMode: Text.Wrap; Layout.fillWidth: true }
                CatSpinBox {
                    objectName: "dataCapacity"
                    Layout.fillWidth: true
                    from: Math.max(1, control.demo.count); to: control.demo.maximumCapacity
                    value: control.demo.capacity
                    editable: true; enabled: !control.demo.running
                    onValueModified: control.demo.capacity = value
                    Accessible.name: qsTr("Data limit")
                }
            }
            ColumnLayout {
                width: control.availableWidth < 560 ? control.availableWidth : (control.availableWidth - 24) / 3
                CatLabel { text: qsTr("Update interval (ms)"); wrapMode: Text.Wrap; Layout.fillWidth: true }
                CatSpinBox {
                    objectName: "dataInterval"
                    Layout.fillWidth: true
                    from: 16; to: 2000; value: control.demo.interval
                    editable: true; enabled: !control.demo.running
                    onValueModified: control.demo.interval = value
                    Accessible.name: qsTr("Update interval (ms)")
                }
            }
        }
        CatComboBox {
            objectName: "dataWorkload"
            Layout.fillWidth: true
            visible: control.demo.mode !== 2
            enabled: !control.demo.running
            model: [qsTr("Continuous additions"), qsTr("Mixed additions, updates and deletions"), qsTr("Continuous updates")]
            currentIndex: control.demo.workload
            onActivated: index => control.demo.workload = index
            Accessible.name: qsTr("Stress workload")
        }
        Flow {
            Layout.fillWidth: true
            spacing: 8
            CatButton {
                objectName: "dataLargePreset"
                text: control.demo.mode === 0 ? qsTr("100,000-row preset") : qsTr("10,000-message preset")
                enabled: !control.demo.running
                onClicked: control.demo.configureLargeData()
            }
            CatButton {
                objectName: "dataStressButton"
                text: control.demo.running ? qsTr("Stop stress test") : qsTr("Start stress test")
                primary: true
                onClicked: control.demo.running ? control.demo.stopStress() : control.demo.startStress()
            }
        }
        CatLabel {
            objectName: "dataStressStatistics"
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            text: qsTr("%1 s · %2 batches · %3 operations/s\nAdded: %4 · updated: %5 · removed: %6 · rejected blank submissions: %7\nLast batch: %8 operations · slowest data batch: %9 ms")
                .arg(control.demo.elapsedSeconds.toFixed(1)).arg(control.countText(control.demo.batches))
                .arg(control.demo.operationsPerSecond.toFixed(1)).arg(control.countText(control.demo.added))
                .arg(control.countText(control.demo.changed)).arg(control.countText(control.demo.removed))
                .arg(control.countText(control.demo.rejected)).arg(control.countText(control.demo.lastBatchCount))
                .arg(control.demo.maximumBatchMs.toFixed(2))
        }
        CatLabel {
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            color: Theme.tokens.textSecondary
            text: control.demo.mode === 0
                ? qsTr("All rows remain in the table model. At the limit, additions update existing rows. Mixed mode adds four batches, updates one, then deletes one.")
                : control.demo.mode === 1
                  ? qsTr("All retained messages remain in the model. At the limit, new messages remove the oldest. Mixed mode adds four batches, changes delivery states once, then deletes one batch.")
                  : qsTr("Calls the real composer submit action, with a blank attempt every fifth submission. The draft is restored when stopped. At the limit, the oldest history is removed.")
        }
        CatLabel {
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            color: Theme.tokens.textSecondary
            text: qsTr("Presets configure the limit; Start fills it gradually. Batches yield after about 12 ms of data work. Clear data before lowering the limit below the current count. Leaving or hiding the page stops the test. Statistics cover data operations, not rendering FPS or network performance.")
        }
    }
}
