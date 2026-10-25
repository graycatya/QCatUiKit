// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit

VectorItem {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    format: VectorItem.Lottie
    autoPlay: true
    animationEnabled: !(theme || Theme).reducedMotion
    Accessible.role: Accessible.Graphic
    Accessible.name: qsTr("Lottie animation")
    CatBusyIndicator {
        anchors.centerIn: parent
        theme: control.theme
        visible: control.status === VectorItem.Loading
    }
    CatLabel {
        anchors.fill: parent
        anchors.margins: control.tokens.spacing
        theme: control.theme
        visible: control.status === VectorItem.Error
        text: control.errorString
        wrapMode: Text.Wrap
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
