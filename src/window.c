#include "headers/window.h"

void window_init(_app *p_app) {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	p_app->win.window = glfwCreateWindow(p_app->config.width, p_app->config.height, p_app->config.title, NULL, NULL);

	glfwSetWindowUserPointer(p_app->win.window, p_app);
	glfwSetFramebufferSizeCallback(p_app->win.window, framebuffer_resize_callback);
}

void framebuffer_resize_callback(GLFWwindow* window, int width, int height) {
	_app *p_app = (_app*)glfwGetWindowUserPointer(window);
	p_app->swp.framebuffer_resized = true;
}
