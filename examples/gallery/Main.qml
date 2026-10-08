// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic as B
import QtQuick.Layouts
import QCatUiKit

B.ApplicationWindow {
    id: window
    property bool darkTheme: false
    property int initialDensity: 0
    width: 1100
    height: 820
    minimumWidth: 360
    minimumHeight: 540
    visible: true
    title: "QCatUiKit " + LibraryInfo.version + " · Component Gallery"
    color: Theme.tokens.background
    TableModel {
        id: tableModel
        columns: ["component", "state", "platform"]
        headers: [qsTr("Component"), qsTr("Status"), qsTr("Platform")]
    }
    ChatModel {
        id: chatModel
    }
    Component.onCompleted: {
        Theme.colorScheme = darkTheme ? ThemePalette.Dark : ThemePalette.Light;
        Theme.density = initialDensity;
        tableModel.append({
            component: "Button",
            state: "Ready",
            platform: "Desktop / Touch / Mobile"
        });
        tableModel.append({
            component: "ThemePalette",
            state: "Live",
            platform: "Light / Dark"
        });
        tableModel.append({
            component: "TableModel",
            state: "Pimpl",
            platform: "Qt 6"
        });
        chatModel.append({
            sender: "guide",
            username: "QCatUiKit",
            content: qsTr("Welcome. Try the theme and density controls."),
            status: ChatModel.Sent
        });
    }
    B.ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth
        ColumnLayout {
            width: parent.width
            spacing: Theme.tokens.padding
            CatPane {
                Layout.fillWidth: true
                Layout.margins: 20
                ColumnLayout {
                    width: parent.width
                    CatLabel {
                        text: "QCatUiKit"
                        font.pixelSize: 30
                        font.bold: true
                    }
                    CatLabel {
                        text: qsTr("Qt 6 components · One theme across desktop, touch and mobile")
                        wrapMode: Text.Wrap
                        Layout.fillWidth: true
                    }
                    Flow {
                        Layout.fillWidth: true
                        spacing: Theme.tokens.spacing
                        CatButton {
                            text: Theme.colorScheme === ThemePalette.Dark ? qsTr("Light mode") : qsTr("Dark mode")
                            onClicked: Theme.colorScheme = Theme.colorScheme === ThemePalette.Dark ? ThemePalette.Light : ThemePalette.Dark
                        }
                        CatButton {
                            text: qsTr("Desktop")
                            checked: Theme.density === ThemePalette.Desktop
                            onClicked: Theme.density = ThemePalette.Desktop
                        }
                        CatButton {
                            text: qsTr("Touch")
                            checked: Theme.density === ThemePalette.Touch
                            onClicked: Theme.density = ThemePalette.Touch
                        }
                        CatButton {
                            text: qsTr("Mobile")
                            checked: Theme.density === ThemePalette.Mobile
                            onClicked: Theme.density = ThemePalette.Mobile
                        }
                        CatButton {
                            text: qsTr("Reduce motion")
                            checkable: true
                            onToggled: Theme.reducedMotion = checked
                        }
                    }
                }
            }
            GridLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                columns: window.width < 760 ? 1 : 2
                uniformCellWidths: true
                columnSpacing: 16
                rowSpacing: 16
                CatPane {
                    Layout.alignment: Qt.AlignTop
                    Layout.fillWidth: true
                    ColumnLayout {
                        width: parent.width
                        CatLabel {
                            text: qsTr("Actions & input")
                            font.bold: true
                            font.pixelSize: 20
                        }
                        Flow {
                            Layout.fillWidth: true
                            spacing: 8
                            CatButton {
                                text: qsTr("Primary")
                                primary: true
                                onClicked: popup.open()
                            }
                            CatButton {
                                text: qsTr("Default")
                            }
                            CatButton {
                                text: qsTr("Disabled")
                                enabled: false
                            }
                            CatButton {
                                text: qsTr("Loading")
                                loading: true
                            }
                        }
                        CatTextField {
                            Layout.fillWidth: true
                            placeholderText: qsTr("Keyboard, mouse or touch")
                        }
                        CatIpAddressField {
                            Layout.fillWidth: true
                        }
                        CatAlert {
                            Layout.fillWidth: true
                            text: qsTr("Theme changes apply immediately.")
                            severity: 1
                            closable: true
                        }
                        CatProgressBar {
                            Layout.fillWidth: true
                            value: 0.64
                        }
                        CatCircularProgress {
                            value: 64
                        }
                        CatPagination {
                            total: 95
                        }
                    }
                }
                CatCalendar {
                    Layout.alignment: Qt.AlignTop
                    Layout.fillWidth: true
                }
                CatPane {
                    Layout.alignment: Qt.AlignTop
                    Layout.fillWidth: true
                    ColumnLayout {
                        width: parent.width
                        CatLabel {
                            text: qsTr("Virtualized table")
                            font.bold: true
                            font.pixelSize: 20
                        }
                        CatTableView {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 220
                            model: tableModel
                        }
                    }
                }
                CatPane {
                    Layout.alignment: Qt.AlignTop
                    Layout.fillWidth: true
                    ColumnLayout {
                        width: parent.width
                        CatLabel {
                            text: qsTr("Conversation")
                            font.bold: true
                            font.pixelSize: 20
                        }
                        CatChatView {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 220
                            model: chatModel
                            currentUser: "me"
                        }
                        CatChatInput {
                            Layout.fillWidth: true
                            onSubmitted: text => chatModel.append({
                                    sender: "me",
                                    content: text,
                                    status: ChatModel.Sent
                                })
                        }
                    }
                }
            }
            Item {
                Layout.preferredHeight: 20
            }
        }
    }
    CatPopup {
        id: popup
        anchors.centerIn: parent
        width: Math.min(360, window.width - 32)
        contentItem: ColumnLayout {
            CatLabel {
                text: qsTr("Ready for your application")
                font.bold: true
            }
            CatLabel {
                text: qsTr("Escape or click outside to close.")
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }
            CatButton {
                text: qsTr("Close")
                onClicked: popup.close()
            }
        }
    }
}
