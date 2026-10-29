// SPDX-License-Identifier: Apache-2.0
export const PROTOCOL = 1;
export const VERSION = '0.1.0';
export const BUS = 'org.qcat.MascotProbe1';
export const PATH = '/org/qcat/MascotProbe1';
export const TITLE_PREFIX = 'QCat mascot probe:';
export const XML = `<node><interface name="org.qcat.MascotProbe1">
  <method name="Hello"><arg type="u" direction="in"/><arg type="u" direction="out"/>
    <arg type="s" direction="out"/><arg type="s" direction="out"/></method>
  <method name="Call"><arg type="u" direction="in"/><arg type="s" direction="in"/>
    <arg type="s" direction="in"/><arg type="s" direction="in"/>
    <arg type="s" direction="out"/></method>
</interface></node>`;

export class ProtocolError extends Error {
    constructor(code, message) {
        super(message);
        this.code = code;
    }
}
export function requireProtocol(version) {
    if (version !== PROTOCOL)
        throw new ProtocolError('VersionMismatch', 'Only protocol 1 is implemented');
}
export function validateRequest(version, token, operation, json) {
    requireProtocol(version);
    if (!/^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$/.test(token))
        throw new ProtocolError('InvalidToken', 'Expected canonical lowercase UUID');
    if (!['snapshot', 'move', 'above'].includes(operation))
        throw new ProtocolError('InvalidOperation', 'Unknown window operation');
    if (typeof json !== 'string' || json.length > 4096)
        throw new ProtocolError('InvalidArguments', 'Arguments must be at most 4096 characters');
    let args;
    try { args = JSON.parse(json); } catch {
        throw new ProtocolError('InvalidArguments', 'Invalid JSON');
    }
    if (!args || Array.isArray(args) || typeof args !== 'object')
        throw new ProtocolError('InvalidArguments', 'Arguments must be an object');
    const fields = operation === 'move' ? ['x', 'y'] : operation === 'above' ? ['enabled'] : [];
    if (Object.keys(args).length !== fields.length || fields.some(key => !(key in args)))
        throw new ProtocolError('InvalidArguments', 'Unexpected or missing fields');
    if (operation === 'move' && [args.x, args.y].some(v => !Number.isInteger(v) || Math.abs(v) > 1000000))
        throw new ProtocolError('InvalidArguments', 'Coordinates must be bounded logical integers');
    if (operation === 'above' && typeof args.enabled !== 'boolean')
        throw new ProtocolError('InvalidArguments', 'enabled must be boolean');
    return args;
}
export function ownedWindow(windows, pid, token, waylandType) {
    const title = TITLE_PREFIX + token;
    const matches = windows.filter(window => window.get_pid() === pid &&
        window.get_client_type() === waylandType && window.get_title() === title);
    if (matches.length !== 1)
        throw new ProtocolError('WindowNotFound', 'Exactly one tagged native window of the caller is required');
    return matches[0];
}
export function snapshot(window) {
    const rect = window.get_frame_rect();
    return {pid: window.get_pid(), rect: [rect.x, rect.y, rect.width, rect.height],
        above: window.is_above(), focused: window.has_focus(), nativeWayland: true};
}
export function execute(window, operation, args) {
    if (operation === 'move') {
        if (!window.allows_move())
            throw new ProtocolError('Unavailable', 'Mutter does not allow moving this window');
        window.move_frame(false, args.x, args.y);
    } else if (operation === 'above') {
        if (args.enabled) window.make_above();
        else window.unmake_above();
    }
    const result = snapshot(window);
    result.applied = operation === 'move' ? result.rect[0] === args.x && result.rect[1] === args.y :
        operation === 'above' ? result.above === args.enabled : true;
    return result;
}
