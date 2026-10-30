// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit
import QCatUiKit.Gallery

TestCase {
    id: test
    name: "GalleryMascot"
    when: windowShown
    width: 720; height: 700
    visible: true
    SignalSpy { id: actionSpy; signalName: "bubbleActionTriggered" }
    Component { id: factory; GalleryMascotPreview { width: 680 } }
    function init() { failOnWarning(/Binding loop|ReferenceError|TypeError/); GalleryLanguage.language = "en"; }
    function cleanup() {
        GalleryLanguage.language = "en"
        Theme.colorScheme = ThemePalette.Light
        Theme.density = ThemePalette.Desktop
    }
    function test_accessibleStatusAndKeyboard_data() {
        return [
            {tag: "desktop-light", density: ThemePalette.Desktop, scheme: ThemePalette.Light, width: 680},
            {tag: "desktop-dark", density: ThemePalette.Desktop, scheme: ThemePalette.Dark, width: 400},
            {tag: "touch-light", density: ThemePalette.Touch, scheme: ThemePalette.Light, width: 400},
            {tag: "touch-dark", density: ThemePalette.Touch, scheme: ThemePalette.Dark, width: 680},
            {tag: "mobile-light", density: ThemePalette.Mobile, scheme: ThemePalette.Light, width: 400},
            {tag: "mobile-dark", density: ThemePalette.Mobile, scheme: ThemePalette.Dark, width: 400}
        ]
    }
    function test_accessibleStatusAndKeyboard(data) {
        Theme.density = data.density
        Theme.colorScheme = data.scheme
        const preview = createTemporaryObject(factory, test, {width: data.width})
        const pet = findChild(preview, "galleryMascotPet")
        tryCompare(pet, "status", MascotPetController.Ready, 10000)
        const status = findChild(preview, "mascotStatus")
        verify(status.text.indexOf("Ready") >= 0)
        verify(status.text.indexOf("Off") >= 0)
        const show = findChild(preview, "mascotShow")
        show.forceActiveFocus()
        keyClick(Qt.Key_Space)
        tryCompare(pet, "visible", true)
        pet.hide()
        GalleryLanguage.language = "zh_CN"
        tryVerify(() => status.text.indexOf("就绪") >= 0)
        verify(status.text.indexOf("关闭") >= 0)
        compare(show.Accessible.name, "显示桌宠")
        verify(findChild(preview, "mascotCapability_inputRegions").text.indexOf("区域输入") >= 0)
        const regions = findChild(preview, "mascotRegions")
        compare(regions.Accessible.name, "区域输入")
        compare(regions.enabled, pet.capabilities.inputRegions.status !== "Unavailable")
        compare(regions.checked, false)
        pet.inputRegionsEnabled = true
        tryCompare(regions, "checked", true)
        const play = findChild(preview, "mascotPlay_" + pet.availableActions[0])
        verify(play.Accessible.name.indexOf("播放动作") >= 0)
        verify(show.width >= show.implicitWidth)
        verify(show.height >= show.implicitHeight)
        verify(status.width <= preview.width)
    }
    function test_nativeBubbleKeepsRenderedPosition_data() {
        return [
            {tag: "sprite", source: "qrc:/gallery-mascot/catgray/manifest.json"},
            {tag: "lottie", source: "qrc:/gallery-mascot/catgray-lottie/mascot.json"}
        ]
    }
    function test_nativeBubbleKeepsRenderedPosition(data) {
        if (!nativeMascotTest.nativeCocoa()) skip("Requires Cocoa native rendering")
        const preview = createTemporaryObject(factory, test)
        const pet = findChild(preview, "galleryMascotPet")
        pet.packageSource = data.source
        nativeMascotTest.activateHost(test)
        mouseClick(findChild(preview, "mascotShow"))
        tryCompare(pet, "visible", true)
        tryCompare(pet, "status", MascotPetController.Ready, 30000)
        wait(200)
        const character = findChild(pet.contentItem, "mascotCharacter")
        verify(character !== null)
        const before = nativeMascotTest.nativePosition(character)
        const tolerance = nativeMascotTest.nativePositionTolerance(character)
        const checkPosition = actual => {
            verify(Math.abs(actual.x - before.x) <= tolerance && Math.abs(actual.y - before.y) <= tolerance,
                   "Actual rendered position " + actual + "; expected " + before)
        }
        console.info("Before Qt", pet.x, pet.y, pet.width, pet.height, character.x, character.y)
        mouseClick(findChild(preview, "mascotShow"))
        wait(200)
        console.info("Repeated show button", nativeMascotTest.nativePosition(character))
        checkPosition(nativeMascotTest.nativePosition(character))
        for (let i = 0; i < 3; ++i) {
            mouseClick(findChild(preview, "mascotBubble"))
            wait(200)
            const shown = nativeMascotTest.nativePosition(character)
            console.info("Actual character origin", before, shown, "Qt", pet.x, pet.y,
                         "local", character.x, character.y)
            checkPosition(shown)
            pet.closeBubble(pet.bubbleId)
            wait(200)
            checkPosition(nativeMascotTest.nativePosition(character))
        }
        pet.hide()
    }
    function test_hostControlsIndependentWindows() {
        const preview = createTemporaryObject(factory, test)
        verify(preview !== null)
        const pet = findChild(preview, "galleryMascotPet")
        const second = findChild(preview, "galleryMascotSecond")
        verify(pet !== null && second !== null)
        tryCompare(pet, "status", MascotPetController.Ready, 10000)
        compare(pet.visible, false)
        mouseClick(findChild(preview, "mascotShow"))
        tryCompare(pet, "visible", true)
        mouseClick(findChild(preview, "mascotBubble"))
        verify(pet.bubbleId.length > 0)
        compare(second.bubbleId, "")
        actionSpy.target = pet
        actionSpy.clear()
        const reply = findChild(pet.contentItem, "mascotBubbleAction_reply")
        verify(reply !== null)
        mouseClick(reply)
        compare(actionSpy.count, 1)
        compare(actionSpy.signalArguments[0][1], "reply")
        verify(pet.bubbleId.length > 0)
        actionSpy.target = null
        pet.close()
        tryCompare(pet, "visible", false)
        compare(second.visible, false)
        compare(pet.controller.packageSource, second.controller.packageSource)
    }
}
