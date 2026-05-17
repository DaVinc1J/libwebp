#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>

typedef void (*edit_callback_t)(int active);
static edit_callback_t s_edit_callback = NULL;

void macos_set_edit_callback(edit_callback_t cb) {
	s_edit_callback = cb;
}

@interface EditButtonController : NSTitlebarAccessoryViewController {
BOOL _active;
NSButton *_btn;
}
@end

@implementation EditButtonController
- (void)loadView {
	_active = NO;
	NSView *container = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 28, 28)];
	container.wantsLayer = YES;
	container.layer.backgroundColor = NSColor.clearColor.CGColor;

	_btn = [[NSButton alloc] initWithFrame:NSMakeRect(0, 2, 26, 24)];
	_btn.image = [NSImage imageWithSystemSymbolName:@"pencil.circle"
		accessibilityDescription:@"Edit"];
	_btn.bezelStyle = NSBezelStyleTexturedRounded;
	_btn.bordered = NO;
	_btn.target = self;
	_btn.action = @selector(onEdit:);
	[container addSubview:_btn];

	self.view = container;
}

- (void)onEdit:(id)sender {
	_active = !_active;

	NSString *symbol = _active ? @"pencil.circle.fill" : @"pencil.circle";
	_btn.image = [NSImage imageWithSystemSymbolName:symbol
		accessibilityDescription:@"Edit"];

	if (s_edit_callback) {
		s_edit_callback(_active ? 1 : 0);
	}
}
@end

static NSVisualEffectView *s_vfx = NULL;

void macos_install_blur(void *nswindow) {
	NSWindow *w = (__bridge NSWindow *)nswindow;
	NSView *glfw = w.contentView;
	NSRect bounds = glfw.bounds;
	s_vfx = [[NSVisualEffectView alloc] initWithFrame:bounds];
	s_vfx.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
	s_vfx.blendingMode = NSVisualEffectBlendingModeBehindWindow;
	s_vfx.material = NSVisualEffectMaterialHUDWindow;
	s_vfx.state = NSVisualEffectStateActive;
	w.contentView = s_vfx;
	[s_vfx addSubview:glfw];
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

void macos_install_backdrop(void *nswindow) {
	NSWindow *w = (__bridge NSWindow *)nswindow;
	w.opaque = NO;
	w.backgroundColor = [NSColor clearColor];
	w.titlebarAppearsTransparent = YES;
	w.styleMask |= NSWindowStyleMaskFullSizeContentView;
	w.titleVisibility = NSWindowTitleHidden;
	EditButtonController *edit = [[EditButtonController alloc] init];
	edit.layoutAttribute = NSLayoutAttributeRight;
	[w addTitlebarAccessoryViewController:edit];
}
