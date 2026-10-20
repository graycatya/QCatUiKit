// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic as B
import QtQuick.Layouts
import QCatUiKit

ColumnLayout {
    id: demo
    objectName: "dockPreview"
    property string savedLayout: ""
    property string initialLayout: ""
    property real previewOpacity: 0.8
    spacing: Theme.tokens.spacing
    CatLabel {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        text: qsTr("Drag tabs to dock, or right-click a tab to float it or its group. Dock back preserves content and restores its previous location.")
    }
    Flow {
        Layout.fillWidth: true
        spacing: Theme.tokens.spacing
        CatButton {
            text: qsTr("Save layout")
            onClicked: demo.savedLayout = workspace.controller.saveLayout()
        }
        CatButton {
            text: qsTr("Restore layout")
            enabled: demo.savedLayout.length > 0
            onClicked: workspace.controller.restoreLayout(demo.savedLayout)
        }
        CatButton {
            text: qsTr("Reset layout")
            onClicked: workspace.controller.restoreLayout(demo.initialLayout)
        }
        CatButton {
            text: qsTr("Show log")
            onClicked: workspace.controller.openPanel("log")
        }
        CatButton {
            objectName: "floatDockEditor"
            text: qsTr("Float editor")
            enabled: !workspace.layoutLocked && !workspace.controller.floatingWindows.some(window => window.tabs.indexOf("editor") >= 0)
            onClicked: workspace.floatPanel("editor")
        }
        CatButton {
            objectName: "floatDockGroup"
            text: qsTr("Float editor group")
            enabled: !workspace.layoutLocked && !workspace.controller.floatingWindows.some(window => window.tabs.indexOf("editor") >= 0)
            onClicked: workspace.floatPanel("editor", true)
        }
        CatButton {
            objectName: "returnDockWindows"
            text: qsTr("Dock all windows back")
            enabled: !workspace.layoutLocked && workspace.controller.floatingWindows.length > 0
            onClicked: {
                const windows = workspace.controller.floatingWindows;
                for (const window of windows)
                    workspace.controller.dockBack(window.id);
            }
        }
        CatSwitch {
            text: qsTr("Lock layout")
            onToggled: workspace.layoutLocked = checked
        }
    }
    B.ScrollView {
        id: viewport
        Layout.fillWidth: true
        Layout.preferredHeight: 460
        contentWidth: Math.max(availableWidth, workspace.minimumDockWidth)
        contentHeight: availableHeight
        B.ScrollBar.vertical.policy: B.ScrollBar.AlwaysOff
        CatDockWorkspace {
            id: workspace
            objectName: "demoDockWorkspace"
            width: viewport.contentWidth
            height: viewport.availableHeight
            CatDockPanel {
                dockId: "project"
                title: qsTr("Project")
                minimumDockWidth: 130
                onCloseRequested: workspace.controller.closePanel(dockId)
                CatLabel {
                    anchors.fill: parent
                    text: qsTr("Workspace\n  Components\n  Themes\n  Tests")
                    wrapMode: Text.Wrap
                }
            }
            CatDockPanel {
                dockId: "editor"
                title: qsTr("Editor")
                closable: false
                CatTextArea {
                    anchors.fill: parent
                    text: qsTr("Edit this text, then move this panel. Your changes stay here.")
                    wrapMode: TextEdit.Wrap
                }
            }
            CatDockPanel {
                dockId: "properties"
                title: qsTr("Properties")
                onCloseRequested: workspace.controller.closePanel(dockId)
                ColumnLayout {
                    anchors.fill: parent
                    CatLabel {
                        text: qsTr("Preview opacity")
                    }
                    CatSlider {
                        Layout.fillWidth: true
                        from: 0
                        to: 1
                        value: demo.previewOpacity
                        onMoved: demo.previewOpacity = value
                    }
                    Item {
                        Layout.fillHeight: true
                    }
                }
            }
            CatDockPanel {
                dockId: "log"
                title: qsTr("Log")
                onCloseRequested: workspace.controller.closePanel(dockId)
                CatTextArea {
                    anchors.fill: parent
                    readOnly: true
                    text: qsTr("Workspace ready.\nLayout changes preserve panel instances.")
                    wrapMode: TextEdit.Wrap
                }
            }
            CatDockPanel {
                dockId: "preview"
                title: qsTr("Chart preview")
                onCloseRequested: workspace.controller.closePanel(dockId)
                CatChart {
                    anchors.fill: parent
                    opacity: demo.previewOpacity
                    legendVisible: false
                    CatChartSeries {
                        name: qsTr("Signal")
                        Component.onCompleted: setData([
                            {
                                x: 0,
                                y: 1
                            },
                            {
                                x: 1,
                                y: 3
                            },
                            {
                                x: 2,
                                y: 2
                            },
                            {
                                x: 3,
                                y: 4
                            }
                        ])
                    }
                }
            }
            Component.onCompleted: Qt.callLater(function () {
                controller.dock("project", "editor", DockController.Left);
                controller.dock("properties", "editor", DockController.Right);
                controller.dock("log", "editor", DockController.Bottom);
                controller.setSplitRatio(controller.layout.id, 0.24);
                controller.setSplitRatio(controller.layout.second.id, 0.70);
                controller.setSplitRatio(controller.layout.second.first.id, 0.67);
                controller.activatePanel("editor");
                demo.initialLayout = controller.saveLayout();
            })
        }
    }
    CatLabel {
        Layout.fillWidth: true
        visible: viewport.availableWidth < workspace.minimumDockWidth
        wrapMode: Text.Wrap
        text: qsTr("Scroll horizontally to reach all panels, or merge panels to fit the workspace.")
        color: Theme.tokens.textSecondary
    }
}
