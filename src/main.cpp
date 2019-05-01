#include <iostream>
#include <array>

#include <cctype>

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

std::unordered_map<char, std::vector<glm::vec3>> char_map;

float sensitivity = 10.f;

int width = 1280, height = 720;

float delta = 0;

glm::mat4 slab_model = glm::mat4(1.0f);
glm::mat4 view  = glm::mat4(1.0f);
glm::mat4 projection;

glm::vec3 block_rotation = glm::vec3(0,0,0);
glm::vec3 block_scale    = glm::vec3(1, 0.25, 1);

bool can_pan = false;

double old_mousey   = 0.0, old_mousex   = 0.0;
double delta_mousey = 0.0, delta_mousex = 0.0;

bool just_clicked = false;

class Cube {

public:

    glm::mat4 model    = glm::mat4(1.0f);
    glm::vec3 rotation = glm::vec3(0,0,0);
    glm::vec3 scale    = glm::vec3(0,0,0);
    glm::vec3 position = glm::vec3(0,0,0);

    void prepare(GLShader& shader, GLElementBuffer& ebo) {

        model = glm::mat4(1.f);
        model = glm::translate(model, position) * glm::mat4_cast(glm::quat(rotation));
        model = glm::scale(model, scale);

        shader.set_uniform("u_model", model);
        shader.set_uniform("u_view", view);
        shader.set_uniform("u_projection", projection);

        GLCALL(glDrawElements(GL_TRIANGLES, ebo.get_count(), GL_UNSIGNED_INT, 0));

    }

};

std::vector<std::vector<Cube>> bumps;

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
        block_rotation.x += -delta_mousey * delta * sensitivity;
        block_rotation.y += -delta_mousex * delta * sensitivity;
        block_rotation.z = 0;
    }
}

int main(int argc, char* argv[]) {

    char_map['a'] = {
        {0, 1, 0}
    };
    char_map['b'] = {
        {0, 1, 0},
        {0, 0, 0}
    };
    char_map['c'] = {
        {0, 1, 0},
        {1, 1, 0}
    };
    char_map['d'] = {
        {0, 1, 0},
        {1, 1, 0},
        {1, 0, 0}
    };
    char_map['e'] = {
        {0, 1, 0},
        {1, 0, 0}
    };
    char_map['f'] = {
        {0, 1, 0},
        {0, 0, 0},
        {1, 1, 0}
    };
    char_map['g'] = {
        {0, 1, 0},
        {0, 0, 0},
        {1, 1, 0},
        {1, 0, 0}
    };
    char_map['h'] = {
        {0, 1, 0},
        {0, 0, 0},
        {1, 0, 0}
    };
    char_map['i'] = {
        {0, 0, 0},
        {1, 1, 0}
    };
    char_map['j'] = {
        {0, 0, 0},
        {1, 1, 0},
        {1, 0, 0}
    };
    char_map['k'] = {
        {0, 1, 0},
        {0, -1, 0}
    };
    char_map['l'] = {
        {0, 1, 0},
        {0, 0, 0},
        {0, -1, 0}
    };
    char_map['m'] = {
        {0, 1, 0},
        {0, -1, 0},
        {1, 1, 0}
    };
    char_map['n'] = {
        {0, 1, 0},
        {0, -1, 0},
        {1, 1, 0},
        {1, 0, 0}
    };
    char_map['o'] = {
        {0, 1, 0},
        {0, -1, 0},
        {1, 0, 0}
    };
    char_map['p'] = {
        {0, 1, 0},
        {0, 0, 0},
        {0, -1, 0},
        {1, 1, 0}
    };
    char_map['q'] = {
        {0, 1, 0},
        {0, 0, 0},
        {0, -1, 0},
        {1, 1, 0},
        {1, 0, 0}
    };
    char_map['r'] = {
        {0, 1, 0},
        {0, 0, 0},
        {0, -1, 0},
        {1, 0, 0}
    };
    char_map['s'] = {
        {0, 0, 0},
        {0, -1, 0},
        {1, 1, 0}
    };
    char_map['t'] = {
        {0, 0, 0},
        {0, -1, 0},
        {1, 1, 0},
        {1, 0, 0}
    };
    char_map['u'] = {
        {0, 1, 0},
        {0, -1, 0},
        {1, -1, 0}
    };
    char_map['v'] = {
        {0, 1, 0},
        {0, 0, 0},
        {0, -1, 0},
        {1, -1, 0}
    };
    char_map['w'] = {
        {0, 0, 0},
        {1, 1, 0},
        {1, 0, 0},
        {1, -1, 0}
    };
    char_map['x'] = {
        {0, 1, 0},
        {0, -1, 0},
        {1, 1, 0},
        {1, -1, 0}
    };
    char_map['y'] = {
        {0, 1, 0},
        {0, -1, 0},
        {1, 1, 0},
        {1, 0, 0},
        {1, -1, 0}
    };
    char_map['z'] = {
        {0, 1, 0},
        {0, -1, 0},
        {1, 0, 0},
        {1, -1, 0}
    };


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
    shader.set_uniform("u_frag_colour", 0.25f, 0.25f, 0.25f, 1.0f);

    float vertices[] = {
        -0.5, -0.5,  0.5,
        -0.5,  0.5,  0.5,
         0.5,  0.5,  0.5,
         0.5, -0.5,  0.5,

        -0.5, -0.5, -0.5,
        -0.5,  0.5, -0.5,
         0.5,  0.5, -0.5,
         0.5, -0.5, -0.5
    };

    // Clockwise

    uint32_t indices[] = {
        0, 1, 2, // Front 
        2, 3, 0,

        4, 5, 6, // Back
        6, 7, 4,

        1, 5, 6, // Top
        6, 2, 1,

        4, 5, 1, // Left
        1, 0, 4,

        3, 2, 6, // Right
        6, 7, 3

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



        slab_model = glm::mat4(1.f);//glm::rotate(model, (float)delta * glm::radians(50.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        slab_model = glm::translate(slab_model, glm::vec3(0,0,0)) * glm::mat4_cast(glm::quat(block_rotation));
        slab_model = glm::scale(slab_model, block_scale);

        shader.set_uniform("u_model", slab_model);
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
        ImGui::InputFloat3("Size", &block_scale[0], 8);

        if(ImGui::Button("Generate")) {
            for(size_t i = 0; i < text_buf.size(); i++) {
                text_buf[i] = std::tolower(text_buf[i]);
                bumps.emplace_back();
                for(size_t j = 0; j < char_map[text_buf[i]].size(); j++) {
                    bumps[i].emplace_back();
                    // bumps[i][j].position = 
                }
            }
            println(
                "Generating: ", text_buf.data(),
                ", x: ", block_scale[0],
                ", y: ", block_scale[1],
                ", z: ", block_scale[2]
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