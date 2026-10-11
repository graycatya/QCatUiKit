// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit
import QCatUiKit.Gallery

TestCase {
    id: test
    name: "GalleryCodeSamples"
    when: windowShown
    width: 1000
    height: 800
    visible: true
    property var example

    function init() {
        failOnWarning(/.*/);
        GalleryLanguage.language = "en";
    }
    function cleanup() {
        if (example) {
            if (example.close)
                example.close();
            example.destroy();
        }
        example = null;
        wait(1);
    }
    function test_copyableExamples_data() {
        return GalleryCatalog.components.map(entry => ({ tag: entry.id, entry: entry }));
    }
    function test_copyableExamples(data) {
        example = Qt.createQmlObject("import QtQuick\nimport QCatUiKit\n" + data.entry.code,
                                    test, "gallery_example_" + data.entry.id + ".qml");
        verify(example !== null);
        wait(10);
    }
}
