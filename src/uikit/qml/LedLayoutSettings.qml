// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit

CatDialog {
    id: control
    property LedLayoutModel layoutModel
    property var draft: ({})
    property string error: ""
    readonly property var types: ["Matrix2D","Matrix3D","Ring","Disk","SphereSurface","Custom"]
    title: qsTr("Shared LED layout")
    width: Math.min(540,parent ? parent.width-24 : 540)
    height: Math.min(720,parent ? parent.height-24 : 720)
    function change(key: string,value: var): void { const next=Object.assign({},draft);next[key]=value;draft=next; }
    onOpened: { draft=Object.assign({},layoutModel.configuration);error=""; }
    contentItem: CatScrollView {
        contentWidth: availableWidth
        ColumnLayout {
            width: parent.width
            spacing: Theme.tokens.spacing
            CatComboBox {
                Layout.fillWidth: true
                model: [qsTr("2D matrix"),qsTr("3D matrix"),qsTr("Ring"),qsTr("Disk"),qsTr("Sphere surface"),qsTr("Custom XYZ")]
                currentIndex: control.types.indexOf(control.draft.type)
                onActivated: index => control.change("type",control.types[index])
            }
            Repeater {
                model: [
                    {key:"countX",label:qsTr("Count X"),min:0,max:1000000,types:["Matrix2D","Matrix3D"]},
                    {key:"countY",label:qsTr("Count Y"),min:0,max:1000000,types:["Matrix2D","Matrix3D"]},
                    {key:"countZ",label:qsTr("Count Z"),min:0,max:1000000,types:["Matrix3D"]},
                    {key:"spacingX",label:qsTr("Layout spacing X"),min:0.0001,max:1000000,types:["Matrix2D","Matrix3D"]},
                    {key:"spacingY",label:qsTr("Layout spacing Y"),min:0.0001,max:1000000,types:["Matrix2D","Matrix3D"]},
                    {key:"spacingZ",label:qsTr("Layout spacing Z"),min:0.0001,max:1000000,types:["Matrix3D"]},
                    {key:"count",label:qsTr("Point count"),min:0,max:1000000,types:["Ring","SphereSurface"]},
                    {key:"radius",label:qsTr("Radius"),min:0.0001,max:1000000,types:["Ring","Disk","SphereSurface"]},
                    {key:"startAngle",label:qsTr("Start angle (degrees)"),min:-360000,max:360000,types:["Ring","Disk"]},
                    {key:"radialLayers",label:qsTr("Rings outside center"),min:0,max:1000000,types:["Disk"]},
                    {key:"angularPoints",label:qsTr("Points per ring"),min:1,max:1000000,types:["Disk"]}]
                delegate: RowLayout {
                    id: field
                    required property var modelData
                    Layout.fillWidth: true
                    visible: modelData.types.indexOf(control.draft.type)>=0
                    CatLabel { Layout.fillWidth: true; text: field.modelData.label; wrapMode: Text.Wrap }
                    CatNumberBox {
                        Layout.preferredWidth: 145
                        minimum: field.modelData.min;maximum: field.modelData.max
                        decimals: field.modelData.min===0.0001 ? 4 : 0
                        value: control.draft[field.modelData.key] ?? 0
                        onValueEdited: value => control.change(field.modelData.key,value)
                    }
                }
            }
            CatCheckBox { text: qsTr("Clockwise"); visible: control.draft.type==="Ring" || control.draft.type==="Disk"; checked: control.draft.clockwise ?? false; onToggled: control.change("clockwise",checked) }
            CatLabel { Layout.fillWidth: true; wrapMode: Text.Wrap; visible: control.draft.type==="Custom"; text: qsTr("XYZ triples as JSON; custom layouts have no implicit grid dimensions.") }
            CatTextArea {
                id: positions
                Layout.fillWidth: true; Layout.preferredHeight: 150
                visible: control.draft.type==="Custom"
                text: JSON.stringify(control.draft.positions ?? [])
            }
            CatLabel {
                Layout.fillWidth: true; wrapMode: Text.Wrap
                text: control.draft.type?.startsWith("Matrix") ? qsTr("Draft: %1 × %2 × %3 = %4 LEDs").arg(control.draft.countX).arg(control.draft.countY).arg(control.draft.type==="Matrix2D"?1:control.draft.countZ).arg(control.draft.countX*control.draft.countY*(control.draft.type==="Matrix2D"?1:control.draft.countZ))
                    : control.draft.type==="Disk" ? qsTr("Draft: 1 center + %1 rings × %2 = %3 LEDs").arg(control.draft.radialLayers).arg(control.draft.angularPoints).arg(1+control.draft.radialLayers*control.draft.angularPoints) : ""
            }
            CatLabel { Layout.fillWidth: true; wrapMode: Text.Wrap; text: qsTr("Origin is centered. Matrix index = x + countX × (y + countY × z); rows run downward. Apply updates all channels to one shared layout."); color: Theme.tokens.textSecondary }
            CatLabel { Layout.fillWidth: true; wrapMode: Text.Wrap; text: control.error || (control.layoutModel ? control.layoutModel.lastError : ""); visible: text.length>0 }
        }
    }
    footer: Flow {
        padding: Theme.tokens.padding;spacing: Theme.tokens.spacing
        CatButton { text: qsTr("Defaults"); onClicked: control.draft=control.layoutModel.defaultConfiguration() }
        CatButton { text: qsTr("Cancel"); onClicked: control.close() }
        CatButton {
            text: qsTr("Apply");primary: true
            onClicked: {
                if(control.draft.type==="Custom") {
                    try { control.change("positions",JSON.parse(positions.text)); }
                    catch(e) { control.error=qsTr("Invalid XYZ JSON"); return; }
                }
                control.error=control.layoutModel.validateConfiguration(control.draft);
                if(!control.error) { control.layoutModel.apply(control.draft);control.close(); }
            }
        }
    }
}
