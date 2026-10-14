// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit

CatDialog {
    id: control
    property LedPreviewRenderItem preview
    property var draft: ({})
    property string validationError: ""
    title: qsTr("Preview settings")
    width: Math.min(560, parent ? parent.width - 24 : 560)
    height: Math.min(750, parent ? parent.height - 24 : 750)
    function snapshot(): var {
        if (!preview) return {};
        return {version: 1, camera: preview.camera(), viewMode: preview.viewMode, pointShape: preview.pointShape,
            sizeMode: preview.sizeMode, pointSize: preview.pointSize, sceneSize: preview.sceneSize,
            spacingX: preview.displaySpacing.x, spacingY: preview.displaySpacing.y,
            spacingZ: preview.displaySpacing.z, gapX: preview.gapX, gapY: preview.gapY,
            sliceAxis: preview.sliceAxis, slicesVertical: preview.slicesVertical,
            previewBrightness: preview.previewBrightness, previewEnabled: preview.previewEnabled,
            maximumFps: preview.maximumFps, showStatistics: preview.showStatistics,
            showGrid: preview.showGrid, showAxes: preview.showAxes,
            backgroundColor: String(preview.backgroundColor), backgroundImage: preview.backgroundImage};
    }
    function defaults(): var {
        return {version:1,camera:{yaw:0,pitch:0,zoom:1,panX:0,panY:0},viewMode:1,pointShape:1,sizeMode:0,pointSize:6,sceneSize:0.8,
            spacingX:1,spacingY:1,spacingZ:1,gapX:0,gapY:0,sliceAxis:0,slicesVertical:false,
            previewBrightness:1,previewEnabled:true,maximumFps:60,showStatistics:true,
            showGrid:false,showAxes:false,backgroundColor:"#090d15",backgroundImage:""};
    }
    function update(key: string, value: var): void { const next=Object.assign({}, draft); next[key]=value; draft=next; }
    function applySettings(s: var): bool {
        if (!preview || !s || s.version !== 1) { validationError=qsTr("Unsupported preview settings version"); return false; }
        const ranges={viewMode:[0,1],pointShape:[0,3],sizeMode:[0,1],pointSize:[1,128],sceneSize:[0.0001,1000000],
            spacingX:[0.1,10],spacingY:[0.1,10],spacingZ:[0.1,10],gapX:[0,0.95],gapY:[0,0.95],sliceAxis:[0,3],
            previewBrightness:[0,1],maximumFps:[1,240]};
        for (const key in ranges) {
            if (!isFinite(s[key]) || s[key]<ranges[key][0] || s[key]>ranges[key][1]) {
                validationError=qsTr("%1: value outside allowed range").arg(key); return false;
            }
        }
        if (!/^#[0-9a-fA-F]{6}([0-9a-fA-F]{2})?$/.test(s.backgroundColor)) {
            validationError=qsTr("Background: use #RRGGBB or #AARRGGBB"); return false;
        }
        const camera=s.camera;
        if(!camera || !isFinite(camera.yaw) || !isFinite(camera.pitch) || Math.abs(camera.pitch)>89
            || !isFinite(camera.zoom) || camera.zoom<0.2 || camera.zoom>100 || !isFinite(camera.panX)
            || !isFinite(camera.panY) || Math.abs(camera.panX)>100 || Math.abs(camera.panY)>100) {
            validationError=qsTr("Invalid camera settings"); return false;
        }
        preview.viewMode=s.viewMode; preview.pointShape=s.pointShape; preview.sizeMode=s.sizeMode;
        preview.pointSize=s.pointSize; preview.sceneSize=s.sceneSize;
        preview.displaySpacing=Qt.vector3d(s.spacingX,s.spacingY,s.spacingZ);
        preview.gapX=s.gapX; preview.gapY=s.gapY; preview.sliceAxis=s.sliceAxis;
        preview.slicesVertical=s.slicesVertical; preview.previewBrightness=s.previewBrightness;
        preview.previewEnabled=s.previewEnabled; preview.maximumFps=s.maximumFps;
        preview.showStatistics=s.showStatistics; preview.showGrid=s.showGrid; preview.showAxes=s.showAxes;
        preview.backgroundColor=s.backgroundColor; preview.backgroundImage=s.backgroundImage;
        preview.setCamera(camera);
        validationError=""; return true;
    }
    function commitFields(): bool {
        for (let i=0;i<fields.count;++i) {
            const editor=(fields.itemAt(i) as NumberField).editor;
            if (editor.visible && !editor.commitText()) { editor.forceActiveFocus(); return false; }
        }
        return true;
    }
    function resetFields(): void {
        for (let i=0;i<fields.count;++i) (fields.itemAt(i) as NumberField).editor.synchronize();
    }
    onOpened: { draft=snapshot(); validationError="";resetFields(); }
    component NumberField: RowLayout {
        id: field
        required property var modelData
        property alias editor: numberEditor
        Layout.fillWidth: true
        visible: modelData.key==="pointSize" ? control.draft.sizeMode===0
            : modelData.key==="sceneSize" ? control.draft.sizeMode===1
            : modelData.key==="spacingZ" ? control.draft.viewMode===1
            : modelData.key.startsWith("gap") ? control.draft.viewMode===0
                && control.preview && control.preview.source
                && control.preview.source.layoutMetadata().type?.startsWith("Matrix") : true
        CatLabel { Layout.fillWidth: true; text: field.modelData.label; wrapMode: Text.Wrap }
        CatNumberBox {
            id: numberEditor
            objectName: "preview_" + field.modelData.key
            Layout.preferredWidth: 120
            minimum: field.modelData.min; maximum: field.modelData.max
            Binding on value { value: control.draft[field.modelData.key] ?? 1 }
            onValueEdited: value => control.update(field.modelData.key,value)
        }
    }
    contentItem: CatScrollView {
        contentWidth: availableWidth
        ColumnLayout {
            width: parent.width
            spacing: Theme.tokens.spacing
            CatComboBox {
                Layout.fillWidth: true
                model: [qsTr("2D view"),qsTr("3D view")]
                currentIndex: control.draft.viewMode ?? 1
                onActivated: index => control.update("viewMode",index)
            }
            CatComboBox {
                Layout.fillWidth: true
                model: [qsTr("Square"),qsTr("Circle"),qsTr("Cube"),qsTr("Sphere (12 × 24)")]
                currentIndex: control.draft.pointShape ?? 1
                onActivated: index => control.update("pointShape",index)
            }
            CatComboBox {
                Layout.fillWidth: true
                model: [qsTr("Fixed screen pixels"),qsTr("Scene units")]
                currentIndex: control.draft.sizeMode ?? 0
                onActivated: index => control.update("sizeMode",index)
            }
            CatComboBox {
                Layout.fillWidth: true
                model: [qsTr("Camera: current"),qsTr("Front"),qsTr("Isometric"),qsTr("Right"),qsTr("Top")]
                onActivated: index => {
                    if(index>0)control.update("camera",{yaw:index===2?35:index===3?90:0,pitch:index===2?25:index===4?89:0,zoom:1,panX:0,panY:0});
                }
            }
            Repeater {
                id: fields
                model: [ {key:"pointSize",label:qsTr("Point diameter (px)"),min:1,max:128},
                    {key:"sceneSize",label:qsTr("Scene diameter / cube edge"),min:0.0001,max:1000000},
                    {key:"spacingX",label:qsTr("Display spacing X"),min:0.1,max:10},
                    {key:"spacingY",label:qsTr("Display spacing Y"),min:0.1,max:10},
                    {key:"spacingZ",label:qsTr("Display spacing Z"),min:0.1,max:10},
                    {key:"gapX",label:qsTr("Matrix X gap ratio"),min:0,max:0.95},
                    {key:"gapY",label:qsTr("Matrix Y gap ratio"),min:0,max:0.95},
                    {key:"previewBrightness",label:qsTr("Preview brightness"),min:0,max:1},
                    {key:"maximumFps",label:qsTr("Maximum preview FPS"),min:1,max:240} ]
                delegate: NumberField {}
            }
            CatComboBox {
                Layout.fillWidth: true
                visible: control.draft.viewMode===0 && control.preview && control.preview.source
                         && control.preview.source.layoutMetadata().type === "Matrix3D"
                model: [qsTr("XY projection (overlapping layers)"),qsTr("Unfold X slices"),qsTr("Unfold Y slices"),qsTr("Unfold Z slices")]
                currentIndex: control.draft.sliceAxis ?? 0
                onActivated: index => control.update("sliceAxis",index)
            }
            CatCheckBox { text: qsTr("Arrange slices vertically"); visible: control.draft.sliceAxis>0; checked: control.draft.slicesVertical ?? false; onToggled: control.update("slicesVertical",checked) }
            Repeater {
                model: [{key:"previewEnabled",label:qsTr("Enable preview")},{key:"showStatistics",label:qsTr("Statistics")},
                        {key:"showGrid",label:qsTr("Screen reference grid")},{key:"showAxes",label:qsTr("Axis legend")}]
                delegate: CatCheckBox {
                    required property var modelData
                    text: modelData.label; checked: control.draft[modelData.key] ?? false
                    onToggled: control.update(modelData.key,checked)
                }
            }
            CatLabel { text: qsTr("Background color") }
            CatTextField { Layout.fillWidth: true; text: control.draft.backgroundColor ?? ""; onTextEdited: control.update("backgroundColor",text) }
            CatLabel { text: qsTr("Background image URL (optional)") }
            CatTextField { Layout.fillWidth: true; text: control.draft.backgroundImage ?? ""; onTextEdited: control.update("backgroundImage",text) }
            CatLabel {
                Layout.fillWidth: true; wrapMode: Text.Wrap; color: Theme.tokens.textSecondary
                text: qsTr("Display spacing changes centers without refitting. Gap ratios reduce point size; round shapes use the smaller fill. VSync is controlled by the window/backend.")
            }
            CatLabel { Layout.fillWidth: true; wrapMode: Text.Wrap; text: control.validationError; visible: text.length>0 }
        }
    }
    footer: Flow {
        padding: Theme.tokens.padding; spacing: Theme.tokens.spacing
        CatButton { objectName: "previewDefaults"; text: qsTr("Defaults"); onClicked: { control.draft=control.defaults();control.resetFields(); } }
        CatButton { objectName: "previewCancel"; text: qsTr("Cancel"); onClicked: control.close() }
        CatButton { objectName: "previewApply"; text: qsTr("Apply"); primary: true; onClicked: if(control.commitFields() && control.applySettings(control.draft)) control.close() }
    }
}
