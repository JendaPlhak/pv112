// PV112 2017, lesson 4 - textures
#include <algorithm>
#include <memory>
#include <time.h>
#include <random>

#include "game/libs.hpp"
#include "game/game.hpp"

#include <imgui/imgui.h>
#include "game/imgui_impl_glfw_gl3.h"

#include "game/PV112.h"
#include "game/helpers.hpp"
#include "game/cuboid.hpp"
#include "game/ball.hpp"
#include "game/enemy.hpp"

using namespace std;
using namespace PV112;

constexpr uint SLEEP_MS = 15;
// Current window size
int win_width = 1900;
int win_height = 1000;

bool exit_game = false;
bool fire = false;

using namespace irrklang;
ISoundEngine *SoundEngine;

// Shader program and its uniforms
GLuint program;

GLint model_matrix_loc;
GLint PVM_matrix_loc;
GLint normal_matrix_loc;
GLint tex_scale_loc;

GLint material_ambient_color_loc;
GLint material_diffuse_color_loc;
GLint material_specular_color_loc;
GLint material_shininess_loc;

GLint light1_position_loc;
GLint light2_position_loc;
GLint light_diffuse_color_loc;
GLint light_ambient_color_loc;
GLint light_specular_color_loc;

GLint eye_position_loc;

// Simple geometries that we will use in this lecture
PV112Geometry my_cube;

//Space boundaries
using Bound = std::array<float, 2>;
std::array<Bound, 3> bounds = {
    Bound({-15, 15}), Bound({0, 7}), Bound({-15, 15})
};
std::array<bool, 4> arrows_pressed = {false, false, false, false};
// Simple camera that allows us to look at the object from different views
PV112Camera my_camera(bounds);

auto light1_pos = glm::vec4(-5.5, 6.6, -11, 1.);
auto light2_pos = glm::vec4(5.5, 6.6, 11, 1.);

// OpenGL texture objects
GLuint rocks_tex;
GLuint metal_tex;
GLuint spike_tex;
GLuint glass_tex;
GLuint dice_tex[6];

std::vector<std::shared_ptr<Object>> g_objects;
std::vector<std::shared_ptr<Object>> g_enemies;

// Current time of the application in seconds, for animations
float app_time_s = 0.0f;
float prev_time_s = 0.0f;
float close_time_s = std::numeric_limits<float>::max();
float last_fired = 0.f;

GameOptions game_opts;

// Simple timer function for animations
void timer()
{
    prev_time_s = app_time_s;
    app_time_s = glfwGetTime();
}

void CheckArrowPressed(int key)
{
    if (key == GLFW_KEY_UP) {
        arrows_pressed.at(0) = true;
    }
    // Move backward
    if (key == GLFW_KEY_DOWN) {
        arrows_pressed.at(1) = true;
    }
    // Strafe right
    if (key == GLFW_KEY_RIGHT) {
        arrows_pressed.at(2) = true;
    }
    // Strafe left
    if (key == GLFW_KEY_LEFT) {
        arrows_pressed.at(3) = true;
    }
}

void CheckArrowReleased(int key)
{
    if (key == GLFW_KEY_UP) {
        arrows_pressed.at(0) = false;
    }
    // Move backward
    if (key == GLFW_KEY_DOWN) {
        arrows_pressed.at(1) = false;
    }
    // Strafe right
    if (key == GLFW_KEY_RIGHT) {
        arrows_pressed.at(2) = false;
    }
    // Strafe left
    if (key == GLFW_KEY_LEFT) {
        arrows_pressed.at(3) = false;
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        exit_game = true;
    }
    if (action == GLFW_PRESS) {
        CheckArrowPressed(key);
    } else if (action == GLFW_RELEASE) {
        CheckArrowReleased(key);
    }
    if (app_time_s > game_opts.game_time) {
        arrows_pressed.fill(false);
    }
}

auto random_range = [](float LO, float HI) {
    return LO + (HI-LO) * ((rand() % 100) / 100.f);
};

void fire_ball() {
    const auto position = my_camera.get_position();
    const float radius = random_range(0.1, 0.3);
    const float speed = random_range(5., 17.);
    auto dir = glm::normalize(my_camera.get_direction());
    dir *= (radius + 0.6);

    g_objects.push_back(std::move(std::make_shared<Ball>(program,
        position + dir, radius, Motion(dir, speed)
    )));
    g_objects.back()->set_expiration_time(app_time_s + game_opts.ball_time);
    SoundEngine->play2D("audio/fire.mp3", GL_FALSE);
}

// Called when the user presses a mouse button
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (app_time_s > game_opts.game_time) {
        return;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS && game_opts.machine_gun) {
            fire = true;
        }
        if (action == GLFW_RELEASE) {
            fire = false;
            if (!game_opts.machine_gun) {
                fire_ball();
            }
        }
    }
}

// Called when the user moves with the mouse
void mouse_moved(GLFWwindow* window, double x, double y)
{
    my_camera.OnMouseMoved(x, y, app_time_s - prev_time_s);
}

void bind_tex(const GLuint tex)
{
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void init_imgui(GLFWwindow* window)
{
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("extern/imgui/extra_fonts/Cousine-Regular.ttf", 40.0f);
    io.MouseDrawCursor = false;
    // Setup ImGui binding
    ImGui_ImplGlfwGL3_Init(window, false);
}

// Initializes OpenGL stuff
void init()
{
    timer();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    glEnable(GL_DEPTH_TEST);

    // Create shader program
    program = CreateAndLinkProgram("vertex.glsl", "fragment.glsl");
    if (0 == program)
        WaitForEnterAndExit();

    // Get attribute and uniform locations
    int position_loc = glGetAttribLocation(program, "position");
    int normal_loc = glGetAttribLocation(program, "normal");
    int tex_coord_loc = glGetAttribLocation(program, "tex_coord");

    model_matrix_loc = glGetUniformLocation(program, "model_matrix");
    PVM_matrix_loc = glGetUniformLocation(program, "PVM_matrix");
    normal_matrix_loc = glGetUniformLocation(program, "normal_matrix");
    tex_scale_loc = glGetUniformLocation(program, "tex_scale");

    material_ambient_color_loc = glGetUniformLocation(program, "material_ambient_color");
    material_diffuse_color_loc = glGetUniformLocation(program, "material_diffuse_color");
    material_specular_color_loc = glGetUniformLocation(program, "material_specular_color");
    material_shininess_loc = glGetUniformLocation(program, "material_shininess");

    light1_position_loc = glGetUniformLocation(program, "light1_position");
    light2_position_loc = glGetUniformLocation(program, "light2_position");
    light_ambient_color_loc = glGetUniformLocation(program, "light_ambient_color");
    light_diffuse_color_loc = glGetUniformLocation(program, "light_diffuse_color");
    light_specular_color_loc = glGetUniformLocation(program, "light_specular_color");

    eye_position_loc = glGetUniformLocation(program, "eye_position");

    std::vector<GLuint> dooms;
    for (uint32_t i = 0; i < 7; ++i) {
        std::string path("img/doom" + std::to_string(i) + ".png");
        auto tex = PV112::CreateAndLoadTexture(path.c_str());
        dooms.push_back(tex);
        bind_tex(tex);
    }

    auto metal_tex = PV112::CreateAndLoadTexture("img/table_metal.jpg");
    auto spike_tex = PV112::CreateAndLoadTexture("img/spikes.jpg");
    auto stone_tex = PV112::CreateAndLoadTexture("img/rocks.jpg");
    auto glass_tex = PV112::CreateAndLoadTexture("img/glass.jpg");
    bind_tex(metal_tex);
    bind_tex(spike_tex);
    bind_tex(stone_tex);
    bind_tex(glass_tex);


    auto bulb_obj = PV112::LoadOBJ("obj/bulb.obj", position_loc, normal_loc, tex_coord_loc);
    auto table_obj = PV112::LoadOBJ("obj/table.obj", position_loc, normal_loc, tex_coord_loc);
    auto box_obj = PV112::LoadOBJ("obj/box.obj", position_loc, normal_loc, tex_coord_loc);
    auto cube_obj = PV112::CreateCube(position_loc, normal_loc, tex_coord_loc);


    // Walls
    for (const auto dir : {0, 1}) {
        for (uint32_t i = 0; i < 3; ++i) {
            glm::vec3 center(0);
            glm::vec3 widths(20);
            const float thickness = 0.4;

            center[i] = bounds[i][dir] + thickness * (dir ? 1 : -1);
            widths[i] = thickness;
            g_objects.push_back(std::move(std::make_shared<Cuboid>(program, cube_obj,
                stone_tex, center, widths, Motion(false)
            )));
        }
    }


    // Table in the middle
    g_objects.push_back(std::move(std::make_shared<Cuboid>(program, table_obj,
        metal_tex, glm::vec3(0, 0, 0), glm::vec3(4.5, 4, 4.5), Motion(false)
    )));
    // Balls on the table
    {
        std::vector<std::array<int, 2>> p = {
            {1, 1}, {-1, 1}, {1, -1}, {-1, -1}
        };
        for (unsigned i = 0; i < 4; ++i) {
            g_objects.push_back(std::move(std::make_shared<Ball>(program,
                glm::vec3(1*p[i][0], 2. + i, 1*p[i][1]), 0.25,
                Motion({0, 1, 0}, 3.)
            )));
        }
    }
    // Boxes
    auto create_boxes = [&](const unsigned D) {
        auto one = []() {
            return rand() % 2 == 0 ? 1 : -1;
        };
        const int width = bounds[D][1] - bounds[D][0];
        const float spread = 3.;
        const int count = width / spread;

        glm::vec3 dir(0, 1, 0);
        for (unsigned i = 0; i < count; ++i) {
            glm::vec3 position(0, i/2. + 2, 0);
            position[D] = bounds[0][0] + i*spread;
            dir[D] = one();

            g_objects.push_back(std::move(std::make_shared<Cuboid>(program, box_obj,
                spike_tex, position, glm::vec3(0.5, 0.75, 0.4), Motion(dir, 3.)
            )));
        }
    };
    create_boxes(0);
    create_boxes(2);
    MaterialProperties props = {
        .ambient_color = glm::vec3(0.315f),
        .diffuse_color = glm::vec3(0.),
        .specular_color = glm::vec3(0.),
        .shininess = 0
    };

    // Make some light bulbs
    g_objects.push_back(std::move(std::make_shared<Cuboid>(program, bulb_obj,
        glass_tex, glm::vec3(light1_pos) + glm::vec3(0, 0.2, 0),
        glm::vec3(0.5, 0.5, 0.5), Motion(false)
    )));
    g_objects.back()->set_material_properties(props);

    g_objects.push_back(std::move(std::make_shared<Cuboid>(program, bulb_obj,
        glass_tex, glm::vec3(light2_pos) + glm::vec3(0, 0.2, 0),
        glm::vec3(0.5, 0.5, 0.5), Motion(false)
    )));
    g_objects.back()->set_material_properties(props);

    // Create enemies
    {
        std::vector<std::array<int, 2>> p = {
            {1, 1}, {-1, 1}, {1, -1}, {-1, -1}
        };
        for (unsigned i = 0; i < 5; ++i) {
            for (unsigned j = 0; j < 4; ++j) {
                float s = 2.5 * (i + 1);
                g_enemies.push_back(std::move(std::make_shared<Enemy>(dooms,
                    SoundEngine, program, 0,
                    glm::vec3(s*p[j][0], i + 2, s*p[j][1]), 1. / (i + 1),
                    Motion(false)
                )));
            }
        }
    }
    g_objects.insert(g_objects.end(), g_enemies.begin(), g_enemies.end());


    // Play some music please
    SoundEngine->play2D("audio/kill_them_all.mp3", GL_TRUE);
}

void clear_objects() {
    auto clear = [](auto& objects) {
        objects.erase(std::remove_if(objects.begin(), objects.end(),
                [](const auto& obj) {
                    return obj->is_expired(app_time_s);
                }),
            objects.end());
    };
    clear(g_objects);
    clear(g_enemies);
}

// Called when the window needs to be rendered
void render()
{
    my_camera.ProcessArrowKeys(arrows_pressed, app_time_s - prev_time_s);

    clear_objects();
    for (size_t i = 0; i < g_objects.size(); ++i) {
        const auto& obj_A = g_objects.at(i);
        for (size_t j = i + 1; j < g_objects.size(); ++j) {
            const auto& obj_B = g_objects.at(j);
            if (!obj_A->is_active() && !obj_B->is_active()) {
                continue;
            }
            if (obj_A->check_collision(*obj_B)) {
                obj_A->bounce(*obj_B, app_time_s);
            }
        }
    }


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection_matrix, view_matrix, model_matrix, PVM_matrix;
    glm::mat3 normal_matrix;

    projection_matrix = glm::perspective(glm::radians(45.0f),
            float(win_width) / float(win_height), 0.1f, 100.0f);
    view_matrix = my_camera.get_view_matrix();


    glUseProgram(program);

    glUniform3fv(eye_position_loc, 1, glm::value_ptr(my_camera.get_position()));

    glUniform4fv(light1_position_loc, 1, glm::value_ptr(light1_pos));
    glUniform4fv(light2_position_loc, 1, glm::value_ptr(light2_pos));
    glUniform3f(light_ambient_color_loc, 1.0f, 1.0f, 1.0f);
    glUniform3f(light_diffuse_color_loc, 1.0f, 1.0f, 1.0f);
    glUniform3f(light_specular_color_loc, 1.0f, 1.0f, 1.0f);

    for (const auto& obj : g_objects) {
        const auto& p = obj->get_material_properties();
        glUniform3fv(material_ambient_color_loc, 1, glm::value_ptr(p.ambient_color));
        glUniform3fv(material_diffuse_color_loc, 1, glm::value_ptr(p.diffuse_color));
        glUniform3fv(material_specular_color_loc, 1, glm::value_ptr(p.specular_color));
        glUniform1f(material_shininess_loc, p.shininess);

        glUniform1f(tex_scale_loc, obj->get_max_scale());
        model_matrix = obj->update_geometry(app_time_s - prev_time_s);

        PVM_matrix = projection_matrix * view_matrix * model_matrix;
        normal_matrix = glm::inverse(glm::transpose(glm::mat3(model_matrix)));
        glUniformMatrix4fv(model_matrix_loc, 1, GL_FALSE, glm::value_ptr(model_matrix));
        glUniformMatrix4fv(PVM_matrix_loc, 1, GL_FALSE, glm::value_ptr(PVM_matrix));
        glUniformMatrix3fv(normal_matrix_loc, 1, GL_FALSE, glm::value_ptr(normal_matrix));

        obj->render(app_time_s);
    }

    glBindVertexArray(0);
    glUseProgram(0);
}

// Callback function to be called when we make an error in OpenGL
void GLAPIENTRY simple_debug_callback(GLenum source, GLenum type, GLuint id,
        GLenum severity, GLsizei length, const char* message, const void* userParam)
{
    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        cout << message << endl; // Put the breakpoint here
        return;
    default:
        return;
    }
}

void get_resolution() {
    const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    win_width = mode->width;
    win_height = mode->height;
}

int alive_enemies() {
    return std::count_if(g_enemies.begin(), g_enemies.end(), [](const auto& enemy) {
        return std::static_pointer_cast<Enemy>(enemy)->is_alive();
    });
}

int run_game(const GameOptions& opts)
{
    srand(time(NULL));
    SoundEngine = createIrrKlangDevice();
    exit_game = false;
    game_opts = opts;

    GLFWwindow* window;
    /* Initialize the library */
    if (!glfwInit()) {
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    get_resolution();

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(win_width, win_height, "The Game",
        glfwGetPrimaryMonitor(), NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, mouse_moved);
    glfwSetKeyCallback(window, key_callback);
    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    glewInit();

    // Initialize DevIL library
    ilInit();

    init_imgui(window);
    init();

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window) && !exit_game && close_time_s >= app_time_s)
    {
        /* Poll for and process events */
        glfwPollEvents();
        ImGui_ImplGlfwGL3_NewFrame();
        timer();

        if (fire && last_fired + 0.1 < app_time_s) {
            fire_ball();
            last_fired = app_time_s;
        }

        int remaining_enemies = alive_enemies();
        if (app_time_s < game_opts.game_time && remaining_enemies != 0) {
            ImGui::Text(" ---- PLAY! ----");
            ImGui::Text("REMAINING ENEMIES: %d", alive_enemies());
        } else {
            fire = false;
            if (remaining_enemies == 0) {
                ImGui::Text(" ---- YOU WON! ---- ");
            } else {
                ImGui::Text(" ---- YOU LOST! ---- ");
            }
            if (close_time_s == std::numeric_limits<float>::max()) {
                close_time_s = app_time_s + 10.;
            }
        }
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        render();
        ImGui::Render();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);
    }

    ImGui_ImplGlfwGL3_Shutdown();
    SoundEngine->drop();
    glfwDestroyWindow(window);
    return 0;
}
