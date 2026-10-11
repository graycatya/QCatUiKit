// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QtQuick.Layouts
import QCatUiKit

CatDialog {
    id: control
    property string primaryButtonText
    property string secondaryButtonText
    property string closeButtonText: qsTr("Close")
    signal primaryClicked
    signal secondaryClicked
    footer: T.Control {
        padding: control.tokens.padding
        implicitHeight: footerRow.implicitHeight + topPadding + bottomPadding
        contentItem: RowLayout {
            id: footerRow
            spacing: control.tokens.spacing
            CatButton {
                theme: control.theme
                text: control.primaryButtonText
                visible: text.length > 0
                primary: true
                Layout.fillWidth: true
                onClicked: {
                    control.primaryClicked();
                    control.accept();
                }
            }
            CatButton {
                theme: control.theme
                text: control.secondaryButtonText
                visible: text.length > 0
                Layout.fillWidth: true
                onClicked: {
                    control.secondaryClicked();
                    control.close();
                }
            }
            CatButton {
                theme: control.theme
                text: control.closeButtonText
                visible: text.length > 0
                Layout.fillWidth: true
                onClicked: control.reject()
            }
        }
    }
}
