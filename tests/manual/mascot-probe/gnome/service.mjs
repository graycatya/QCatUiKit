// SPDX-License-Identifier: Apache-2.0
import Gio from 'gi://Gio';
import GLib from 'gi://GLib';
import {PROTOCOL, VERSION, BUS, PATH, XML, ProtocolError, requireProtocol,
    validateRequest, ownedWindow, execute} from './protocol.mjs';

// Only the compositor adapter is injected, so the same service can be exercised on a private test bus.
export class WindowService {
    constructor(shellVersion, windows, waylandType) {
        this._shellVersion = shellVersion;
        this._windows = windows;
        this._waylandType = waylandType;
    }
    enable() {
        this._generation = (this._generation ?? 0) + 1;
        const generation = this._generation;
        this._pending = 0;
        this._managed = new Map();
        this._cancellable = new Gio.Cancellable();
        this._object = Gio.DBusExportedObject.wrapJSObject(XML, this);
        this._object.export(Gio.DBus.session, PATH);
        this._busId = Gio.bus_own_name_on_connection(Gio.DBus.session, BUS,
            Gio.BusNameOwnerFlags.NONE, null, () => {
                if (generation !== this._generation) return;
                // Do not leave an exported endpoint after losing its well-known name.
                ++this._generation;
                this._object?.unexport();
                this._object = null;
                this._cancellable?.cancel();
                this._restoreAll();
            });
        this._watchId = Gio.DBus.session.signal_subscribe('org.freedesktop.DBus',
            'org.freedesktop.DBus', 'NameOwnerChanged', '/org/freedesktop/DBus', null,
            Gio.DBusSignalFlags.NONE, (_c, _s, _p, _i, _n, parameters) => {
                if (generation !== this._generation) return;
                const [name, , owner] = parameters.deepUnpack();
                if (name.startsWith(':') && owner === '')
                    this._restoreSender(name);
            });
    }
    disable() {
        ++this._generation;
        this._cancellable?.cancel();
        this._object?.unexport();
        this._object = null;
        if (this._watchId) Gio.DBus.session.signal_unsubscribe(this._watchId);
        this._watchId = 0;
        if (this._busId) Gio.bus_unown_name(this._busId);
        this._busId = 0;
        this._restoreAll();
        this._cancellable = null;
    }
    _error(invocation, error) {
        const code = error instanceof ProtocolError ? error.code : 'Internal';
        invocation.return_dbus_error(`${BUS}.Error.${code}`, error.message);
    }
    HelloAsync([version], invocation) {
        try {
            requireProtocol(version);
            invocation.return_value(new GLib.Variant('(uss)', [PROTOCOL, VERSION, this._shellVersion]));
        } catch (error) { this._error(invocation, error); }
    }
    CallAsync([version, token, operation, json], invocation) {
        let args;
        try {
            args = validateRequest(version, token, operation, json);
            if (this._pending >= 32)
                throw new ProtocolError('Busy', 'Too many outstanding requests');
        } catch (error) {
            this._error(invocation, error);
            return;
        }
        const sender = invocation.get_sender();
        const generation = this._generation;
        ++this._pending;
        // Resolve the caller through the bus daemon. Never accept a PID from request JSON.
        Gio.DBus.session.call('org.freedesktop.DBus', '/org/freedesktop/DBus',
            'org.freedesktop.DBus', 'GetConnectionUnixProcessID', new GLib.Variant('(s)', [sender]),
            new GLib.VariantType('(u)'), Gio.DBusCallFlags.NONE, 1500, this._cancellable,
            (connection, result) => {
                try {
                    const [pid] = connection.call_finish(result).deepUnpack();
                    if (generation !== this._generation || !this._object)
                        throw new ProtocolError('Disconnected', 'Extension was disabled');
                    const windows = this._windows();
                    const window = ownedWindow(windows, pid, token, this._waylandType);
                    if (operation === 'above') this._remember(window, sender);
                    const response = execute(window, operation, args);
                    invocation.return_value(new GLib.Variant('(s)', [JSON.stringify(response)]));
                } catch (error) {
                    this._error(invocation, error);
                } finally {
                    // A late callback from an older enable cycle must not change its new quota.
                    if (generation === this._generation) --this._pending;
                }
            });
    }
    _remember(window, sender) {
        if (this._managed.has(window)) return;
        if (this._managed.size >= 64)
            throw new ProtocolError('Busy', 'Too many managed windows');
        const entry = {sender, originalAbove: window.is_above(), signal: 0};
        entry.signal = window.connect('unmanaged', () => {
            window.disconnect(entry.signal);
            this._managed.delete(window);
        });
        this._managed.set(window, entry);
    }
    _restore(window, entry) {
        try {
            window.disconnect(entry.signal);
            if (entry.originalAbove) window.make_above();
            else window.unmake_above();
        } catch (error) { console.warn(`QCat window cleanup: ${error.message}`); }
        this._managed.delete(window);
    }
    _restoreSender(sender) {
        for (const [window, entry] of this._managed ?? [])
            if (entry.sender === sender) this._restore(window, entry);
    }
    _restoreAll() {
        for (const [window, entry] of this._managed ?? []) this._restore(window, entry);
    }
}
