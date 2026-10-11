// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QCatUiKit

T.Control {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property real value: -1
    property real placeholderValue: -1
    property int maxRating: 5
    property bool isReadOnly: false
    property bool isClearEnabled: true
    property string caption
    property real stepSize: 1
    signal ratingEdited(real value)
    padding: 0
    implicitWidth: ratings.implicitWidth
    implicitHeight: ratings.implicitHeight
    focusPolicy: Qt.StrongFocus
    Accessible.role: Accessible.Slider
    Accessible.name: caption
    Accessible.description: value < 0 ? qsTr("No rating") : qsTr("%1 of %2").arg(value).arg(
                                            maxRating)

    function rate(candidate: real): void {
        if (!enabled || isReadOnly)
            return;
        value = Math.max(-1, Math.min(maxRating, candidate));
        ratingEdited(value);
    }
    onValueChanged: if (value < -1 || value > maxRating)
                        value = Math.max(-1, Math.min(maxRating, value))
    onMaxRatingChanged: {
        if (maxRating < 1)
            maxRating = 1;
        if (value > maxRating)
            value = maxRating;
    }
    Keys.onRightPressed: event => {
        rate(Math.max(0, value) + stepSize);
        event.accepted = true;
    }
    Keys.onLeftPressed: event => {
        rate(Math.max(0, value) - stepSize);
        event.accepted = true;
    }
    Keys.onDeletePressed: event => {
        if (isClearEnabled)
            rate(-1);
        event.accepted = true;
    }
    contentItem: Column {
        id: ratings
        spacing: 4
        Row {
            Repeater {
                model: control.maxRating
                CatButton {
                    id: starButton
                    required property int index
                    theme: control.theme
                    flat: true
                    width: control.tokens.controlHeight
                    implicitWidth: width
                    height: control.tokens.controlHeight
                    enabled: control.enabled && !control.isReadOnly
                    Accessible.name: qsTr("Rate %1").arg(index + 1)
                    padding: 0
                    contentItem: Item {
                        implicitWidth: 24
                        implicitHeight: 28
                        Text {
                            anchors.fill: parent
                            text: "☆"
                            font.pixelSize: 24
                            color: control.tokens.textSecondary
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        Item {
                            width: parent.width * Math.max(0, Math.min(1, (control.value < 0
                                                                           ? control.placeholderValue :
                                                                             control.value)
                                                                       - starButton.index))
                            height: parent.height
                            clip: true
                            Text {
                                width: starButton.availableWidth
                                height: starButton.availableHeight
                                text: "★"
                                font.pixelSize: 24
                                color: !control.enabled ? control.tokens.textDisabled :
                                                          control.value < 0
                                                          ? control.tokens.textSecondary :
                                                            control.tokens.primary
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                    }
                    onClicked: control.rate(control.isClearEnabled && control.value === index + 1 ?
                                                -1 : index + 1)
                }
            }
        }
        CatLabel {
            theme: control.theme
            text: control.caption
            visible: text.length > 0
            color: control.tokens.textSecondary
        }
    }
    background: Rectangle {
        color: "transparent"
        radius: control.tokens.radius
        border.width: control.visualFocus ? 2 : 0
        border.color: control.tokens.primary
    }
}
