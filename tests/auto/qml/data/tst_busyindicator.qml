// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit

TestCase {
    id: test
    name: "BusyIndicator"
    when: windowShown
    width: 96
    height: 96
    visible: true

    ThemePalette {
        id: palette
        primaryColor: "#0000ff"
    }
    CatBusyIndicator {
        id: spinner
        theme: palette
        width: 64
        height: 64
    }

    function init() {
        failOnWarning(/.*/);
        spinner.visible = true;
        spinner.running = true;
        palette.reducedMotion = false;
        verify(waitForRendering(spinner));
        wait(50);
    }

    // Read the rendered arc's orientation rather than an internal QML property:
    // render-thread Animator types do not update that property on every frame.
    function angle() {
        const image = grabImage(spinner);
        let xWeight = 0, yWeight = 0, total = 0;
        for (let y = 0; y < image.height; y += 2) {
            for (let x = 0; x < image.width; x += 2) {
                const weight = Math.max(0, image.blue(x, y) - image.red(x, y));
                xWeight += (x + 0.5 - image.width / 2) * weight;
                yWeight += (y + 0.5 - image.height / 2) * weight;
                total += weight;
            }
        }
        verify(total > 0, "The loading arc must be rendered");
        return Math.atan2(yWeight, xWeight) * 180 / Math.PI;
    }

    function angleDelta(next, previous) {
        return (next - previous + 540) % 360 - 180;
    }

    function test_rotationSpeed() {
        let previous = angle(), rotation = 0;
        const start = Date.now();
        for (let i = 0; i < 16; ++i) {
            wait(25);
            const next = angle();
            rotation += angleDelta(next, previous);
            previous = next;
        }
        const elapsed = Date.now() - start;
        const expected = 360 * elapsed / 900;
        verify(Math.abs(rotation - expected) < 45, "Expected a 900 ms turn; measured " + rotation.toFixed(1) + " degrees in " + elapsed + " ms");
    }

    function test_pause_data() {
        return [
            {
                tag: "running",
                mode: "running"
            },
            {
                tag: "reducedMotion",
                mode: "reducedMotion"
            },
            {
                tag: "hidden",
                mode: "hidden"
            }
        ];
    }

    function test_pause(data) {
        let stoppedAngle = angle();
        if (data.mode === "running")
            spinner.running = false;
        else if (data.mode === "reducedMotion")
            palette.reducedMotion = true;
        else
            spinner.visible = false;

        // A hidden item cannot be grabbed; the resume check below covers it.
        wait(50);
        const before = data.mode === "hidden" ? null : grabImage(spinner);
        if (before)
            stoppedAngle = angle();
        wait(250);
        if (before)
            verify(before.equals(grabImage(spinner)), "A stopped spinner must stay still");

        spinner.running = true;
        palette.reducedMotion = false;
        spinner.visible = true;
        verify(waitForRendering(spinner));
        let previous = angle();
        verify(Math.abs(angleDelta(previous, stoppedAngle)) < 55, "Resuming must exclude time spent stopped");
        wait(60);
        verify(angleDelta(angle(), previous) > 5, "The spinner must resume rotating");
    }
}
