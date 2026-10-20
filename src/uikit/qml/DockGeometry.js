// SPDX-License-Identifier: Apache-2.0
.pragma library

function measure(node, panels, header, gap) {
    if (!node || !node.kind) return { width: 0, height: 0 }
    if (node.kind === "tabs") {
        let w = 0, h = 0
        for (const id of node.tabs) {
            const p = panels[id]
            if (p) { w = Math.max(w, p.minimumDockWidth); h = Math.max(h, p.minimumDockHeight) }
        }
        return { width: w, height: h + header }
    }
    const a = measure(node.first, panels, header, gap), b = measure(node.second, panels, header, gap)
    return node.orientation === "horizontal" ? { width: a.width + gap + b.width, height: Math.max(a.height, b.height) }
                                              : { width: Math.max(a.width, b.width), height: a.height + gap + b.height }
}
function calculate(root, panels, width, height, header, gap) {
    const minimum = measure(root, panels, header, gap)
    const result = { groups: [], handles: [], minimumWidth: minimum.width, minimumHeight: minimum.height }
    function visit(n, x, y, w, h) {
        if (!n || !n.kind) return
        if (n.kind === "tabs") {
            result.groups.push({ id: n.id, tabs: n.tabs, active: n.active, x: x, y: y, width: w, height: h })
            return
        }
        const horizontal = n.orientation === "horizontal"
        const available = Math.max(0, (horizontal ? w : h) - gap)
        const a = measure(n.first, panels, header, gap), b = measure(n.second, panels, header, gap)
        const amin = horizontal ? a.width : a.height, bmin = horizontal ? b.width : b.height
        const length = Math.max(amin, Math.min(available - bmin, available * n.ratio))
        result.handles.push({ id: n.id, horizontal: horizontal, origin: horizontal ? x : y, available: available,
                              x: horizontal ? x + length : x, y: horizontal ? y : y + length,
                              width: horizontal ? gap : w, height: horizontal ? h : gap,
                              minRatio: available > 0 ? amin / available : 0.5,
                              maxRatio: available > 0 ? 1 - bmin / available : 0.5, ratio: n.ratio })
        visit(n.first, x, y, horizontal ? length : w, horizontal ? h : length)
        visit(n.second, horizontal ? x + length + gap : x, horizontal ? y : y + length + gap,
              horizontal ? available - length : w, horizontal ? h : available - length)
    }
    // Preserve constraints. The workspace clips overflowing content and exposes minima to its owner.
    visit(root, 0, 0, Math.max(width, minimum.width), Math.max(height, minimum.height))
    return result
}
