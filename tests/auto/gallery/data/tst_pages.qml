// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit
import QCatUiKit.Gallery

TestCase {
    id: test
    name: "GalleryPages"
    when: windowShown
    property var gallery
    Component { id: factory; Main { width: 1200; height: 840 } }

    function initTestCase() {
        failOnWarning(/.*/);
        GalleryLanguage.language = "en";
        gallery = factory.createObject(test);
        verify(gallery !== null);
        verify(waitForRendering(gallery.contentItem));
    }
    function init() { failOnWarning(/.*/); }
    function cleanupTestCase() {
        gallery.close();
        gallery.destroy();
        GalleryLanguage.language = "en";
    }
    function test_allExamples_data() {
        const rows = [];
        for (const language of ["en", "zh_CN"])
            for (const entry of GalleryCatalog.components)
                rows.push({ tag: language + "_" + entry.id, language: language,
                            width: 1200, entry: entry });
        for (const entry of GalleryCatalog.components)
            rows.push({ tag: "zh_mobile_" + entry.id, language: "zh_CN", width: 360, entry: entry });
        return rows;
    }
    function test_allExamples(data) {
        gallery.width = data.width;
        GalleryLanguage.language = data.language;
        gallery.showComponent(data.entry.id);
        // The first frame already paints the synchronous preview loader.
        // A second wait would time out for a static page with no pending frame.
        verify(waitForRendering(gallery.contentItem));
        const loader = findChild(gallery.contentItem, "componentPreview");
        verify(loader !== null);
        tryCompare(loader, "status", Loader.Ready);
        verify(loader.item !== null);
        compare(gallery.componentId, data.entry.id);
        const code = findChild(gallery.contentItem, "codeSample");
        verify(code !== null);
        verify(code.text.includes(data.entry.name));
    }
}
