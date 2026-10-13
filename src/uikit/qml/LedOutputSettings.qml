// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit

CatDialog {
    id: control
    property LedOutput output
    property var draft: []
    property int selected: 0
    readonly property var profile: draft[selected] ?? ({})
    title:qsTr("Fixture profiles and patch")
    width:Math.min(590,parent ? parent.width-24 : 590)
    height:Math.min(760,parent ? parent.height-24 : 760)
    function change(key:string,value:var):void {const list=JSON.parse(JSON.stringify(draft));if(list[selected]){list[selected][key]=value;draft=list;}}
    onOpened:{draft=JSON.parse(JSON.stringify(output.profiles));selected=0}
    contentItem:CatScrollView {
        contentWidth:availableWidth
        ColumnLayout {
            width:parent.width;spacing:Theme.tokens.spacing
            CatComboBox {Layout.fillWidth:true;model:control.draft;textRole:"id";currentIndex:control.selected;onActivated:index=>control.selected=index}
            Flow {
                Layout.fillWidth:true;spacing:8
                CatButton {text:qsTr("Add group");onClicked:{const list=control.draft.slice();list.push({id:"group-"+Date.now(),deviceId:"memory-device",portId:"port-"+list.length,schema:3,channelOrder:["R","G","B"],firstPoint:0,pointCount:1,startChannel:1,conversion:2,whiteKelvin:4000,coldKelvin:6500,warmKelvin:2700});control.draft=list;control.selected=list.length-1}}
                CatButton {text:qsTr("Remove group");enabled:control.draft.length>0;onClicked:{const list=control.draft.slice();list.splice(control.selected,1);control.draft=list;control.selected=Math.max(0,control.selected-1)}}
            }
            Repeater {
                model:[{key:"id",label:qsTr("Group ID")},{key:"deviceId",label:qsTr("Device ID")},{key:"portId",label:qsTr("Port ID")}]
                delegate:ColumnLayout {
                    id:entry
                    required property var modelData
                    Layout.fillWidth:true
                    CatLabel {text:entry.modelData.label}
                    CatTextField {Layout.fillWidth:true;text:control.profile[entry.modelData.key] ?? "";onTextEdited:control.change(entry.modelData.key,text)}
                }
            }
            CatComboBox {
                Layout.fillWidth:true
                model:[qsTr("RGB"),qsTr("RGB + white"),qsTr("RGB + cold white + warm white")]
                currentIndex:(control.profile.schema ?? 3)-3
                onActivated:index=>{control.change("schema",index+3);control.change("channelOrder",index===0?["R","G","B"]:index===1?["R","G","B","W"]:["R","G","B","CW","WW"])}
            }
            CatLabel {text:qsTr("Channel order (comma separated)")}
            CatTextField {Layout.fillWidth:true;text:(control.profile.channelOrder ?? []).join(",");onTextEdited:control.change("channelOrder",text.split(",").map(s=>s.trim()))}
            Repeater {
                model:[{key:"firstPoint",label:qsTr("First logical point (0-based)"),min:0,max:1000000},
                    {key:"pointCount",label:qsTr("Point count"),min:0,max:1000000},
                    {key:"startChannel",label:qsTr("Start address (1-based)"),min:1,max:65536},
                    {key:"whiteKelvin",label:qsTr("Single white reference (K)"),min:1000,max:20000},
                    {key:"coldKelvin",label:qsTr("Cold white reference (K)"),min:1000,max:20000},
                    {key:"warmKelvin",label:qsTr("Warm white reference (K)"),min:1000,max:20000}]
                delegate:RowLayout {
                    id:number
                    required property var modelData
                    Layout.fillWidth:true
                    CatLabel {Layout.fillWidth:true;wrapMode:Text.Wrap;text:number.modelData.label}
                    CatNumberBox {Layout.preferredWidth:130;decimals:0;minimum:number.modelData.min;maximum:number.modelData.max;value:control.profile[number.modelData.key] ?? number.modelData.min;onValueEdited:value=>control.change(number.modelData.key,value)}
                }
            }
            CatComboBox {Layout.fillWidth:true;model:[qsTr("Strict"),qsTr("Extra whites = 0"),qsTr("Approximate white mapping")];currentIndex:control.profile.conversion ?? 0;onActivated:index=>control.change("conversion",index)}
            CatLabel {Layout.fillWidth:true;wrapMode:Text.Wrap;color:Theme.tokens.textSecondary;text:qsTr("Approximation: CW/WW add with reference colors for RGB; CW + WW clamp to W. This is not calibrated fixture colorimetry. Memory output does not send to hardware.")}
            CatLabel {Layout.fillWidth:true;wrapMode:Text.Wrap;text:control.output ? control.output.lastError : ""}
        }
    }
    footer:Flow {
        spacing:8;padding:12
        CatButton {text:qsTr("Cancel");onClicked:control.close()}
        CatButton {text:qsTr("Apply");primary:true;onClicked:if(control.output.applyProfiles(control.draft))control.close()}
    }
}
