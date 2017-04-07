// PV112 2017, lesson 4 - textures

#include <iostream>
#include "PV112.h"

#define _USE_MATH_DEFINES
#include <math.h>

// Include GLEW, use static library
#define GLEW_STATIC
#include <GL/glew.h>
#pragma comment(lib, "glew32s.lib") // Link with GLEW library

// Include DevIL for image loading
#if defined(_WIN32)
#pragma comment(lib, "glew32s.lib")
// On Windows, we use Unicode dll of DevIL
// That also means we need to use wide strings
#ifndef _UNICODE
#define _UNICODE
#include <IL/il.h>
#undef _UNICODE
#else
#include <IL/il.h>
#endif
#pragma comment(lib, "DevIL.lib") // Link with DevIL library
typedef wchar_t maybewchar;
#define MAYBEWIDE(s) L##s
#else // On Linux, we need regular (not wide) strings
#include <IL/il.h>
typedef char maybewchar;
#define MAYBEWIDE(s) s
#endif

// Include FreeGLUT
#include <GL/freeglut.h>

// Include the most important GLM functions
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "ball.hpp"
#include "box.hpp"
#include "helpers.hpp"

using namespace std;
using namespace PV112;

// Current window size
int win_width = 480;
int win_height = 480;

// Shader program and its uniforms
GLuint program;

GLint model_matrix_loc;
GLint PVM_matrix_loc;
GLint normal_matrix_loc;

GLint material_ambient_color_loc;
GLint material_diffuse_color_loc;
GLint material_specular_color_loc;
GLint material_shininess_loc;

GLint light_position_loc;
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

Ball g_ball;

// Current time of the application in seconds, for animations
float app_time_s = 0.0f;

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

    light_position_loc = glGetUniformLocation(program, "light_position");
    light_ambient_color_loc = glGetUniformLocation(program, "light_ambient_color");
    light_diffuse_color_loc = glGetUniformLocation(program, "light_diffuse_color");
    light_specular_color_loc = glGetUniformLocation(program, "light_specular_color");

    eye_position_loc = glGetUniformLocation(program, "eye_position");

    my_tex_loc = glGetUniformLocation(program, "my_tex");
    rocks_tex_loc = glGetUniformLocation(program, "rocks_tex");
    wood_tex_loc = glGetUniformLocation(program, "wood_tex");
    dice_tex_loc = glGetUniformLocation(program, "dice_tex");

    g_ball = Ball(program);

    // Create geometries
    my_cube = CreateCube(position_loc, normal_loc, tex_coord_loc);

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

    rocks_tex = CreateAndLoadTexture(MAYBEWIDE("rocks.jpg"));
    glBindTexture(GL_TEXTURE_2D, rocks_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);


    // Wood texture

    wood_tex = CreateAndLoadTexture(MAYBEWIDE("wood.jpg"));
    glBindTexture(GL_TEXTURE_2D, wood_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);


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
    glm::vec4 light_pos =
        glm::rotate(glm::mat4(1.0f), app_time_s, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 2.0f, 0.0f)) *
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    glUseProgram(program);

    glUniform3fv(eye_position_loc, 1, glm::value_ptr(my_camera.GetEyePosition()));

    glUniform4fv(light_position_loc, 1, glm::value_ptr(light_pos));
    glUniform3f(light_ambient_color_loc, 0.3f, 0.3f, 0.3f);
    glUniform3f(light_diffuse_color_loc, 1.0f, 1.0f, 1.0f);
    glUniform3f(light_specular_color_loc, 1.0f, 1.0f, 1.0f);

    // Cube
    // glBindVertexArray(my_cube.VAO);

    model_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f, 1.0f, 3.0f));
    // model_matrix = glm::mat4(1.0f);
    model_matrix = g_ball.get_model_matrix(app_time_s);


    glUniform1i(my_tex_loc, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, lenna_tex);


    glUniform3f(material_ambient_color_loc, 1.0f, 1.0f, 1.0f);
    glUniform3f(material_diffuse_color_loc, 1.0f, 1.0f, 1.0f);
    glUniform3f(material_specular_color_loc, 1.0f, 1.0f, 1.0f);
    glUniform1f(material_shininess_loc, 40.0f);

    PVM_matrix = projection_matrix * view_matrix * model_matrix;
    normal_matrix = glm::inverse(glm::transpose(glm::mat3(model_matrix)));
    glUniformMatrix4fv(model_matrix_loc, 1, GL_FALSE, glm::value_ptr(model_matrix));
    glUniformMatrix4fv(PVM_matrix_loc, 1, GL_FALSE, glm::value_ptr(PVM_matrix));
    glUniformMatrix3fv(normal_matrix_loc, 1, GL_FALSE, glm::value_ptr(normal_matrix));

#if 1
    // Draw a cube ...
    // DrawGeometry(my_cube);
#else

    // ... or draw a dice
    glUniform1i(wood_tex_loc, 0); // Choose proper texture unit
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, wood_tex);
    DrawGeometry(my_cube);

    glUniform1i(rocks_tex_loc, 1); // Choose proper texture unit
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, rocks_tex);

    glUniform1i(dice_tex_loc, 2); // Choose proper texture unit
    glActiveTexture(GL_TEXTURE2);

    glBindTexture(GL_TEXTURE_2D, dice_tex[0]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT,
            (const void *)(sizeof(unsigned int) *  0)); // Front face
    glBindTexture(GL_TEXTURE_2D, dice_tex[1]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT,
            (const void *)(sizeof(unsigned int) *  6)); // Right face
    glBindTexture(GL_TEXTURE_2D, dice_tex[5]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT,
            (const void *)(sizeof(unsigned int) * 12)); // Back face
    glBindTexture(GL_TEXTURE_2D, dice_tex[4]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT,
            (const void *)(sizeof(unsigned int) * 18)); // Left face
    glBindTexture(GL_TEXTURE_2D, dice_tex[2]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT,
            (const void *)(sizeof(unsigned int) * 24)); // Top face
    // glBindTexture(GL_TEXTURE_2D, dice_tex[3]);
    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT,
    //         (const void *)(sizeof(unsigned int) * 30)); // Bottom face
#endif

    g_ball.render(app_time_s);

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
    app_time_s += 0.020f;
    glutTimerFunc(20, timer, 0);
    glutPostRedisplay();
}

int main(int argc, char ** argv)
{
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
    glutTimerFunc(20, timer, 0);
    glutMouseFunc(mouse_button_changed);
    glutMotionFunc(mouse_moved);

    // Run the main loop
    glutMainLoop();

    return 0;
}
