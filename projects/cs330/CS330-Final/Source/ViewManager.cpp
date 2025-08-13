///////////////////////////////////////////////////////////////////////////////
// viewmanager.cpp
// ===============
// manage the viewing of 3D objects within the viewport
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//  Updated for CS-330 Final Project by Charity & ChatGPT
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>    

namespace
{
    const int WINDOW_WIDTH = 1000;
    const int WINDOW_HEIGHT = 800;
    const char* g_ViewName = "view";
    const char* g_ProjectionName = "projection";

    Camera* g_pCamera = nullptr;

    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    float gDeltaTime = 0.0f;
    float gLastFrame = 0.0f;

    bool bOrthographicProjection = false;
}

ViewManager::ViewManager(ShaderManager* pShaderManager)
{
    m_pShaderManager = pShaderManager;
    m_pWindow = NULL;
    g_pCamera = new Camera();

    // Default camera view
    g_pCamera->Position = glm::vec3(0.0f, 5.0f, 12.0f);
    g_pCamera->Front = glm::vec3(0.0f, -0.5f, -2.0f);
    g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
    g_pCamera->Zoom = 80.0f;
}

ViewManager::~ViewManager()
{
    m_pShaderManager = NULL;
    m_pWindow = NULL;
    if (g_pCamera)
    {
        delete g_pCamera;
        g_pCamera = NULL;
    }
}

GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle)
{
    GLFWwindow* window = glfwCreateWindow(
        WINDOW_WIDTH, WINDOW_HEIGHT,
        windowTitle,
        NULL, NULL);

    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);

    // Capture the mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);

    // Enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_pWindow = window;
    return window;
}

void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos)
{
    if (gFirstMouse)
    {
        gLastX = (float)xMousePos;
        gLastY = (float)yMousePos;
        gFirstMouse = false;
    }

    float xoffset = (float)xMousePos - gLastX;
    float yoffset = gLastY - (float)yMousePos;

    gLastX = (float)xMousePos;
    gLastY = (float)yMousePos;

    g_pCamera->ProcessMouseMovement(xoffset, yoffset);
}

void ViewManager::ProcessKeyboardEvents()
{
    if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(m_pWindow, true);

    if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
        g_pCamera->ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
        g_pCamera->ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
        g_pCamera->ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
        g_pCamera->ProcessKeyboard(RIGHT, gDeltaTime);
    if (glfwGetKey(m_pWindow, GLFW_KEY_P) == GLFW_PRESS)
        bOrthographicProjection = !bOrthographicProjection;
}

void ViewManager::PrepareSceneView()
{
    glm::mat4 view;
    glm::mat4 projection;

    float currentFrame = glfwGetTime();
    gDeltaTime = currentFrame - gLastFrame;
    gLastFrame = currentFrame;

    ProcessKeyboardEvents();
    view = g_pCamera->GetViewMatrix();

    if (bOrthographicProjection)
    {
        float orthoSize = 10.0f;
        projection = glm::ortho(
            -orthoSize, orthoSize,
            -orthoSize * ((float)WINDOW_HEIGHT / WINDOW_WIDTH),
            orthoSize * ((float)WINDOW_HEIGHT / WINDOW_WIDTH),
            0.1f, 100.0f);
    }
    else
    {
        projection = glm::perspective(
            glm::radians(g_pCamera->Zoom),
            (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT,
            0.1f, 100.0f);
    }

    if (m_pShaderManager)
    {
        m_pShaderManager->setMat4Value(g_ViewName, view);
        m_pShaderManager->setMat4Value(g_ProjectionName, projection);
        m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);
    }
}
