#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

int main(int argc, char* argv[]) {

    GLFWwindow* window;

    if(!glfwInit()) {
        return 1;
    }

    window = glfwCreateWindow(1280, 720, "TSA Braille Generator", nullptr, nullptr);
    if(!window) {
        std::cerr << "Failed to create window!" << std::endl;
        return 2;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed to initialize glad!" << std::endl;
        return false;
    }

    while(!glfwWindowShouldClose(window)) {

        

        glfwSwapBuffers(window);
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);
    }

    glfwTerminate();

    return 0;
}