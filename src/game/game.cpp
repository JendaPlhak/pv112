// PV112 2017, lesson 4 - textures

#include <memory>
#include <time.h>
#include <random>

#include "game/libs.hpp"
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

GLint my_tex_loc;
GLint rocks_tex_loc;
GLint wood_tex_loc;
GLint dice_tex_loc;

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

// OpenGL texture objects
GLuint lenna_tex;
GLuint rocks_tex;
GLuint wood_tex;
GLuint alpha_circle_tex;
GLuint dice_tex[6];

std::vector<std::unique_ptr<Object>> g_objects;

// Current time of the application in seconds, for animations
float app_time_s = 0.0f;
float prev_time_s = 0.0f;

// Called when the user presses a key
void key_pressed(unsigned char key, int mouseX, int mouseY)
{
    switch (key)
    {
    case 27: // Escape
        exit(0);
        break;
    case 'l':
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glutPostRedisplay();
        break;
    case 'f':
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glutPostRedisplay();
        break;
    case 't':
        glutFullScreenToggle();
        break;
    }
}

auto random_range = [](float LO, float HI) {
    return LO + (HI-LO) * ((rand() % 100) / 100.f);
};

// Called when the user presses a mouse button
void mouse_button_changed(int button, int state, int x, int y)
{
    const auto position = my_camera.get_position();

    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_UP) {
            const float radius = random_range(0.1, 0.3);
            const float speed = random_range(5., 17.);
            auto dir = glm::normalize(my_camera.get_direction());
            dir *= (radius + 0.6);

            g_objects.push_back(std::move(std::make_unique<Ball>(program,
                position + dir, radius, Motion(dir, speed)
            )));
        }
    } else if (button == GLUT_RIGHT_BUTTON) {
        if (state == GLUT_UP) {
            const float radius = random_range(0.3, 0.5);
            const float speed = random_range(3., 9.);
            auto dir = glm::normalize(my_camera.get_direction());
            dir *= (radius + 0.6);
            g_objects.push_back(std::move(std::make_unique<Ball>(program,
                position + dir, radius, Motion(dir, speed)
            )));
        }
    }
}

// Called when the user moves with the mouse (when some mouse button is pressed)
void mouse_moved(int x, int y)
{
    my_camera.OnMouseMoved(x, y, app_time_s - prev_time_s);
}

void SpecialInputPressed(int key, int x, int y)
{
    if (key == GLUT_KEY_UP) {
        arrows_pressed.at(0) = true;
    }
    // Move backward
    if (key == GLUT_KEY_DOWN) {
        arrows_pressed.at(1) = true;
    }
    // Strafe right
    if (key == GLUT_KEY_RIGHT) {
        arrows_pressed.at(2) = true;
    }
    // Strafe left
    if (key == GLUT_KEY_LEFT) {
        arrows_pressed.at(3) = true;
    }
}

void SpecialInputReleased(int key, int x, int y)
{
    if (key == GLUT_KEY_UP) {
        arrows_pressed.at(0) = false;
    }
    // Move backward
    if (key == GLUT_KEY_DOWN) {
        arrows_pressed.at(1) = false;
    }
    // Strafe right
    if (key == GLUT_KEY_RIGHT) {
        arrows_pressed.at(2) = false;
    }
    // Strafe left
    if (key == GLUT_KEY_LEFT) {
        arrows_pressed.at(3) = false;
    }
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

// Initializes OpenGL stuff
void init()
{
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

    my_tex_loc = glGetUniformLocation(program, "my_tex");
    rocks_tex_loc = glGetUniformLocation(program, "rocks_tex");
    wood_tex_loc = glGetUniformLocation(program, "wood_tex");
    dice_tex_loc = glGetUniformLocation(program, "dice_tex");

    std::vector<GLuint> dooms;
    for (uint32_t i = 0; i < 7; ++i) {
        std::string path("img/doom" + std::to_string(i) + ".png");
        auto tex = PV112::CreateAndLoadTexture(path.c_str());
        dooms.push_back(tex);
        bind_tex(tex);
    }

    auto wood_tex = PV112::CreateAndLoadTexture("img/wood.jpg");
    auto stone_tex = PV112::CreateAndLoadTexture("img/rocks.jpg");
    bind_tex(wood_tex);
    bind_tex(stone_tex);


    auto library_obj = PV112::LoadOBJ("obj/library.obj", position_loc, normal_loc, tex_coord_loc);
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
            g_objects.push_back(std::move(std::make_unique<Cuboid>(program, cube_obj,
                stone_tex, center, widths, Motion(false)
            )));
        }
    }


    // Table in the middle
    g_objects.push_back(std::move(std::make_unique<Cuboid>(program, table_obj,
        wood_tex, glm::vec3(0, 0, 0), glm::vec3(4.5, 4, 4.5), Motion(false)
    )));
    // Balls on the table
    {
        std::vector<std::array<int, 2>> p = {
            {1, 1}, {-1, 1}, {1, -1}, {-1, -1}
        };
        for (unsigned i = 0; i < 4; ++i) {
            g_objects.push_back(std::move(std::make_unique<Ball>(program,
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

            g_objects.push_back(std::move(std::make_unique<Cuboid>(program, box_obj,
                wood_tex, position, glm::vec3(0.5, 0.75, 0.4), Motion(dir, 3.)
            )));
        }
    };
    create_boxes(0);
    create_boxes(2);

    // Make some libs
    g_objects.push_back(std::move(std::make_unique<Cuboid>(program, library_obj,
        wood_tex, glm::vec3(-11, 1.5, -11), glm::vec3(3, 3, 3), Motion(false)
    )));
     g_objects.push_back(std::move(std::make_unique<Cuboid>(program, library_obj,
        wood_tex, glm::vec3(11, 1.5, -11), glm::vec3(3, 3, 3), Motion(false)
    )));

    // Create enemies
    {
        std::vector<std::array<int, 2>> p = {
            {1, 1}, {-1, 1}, {1, -1}, {-1, -1}
        };
        for (unsigned i = 0; i < 5; ++i) {
            for (unsigned j = 0; j < 4; ++j) {
                float s = 2.5 * (i + 1);
                g_objects.push_back(std::move(std::make_unique<Enemy>(dooms, program, 0,
                    glm::vec3(s*p[j][0], i + 2, s*p[j][1]), 1. / (i + 1),
                    Motion(false)
                )));
            }
        }
    }
}

// Called when the window needs to be rendered
void render()
{
    my_camera.ProcessArrowKeys(arrows_pressed, app_time_s - prev_time_s);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection_matrix, view_matrix, model_matrix, PVM_matrix;
    glm::mat3 normal_matrix;

    projection_matrix = glm::perspective(glm::radians(45.0f),
            float(win_width) / float(win_height), 0.1f, 100.0f);
    view_matrix = my_camera.get_view_matrix();

    // Light position, with a simple animation
    glm::vec4 light1_pos =
        glm::rotate(glm::mat4(1.0f), app_time_s, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 6.0f, -5.0f)) *
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 light2_pos =
        glm::rotate(glm::mat4(1.0f), -1.76794f*app_time_s + 2.11f,
            glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::translate(glm::mat4(1.0f), glm::vec3(-5.0f, 6.0f, 5.0f)) *
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    glUseProgram(program);

    glUniform3fv(eye_position_loc, 1, glm::value_ptr(my_camera.get_position()));

    glUniform4fv(light1_position_loc, 1, glm::value_ptr(light1_pos));
    glUniform4fv(light2_position_loc, 1, glm::value_ptr(light2_pos));
    glUniform3f(light_ambient_color_loc, 0.3f, 0.3f, 0.3f);
    glUniform3f(light_diffuse_color_loc, 1.0f, 1.0f, 1.0f);
    glUniform3f(light_specular_color_loc, 1.0f, 1.0f, 1.0f);

    glUniform3f(material_ambient_color_loc, 1.0f, 1.0f, 1.0f);
    glUniform3f(material_diffuse_color_loc, 1.0f, 1.0f, 1.0f);
    glUniform3f(material_specular_color_loc, 1.0f, 1.0f, 1.0f);
    glUniform1f(material_shininess_loc, 40.0f);

    for (size_t i = 0; i < g_objects.size(); ++i) {
        const auto& obj_A = g_objects.at(i);
        for (size_t j = i + 1; j < g_objects.size(); ++j) {
            const auto& obj_B = g_objects.at(j);
            if (!obj_A->is_active() && !obj_B->is_active()) {
                continue;
            }
            if (obj_A->check_collision(*obj_B)) {
                obj_A->bounce(*obj_B);
            }
        }
    }

    for (const auto& obj : g_objects) {
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

    glutSwapBuffers();
}

// Called when the window changes its size
void reshape(int width, int height)
{
    win_width = width;
    win_height = height;

    // Set the area into which we render
    glViewport(0, 0, win_width, win_height);
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

// Simple timer function for animations
void timer(int)
{
    prev_time_s = app_time_s;
    app_time_s += SLEEP_MS / 1000.f;
    glutTimerFunc(SLEEP_MS, timer, 0);
    glutPostRedisplay();
}

int main(int argc, char ** argv)
{
    srand(time(NULL));
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

    // Set OpenGL Context parameters
    glutInitContextVersion(3, 3);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitContextFlags(GLUT_DEBUG);

    // Initialize window
    glutInitWindowSize(win_width, win_height);
    glutCreateWindow("PV112 - cv4 - textures");

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    glewInit();

    // Initialize DevIL library
    ilInit();

    // Set the debug callback
    SetDebugCallback(simple_debug_callback);

    // Initialize our OpenGL stuff
    init();

    // Register callbacks
    glutDisplayFunc(render);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(key_pressed);
    glutTimerFunc(SLEEP_MS, timer, 0);
    glutMouseFunc(mouse_button_changed);
    glutPassiveMotionFunc(mouse_moved);
    glutSpecialFunc(SpecialInputPressed);
    glutSpecialUpFunc(SpecialInputReleased);
    glutFullScreenToggle();

    // Run the main loop
    glutMainLoop();

    return 0;
}
