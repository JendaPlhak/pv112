// PV112 2017, lesson 4 - textures

#include <memory>

#include "libs.hpp"
#include "helpers.hpp"
#include "cuboid.hpp"
#include "ball.hpp"
#include "enemy.hpp"

using namespace std;
using namespace PV112;

// Current window size
int win_width = 1900;
int win_height = 1000;

// Shader program and its uniforms
GLuint program;

GLint model_matrix_loc;
GLint PVM_matrix_loc;
GLint normal_matrix_loc;

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

// Simple camera that allows us to look at the object from different views
PV112Camera my_camera;

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

// Called when the user presses a mouse button
void mouse_button_changed(int button, int state, int x, int y)
{
    my_camera.OnMouseButtonChanged(button, state, x, y);
}

// Called when the user moves with the mouse (when some mouse button is pressed)
void mouse_moved(int x, int y)
{
    my_camera.OnMouseMoved(x, y);
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
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }


    g_objects.push_back(std::move(std::make_unique<Cuboid>(program,
        glm::vec3(0, -2, 0), glm::vec3(3, 3, 3), Motion(false)
    )));
    // g_objects.push_back(std::move(std::make_unique<Ball>(
    //     program, glm::vec3(0,0,0.), 1, Motion(false)
    // )));
    // g_objects.push_back(std::move(std::make_unique<Ball>(
    //     program, glm::vec3(0,1.5,0.), 0.3, Motion({0, 1., 0}, 2)
    // )));


    // Walls
    for (const auto sign : {1, -1}) {
        for (uint32_t i = 0; i < 3; ++i) {
            glm::vec3 center(0);
            glm::vec3 widths(20);
            if (i == 2&& sign == 1) {
                center[i] = sign * 5.1;

            } else {
                center[i] = sign * 2.5;
            }
            widths[i] = 0.1;
            g_objects.push_back(std::move(std::make_unique<Cuboid>(program,
                center, widths, Motion(false)
            )));
        }
    }

    // g_objects.push_back(std::move(std::make_unique<Enemy>(dooms, program,
    //     glm::vec3(0,1,0), 0.6,  Motion(false)
    // )));

    // g_objects.push_back(std::move(std::make_unique<Ball>(
    //     program, glm::vec3(-1,0,0.), 0.4, Motion({1., 0.0, 0}, 7.)
    // )));
    // g_objects.push_back(std::move(std::make_unique<Ball>(
    //     program, glm::vec3(1,0.,0.), 0.3, Motion({-1., 0.0, 0}, 5.)
    // )));
    // g_objects.push_back(std::move(std::make_unique<Ball>(
    //     program, glm::vec3(0,1.,0.), 0.4, Motion({-0.33, 0.754, -1.2}, 5.)
    // )));
    // g_objects.push_back(std::move(std::make_unique<Cuboid>(program,
    //     glm::vec3(0,0,0), glm::vec3(0.01, 0.05, 0.02), Motion({-0.1, -1., 0}, 3.)
    // )));
    g_objects.push_back(std::move(std::make_unique<Cuboid>(program,
        glm::vec3(0,0,0), glm::vec3(0.01, 0.05, 0.02), Motion(false)
    )));
    g_objects.push_back(std::move(std::make_unique<Ball>(
        program, glm::vec3(1,1.,0.), 0.2, Motion({-1., -1.1, 0}, 3)
    )));
    g_objects.push_back(std::move(std::make_unique<Ball>(
        program, glm::vec3(0,0.,1.), 0.2, Motion({0., 0.1, 0.0}, 3.5)
    )));

    g_objects.push_back(std::move(std::make_unique<Ball>(program,
        glm::vec3(-2,0,0), 0.4, Motion({1., 0, 0}, 4.)
    )));
    g_objects.push_back(std::move(std::make_unique<Ball>(program,
        glm::vec3(0,0,2), 0.4, Motion({1., 0, 0}, 5.)
    )));


    // Lenna texture

    // lenna_tex = CreateAndLoadTexture(MAYBEWIDE("Lenna.png"));
    // glBindTexture(GL_TEXTURE_2D, lenna_tex);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    // glm::vec4 border_color = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
    // glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(border_color));
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
    // glGenerateMipmap(GL_TEXTURE_2D);
    // glBindTexture(GL_TEXTURE_2D, 0);


    // Rocks texture

    // rocks_tex = CreateAndLoadTexture(MAYBEWIDE("rocks.jpg"));
    // glBindTexture(GL_TEXTURE_2D, rocks_tex);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glGenerateMipmap(GL_TEXTURE_2D);
    // glBindTexture(GL_TEXTURE_2D, 0);


    // // Wood texture

    // wood_tex = CreateAndLoadTexture(MAYBEWIDE("wood.jpg"));
    // glBindTexture(GL_TEXTURE_2D, wood_tex);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glGenerateMipmap(GL_TEXTURE_2D);
    // glBindTexture(GL_TEXTURE_2D, 0);


    // Dice textures

    // dice_tex[0] = CreateAndLoadTexture(MAYBEWIDE("dice1.png"));
    // dice_tex[1] = CreateAndLoadTexture(MAYBEWIDE("dice2.png"));
    // dice_tex[2] = CreateAndLoadTexture(MAYBEWIDE("dice3.png"));
    // dice_tex[3] = CreateAndLoadTexture(MAYBEWIDE("dice4.png"));
    // dice_tex[4] = CreateAndLoadTexture(MAYBEWIDE("dice5.png"));
    // dice_tex[5] = CreateAndLoadTexture(MAYBEWIDE("dice6.png"));
    // for (int i = 0; i < 6; i++)
    // {
    //     glBindTexture(GL_TEXTURE_2D, dice_tex[i]);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //     glGenerateMipmap(GL_TEXTURE_2D);
    // }
    // glBindTexture(GL_TEXTURE_2D, 0);



    // Example of how to load a cube texture
    /*
    GLuint example_cube_tex =
    CreateAndLoadTextureCube(MAYBEWIDE("skybox_px.png"),
    MAYBEWIDE("skybox_nx.png"), MAYBEWIDE("skybox_py.png"),
    MAYBEWIDE("skybox_ny.png"), MAYBEWIDE("skybox_pz.png"),
    MAYBEWIDE("skybox_nz.png"));
    */
}

// Called when the window needs to be rerendered
void render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection_matrix, view_matrix, model_matrix, PVM_matrix;
    glm::mat3 normal_matrix;

    projection_matrix = glm::perspective(glm::radians(45.0f),
            float(win_width) / float(win_height), 0.1f, 100.0f);
    view_matrix = glm::lookAt(my_camera.GetEyePosition(),
            glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // Light position, with a simple animation
    glm::vec4 light1_pos =
        glm::rotate(glm::mat4(1.0f), app_time_s, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 2.0f, 0.0f)) *
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 light2_pos(0.0f, 0.0f, 3.0f, 1.0f);

    glUseProgram(program);

    glUniform3fv(eye_position_loc, 1, glm::value_ptr(my_camera.GetEyePosition()));

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
    if (app_time_s > 60) {
        throw "AAAA";
    }
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
    app_time_s += 0.050f;
    glutTimerFunc(50, timer, 0);
    glutPostRedisplay();
}

int main(int argc, char ** argv)
{
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

    // Set OpenGL Context parameters
    glutInitContextVersion(1, 3);
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
    glutTimerFunc(50, timer, 0);
    glutMouseFunc(mouse_button_changed);
    glutMotionFunc(mouse_moved);

    // Run the main loop
    glutMainLoop();

    return 0;
}
