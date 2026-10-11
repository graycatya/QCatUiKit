// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QCatUiKit
import QtQuick
import QtQuick.Controls.Basic as B

B.StackView {
    id: control
    property ThemePalette theme: Theme
    implicitWidth: 320
    implicitHeight: 240
    pushEnter: Transition {
        NumberAnimation {
            property: "opacity"
            from: 0
            to: 1
            duration: (control.theme || Theme).tokens.motionDuration
        }
    }
    pushExit: Transition {
        NumberAnimation {
            property: "opacity"
            from: 1
            to: 0
            duration: (control.theme || Theme).tokens.motionDuration
        }
    }
    popEnter: Transition {
        NumberAnimation {
            property: "opacity"
            from: 0
            to: 1
            duration: (control.theme || Theme).tokens.motionDuration
        }
    }
    popExit: Transition {
        NumberAnimation {
            property: "opacity"
            from: 1
            to: 0
            duration: (control.theme || Theme).tokens.motionDuration
        }
    }
    replaceEnter: Transition {
        NumberAnimation {
            property: "opacity"
            from: 0
            to: 1
            duration: (control.theme || Theme).tokens.motionDuration
        }
    }
    replaceExit: Transition {
        NumberAnimation {
            property: "opacity"
            from: 1
            to: 0
            duration: (control.theme || Theme).tokens.motionDuration
        }
    }
}
