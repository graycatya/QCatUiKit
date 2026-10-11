// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QCatUiKit
import QtQuick

CatButton {
    id: control
    property alias delay: control.autoRepeatDelay
    property alias interval: control.autoRepeatInterval
    autoRepeat: true
}
