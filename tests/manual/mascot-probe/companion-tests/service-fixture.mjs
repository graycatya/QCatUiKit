// SPDX-License-Identifier: Apache-2.0
// A real GJS/D-Bus service with only Meta.Window replaced. Never enabled in GNOME Shell.
import GLib from 'gi://GLib';
import Gio from 'gi://Gio';
import {WindowService} from '../gnome/service.mjs';
import {TITLE_PREFIX} from '../gnome/protocol.mjs';
const pid = Number(ARGV[0]);
if (ARGV[1] !== 'untrusted')
    Gio.bus_own_name_on_connection(Gio.DBus.session, 'org.gnome.Shell', Gio.BusNameOwnerFlags.NONE, null, null);
const tokens = ['01234567-89ab-cdef-0123-456789abcdef', 'aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa'];
const windows = tokens.map(token => ({
    rect: {x: 10, y: 20, width: 420, height: 320}, above: false,
    get_pid: () => pid, get_title: () => TITLE_PREFIX + token, get_client_type: () => 1,
    get_frame_rect() { return this.rect; }, is_above() { return this.above; }, has_focus: () => false,
    allows_move: () => true, move_frame(_user, x, y) { this.rect = {...this.rect, x, y}; },
    make_above() { this.above = true; }, unmake_above() { this.above = false; },
    connect: () => 1, disconnect: () => {},
}));
const service = new WindowService('50.1', () => windows, 1);
if (ARGV[1] === 'incompatible') {
    service.HelloAsync = (_args, invocation) => {
        invocation.return_value(new GLib.Variant('(uss)', [2, '99.0.0', '50.1']));
    };
}
if (ARGV[1] === 'slow') {
    const call = service.CallAsync.bind(service);
    service.CallAsync = (params, invocation) => {
        GLib.timeout_add(GLib.PRIORITY_DEFAULT, 3000, () => {
            call(params, invocation);
            return GLib.SOURCE_REMOVE;
        });
    };
}
if (ARGV[1] === 'malformed') {
    service.CallAsync = (_params, invocation) => {
        invocation.return_value(new GLib.Variant('(s)', ['{"applied":true}']));
    };
}
service.enable();
GLib.unix_signal_add(GLib.PRIORITY_DEFAULT, 10, () => {
    service.disable();
    return GLib.SOURCE_CONTINUE;
});
GLib.unix_signal_add(GLib.PRIORITY_DEFAULT, 12, () => {
    service.enable();
    return GLib.SOURCE_CONTINUE;
});
new GLib.MainLoop(null, false).run();
