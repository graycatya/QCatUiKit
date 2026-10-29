// SPDX-License-Identifier: Apache-2.0
import Meta from 'gi://Meta';
import {Extension} from 'resource:///org/gnome/shell/extensions/extension.js';
import * as Config from 'resource:///org/gnome/shell/misc/config.js';
import {WindowService} from './service.mjs';

export default class MascotProbeExtension extends Extension {
    enable() {
        // Shell 50 removed the X11 compositor and Meta.is_wayland_compositor().
        // Keep the strict Shell version gate; window ownership also requires WAYLAND.
        if (Config.PACKAGE_VERSION.split('.')[0] !== '50')
            throw new Error('QCat window probe requires GNOME Shell 50 on native Wayland');
        this._service = new WindowService(Config.PACKAGE_VERSION,
            () => global.get_window_actors().map(actor => actor.meta_window), Meta.WindowClientType.WAYLAND);
        this._service.enable();
    }
    disable() {
        this._service?.disable();
        this._service = null;
    }
}
