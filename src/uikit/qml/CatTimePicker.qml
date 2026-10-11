// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick.Layouts
import QtQuick
import QCatUiKit

CatButton {
    id: control
    property var selectedTime: null
    property int minuteIncrement: 1
    property string placeholderText: qsTr("Choose a time")
    signal timeSelected(date value)
    text: selectedTime ? selectedTime.toLocaleTimeString(locale, "hh:mm") : placeholderText
    function choose(hour: int, minute: int): bool {
        if (hour < 0 || hour > 23 || minute < 0 || minute > 59)
            return false;
        selectedTime = new Date(1970, 0, 1, hour, minute);
        timeSelected(selectedTime);
        return true;
    }
    onClicked: {
        const time = selectedTime || new Date();
        hours.value = time.getHours();
        minutes.value = time.getMinutes();
        timePopup.open();
    }
    CatPopup {
        id: timePopup
        theme: control.theme
        y: control.height + 4
        width: 260
        modal: false
        contentItem: ColumnLayout {
            spacing: 12
            CatLabel {
                theme: control.theme
                text: qsTr("Hour")
            }
            CatSpinBox {
                id: hours
                theme: control.theme
                from: 0
                to: 23
                editable: true
                Layout.fillWidth: true
                Accessible.name: qsTr("Hour")
            }
            CatLabel {
                theme: control.theme
                text: qsTr("Minute")
            }
            CatSpinBox {
                id: minutes
                theme: control.theme
                from: 0
                to: 59
                stepSize: Math.max(1, control.minuteIncrement)
                editable: true
                Layout.fillWidth: true
                Accessible.name: qsTr("Minute")
            }
            RowLayout {
                Layout.fillWidth: true
                CatButton {
                    theme: control.theme
                    text: qsTr("Cancel")
                    onClicked: timePopup.close()
                }
                Item {
                    Layout.fillWidth: true
                }
                CatButton {
                    theme: control.theme
                    text: qsTr("Apply")
                    primary: true
                    onClicked: {
                        control.choose(hours.value, minutes.value);
                        timePopup.close();
                    }
                }
            }
        }
    }
}
