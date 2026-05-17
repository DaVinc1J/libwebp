#ifndef MACOS_H
#define MACOS_H
void macos_set_edit_callback(void (*cb)(int active));
void macos_install_backdrop(void *nswindow);
void macos_install_blur(void *nswindow);
#endif
