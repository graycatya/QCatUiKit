// SPDX-License-Identifier: Apache-2.0
#include "qcatmascotmacosinput_p.h"
#import <AppKit/AppKit.h>
#include <QCoreApplication>
#include <QEnterEvent>
#include <QMouseEvent>
#include <QPointer>
#include <QTimer>
#include <QWheelEvent>
#include <cstdint>
#include <functional>
#include <unordered_set>
#include <vector>

@interface QCatMascotInputPanel : NSPanel
@end
@implementation QCatMascotInputPanel
- (BOOL)canBecomeKeyWindow
{
    return NO;
}
- (BOOL)canBecomeMainWindow
{
    return NO;
}
@end

@interface QCatMascotInputView : NSView
{
  @public
    std::function<void(NSEvent *)> dispatch;
}
@end
@implementation QCatMascotInputView
- (void)deliver:(NSEvent *)event
{
    [self retain];
    const auto callback = dispatch;
    if (callback)
        callback(event);
    [self release];
}
- (BOOL)isFlipped
{
    return YES;
}
- (BOOL)acceptsFirstMouse:(NSEvent *)event
{
    Q_UNUSED(event);
    return YES;
}
- (void)updateTrackingAreas
{
    NSArray *previous = [self.trackingAreas copy];
    for (NSTrackingArea *area in previous)
        [self removeTrackingArea:area];
    [previous release];
    auto *area = [[NSTrackingArea alloc]
        initWithRect:NSZeroRect
             options:NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved |
                     NSTrackingActiveAlways | NSTrackingInVisibleRect
               owner:self
            userInfo:nil];
    [self addTrackingArea:area];
    [area release];
    [super updateTrackingAreas];
}
- (void)mouseDown:(NSEvent *)event
{
    [self deliver:event];
}
- (void)mouseUp:(NSEvent *)event
{
    [self deliver:event];
}
- (void)mouseDragged:(NSEvent *)event
{
    [self deliver:event];
}
- (void)mouseMoved:(NSEvent *)event
{
    [self deliver:event];
}
- (void)rightMouseDown:(NSEvent *)event
{
    [self deliver:event];
}
- (void)rightMouseUp:(NSEvent *)event
{
    [self deliver:event];
}
- (void)rightMouseDragged:(NSEvent *)event
{
    [self deliver:event];
}
- (void)otherMouseDown:(NSEvent *)event
{
    [self deliver:event];
}
- (void)otherMouseUp:(NSEvent *)event
{
    [self deliver:event];
}
- (void)otherMouseDragged:(NSEvent *)event
{
    [self deliver:event];
}
- (void)mouseEntered:(NSEvent *)event
{
    [self deliver:event];
}
- (void)mouseExited:(NSEvent *)event
{
    [self deliver:event];
}
- (void)scrollWheel:(NSEvent *)event
{
    [self deliver:event];
}
@end

namespace QCatUiKit
{
struct MascotMacosInput::Private : std::enable_shared_from_this<Private>
{
    QPointer<QQuickWindow> target;
    NSWindow *owner = nil;
    std::vector<QCatMascotInputPanel *> surfaces;
    QRegion pending;
    Qt::MouseButtons buttons;
    QString error;
    bool enabled = false, stopped = false, applied = false;
    QRegion appliedRegion;
    NSRect appliedFrame{};
    NSInteger appliedLevel = 0;
    bool restackPending = false;
    std::vector<std::pair<NSNotificationCenter *, id>> observations;

    explicit Private(QQuickWindow *window) : target(window) {}
    ~Private()
    {
        clear();
    }
    void cancel()
    {
        const bool grabbed = bool(buttons);
        buttons = {};
        if (grabbed && target)
        {
            QEvent event(QEvent::UngrabMouse);
            QCoreApplication::sendEvent(target, &event);
        }
    }
    void clear()
    {
        for (const auto &[center, token] : observations)
        {
            [center removeObserver:token];
            [token release];
        }
        observations.clear();
        for (auto *surface : surfaces)
        {
            static_cast<QCatMascotInputView *>(surface.contentView)->dispatch = {};
            [surface orderOut:nil];
            [surface close];
            [surface release];
        }
        surfaces.clear();
        applied = false;
        [owner release];
        owner = nil;
    }
    void requestRestack()
    {
        if (restackPending || stopped || !target)
            return;
        restackPending = true;
        std::weak_ptr<Private> weak = shared_from_this();
        QTimer::singleShot(0, target,
                           [weak]
                           {
                               if (auto state = weak.lock())
                               {
                                   state->restackPending = false;
                                   state->applied = false;
                                   state->apply();
                               }
                           });
    }
    void observe(NSNotificationCenter *center, NSNotificationName name, id object)
    {
        const std::weak_ptr<Private> weak = shared_from_this();
        id token = [center addObserverForName:name
                                       object:object
                                        queue:NSOperationQueue.mainQueue
                                   usingBlock:^(NSNotification *) {
                                     if (auto state = weak.lock())
                                         state->requestRestack();
                                   }];
        observations.emplace_back(center, [token retain]);
    }
    void observeOwner()
    {
        for (auto name :
             {NSWindowDidMoveNotification, NSWindowDidResizeNotification,
              NSWindowDidChangeScreenNotification, NSWindowDidExposeNotification,
              NSWindowDidChangeOcclusionStateNotification, NSWindowDidMiniaturizeNotification,
              NSWindowDidDeminiaturizeNotification, NSWindowDidUpdateNotification})
            observe(NSNotificationCenter.defaultCenter, name, owner);
        observe(NSNotificationCenter.defaultCenter, NSWindowDidBecomeKeyNotification, nil);
        observe(NSWorkspace.sharedWorkspace.notificationCenter,
                NSWorkspaceDidActivateApplicationNotification, nil);
        observe(NSWorkspace.sharedWorkspace.notificationCenter,
                NSWorkspaceActiveSpaceDidChangeNotification, nil);
    }
    void shutdown()
    {
        stopped = true;
        cancel();
        clear();
        target.clear();
    }
    static Qt::KeyboardModifiers modifiers(NSEvent *event)
    {
        Qt::KeyboardModifiers result;
        if (event.modifierFlags & NSEventModifierFlagShift)
            result |= Qt::ShiftModifier;
        if (event.modifierFlags & NSEventModifierFlagOption)
            result |= Qt::AltModifier;
        if (event.modifierFlags & NSEventModifierFlagCommand)
            result |= Qt::ControlModifier;
        if (event.modifierFlags & NSEventModifierFlagControl)
            result |= Qt::MetaModifier;
        return result;
    }
    QPointF localPoint(NSPoint screen) const
    {
        NSView *view = reinterpret_cast<NSView *>(target->winId());
        const NSPoint local = [view convertPoint:[view.window convertPointFromScreen:screen]
                                        fromView:nil];
        const NSRect bounds = view.bounds;
        return QPointF((local.x - bounds.origin.x) * target->width() / bounds.size.width,
                       (local.y - bounds.origin.y) * target->height() / bounds.size.height);
    }
    void route(NSEvent *native)
    {
        if (stopped || !target || !enabled || !target->isVisible())
            return;
        // Keep the state and sender alive if a Qt callback deletes the pet or
        // changes its action/geometry during synchronous event delivery.
        const auto lifetime = shared_from_this();
        NSWindow *sender = [native.window retain];
        const NSPoint screen = [sender convertPointToScreen:native.locationInWindow];
        const QPointF local = localPoint(screen);
        const QPointF global = target->mapToGlobal(local);
        const auto mods = modifiers(native);
        QEvent::Type type = QEvent::MouseMove;
        Qt::MouseButton button = Qt::NoButton;
        switch (native.type)
        {
        case NSEventTypeLeftMouseDown:
        case NSEventTypeRightMouseDown:
        case NSEventTypeOtherMouseDown:
            type = native.clickCount == 2 ? QEvent::MouseButtonDblClick : QEvent::MouseButtonPress;
            break;
        case NSEventTypeLeftMouseUp:
        case NSEventTypeRightMouseUp:
        case NSEventTypeOtherMouseUp:
            type = QEvent::MouseButtonRelease;
            break;
        case NSEventTypeMouseEntered:
        {
            QEnterEvent event(local, local, global);
            QCoreApplication::sendEvent(target, &event);
            [sender release];
            return;
        }
        case NSEventTypeMouseExited:
        {
            // Crossing adjacent input rectangles stays inside the same Qt window.
            const NSPoint now = NSEvent.mouseLocation;
            if (!pending.contains(localPoint(now).toPoint()) && !buttons)
            {
                QEvent event(QEvent::Leave);
                QCoreApplication::sendEvent(target, &event);
            }
            [sender release];
            return;
        }
        case NSEventTypeScrollWheel:
        {
            const QPoint pixels =
                native.hasPreciseScrollingDeltas
                    ? QPoint(qRound(native.scrollingDeltaX), qRound(native.scrollingDeltaY))
                    : QPoint();
            QWheelEvent event(
                local, global, pixels,
                QPoint(qRound(native.scrollingDeltaX * 8), qRound(native.scrollingDeltaY * 8)),
                buttons, mods, Qt::NoScrollPhase, native.isDirectionInvertedFromDevice);
            event.setTimestamp(quint64(native.timestamp * 1000));
            QCoreApplication::sendEvent(target, &event);
            [sender release];
            return;
        }
        default:
            break;
        }
        if (type != QEvent::MouseMove)
        {
            switch (native.buttonNumber)
            {
            case 0:
                button = Qt::LeftButton;
                break;
            case 1:
                button = Qt::RightButton;
                break;
            case 2:
                button = Qt::MiddleButton;
                break;
            case 3:
                button = Qt::BackButton;
                break;
            case 4:
                button = Qt::ForwardButton;
                break;
            default:
                break;
            }
            if (type == QEvent::MouseButtonRelease)
                buttons &= ~button;
            else
                buttons |= button;
        }
        QMouseEvent event(type, local, local, global, button, buttons, mods);
        event.setTimestamp(quint64(native.timestamp * 1000));
        QCoreApplication::sendEvent(target, &event);
        [sender release];
        if (!stopped && target && !buttons)
            apply();
    }
    bool needsOrdering() const
    {
        if (surfaces.empty())
            return false;
        CFArrayRef above = CGWindowListCreate(kCGWindowListOptionOnScreenAboveWindow |
                                                  kCGWindowListExcludeDesktopElements,
                                              CGWindowID(owner.windowNumber));
        if (!above)
            return true;
        const CFIndex count = CFArrayGetCount(above);
        bool reorder = count < CFIndex(surfaces.size());
        std::unordered_set<CGWindowID> expected;
        for (auto *surface : surfaces)
            expected.insert(CGWindowID(surface.windowNumber));
        if (!reorder)
            for (CFIndex i = count - CFIndex(surfaces.size()); i < count; ++i)
                if (!expected.erase(
                        CGWindowID(reinterpret_cast<uintptr_t>(CFArrayGetValueAtIndex(above, i)))))
                    reorder = true;
        CFRelease(above);
        return reorder;
    }
    bool apply()
    {
        if (stopped || !target)
            return true;
        // Empty input regions must also leave the visual window transparent.
        // Qt may have reapplied flags before this sync, including on first expose.
        if (target->isVisible())
        {
            auto *visual = reinterpret_cast<NSView *>(target->winId());
            visual.window.ignoresMouseEvents = YES;
            // The pet must outlive host application activation, unlike a tool
            // palette. Apply this to the visual panel as well as input panels.
            visual.window.hidesOnDeactivate = NO;
            if ([visual.window isKindOfClass:NSPanel.class])
                static_cast<NSPanel *>(visual.window).floatingPanel = NO;
            // Qt::Tool otherwise stays at NSFloatingWindowLevel even after the
            // host disables alwaysOnTop. Apply the explicit public policy.
            const NSInteger level = target->flags().testFlag(Qt::WindowStaysOnTopHint)
                                        ? NSModalPanelWindowLevel
                                        : NSNormalWindowLevel;
            if (visual.window.level != level)
                visual.window.level = level;
        }
        if (!enabled || !target->isVisible() || pending.isEmpty())
        {
            cancel();
            applied = false;
            for (auto *surface : surfaces)
                [surface orderOut:nil];
            return true;
        }
        // AppKit retains the original down-event view until release. Keep the
        // current rectangles during its implicit mouse grab; geometry is applied
        // at release; the original AppKit view retains its implicit grab.
        if (buttons)
            return true;
        auto *view = reinterpret_cast<NSView *>(target->winId());
        NSWindow *nativeOwner = view.window;
        if (!nativeOwner)
        {
            error = "Cocoa visual window is unavailable";
            return false;
        }
        // Qt's WindowTransparentForInput also refuses keyboard focus on Cocoa.
        // Keep transparency at the native layer so explicit bubble focus can work.
        nativeOwner.ignoresMouseEvents = YES;
        if (owner != nativeOwner)
        {
            clear();
            owner = [nativeOwner retain];
            observeOwner();
        }
        if (!owner.visible || !owner.isOnActiveSpace || owner.miniaturized)
        {
            cancel();
            applied = false;
            for (auto *surface : surfaces)
                [surface orderOut:nil];
            return true;
        }
        const QRegion clipped = pending.intersected(QRegion(QRect(QPoint(), target->size())));
        const NSRect frame = [view convertRect:view.bounds toView:nil];
        const NSRect screenFrame = [owner convertRectToScreen:frame];
        if (applied && appliedRegion == clipped && NSEqualRects(appliedFrame, screenFrame) &&
            appliedLevel == owner.level && !needsOrdering())
            return true;
        appliedRegion = clipped;
        appliedFrame = screenFrame;
        appliedLevel = owner.level;
        applied = true;
        const qreal scaleX = screenFrame.size.width / target->width();
        const qreal scaleY = screenFrame.size.height / target->height();
        size_t i = 0;
        for (const QRect &rect : clipped)
        {
            if (i == surfaces.size())
            {
                auto *surface = [[QCatMascotInputPanel alloc]
                    initWithContentRect:NSMakeRect(0, 0, 1, 1)
                              styleMask:NSWindowStyleMaskBorderless |
                                        NSWindowStyleMaskNonactivatingPanel
                                backing:NSBackingStoreBuffered
                                  defer:NO];
                if (!surface)
                {
                    error = "Cannot create Cocoa input rectangle";
                    return false;
                }
                surface.title = [NSString
                    stringWithFormat:@"QCatUiKit mascot input:%ld", (long)owner.windowNumber];
                surface.animationBehavior = NSWindowAnimationBehaviorNone;
                surface.floatingPanel = NO;
                surface.excludedFromWindowsMenu = YES;
                surface.releasedWhenClosed = NO;
                surface.opaque = NO;
                surface.backgroundColor = NSColor.clearColor;
                surface.hasShadow = NO;
                surface.hidesOnDeactivate = NO;
                surface.acceptsMouseMovedEvents = YES;
                // Explicit NO disables AppKit's initial alpha-based hit testing.
                // The content remains fully clear; only this rectangle receives input.
                surface.ignoresMouseEvents = NO;
                auto *input = [[QCatMascotInputView alloc] initWithFrame:NSMakeRect(0, 0, 1, 1)];
                std::weak_ptr<Private> weak = shared_from_this();
                input->dispatch = [weak](NSEvent *event)
                {
                    if (auto state = weak.lock())
                        state->route(event);
                };
                surface.contentView = input;
                [input release];
                surfaces.push_back(surface);
            }
            auto *surface = surfaces[i++];
            surface.level = owner.level;
            surface.collectionBehavior =
                owner.collectionBehavior | NSWindowCollectionBehaviorIgnoresCycle;
            [surface setFrame:NSMakeRect(screenFrame.origin.x + rect.x() * scaleX,
                                         screenFrame.origin.y + screenFrame.size.height -
                                             (rect.y() + rect.height()) * scaleY,
                                         rect.width() * scaleX, rect.height() * scaleY)
                      display:NO];
            [surface orderWindow:NSWindowAbove relativeTo:owner.windowNumber];
        }
        // Release surplus native windows instead of retaining the largest-ever shape.
        while (surfaces.size() > i)
        {
            auto *surface = surfaces.back();
            static_cast<QCatMascotInputView *>(surface.contentView)->dispatch = {};
            [surface close];
            [surface release];
            surfaces.pop_back();
        }
        return true;
    }
};
MascotMacosInput::MascotMacosInput(QQuickWindow *window) : d(std::make_shared<Private>(window)) {}
MascotMacosInput::~MascotMacosInput()
{
    d->shutdown();
}
bool MascotMacosInput::sync(const QRegion &region, bool enabled)
{
    const auto lifetime = d;
    d->pending = region;
    d->enabled = enabled;
    return d->apply();
}
void MascotMacosInput::requestKeyboardFocus()
{
    if (!d->target || !d->target->isVisible())
        return;
    auto *view = reinterpret_cast<NSView *>(d->target->winId());
    view.window.ignoresMouseEvents = YES;
    [NSApp activateIgnoringOtherApps:YES];
    [view.window makeFirstResponder:view];
    [view.window makeKeyWindow];
}
QString MascotMacosInput::errorString() const
{
    return d->error;
}
} // namespace QCatUiKit
