#include <iostream>
#include <array>

#include <cctype>

#include <fstream>

#include <chrono>
#include <thread>

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
    6, 7, 3,

    0, 4, 7, // Bottom
    7, 3, 0

};

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

std::unordered_map<char, std::vector<glm::vec3>> char_map;

float char_size = 1.f;

float sensitivity = 1.f;

int width = 1280, height = 720;

float delta = 0;
float fps_cap = 60;

glm::mat4 slab_model = glm::mat4(1.0f);
glm::mat4 view       = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);

glm::vec3 block_rotation = glm::vec3(0,0,0);
glm::vec3 block_scale    = glm::vec3(1, 1.5, 0.25);

bool can_pan = false;

double old_mousey   = 0.0, old_mousex   = 0.0;
double delta_mousey = 0.0, delta_mousex = 0.0;

bool just_clicked = false;

glm::mat4 relative_plane = glm::mat4(1.f);

struct ObjExportData {
    std::vector<glm::vec3> vertices;
    std::vector<uint32_t>  indices;

    void append(const ObjExportData& other) {
        this->vertices.insert(this->vertices.end(), other.vertices.begin(), other.vertices.end());
        this->indices .insert(this->indices .end(), other.indices .begin(), other.indices .end());
    }

    void write(const std::array<char, 1024>& file_name) {
        std::ofstream file;
        file.open(file_name.data());

        file << "# TSA Braille Generator\n# https://github.com/Louisa-TSA/SoftwareDev2019\n";

        for(auto& v : vertices) {
            file << "v " << v.x << " " << v.y << " " << v.z << '\n';
        }
        for(size_t i = 0; i < indices.size(); i += 3) {
            file << "f " << indices[i]+1 << " " << indices[i+1]+1 << " " << indices[i+2]+1 << '\n';
        }

        file.close();
    }
};

class Cube {

public:

    glm::mat4 model    = glm::mat4(1.0f);
    glm::vec3 rotation = glm::vec3(0,0,0);
    glm::vec3 scale    = glm::vec3(0.25,0.25,0.25);
    glm::vec3 position = glm::vec3(0,0,0);

    void draw(GLShader& shader, GLElementBuffer& ebo, const glm::vec3& offset) {

        model = relative_plane;
        model = glm::translate(model, position + offset) * glm::mat4_cast(glm::quat(rotation));
        model = glm::scale(model, scale);

        shader.set_uniform("u_model", model);
        shader.set_uniform("u_view", view);
        shader.set_uniform("u_projection", projection);

        shader.set_uniform("u_frag_colour", 0.1f, 0.9f, 0.1f, 1.0f);

        GLCALL(glDrawElements(GL_TRIANGLES, ebo.get_count(), GL_UNSIGNED_INT, 0));

    }

    ObjExportData exportobj(size_t index, const glm::vec3& offset) {

        relative_plane = glm::translate(glm::mat4(1.f), glm::vec3(-(block_scale.x - 0.5) / 2, -(block_scale.y - 1.5) / 4, block_scale.z / 2));
        relative_plane = glm::scale(relative_plane, glm::vec3(0.5, 0.5, 0.5));

        model = relative_plane;
        model = glm::translate(model, position + offset);
        model = glm::scale(model, scale);

        ObjExportData data;
        for(size_t i = 0; i < sizeof(vertices) / sizeof(float); i += 3) {
            data.vertices.emplace_back(vertices[i], vertices[i + 1], vertices[i + 2]);
            data.vertices.back() = model * glm::vec4(data.vertices.back(), 1.f);
        }
        for(size_t i = 0; i < sizeof(indices) / sizeof(uint32_t); i++) {
            data.indices.emplace_back(indices[i] + ((sizeof(vertices) / sizeof(float)) / 3 * (index)));
        }
        return data;
    }

};

std::vector<std::vector<Cube>> bumps;

void update_rel_plane() {
    relative_plane   = glm::mat4_cast(glm::quat(block_rotation));
    relative_plane   = glm::translate(relative_plane, glm::vec3(-(block_scale.x - 0.5) / 2, -(block_scale.y - 1.5) / 4, block_scale.z / 2));
    relative_plane   = glm::scale(relative_plane, glm::vec3(0.5, 0.5, 0.5));
}

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
    char_map[','] = {
        {0, 0, 0}
    };
    char_map['.'] = {
        {0, 0, 0},
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
    glfwMakeContextCurrent(window);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed to initialize glad!" << std::endl;
        return 3;
    }

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    std::array<char, 1024> text_buf = {0};

    GLShader shader(shader_src);
    shader.bind();

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


    std::array<char, 1024> file_name = {0};

    bool first_frame = true;


    while(!glfwWindowShouldClose(window)) {


        glViewport(0, 0, width, height);
        glfwGetFramebufferSize(window, &width, &height);
        projection = glm::perspective(glm::radians(65.0f), static_cast<float>(static_cast<float>(width)/static_cast<float>(height)), 0.1f, 100.0f);



        slab_model = glm::mat4(1.f);
        slab_model = glm::translate(slab_model, glm::vec3(0,0,0)) * glm::mat4_cast(glm::quat(block_rotation));
        slab_model = glm::scale(slab_model, block_scale);



        float current_frame = glfwGetTime();
        delta = current_frame - last_frame;
        last_frame = current_frame;
        int64_t milliseconds = ((1.f/fps_cap) - delta) * 1000;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if(first_frame) {
            ImGui::SetNextWindowSize(ImVec2(width / 3, height / 3));
            ImGui::SetNextWindowPos(ImVec2(0, 0));
        }

        if(ImGui::Begin("Braille")) {
            ImGui::InputText("Text", text_buf.data(), text_buf.size());
            ImGui::InputFloat3("Size", &block_scale[0], 8);
            ImGui::SliderFloat("Character size", &char_size, 0.75f, 2.f);
            ImGui::Spacing();


            update_rel_plane();

            //if(ImGui::Button("Generate")) {
                bumps.clear();
                block_scale.x = 0;
                for(size_t i = 0; i < strlen(text_buf.data()); i++) {
                    block_scale.x += 1;
                    text_buf[i] = std::tolower(text_buf[i]);
                    bumps.emplace_back();
                    for(size_t j = 0; j < char_map[text_buf[i]].size(); j++) {
                        bumps[i].emplace_back();
                        bumps[i][j].position = char_map[text_buf[i]][j] * glm::vec3(char_size / 2);
                    }
                }
            //}

            ImGui::Spacing();

            if(ImGui::Button("Export")) {

                size_t str_len = strlen(file_name.data());
                if(str_len < 1) {
                    memcpy(file_name.data(), text_buf.data(), file_name.size());
                }
                str_len = strlen(file_name.data());
                file_name[str_len  ] = '.';
                file_name[str_len+1] = 'o';
                file_name[str_len+2] = 'b';
                file_name[str_len+3] = 'j';
                file_name[str_len+4] = '\0';

                slab_model = glm::mat4(1.f);
                slab_model = glm::translate(slab_model, glm::vec3(0,0,0));
                slab_model = glm::scale(slab_model, block_scale);

                ObjExportData obj;
                for(size_t i = 0; i < sizeof(vertices) / sizeof(float); i += 3) {
                    obj.vertices.emplace_back(vertices[i], vertices[i + 1], vertices[i + 2]);
                    obj.vertices.back() = slab_model * glm::vec4(obj.vertices.back(), 1.f);
                }
                for(size_t i = 0; i < sizeof(indices) / sizeof(uint32_t); i++) {
                    obj.indices.emplace_back(indices[i]);
                }
                size_t total = 1;
                for(size_t i = 0; i < bumps.size(); i++) {
                    for(size_t j = 0; j < bumps[i].size(); j++) {
                        obj.append(bumps[i][j].exportobj(total++, {i * 2, 0, 0}));
                    }
                }
                obj.write(file_name);

            }

            ImGui::SameLine(); ImGui::InputText("File name", file_name.data(), file_name.size());

        }

        ImGui::End();

        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        for(size_t i = 0; i < bumps.size(); i++) {
            for(auto& b : bumps[i]) {
                b.draw(shader, ebo, {i * 2, 0, 0});
            }
        }

        shader.set_uniform("u_model", slab_model);
        shader.set_uniform("u_view", view);
        shader.set_uniform("u_projection", projection);
        shader.set_uniform("u_frag_colour", 0.25f, 0.25f, 0.25f, 1.0f);

        GLCALL(glDrawElements(GL_TRIANGLES, ebo.get_count(), GL_UNSIGNED_INT, 0));
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();

        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));

        first_frame = false;
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