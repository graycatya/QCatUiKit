// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit
import QCatUiKit.Gallery

TestCase {
    id: test
    name: "GalleryAttitudeAndTilt"
    when: windowShown
    width: 900; height: 1200; visible: true
    Component { id: aircraftFactory; GalleryAttitudePreview { width: 360 } }
    Component { id: phoneFactory; GalleryTiltPreview { x: 420; width: 360 } }
    function init() { failOnWarning(/.*/); GalleryLanguage.language = "en"; Theme.reducedMotion = true; }
    function cleanup() { GalleryLanguage.language = "en"; Theme.reducedMotion = false; }
    function test_independentSourcesAndLifecycle() {
        const aircraft = createTemporaryObject(aircraftFactory, test);
        const phone = createTemporaryObject(phoneFactory, test);
        const c = phone.controller;
        c.enabled = true;
        tryCompare(c, "sampleFresh", true);
        verify(c.calibrate());
        tryCompare(c, "state", TiltController.Ready, 3000);
        phone.phoneRoll = 25;
        tryVerify(() => c.rollInput > .95);
        phone.phonePitch = -10;
        tryVerify(() => c.pitchInput < -.2);
        compare(aircraft.indicator.pitchDegrees, 12); compare(aircraft.indicator.rollDegrees, 25);
        aircraft.aircraftRoll = -40;
        compare(phone.phoneRoll, 25);
        verify(c.engage()); verify(c.outputValid);
        phone.allowControl = false;
        compare(c.outputValid, false);
        phone.pauseInput = true;
        tryCompare(c, "state", TiltController.Fault, 2000);
        phone.pauseInput = false; c.resetCalibration();
        tryCompare(c, "sampleFresh", true);
        phone.visible = false;
        compare(c.enabled, false); compare(c.outputValid, false);
    }
    function test_translationAndBindings() {
        const phone = createTemporaryObject(phoneFactory, test);
        const panel = findChild(phone, "demoTiltControlPanel");
        const enable = findChild(panel, "tiltEnableSwitch");
        compare(enable.text, "Enable tilt input");
        GalleryLanguage.language = "zh_CN";
        tryCompare(enable, "text", "启用体感输入");
        panel.fullScaleAngleEdited(35); compare(phone.controller.fullScaleAngleDegrees, 35);
        phone.controller.fullScaleAngleDegrees = 20;
        compare(panel.fullScaleAngleDegrees, 20);
        compare(findChild(panel, "tiltFullScaleField").value, 20);
    }
}
