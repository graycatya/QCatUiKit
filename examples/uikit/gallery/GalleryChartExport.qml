// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QtQuick.Dialogs as Dialogs
import QtCore
import QCatUiKit

ColumnLayout {
    id: control
    required property CatChart chart
    property string statusText: ""
    property bool failed: false
    property bool doubleSize: false
    onChartChanged: { statusText = ""; failed = false; }
    spacing: 8

    function saveImage(destination: url): bool {
        if (!chart) return false;
        statusText = qsTr("Exporting image…");
        failed = false;
        return chart.exportImage(destination, doubleSize
            ? Qt.size(Math.ceil(chart.width * 2), Math.ceil(chart.height * 2))
            : Qt.size(-1, -1));
    }

    Flow {
        Layout.fillWidth: true
        spacing: 12
        CatButton {
            objectName: "chartExportButton"
            text: control.chart?.exportingImage ? qsTr("Exporting image…") : qsTr("Export PNG…")
            enabled: control.chart !== null && !control.chart.exportingImage
            onClicked: chooser.open()
        }
        CatCheckBox {
            objectName: "chartExportDoubleSize"
            text: qsTr("2× chart size")
            checked: control.doubleSize
            onToggled: control.doubleSize = checked
        }
    }
    CatLabel {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        color: Theme.tokens.textSecondary
        text: qsTr("Save the current chart, including its title, axes, legend and overlays. Larger images scale the current view.")
    }
    CatLabel {
        objectName: "chartExportStatus"
        Layout.fillWidth: true
        visible: text.length > 0
        wrapMode: Text.Wrap
        color: control.failed ? Theme.tokens.danger : Theme.tokens.textSecondary
        text: control.statusText
    }
    Connections {
        target: control.chart
        function onImageExportFinished(destination: url, success: bool, error: string) {
            control.failed = !success;
            control.statusText = success ? qsTr("Saved to %1").arg(destination.toString())
                                         : qsTr("Export failed: %1").arg(error);
        }
    }
    Dialogs.FileDialog {
        id: chooser
        title: qsTr("Export chart image")
        fileMode: Dialogs.FileDialog.SaveFile
        nameFilters: [qsTr("PNG image (*.png)")]
        defaultSuffix: "png"
        currentFolder: StandardPaths.writableLocation(StandardPaths.PicturesLocation)
        onAccepted: control.saveImage(selectedFile)
    }
}
