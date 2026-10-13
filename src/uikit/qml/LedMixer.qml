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
        CatSlider {objectName:"ledMixSlider";Layout.fillWidth:true;from:0;to:1;value:control.runtime ? control.runtime.crossfader : 0.5;onMoved:control.runtime.crossfader=value}
        CatLabel {text:qsTr("Master: %1%").arg(Math.round((control.runtime ? control.runtime.master : 1)*100))}
        CatSlider {objectName:"ledMasterSlider";Layout.fillWidth:true;from:0;to:1;value:control.runtime ? control.runtime.master : 1;onMoved:control.runtime.master=value}
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
