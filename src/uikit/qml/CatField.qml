// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QCatUiKit
import QtQuick

Item {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property string labelText
    property bool required: false
    property string helperText
    property string validationMessage
    property bool invalid: validationMessage.length > 0
    property Item editor
    implicitWidth: 280
    implicitHeight: fields.implicitHeight
    Column {
        id: fields
        width: control.width
        spacing: control.tokens.spacing
        CatLabel {
            theme: control.theme
            width: parent.width
            text: control.labelText + (control.required ? " *" : "")
            font.bold: true
            wrapMode: Text.Wrap
            visible: text.length > 0
        }
        Item {
            id: editorHost
            width: parent.width
            height: control.editor ? control.editor.implicitHeight : 0
            children: control.editor ? [control.editor] : []
            Binding {
                target: control.editor
                property: "width"
                value: editorHost.width
                when: control.editor !== null
                restoreMode: Binding.RestoreBindingOrValue
            }
        }
        CatLabel {
            theme: control.theme
            width: parent.width
            text: control.invalid ? control.validationMessage : control.helperText
            color: control.invalid ? control.tokens.danger : control.tokens.textSecondary
            visible: text.length > 0
            wrapMode: Text.Wrap
        }
    }
}
