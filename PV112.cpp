#include "PV112.h"

#define GLEW_STATIC
#if defined(_WIN32)
#define NOMINMAX      // Make Windows.h not define 'min' and 'max' macros
#include <GL/wglew.h> // Include on Windows
#else
#include <GL/glxew.h> // Include on Linux and Mac
#endif

#include <GL/freeglut.h>

#include <memory>
#include <sstream>
#include <fstream>
#include <iostream>

#include "cube.inl"
#include "sphere.inl"
#include "teapot.inl"

using namespace std;

namespace PV112
{

//------------------------------------------
//----    APPLICATION INITIALIZATION    ----
//------------------------------------------

void SetDebugCallback(GLDEBUGPROCARB callback)
{
    // Setup callback that will inform us when we make an error.
    // glDebugMessageCallbackARB is sometimes missed by glew, due to a bug in it.

#if defined(_WIN32)
    // On Windows, use this:
    PFNGLDEBUGMESSAGECALLBACKARBPROC myglDebugMessageCallbackARB =
        (PFNGLDEBUGMESSAGECALLBACKARBPROC)wglGetProcAddress("glDebugMessageCallbackARB");
    if (myglDebugMessageCallbackARB)
    {
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        myglDebugMessageCallbackARB(callback, nullptr);
    }
#elif defined(__APPLE__)
    // On MacOS, use this (not tested):
    if (glDebugMessageCallbackARB)
    {
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallbackARB(callback, nullptr);
    }
#else
    // On Linux, use this:
    PFNGLDEBUGMESSAGECALLBACKARBPROC myglDebugMessageCallbackARB =
        (PFNGLDEBUGMESSAGECALLBACKARBPROC)glXGetProcAddress((unsigned char *)"glDebugMessageCallbackARB");
    if (myglDebugMessageCallbackARB)
    {
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        myglDebugMessageCallbackARB(callback, nullptr);
    }
#endif
}

//------------------------------------
//----    SHADERS AND PROGRAMS    ----
//------------------------------------

string LoadFileToString(const char *file_name)
{
    ifstream file(file_name);
    stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

void WaitForEnterAndExit()
{
    cout << "Press Enter to exit" << endl;
    getchar();
    exit(1);
}

GLuint LoadAndCompileShader(GLenum shader_type, const char *file_name)
{
    // Load the file from the disk
    string s_source = LoadFileToString(file_name);
    if (s_source.empty())
    {
        cout << "File " << file_name << " is empty or failed to load" << endl;
        return 0;
    }

    // Create shader object and set the source
    GLuint shader = glCreateShader(shader_type);
    const char *source = s_source.c_str();
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    // Compile and get errors
    int compile_status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
    if (GL_FALSE == compile_status)
    {
        switch (shader_type)
        {
            case GL_VERTEX_SHADER:            cout << "Failed to compile vertex shader " << file_name << endl;                    break;
            case GL_FRAGMENT_SHADER:        cout << "Failed to compile fragment shader " << file_name << endl;                    break;
            case GL_GEOMETRY_SHADER:        cout << "Failed to compile geometry shader " << file_name << endl;                    break;
            case GL_TESS_CONTROL_SHADER:    cout << "Failed to compile tessellation control shader " << file_name << endl;        break;
            case GL_TESS_EVALUATION_SHADER:    cout << "Failed to compile tessellation evaluation shader " << file_name << endl;    break;
            default:                        cout << "Failed to compile shader " << file_name << endl;                            break;
        }

        int log_len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
        unique_ptr<char []> log(new char[log_len]);
        glGetShaderInfoLog(shader, log_len, nullptr, log.get());
        cout << log.get() << endl;

        glDeleteShader(shader);
        return 0;
    }
    else return shader;
}

GLuint CreateAndLinkProgram(const char *vertex_shader, const char *fragment_shader,
        GLint bind_attrib_0_idx, const char *bind_attrib_0_name,
        GLint bind_attrib_1_idx, const char *bind_attrib_1_name,
        GLint bind_attrib_2_idx, const char *bind_attrib_2_name)
{
    // Load the vertex shader
    GLuint vs_shader = LoadAndCompileShader(GL_VERTEX_SHADER, vertex_shader);
    if (0 == vs_shader)
    {
        return 0;
    }

    // Load the fragment shader
    GLuint fs_shader = LoadAndCompileShader(GL_FRAGMENT_SHADER, fragment_shader);
    if (0 == fs_shader)
    {
        glDeleteShader(vs_shader);
        return 0;
    }

    // Create program and attach shaders
    GLuint program = glCreateProgram();
    glAttachShader(program, vs_shader);
    glAttachShader(program, fs_shader);

    // Bind attributes
    if (bind_attrib_0_idx != -1)
        glBindAttribLocation(program, bind_attrib_0_idx, bind_attrib_0_name);
    if (bind_attrib_1_idx != -1)
        glBindAttribLocation(program, bind_attrib_1_idx, bind_attrib_1_name);
    if (bind_attrib_2_idx != -1)
        glBindAttribLocation(program, bind_attrib_2_idx, bind_attrib_2_name);

    // Link program
    glLinkProgram(program);

    // Link and get errors
    int link_status;
    glGetProgramiv(program, GL_LINK_STATUS, &link_status);
    if (GL_FALSE == link_status)
    {
        cout << "Failed to link program with vertex shader " << vertex_shader << " and fragment shader " << fragment_shader << endl;

        int log_len = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len);
        unique_ptr<char []> log(new char[log_len]);
        glGetProgramInfoLog(program, log_len, nullptr, log.get());
        cout << log.get() << endl;

        glDeleteShader(vs_shader);
        glDeleteShader(fs_shader);
        glDeleteProgram(program);
        return 0;
    }
    else return program;
}

GLuint CreateAndLinkProgram(const char *vertex_shader, const char *fragment_shader)
{
    return CreateAndLinkProgram(vertex_shader, fragment_shader,
            -1, nullptr, -1, nullptr, -1, nullptr);
}

//-------------------------------------------
//----    SIMPLE PV112 GEOMETRY CLASS    ----
//-------------------------------------------

PV112Geometry::PV112Geometry()
{
    VertexBuffers[0] = 0;
    VertexBuffers[1] = 0;
    VertexBuffers[2] = 0;
    IndexBuffer = 0;
    VAO = 0;
    Mode = GL_POINTS;
    DrawArraysCount = 0;
    DrawElementsCount = 0;
}

PV112Geometry::PV112Geometry(const PV112Geometry &rhs)
{
    *this = rhs;
}

PV112Geometry &PV112Geometry::operator =(const PV112Geometry &rhs)
{
    VertexBuffers[0] = rhs.VertexBuffers[0];
    VertexBuffers[1] = rhs.VertexBuffers[1];
    VertexBuffers[2] = rhs.VertexBuffers[2];
    IndexBuffer = rhs.IndexBuffer;
    VAO = rhs.VAO;
    Mode = rhs.Mode;
    DrawArraysCount = rhs.DrawArraysCount;
    DrawElementsCount = rhs.DrawElementsCount;
    return *this;
}

void DeleteGeometry(PV112Geometry &geom)
{
    // This is mostly an example of what should be destroyed and how. We won't be using it anywhere.
    //
    // When using it, make sure the OpenGL context still exists (i.e. the main window still exists).

    // OpenGL silently ignores deleting objects that are 0, so this is safe even if the buffers were not created.
    glDeleteBuffers(3, geom.VertexBuffers);
    glDeleteBuffers(1, &geom.IndexBuffer);
    glDeleteVertexArrays(1, &geom.VAO);

    geom = PV112Geometry();        // Reset the state to 'no geometry'
}

void DrawGeometry(const PV112Geometry &geom)
{
    if (geom.DrawArraysCount > 0)
        glDrawArrays(geom.Mode, 0, geom.DrawArraysCount);
    if (geom.DrawElementsCount > 0)
        glDrawElements(geom.Mode, geom.DrawElementsCount, GL_UNSIGNED_INT, nullptr);
}

//-----------------------------
//----    BASIC OBJECTS    ----
//-----------------------------

PV112Geometry CreateCube(GLint position_location, GLint normal_location, GLint tex_coord_location)
{
    PV112Geometry geometry;

    // Create a single buffer for vertex data
    glGenBuffers(1, &geometry.VertexBuffers[0]);
    glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, cube_vertices_count * sizeof(float) * 8, cube_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    geometry.VertexBuffers[1] = 0;
    geometry.VertexBuffers[2] = 0;

    // Create a buffer for indices
    glGenBuffers(1, &geometry.IndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.IndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cube_indices_count * sizeof(unsigned int), cube_indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Create a vertex array object for the geometry
    glGenVertexArrays(1, &geometry.VAO);

    // Set the parameters of the geometry
    glBindVertexArray(geometry.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[0]);
    if (position_location >= 0)
    {
        glEnableVertexAttribArray(position_location);
        glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, 0);
    }
    if (normal_location >= 0)
    {
        glEnableVertexAttribArray(normal_location);
        glVertexAttribPointer(normal_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (const void *)(sizeof(float) * 3));
    }
    if (tex_coord_location >= 0)
    {
        glEnableVertexAttribArray(tex_coord_location);
        glVertexAttribPointer(tex_coord_location, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (const void *)(sizeof(float) * 6));
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.IndexBuffer);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    geometry.Mode = GL_TRIANGLES;
    geometry.DrawArraysCount = 0;
    geometry.DrawElementsCount = cube_indices_count;

    return geometry;
}

PV112Geometry CreateSphere(GLint position_location, GLint normal_location, GLint tex_coord_location)
{
    PV112Geometry geometry;

    // Create a single buffer for vertex data
    glGenBuffers(1, &geometry.VertexBuffers[0]);
    glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sphere_vertices_count * sizeof(float) * 8, sphere_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    geometry.VertexBuffers[1] = 0;
    geometry.VertexBuffers[2] = 0;

    // Create a buffer for indices
    glGenBuffers(1, &geometry.IndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.IndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere_indices_count * sizeof(unsigned int), sphere_indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Create a vertex array object for the geometry
    glGenVertexArrays(1, &geometry.VAO);

    // Set the parameters of the geometry
    glBindVertexArray(geometry.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[0]);
    if (position_location >= 0)
    {
        glEnableVertexAttribArray(position_location);
        glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, 0);
    }
    if (normal_location >= 0)
    {
        glEnableVertexAttribArray(normal_location);
        glVertexAttribPointer(normal_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (const void *)(sizeof(float) * 3));
    }
    if (tex_coord_location >= 0)
    {
        glEnableVertexAttribArray(tex_coord_location);
        glVertexAttribPointer(tex_coord_location, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (const void *)(sizeof(float) * 6));
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.IndexBuffer);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    geometry.Mode = GL_TRIANGLE_STRIP;
    geometry.DrawArraysCount = 0;
    geometry.DrawElementsCount = sphere_indices_count;

    return geometry;
}

PV112Geometry CreateTeapot(GLint position_location, GLint normal_location, GLint tex_coord_location)
{
    PV112Geometry geometry;

    // Create a single buffer for vertex data
    glGenBuffers(1, &geometry.VertexBuffers[0]);
    glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, teapot_vertices_count * sizeof(float) * 8, teapot_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    geometry.VertexBuffers[1] = 0;
    geometry.VertexBuffers[2] = 0;

    // Create a buffer for indices
    glGenBuffers(1, &geometry.IndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.IndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, teapot_indices_count * sizeof(unsigned int), teapot_indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Create a vertex array object for the geometry
    glGenVertexArrays(1, &geometry.VAO);

    // Set the parameters of the geometry
    glBindVertexArray(geometry.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[0]);
    if (position_location >= 0)
    {
        glEnableVertexAttribArray(position_location);
        glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, 0);
    }
    if (normal_location >= 0)
    {
        glEnableVertexAttribArray(normal_location);
        glVertexAttribPointer(normal_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (const void *)(sizeof(float) * 3));
    }
    if (tex_coord_location >= 0)
    {
        glEnableVertexAttribArray(tex_coord_location);
        glVertexAttribPointer(tex_coord_location, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (const void *)(sizeof(float) * 6));
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.IndexBuffer);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    geometry.Mode = GL_TRIANGLE_STRIP;
    geometry.DrawArraysCount = 0;
    geometry.DrawElementsCount = teapot_indices_count;

    return geometry;
}

//--------------------------
//----    OBJ LOADER    ----
//--------------------------

bool ParseOBJFile(const char *file_name, std::vector<glm::vec3> &out_vertices, std::vector<glm::vec3> &out_normals, std::vector<glm::vec2> &out_tex_coords)
{
    // I love lambda functions :-)
    auto error_msg = [file_name] {
        cout << "Failed to read OBJ file " << file_name << ", its format is not supported" << endl;
    };

    struct OBJTriangle
    {
        int v0, v1, v2;
        int n0, n1, n2;
        int t0, t1, t2;
    };

    // Prepare the arrays for the data from the file.
    std::vector<glm::vec3> raw_vertices;        raw_vertices.reserve(1000);
    std::vector<glm::vec3> raw_normals;            raw_normals.reserve(1000);
    std::vector<glm::vec2> raw_tex_coords;        raw_tex_coords.reserve(1000);
    std::vector<OBJTriangle> raw_triangles;        raw_triangles.reserve(1000);

    // Load OBJ file
    ifstream file(file_name);
    if (!file.is_open())
    {
        cout << "Cannot open OBJ file " << file_name << endl;
        return false;
    }

    while (!file.fail())
    {
        string prefix;
        file >> prefix;

        if (prefix == "v")
        {
            glm::vec3 v;
            file >> v.x >> v.y >> v.z;
            raw_vertices.push_back(v);
            file.ignore(numeric_limits<streamsize>::max(), '\n');        // Ignore the rest of the line
        }
        else if (prefix == "vt")
        {
            glm::vec2 vt;
            file >> vt.x >> vt.y;
            raw_tex_coords.push_back(vt);
            file.ignore(numeric_limits<streamsize>::max(), '\n');        // Ignore the rest of the line
        }
        else if (prefix == "vn")
        {
            glm::vec3 vn;
            file >> vn.x >> vn.y >> vn.z;
            raw_normals.push_back(vn);
            file.ignore(numeric_limits<streamsize>::max(), '\n');        // Ignore the rest of the line
        }
        else if (prefix == "f")
        {
            OBJTriangle t;
            char slash;

            // And now check whether the geometry is of a correct format (that it contains only triangles,
            // and all vertices have their position, normal, and texture coordinate set).

            // Read the first vertex
            file >> ws;        if (!isdigit(file.peek()))    {    error_msg();        return false;    }
            file >> t.v0;
            file >> ws;        if (file.peek() != '/')        {    error_msg();        return false;    }
            file >> slash;
            file >> ws;        if (!isdigit(file.peek()))    {    error_msg();        return false;    }
            file >> t.t0;
            file >> ws;        if (file.peek() != '/')        {    error_msg();        return false;    }
            file >> slash;
            file >> ws;        if (!isdigit(file.peek()))    {    error_msg();        return false;    }
            file >> t.n0;

            // Read the second vertex
            file >> ws;        if (!isdigit(file.peek()))    {    error_msg();        return false;    }
            file >> t.v1;
            file >> ws;        if (file.peek() != '/')        {    error_msg();        return false;    }
            file >> slash;
            file >> ws;        if (!isdigit(file.peek()))    {    error_msg();        return false;    }
            file >> t.t1;
            file >> ws;        if (file.peek() != '/')        {    error_msg();        return false;    }
            file >> slash;
            file >> ws;        if (!isdigit(file.peek()))    {    error_msg();        return false;    }
            file >> t.n1;

            // Read the third vertex
            file >> ws;        if (!isdigit(file.peek()))    {    error_msg();        return false;    }
            file >> t.v2;
            file >> ws;        if (file.peek() != '/')        {    error_msg();        return false;    }
            file >> slash;
            file >> ws;        if (!isdigit(file.peek()))    {    error_msg();        return false;    }
            file >> t.t2;
            file >> ws;        if (file.peek() != '/')        {    error_msg();        return false;    }
            file >> slash;
            file >> ws;        if (!isdigit(file.peek()))    {    error_msg();        return false;    }
            file >> t.n2;

            // Check that this polygon has only three vertices (we support triangles only).
            // It also skips all white spaces, effectively ignoring the rest of the line (if empty).
            file >> ws;        if (isdigit(file.peek()))    {    error_msg();        return false;    }

            // Subtract one, OBJ indexes from 1, not from 0
            t.v0--;        t.v1--;        t.v2--;
            t.n0--;        t.n1--;        t.n2--;
            t.t0--;        t.t1--;        t.t2--;

            raw_triangles.push_back(t);
        }
        else
        {
            // Ignore other cases
            file.ignore(numeric_limits<streamsize>::max(), '\n');        // Ignore the rest of the line
        }
    }
    file.close();

    // Indices in OBJ file cannot be used, we need to convert the geometry in a way we could draw it
    // with glDrawArrays.
    out_vertices.clear();        out_vertices.reserve(raw_triangles.size() * 3);
    out_normals.clear();        out_normals.reserve(raw_triangles.size() * 3);
    out_tex_coords.clear();        out_tex_coords.reserve(raw_triangles.size() * 3);
    for (size_t i = 0; i < raw_triangles.size(); i++)
    {
        if ((raw_triangles[i].v0 >= int(raw_vertices.size())) ||
                (raw_triangles[i].v1 >= int(raw_vertices.size())) ||
                (raw_triangles[i].v2 >= int(raw_vertices.size())) ||
                (raw_triangles[i].n0 >= int(raw_normals.size())) ||
                (raw_triangles[i].n1 >= int(raw_normals.size())) ||
                (raw_triangles[i].n2 >= int(raw_normals.size())) ||
                (raw_triangles[i].t0 >= int(raw_tex_coords.size())) ||
                (raw_triangles[i].t1 >= int(raw_tex_coords.size())) ||
                (raw_triangles[i].t2 >= int(raw_tex_coords.size())))
        {
            // Invalid out-of-range indices
            error_msg();
            return false;
        }

        out_vertices.push_back(raw_vertices[raw_triangles[i].v0]);
        out_vertices.push_back(raw_vertices[raw_triangles[i].v1]);
        out_vertices.push_back(raw_vertices[raw_triangles[i].v2]);
        out_normals.push_back(raw_normals[raw_triangles[i].n0]);
        out_normals.push_back(raw_normals[raw_triangles[i].n1]);
        out_normals.push_back(raw_normals[raw_triangles[i].n2]);
        out_tex_coords.push_back(raw_tex_coords[raw_triangles[i].t0]);
        out_tex_coords.push_back(raw_tex_coords[raw_triangles[i].t1]);
        out_tex_coords.push_back(raw_tex_coords[raw_triangles[i].t2]);
    }

    return true;
}

PV112Geometry LoadOBJ(const char *file_name, GLint position_location, GLint normal_location, GLint tex_coord_location)
{
    PV112Geometry geometry;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> tex_coords;
    if (!ParseOBJFile(file_name, vertices, normals, tex_coords))
    {
        return geometry;        // Return empty geometry, the error message was already printed
    }

    // Create buffers for vertex data
    glGenBuffers(3, geometry.VertexBuffers);
    glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) * 3, vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[1]);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float) * 3, normals.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[2]);
    glBufferData(GL_ARRAY_BUFFER, tex_coords.size() * sizeof(float) * 2, tex_coords.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // No indices
    geometry.IndexBuffer = 0;

    // Create a vertex array object for the geometry
    glGenVertexArrays(1, &geometry.VAO);

    // Set the parameters of the geometry
    glBindVertexArray(geometry.VAO);
    if (position_location >= 0)
    {
        glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[0]);
        glEnableVertexAttribArray(position_location);
        glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }
    if (normal_location >= 0)
    {
        glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[1]);
        glEnableVertexAttribArray(normal_location);
        glVertexAttribPointer(normal_location, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }
    if (tex_coord_location >= 0)
    {
        glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[2]);
        glEnableVertexAttribArray(tex_coord_location);
        glVertexAttribPointer(tex_coord_location, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    geometry.Mode = GL_TRIANGLES;
    geometry.DrawArraysCount = vertices.size();
    geometry.DrawElementsCount = 0;

    return geometry;
}

//-----------------------------------------
//----    SIMPLE PV112 CAMERA CLASS    ----
//-----------------------------------------

const float PV112Camera::min_elevation = -1.5f;
const float PV112Camera::max_elevation = 1.5f;
const float PV112Camera::min_distance = 1.0f;
const float PV112Camera::angle_sensitivity = 0.008f;
const float PV112Camera::zoom_sensitivity = 0.003f;

PV112Camera::PV112Camera()
    : angle_direction(0.0f), angle_elevation(0.0f), distance(5.0f), last_x(0), last_y(0), is_rotating(false), is_zooming(false)
{
    update_eye_pos();
}

void PV112Camera::update_eye_pos()
{
    eye_position.x = distance * cosf(angle_elevation) * -sinf(angle_direction);
    eye_position.y = distance * sinf(angle_elevation);
    eye_position.z = distance * cosf(angle_elevation) * cosf(angle_direction);
}

void PV112Camera::OnMouseButtonChanged(int button, int state, int x, int y)
{
    // Left mouse button affects the angles
    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            last_x = x;
            last_y = y;
            is_rotating = true;
        }
        else is_rotating = false;
    }
    // Right mouse button affects the zoom
    if (button == GLUT_RIGHT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            last_x = x;
            last_y = y;
            is_zooming = true;
        }
        else is_zooming = false;
    }
}

void PV112Camera::OnMouseMoved(int x, int y)
{
    float dx = float(x - last_x);
    float dy = float(y - last_y);
    last_x = x;
    last_y = y;

    if (is_rotating)
    {
        angle_direction += dx * angle_sensitivity;
        angle_elevation += dy * angle_sensitivity;

        // Clamp the results
        if (angle_elevation > max_elevation)    angle_elevation = max_elevation;
        if (angle_elevation < min_elevation)    angle_elevation = min_elevation;
    }
    if (is_zooming)
    {
        distance *= (1.0f + dy * zoom_sensitivity);

        // Clamp the results
        if (distance < min_distance)        distance = min_distance;
    }
    update_eye_pos();
}

glm::vec3 PV112Camera::GetEyePosition() const
{
    return eye_position;
}

}
