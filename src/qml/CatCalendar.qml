// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit
import QtQuick.Controls.Basic as B
import QtQuick.Layouts

CatPane {
    id: control
    property date selectedDate: new Date()
    property int month: selectedDate.getMonth()
    property int year: selectedDate.getFullYear()
    property date minimumDate: new Date(1900, 0, 1)
    property date maximumDate: new Date(2100, 11, 31)
    signal dateSelected(date value)
    implicitWidth: Math.max(308, tokens.controlHeight * 7 + leftPadding + rightPadding)
    function moveMonth(delta: int): void {
        const d = new Date(year, month + delta, 1);
        month = d.getMonth();
        year = d.getFullYear();
    }
    function selectDate(value: date): void {
        const day = new Date(value.getFullYear(), value.getMonth(), value.getDate());
        const low = new Date(minimumDate.getFullYear(), minimumDate.getMonth(), minimumDate.getDate());
        const high = new Date(maximumDate.getFullYear(), maximumDate.getMonth(), maximumDate.getDate());
        if (day < low || day > high || isNaN(day.getTime()))
            return;
        selectedDate = day;
        month = day.getMonth();
        year = day.getFullYear();
        dateSelected(day);
    }
    contentItem: ColumnLayout {
        spacing: control.tokens.spacing
        RowLayout {
            Layout.fillWidth: true
            CatButton {
                theme: control.theme
                text: "‹"
                implicitWidth: control.tokens.controlHeight
                Accessible.name: qsTr("Previous month")
                onClicked: control.moveMonth(-1)
            }
            CatLabel {
                theme: control.theme
                text: grid.title
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }
            CatButton {
                theme: control.theme
                text: "›"
                implicitWidth: control.tokens.controlHeight
                Accessible.name: qsTr("Next month")
                onClicked: control.moveMonth(1)
            }
        }
        B.DayOfWeekRow {
            Layout.fillWidth: true
            locale: control.locale
            font.pixelSize: control.tokens.fontSize
            delegate: CatLabel {
                required property string shortName
                theme: control.theme
                text: shortName
                horizontalAlignment: Text.AlignHCenter
            }
        }
        B.MonthGrid {
            id: grid
            Layout.fillWidth: true
            Layout.preferredHeight: control.tokens.controlHeight * 6
            month: control.month
            year: control.year
            locale: control.locale
            delegate: CatButton {
                required property var model
                theme: control.theme
                implicitWidth: control.tokens.controlHeight
                text: model.day
                primary: model.year === control.selectedDate.getFullYear() && model.month === control.selectedDate.getMonth() && model.day === control.selectedDate.getDate()
                opacity: model.month === control.month ? 1 : 0.45
                enabled: model.date >= new Date(control.minimumDate.getFullYear(), control.minimumDate.getMonth(), control.minimumDate.getDate()) && model.date <= control.maximumDate
                Accessible.name: model.date.toLocaleDateString(control.locale)
                onClicked: control.selectDate(model.date)
            }
        }
    }
}
