#include "headers/window.h"
#include "headers/loop.h"
#include "headers/macos.h"
#include "headers/helper.h"

static void window_refresh_callback(GLFWwindow* window) {
	_app *p_app = (_app*)glfwGetWindowUserPointer(window);
	draw_frame(p_app);
}

static void on_edit_toggled(int active) {
	if (active) {
		printf("ensure edit window is opened\n");
		fflush(stdout);
	} else {
		printf("ensure edit window is closed\n");
		fflush(stdout);
	}
}

void window_init(_app *p_app) {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

	p_app->win.window = glfwCreateWindow(minu32(p_app->config.width, 800), minu32(p_app->config.height, 600), p_app->config.title, NULL, NULL);

#ifdef __APPLE__
	macos_install_backdrop(glfwGetCocoaWindow(p_app->win.window));
	if (p_app->config.blur) {
		macos_install_blur(glfwGetCocoaWindow(p_app->win.window));
	}
	macos_set_edit_callback(on_edit_toggled);
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
