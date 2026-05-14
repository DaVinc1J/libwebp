#include "headers/window.h"
#include "headers/loop.h"
#include "headers/macos.h"

static void window_refresh_callback(GLFWwindow* window) {
	_app *p_app = (_app*)glfwGetWindowUserPointer(window);
	draw_frame(p_app);
}

void window_init(_app *p_app) {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

	p_app->win.window = glfwCreateWindow(p_app->config.width, p_app->config.height, p_app->config.title, NULL, NULL);

#ifdef __APPLE__
    macos_install_backdrop(glfwGetCocoaWindow(p_app->win.window));
#endif

	glfwSetWindowUserPointer(p_app->win.window, p_app);
	glfwSetFramebufferSizeCallback(p_app->win.window, framebuffer_resize_callback);
	glfwSetWindowRefreshCallback(p_app->win.window, window_refresh_callback);
}

void framebuffer_resize_callback(GLFWwindow* window, int width, int height) {
	_app *p_app = (_app*)glfwGetWindowUserPointer(window);
	p_app->swp.framebuffer_resized = true;
	if (width > 0 && height > 0) draw_frame(p_app);
}
