// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit

TestCase {
    id: test
    name: "LedUiContracts"
    when: windowShown
    width: 960; height: 720; visible: true
    Component { id: previewFactory; LedPreview { width: 320; height: 240; showStatistics: false } }
    Component { id: settingsFactory; LedPreviewSettings {} }
    Component { id: layoutSettingsFactory; LedLayoutSettings {} }
    Component { id: outputSettingsFactory; LedOutputSettings {} }
    Component { id: layoutFactory; LedLayoutModel {} }
    Component { id: repositoryFactory; LedStorageRepository {} }
    Component { id: projectFactory; LedProject {} }
    Component { id: deckFactory; LedDeck { width: 360 } }
    Component { id: browserFactory; LedStorageBrowser { width: 360 } }
    function init() { failOnWarning(/.*/); }
    function test_previewDraftCancelDefaultsAndValidation() {
        const preview=createTemporaryObject(previewFactory,test);
        const settings=createTemporaryObject(settingsFactory,test,{preview:preview});
        const initial=settings.snapshot();
        settings.draft=Object.assign({},initial);
        settings.update("pointShape",LedPreviewRenderItem.Cube);
        compare(preview.pointShape,LedPreviewRenderItem.Circle);
        settings.close();compare(preview.pointShape,LedPreviewRenderItem.Circle);
        verify(settings.applySettings(settings.draft));compare(preview.pointShape,LedPreviewRenderItem.Cube);
        const invalid=Object.assign({},settings.draft,{maximumFps:0,pointSize:30});
        verify(!settings.applySettings(invalid));compare(preview.pointSize,initial.pointSize);
        verify(settings.applySettings(settings.defaults()));compare(preview.pointShape,LedPreviewRenderItem.Circle);
        preview.setCamera({yaw:35,pitch:25,zoom:2,panX:0.2,panY:0.1});
        const saved=settings.snapshot();preview.resetView();verify(settings.applySettings(saved));compare(preview.camera().yaw,35);
    }
    function test_independentDeckUiAndBrowse() {
        const project=createTemporaryObject(projectFactory,test);
        project.repository.populateExamples();
        const storage=project.repository.storages[0].id;
        const place=project.repository.places(storage)[0].id;
        project.deckA.browseStorage(storage);project.deckB.browseStorage(storage);
        const a=createTemporaryObject(deckFactory,test,{runtime:project.deckA});
        const b=createTemporaryObject(deckFactory,test,{runtime:project.deckB,x:400});
        verify(project.deckA.loadPlace(storage,place));verify(project.deckB.loadPlace(storage,place));
        tryCompare(project.deckA,"loading",false);tryCompare(project.deckB,"loading",false);
        a.preview.pointShape=LedPreviewRenderItem.Sphere;
        compare(b.preview.pointShape,LedPreviewRenderItem.Circle);
        a.preview.previewEnabled=false;compare(b.preview.previewEnabled,true);
        project.deckA.browseStorage(project.repository.storages[1].id);
        compare(project.deckA.activeStorage,storage);compare(project.deckB.browsingStorage,storage);
        compare(a.storageBrowser.repository,project.repository);
        project.repository.readOnly=true;
        verify(!project.deckA.savePlace(storage,place,"No overwrite"));
    }
    function test_keyboardPreselectionAndStorageChange() {
        const project=createTemporaryObject(projectFactory,test);project.repository.populateExamples();
        const first=project.repository.storages[0].id,second=project.repository.storages[1].id;
        const active=project.repository.places(first)[0].id;
        project.deckA.browseStorage(first);verify(project.deckA.loadPlace(first,active));tryCompare(project.deckA,"loading",false);
        const browser=createTemporaryObject(browserFactory,test,{runtime:project.deckA});
        const grid=findChild(browser,"ledPlaceGrid");verify(grid!==null);
        browser.currentIndex=1;grid.forceActiveFocus();keyClick(Qt.Key_Right);
        compare(browser.currentIndex,2);compare(project.deckA.activePlace,active);
        keyClick(Qt.Key_Return);tryCompare(project.deckA,"loading",false);
        compare(project.deckA.activePlace,project.repository.places(first)[2].id);
        project.deckA.browseStorage(second);
        verify(browser.selectedPlace==="" || project.repository.places(second).some(p=>p.id===browser.selectedPlace));
        compare(project.deckA.activeStorage,first);
        const other=createTemporaryObject(repositoryFactory,test);
        verify(other.importJson(project.repository.exportJson()));
        project.deckA.repository=other;
        compare(project.deckA.activeRepositoryId,project.repository.repositoryId);
        verify(project.deckA.activeRepositoryId!==other.repositoryId);
        compare(project.deckA.activeStorage,first);
    }
    function test_layoutDraftValidation() {
        const layout=createTemporaryObject(layoutFactory,test);
        const original=layout.revision;
        const bad=Object.assign({},layout.configuration,{countX:-1});
        verify(layout.validateConfiguration(bad).length>0);layout.apply(bad);compare(layout.revision,original);
        const next=Object.assign({},layout.configuration,{type:"Matrix3D",countX:16,countY:16,countZ:8});
        layout.apply(next);tryCompare(layout,"busy",false);compare(layout.pointCount,2048);
    }
    function field(dialog, name) {
        const item=findChild(dialog.contentItem,name) || findChild(dialog.footer,name);
        verify(item!==null,name);
        return item;
    }
    function typeNumber(editor,text) {
        editor.forceActiveFocus();editor.selectAll();
        for(const character of text)keyClick(character);
    }
    function test_deckSpeedsFinishEditing() {
        const project=createTemporaryObject(projectFactory,test);
        project.repository.populateExamples();
        const storage=project.repository.storages[0].id;
        verify(project.deckA.loadPlace(storage,project.repository.places(storage)[0].id));
        tryCompare(project.deckA,"loading",false);
        const deck=createTemporaryObject(deckFactory,test,{runtime:project.deckA});
        const speed=findChild(deck,"deckSpeed"),place=findChild(deck,"placeSpeed");
        verify(speed!==null && place!==null);
        typeNumber(speed,"2.5");
        typeNumber(place,"3");
        compare(project.deckA.speed,2.5);
        test.forceActiveFocus();
        tryVerify(()=>project.deckA.workingPlace.speed===3);
        project.deckA.speed=1;
        project.deckA.editPlace({speed:1});
        tryVerify(()=>project.deckA.workingPlace.speed===1);
        compare(speed.value,1);compare(place.value,1);
    }
    function test_settingsInput_data() {
        return [{tag:"layout",key:"countX",text:"17",value:17},
                {tag:"preview",key:"pointSize",text:"13",value:13},
                {tag:"output",key:"startChannel",text:"42",value:42}];
    }
    function test_layoutFields_data() {
        return [
            {tag:"countX",type:"Matrix3D",text:"7",value:7},
            {tag:"countY",type:"Matrix3D",text:"5",value:5},
            {tag:"countZ",type:"Matrix3D",text:"3",value:3},
            {tag:"spacingX",type:"Matrix3D",text:"0.125",value:0.125},
            {tag:"spacingY",type:"Matrix3D",text:"2.5",value:2.5},
            {tag:"spacingZ",type:"Matrix3D",text:"0.0001",value:0.0001},
            {tag:"count",type:"SphereSurface",text:"17",value:17},
            {tag:"radius",type:"Ring",text:"3.5",value:3.5},
            {tag:"startAngle",type:"Ring",text:"-45",value:-45},
            {tag:"radialLayers",type:"Disk",text:"3",value:3},
            {tag:"angularPoints",type:"Disk",text:"12",value:12}
        ];
    }
    function test_layoutFields(data) {
        const layout=createTemporaryObject(layoutFactory,test);
        const config=Object.assign({},layout.defaultConfiguration(),{type:data.type,
            countX:4,countY:3,countZ:2,count:8,radialLayers:2,angularPoints:8});
        layout.apply(config);tryCompare(layout,"busy",false);
        const dialog=createTemporaryObject(layoutSettingsFactory,test,{layoutModel:layout});
        dialog.open();tryCompare(dialog,"opened",true);
        const editor=field(dialog,"layout_"+data.tag);
        verify(editor.visible);
        typeNumber(editor,data.text);
        keyClick(Qt.Key_Tab);
        compare(dialog.draft[data.tag],data.value,"Tab must commit the field to the draft");
        mouseClick(field(dialog,"layoutApply"));
        tryCompare(dialog,"opened",false);
        tryVerify(()=>layout.configuration[data.tag]===data.value);
    }
    function test_settingsInput(data) {
        const project=createTemporaryObject(projectFactory,test);
        const preview=createTemporaryObject(previewFactory,test);
        verify(project.output.applyProfiles([{id:"group",deviceId:"memory",portId:"RGB",schema:3,
            channelOrder:["R","G","B"],firstPoint:0,pointCount:1,startChannel:1,conversion:2,
            whiteKelvin:4000,coldKelvin:6500,warmKelvin:2700}]));
        const dialog=data.tag==="layout" ? createTemporaryObject(layoutSettingsFactory,test,{layoutModel:project.layout})
            : data.tag==="preview" ? createTemporaryObject(settingsFactory,test,{preview:preview})
            : createTemporaryObject(outputSettingsFactory,test,{output:project.output});
        function actual() {
            return data.tag==="layout" ? project.layout.configuration[data.key]
                : data.tag==="preview" ? preview[data.key] : project.output.profiles[0][data.key];
        }
        const original=actual();
        dialog.open();tryCompare(dialog,"opened",true);
        const editor=field(dialog,data.tag+"_"+data.key);
        typeNumber(editor,data.text);
        compare(actual(),original,"Edits remain a draft until Apply");
        mouseClick(field(dialog,data.tag+"Apply"));
        tryCompare(dialog,"opened",false);
        tryVerify(()=>actual()===data.value,5000,"Apply must include the focused editor without Return");
        dialog.open();tryCompare(dialog,"opened",true);
        compare(editor.value,data.value);
        typeNumber(editor,"invalid");
        mouseClick(field(dialog,data.tag+"Apply"));
        verify(dialog.opened,"Invalid text must block Apply");
        verify(editor.invalid);
        compare(actual(),data.value);
        mouseClick(field(dialog,data.tag+"Cancel"));
        tryCompare(dialog,"opened",false);
        dialog.open();tryCompare(dialog,"opened",true);
        verify(!editor.invalid);
        compare(editor.text,Number(data.value).toLocaleString(editor.locale,'f',editor.decimals));
        typeNumber(editor,"23");
        mouseClick(field(dialog,data.tag+"Cancel"));
        tryCompare(dialog,"opened",false);
        compare(actual(),data.value,"Cancel must discard a valid edit too");
        dialog.open();tryCompare(dialog,"opened",true);
        compare(editor.value,data.value,"Reopen must restore the applied value");
        if(data.tag!=="output") {
            typeNumber(editor,"19");
            mouseClick(field(dialog,data.tag+"Defaults"));
            const defaults=data.tag==="layout" ? project.layout.defaultConfiguration() : dialog.defaults();
            compare(editor.value,defaults[data.key]);
            compare(editor.text,Number(defaults[data.key]).toLocaleString(editor.locale,'f',editor.decimals));
        }
        dialog.close();tryCompare(dialog,"opened",false);
    }
}
