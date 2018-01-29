#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLProgram.hpp>

class Application
{
public:
    Application(int argc, char** argv);

    int run();
private:
    const size_t m_nWindowWidth = 1280;
    const size_t m_nWindowHeight = 720;
    glmlv::GLFWHandle m_GLFWHandle{ m_nWindowWidth, m_nWindowHeight, "forward renderer" }; // Note: the handle must be declared before the creation of any object managing OpenGL resource (e.g. GLProgram, GLShader)

    const glmlv::fs::path m_AppPath;
    const std::string m_AppName;
    const std::string m_ImGuiIniFilename;
    const glmlv::fs::path m_ShadersRootPath;
    
    GLuint cube_vbo;
    GLuint cube_vao;
    GLuint cube_ibo;
    
    GLuint cube_indexBufferSize;
    const GLvoid * cube_indexBufferData;
    
    GLuint sphere_vbo;
    GLuint sphere_vao;
    GLuint sphere_ibo;
    GLuint sphere_indexBufferSize;
    const GLvoid * sphere_indexBufferData;
    
    
    GLuint modelViewProject;
    GLuint modelView;
    GLuint normalMatrix;
    
    glmlv::GLProgram prog;
};
