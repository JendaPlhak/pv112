#pragma once
#ifndef INCLUDED_PV112_H
#define INCLUDED_PV112_H

#include <vector>
#include <string>

#define GLEW_STATIC
#include <GL/glew.h>

#include <glm/glm.hpp>
#include "object.hpp"


namespace PV112
{

//------------------------------------------
//----    APPLICATION INITIALIZATION    ----
//------------------------------------------

/// Uses the proper functions to set the debug callback method
void SetDebugCallback(GLDEBUGPROCARB callback);

//------------------------------------
//----    SHADERS AND PROGRAMS    ----
//------------------------------------

/// Loads a file and returns its content as std::string
std::string LoadFileToString(const char *file_name);

/// Waits for Enter and exits the application with exit(1)
void WaitForEnterAndExit();

/// Creates a shader of given type, loads and sets its source code, compiles it, and prints errors
/// if some happens.
///
/// Returns shader object on success or 0 if failed.
GLuint LoadAndCompileShader(GLenum shader_type, const char *file_name);

/// Creates a shader program, loads, compiles and sets the vertex and fragment shaders, links it,
/// and prints errors if some occur.
///
/// It also binds given input variables to given indices. Variables with -1 as 'idx' and nullptr
/// as 'name' are ignored.
///
/// Returns program object on success or 0 if failed.
GLuint CreateAndLinkProgram(const char *vertex_shader, const char *fragment_shader,
        GLint bind_attrib_0_idx, const char *bind_attrib_0_name,
        GLint bind_attrib_1_idx, const char *bind_attrib_1_name,
        GLint bind_attrib_2_idx, const char *bind_attrib_2_name);

/// Creates a shader program, loads, compiles and sets the vertex and fragment shaders, links it,
/// and prints errors if some happens.
///
/// Returns program object on success or 0 if failed.
GLuint CreateAndLinkProgram(const char *vertex_shader, const char *fragment_shader);

//-------------------------------------------
//----    SIMPLE PV112 GEOMETRY CLASS    ----
//-------------------------------------------

/// This is a VERY SIMPLE class to contain all buffers and vertex array objects for geometries of
/// PV112 lectures. It is not a perfect, brilliant, smart, or whatever implementation of a geometry.
///
/// Although this is a class, it has no private attributes, it has no methods etc. It behaves more
/// as a struct. This design was chosen because OpenGL is more C-like, so I wanted the class and all
/// functions that work with it to be more C-like too.
///
/// When drawing the geometry, bind its VAO and call the draw command. The whole geometry is always
/// drawn using a single draw call. Use glDrawArrays if DrawArraysCount > 0, or use glDrawElements
/// if DrawElementsCount > 0.
class PV112Geometry
{
public:
    PV112Geometry();
    PV112Geometry(const PV112Geometry &rhs);
    PV112Geometry &operator =(const PV112Geometry &rhs);

    // Note that there is no destructor, especially no destructor that would automatically destroy
    // the created objects. When calling functions like glDelete*, the OpenGL rendering context
    // must exist, however, it is destroyed when the main window is closed. For this reason, calling
    // a destructor afterwards (for example at the very end of the application when global objects
    // are destroyed) could lead to an access violation errors and a crash of the application.
    //
    // To make it short, all OpenGL objects must be destroyed BEFORE the main window is closed
    // (or left alive, as in our case).

    // Up to three buffers with the data of the geometry (positions, normals, texture coordinates).
    // If the data is only in one buffer, other buffers are 0
    GLuint VertexBuffers[3];

    // Buffer with the indices of the geometry
    GLuint IndexBuffer;

    // Vertex Array Object with the geometry
    GLuint VAO;

    // Type of the primitives to be drawn
    GLenum Mode;
    // Number of vertices to be drawn using glDrawArrays
    GLsizei DrawArraysCount;
    // Number of vertices to be drawn using glDrawElements
    GLsizei DrawElementsCount;
    AABB aabb;
};

/// Deletes OpenGL objects of the geometry.
void DeleteGeometry(PV112Geometry &geom);

/// Chooses glDrawArrays or glDrawElements to draw the geometry.
void DrawGeometry(const PV112Geometry &geom);

//-----------------------------
//----    BASIC OBJECTS    ----
//-----------------------------

/// Creates a simple cube object. The center of the cube is in (0,0,0) and the length of its side is 2
/// (positions of its vertices are from -1 to 1).
///
/// 'position_location', 'normal_location', and 'tex_coord_location' are locations of vertex attributes,
/// obtained by glGetAttribLocation. Use -1 if not necessary.
PV112Geometry CreateCube(GLint position_location, GLint normal_location = -1, GLint tex_coord_location = -1);

/// Creates a simple sphere object. The center of the sphere is in (0,0,0) and its radius is 1
/// (positions of its vertices are from -1 to 1).
///
/// 'position_location', 'normal_location', and 'tex_coord_location' are locations of vertex attributes,
/// obtained by glGetAttribLocation. Use -1 if not necessary.
PV112Geometry CreateSphere(GLint position_location, GLint normal_location = -1, GLint tex_coord_location = -1);

/// Creates a simple teapot object. The center of the bottom of its body is roughly in (0,0,0) and
/// the radius of the body is roughly 1. Its handle is in -X direction, its spout is in +X direction,
/// and its lid is in +Y direction.
///
/// 'position_location', 'normal_location', and 'tex_coord_location' are locations of vertex attributes,
/// obtained by glGetAttribLocation. Use -1 if not necessary.
PV112Geometry CreateTeapot(GLint position_location, GLint normal_location = -1, GLint tex_coord_location = -1);

//--------------------------
//----    OBJ LOADER    ----
//--------------------------

/// Parses an OBJ file. For OBJ file format, see https://en.wikipedia.org/wiki/Wavefront_.obj_file
///
/// This OBJ parser is very simple and serves only for PV112 lectures. It handles only geometries with
/// triangles, and each vertex must have its position, normal, and texture coordinate defined. When
/// parsing other OBJ files, write your own parser or download another one, you may use for example
/// Open Asset Import Library (Assimp).
///
/// When the file is correctly parsed, the function returns true and 'out_vertices', 'out_normals' and
/// 'out_tex_coords' contains the data of individual triangles (use glDrawArrays with GL_TRIANGLES).
/// If something goes wrong, error messsage is printed and this function returns false.
bool ParseOBJFile(const char *file_name, std::vector<glm::vec3> &out_vertices, std::vector<glm::vec3> &out_normals, std::vector<glm::vec2> &out_tex_coords);

/// Loads an OBJ file and creates a corresponding PV112Geometry object.
///
/// 'position_location', 'normal_location', and 'tex_coord_location' are locations of vertex attributes,
/// obtained by glGetAttribLocation. Use -1 if not necessary.
PV112Geometry LoadOBJ(const char *file_name, GLint position_location, GLint normal_location = -1, GLint tex_coord_location = -1);

//-----------------------------------------
//----    SIMPLE PV112 CAMERA CLASS    ----
//-----------------------------------------

/// This is a VERY SIMPLE class that allows to very simply move with the camera.
/// It is not a perfect, brilliant, smart, or whatever implementation of a camera,
/// but it is sufficient for PV112 lectures.
///
/// Use left mouse button to change the point of view.
/// Use right mouse button to zoom in and zoom out.
class PV112Camera
{
private:
    /// Constants that defines the behaviour of the camera
    ///		- Minimum elevation in radians
    static const float min_elevation;
    ///		- Maximum elevation in radians
    static const float max_elevation;
    ///		- Minimum distance from the point of interest
    static const float min_distance;
    ///		- Sensitivity of the mouse when changing elevation or direction angles
    static const float angle_sensitivity;
    ///		- Sensitivity of the mouse when changing zoom
    static const float zoom_sensitivity;

    /// angle_direction is an angle in which determines into which direction in xz plane I look.
    ///		- 0 degrees .. I look in -z direction
    ///		- 90 degrees .. I look in -x direction
    ///		- 180 degrees .. I look in +z direction
    ///		- 270 degrees .. I look in +x direction
    float angle_direction;

    /// angle_direction is an angle in which determines from which "height" I look.
    ///		- positive elevation .. I look from above the xz plane
    ///		- negative elevation .. I look from below the xz plane
    float angle_elevation;

    /// Distance from (0,0,0), the point at which I look
    float distance;

    /// Final position of the eye in world space coordinates, for LookAt or shaders
    glm::vec3 eye_position;

    /// Last X and Y coordinates of the mouse cursor
    int last_x, last_y;

    /// True or false if moused buttons are pressed and the user rotates/zooms the camera
    bool is_rotating, is_zooming;

    /// Recomputes 'eye_position' from 'angle_direction', 'angle_elevation', and 'distance'
    void update_eye_pos();

public:
    PV112Camera();

    /// Call when the user presses or releases a mouse button (see glutMouseFunc)
    void OnMouseButtonChanged(int button, int state, int x, int y);

    /// Call when the user moves with the mouse cursor (see glutMotionFunc)
    void OnMouseMoved(int x, int y);

    /// Returns the position of the eye in world space coordinates
    glm::vec3 GetEyePosition() const;
};

}

#endif	// INCLUDED_PV112_H
