#include "Application.hpp"

#include <iostream>
#include <unordered_set>
#include <algorithm>

#include <imgui.h>
#include <glmlv/imgui_impl_glfw_gl3.hpp>
#include <glmlv/Image2DRGBA.hpp>
#include <glmlv/load_obj.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>

int Application::run()
{
    float clearColor[3] = { 0, 0, 0 };
    int selected = Gposition;

    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
        const auto seconds = glfwGetTime();

        // Put here rendering code
        const auto viewportSize = m_GLFWHandle.framebufferSize();
        glViewport(0, 0, viewportSize.x, viewportSize.y);

        m_program.use();

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const auto projMatrix = glm::perspective(70.f, float(viewportSize.x) / viewportSize.y, 0.01f * m_SceneSize, m_SceneSize);
        const auto viewMatrix = m_viewController.getViewMatrix();

        const auto modelMatrix = glm::mat4();

        const auto mvMatrix = viewMatrix * modelMatrix;
        const auto mvpMatrix = projMatrix * mvMatrix;
        const auto normalMatrix = glm::transpose(glm::inverse(mvMatrix));

        glUniformMatrix4fv(m_uModelViewProjMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
        glUniformMatrix4fv(m_uModelViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvMatrix));
        glUniformMatrix4fv(m_uNormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(normalMatrix));

        // Same sampler for all texture units
        for (GLuint i : {0, 1, 2, 3})
            glBindSampler(i, m_textureSampler);

        // Set texture unit of each sampler
        glUniform1i(m_uKaSamplerLocation, 0);
        glUniform1i(m_uKdSamplerLocation, 1);
        glUniform1i(m_uKsSamplerLocation, 2);
        glUniform1i(m_uShininessSamplerLocation, 3);

        const auto bindMaterial = [&](const PhongMaterial & material)
        {
            glUniform3fv(m_uKaLocation, 1, glm::value_ptr(material.Ka));
            glUniform3fv(m_uKdLocation, 1, glm::value_ptr(material.Kd));
            glUniform3fv(m_uKsLocation, 1, glm::value_ptr(material.Ks));
            glUniform1fv(m_uShininessLocation, 1, &material.shininess);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, material.KaTextureId);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, material.KdTextureId);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, material.KsTextureId);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, material.shininessTextureId);
        };

        glBindVertexArray(m_SceneVAO);
        
        const PhongMaterial * currentMaterial = nullptr;

        // We draw each shape by specifying how much indices it carries, and with an offset in the global index buffer
        for (const auto shape : m_shapes)
        {
            const auto & material = shape.materialID >= 0 ? m_SceneMaterials[shape.materialID] : m_DefaultMaterial;
            if (currentMaterial != &material)
            {
                bindMaterial(material);
                currentMaterial = &material;
            }
            glDrawElements(GL_TRIANGLES, shape.indexCount, GL_UNSIGNED_INT, (const GLvoid*) (shape.indexOffset * sizeof(GLuint)));
        }

        for (GLuint i : {0, 1, 2, 3})
            glBindSampler(i, 0);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        // Shading pass
        m_program2.use();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUniform3fv(m_uDirectionalLightDirLocation, 1, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(glm::normalize(m_DirLightDirection), 0))));
        glUniform3fv(m_uDirectionalLightIntensityLocation, 1, glm::value_ptr(m_DirLightColor * m_DirLightIntensity));

        glUniform3fv(m_uPointLightPositionLocation, 1, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(m_PointLightPosition, 1))));
        glUniform3fv(m_uPointLightIntensityLocation, 1, glm::value_ptr(m_PointLightColor * m_PointLightIntensity));
        
        for (GLuint i : {0, 1, 2, 3, 4})
            glBindSampler(i, m_textureSampler);
        
        glUniform1i(m_GPositionLocation, 0);
        glUniform1i(m_GNormalLocation, 1);
        glUniform1i(m_GAmbientLocation, 2);
        glUniform1i(m_GDiffuseLocation, 3);
        glUniform1i(m_GlossyShininessLocation, 4);


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[Gposition]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GNormal]);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GDiffuse]);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GAmbient]);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GGlossyShininess]);

        // Put here rendering code
        glBindVertexArray(m_TriangleVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        // GUI code:
        ImGui_ImplGlfwGL3_NewFrame();

        {
            ImGui::Begin("GUI");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::ColorEditMode(ImGuiColorEditMode_RGB);
            if (ImGui::ColorEdit3("clearColor", clearColor)) {
                glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.f);
            }
            if (ImGui::Button("Sort shapes wrt materialID"))
            {
                std::sort(begin(m_shapes), end(m_shapes), [&](auto lhs, auto rhs)
                {
                    return lhs.materialID < rhs.materialID;
                });
            }
            if (ImGui::CollapsingHeader("Directional Light"))
            {
                ImGui::ColorEdit3("DirLightColor", glm::value_ptr(m_DirLightColor));
                ImGui::DragFloat("DirLightIntensity", &m_DirLightIntensity, 0.1f, 0.f, 100.f);
                if (ImGui::DragFloat("Phi Angle", &m_DirLightPhiAngleDegrees, 1.0f, 0.0f, 360.f) ||
                    ImGui::DragFloat("Theta Angle", &m_DirLightThetaAngleDegrees, 1.0f, 0.0f, 180.f)) {
                    m_DirLightDirection = computeDirectionVector(glm::radians(m_DirLightPhiAngleDegrees), glm::radians(m_DirLightThetaAngleDegrees));
                }
            }

            if (ImGui::CollapsingHeader("Point Light"))
            {
                ImGui::ColorEdit3("PointLightColor", glm::value_ptr(m_PointLightColor));
                ImGui::DragFloat("PointLightIntensity", &m_PointLightIntensity, 0.1f, 0.f, 16000.f);
                ImGui::InputFloat3("Position", glm::value_ptr(m_PointLightPosition));
            }

            ImGui::RadioButton("position", &selected, 0);
            ImGui::RadioButton("Normal", &selected, 1);
            ImGui::RadioButton("Ambient", &selected, 2);
            ImGui::RadioButton("Diffuse", &selected, 3);
            ImGui::RadioButton("GlossyShininess", &selected, 4);
            ImGui::RadioButton("Depth", &selected, 5);
            
            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBO);

            switch(selected){
                case Gposition:
                    glReadBuffer(GL_COLOR_ATTACHMENT0 + Gposition);
                    break;
                case GNormal:
                    glReadBuffer(GL_COLOR_ATTACHMENT0 + GNormal);
                    break;
                case GAmbient:
                    glReadBuffer(GL_COLOR_ATTACHMENT0 + GAmbient);
                    break;
                case GDiffuse:
                    glReadBuffer(GL_COLOR_ATTACHMENT0 + GDiffuse);
                    break;
                case GGlossyShininess:
                    glReadBuffer(GL_COLOR_ATTACHMENT0 + GGlossyShininess);
                    break;
                case GDepth:
                    glReadBuffer(GL_COLOR_ATTACHMENT0 + GDepth);
                    break;
                default:
                    break;
            }

            //glBlitFramebuffer(0, 0, m_nWindowWidth, m_nWindowHeight, 0, 0, m_nWindowWidth, m_nWindowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

            ImGui::End();
        }

        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        
        ImGui::Render();

        /* Poll for and process events */
        glfwPollEvents();

        /* Swap front and back buffers*/
        m_GLFWHandle.swapBuffers();

        auto ellapsedTime = glfwGetTime() - seconds;
        auto guiHasFocus = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
        if (!guiHasFocus) {
            m_viewController.update(float(ellapsedTime));
        }
    }

    return 0;
}

struct Vertex
{
    glm::vec2 position;

    Vertex(glm::vec2 positionr):
        position(position)
    {}
};


Application::Application(int argc, char** argv):
    m_AppPath { glmlv::fs::path{ argv[0] } },
    m_AppName { m_AppPath.stem().string() },
    m_ImGuiIniFilename { m_AppName + ".imgui.ini" },
    m_ShadersRootPath { m_AppPath.parent_path() / "shaders" },
    m_AssetsRootPath { m_AppPath.parent_path() / "assets" }

{
    ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file

    glGenBuffers(1, &m_SceneVBO);
    glGenBuffers(1, &m_SceneIBO);

    {
        const auto objPath =  m_AssetsRootPath / "glmlv" / "models" / "crytek-sponza" / "sponza.obj";
        glmlv::ObjData data;
        loadObj(objPath, data);
        m_SceneSize = glm::length(data.bboxMax - data.bboxMin);

        std::cout << "# of shapes    : " << data.shapeCount << std::endl;
        std::cout << "# of materials : " << data.materialCount << std::endl;
        std::cout << "# of vertex    : " << data.vertexBuffer.size() << std::endl;
        std::cout << "# of triangles    : " << data.indexBuffer.size() / 3 << std::endl;

        // Fill VBO
        glBindBuffer(GL_ARRAY_BUFFER, m_SceneVBO);
        glBufferStorage(GL_ARRAY_BUFFER, data.vertexBuffer.size() * sizeof(glmlv::Vertex3f3f2f), data.vertexBuffer.data(), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Fill IBO
        glBindBuffer(GL_ARRAY_BUFFER, m_SceneIBO);
        glBufferStorage(GL_ARRAY_BUFFER, data.indexBuffer.size() * sizeof(uint32_t), data.indexBuffer.data(), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Init shape infos
        uint32_t indexOffset = 0;
        for (auto shapeID = 0; shapeID < data.indexCountPerShape.size(); ++shapeID)
        {
            m_shapes.emplace_back();
            auto & shape = m_shapes.back();
            shape.indexCount = data.indexCountPerShape[shapeID];
            shape.indexOffset = indexOffset;
            shape.materialID = data.materialIDPerShape[shapeID];
            indexOffset += shape.indexCount;
        }

        glGenTextures(1, &m_WhiteTexture);
        glBindTexture(GL_TEXTURE_2D, m_WhiteTexture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, 1, 1);
        glm::vec4 white(1.f, 1.f, 1.f, 1.f);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_FLOAT, &white);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Upload all textures to the GPU
        std::vector<GLint> textureIds;
        for (const auto & texture : data.textures)
        {
            GLuint texId = 0;
            glGenTextures(1, &texId);
            glBindTexture(GL_TEXTURE_2D, texId);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, texture.width(), texture.height());
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture.width(), texture.height(), GL_RGBA, GL_UNSIGNED_BYTE, texture.data());
            glBindTexture(GL_TEXTURE_2D, 0);

            textureIds.emplace_back(texId);
        }

        for (const auto & material : data.materials)
        {
            PhongMaterial newMaterial;
            newMaterial.Ka = material.Ka;
            newMaterial.Kd = material.Kd;
            newMaterial.Ks = material.Ks;
            newMaterial.shininess = material.shininess;
            newMaterial.KaTextureId = material.KaTextureId >= 0 ? textureIds[material.KaTextureId] : m_WhiteTexture;
            newMaterial.KdTextureId = material.KdTextureId >= 0 ? textureIds[material.KdTextureId] : m_WhiteTexture;
            newMaterial.KsTextureId = material.KsTextureId >= 0 ? textureIds[material.KsTextureId] : m_WhiteTexture;
            newMaterial.shininessTextureId = material.shininessTextureId >= 0 ? textureIds[material.shininessTextureId] : m_WhiteTexture;

            m_SceneMaterials.emplace_back(newMaterial);
        }

        m_DefaultMaterial.Ka = glm::vec3(0);
        m_DefaultMaterial.Kd = glm::vec3(1);
        m_DefaultMaterial.Ks = glm::vec3(1);
        m_DefaultMaterial.shininess = 32.f;
        m_DefaultMaterial.KaTextureId = m_WhiteTexture;
        m_DefaultMaterial.KdTextureId = m_WhiteTexture;
        m_DefaultMaterial.KsTextureId = m_WhiteTexture;
        m_DefaultMaterial.shininessTextureId = m_WhiteTexture;
    }

    // Fill VAO
    glGenVertexArrays(1, &m_SceneVAO);
    glBindVertexArray(m_SceneVAO);

    const GLint positionLocation = 0;
    const GLint normalAttrLocation = 1;
    const GLint texCoordsAttrLocation = 2;

    // We tell OpenGL what vertex attributes our VAO is describing:
    glEnableVertexAttribArray(positionLocation);
    glEnableVertexAttribArray(normalAttrLocation);
    glEnableVertexAttribArray(texCoordsAttrLocation);

    glBindBuffer(GL_ARRAY_BUFFER, m_SceneVBO); // We bind the VBO because the next 3 calls will read what VBO is bound in order to know where the data is stored

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*)offsetof(glmlv::Vertex3f3f2f, position));
    glVertexAttribPointer(normalAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*)offsetof(glmlv::Vertex3f3f2f, normal));
    glVertexAttribPointer(texCoordsAttrLocation, 2, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*)offsetof(glmlv::Vertex3f3f2f, texCoords));

    glBindBuffer(GL_ARRAY_BUFFER, 0); // We can unbind the VBO because OpenGL has "written" in the VAO what VBO it needs to read when the VAO will be drawn

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_SceneIBO); // Binding the IBO to GL_ELEMENT_ARRAY_BUFFER while a VAO is bound "writes" it in the VAO for usage when the VAO will be drawn

    glBindVertexArray(0);

    // Note: no need to bind a sampler for modifying it: the sampler API is already direct_state_access
    glGenSamplers(1, &m_textureSampler);
    glSamplerParameteri(m_textureSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(m_textureSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    const GLenum m_GBufferTextureFormat[GBufferTextureCount] = { GL_RGB32F, GL_RGB32F, GL_RGB32F, GL_RGB32F, GL_RGBA32F, GL_DEPTH_COMPONENT32F };

    glGenTextures(GBufferTextureCount, m_GBufferTextures);

    for (int i = 0; i < GBufferTextureCount; i++){
        glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[i]);
        glTexStorage2D(GL_TEXTURE_2D, 1, m_GBufferTextureFormat[i], m_nWindowWidth, m_nWindowHeight);
        glBindTexture(GL_TEXTURE_2D, 0);
    }


    glGenFramebuffers(1, &m_FBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);

    const auto drawBufferCount = GDepth;
    for(int i = 0; i < drawBufferCount; i++) {
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_GBufferTextures[i], 0);
    }

    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_GBufferTextures[GDepth], 0);

    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };

    glDrawBuffers(drawBufferCount, drawBuffers);

    if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER))
    {
        std::cerr << "Oups" << std::endl;
        exit(-1);
    }

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    // Triangle for full screen rendering on shading pass
    glGenBuffers(1, &m_TriangleVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_TriangleVBO);

    GLfloat data[] = { -1, -1, 3, -1, -1, 3 };
    glBufferStorage(GL_ARRAY_BUFFER, sizeof(data), data, 0);

    glGenVertexArrays(1, &m_TriangleVAO);
    glBindVertexArray(m_TriangleVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);

    m_program = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "geometryPass.vs.glsl", m_ShadersRootPath / m_AppName / "geometryPass.fs.glsl" });
    m_program.use();

    m_uModelViewProjMatrixLocation = glGetUniformLocation(m_program.glId(), "uModelViewProjMatrix");
    m_uModelViewMatrixLocation = glGetUniformLocation(m_program.glId(), "uModelViewMatrix");
    m_uNormalMatrixLocation = glGetUniformLocation(m_program.glId(), "uNormalMatrix");

    m_uKaLocation = glGetUniformLocation(m_program.glId(), "uKa");
    m_uKdLocation = glGetUniformLocation(m_program.glId(), "uKd");
    m_uKsLocation = glGetUniformLocation(m_program.glId(), "uKs");
    m_uShininessLocation = glGetUniformLocation(m_program.glId(), "uShininess");
    m_uKaSamplerLocation = glGetUniformLocation(m_program.glId(), "uKaSampler");
    m_uKdSamplerLocation = glGetUniformLocation(m_program.glId(), "uKdSampler");
    m_uKsSamplerLocation = glGetUniformLocation(m_program.glId(), "uKsSampler");
    m_uShininessSamplerLocation = glGetUniformLocation(m_program.glId(), "uShininessSampler");

    m_viewController.setSpeed(m_SceneSize * 0.1f); // Let's travel 10% of the scene per second

    m_program2 = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "shadingPass.vs.glsl", m_ShadersRootPath / m_AppName / "shadingPass.fs.glsl" });
    m_program2.use();

    m_GPositionLocation = glGetUniformLocation(m_program2.glId(), "uGPosition");
    m_GNormalLocation = glGetUniformLocation(m_program2.glId(), "uGNormal");
    m_GAmbientLocation = glGetUniformLocation(m_program2.glId(), "uGAmbient");
    m_GDiffuseLocation = glGetUniformLocation(m_program2.glId(), "uGDiffuse");
    m_GlossyShininessLocation = glGetUniformLocation(m_program2.glId(), "uGlossyShininess");

    m_uDirectionalLightDirLocation = glGetUniformLocation(m_program2.glId(), "uDirectionalLightDir");
    m_uDirectionalLightIntensityLocation = glGetUniformLocation(m_program2.glId(), "uDirectionalLightIntensity");

    m_uPointLightPositionLocation = glGetUniformLocation(m_program2.glId(), "uPointLightPosition");
    m_uPointLightIntensityLocation = glGetUniformLocation(m_program2.glId(), "uPointLightIntensity");

}
