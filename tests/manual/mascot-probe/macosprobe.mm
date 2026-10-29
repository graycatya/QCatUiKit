// SPDX-License-Identifier: Apache-2.0
#include "macosprobe.h"
#import <AppKit/AppKit.h>
#import <ApplicationServices/ApplicationServices.h>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QProcess>
#include <QSaveFile>
#include <QTimer>
#include <functional>
#include <vector>

// Each panel is one declared rectangular input region. No cursor polling,
// event forwarding, alpha scanning or process-wide AppKit method replacement.
@interface QCatProbePanel : NSPanel
@property BOOL keyboardEnabled;
@end
@implementation QCatProbePanel
- (BOOL)canBecomeKeyWindow
{
    return self.keyboardEnabled;
}
- (BOOL)canBecomeMainWindow
{
    return NO;
}
@end

@interface QCatProbeButton : NSButton
{
  @public
    std::function<void(int)> keyObserved;
}
@end
@implementation QCatProbeButton
- (void)keyDown:(NSEvent *)event
{
    if (keyObserved)
        keyObserved(event.keyCode);
    if (event.keyCode == 48)
    {
        [self.window makeFirstResponder:(event.modifierFlags & NSEventModifierFlagShift)
                                            ? self.previousKeyView
                                            : self.nextKeyView];
    }
    else if (event.keyCode == 49)
    {
        [self performClick:nil];
    }
    else
    {
        [super keyDown:event];
    }
}
- (BOOL)acceptsFirstResponder
{
    return YES;
}
@end

@interface QCatProbeView : NSView
{
  @public
    std::function<void(QString)> callback;
    NSString *caption;
    NSPoint dragPoint;
    BOOL draggable;
}
@end
@implementation QCatProbeView
- (BOOL)acceptsFirstResponder
{
    return YES;
}
- (BOOL)acceptsFirstMouse:(NSEvent *)event
{
    Q_UNUSED(event);
    return YES;
}
- (void)drawRect:(NSRect)rect
{
    Q_UNUSED(rect);
    [[NSColor colorWithCalibratedRed:0.9 green:0.65 blue:0.29 alpha:1] setFill];
    NSRectFill(self.bounds);
    [caption drawInRect:NSInsetRect(self.bounds, 12, 16)
         withAttributes:@{
             NSFontAttributeName : [NSFont systemFontOfSize:16],
             NSForegroundColorAttributeName : NSColor.blackColor
         }];
}
- (void)mouseDown:(NSEvent *)event
{
    dragPoint = NSEvent.mouseLocation;
    if (callback)
        callback("mouse");
    Q_UNUSED(event);
}
- (void)mouseDragged:(NSEvent *)event
{
    Q_UNUSED(event);
    if (!draggable)
        return;
    NSPoint current = NSEvent.mouseLocation;
    NSPoint origin = self.window.frame.origin;
    [self.window setFrameOrigin:NSMakePoint(origin.x + current.x - dragPoint.x,
                                            origin.y + current.y - dragPoint.y)];
    dragPoint = current;
    if (callback)
        callback("drag");
}
- (void)keyDown:(NSEvent *)event
{
    Q_UNUSED(event);
    if (callback)
        callback("key");
}
- (void)activate:(id)sender
{
    // A deliberate bubble action may activate the application; passive body input never does.
    [NSApp activateIgnoringOtherApps:YES];
    [self.window makeKeyWindow];
    [self.window makeFirstResponder:sender];
    if (callback)
        callback("bubble");
}
@end

@interface QCatProbeMenuTarget : NSObject
{
  @public
    std::function<void(QString)> callback;
}
- (void)perform:(id)sender;
@end
@implementation QCatProbeMenuTarget
- (void)perform:(id)sender
{
    if (callback)
        callback(QString::fromNSString([sender representedObject]));
}
@end

static QCatProbePanel *panel(NSRect frame, bool keyboard)
{
    auto *window = [[QCatProbePanel alloc]
        initWithContentRect:frame
                  styleMask:NSWindowStyleMaskBorderless | NSWindowStyleMaskNonactivatingPanel
                    backing:NSBackingStoreBuffered
                      defer:NO];
    window.keyboardEnabled = keyboard;
    window.opaque = NO;
    window.backgroundColor = NSColor.clearColor;
    window.hasShadow = NO;
    window.hidesOnDeactivate = NO;
    window.releasedWhenClosed = NO;
    return window;
}

struct MacosProbe::Private
{
    MacosProbe *q;
    QJsonObject report;
    QJsonArray checks;
    QProcess target;
    QByteArray buffer;
    QJsonObject targetState;
    QCatProbePanel *body = nil, *bubble = nil;
    QCatProbeView *bodyView = nil, *bubbleView = nil;
    NSStatusItem *status = nil;
    NSMenu *menu = nil;
    id menuTarget = nil;
    bool transparent = false, top = true, bubbleVisible = true, visible = true;
    int bodyClicks = 0, bubbleClicks = 0, drags = 0, exitRequests = 0;
    QString output;
    QTimer timer;
    std::vector<std::function<void()>> steps;
    size_t step = 0;
    bool passed = true;
    bool focusBaseline = false;
    pid_t previousApp = 0;
    CGPoint previousCursor{};
    void update()
    {
        report.insert("target", targetState);
        report.insert("bodyClicks", bodyClicks);
        report.insert("bubbleClicks", bubbleClicks);
        report.insert("dragEvents", drags);
        report.insert("exitRequests", exitRequests);
        report.insert("passthrough", transparent);
        report.insert("topmost", top);
        report.insert("panelsVisible", visible);
        report.insert("bubbleVisible", bubbleVisible);
        if (body && bubble)
        {
            report.insert("bodyOrigin", QJsonArray{body.frame.origin.x, body.frame.origin.y});
            report.insert("bubbleKeyWindow", bool(bubble.keyWindow));
            report.insert("bubbleResponderClass",
                          QString::fromNSString(NSStringFromClass([bubble.firstResponder class])));
            report.insert("bubbleOrigin", QJsonArray{bubble.frame.origin.x, bubble.frame.origin.y});
        }
        report.insert("checks", checks);
        emit q->changed();
    }
    void record(QString name, bool ok)
    {
        checks.append(QJsonObject{{"name", name},
                                  {"status", ok ? "Passed" : "Failed"},
                                  {"bodyClicks", bodyClicks},
                                  {"bubbleClicks", bubbleClicks},
                                  {"target", targetState},
                                  {"bubbleKeyWindow", bool(bubble.keyWindow)},
                                  {"bubbleResponderClass", QString::fromNSString(NSStringFromClass(
                                                               [bubble.firstResponder class]))}});
        passed &= ok;
        update();
    }
    void save()
    {
        report.insert("recordedAt", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
        update();
        QSaveFile file(output);
        if (!file.open(QIODevice::WriteOnly))
        {
            report.insert("writeError", file.errorString());
            emit q->changed();
            return;
        }
        file.write(QJsonDocument(report).toJson());
        if (!file.commit())
            report.insert("writeError", file.errorString());
    }
    void move(NSPoint point)
    {
        [body setFrameOrigin:point];
        [bubble setFrameOrigin:NSMakePoint(point.x - 60, point.y + 200)];
    }
    void click(NSPoint point, CGEventType type = kCGEventLeftMouseDown)
    {
        // AppKit screen points are bottom-up; CGEvent points are top-down.
        const CGFloat height = NSScreen.screens.firstObject.frame.size.height;
        auto event = CGEventCreateMouseEvent(nullptr, type, CGPointMake(point.x, height - point.y),
                                             kCGMouseButtonLeft);
        CGEventPost(kCGHIDEventTap, event);
        CFRelease(event);
    }
    void tap(NSPoint point)
    {
        click(point);
        click(point, kCGEventLeftMouseUp);
    }
    void key(CGKeyCode code)
    {
        for (bool down : {true, false})
        {
            auto event = CGEventCreateKeyboardEvent(nullptr, code, down);
            CGEventPost(kCGHIDEventTap, event);
            CFRelease(event);
        }
    }
};

MacosProbe::MacosProbe(QJsonObject environment) : d(std::make_unique<Private>())
{
    d->q = this;
    d->report = environment;
    d->report.insert("prototypeAcceptance", "Pending");
    d->report.insert("nativeRenderer", "AppKit (not a QRhi rendering test)");
    d->report.insert("inputRegionStrategy",
                     "Separate nonactivating rectangular body and bubble panels");
    connect(&d->target, &QProcess::readyReadStandardOutput, this,
            [this]
            {
                d->buffer += d->target.readAllStandardOutput();
                while (d->buffer.contains('\n'))
                {
                    auto end = d->buffer.indexOf('\n');
                    auto state = QJsonDocument::fromJson(d->buffer.left(end)).object();
                    d->buffer.remove(0, end + 1);
                    if (!state.isEmpty())
                        d->targetState = state;
                }
                d->update();
            });
}
MacosProbe::~MacosProbe()
{
    d->timer.stop();
    [d->body close];
    [d->bubble close];
    if (d->status)
    {
        [NSStatusBar.systemStatusBar removeStatusItem:d->status];
        [d->status release];
    }
    [d->menu release];
    [d->menuTarget release];
    [d->body release];
    [d->bubble release];
    [d->bodyView release];
    [d->bubbleView release];
    if (d->target.state() != QProcess::NotRunning)
    {
        d->target.terminate();
        if (!d->target.waitForFinished(2000))
        {
            d->target.kill();
            d->target.waitForFinished();
        }
    }
    if (d->previousApp)
    {
        [[NSRunningApplication runningApplicationWithProcessIdentifier:d->previousApp]
            activateWithOptions:0];
    }
}
QString MacosProbe::report() const
{
    return QString::fromUtf8(QJsonDocument(d->report).toJson());
}

int MacosProbe::runTarget()
{
    auto *window =
        [[NSWindow alloc] initWithContentRect:NSMakeRect(80, 100, 700, 550)
                                    styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable
                                      backing:NSBackingStoreBuffered
                                        defer:NO];
    window.title = @"QCat independent input target";
    window.releasedWhenClosed = NO;
    auto *view = [[QCatProbeView alloc] initWithFrame:NSMakeRect(0, 0, 700, 550)];
    view->caption = @"Independent process: click and type here. Counters go to the probe.";
    int mouse = 0, keys = 0;
    view->callback = [&](QString action)
    {
        if (action == "mouse")
            ++mouse;
        if (action == "key")
            ++keys;
    };
    window.contentView = view;
    [window makeFirstResponder:view];
    [NSApp activateIgnoringOtherApps:YES];
    [window makeKeyAndOrderFront:nil];
    QTimer timer;
    QObject::connect(
        &timer, &QTimer::timeout,
        [&]
        {
            auto bytes = QJsonDocument(QJsonObject{{"pid", int(QCoreApplication::applicationPid())},
                                                   {"mouse", mouse},
                                                   {"keys", keys},
                                                   {"keyWindow", bool(window.keyWindow)},
                                                   {"active", bool(NSApp.active)}})
                             .toJson(QJsonDocument::Compact);
            fwrite(bytes.constData(), 1, size_t(bytes.size()), stdout);
            fputc('\n', stdout);
            fflush(stdout);
        });
    timer.start(100);
    const int result = QCoreApplication::exec();
    [window close];
    [view release];
    [window release];
    return result;
}

void MacosProbe::command(const QString &action)
{
    if (!d->body)
        return;
    if (action == "show" || action == "hide")
    {
        d->visible = action == "show";
        if (d->visible)
        {
            [d->body orderFrontRegardless];
            if (d->bubbleVisible)
                [d->bubble orderFrontRegardless];
        }
        else
        {
            [d->body orderOut:nil];
            [d->bubble orderOut:nil];
        }
    }
    else if (action == "passthrough")
    {
        d->transparent = !d->transparent;
        d->body.ignoresMouseEvents = d->transparent;
        d->bubble.ignoresMouseEvents = d->transparent;
    }
    else if (action == "topmost")
    {
        d->top = !d->top;
        d->body.level = d->bubble.level = d->top ? NSFloatingWindowLevel : NSNormalWindowLevel;
    }
    else if (action == "bubble")
    {
        d->bubbleVisible = !d->bubbleVisible;
        if (d->bubbleVisible && d->visible)
            [d->bubble orderFrontRegardless];
        else
            [d->bubble orderOut:nil];
    }
    else if (action == "reset")
        d->move(NSMakePoint(200, 220));
    else if (action == "save")
        d->save();
    else if (action == "exit")
        ++d->exitRequests;
    else if (action == "menu")
    {
        QTimer::singleShot(0, this,
                           [this]
                           {
                               [NSApp activateIgnoringOtherApps:YES];
                               d->report.insert("menuOpenRequests",
                                                d->report.value("menuOpenRequests").toInt() + 1);
                               const bool selected =
                                   [d->menu popUpMenuPositioningItem:nil
                                                          atLocation:NSEvent.mouseLocation
                                                              inView:nil];
                               d->report.insert("menuSelected", selected);
                               d->update();
                           });
    }
    d->update();
}

void MacosProbe::start(const QString &output)
{
    d->output = output.isEmpty() ? QDir::tempPath() + "/qcat-macos-mascot-report.json" : output;
    d->report.insert("outputPath", d->output);
    d->previousApp = NSWorkspace.sharedWorkspace.frontmostApplication.processIdentifier;
    auto cursor = CGEventCreate(nullptr);
    d->previousCursor = CGEventGetLocation(cursor);
    CFRelease(cursor);
    d->body = panel(NSMakeRect(200, 220, 180, 170), false);
    d->bubble = panel(NSMakeRect(140, 420, 300, 60), true);
    d->bodyView = [[QCatProbeView alloc] initWithFrame:NSMakeRect(0, 0, 180, 170)];
    d->bodyView->caption = @"Drag / click\nprototype body";
    d->bodyView->draggable = YES;
    d->bodyView->callback = [this](QString action)
    {
        if (action == "mouse")
            ++d->bodyClicks;
        if (action == "drag")
        {
            ++d->drags;
            d->move(d->body.frame.origin);
        }
        d->update();
    };
    d->body.contentView = d->bodyView;
    d->bubbleView = [[QCatProbeView alloc] initWithFrame:NSMakeRect(0, 0, 300, 60)];
    d->bubbleView->caption = @"";
    d->bubbleView->callback = [this](QString action)
    {
        if (action == "bubble")
            ++d->bubbleClicks;
        d->update();
    };
    d->bubble.contentView = d->bubbleView;
    NSButton *buttons[2];
    for (int i = 0; i < 2; ++i)
    {
        auto *button =
            [[QCatProbeButton alloc] initWithFrame:NSMakeRect(10 + 145 * i, 10, 135, 40)];
        button->keyObserved = [this](int code)
        {
            d->report.insert("bubbleLastKeyCode", code);
            d->report.insert("bubbleKeyEvents", d->report.value("bubbleKeyEvents").toInt() + 1);
            d->update();
        };
        button.title = i ? @"Tab / Space" : @"Focus action";
        button.target = d->bubbleView;
        button.action = @selector(activate:);
        button.bezelStyle = NSBezelStyleRounded;
        [button setButtonType:NSButtonTypeMomentaryPushIn];
        [d->bubbleView addSubview:button];
        buttons[i] = button;
        [button release];
    }
    buttons[0].nextKeyView = buttons[1];
    buttons[1].nextKeyView = buttons[0];
    d->bubble.initialFirstResponder = buttons[0];
    d->body.level = d->bubble.level = NSFloatingWindowLevel;
    d->menu = [[NSMenu alloc] initWithTitle:@"QCat probe"];
    auto *menuTarget = [[QCatProbeMenuTarget alloc] init];
    menuTarget->callback = [this](QString action)
    {
        d->report.insert("lastMenuAction", action);
        d->report.insert("menuActions", d->report.value("menuActions").toInt() + 1);
        command(action);
    };
    d->menuTarget = menuTarget;
    for (const auto &action : {"show", "hide", "passthrough", "exit"})
    {
        auto *item = [[NSMenuItem alloc] initWithTitle:QString::fromLatin1(action).toNSString()
                                                action:@selector(perform:)
                                         keyEquivalent:@""];
        item.target = menuTarget;
        item.representedObject = QString::fromLatin1(action).toNSString();
        [d->menu addItem:item];
        [item release];
    }
    d->status =
        [[NSStatusBar.systemStatusBar statusItemWithLength:NSVariableStatusItemLength] retain];
    d->status.button.title = @"QCat";
    d->status.menu = d->menu;
    d->report.insert("trayRegistered", bool(d->status.button));
    d->target.start(QCoreApplication::applicationFilePath(), {"--macos-target"});
    command("show");
    d->report.insert("postEventAccess", bool(CGPreflightPostEventAccess()));
    d->report.insert("scope",
                     "Single-screen rectangular native window prototype; full acceptance Pending");
    d->update();
    if (output.isEmpty())
        return;
    // Delayed steps allow independent-process delivery and WindowServer updates.
    d->steps = {
        [this]
        {
            d->record("independent target",
                      d->targetState.value("pid").toInt() == d->target.processId() &&
                          d->target.processId() != QCoreApplication::applicationPid());
            command("hide");
            [[NSRunningApplication
                runningApplicationWithProcessIdentifier:pid_t(d->target.processId())]
                activateWithOptions:0];
        },
        [this]
        {
            d->focusBaseline = d->targetState.value("keyWindow").toBool();
            d->report.insert("focusBaselineEstablished", d->focusBaseline);
            command("show");
        },
        [this]
        {
            if (d->focusBaseline)
                d->record("passive display preserves target focus",
                          d->targetState.value("keyWindow").toBool());
            else
                d->checks.append(
                    QJsonObject{{"name", "passive display preserves target focus"},
                                {"status", "Pending"},
                                {"reason", "Target did not acquire initial key window"}});
            d->record("nonactivating body", !d->body.canBecomeKeyWindow);
            d->record("native topmost", d->body.level == NSFloatingWindowLevel);
            command("topmost");
        },
        [this]
        {
            d->record("disable topmost", d->body.level == NSNormalWindowLevel);
            command("topmost");
            command("reset");
            d->record("native position",
                      NSEqualPoints(d->body.frame.origin, NSMakePoint(200, 220)));
            command("hide");
            d->record("hide both regions", !d->body.visible && !d->bubble.visible);
            command("show");
        },
        [this]
        {
            d->record("host restore", d->body.visible && d->bubble.visible);
            command("passthrough");
            d->record("native whole-window passthrough",
                      d->body.ignoresMouseEvents && d->bubble.ignoresMouseEvents);
            command("passthrough");
            if (!CGPreflightPostEventAccess())
            {
                d->report.insert(
                    "prototypeReason",
                    "System event posting permission unavailable; real input checks Pending");
                d->report.insert("prototypeAcceptance", d->passed ? "Pending" : "Failed");
                d->save();
                emit finished(d->passed ? 2 : 1);
                return;
            }
            d->tap(NSMakePoint(110, 300));
        },
        [this]
        {
            d->record("blank reaches target", d->targetState.value("mouse").toInt() == 1);
            d->tap(NSMakePoint(280, 300));
        },
        [this]
        {
            d->record("body receives input without target click",
                      d->bodyClicks == 1 && d->targetState.value("mouse").toInt() == 1);
            d->record("body preserves target focus", d->targetState.value("keyWindow").toBool());
            d->key(0);
        },
        [this]
        {
            d->record("keyboard stays with target", d->targetState.value("keys").toInt() == 1);
            command("passthrough");
        },
        [this] { d->tap(NSMakePoint(280, 300)); },
        [this]
        {
            d->record("body passthrough",
                      d->bodyClicks == 1 && d->targetState.value("mouse").toInt() == 2);
            d->tap(NSMakePoint(210, 450));
        },
        [this]
        {
            d->record("bubble passthrough",
                      d->bubbleClicks == 0 && d->targetState.value("mouse").toInt() == 3);
            command("passthrough");
        },
        [this] { d->tap(NSMakePoint(210, 450)); },
        [this]
        {
            d->record("bubble action", d->bubbleClicks == 1);
            d->key(48);
            d->key(49);
        },
        [this]
        {
            d->record("bubble keyboard", d->bubbleClicks == 2);
            command("bubble");
        },
        [this] { d->tap(NSMakePoint(210, 450)); },
        [this]
        {
            d->record("removed bubble region reaches target",
                      d->targetState.value("mouse").toInt() == 4);
            command("bubble");
            d->click(NSMakePoint(280, 300));
        },
        [this] { d->click(NSMakePoint(310, 320), kCGEventLeftMouseDragged); },
        [this] { d->click(NSMakePoint(310, 320), kCGEventLeftMouseUp); },
        [this]
        {
            d->record("native drag and attached bubble",
                      d->drags > 0 && NSEqualPoints(d->body.frame.origin, NSMakePoint(230, 240)) &&
                          NSEqualPoints(d->bubble.frame.origin, NSMakePoint(170, 440)));
            // Tray interaction is independently recorded; do not claim full prototype success.
            d->report.insert("prototypeAcceptance", d->passed ? "Pending" : "Failed");
            d->report.insert("prototypeReason",
                             "Tray/menu and compositor visual checks require interactive evidence");
            d->save();
            CGWarpMouseCursorPosition(d->previousCursor);
            emit finished(d->passed ? 2 : 1);
        }};
    connect(&d->timer, &QTimer::timeout, this,
            [this]
            {
                if (d->step < d->steps.size())
                    d->steps[d->step++]();
                else
                    d->timer.stop();
            });
    d->timer.start(700);
}
