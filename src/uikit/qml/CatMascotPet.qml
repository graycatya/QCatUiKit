// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic as B
import QtQuick.Layouts
import QCatUiKit

Window {
    id: root
    property ThemePalette theme: Theme
    readonly property alias controller: actor
    property alias packageSource: actor.packageSource
    property alias petScale: actor.petScale
    property alias facing: actor.facing
    property alias dragEnabled: actor.dragEnabled
    property alias dragAction: actor.dragAction
    property alias actionCacheLimitMiB: actor.actionCacheLimitMiB
    readonly property alias status: actor.status
    readonly property alias errorCode: actor.errorCode
    readonly property alias errorString: actor.errorString
    readonly property alias availableActions: actor.availableActions
    readonly property alias currentAction: actor.currentAction
    property alias inputRegionsEnabled: desktop.inputRegionsEnabled
    property alias inputTransparent: desktop.inputTransparent
    property alias alwaysOnTop: desktop.alwaysOnTop
    property alias constrainToAvailableGeometry: desktop.constrainToAvailableGeometry
    readonly property alias capabilities: desktop.capabilities
    readonly property alias screens: desktop.screens
    property bool closeHides: true
    readonly property string platformStatus: "Pending"
    readonly property string bubbleId: bubbleState.identifier
    readonly property string bubbleText: bubbleState.text
    property bool smooth: true
    readonly property real effectivePetScale: actor.petScale
    readonly property real bubbleSpaceAbove: Math.max(0, desktop.characterPosition.y - desktop.availableGeometry.y - 8)
    readonly property real bubbleSpaceBelow: Math.max(0, desktop.availableGeometry.y + desktop.availableGeometry.height - desktop.characterPosition.y - sprite.height - 8)
    readonly property real preferredBubbleHeight: Math.min(message.implicitHeight, 240) + closeMessage.implicitHeight + bubbleActions.implicitHeight + 40
    readonly property bool bubbleAbove: bubbleSpaceAbove >= preferredBubbleHeight || bubbleSpaceAbove >= bubbleSpaceBelow
    readonly property real bubbleCharacterOffsetX: Math.max(0, Math.min(Math.max(0, bubble.width - sprite.width), desktop.characterPosition.x + bubble.width - desktop.availableGeometry.x - desktop.availableGeometry.width))
    readonly property bool bubbleAlignRight: bubbleCharacterOffsetX > 0
    signal moveEnded(string requestId, string reason)
    signal platformError(string code, string detail)
    signal bubbleClosed(string id, string reason)
    signal bubbleActionTriggered(string id, string actionId)
    signal regionActivated(string name, point logicalPosition)
    signal dragStarted()
    signal dragFinished()
    color: "transparent"
    readonly property int contentWidth: Math.ceil(Math.max(1, sprite.width, root.bubbleId.length > 0 ? bubble.width : 0))
    readonly property int contentHeight: Math.ceil(Math.max(1, sprite.height + (root.bubbleId.length > 0 ? bubble.height + 8 : 0)))
    width: contentWidth
    height: contentHeight
    minimumWidth: desktop.capabilities.systemDrag.status === "Unavailable" ? 0 : contentWidth
    maximumWidth: desktop.capabilities.systemDrag.status === "Unavailable" ? 16777215 : contentWidth
    minimumHeight: desktop.capabilities.systemDrag.status === "Unavailable" ? 0 : contentHeight
    maximumHeight: desktop.capabilities.systemDrag.status === "Unavailable" ? 16777215 : contentHeight
    function syncContentSize() {
        if (width !== contentWidth) width = contentWidth
        if (height !== contentHeight) height = contentHeight
    }
    onContentWidthChanged: Qt.callLater(syncContentSize)
    onContentHeightChanged: Qt.callLater(syncContentSize)
    onWidthChanged: Qt.callLater(syncContentSize)
    onHeightChanged: Qt.callLater(syncContentSize)
    transientParent: null
    flags: Qt.Tool | Qt.FramelessWindowHint | Qt.WindowDoesNotAcceptFocus | Qt.WindowStaysOnTopHint
    title: qsTr("Desktop character")
    visible: false
    onClosing: close => {
        if (closeHides) { close.accepted = false; hide() }
    }
    onVisibleChanged: bubbleState.syncTimer()
    function requestAction(name, policy = MascotPetController.Replace, options = {}) {
        return actor.requestAction(name, policy, options)
    }
    function pauseActions() { actor.pauseActions() }
    function resumeActions() { actor.resumeActions() }
    function resetToDefault() { actor.resetToDefault() }
    function setPosition(screenId: string, point: point): bool { return desktop.setPosition(screenId, point) }
    function moveTo(screenId: string, point: point, durationMs: int): string { return desktop.moveTo(screenId, point, durationMs) }
    function cancelMove() { desktop.cancelMove() }
    function exportState(): var { return desktop.exportState() }
    function restoreState(state: var): var { return desktop.restoreState(state) }
    function showBubble(text, options = {}) {
        const duration = options.durationMs === undefined ? 0 : options.durationMs
        const actions = options.actions === undefined ? [] : options.actions
        if (!Number.isInteger(duration) || duration < 0 || duration > 2147483647 || !Array.isArray(actions) || actions.length > 8)
            return ""
        const ids = []
        for (const action of actions) {
            if (typeof action.id !== "string" || !action.id || typeof action.text !== "string" || ids.indexOf(action.id) >= 0)
                return ""
            ids.push(action.id)
        }
        const generation = bubbleState.serial
        bubbleState.finish("Replaced")
        if (bubbleState.serial !== generation) return ""
        bubbleState.serial++
        bubbleState.identifier = String(bubbleState.serial)
        bubbleState.text = text
        bubbleState.actions = actions.slice()
        bubbleState.remaining = duration
        bubbleState.syncTimer()
        return bubbleState.identifier
    }
    function updateBubble(id: string, text: string): bool {
        if (!id || id !== bubbleId) return false
        bubbleState.text = text
        return true
    }
    function appendBubbleText(id: string, text: string): bool {
        return updateBubble(id, bubbleText + text)
    }
    function closeBubble(id: string): bool {
        if (!id || id !== bubbleId) return false
        bubbleState.finish("Closed")
        return true
    }
    MascotPetController {
        id: actor
        onPackageSourceChanged: bubbleState.finish("PackageChanged")
        onRegionActivated: (name, logicalPosition) => root.regionActivated(name, logicalPosition)
        onDragStarted: root.dragStarted()
        onDragFinished: root.dragFinished()
    }
    MascotDesktop {
        id: desktop
        window: root
        controller: actor
        characterOffset: Qt.point(sprite.x, sprite.y)
        bubbleRect: (root.bubbleId.length > 0) ? Qt.rect(bubble.x, bubble.y, bubble.width, bubble.height) : Qt.rect(0, 0, 0, 0)
        onMoveEnded: (requestId, reason) => root.moveEnded(requestId, reason)
        onPlatformError: (code, detail) => root.platformError(code, detail)
    }
    MascotSprite {
        id: sprite
        objectName: "mascotCharacter"
        x: (root.bubbleId.length > 0) ? root.bubbleCharacterOffsetX : 0
        y: (root.bubbleId.length > 0) && root.bubbleAbove ? bubble.height + 8 : 0
        width: Math.max(actor.status === MascotPetController.Error ? 160 : 0, implicitWidth)
        height: Math.max(actor.status === MascotPetController.Error ? 160 : 0, implicitHeight)
        controller: actor
        theme: root.theme
        smooth: root.smooth
    }
    QtObject {
        id: bubbleState
        property int serial: 0
        property string identifier: ""
        property string text: ""
        property var actions: []
        property double remaining: 0
        property double started: 0
        function syncTimer() {
            if (expiry.running) {
                remaining = Math.max(1, remaining - (Date.now() - started))
                expiry.stop()
            }
            if (root.visible && identifier && remaining > 0) {
                started = Date.now()
                expiry.interval = Math.max(1, Math.ceil(remaining))
                expiry.start()
            }
        }
        function finish(reason: string) {
            expiry.stop()
            const old = identifier
            identifier = ""
            text = ""
            actions = []
            remaining = 0
            if (old) root.bubbleClosed(old, reason)
        }
    }
    Timer {
        id: expiry
        onTriggered: bubbleState.finish("TimedOut")
    }
    Rectangle {
        id: bubble
        objectName: "mascotBubble"
        x: 0
        y: root.bubbleAbove ? 0 : sprite.height + 8
        width: Math.max(1, Math.min(320, desktop.availableGeometry.width ? desktop.availableGeometry.width : 320))
        height: Math.max(1, Math.min(root.preferredBubbleHeight, root.bubbleAbove ? root.bubbleSpaceAbove : root.bubbleSpaceBelow))
        clip: true
        visible: !!root.bubbleId
        color: (root.theme || Theme).tokens.background
        border.color: (root.theme || Theme).tokens.primary
        radius: 8
        ColumnLayout {
            id: bubbleLayout
            anchors.fill: parent
            anchors.margins: 12
            spacing: 8
            CatButton {
                id: closeMessage
                objectName: "mascotBubbleClose"
                Layout.alignment: Qt.AlignRight
                theme: root.theme
                text: qsTr("Close")
                Accessible.name: qsTr("Close character message")
                KeyNavigation.priority: KeyNavigation.BeforeItem
                KeyNavigation.tab: bubbleScroll
                KeyNavigation.backtab: bubbleActionRepeater.count > 0 ? bubbleActionRepeater.itemAt(bubbleActionRepeater.count - 1) : bubbleScroll
                onClicked: root.closeBubble(root.bubbleId)
            }
            B.ScrollView {
                id: bubbleScroll
                objectName: "mascotBubbleScroll"
                focusPolicy: Qt.StrongFocus
                KeyNavigation.priority: KeyNavigation.BeforeItem
                KeyNavigation.tab: bubbleActionRepeater.count > 0 ? bubbleActionRepeater.itemAt(0) : closeMessage
                KeyNavigation.backtab: closeMessage
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredHeight: Math.min(message.implicitHeight, 240)
                clip: true
                contentWidth: availableWidth
                CatLabel {
                    id: message
                    width: bubble.width - 40
                    theme: root.theme
                    text: root.bubbleText
                    Accessible.name: root.bubbleText
                    Accessible.role: Accessible.StaticText
                    textFormat: Text.PlainText
                    wrapMode: Text.Wrap
                }
            }
            Flow {
                id: bubbleActions
                Layout.fillWidth: true
                spacing: 4
                Repeater {
                    id: bubbleActionRepeater
                    model: bubbleState.actions
                    CatButton {
                        id: actionButton
                        required property var modelData
                        required property int index
                        KeyNavigation.priority: KeyNavigation.BeforeItem
                        KeyNavigation.tab: index + 1 < bubbleActionRepeater.count ? bubbleActionRepeater.itemAt(index + 1) : closeMessage
                        KeyNavigation.backtab: index > 0 ? bubbleActionRepeater.itemAt(index - 1) : bubbleScroll
                        objectName: "mascotBubbleAction_" + modelData.id
                        theme: root.theme
                        text: modelData.text
                        onClicked: {
                            desktop.requestKeyboardFocus()
                            actionButton.forceActiveFocus(Qt.MouseFocusReason)
                            root.bubbleActionTriggered(root.bubbleId, modelData.id)
                        }
                    }
                }
            }
        }
    }
}
