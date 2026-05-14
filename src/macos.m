#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>

void macos_install_backdrop(void *nswindow) {
    NSWindow *w = (__bridge NSWindow *)nswindow;

    w.opaque = NO;
    w.backgroundColor = [NSColor clearColor];

    NSView *content = w.contentView;
    content.wantsLayer = YES;

    NSVisualEffectView *vfx = [[NSVisualEffectView alloc] initWithFrame:content.bounds];
    vfx.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    vfx.blendingMode = NSVisualEffectBlendingModeBehindWindow;
    vfx.material = NSVisualEffectMaterialHUDWindow;
    vfx.state = NSVisualEffectStateActive;

    [content addSubview:vfx positioned:NSWindowBelow relativeTo:nil];

    for (CALayer *sub in content.layer.sublayers) {
        if ([sub isKindOfClass:[CAMetalLayer class]]) {
            ((CAMetalLayer *)sub).opaque = NO;
            sub.backgroundColor = NSColor.clearColor.CGColor;
        }
    }
}
