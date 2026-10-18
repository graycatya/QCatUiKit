// SPDX-License-Identifier: Apache-2.0
.pragma library

function clamp(value, low, high) { return Math.max(low, Math.min(high, value)); }
function position(value, from, to) {
    return to === from ? 0 : clamp((value - from) / (to - from), 0, 1);
}
function format(value, locale, decimals, suffix, formatter) {
    return typeof formatter === "function" ? String(formatter(value))
        : Number(value).toLocaleString(locale, "f", clamp(decimals, 0, 12)) + suffix;
}
// Bound delegate count even for a tiny interval over a very large domain.
function ticks(from, to, interval, values) {
    const low = Math.min(from, to), high = Math.max(from, to);
    if (!isFinite(low) || !isFinite(high) || !isFinite(high - low) || low === high) return [];
    if (values && values.length)
        return values.filter(v => isFinite(v) && v >= low && v <= high)
            .sort((a, b) => a - b).filter((v, i, all) => !i || v !== all[i - 1]).slice(0, 201);
    if (!(interval > 0) || !isFinite(interval)) return [];
    const count = Math.floor((high - low) / interval);
    if (!isFinite(count)) return Array.from({length: 201}, (_, i) => low + (i / 200) * (high - low));
    const stride = Math.max(1, Math.ceil(count / 200));
    const result = [];
    for (let i = 0; i <= count && result.length < 200; i += stride)
        result.push(low + i * interval);
    if (!result.length || Math.abs(result[result.length - 1] - high) > (high - low) * 1e-10)
        result.push(high);
    return result;
}
// Increasing offsets from 'from' also support reversed numeric ranges.
function constrain(first, second, from, to, minimum, maximum, fixed, anchor) {
    const length = Math.abs(to - from), direction = to >= from ? 1 : -1;
    let a = clamp((first - from) * direction, 0, length);
    let b = clamp((second - from) * direction, a, length);
    const min = clamp(isFinite(minimum) ? minimum : 0, 0, length);
    const max = clamp(isFinite(maximum) ? maximum : length, min, length);
    if (fixed >= 0 && isFinite(fixed)) {
        const span = clamp(fixed, min, max);
        a = clamp(anchor === 2 ? b - span : a, 0, length - span);
        b = a + span;
    } else if (anchor === 1) {
        b = Math.max(b, min);
        a = clamp(a, Math.max(0, b - max), b - min);
    } else {
        a = Math.min(a, length - min);
        b = clamp(b, a + min, Math.min(length, a + max));
    }
    return [from + a * direction, from + b * direction];
}
