#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLProgram.hpp>
#include <glmlv/load_obj.hpp>

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
    
    GLint modelViewProject;
    GLint modelView;
    GLint normalMatrix;
    
    GLint directionalLightDir;
	GLint directionalLightIntensity;
	GLint pointLightPosition;
	GLint pointLightIntensity;

	GLint kd;
    GLint ks;
    GLint ka;
	
	glmlv::ObjData data;
	
	GLuint scene_vbo;
	GLuint scene_ibo;
	GLuint scene_vao;
	std::vector<GLuint> tex_objects;
	
	GLuint texObject = 0;
	GLuint texObject2 = 0;
	
    GLuint samplerObject = 0;

    GLint aSamplerLocation = -1;
    GLint dSamplerLocation = -1;
    GLint sSamplerLocation = -1;
    glmlv::GLProgram prog;
};
