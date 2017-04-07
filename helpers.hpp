#pragma once

#include <iostream>

namespace PV112 {

// Loads a texture from file and calls glTexImage2D to se its data.
// Returns true on success or false on failure.
// NOTE 1a) Describe
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
        std::cerr << "Couldn't load texture: " << filename << std::endl;
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
        std::cerr << "Texture " << filename << " has unsupported format\n";
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
    if (!LoadAndSetTexture(filename_px, GL_TEXTURE_CUBE_MAP_POSITIVE_X) ||
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

}
