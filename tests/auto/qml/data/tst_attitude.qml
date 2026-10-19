// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtQuick.Window
import QtTest
import QCatUiKit

TestCase {
    id: test
    name: "AttitudeAndTiltUI"
    when: windowShown
    width: 800; height: 1000; visible: true
    Rectangle { id: backdrop; anchors.fill: parent; color: "#17212b" }
    Component {
        id: attitudeFactory
        CatAttitudeIndicator {
            width: 300; height: 300; attitudeValid: true
            pitchLadderVisible: false; rollScaleVisible: false; readoutVisible: false
        }
    }
    Component {
        id: panelFactory
        CatTiltControlPanel {
            width: 360
            controlEnabled: true; sampleFresh: true; controlAllowed: true
            controllerState: TiltController.Ready
        }
    }
    SignalSpy { id: engageSpy; signalName: "engageRequested" }
    SignalSpy { id: releaseSpy; signalName: "disengageRequested" }
    function init() { failOnWarning(/.*/); Theme.reducedMotion = true; }
    function cleanup() {
        engageSpy.target = null; releaseSpy.target = null;
        engageSpy.clear(); releaseSpy.clear(); Theme.reducedMotion = false;
    }
    function colorAt(image, x, y) {
        // Qt Quick Test images include the device pixel ratio.
        return image.pixel(Math.floor(x * image.width / 300), Math.floor(y * image.height / 300));
    }
    function test_aircraftDirections_data() {
        return [
            {tag: "level", pitch: 0, roll: 0, x: 100, y: 190, ground: true},
            {tag: "nose-up", pitch: 15, roll: 0, x: 100, y: 190, ground: false},
            {tag: "nose-down", pitch: -15, roll: 0, x: 100, y: 110, ground: true},
            {tag: "right-bank-left-sky", pitch: 0, roll: 45, x: 70, y: 150, ground: false},
            {tag: "right-bank-right-ground", pitch: 0, roll: 45, x: 230, y: 150, ground: true},
            {tag: "inverted", pitch: 0, roll: 180, x: 100, y: 70, ground: true},
            {tag: "vertical-up", pitch: 90, roll: 0, x: 100, y: 240, ground: false},
            {tag: "vertical-down", pitch: -90, roll: 0, x: 100, y: 60, ground: true}
        ];
    }
    function test_aircraftDirections(data) {
        const a = createTemporaryObject(attitudeFactory, test, {pitchDegrees: data.pitch, rollDegrees: data.roll});
        verify(waitForRendering(a));
        const img = grabImage(a);
        compare(colorAt(img, data.x, data.y), data.ground ? a.groundColor : a.skyColor);
        // The fixed aircraft marker remains at the instrument center.
        compare(colorAt(img, 150, 151), a.aircraftColor);
        compare(img.pixel(2, 2), backdrop.color, "Circular aperture must not paint its corners");
    }
    function test_invalidAndWrap() {
        const a = createTemporaryObject(attitudeFactory, test, {pitchDegrees: 10, rollDegrees: 179});
        verify(waitForRendering(a));
        const image = grabImage(a);
        a.rollDegrees = -181; wait(20);
        verify(image.equals(grabImage(a)));
        a.attitudeValid = false; verify(!a.displayValid);
        verify(waitForRendering(a)); verify(!image.equals(grabImage(a)));
        a.attitudeValid = true; a.pitchDegrees = NaN; verify(!a.displayValid);
        a.pitchDegrees = 91; verify(!a.displayValid);
        a.pitchDegrees = 0; a.rollDegrees = Infinity; verify(!a.displayValid);
        a.rollDegrees = 0; a.visiblePitchSpanDegrees = 0; verify(!a.displayValid);
    }
    function test_holdAndCancellation_data() {
        return ["release", "hidden", "disabled", "permission", "stale"].map(reason => ({tag: reason, reason: reason}));
    }
    function test_holdAndCancellation(data) {
        const p = createTemporaryObject(panelFactory, test);
        engageSpy.target = p; releaseSpy.target = p;
        verify(waitForRendering(p));
        const button = findChild(p, "tiltHoldButton"); verify(button !== null);
        mousePress(button, button.width / 2, button.height / 2);
        compare(engageSpy.count, 1); compare(p.holding, true);
        if (data.reason === "hidden") p.visible = false;
        if (data.reason === "disabled") p.enabled = false;
        if (data.reason === "permission") p.controlAllowed = false;
        if (data.reason === "stale") p.sampleFresh = false;
        mouseRelease(button, button.width / 2, button.height / 2);
        compare(p.holding, false); compare(releaseSpy.count, 1);
    }
    function test_keyboardAndProgrammaticState() {
        const p = createTemporaryObject(panelFactory, test);
        engageSpy.target = p; releaseSpy.target = p;
        verify(waitForRendering(p));
        p.Window.window.requestActivate(); tryCompare(p.Window.window, "active", true);
        const button = findChild(p, "tiltHoldButton"); button.forceActiveFocus(); tryCompare(button, "activeFocus", true);
        keyPress(Qt.Key_Space); compare(p.holding, true); compare(engageSpy.count, 1);
        keyRelease(Qt.Key_Space); compare(p.holding, false); compare(releaseSpy.count, 1);
        p.controllerState = TiltController.Active;
        compare(engageSpy.count, 1, "State synchronization must never request engagement");
    }
    function test_narrowMobile() {
        Theme.density = ThemePalette.Mobile;
        const p = createTemporaryObject(panelFactory, test, {width: 280});
        verify(waitForRendering(p));
        const hold = findChild(p, "tiltHoldButton");
        verify(hold.height >= Theme.tokens.controlHeight);
        const field = findChild(p, "tiltFullScaleField");
        verify(field.compact); verify(field.width <= p.availableWidth);
        Theme.density = ThemePalette.Desktop;
    }
    function test_touchAndDestroyWhileHolding() {
        const p = panelFactory.createObject(test);
        verify(p !== null);
        engageSpy.target = p; releaseSpy.target = p;
        verify(waitForRendering(p));
        const button = findChild(p, "tiltHoldButton");
        const touch = touchEvent(button);
        touch.press(0, button, button.width / 2, button.height / 2).commit();
        compare(p.holding, true); compare(engageSpy.count, 1);
        touch.release(0, button, button.width / 2, button.height / 2).commit();
        compare(p.holding, false); compare(releaseSpy.count, 1);
        mousePress(button, button.width / 2, button.height / 2);
        compare(p.holding, true);
        p.destroy(); wait(20);
        compare(releaseSpy.count, 2, "Destroying the panel must release its held control");
        mouseRelease(test, 790, 990);
    }
    function test_validSmallSpanDoesNotChangeParameters() {
        const p = createTemporaryObject(panelFactory, test, {fullScaleAngleDegrees: 5, deadzoneDegrees: 4.5});
        verify(waitForRendering(p));
        compare(findChild(p, "tiltFullScaleField").value, 5);
        compare(p.fullScaleAngleDegrees, 5);
        compare(p.deadzoneDegrees, 4.5);
    }
}
