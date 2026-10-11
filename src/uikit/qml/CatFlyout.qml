// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QCatUiKit

CatPopup {
    id: control
    property Item target
    property int placement: Qt.BottomEdge
    property real placementMargin: 8
    modal: false
    parent: target ? target : T.Overlay.overlay
    x: target ? (placement === Qt.RightEdge ? target.width + placementMargin : placement
                                              === Qt.LeftEdge ? -width - placementMargin : (
                                                                    target.width - width) / 2) : 0
    y: target ? (placement === Qt.TopEdge ? -height - placementMargin : placement === Qt.BottomEdge
                                            ? target.height + placementMargin : (target.height
                                                                                 - height) / 2) : 0
    margins: 8
}
