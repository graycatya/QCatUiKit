// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic as B
import QtQuick.Layouts
import QCatUiKit

Item {
    id: control
    required property string componentId
    property int requestedChartMode: -1
    readonly property int chartMode: (exampleLoader.item as GalleryChartPreview)?.mode ?? -1
    readonly property string exampleCode: (exampleLoader.item as GalleryChartPreview)?.exampleCode
        || (exampleLoader.item as GallerySliderPreview)?.exampleCode
        || (exampleLoader.item as GalleryVectorPreview)?.exampleCode
        || (exampleLoader.item as GalleryMascotPreview)?.exampleCode || ""
    function selectChartMode(mode: int): void {
        const preview = exampleLoader.item as GalleryChartPreview;
        if (preview && preview.allowedModes.includes(mode)) preview.mode = mode;
    }
    implicitHeight: Math.max(240, exampleLoader.implicitHeight + 48)
    clip: true
    Loader {
        id: exampleLoader
        anchors.centerIn: parent
        width: Math.max(0, control.width - 8)
        sourceComponent: {
            if (GalleryChartCatalog.entryForId(control.componentId)) return chartScene;
            switch (control.componentId) {
            case "svg-image": case "lottie-animation": case "vector-canvas": return vectorScene;
            case "mascot-pet": return mascotScene;
            case "dock-workspace": return dockScene;
            case "attitude-indicator": return attitudeScene;
            case "tilt-control": return tiltScene;
            case "led-preview":
                return ledScene;
            case "button":
                return buttonScene;
            case "checkbox":
                return checkBoxScene;
            case "radio":
                return radioScene;
            case "switch":
                return switchScene;
            case "slider": case "rangeslider": case "sliderfield":
                return sliderScene;
            case "combobox":
                return comboBoxScene;
            case "textfield":
                return fieldScene;
            case "ipaddress":
                return ipScene;
            case "label":
                return labelScene;
            case "pane":
                return paneScene;
            case "alert":
                return alertScene;
            case "busy":
                return busyScene;
            case "progress":
                return progressScene;
            case "circular":
                return circularScene;
            case "tooltip":
                return tooltipScene;
            case "pagination":
                return paginationScene;
            case "calendar":
                return calendarScene;
            case "popup":
                return popupScene;
            case "drawer":
                return drawerScene;
            case "table": case "chat": case "chatinput":
                return dataScene;
            case "flip":
                return flipScene;
            case "marquee":
                return marqueeScene;
            case "typewriter":
                return typewriterScene;
            case "image":
                return imageScene;
            case "imagemotion":
                return imageMotionScene;
            case "gradient":
                return gradientScene;
            case "effect":
                return effectScene;
            case "drag":
                return dragScene;
            case "resize":
                return resizeScene;
            case "fps":
                return fpsScene;
            default:
                return extendedScene;
            }
        }
    }
    Component { id: mascotScene; GalleryMascotPreview {} }
    Component { id: vectorScene; GalleryVectorPreview { componentId: control.componentId } }
    Component { id: dockScene; GalleryDockPreview {} }
    Component { id: attitudeScene; GalleryAttitudePreview {} }
    Component { id: tiltScene; GalleryTiltPreview {} }
    Component {
        id: ledScene
        GalleryLedPreview {}
    }
    Component {
        id: chartScene
        GalleryChartPreview { componentId: control.componentId; requestedMode: control.requestedChartMode }
    }
    Component {
        id: extendedScene
        GalleryExtendedPreview { componentId: control.componentId }
    }
    Component {
        id: buttonScene
        ColumnLayout {
            id: buttons
            property int activations: 0
            spacing: 20
            Flow {
                Layout.fillWidth: true
                spacing: 12
                CatButton {
                    text: qsTr("Continue")
                    primary: true
                    onClicked: buttons.activations++
                }
                CatButton {
                    text: qsTr("Default")
                    onClicked: buttons.activations++
                }
                CatButton {
                    text: qsTr("Danger")
                    danger: true
                    onClicked: buttons.activations++
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
            CatLabel {
                text: qsTr("Activated %1 times").arg(buttons.activations)
                color: Theme.tokens.textSecondary
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
        }
    }
    Component {
        id: checkBoxScene
        ColumnLayout {
            spacing: 12
            CatCheckBox { id: check; objectName: "demoCheckBox"; text: qsTr("Enable notifications"); checked: true }
            CatCheckBox { text: qsTr("Select all"); tristate: true; checkState: Qt.PartiallyChecked }
            CatCheckBox { text: qsTr("Disabled"); checked: true; enabled: false }
            CatLabel { text: check.checked ? qsTr("Notifications enabled") : qsTr("Notifications disabled"); color: Theme.tokens.textSecondary }
        }
    }
    Component {
        id: radioScene
        ColumnLayout {
            spacing: 12
            CatRadioButton { text: qsTr("Local workspace"); checked: true }
            CatRadioButton { text: qsTr("Cloud workspace") }
            CatRadioButton { text: qsTr("Disabled"); enabled: false }
        }
    }
    Component {
        id: switchScene
        ColumnLayout {
            spacing: 12
            CatSwitch { id: wifi; objectName: "demoSwitch"; text: qsTr("Wi-Fi"); checked: true }
            CatSwitch { text: qsTr("Airplane mode") }
            CatSwitch { text: qsTr("Disabled"); enabled: false }
            CatLabel { text: wifi.checked ? qsTr("Connected") : qsTr("Disconnected"); color: Theme.tokens.textSecondary }
        }
    }
    Component {
        id: sliderScene
        GallerySliderPreview { componentId: control.componentId }
    }
    Component {
        id: comboBoxScene
        ColumnLayout {
            spacing: 12
            CatLabel { text: qsTr("Workspace") }
            CatComboBox { objectName: "demoComboBox"; model: [qsTr("Local workspace"), qsTr("Cloud workspace")]; Layout.fillWidth: true; Accessible.name: qsTr("Workspace") }
            CatComboBox { editable: true; model: [qsTr("Desktop"), qsTr("Touch"), qsTr("Mobile")]; Layout.fillWidth: true; Accessible.name: qsTr("Interaction density") }
            CatComboBox { model: [qsTr("Disabled")]; enabled: false; Layout.fillWidth: true }
        }
    }
    Component {
        id: fieldScene
        ColumnLayout {
            spacing: 16
            CatLabel {
                text: qsTr("Your message")
                font.bold: true
            }
            CatTextField {
                id: field
                objectName: "demoTextField"
                placeholderText: qsTr("Type something…")
                Layout.fillWidth: true
            }
            CatLabel {
                text: qsTr("%1 characters").arg(field.text.length)
                color: Theme.tokens.textSecondary
            }
        }
    }
    Component {
        id: ipScene
        ColumnLayout {
            spacing: 16
            CatLabel {
                text: qsTr("Server address")
                font.bold: true
            }
            CatIpAddressField {
                id: ip
                text: "192.168.1.1"
                Layout.fillWidth: true
            }
            CatLabel {
                text: ip.acceptableInput ? qsTr("Valid IPv4 address") : qsTr("Enter an IPv4 address")
                color: Theme.tokens.textSecondary
            }
        }
    }
    Component {
        id: labelScene
        ColumnLayout {
            spacing: 16
            CatLabel {
                text: qsTr("Make room for good design.")
                font.pixelSize: 28
                font.bold: true
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
            CatLabel {
                text: qsTr("Clear hierarchy makes an interface easier to read.")
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
            CatLabel {
                text: qsTr("Secondary text adds context without competing for attention.")
                color: Theme.tokens.textSecondary
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
        }
    }
    Component {
        id: paneScene
        CatPane {
            padding: 24
            contentItem: ColumnLayout {
                spacing: 12
                CatLabel {
                    text: qsTr("A place for your content.")
                    font.pixelSize: 20
                    font.bold: true
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                }
                CatLabel {
                    text: qsTr("Group related controls on a consistent surface.")
                    color: Theme.tokens.textSecondary
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                }
                CatButton {
                    text: qsTr("Continue")
                    primary: true
                }
            }
        }
    }
    Component {
        id: alertScene
        ColumnLayout {
            spacing: 16
            GalleryComboBox {
                id: severitySelector
                model: [qsTr("Information"), qsTr("Success"), qsTr("Warning"), qsTr("Error")]
                currentIndex: 1
                Accessible.name: qsTr("Alert severity")
            }
            CatAlert {
                id: alert
                text: qsTr("Your changes have been saved.")
                severity: severitySelector.currentIndex
                closable: true
                Layout.fillWidth: true
            }
            CatButton {
                visible: !alert.visible
                text: qsTr("Show alert")
                onClicked: alert.visible = true
            }
        }
    }
    Component {
        id: busyScene
        ColumnLayout {
            spacing: 24
            RowLayout {
                spacing: 24
                CatBusyIndicator {
                    running: runButton.checked
                    Layout.preferredWidth: 18
                    Layout.preferredHeight: 18
                }
                CatBusyIndicator {
                    running: runButton.checked
                    Layout.preferredWidth: 32
                    Layout.preferredHeight: 32
                }
                CatBusyIndicator {
                    running: runButton.checked
                    Layout.preferredWidth: 48
                    Layout.preferredHeight: 48
                }
                CatLabel {
                    text: qsTr("Working on it…")
                    color: Theme.tokens.textSecondary
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                }
            }
            CatButton {
                id: runButton
                text: checked ? qsTr("Pause") : qsTr("Resume")
                checkable: true
                checked: true
            }
            CatLabel {
                text: qsTr("One turn every 900 ms. Reduced motion stops the animation.")
                color: Theme.tokens.textSecondary
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }
        }
    }
    Component {
        id: progressScene
        ColumnLayout {
            spacing: 20
            CatProgressBar {
                value: progress.value / 100
                Layout.fillWidth: true
            }
            CatSlider {
                id: progress
                objectName: "progressSlider"
                from: 0
                to: 100
                value: 64
                stepSize: 1; suffix: "%"
                tickInterval: 25; tickLabelsVisible: true; valueToolTipVisible: true
                defaultValue: 64; defaultIndicatorVisible: true
                Layout.fillWidth: true
                Accessible.name: qsTr("Progress")
            }
            CatButton { objectName: "progressReset"; text: qsTr("Reset"); onClicked: progress.reset() }
            CatLabel {
                text: qsTr("%1% complete").arg(Math.round(progress.value))
                color: Theme.tokens.textSecondary
            }
        }
    }
    Component {
        id: circularScene
        ColumnLayout {
            spacing: 20
            CatCircularProgress {
                value: progress.value
                Layout.alignment: Qt.AlignHCenter
            }
            CatSlider {
                id: progress
                objectName: "progressSlider"
                from: 0
                to: 100
                value: 64
                stepSize: 1; suffix: "%"
                tickInterval: 25; tickLabelsVisible: true; valueToolTipVisible: true
                defaultValue: 64; defaultIndicatorVisible: true
                Layout.fillWidth: true
                Accessible.name: qsTr("Progress")
            }
            CatButton { objectName: "progressReset"; text: qsTr("Reset"); onClicked: progress.reset() }
        }
    }
    Component {
        id: tooltipScene
        ColumnLayout {
            spacing: 20
            CatButton {
                id: helpButton
                text: qsTr("Hover over me")
                CatToolTip {
                    text: qsTr("A little context, right where you need it.")
                    visible: helpButton.hovered
                }
            }
            CatLabel {
                text: qsTr("Leave the pointer over the button to see the tooltip.")
                color: Theme.tokens.textSecondary
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }
        }
    }
    Component {
        id: paginationScene
        ColumnLayout {
            spacing: 20
            CatPagination {
                id: pager
                total: 190
                pageSize: 10
                Layout.fillWidth: true
            }
            CatLabel {
                text: qsTr("Current page: %1").arg(pager.currentPage)
                color: Theme.tokens.textSecondary
            }
        }
    }
    Component {
        id: calendarScene
        ColumnLayout {
            spacing: 16
            CatCalendar {
                id: calendar
                objectName: "calendarPreview"
                locale: Qt.locale(GalleryLanguage.localeName)
                Layout.fillWidth: true
            }
            CatLabel {
                text: qsTr("Selected: %1").arg(calendar.selectedDate.toLocaleDateString(Qt.locale(GalleryLanguage.localeName), Locale.ShortFormat))
                color: Theme.tokens.textSecondary
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }
        }
    }
    Component {
        id: popupScene
        Item {
            implicitHeight: 150
            ColumnLayout {
                anchors.fill: parent
                spacing: 16
                CatButton {
                    objectName: "openPopupButton"
                    text: qsTr("Open popup")
                    primary: true
                    onClicked: popup.open()
                }
                CatLabel {
                    text: qsTr("Escape or click outside to close.")
                    color: Theme.tokens.textSecondary
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }
            }
            CatPopup {
                id: popup
                objectName: "demoPopup"
                anchors.centerIn: parent
                width: Math.min(320, control.width)
                contentItem: ColumnLayout {
                    spacing: 16
                    CatLabel {
                        text: qsTr("A moment of focus.")
                        font.pixelSize: 20
                        font.bold: true
                        Layout.fillWidth: true
                        wrapMode: Text.Wrap
                    }
                    CatLabel {
                        text: qsTr("Keep the important action within reach.")
                        Layout.fillWidth: true
                        wrapMode: Text.Wrap
                    }
                    CatButton {
                        text: qsTr("Close")
                        onClicked: popup.close()
                    }
                }
            }
        }
    }
    Component {
        id: drawerScene
        Item {
            implicitHeight: 150
            ColumnLayout {
                anchors.fill: parent
                spacing: 16
                CatButton {
                    objectName: "openDrawerButton"
                    text: qsTr("Open drawer")
                    primary: true
                    onClicked: drawer.open()
                }
                CatLabel {
                    text: qsTr("A side panel keeps the current page in view.")
                    color: Theme.tokens.textSecondary
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                }
            }
            CatDrawer {
                id: drawer
                objectName: "demoDrawer"
                width: Math.min(300, control.width)
                height: B.Overlay.overlay ? B.Overlay.overlay.height : 400
                contentItem: ColumnLayout {
                    spacing: 16
                    CatLabel {
                        text: qsTr("More room for the details.")
                        font.pixelSize: 20
                        font.bold: true
                        Layout.fillWidth: true
                        wrapMode: Text.Wrap
                    }
                    CatLabel {
                        text: qsTr("Navigation, settings or supporting content can live here.")
                        Layout.fillWidth: true
                        wrapMode: Text.Wrap
                    }
                    CatButton {
                        text: qsTr("Close")
                        onClicked: drawer.close()
                    }
                    Item {
                        Layout.fillHeight: true
                    }
                }
            }
        }
    }
    Component {
        id: dataScene
        GalleryDataPreview { componentId: control.componentId }
    }
    Component {
        id: flipScene
        ColumnLayout {
            spacing: 20
            CatFlipView {
                id: flip
                Layout.preferredWidth: 200
                Layout.preferredHeight: 120
                Layout.alignment: Qt.AlignHCenter
                front: CatPane {
                    anchors.fill: parent
                    contentItem: CatLabel {
                        text: qsTr("Front")
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                back: CatPane {
                    anchors.fill: parent
                    contentItem: CatLabel {
                        text: qsTr("Back")
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
            CatButton {
                text: qsTr("Flip card")
                Layout.alignment: Qt.AlignHCenter
                onClicked: flip.flipped = !flip.flipped
            }
        }
    }
    Component {
        id: marqueeScene
        Item {
            implicitHeight: marquee.implicitHeight
            CatMarquee {
                id: marquee
                objectName: "marqueePreview"
                anchors.horizontalCenter: parent.horizontalCenter
                width: Math.min(200, parent.width)
                text: qsTr("Thoughtful details make everyday interfaces feel better.")
                duration: 3500
            }
        }
    }
    Component {
        id: typewriterScene
        ColumnLayout {
            spacing: 20
            CatTypewriter {
                id: writer
                fullText: qsTr("Every good interface starts with a small detail.")
                interval: 70
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
            CatButton {
                text: qsTr("Restart")
                onClicked: writer.restart()
            }
        }
    }
    Component {
        id: imageScene
        Item {
            implicitHeight: Math.min(240, width * 280 / 480)
            CatImage {
                anchors.fill: parent
                source: Qt.resolvedUrl("assets/preview-art.svg")
                sourceSize: Qt.size(480, 280)
            }
        }
    }
    Component {
        id: imageMotionScene
        Item {
            implicitHeight: 210
            CatImageMotion {
                anchors.centerIn: parent
                source: Qt.resolvedUrl("assets/preview-art.svg")
                sourceSize: Qt.size(480, 280)
                width: Math.min(280, parent.width)
                height: width * 280 / 480
                fromScale: 0.94
                toScale: 1
                fromOpacity: 0.65
                toOpacity: 1
            }
        }
    }
    Component {
        id: gradientScene
        Item {
            implicitHeight: 220
            CatConicalGradient {
                width: 180
                height: 180
                anchors.centerIn: parent
                startColor: Theme.tokens.primary
                endColor: "#a8d6ff"
            }
        }
    }
    Component {
        id: effectScene
        Item {
            id: effectPreview
            implicitHeight: 210
            readonly property bool software: GraphicsInfo.api === GraphicsInfo.Software
            CatImage {
                id: picture
                source: Qt.resolvedUrl("assets/preview-art.svg")
                sourceSize: Qt.size(480, 280)
                width: Math.min(240, parent.width - 24)
                height: width * 280 / 480
                anchors.centerIn: parent
                visible: effectPreview.software
            }
            CatEffect {
                source: picture
                anchors.fill: picture
                visible: !effectPreview.software
            }
        }
    }
    Component {
        id: dragScene
        Item {
            implicitHeight: 210
            Rectangle {
                id: tile
                x: 32
                y: 48
                width: 160
                height: 90
                radius: 10
                color: Theme.tokens.surfaceHover
                border.color: Theme.tokens.primary
                CatLabel {
                    anchors.centerIn: parent
                    text: qsTr("Drag me")
                    color: Theme.tokens.primary
                }
                CatDragHandle {
                    anchors.fill: parent
                    target: tile
                }
            }
        }
    }
    Component {
        id: resizeScene
        Item {
            id: resizeArea
            implicitHeight: 230
            Rectangle {
                id: tile
                x: 8
                y: 24
                width: Math.min(200, resizeArea.width - 16)
                height: 110
                radius: 10
                color: Theme.tokens.surfaceHover
                border.color: Theme.tokens.primary
                CatLabel {
                    anchors.centerIn: parent
                    text: qsTr("Resize me")
                    color: Theme.tokens.primary
                }
                CatResizeHandle {
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    width: 24
                    height: 24
                    target: tile
                    maximumWidth: Math.min(300, resizeArea.width - 16)
                    maximumHeight: 180
                }
            }
        }
    }
    Component {
        id: fpsScene
        ColumnLayout {
            spacing: 20
            CatFpsCounter {
                font.pixelSize: 32
                font.bold: true
            }
            CatBusyIndicator {
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
            }
            CatLabel {
                text: qsTr("The counter reports frame callbacks, not monitor refresh rate.")
                color: Theme.tokens.textSecondary
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }
        }
    }
}
