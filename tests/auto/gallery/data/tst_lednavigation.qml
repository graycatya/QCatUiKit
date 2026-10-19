// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit
import QCatUiKit.Gallery

TestCase {
    id: test
    name: "GalleryLedNavigation"
    when: windowShown
    property var gallery
    Component { id: factory; Main { width: 1200; height: 840 } }
    function init() {
        failOnWarning(/.*/);
        gallery = createTemporaryObject(factory, test);
        verify(waitForRendering(gallery.contentItem));
    }
    function cleanup() {
        gallery.close();
        GalleryLanguage.language = "en";
    }
    function test_unloadAfterFirstFrame_data() {
        return [{tag:"en_desktop",width:1200,language:"en"},
                {tag:"zh_desktop",width:1200,language:"zh_CN"},
                {tag:"zh_mobile",width:360,language:"zh_CN"}];
    }
    function test_unloadAfterFirstFrame(data) {
        gallery.width=data.width;
        GalleryLanguage.language=data.language;
        for (let round=0; round<3; ++round) {
            gallery.showComponent("led-preview");
            verify(waitForRendering(gallery.contentItem));
            const led = findChild(gallery.contentItem,"ledPreviewDemo");
            verify(led !== null);
            verify(led.dataController.repository.storages.length > 0);
            gallery.showComponent("accordion");
            verify(waitForRendering(gallery.contentItem));
            compare(findChild(gallery.contentItem,"ledPreviewDemo"),null);
        }
    }
}
