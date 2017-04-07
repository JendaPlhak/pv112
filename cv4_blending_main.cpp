// PV112 2017, lesson 4 - blending

#include <iostream>
#include "PV112.h"

#define _USE_MATH_DEFINES
#include <math.h>

// Include GLEW, use static library
#define GLEW_STATIC
#include <GL/glew.h>
#pragma comment(lib, "glew32s.lib")        // Link with GLEW library

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

using namespace std;
using namespace PV112;

// Current window size
int win_width = 480;
int win_height = 480;

// Shader program for cube with uniform locations
GLuint cube_program;

GLint cube_PVM_matrix_loc;
GLint cube_my_color_tex_loc;
GLint cube_my_alpha_tex_loc;

// Shader program for cube with uniform locations
GLuint teapot_program;

GLint teapot_model_matrix_loc;
GLint teapot_PVM_matrix_loc;
GLint teapot_normal_matrix_loc;

GLint teapot_material_ambient_color_loc;
GLint teapot_material_diffuse_color_loc;
GLint teapot_material_specular_color_loc;
GLint teapot_material_shininess_loc;

GLint teapot_light_position_loc;
GLint teapot_light_diffuse_color_loc;
GLint teapot_light_ambient_color_loc;
GLint teapot_light_specular_color_loc;

GLint teapot_eye_position_loc;

GLint teapot_my_color_tex_loc;
GLint teapot_my_alpha_tex_loc;

// Simple geometries that we will use in this lecture
PV112Geometry my_cube;
PV112Geometry my_sphere;
PV112Geometry my_teapot;

// Simple camera that allows us to look at the object from different views
PV112Camera my_camera;

// OpenGL texture objects
GLuint lenna_tex;
GLuint alpha_circle_tex;

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

// Loads a texture from file and calls glTexImage2D to se its data.
// Returns true on success or false on failure.
bool LoadAndSetTexture(const maybewchar *filename, GLenum target)
{
    // Create IL image
    ILuint IL_tex;
    ilGenImages(1, &IL_tex);

    ilBindImage(IL_tex);

    // Solve upside down textures
    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_LOWER_LEFT);

    // Load IL image
    ILboolean success = ilLoadImage(filename);
    if (!success)
    {
        ilBindImage(0);
        ilDeleteImages(1, &IL_tex);
        cerr << "Couldn't load texture: " << filename << endl;
        return false;
    }

    // Get IL image parameters
    int img_width = ilGetInteger(IL_IMAGE_WIDTH);
    int img_height = ilGetInteger(IL_IMAGE_HEIGHT);
    int img_format = ilGetInteger(IL_IMAGE_FORMAT);
    int img_type = ilGetInteger(IL_IMAGE_TYPE);

    // Choose internal format, format, and type for glTexImage2D
    GLint internal_format = 0;
    GLenum format = 0;
    GLenum type = img_type; // IL constants matches GL constants
    switch (img_format)
    {
    case IL_RGB:  internal_format = GL_RGB;  format = GL_RGB;  break;
    case IL_RGBA: internal_format = GL_RGBA; format = GL_RGBA; break;
    case IL_BGR:  internal_format = GL_RGB;  format = GL_BGR;  break;
    case IL_BGRA: internal_format = GL_RGBA; format = GL_BGRA; break;
    case IL_COLOR_INDEX:
    case IL_ALPHA:
    case IL_LUMINANCE:
    case IL_LUMINANCE_ALPHA:
        // Unsupported format
        ilBindImage(0);
        ilDeleteImages(1, &IL_tex);
        cerr << "Texture " << filename << " has unsupported format\n";
        return false;
    }

    // Set the data to OpenGL (assumes texture object is already bound)
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(target, 0, internal_format, img_width, img_height, 0, format,
            type, ilGetData());

    // Unset and delete IL texture
    ilBindImage(0);
    ilDeleteImages(1, &IL_tex);

    return true;
}

GLuint CreateAndLoadTexture(const maybewchar *filename)
{
    // Create OpenGL texture object
    GLuint tex_obj;
    glGenTextures(1, &tex_obj);
    glBindTexture(GL_TEXTURE_2D, tex_obj);

    // Load the data into OpenGL texture object
    if (!LoadAndSetTexture(filename, GL_TEXTURE_2D))
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        glDeleteTextures(1, &tex_obj);
        return 0;
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    return tex_obj;
}

GLuint CreateAndLoadTextureCube(
    const maybewchar *filename_px, const maybewchar *filename_nx,
    const maybewchar *filename_py, const maybewchar *filename_ny,
    const maybewchar *filename_pz, const maybewchar *filename_nz)
{
    // Create OpenGL texture object
    GLuint tex_obj;
    glGenTextures(1, &tex_obj);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex_obj);

    // Load the data into OpenGL texture object
    if (
        !LoadAndSetTexture(filename_px, GL_TEXTURE_CUBE_MAP_POSITIVE_X) ||
        !LoadAndSetTexture(filename_nx, GL_TEXTURE_CUBE_MAP_NEGATIVE_X) ||
        !LoadAndSetTexture(filename_py, GL_TEXTURE_CUBE_MAP_POSITIVE_Y) ||
        !LoadAndSetTexture(filename_ny, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y) ||
        !LoadAndSetTexture(filename_pz, GL_TEXTURE_CUBE_MAP_POSITIVE_Z) ||
        !LoadAndSetTexture(filename_nz, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z))
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        glDeleteTextures(1, &tex_obj);
        return 0;
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return tex_obj;
}

// Initializes OpenGL stuff
void init()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    glEnable(GL_DEPTH_TEST);

    // Force indices of input variables
    int position_loc = 0;
    int normal_loc = 1;
    int tex_coord_loc = 2;

    // Create shader program for cube
    cube_program = CreateAndLinkProgram("cube_vertex.glsl", "cube_fragment.glsl",
        position_loc, "position", normal_loc, "normal", tex_coord_loc, "tex_coord");
    if (0 == cube_program)
        WaitForEnterAndExit();

    // Get uniform locations
    cube_PVM_matrix_loc = glGetUniformLocation(cube_program, "PVM_matrix");

    cube_my_alpha_tex_loc = glGetUniformLocation(cube_program, "my_alpha_tex");
    cube_my_color_tex_loc = glGetUniformLocation(cube_program, "my_color_tex");

    // Create shader program for teapot
    teapot_program = CreateAndLinkProgram("teapot_vertex.glsl", "teapot_fragment.glsl",
        position_loc, "position", normal_loc, "normal", tex_coord_loc, "tex_coord");
    if (0 == teapot_program)
        WaitForEnterAndExit();

    // Get uniform locations
    teapot_model_matrix_loc = glGetUniformLocation(teapot_program, "model_matrix");
    teapot_PVM_matrix_loc = glGetUniformLocation(teapot_program, "PVM_matrix");
    teapot_normal_matrix_loc = glGetUniformLocation(teapot_program, "normal_matrix");

    teapot_material_ambient_color_loc = glGetUniformLocation(teapot_program, "material_ambient_color");
    teapot_material_diffuse_color_loc = glGetUniformLocation(teapot_program, "material_diffuse_color");
    teapot_material_specular_color_loc = glGetUniformLocation(teapot_program, "material_specular_color");
    teapot_material_shininess_loc = glGetUniformLocation(teapot_program, "material_shininess");

    teapot_light_position_loc = glGetUniformLocation(teapot_program, "light_position");
    teapot_light_ambient_color_loc = glGetUniformLocation(teapot_program, "light_ambient_color");
    teapot_light_diffuse_color_loc = glGetUniformLocation(teapot_program, "light_diffuse_color");
    teapot_light_specular_color_loc = glGetUniformLocation(teapot_program, "light_specular_color");

    teapot_eye_position_loc = glGetUniformLocation(teapot_program, "eye_position");

    teapot_my_color_tex_loc = glGetUniformLocation(teapot_program, "my_color_tex");
    teapot_my_alpha_tex_loc = glGetUniformLocation(teapot_program, "my_alpha_tex");

    // Create geometries
    my_cube = CreateCube(position_loc, normal_loc, tex_coord_loc);
    my_sphere = CreateSphere(position_loc, normal_loc, tex_coord_loc);
    my_teapot = CreateTeapot(position_loc, normal_loc, tex_coord_loc);

    // Lenna texture
    lenna_tex = CreateAndLoadTexture(MAYBEWIDE("Lenna.png"));
    glBindTexture(GL_TEXTURE_2D, lenna_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Alpha texture
    alpha_circle_tex = CreateAndLoadTexture(MAYBEWIDE("alpha_circle.png"));
    glBindTexture(GL_TEXTURE_2D, alpha_circle_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
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

	//----------------------------------------------------------
	// Render the teapot

	glUseProgram(teapot_program);

	glUniform3fv(teapot_eye_position_loc, 1, glm::value_ptr(my_camera.GetEyePosition()));

	glUniform4fv(teapot_light_position_loc, 1, glm::value_ptr(light_pos));
	glUniform3f(teapot_light_ambient_color_loc, 0.3f, 0.3f, 0.3f);
	glUniform3f(teapot_light_diffuse_color_loc, 1.0f, 1.0f, 1.0f);
	glUniform3f(teapot_light_specular_color_loc, 1.0f, 1.0f, 1.0f);

	glBindVertexArray(my_teapot.VAO);

	model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));

	glUniform1i(teapot_my_alpha_tex_loc, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, alpha_circle_tex);

	glUniform1i(teapot_my_color_tex_loc, 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, lenna_tex);

	glUniform3f(teapot_material_ambient_color_loc, 1.0f, 1.0f, 1.0f);
	glUniform3f(teapot_material_diffuse_color_loc, 1.0f, 1.0f, 1.0f);
	glUniform3f(teapot_material_specular_color_loc, 1.0f, 1.0f, 1.0f);
	glUniform1f(teapot_material_shininess_loc, 40.0f);

	PVM_matrix = projection_matrix * view_matrix * model_matrix;
	normal_matrix = glm::inverse(glm::transpose(glm::mat3(model_matrix)));
	glUniformMatrix4fv(teapot_model_matrix_loc, 1, GL_FALSE, glm::value_ptr(model_matrix));
	glUniformMatrix4fv(teapot_PVM_matrix_loc, 1, GL_FALSE, glm::value_ptr(PVM_matrix));
	glUniformMatrix3fv(teapot_normal_matrix_loc, 1, GL_FALSE, glm::value_ptr(normal_matrix));

	DrawGeometry(my_teapot);

    //----------------------------------------------------------
    // Render the cube

    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    

    glDepthMask(GL_FALSE);

    glUseProgram(cube_program);

    glBindVertexArray(my_cube.VAO);

    model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    glUniform1i(cube_my_alpha_tex_loc, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, alpha_circle_tex);

    glUniform1i(cube_my_color_tex_loc, 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, lenna_tex);

    PVM_matrix = projection_matrix * view_matrix * model_matrix;
    glUniformMatrix4fv(cube_PVM_matrix_loc, 1, GL_FALSE, glm::value_ptr(PVM_matrix));

    DrawGeometry(my_cube);

    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    DrawGeometry(my_cube);
    glCullFace(GL_BACK);
    DrawGeometry(my_cube);
    glDisable(GL_CULL_FACE);
    

    glDisable(GL_BLEND);

    glDepthMask(GL_TRUE);

    

    //----------------------------------------------------------
    // Reset the VAO and Program
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
    glutCreateWindow("PV112 - cv4 - blending");

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
