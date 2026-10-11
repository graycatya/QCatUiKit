// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QCatUiKit
import QtQuick

CatButton {
    id: control
    property string secondaryText
    contentItem: Item {
        implicitWidth: Math.max(titleLabel.implicitWidth, descriptionLabel.implicitWidth)
        implicitHeight: labels.implicitHeight
        Column {
            id: labels
            width: parent.width
            spacing: 4
            Text {
                id: titleLabel
                width: parent.width
                text: control.text
                font: control.font
                color: !control.enabled ? control.tokens.textDisabled : control.primary
                                          ? control.tokens.onPrimary : control.tokens.text
                textFormat: Text.PlainText
                elide: Text.ElideRight
            }
            Text {
                id: descriptionLabel
                width: parent.width
                text: control.secondaryText
                font.pixelSize: Math.max(12, control.font.pixelSize - 2)
                color: !control.enabled ? control.tokens.textDisabled : control.primary
                                          ? control.tokens.onPrimary : control.tokens.textSecondary
                textFormat: Text.PlainText
                wrapMode: Text.Wrap
                visible: text.length > 0
            }
        }
    }
    implicitWidth: Math.max(200, implicitContentWidth + leftPadding + rightPadding)
}
