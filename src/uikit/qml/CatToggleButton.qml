// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QCatUiKit
import QtQuick

CatButton {
    id: control
    property bool threeState: false
    property int checkState: Qt.Unchecked
    checkable: true
    onCheckedChanged: if (!threeState)
                          checkState = checked ? Qt.Checked : Qt.Unchecked
    onCheckStateChanged: checked = checkState !== Qt.Unchecked
    onClicked: if (threeState)
                   checkState = (checkState + 1) % 3
}
