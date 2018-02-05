#include "Application.hpp"

#include <iostream>

#include <imgui.h>
#include <glmlv/imgui_impl_glfw_gl3.hpp>
#include <glmlv/simple_geometry.hpp>
#include <glmlv/ViewController.hpp>
#include <glmlv/Image2DRGBA.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


using namespace glmlv;
using namespace std;

int Application::run()
{
	glm::mat4 ProjMatrix = glm::perspective(glm::radians(70.f), float(m_nWindowWidth)/m_nWindowHeight, 0.1f, 100.f);
	ViewController viewController(m_GLFWHandle.window());
	
    float clearColor[3] = { 0, 0, 0 };
    float udk[3] = { 1, 1, 1};
    
    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
        const auto seconds = glfwGetTime();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 viewMatrix = viewController.getViewMatrix();
        // Put here rendering code
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(uSamplerLocation, 0);
        glBindSampler(0, samplerObject);
        
        glBindTexture(GL_TEXTURE_2D, texObject);     
        
        glBindVertexArray(cube_vao);
        
        glUniform3f(directionalLightDir, 1.0, 1.0, 1.0);
        glUniform3f(directionalLightIntensity,1.0, 1.0, 1.0);
        glUniform3f(pointLightPosition,1.0, 1.0, 1.0);
        glUniform3f(pointLightIntensity,1.0, 1.0, 1.0);
        glUniform3f(kd,udk[0], udk[1], udk[2]);
        
		glm::mat4 MVMatrix = glm::translate(viewMatrix, glm::vec3(5, 0, -5));
		glm::mat4 NormalMatrix = glm::transpose(glm::inverse(MVMatrix));
        
        glUniformMatrix4fv(modelViewProject,1,GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
        glUniformMatrix4fv(modelView,1,GL_FALSE, glm::value_ptr(MVMatrix));
        glUniformMatrix4fv(normalMatrix,1,GL_FALSE, glm::value_ptr(NormalMatrix));
        
        glDrawElements(GL_TRIANGLES, cube_indexBufferSize, GL_UNSIGNED_INT, nullptr);
        
        glBindTexture(GL_TEXTURE_2D, texObject2);     

        glBindVertexArray(0);

		glBindVertexArray(sphere_vao);
		
		glm::vec3 vv = glm::normalize(glm::vec3(viewMatrix * glm::vec4(1.0, 1.0, 1.0, 0.0)));
		
        glUniform3fv(directionalLightDir, 1, glm::value_ptr(vv));
        glUniform3f(directionalLightIntensity,1.0, 1.0, 1.0);
        glUniform3fv(pointLightPosition, 1, value_ptr(glm::vec3(viewMatrix * glm::vec4(1.0, 1.0, 1.0, 1.0))));
        glUniform3f(pointLightIntensity,1.0, 1.0, 1.0);
        glUniform3f(kd,udk[0], udk[1], udk[2]);
        		
		glm::mat4 SMVMatrix = glm::translate(viewMatrix, glm::vec3(0, 0, -5));
		glm::mat4 SNormalMatrix = glm::transpose(glm::inverse(SMVMatrix));
		
        glUniformMatrix4fv(modelViewProject,1,GL_FALSE, glm::value_ptr(ProjMatrix * SMVMatrix));
        glUniformMatrix4fv(modelView,1,GL_FALSE, glm::value_ptr(SMVMatrix));
        glUniformMatrix4fv(normalMatrix,1,GL_FALSE, glm::value_ptr(SNormalMatrix));
        
        glDrawElements(GL_TRIANGLES, sphere_indexBufferSize, GL_UNSIGNED_INT, nullptr);
        
        glBindVertexArray(0);
        //
        //
        //

        // GUI code:
        ImGui_ImplGlfwGL3_NewFrame();

        {
            ImGui::Begin("GUI");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::ColorEditMode(ImGuiColorEditMode_RGB);
            if (ImGui::ColorEdit3("clearColor", clearColor)) {
                glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.f);
            }
            ImGui::ColorEdit3("Color", udk);

          
            
            ImGui::End();
        }

        const auto viewportSize = m_GLFWHandle.framebufferSize();
        glViewport(0, 0, viewportSize.x, viewportSize.y);
        ImGui::Render();

        /* Poll for and process events */
        glfwPollEvents();

        /* Swap front and back buffers*/
        m_GLFWHandle.swapBuffers();

        auto ellapsedTime = glfwGetTime() - seconds;
        auto guiHasFocus = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
        if (!guiHasFocus) {
            viewController.update(float(ellapsedTime));
        }
    }

    return 0;
}

Application::Application(int argc, char** argv):
    m_AppPath { glmlv::fs::path{ argv[0] } },
    m_AppName { m_AppPath.stem().string() },
    m_ImGuiIniFilename { m_AppName + ".imgui.ini" },
    m_ShadersRootPath { m_AppPath.parent_path() / "shaders" },
    m_AssetsRootPath {m_AppPath.parent_path() / "assets"}

{
	

    ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file
    
    prog = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "forward.vs.glsl", m_ShadersRootPath / m_AppName / "forward.fs.glsl" });
    
    //Récupération des locations des variables uniformes
	modelViewProject = glGetUniformLocation(prog.glId(), "uModelViewProjMatrix");
	modelView = glGetUniformLocation(prog.glId(), "uModelViewMatrix");
	normalMatrix = glGetUniformLocation(prog.glId(), "uNormalMatrix");
	
	directionalLightDir = glGetUniformLocation(prog.glId(), "uDirectionalLightDir");
	directionalLightIntensity = glGetUniformLocation(prog.glId(), "uDirectionalLightIntensity");
	pointLightPosition = glGetUniformLocation(prog.glId(), "uPointLightPosition");
	pointLightIntensity = glGetUniformLocation(prog.glId(), "uPointLightIntensity");
	kd = glGetUniformLocation(prog.glId(), "uKd");
	
	prog.use();
	
//	############################################ CUBE INITIALISATION #########################################################
    
    glGenBuffers(1, &cube_vbo);
    
    SimpleGeometry cube_vertices = makeCube();
    cube_indexBufferSize = cube_vertices.indexBuffer.size();
    
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);

    glBufferStorage(GL_ARRAY_BUFFER, cube_indexBufferSize * sizeof(cube_vertices.vertexBuffer[0]), cube_vertices.vertexBuffer.data(), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glGenBuffers(1, &cube_ibo);

    glBindBuffer(GL_ARRAY_BUFFER, cube_ibo);
    
    glBufferStorage(GL_ARRAY_BUFFER, cube_indexBufferSize * sizeof(cube_vertices.indexBuffer[0]), cube_vertices.indexBuffer.data(), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    
    glGenVertexArrays(1, &cube_vao);
	const GLint positionAttrLocation = 0;
    const GLint normalAttrLocation = 1;
    const GLint texCoordsAttrLocation = 2;
 
    glBindVertexArray(cube_vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
    
    glEnableVertexAttribArray(positionAttrLocation);
    glVertexAttribPointer(positionAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3f3f2f), (const GLvoid*) offsetof(Vertex3f3f2f, position));
    
    glEnableVertexAttribArray(normalAttrLocation);
    glVertexAttribPointer(normalAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3f3f2f), (const GLvoid*) offsetof(Vertex3f3f2f, normal));
    
    glEnableVertexAttribArray(texCoordsAttrLocation);
    glVertexAttribPointer(texCoordsAttrLocation, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3f3f2f), (const GLvoid*) offsetof(Vertex3f3f2f, texCoords));


    glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_ibo);

    glBindVertexArray(0);
    
    Image2DRGBA img = glmlv::readImage("/home/2inl2/rbilal/Synthese/GLImac-Template/assets/textures/triforce.png");
	glActiveTexture(GL_TEXTURE0);

	glGenTextures(1, &texObject);
	glBindTexture(GL_TEXTURE_2D, texObject);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, img.width(), img.height());
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img.width(), img.height(), GL_RGBA, GL_UNSIGNED_BYTE, img.data());
	glBindTexture(GL_TEXTURE_2D, 0);
	
	Image2DRGBA img2 = glmlv::readImage(m_AssetsRootPath / m_AppName / "textures" / "opengl-logo.png");

	glGenTextures(1, &texObject2);
	glBindTexture(GL_TEXTURE_2D, texObject2);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, img2.width(), img2.height());
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img2.width(), img2.height(), GL_RGBA, GL_UNSIGNED_BYTE, img2.data());
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenSamplers(1, &samplerObject);
    glSamplerParameteri(samplerObject, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(samplerObject, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    uSamplerLocation = glGetUniformLocation(prog.glId(), "uKdSampler");

//	############################################ SPHERE INITIALISATION #########################################################
    
	glGenBuffers(1, &sphere_vbo);
    
    SimpleGeometry sphere_vertices = makeSphere(10);
    sphere_indexBufferSize = sphere_vertices.indexBuffer.size();
    
    glBindBuffer(GL_ARRAY_BUFFER, sphere_vbo);

    glBufferStorage(GL_ARRAY_BUFFER, sphere_vertices.vertexBuffer.size() * sizeof(sphere_vertices.vertexBuffer[0]), sphere_vertices.vertexBuffer.data(), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glGenBuffers(1, &sphere_ibo);

    glBindBuffer(GL_ARRAY_BUFFER, sphere_ibo);
    
    glBufferStorage(GL_ARRAY_BUFFER, sphere_indexBufferSize * sizeof(sphere_vertices.indexBuffer[0]), sphere_vertices.indexBuffer.data(), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    
    glGenVertexArrays(1, &sphere_vao);
    
    glBindVertexArray(sphere_vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, sphere_vbo);
    
    glEnableVertexAttribArray(positionAttrLocation);
    glVertexAttribPointer(positionAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3f3f2f), (const GLvoid*) offsetof(Vertex3f3f2f, position));
    
    glEnableVertexAttribArray(normalAttrLocation);
    glVertexAttribPointer(normalAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3f3f2f), (const GLvoid*) offsetof(Vertex3f3f2f, normal));
    
    glEnableVertexAttribArray(texCoordsAttrLocation);
    glVertexAttribPointer(texCoordsAttrLocation, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3f3f2f), (const GLvoid*) offsetof(Vertex3f3f2f, texCoords));


    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere_ibo);

    glBindVertexArray(0);
	
    glEnable(GL_DEPTH_TEST);
    
}
