// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit

CatPane {
    id: control
    property LedMixerRuntime runtime
    property alias preview: preview
    implicitWidth: 340
    contentItem: ColumnLayout {
        enabled: control.runtime !== null
        spacing: Theme.tokens.spacing
        CatLabel {text:qsTr("Output / Mixer");font.bold:true;font.pixelSize:20}
        LedPreview {id:preview;Layout.fillWidth:true;Layout.preferredHeight:235;source:control.runtime ? control.runtime.source : null}
        CatLabel {text:qsTr("Crossfader: %1% B").arg(Math.round((control.runtime ? control.runtime.crossfader : 0.5)*100))}
        RowLayout {
            Layout.fillWidth: true
            CatSlider {
                id: mixSlider
                objectName: "ledMixSlider"
                theme: control.theme
                Layout.fillWidth: true; from: 0; to: 1; stepSize: 0.01
                fillOrigin: 0.5; defaultValue: 0.5; defaultIndicatorVisible: true
                tickValues: [0, 0.5, 1]; tickLabelsVisible: true; valueToolTipVisible: true
                valueFormatter: n => n === 0 ? "A" : n === 1 ? "B" : qsTr("%1% B").arg(Math.round(n * 100))
                Accessible.name: qsTr("Crossfader")
                value: control.runtime ? control.runtime.crossfader : 0.5
                onMoved: control.runtime.crossfader = value
            }
            CatButton {
                objectName: "ledMixReset"
                theme: control.theme
                text: qsTr("Reset")
                onClicked: mixSlider.reset()
            }
        }
        CatLabel {text:qsTr("Master: %1%").arg(Math.round((control.runtime ? control.runtime.master : 1)*100))}
        CatSliderField {
            objectName: "ledMasterSlider"
            theme: control.theme
            Layout.fillWidth: true; from: 0; to: 100; stepSize: 1
            suffix: "%"; defaultValue: 100; resetVisible: true; tickInterval: 25
            inputWidth: 90; valueToolTipVisible: true
            Accessible.name: qsTr("Master brightness")
            value: (control.runtime ? control.runtime.master : 1) * 100
            onValueEdited: value => control.runtime.master = value / 100
        }
        Flow {
            Layout.fillWidth:true;spacing:8
            CatCheckBox {text:qsTr("Blackout");checked:control.runtime ? control.runtime.blackout : false;onToggled:control.runtime.blackout=checked}
            CatCheckBox {text:qsTr("Freeze mix");checked:control.runtime ? control.runtime.freeze : false;onToggled:control.runtime.freeze=checked}
        }
        CatLabel {Layout.fillWidth:true;wrapMode:Text.Wrap;text:qsTr("Freeze holds the mixed channels. Master and Blackout still affect output.");color:Theme.tokens.textSecondary}
        CatComboBox {
            Layout.fillWidth:true
            model:[qsTr("Working RGB"),qsTr("Working RGB + white"),qsTr("Working RGB + cold + warm white")]
            currentIndex:(control.runtime ? control.runtime.workingSchema : 5)-3
            onActivated:index=>control.runtime.workingSchema=index+3
        }
        CatComboBox {
            Layout.fillWidth:true
            model:[qsTr("Strict: matching schemas only"),qsTr("RGB expansion: extra whites = 0"),qsTr("Approximate white conversion")]
            currentIndex:control.runtime ? control.runtime.conversion : 0
            onActivated:index=>control.runtime.conversion=index
        }
        CatLabel {Layout.fillWidth:true;wrapMode:Text.Wrap;text:control.runtime ? control.runtime.lastError : "";visible:text.length>0}
    }
}
