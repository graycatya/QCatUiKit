// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit

CatPane {
    id: control
    property string title: qsTr("Deck")
    property LedDeckRuntime runtime
    property var repositories: []
    property var repositoryNames: []
    property LedStorageRepository writableRepository
    property alias preview: preview
    property alias storageBrowser: browser
    implicitWidth: 390
    contentItem: ColumnLayout {
        enabled: control.runtime !== null
        spacing: Theme.tokens.spacing
        RowLayout {
            Layout.fillWidth: true
            CatLabel { text: control.title; font.bold: true; font.pixelSize: 20 }
            CatLabel { Layout.fillWidth: true; horizontalAlignment: Text.AlignRight; elide: Text.ElideRight; text: control.runtime?.loading ? qsTr("Loading…") : control.runtime?.workingPlace.name ?? qsTr("No preset") }
        }
        LedPreview { id: preview; Layout.fillWidth: true; Layout.preferredHeight: 235; source: control.runtime ? control.runtime.source : null }
        Flow {
            Layout.fillWidth: true; spacing: 5
            CatButton { text: qsTr("Previous"); onClicked: control.runtime.previous() }
            CatButton { text: control.runtime?.playing ? qsTr("Pause") : qsTr("Play"); enabled: !!control.runtime?.workingPlace.effectId; primary: true; onClicked: control.runtime.playing ? control.runtime.pause() : control.runtime.play() }
            CatButton { text: qsTr("Stop"); onClicked: control.runtime.stop() }
            CatButton { text: qsTr("Restart"); enabled: !!control.runtime?.workingPlace.effectId; onClicked: control.runtime.restart() }
            CatButton { text: qsTr("Next"); onClicked: control.runtime.next() }
            CatButton { text: qsTr("Unload"); onClicked: control.runtime.unload() }
        }
        RowLayout {
            Layout.fillWidth: true
            CatLabel { text: qsTr("Deck speed") }
            CatNumberBox { Layout.fillWidth: true; minimum: 0; maximum: 8; value: control.runtime ? control.runtime.speed : 1; onValueEdited: value => control.runtime.speed=value }
        }
        CatLabel { text: qsTr("Deck brightness: %1%").arg(Math.round((control.runtime ? control.runtime.brightness : 1)*100)) }
        CatSlider { Layout.fillWidth: true;from:0;to:1;value: control.runtime ? control.runtime.brightness : 1;onMoved:control.runtime.brightness=value }
        CatComboBox {
            Layout.fillWidth: true
            visible: control.repositories.length>1
            model: control.repositoryNames
            onActivated: index => control.runtime.repository=control.repositories[index]
        }
        LedStorageBrowser {id:browser;Layout.fillWidth:true;runtime:control.runtime;writableRepository:control.writableRepository}
        CatLabel {
            Layout.fillWidth:true;wrapMode:Text.Wrap;color:Theme.tokens.textSecondary
            text: control.runtime && control.runtime.activePlace
                ? qsTr("Active: %1\n%2").arg(control.runtime.workingPlace.name ?? "").arg(control.runtime.activeLocation) : qsTr("Unsaved working preset")
        }
        CatLabel { Layout.fillWidth:true;wrapMode:Text.Wrap;text:control.runtime?.workingPlace.description ?? "";visible:text.length>0 }
        CatComboBox {
            id: effects
            Layout.fillWidth:true
            model: control.runtime && control.runtime.registry ? control.runtime.registry.effects : []
            textRole:"name"
            displayText: {
                const p=control.runtime?.workingPlace;
                return control.runtime?.registry && p?.effectId ? String(control.runtime.registry.effect(p.providerId,p.effectId).name) : qsTr("Select effect");
            }
            onActivated: index => {
                const e=model[index];control.runtime.selectEffect(e.providerId,e.id,control.runtime.workingPlace.schema ?? 5);
            }
        }
        CatComboBox {
            Layout.fillWidth:true
            model:[qsTr("RGB"),qsTr("RGB + white"),qsTr("RGB + cold white + warm white")]
            currentIndex:(control.runtime?.workingPlace.schema ?? 5)-3
            onActivated:index=>control.runtime.editPlace({schema:index+3})
        }
        RowLayout {
            Layout.fillWidth:true
            CatLabel {text:qsTr("Place speed")}
            CatNumberBox {Layout.fillWidth:true;minimum:0;maximum:8;value:control.runtime?.workingPlace.speed ?? 1;onValueEdited:value=>control.runtime.editPlace({speed:value})}
        }
        CatLabel {text:qsTr("Place brightness")}
        CatSlider {Layout.fillWidth:true;from:0;to:1;value:control.runtime?.workingPlace.brightness ?? 1;onMoved:control.runtime.editPlace({brightness:value})}
        Repeater {
            model:control.runtime ? control.runtime.parameterDefinitions : []
            delegate: ColumnLayout {
                id: parameter
                required property var modelData
                Layout.fillWidth:true
                CatLabel {text:parameter.modelData.name+": "+Number(control.runtime.workingPlace.parameters?.[parameter.modelData.id] ?? parameter.modelData.default).toFixed(2)}
                CatSlider {
                    Layout.fillWidth:true
                    from:parameter.modelData.min;to:parameter.modelData.max
                    stepSize:parameter.modelData.id==="channel" ? 1 : 0
                    value:control.runtime.workingPlace.parameters?.[parameter.modelData.id] ?? parameter.modelData.default
                    onMoved:control.runtime.setParameter(parameter.modelData.id,value)
                }
            }
        }
        CatLabel {Layout.fillWidth:true;wrapMode:Text.Wrap;text:control.runtime ? control.runtime.lastError : "";visible:text.length>0}
    }
}
