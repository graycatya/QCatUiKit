// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtTest
import QCatUiKit

TestCase {
    id: test
    name: "Accessibility"
    when: windowShown
    width: 480
    height: 320
    visible: true
    Component {
        id: buttonFactory
        CatButton {
            text: "Run"
            readonly property string exposedName: Accessible.name
        }
    }
    Component {
        id: checkBoxFactory
        CatCheckBox {
            text: "Remember"
            readonly property string exposedName: Accessible.name
        }
    }
    Component {
        id: radioFactory
        CatRadioButton {
            text: "Option"
            readonly property string exposedName: Accessible.name
        }
    }
    Component {
        id: switchFactory
        CatSwitch {
            text: "Enabled"
            readonly property string exposedName: Accessible.name
        }
    }
    Component {
        id: menuItemFactory
        CatMenuItem {
            text: "Export"
            readonly property string exposedName: Accessible.name
        }
    }
    Component {
        id: linkFactory
        CatHyperlinkButton {
            text: "Docs"
            readonly property string exposedName: Accessible.name
            readonly property int exposedRole: Accessible.role
        }
    }
    function init() {
        failOnWarning(/.*/);
    }
    // Shared by the test cases and by the external semantic check that runs
    // without the Qt Quick Test harness.
    function collectSemanticFailures() {
        const failures = [];
        const nameCases = [[buttonFactory, "Run"], [checkBoxFactory, "Remember"], [radioFactory, "Option"], [switchFactory, "Enabled"], [menuItemFactory, "Export"]];
        for (let i = 0; i < nameCases.length; ++i) {
            const expected = nameCases[i][1];
            const item = createTemporaryObject(nameCases[i][0], test);
            if (!item) {
                failures.push("cannot create control exposing '" + expected + "'");
                continue;
            }
            if (item.exposedName !== expected)
                failures.push("'" + expected + "' exposes name '" + item.exposedName + "'");
            item.destroy();
        }
        const link = createTemporaryObject(linkFactory, test);
        if (!link) {
            failures.push("cannot create hyperlink button");
        } else {
            if (link.exposedName !== "Docs")
                failures.push("hyperlink exposes name '" + link.exposedName + "'");
            if (link.exposedRole !== Accessible.Link)
                failures.push("hyperlink exposes role " + link.exposedRole);
            link.destroy();
        }
        return failures;
    }
    function test_textControlsExposeNames() {
        const failures = collectSemanticFailures();
        compare(failures.length, 0, failures.join("; "));
    }
    function test_nameFollowsText() {
        const button = createTemporaryObject(buttonFactory, test);
        verify(button);
        button.text = "Save";
        compare(button.exposedName, "Save");
        button.destroy();
    }
}
