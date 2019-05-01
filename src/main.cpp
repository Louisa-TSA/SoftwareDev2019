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

#include <glm/gtc/matrix_transform.hpp>
#include <glm/matrix.hpp>
#include <glm/gtx/quaternion.hpp>

using namespace amalgamation;

int width = 1280, height = 720;

float delta = 0;

glm::mat4 model = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection;

glm::vec3 block_rotation = glm::vec3(0,0,0);
glm::vec3 block_scale    = glm::vec3(1,1,1);

bool can_pan = false;

double old_mousey   = 0.0, old_mousex   = 0.0;
double delta_mousey = 0.0, delta_mousex = 0.0;

bool just_clicked = false;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    view = glm::translate(view, glm::vec3(0, 0, yoffset/2));
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        just_clicked = true;
        can_pan = true;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        can_pan = false;
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    double curmx = xpos, curmy = ypos;
    delta_mousex = old_mousex - curmx;
    delta_mousey = old_mousey - curmy;

    old_mousex = curmx; old_mousey = curmy;

    if(just_clicked) {
        delta_mousex = 0;
        delta_mousey = 0;
        just_clicked = false;
    }

    if(can_pan) {
        block_rotation.x += -delta_mousey * delta;
        block_rotation.y += -delta_mousex * delta;
        block_rotation.z = 0;
    }
}

int main(int argc, char* argv[]) {

    GLFWwindow* window;

    if(!glfwInit()) {
        return 1;
    }

    window = glfwCreateWindow(width, height, "TSA Braille Generator", nullptr, nullptr);
    if(!window) {
        std::cerr << "Failed to create window!" << std::endl;
        return 2;
    }
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

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

uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_model;

void main()
{
    gl_Position =  u_projection * u_view * u_model * vec4(a_pos.x, a_pos.y, a_pos.z, 1.0);
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


    glEnable(GL_DEPTH_TEST);



    float last_frame = 0;



    view = glm::translate(view, glm::vec3(0, 0,-3));





    while(!glfwWindowShouldClose(window)) {


        //block_rotation *= glm::quat(glm::vec3(0,glm::radians(30.f) * delta,0));


        glfwGetFramebufferSize(window, &width, &height);



        projection = glm::perspective(glm::radians(90.0f), (float)width/(float)height, 0.1f, 100.0f);



        model = glm::mat4(1.f);//glm::rotate(model, (float)delta * glm::radians(50.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, glm::vec3(0,0,0)) * glm::mat4_cast(glm::quat(block_rotation));
        model = glm::scale(model, block_scale);

        shader.set_uniform("u_model", model);
        shader.set_uniform("u_view", view);
        shader.set_uniform("u_projection", projection);



        float current_frame = glfwGetTime();
        delta = current_frame - last_frame;
        last_frame = current_frame;

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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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