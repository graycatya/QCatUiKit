// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit

ColumnLayout {
    id: control
    objectName: "ledPreviewDemo"
    readonly property LedProject dataController: project
    property bool synchronizePreviews: false
    property bool applyingPreview: false
    spacing: Theme.tokens.spacing
    LedProject {id:project;objectName:"ledProjectData"}
    LedStorageRepository {id:jsonRepository}
    function syncAppearance(): void {
        if(!synchronizePreviews || applyingPreview)return;
        applyingPreview=true;const settings=a.preview.settings();mix.preview.applySettings(settings);b.preview.applySettings(settings);applyingPreview=false;
    }
    Component.onCompleted: {
        project.repository.populateExamples();
        jsonRepository.importJson(project.repository.exportJson());
        const storage=project.repository.storages[0].id;
        project.deckA.browseStorage(storage);project.deckB.browseStorage(storage);
        project.deckA.activatePlace(storage,project.repository.places(storage)[0].id);
        project.deckB.activatePlace(storage,project.repository.places(storage)[2].id);
        project.mixer.conversion=Lighting.ApproximateWhite;
        project.output.applyProfiles([
            {id:"rgb-group",deviceId:"memory",portId:"RGB",schema:3,channelOrder:["G","R","B"],firstPoint:0,pointCount:1,startChannel:1,conversion:2,whiteKelvin:4000,coldKelvin:6500,warmKelvin:2700},
            {id:"rgbw-group",deviceId:"memory",portId:"RGBW",schema:4,channelOrder:["R","G","B","W"],firstPoint:1,pointCount:1,startChannel:5,conversion:2,whiteKelvin:4000,coldKelvin:6500,warmKelvin:2700},
            {id:"cct-group",deviceId:"memory",portId:"RGB+CCT",schema:5,channelOrder:["R","G","B","CW","WW"],firstPoint:2,pointCount:1,startChannel:9,conversion:0,whiteKelvin:4000,coldKelvin:6500,warmKelvin:2700}
        ]);
    }
    Connections {
        target:a.preview
        function onSettingsChanged(){control.syncAppearance()}
        function onViewModeChanged(){control.syncAppearance()}
        function onPointSizeChanged(){control.syncAppearance()}
        function onPreviewBrightnessChanged(){control.syncAppearance()}
    }
    Connections {
        target:project
        function onPreferencesChanged(){
            const p=project.previewPreferences;
            if(p.length===3){a.preview.applySettings(p[0]);mix.preview.applySettings(p[1]);b.preview.applySettings(p[2]);}
        }
    }
    Flow {
        Layout.fillWidth:true;spacing:8
        CatComboBox {
            objectName:"ledLayoutPicker"
            model:[qsTr("64 × 32 matrix"),qsTr("16 × 16 × 8 matrix"),qsTr("Ring"),qsTr("Disk"),qsTr("Sphere surface"),qsTr("Empty layout"),qsTr("Single LED"),qsTr("100,000 LEDs")]
            onActivated:index=>{
                const c=project.layout.defaultConfiguration();
                if(index===1){c.type="Matrix3D";c.countX=16;c.countY=16;c.countZ=8;}
                else if(index===2)c.type="Ring";
                else if(index===3)c.type="Disk";
                else if(index===4){c.type="SphereSurface";c.count=4096;}
                else if(index===5)c.countX=0;
                else if(index===6){c.countX=1;c.countY=1;}
                else if(index===7){c.countX=400;c.countY=250;}
                project.layout.apply(c);
            }
        }
        CatButton {text:qsTr("Layout…");onClicked:layoutDialog.open()}
        CatButton {text:qsTr("Profiles / patch…");onClicked:outputDialog.open()}
        CatButton {text:qsTr("Save / restore project…");onClicked:{project.previewPreferences=[a.preview.settings(),mix.preview.settings(),b.preview.settings()];projectDialog.open();}}
        CatCheckBox {text:qsTr("Sync appearance from A");checked:control.synchronizePreviews;onToggled:{control.synchronizePreviews=checked;control.syncAppearance();}}
        CatButton {text:qsTr("Fit all");onClicked:{a.preview.resetView();mix.preview.resetView();b.preview.resetView();}}
    }
    CatLabel {
        Layout.fillWidth:true;wrapMode:Text.Wrap
        text:qsTr("%1 LEDs · layout revision %2%3").arg(project.layout.pointCount).arg(project.layout.revision).arg(project.layout.busy?qsTr(" · generating…"):"")
    }
    CatLabel {Layout.fillWidth:true;wrapMode:Text.Wrap;visible:text.length>0;text:project.layout.lastError}
    GridLayout {
        Layout.fillWidth:true
        columns:width<920?1:3;columnSpacing:Theme.tokens.spacing;rowSpacing:Theme.tokens.spacing
        LedDeck {
            id:a
            Layout.fillWidth:true;Layout.preferredWidth:1;Layout.minimumWidth:0;Layout.alignment:Qt.AlignTop
            title:qsTr("Deck A");runtime:project.deckA
            repositories:[project.repository,jsonRepository];repositoryNames:[qsTr("Memory repository"),qsTr("JSON working repository")];writableRepository:project.repository
            preview.objectName:"ledDeckA"
        }
        ColumnLayout {
            Layout.fillWidth:true;Layout.preferredWidth:1;Layout.minimumWidth:0;Layout.alignment:Qt.AlignTop
            LedMixer {id:mix;Layout.fillWidth:true;runtime:project.mixer;preview.objectName:"ledOutput"}
            CatPane {
                Layout.fillWidth:true
                contentItem:ColumnLayout {
                    CatLabel {text:qsTr("Memory channel monitor");font.bold:true}
                    CatLabel {Layout.fillWidth:true;wrapMode:Text.Wrap;text:qsTr("%1 received · frame %2 · mapping %3").arg(project.memoryBackend.receivedFrames).arg(project.memoryBackend.frameId).arg(project.memoryBackend.mappingRevision)}
                    CatLabel {Layout.fillWidth:true;wrapMode:Text.Wrap;text:{const frame=project.memoryBackend.frameId;return qsTr("Output point 0: %1").arg(project.mixer.channelSample().join(", "));}}
                    Repeater {
                        model:project.memoryBackend.ports
                        delegate:CatLabel {
                            required property var modelData
                            Layout.fillWidth:true;wrapMode:Text.Wrap
                            text:qsTr("%1 / %2 · %3 channels at address %4\n%5").arg(modelData.deviceId).arg(modelData.portId).arg(modelData.channelCount).arg(modelData.startChannel).arg(modelData.values.join(", "))
                        }
                    }
                    CatCheckBox {text:qsTr("Enable device branch");checked:project.output.enabled;onToggled:project.output.enabled=checked}
                    CatCheckBox {text:qsTr("Repeat held frames");checked:project.output.repeatFrames;onToggled:project.output.repeatFrames=checked}
                    CatLabel {text:qsTr("Send interval (ms)")}
                    CatNumberBox {Layout.fillWidth:true;minimum:4;maximum:10000;decimals:0;value:project.output.interval;onValueEdited:value=>project.output.interval=value}
                    CatLabel {Layout.fillWidth:true;wrapMode:Text.Wrap;text:project.output.lastError;visible:text.length>0}
                    CatLabel {Layout.fillWidth:true;wrapMode:Text.Wrap;color:Theme.tokens.textSecondary;text:qsTr("Memory verification only. No hardware protocol is connected. Preview settings do not change these values.")}
                }
            }
        }
        LedDeck {
            id:b
            Layout.fillWidth:true;Layout.preferredWidth:1;Layout.minimumWidth:0;Layout.alignment:Qt.AlignTop
            title:qsTr("Deck B");runtime:project.deckB
            repositories:[project.repository,jsonRepository];repositoryNames:[qsTr("Memory repository"),qsTr("JSON working repository")];writableRepository:project.repository
            preview.objectName:"ledDeckB"
        }
    }
    CatLabel {Layout.fillWidth:true;wrapMode:Text.Wrap;color:Theme.tokens.textSecondary;text:qsTr("Left drag: pan in 2D / orbit in 3D · Middle drag: pan · Wheel: zoom · Double click: fit · Right click: preview settings. Browsing a storage does not change playback.")}
    LedLayoutSettings {id:layoutDialog;layoutModel:project.layout}
    LedOutputSettings {id:outputDialog;output:project.output}
    property CatDialog projectPopup: CatDialog {
        id:projectDialog
        title:qsTr("Versioned project JSON")
        implicitWidth:Math.min(720,parent ? parent.width-24 : 720)
        implicitHeight:Math.min(700,parent ? parent.height-24 : 700)
        onOpened:projectJson.text=project.exportJson()
        contentItem:ColumnLayout {
            CatLabel {Layout.fillWidth:true;wrapMode:Text.Wrap;text:qsTr("Restores layout, profiles, presets, working copies and preview preferences. Playback resumes paused at t=0; Blackout and Freeze reset.")}
            CatScrollView {Layout.fillWidth:true;Layout.fillHeight:true;CatTextArea {id:projectJson;selectByMouse:true;wrapMode:TextEdit.NoWrap}}
            CatTextField {id:projectPath;Layout.fillWidth:true;placeholderText:qsTr("Local file URL, e.g. file:///tmp/led-project.json")}
            CatLabel {Layout.fillWidth:true;wrapMode:Text.Wrap;text:project.lastError}
        }
        footer:Flow {
            spacing:6;padding:12
            CatButton {text:qsTr("Refresh JSON");onClicked:projectJson.text=project.exportJson()}
            CatButton {text:qsTr("Restore JSON");onClicked:project.importJson(projectJson.text)}
            CatButton {text:qsTr("Save file");onClicked:project.saveFile(projectPath.text)}
            CatButton {text:qsTr("Load file");onClicked:project.loadFile(projectPath.text)}
            CatButton {text:qsTr("Close");onClicked:projectDialog.close()}
        }
    }
}
