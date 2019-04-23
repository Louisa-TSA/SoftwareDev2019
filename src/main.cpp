#include <iostream>
#include <array>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>

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

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    std::array<char, 1024> text_buf = {0};
    float braille_size[3] = {  1.f, 1.f, 1.f };

    while(!glfwWindowShouldClose(window)) {

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        ImGui::Begin("Braille");
        ImGui::InputText("Text", text_buf.data(), 1024);
        ImGui::InputFloat3("Size", braille_size, 8);

        if(ImGui::Button("Generate")) {
            std::cout <<
            "Generating: " << text_buf.data() <<
            ", x: " << braille_size[0] <<
            ", y: " << braille_size[1] <<
            ", z: " << braille_size[2] <<
            std::endl;
        }
        
        ImGui::End();


        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}