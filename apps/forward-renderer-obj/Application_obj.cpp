#include "Application.hpp"

#include <iostream>

#include <imgui.h>
#include <glmlv/imgui_impl_glfw_gl3.hpp>
#include <glmlv/simple_geometry.hpp>
#include <glmlv/ViewController.hpp>
#include <glmlv/Image2DRGBA.hpp>
#include <glmlv/load_obj.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


using namespace glmlv;
using namespace std;

int Application::run()
{
	ViewController viewController(m_GLFWHandle.window());
	const auto sceneDiagonalSize = glm::length(data.bboxMax - data.bboxMin);
		
	viewController.setSpeed(sceneDiagonalSize * 0.1f); // 10% de la scene parcouru par seconde
    float clearColor[3] = { 0, 0, 0 };
    float udk[3] = { 1, 1, 1};
    
    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
        const auto seconds = glfwGetTime();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        const auto viewportSize = m_GLFWHandle.framebufferSize();
        glViewport(0, 0, viewportSize.x, viewportSize.y);

		glm::mat4 ProjMatrix = glm::perspective(glm::radians(70.f), float(viewportSize.x)/viewportSize.y, 0.01f * sceneDiagonalSize, sceneDiagonalSize);
		glm::mat4 viewMatrix = viewController.getViewMatrix();
        // Put here rendering code

        glBindSampler(0, samplerObject);
           
        glUniform3f(directionalLightDir, 1.0, 1.0, 1.0);
	    glUniform3f(directionalLightIntensity,1.0, 1.0, 1.0);
	    glUniform3f(pointLightPosition,1.0, 1.0, 1.0);
	    glUniform3f(pointLightIntensity,1.0, 1.0, 1.0);

        glBindVertexArray(scene_vao);
        
		glm::mat4 MVMatrix = glm::translate(viewMatrix, glm::vec3(0, 0, -5));
		glm::mat4 NormalMatrix = glm::transpose(glm::inverse(MVMatrix));
        
        glUniformMatrix4fv(modelViewProject,1,GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
        glUniformMatrix4fv(modelView,1,GL_FALSE, glm::value_ptr(MVMatrix));
        glUniformMatrix4fv(normalMatrix,1,GL_FALSE, glm::value_ptr(NormalMatrix));
        
		auto indexOffset = 0;
		for (int i = 0; i < data.shapeCount ; i++)
		{
			glUniform1i(dSamplerLocation, 0);

			int32_t materialId = data.materialIDPerShape[i];
			glUniform3fv(kd, 1, value_ptr(data.materials[materialId].Kd));

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex_objects[data.materials[materialId].KdTextureId]);


			/*glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, tex_objects[data.materials[materialId].KsTextureId]);


			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, tex_objects[data.materials[materialId].KaTextureId]);


			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, tex_objects[data.materials[materialId].shininessTextureId]);*/


			const auto indexCount = data.indexCountPerShape[i];
			glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (const GLvoid*) (indexOffset * sizeof(GLuint)));
			indexOffset += indexCount;


			// glUniform3f(ks,data.materials[i].Ks[0],data.materials[i].Ks[1], data.materials[i].Ks[2]);
			// glUniform3f(ka,data.materials[i].Ka[0],data.materials[i].Ka[1], data.materials[i].Ka[2]);
			// glUniform1i(sSamplerLocation, 1);
			// glUniform1i(aSamplerLocation, 2);
		}


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
	ka = glGetUniformLocation(prog.glId(), "uKa");
	ks = glGetUniformLocation(prog.glId(), "uKs");

	prog.use();
    
	const GLint positionAttrLocation = 0;
    const GLint normalAttrLocation = 1;
    const GLint texCoordsAttrLocation = 2;

//	############################################ SCENE INITIALISATION #########################################################

	glmlv::loadObj(m_AssetsRootPath / m_AppName / "models" / "sponza.obj", data);

	glGenBuffers(1, &scene_vbo);
	
	glBindBuffer(GL_ARRAY_BUFFER, scene_vbo);
	
	glBufferStorage(GL_ARRAY_BUFFER, data.vertexBuffer.size() * sizeof(data.vertexBuffer[0]), data.vertexBuffer.data(), 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glGenBuffers(1, &scene_ibo);
	
	glBindBuffer(GL_ARRAY_BUFFER, scene_ibo);
	
	glBufferStorage(GL_ARRAY_BUFFER, data.indexBuffer.size() * sizeof(data.indexBuffer[0]), data.indexBuffer.data(), 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glGenVertexArrays(1, &scene_vao);
	
	glBindVertexArray(scene_vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, scene_vbo);
	
	glEnableVertexAttribArray(positionAttrLocation);
	glVertexAttribPointer(positionAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3f3f2f), (const GLvoid*) offsetof(Vertex3f3f2f, position));

	glEnableVertexAttribArray(normalAttrLocation);
    glVertexAttribPointer(normalAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3f3f2f), (const GLvoid*) offsetof(Vertex3f3f2f, normal));
    
    glEnableVertexAttribArray(texCoordsAttrLocation);
    glVertexAttribPointer(texCoordsAttrLocation, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3f3f2f), (const GLvoid*) offsetof(Vertex3f3f2f, texCoords));
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, scene_ibo);
	
	glBindVertexArray(0);
	
	
	glActiveTexture(GL_TEXTURE0);
	
	tex_objects.resize(data.textures.size());
	glGenTextures(tex_objects.size(), tex_objects.data());

	for (int i = 0; i < tex_objects.size(); i++){
		glBindTexture(GL_TEXTURE_2D, tex_objects[i]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, data.textures[i].width(), data.textures[i].height());
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, data.textures[i].width(), data.textures[i].height(), GL_RGBA, GL_UNSIGNED_BYTE, data.textures.data());
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glGenSamplers(1, &samplerObject);
    glSamplerParameteri(samplerObject, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(samplerObject, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    dSamplerLocation = glGetUniformLocation(prog.glId(), "uKdSampler");
    aSamplerLocation = glGetUniformLocation(prog.glId(), "uKaSampler");
    sSamplerLocation = glGetUniformLocation(prog.glId(), "uKsSampler");

    glEnable(GL_DEPTH_TEST);
    
}
