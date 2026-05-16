#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>

void macos_install_backdrop(void *nswindow) {
    NSWindow *w = (__bridge NSWindow *)nswindow;

    w.opaque = NO;
    w.backgroundColor = [NSColor clearColor];

    NSView *glfw = w.contentView;
    NSRect bounds = glfw.bounds;

    NSVisualEffectView *vfx = [[NSVisualEffectView alloc] initWithFrame:bounds];
    vfx.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    vfx.blendingMode = NSVisualEffectBlendingModeBehindWindow;
    vfx.material = NSVisualEffectMaterialHUDWindow;
    vfx.state = NSVisualEffectStateActive;

    w.contentView = vfx;
    [vfx addSubview:glfw];
    glfw.frame = bounds;
    glfw.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;

    if ([glfw.layer isKindOfClass:[CAMetalLayer class]]) {
        ((CAMetalLayer *)glfw.layer).opaque = NO;
        glfw.layer.backgroundColor = NSColor.clearColor.CGColor;
    } else {
        for (CALayer *sub in glfw.layer.sublayers) {
            if ([sub isKindOfClass:[CAMetalLayer class]]) {
                ((CAMetalLayer *)sub).opaque = NO;
                sub.backgroundColor = NSColor.clearColor.CGColor;
            }
        }
    }
}
