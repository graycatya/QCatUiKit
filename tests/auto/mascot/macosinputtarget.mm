// SPDX-License-Identifier: Apache-2.0
#import <AppKit/AppKit.h>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <cstdio>

@interface QCatInputTargetWindow : NSWindow
@end
@implementation QCatInputTargetWindow
- (BOOL)canBecomeKeyWindow
{
    return YES;
}
- (BOOL)canBecomeMainWindow
{
    return YES;
}
@end

@interface QCatInputTargetView : NSView
@property int clicks;
@property int keys;
@end
@implementation QCatInputTargetView
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
    [[NSColor colorWithCalibratedRed:0.1 green:0.3 blue:0.5 alpha:1] setFill];
    NSRectFill(self.bounds);
}
- (void)mouseDown:(NSEvent *)event
{
    Q_UNUSED(event);
    self.clicks++;
}
- (void)keyDown:(NSEvent *)event
{
    Q_UNUSED(event);
    self.keys++;
}
@end
int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    if (app.arguments().size() != 2 || QGuiApplication::platformName() != "cocoa")
        return 2;
    const NSRect bounds = NSInsetRect(NSScreen.screens.firstObject.visibleFrame, 20, 20);
    auto *window = [[QCatInputTargetWindow alloc] initWithContentRect:bounds
                                                            styleMask:NSWindowStyleMaskBorderless
                                                              backing:NSBackingStoreBuffered
                                                                defer:NO];
    window.title = app.arguments()[1].toNSString();
    window.releasedWhenClosed = NO;
    window.level = NSFloatingWindowLevel;
    auto *view = [[QCatInputTargetView alloc]
        initWithFrame:NSMakeRect(0, 0, bounds.size.width, bounds.size.height)];
    window.contentView = view;
    [NSApp activateIgnoringOtherApps:YES];
    [window makeKeyAndOrderFront:nil];
    [window makeFirstResponder:view];
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout,
                     [&]
                     {
                         const auto bytes =
                             QJsonDocument(
                                 QJsonObject{{"pid", int(QCoreApplication::applicationPid())},
                                             {"window", int(window.windowNumber)},
                                             {"clicks", view.clicks},
                                             {"keys", view.keys},
                                             {"keyWindow", bool(window.keyWindow)}})
                                 .toJson(QJsonDocument::Compact);
                         fwrite(bytes.constData(), 1, size_t(bytes.size()), stdout);
                         fputc('\n', stdout);
                         fflush(stdout);
                     });
    timer.start(30);
    const int result = app.exec();
    [window close];
    [window release];
    [view release];
    return result;
}
