// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QCatUiKit
import QtQuick

CatScrollView {
    id: control
    property var model: []
    property string textRole: ""
    property int currentIndex: -1
    property real itemWidth: 120
    property real itemHeight: 64
    signal activated(int index)
    implicitWidth: 320
    implicitHeight: 240
    contentWidth: availableWidth
    Flow {
        width: control.availableWidth
        spacing: (control.theme || Theme).tokens.spacing
        Repeater {
            model: control.model
            CatButton {
                required property int index
                required property var modelData
                theme: control.theme
                width: Math.min(control.itemWidth, control.availableWidth)
                height: control.itemHeight
                text: String(control.textRole.length ? modelData[control.textRole] : modelData)
                checked: control.currentIndex === index
                onClicked: {
                    control.currentIndex = index;
                    control.activated(index);
                }
            }
        }
    }
}
