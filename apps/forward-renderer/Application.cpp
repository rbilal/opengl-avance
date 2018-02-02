#include "Application.hpp"

#include <iostream>

#include <imgui.h>
#include <glmlv/imgui_impl_glfw_gl3.hpp>
#include <glmlv/simple_geometry.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glmlv;
using namespace std;

int Application::run()
{
	glm::mat4 ProjMatrix = glm::perspective(glm::radians(70.f), float(m_nWindowWidth)/m_nWindowHeight, 0.1f, 100.f);
	glm::mat4 MVMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -5));
	glm::mat4 NormalMatrix = glm::transpose(glm::inverse(MVMatrix));
    float clearColor[3] = { 0, 0, 0 };
    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
        const auto seconds = glfwGetTime();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Put here rendering code
        
        glBindVertexArray(cube_vao);
        
        glUniformMatrix4fv(modelViewProject,1,GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
        glUniformMatrix4fv(modelView,1,GL_FALSE, glm::value_ptr(MVMatrix));
        glUniformMatrix4fv(normalMatrix,1,GL_FALSE, glm::value_ptr(NormalMatrix));
        
        glDrawElements(GL_TRIANGLES, cube_indexBufferSize, GL_UNSIGNED_INT, nullptr);
        
        glBindVertexArray(0);
/*
		glBindVertexArray(sphere_vao);
        
        glUniformMatrix4fv(modelViewProject,1,GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
        glUniformMatrix4fv(modelView,1,GL_FALSE, glm::value_ptr(MVMatrix));
        glUniformMatrix4fv(normalMatrix,1,GL_FALSE, glm::value_ptr(NormalMatrix));
        
        glDrawElements(GL_TRIANGLES, sphere_indexBufferSize, GL_UNSIGNED_INT, nullptr);
        
        glBindVertexArray(0);*/
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
            //viewController.update(float(ellapsedTime))
        }
    }

    return 0;
}

Application::Application(int argc, char** argv):
    m_AppPath { glmlv::fs::path{ argv[0] } },
    m_AppName { m_AppPath.stem().string() },
    m_ImGuiIniFilename { m_AppName + ".imgui.ini" },
    m_ShadersRootPath { m_AppPath.parent_path() / "shaders" }

{
    ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file
    
    prog = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "forward.vs.glsl", m_ShadersRootPath / m_AppName / "forward.fs.glsl" });
    
    //Récupération des locations des variables uniformes
	modelViewProject = glGetUniformLocation(prog.glId(), "uModelViewProjMatrix");
	modelView = glGetUniformLocation(prog.glId(), "uModelViewMatrix");
	normalMatrix = glGetUniformLocation(prog.glId(), "uNormalMatrix");
	
	prog.use();
	
//	############################################ CUBE INITIALISATION #########################################################
    
    glGenBuffers(1, &cube_vbo);
    
    SimpleGeometry cube_vertices = makeCube();
    cube_indexBufferSize = cube_vertices.indexBuffer.size();
    //cube_indexBufferData = cube_vertices.indexBuffer.data();
    
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);

    glBufferStorage(GL_ARRAY_BUFFER, cube_vertices.vertexBuffer.size() * sizeof(cube_vertices.vertexBuffer[0]), cube_vertices.vertexBuffer.data(), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glGenBuffers(1, &cube_ibo);

    glBindBuffer(GL_ARRAY_BUFFER, cube_ibo);
    
    glBufferStorage(GL_ARRAY_BUFFER, cube_vertices.indexBuffer.size() * sizeof(cube_vertices.indexBuffer[0]), cube_vertices.indexBuffer.data(), 0);

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
    
//	############################################ SPHERE INITIALISATION #########################################################
    /*
    glGenBuffers(1, &sphere_vbo);
    
    SimpleGeometry sphere_vertices = makeCube();
    sphere_indexBufferSize = sphere_vertices.indexBuffer.size();
    sphere_indexBufferData = sphere_vertices.indexBuffer.data();
    
    glBindBuffer(GL_ARRAY_BUFFER, sphere_vbo);

    glBufferStorage(GL_ARRAY_BUFFER, sphere_vertices.vertexBuffer.size() * sizeof(vector<Vertex3f3f2f>), sphere_indexBufferData, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glGenBuffers(1, &sphere_ibo);

    glBindBuffer(GL_ARRAY_BUFFER, sphere_ibo);
    
    glBufferStorage(GL_ARRAY_BUFFER, sizeof(sphere_vertices.indexBuffer), sphere_indexBufferData, 0);

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

    glBindVertexArray(0);
    */
    
    
    glEnable(GL_DEPTH_TEST);
    
}
