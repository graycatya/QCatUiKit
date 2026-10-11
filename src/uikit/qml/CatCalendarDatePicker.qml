// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QCatUiKit
import QtQuick

CatButton {
    id: control
    property var selectedDate: null
    property date minimumDate: new Date(1900, 0, 1)
    property date maximumDate: new Date(2100, 11, 31)
    property string placeholderText: qsTr("Choose a date")
    property string displayFormat: ""
    readonly property bool calendarOpen: calendarPopup.opened
    signal dateSelected(date value)
    text: selectedDate ? (displayFormat.length ? selectedDate.toLocaleDateString(locale,
                                                                                 displayFormat) :
                                                 selectedDate.toLocaleDateString(locale)) :
                         placeholderText
    function choose(candidate: date): bool {
        const day = new Date(candidate.getFullYear(), candidate.getMonth(), candidate.getDate());
        const lower = new Date(minimumDate.getFullYear(), minimumDate.getMonth(),
                               minimumDate.getDate());
        const upper = new Date(maximumDate.getFullYear(), maximumDate.getMonth(),
                               maximumDate.getDate());
        if (isNaN(day.getTime()) || day < lower || day > upper)
            return false;
        selectedDate = day;
        dateSelected(day);
        return true;
    }
    onClicked: calendarPopup.open()
    CatPopup {
        id: calendarPopup
        theme: control.theme
        y: control.height + 4
        modal: false
        width: calendar.implicitWidth + leftPadding + rightPadding
        contentItem: CatCalendar {
            id: calendar
            theme: control.theme
            locale: control.locale
            selectedDate: control.selectedDate || new Date()
            minimumDate: control.minimumDate
            maximumDate: control.maximumDate
            onDateSelected: value => {
                if (control.choose(value))
                    calendarPopup.close();
            }
        }
    }
}
