// SPDX-License-Identifier: Apache-2.0
import QtQuick

Item {
    id: control
    property bool compact: false
    implicitWidth: compact ? 24 : 48
    implicitHeight: implicitWidth
    Accessible.ignored: true
    Image {
        anchors.fill: parent
        source: "qrc:/qt/qml/QCatUiKit/Gallery/assets/brand/png/qcat-uikit-512.png"
        fillMode: Image.PreserveAspectFit
        smooth: true
        mipmap: true
        Accessible.ignored: true
    }
}
