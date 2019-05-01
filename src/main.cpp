#include <iostream>
#include <array>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>

#include <engine/graphics/opengl/buffers/glarray_buffer.hpp>
#include <engine/graphics/opengl/buffers/glelement_buffer.hpp>
#include <engine/graphics/opengl/buffers/glvertex_array.hpp>

#include <engine/graphics/opengl/glshader.hpp>

using namespace amalgamation;

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

    const char shader_src[] = R"glsl(@V#version 330 core
layout (location = 0) in vec3 a_pos;
void main()
{
    gl_Position = vec4(a_pos.x, a_pos.y, a_pos.z, 1.0);
}@F#version 330 core
out vec4 frag_colour;
uniform vec4 u_frag_colour = vec4(0.1f, 0.9f, 0.1f, 1.0f);
void main()
{
    frag_colour = u_frag_colour;
}@)glsl";

    GLShader shader(shader_src);
    shader.bind();
    shader.set_uniform("u_frag_colour", 0.1f, 0.9f, 1.f, 1.0f);

    float vertices[] = {
        -0.5, -0.5, 0.0,
        -0.5,  0.5, 0.0,
         0.5,  0.5, 0.0,
         0.5, -0.5, 0.0
    };

    uint32_t indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    GLVertexArray vao;
    vao.create();
    vao.bind();

    GLArrayBuffer vbo(vertices, sizeof(vertices));
    vbo.bind();
    vbo.get_layout().push<float>(3);

    GLElementBuffer ebo(indices, sizeof(indices) / sizeof(uint32_t));
    ebo.bind();

    vao.set_buffer(vbo);

    while(!glfwWindowShouldClose(window)) {

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        ImGui::Begin("Braille");
        ImGui::InputText("Text", text_buf.data(), 1024);
        ImGui::InputFloat3("Size", braille_size, 8);

        if(ImGui::Button("Generate")) {
            println(
                "Generating: ", text_buf.data(),
                ", x: ", braille_size[0],
                ", y: ", braille_size[1],
                ", z: ", braille_size[2]
            );
        }
        
        ImGui::End();

        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        GLCALL(glDrawElements(GL_TRIANGLES, ebo.get_count(), GL_UNSIGNED_INT, 0));
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    vao.destroy();
    vbo.destroy();
    ebo.destroy();

    shader.destroy();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}