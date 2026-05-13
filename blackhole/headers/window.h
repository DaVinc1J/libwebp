#ifndef WINDOW_H
#define WINDOW_H

#include "define.h"

void window_init(_app *p_app);
void framebuffer_resize_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

#endif
