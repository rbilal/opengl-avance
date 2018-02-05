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
    const int m_nWindowWidth = 1280;
    const int m_nWindowHeight = 720;
    glmlv::GLFWHandle m_GLFWHandle{ m_nWindowWidth, m_nWindowHeight, "forward renderer" }; // Note: the handle must be declared before the creation of any object managing OpenGL resource (e.g. GLProgram, GLShader)

    const glmlv::fs::path m_AppPath;
    const std::string m_AppName;
    const std::string m_ImGuiIniFilename;
    const glmlv::fs::path m_ShadersRootPath;
    const glmlv::fs::path m_AssetsRootPath;
    
    GLuint cube_vbo;
    GLuint cube_vao;
    GLuint cube_ibo;
    
    GLuint cube_indexBufferSize;
    
    GLuint sphere_vbo;
    GLuint sphere_vao;
    GLuint sphere_ibo;
    GLuint sphere_indexBufferSize;
    
    
    GLint modelViewProject;
    GLint modelView;
    GLint normalMatrix;
    
    GLint directionalLightDir;
	GLint directionalLightIntensity;
	GLint pointLightPosition;
	GLint pointLightIntensity;
	GLint kd;
	
	GLuint texObject = 0;
	GLuint texObject2 = 0;
	
    GLuint samplerObject = 0;

    GLint uSamplerLocation = -1;
    
    glmlv::GLProgram prog;
};
