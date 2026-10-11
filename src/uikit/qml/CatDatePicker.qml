// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick.Layouts
import QtQuick
import QCatUiKit

CatButton {
    id: control
    property var selectedDate: null
    property date minimumDate: new Date(1900, 0, 1)
    property date maximumDate: new Date(2100, 11, 31)
    property string placeholderText: qsTr("Choose a date")
    signal dateSelected(date value)
    text: selectedDate ? selectedDate.toLocaleDateString(locale) : placeholderText
    function choose(candidate: date): bool {
        const day = new Date(candidate.getFullYear(), candidate.getMonth(), candidate.getDate());
        if (isNaN(day.getTime()) || day < new Date(minimumDate.getFullYear(), minimumDate.getMonth(),
                                                   minimumDate.getDate()) || day > new Date(
                    maximumDate.getFullYear(), maximumDate.getMonth(), maximumDate.getDate()))
            return false;
        selectedDate = day;
        dateSelected(day);
        return true;
    }
    onClicked: {
        const date = selectedDate || new Date();
        years.value = date.getFullYear();
        months.currentIndex = date.getMonth();
        days.value = date.getDate();
        datePopup.open();
    }
    CatPopup {
        id: datePopup
        theme: control.theme
        y: control.height + 4
        width: 300
        modal: false
        contentItem: ColumnLayout {
            spacing: 12
            CatComboBox {
                id: months
                theme: control.theme
                Layout.fillWidth: true
                model: Array.from({
                                      length: 12
                                  }, (_, index) => new Date(2026, index, 1).toLocaleDateString(
                                                       control.locale, "MMMM"))
                Accessible.name: qsTr("Month")
            }
            CatSpinBox {
                id: days
                theme: control.theme
                from: 1
                to: new Date(years.value, months.currentIndex + 1, 0).getDate()
                value: 1
                editable: true
                Layout.fillWidth: true
                Accessible.name: qsTr("Day")
            }
            CatSpinBox {
                id: years
                theme: control.theme
                from: control.minimumDate.getFullYear()
                to: control.maximumDate.getFullYear()
                value: new Date().getFullYear()
                editable: true
                Layout.fillWidth: true
                Accessible.name: qsTr("Year")
            }
            RowLayout {
                Layout.fillWidth: true
                CatButton {
                    theme: control.theme
                    text: qsTr("Cancel")
                    onClicked: datePopup.close()
                }
                Item {
                    Layout.fillWidth: true
                }
                CatButton {
                    theme: control.theme
                    text: qsTr("Apply")
                    primary: true
                    onClicked: if (control.choose(new Date(years.value, months.currentIndex,
                                                           days.value)))
                                   datePopup.close()
                }
            }
        }
    }
}
