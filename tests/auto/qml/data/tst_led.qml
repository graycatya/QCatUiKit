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
}
