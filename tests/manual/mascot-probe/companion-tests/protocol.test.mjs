// SPDX-License-Identifier: Apache-2.0
import assert from 'node:assert/strict';
import test from 'node:test';
import {TITLE_PREFIX, validateRequest, ownedWindow, execute} from '../gnome/protocol.mjs';
const token = '01234567-89ab-cdef-0123-456789abcdef';
function window(pid = 12, id = token, type = 1) {
    return {rect: {x: 2, y: 3, width: 420, height: 320}, above: false, movable: true,
        get_pid: () => pid, get_title: () => TITLE_PREFIX + id, get_client_type: () => type,
        get_frame_rect() { return this.rect; }, is_above() { return this.above; }, has_focus: () => false,
        allows_move() { return this.movable; }, move_frame(_user, x, y) { this.rect = {...this.rect, x, y}; },
        make_above() { this.above = true; }, unmake_above() { this.above = false; }};
}
test('protocol, token, operations and arguments are rejected before mutation', () => {
    for (const args of [[2, token, 'move', '{}'], [1, 'bad', 'snapshot', '{}'],
        [1, token, 'activate', '{}'], [1, token, 'move', '{"x":1,"y":2,"pid":12}'],
        [1, token, 'move', '{"x":1.5,"y":2}'], [1, token, 'above', '{"enabled":1}'],
        [1, token, 'snapshot', '[]'], [1, token, 'snapshot', 'x'.repeat(4097)]])
        assert.throws(() => validateRequest(...args));
    assert.deepEqual(validateRequest(1, token, 'move', '{"x":-20,"y":30}'), {x:-20, y:30});
});
test('same token on another PID or XWayland window cannot select it', () => {
    const own = window();
    assert.equal(ownedWindow([window(99), window(12, token, 0), own], 12, token, 1), own);
    assert.throws(() => ownedWindow([window(99)], 12, token, 1));
    assert.throws(() => ownedWindow([own, window()], 12, token, 1));
    assert.throws(() => ownedWindow([own], 12, 'aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa', 1));
});
test('native readback determines whether a request actually applied', () => {
    const own = window();
    assert.equal(execute(own, 'move', {x: 50, y: 70}).applied, true);
    assert.deepEqual(execute(own, 'snapshot', {}).rect, [50, 70, 420, 320]);
    assert.equal(execute(own, 'above', {enabled: true}).above, true);
    own.move_frame = () => {};
    assert.equal(execute(own, 'move', {x: 500, y: 700}).applied, false);
    own.movable = false;
    assert.throws(() => execute(own, 'move', {x: 0, y: 0}));
});
