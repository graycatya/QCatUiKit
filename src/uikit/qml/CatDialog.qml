// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.Basic as B
import QCatUiKit

T.Dialog {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    padding: tokens.padding
    spacing: tokens.spacing
    implicitWidth: Math.max(360, implicitContentWidth + leftPadding + rightPadding,
                            implicitHeaderWidth, implicitFooterWidth)
    implicitHeight: implicitContentHeight + topPadding + bottomPadding + implicitHeaderHeight
                    + implicitFooterHeight + (implicitHeaderHeight > 0 ? spacing : 0) + (
                        implicitFooterHeight > 0 ? spacing : 0)
    modal: true
    focus: true
    parent: T.Overlay.overlay
    anchors.centerIn: parent
    closePolicy: T.Popup.CloseOnEscape
    font.pixelSize: tokens.fontSize
    header: CatLabel {
        theme: control.theme
        text: control.title
        visible: text.length > 0
        padding: control.tokens.padding
        font.pixelSize: 20
        font.bold: true
        wrapMode: Text.Wrap
    }
    footer: B.DialogButtonBox {
        id: buttons
        visible: count > 0
        padding: control.tokens.padding
        delegate: CatButton {
            theme: control.theme
            primary: T.DialogButtonBox.buttonRole === T.DialogButtonBox.AcceptRole
                     || T.DialogButtonBox.buttonRole === T.DialogButtonBox.YesRole
        }
        background: null
    }
    background: SurfaceImpl {
        theme: control.theme
    }
    T.Overlay.modal: Rectangle {
        color: Qt.rgba(0, 0, 0, 0.35)
    }
    enter: Transition {
        NumberAnimation {
            property: "opacity"
            from: 0
            to: 1
            duration: control.tokens.motionDuration
        }
    }
    exit: Transition {
        NumberAnimation {
            property: "opacity"
            from: 1
            to: 0
            duration: control.tokens.motionDuration
        }
    }
}
