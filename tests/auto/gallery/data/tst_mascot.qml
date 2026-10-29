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
    function init() { failOnWarning(/Binding loop|ReferenceError|TypeError/); }
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
