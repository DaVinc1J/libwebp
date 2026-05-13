#include "headers/window.h"

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
	_app *p_app = (_app*)glfwGetWindowUserPointer(window);

	if (p_app->view.first_mouse) {
		p_app->view.last_mouse_x = xpos;
		p_app->view.last_mouse_y = ypos;
		p_app->view.first_mouse = false;
		return;
	}

	float dx = xpos - p_app->view.last_mouse_x;
	float dy = p_app->view.last_mouse_y - ypos;

	p_app->view.last_mouse_x = xpos;
	p_app->view.last_mouse_y = ypos;

	dx *= p_app->view.sensitivity;
	dy *= p_app->view.sensitivity;

	p_app->view.yaw += dx;
	p_app->view.pitch += dy;

	if (p_app->view.pitch > 89.0f) p_app->view.pitch = 89.0f;
	if (p_app->view.pitch < -89.0f) p_app->view.pitch = -89.0f;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	_app* p_app = (_app*)glfwGetWindowUserPointer(window);

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !p_app->view.mouse_locked) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		glfwGetCursorPos(window, &p_app->view.last_mouse_x, &p_app->view.last_mouse_y);

		p_app->view.first_mouse = true;
		p_app->view.mouse_locked = true;
	}
}

void window_init(_app *p_app) {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	p_app->win.window = glfwCreateWindow(p_app->config.win.width, p_app->config.win.height, p_app->config.win.title, NULL, NULL);

	glfwSetWindowUserPointer(p_app->win.window, p_app);
	glfwSetFramebufferSizeCallback(p_app->win.window, framebuffer_resize_callback);
	glfwSetInputMode(p_app->win.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(p_app->win.window, mouse_callback);
	glfwSetMouseButtonCallback(p_app->win.window, mouse_button_callback);
}

void framebuffer_resize_callback(GLFWwindow* window, int width, int height) {
	_app *p_app = (_app*)glfwGetWindowUserPointer(window);
	p_app->swp.framebuffer_resized = true;
}
