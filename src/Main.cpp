#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <filesystem>

#include "shader.h"
#include "mesh.h"
#include "model.h"
#include "object.h"
#include "primitive.h"

#pragma warning( disable : 26451 )
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "fmod/fmod.hpp"
#include "fmod/fmod_errors.h"
#include "fmod/fmod_output.h"

//---function prototypes---
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xPos, double yYos);

//---camera vectors---
glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 0.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float screenSizeX = 800.0f;
float screenSizeY = 600.0f;

float yaw = -90.0f;
float pitch = 0;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float lastX = screenSizeX / 2;
float lastY = screenSizeY / 2;

bool firstMouse = false;
float bounceTime = 1.0f;

//some position globals which need to be used in both the update loop and drawScene
glm::vec3 lightPos = glm::vec3(1.0f, 2.0f, 0.0f);
glm::vec3 lightTarget = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 ballPos = glm::vec3(0);
glm::mat4 projection = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
float ballRotation = 0;

float fov = 60.0f;

float totalTime;
float cursor;

//mostly debug stuff, with some functionality no longer being possible. playback stuff was from when I was messing around with miniaudio
void drawIMGUI() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    //ImGui::SetWindowSize(ImVec2(200.0f, 200.0f));
    //ImGui::SetWindowCollapsed(true);

    static float f = 0.0f;

    ImGui::Begin("Boing"); 

    float tempFloat[3] = {lightPos.x, lightPos.y, lightPos.z};
    ImGui::InputFloat3("LightPos", tempFloat);
    lightPos.x = tempFloat[0]; lightPos.y = tempFloat[1]; lightPos.z = tempFloat[2];
    tempFloat[0] = lightTarget.x, tempFloat[1] = lightTarget.y; tempFloat[2] = lightTarget.z;
    ImGui::InputFloat3("LightTarget", tempFloat);
    lightTarget.x = tempFloat[0]; lightTarget.y = tempFloat[1]; lightTarget.z = tempFloat[2];

    ImGui::SliderFloat("Bounce Speed", &bounceTime, 1.0f, 5.0f);
    ImGui::SliderFloat("Camera FOV", &fov, 20.0f, 50.0f);

    ImGui::End();

    ImGui::Begin("Playback"); 

    ImGui::SameLine();
    int currentMin = cursor / 60;
    int currentSec = (int) cursor % 60;

    int totalMin = totalTime / 60;
    int totalSec = (int) totalTime % 60;

    ImGui::Text("time: %.2d:%.2d/%.2d:%.2d", currentMin, currentSec, totalMin, totalSec);
    if(ImGui::Button("pause"))
    { 
        //ma_sound_stop(&bgm);
    }
    if(ImGui::Button("stop"))
    { 
        //ma_sound_stop(&bgm);
        //ma_sound_seek_to_pcm_frame(&bgm, 0);
    }
    if(ImGui::Button("play"))
    { 
        //ma_sound_start(&bgm);
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void drawPlane();
void drawScene(Shader shader, unsigned int texture1, Object ball);

void framebufferSetup(unsigned int* FBO, unsigned int* FB_color, unsigned int* depthFBO, unsigned int* depthFB_tex)
{
    //---framebuffer---
    glGenFramebuffers(1, FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, *FBO);

    //buffer texture (color)
    glGenTextures(1, FB_color);
    glBindTexture(GL_TEXTURE_2D, *FB_color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenSizeX, screenSizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *FB_color, 0);

    //depth map
    unsigned int fb_texture_depth_map;
    glGenTextures(1, &fb_texture_depth_map);
    glBindTexture(GL_TEXTURE_2D, fb_texture_depth_map); 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, fb_texture_depth_map, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "ERROR: Failed to complete framebuffer" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //shadow framebuffer
    glGenFramebuffers(1, depthFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, *depthFBO);

    glGenTextures(1, depthFB_tex);
    glBindTexture(GL_TEXTURE_2D, *depthFB_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glBindFramebuffer(GL_FRAMEBUFFER, *depthFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *depthFB_tex, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "ERROR: Failed to complete framebuffer" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int main()
{
    //instantiate GLFW window
    glfwInit();

    //configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE)

    //create main window instance and make it the main window in the current context
    GLFWwindow* window = glfwCreateWindow(screenSizeX, screenSizeY, "learnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);

    //attempt to initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "failed to initialize GLAD" << std::endl;
        return EXIT_FAILURE;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    //set viewport dimensions
    glViewport(0, 0, screenSizeX, screenSizeY);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    //tell GLFW to call redefinition on window resize by user, respond to cursor activity
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //glfwSetCursorPosCallback(window, mouse_callback);

    char* audioLocation = "stadium_rave.mp3";

    //initialize fmod core
    FMOD::System* system = NULL;
    FMOD_RESULT result = FMOD::System_Create(&system);
    if (result != FMOD_OK)
    {
        std::cerr << "FMOD error: " << FMOD_ErrorString(result) << std::endl;
        exit(1);
    }

    result = system->init(512, FMOD_INIT_NORMAL, 0);
    if (result != FMOD_OK)
    {
        std::cerr << "FMOD error: " << FMOD_ErrorString(result) << std::endl;
        exit(1);
    }

    FMOD::Sound* bgm = NULL;
    //can add FMOD_NONBLOCKING flag to avoid blocking on file load, but that 
    //requires checking if the sound is loaded in the update loop
    result = system->createSound(audioLocation, FMOD_CREATESTREAM, 0, &bgm);

    //--- shader paths ---
    char* basicShaderVertexPath = "C:/code/graphics_for_games/G4G_Basic-master/src/Project1/basicShader.vs";
    char* basicShaderGeoPath = "C:/code/graphics_for_games/G4G_Basic-master/src/Project1/basicShader.gs.glsl";
    char* basicShaderFragmentPath = "C:/code/graphics_for_games/G4G_Basic-master/src/Project1/basicShader.fs";
    char* FBShaderVertexPath = "C:/code/graphics_for_games/G4G_Basic-master/src/Project1/FBShader.vs";
    char* FBShaderFragmentPath = "C:/code/graphics_for_games/G4G_Basic-master/src/Project1/FBShader.fs";
    char* depthShaderVertexPath= "C:/code/graphics_for_games/G4G_Basic-master/src/Project1/shaders/depthShader.vs";
    char* depthShaderFragmentPath= "C:/code/graphics_for_games/G4G_Basic-master/src/Project1/shaders/depthShader.fs";

    Shader basicShader = Shader(basicShaderVertexPath, basicShaderGeoPath, basicShaderFragmentPath);
    Shader FBShader = Shader(FBShaderVertexPath, FBShaderFragmentPath);
    Shader depthShader = Shader(depthShaderVertexPath, depthShaderFragmentPath);

    //set up model
    Object ball("C:/code/graphics_for_games/assets/amiga_ball/amiga_ball.obj", &basicShader);
    //Object ball("C:/code/graphics_for_games/assets/backpack/backpack.obj", &basicShader);
    ball.rotate(-30.0f, 0.0f, 0.0f, 1.0f);
    ball.setScreenSize(screenSizeX, screenSizeY);

    //generate texture in openGL
    unsigned int texture1;
    glGenTextures(1, &texture1);

    //load textures
    int width, height, nrChannels;
    unsigned char* data = stbi_load("C:/code/graphics_for_games/assets/grid.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glBindTexture(GL_TEXTURE_2D, texture1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        //set texture wrap parameters ((s,t) == (x, y))
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        //set texture filtering to nearest
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
        std::cerr << "Failed to load texture" << std::endl;

    //free image memory
    stbi_image_free(data);

    FBShader.use();
    FBShader.setInt("framebuffer_texture", 0);

    //---framebuffer---

    float FB_vertices[] = {
   -1.0f,  1.0f, 0.0f, 1.0f,
   -1.0f, -1.0f, 0.0f, 0.0f,
    1.0f, -1.0f, 1.0f, 0.0f,
   -1.0f,  1.0f, 0.0f, 1.0f,
    1.0f, -1.0f, 1.0f, 0.0f,
    1.0f,  1.0f, 1.0f, 1.0f
    };

    unsigned int FB_VAO, FB_VBO;
    glGenVertexArrays(1, &FB_VAO);
    glGenBuffers(1, &FB_VBO);
    glBindVertexArray(FB_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, FB_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(FB_vertices), FB_vertices, GL_STATIC_DRAW);

    //interpret vertex data
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //normals
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int FBO, fb_texture_color, depthFBO, depthMap;
    framebufferSetup(&FBO, &fb_texture_color, &depthFBO, &depthMap);

    //render loop
    //keep window open until window is told to close

    glEnable(GL_DEPTH_TEST);

    //start playing the sound and attach a DSP node to it
    //this allows the calls in the while loop to monitor peak data
    FMOD::Channel *channel = NULL;
    FMOD::ChannelGroup *channelGroup = NULL;
    system->playSound(bgm, channelGroup, false, &channel);
    
    FMOD::DSP* dsp;
    FMOD_DSP_METERING_INFO dspInfo;

    system->createDSPByType(FMOD_DSP_TYPE_LOUDNESS_METER, &dsp);
    channel->addDSP(0, dsp);
    dsp->setMeteringEnabled(true, true);

    ball.rotate(-30.0f, 0.0f, 0.0f, 1.0f);
    //giving the model the screen size here so Object.Draw() can do the projection calculations itself.
    //with more time, i'd like to give this sort of information to a scene class that can give this information
    //to all the objects within it in a loop, instead of explicitly like this.
    ball.setScreenSize(screenSizeX, screenSizeY); 
    stbi_set_flip_vertically_on_load(true);
    while (!glfwWindowShouldClose(window))
    {
        //setting FOV in update loop so ImGui can update it
        ball.setFOV(fov);
        //---calculate current frame---
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        glEnable(GL_DEPTH_TEST);

        //tell FMOD core to update and retrieve peak data to use later
        system->update();
        dsp->getMeteringInfo(0, &dspInfo);
        float peak = dspInfo.peaklevel[0];

        basicShader.use();
        basicShader.setFloat("magnitude", peak);
        
        //---first shadow buffer pass (from light)---
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        depthShader.use();
        
        //light projection calculations
        float near_plane = 0.0f, far_plane = 10.0f;
        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        glm::mat4 lightView = glm::lookAt(lightPos, lightTarget, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        ball.setShader(&depthShader);

        //glDisable(GL_CULL_FACE);
        drawScene(depthShader, texture1, ball);

        //---first framebuffer pass---
        glViewport(0, 0, screenSizeX, screenSizeY);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);

        //clear screen with color
        glm::vec3 color = glm::vec3(.8f, .3f, .8f);
        color *= peak / 3;
        glClearColor(color.x, color.y, color.z, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ball.setShader(&basicShader);

        //redo projections for actual camera
        projection = glm::perspective(glm::radians(fov), screenSizeX / screenSizeY, 0.1f, 100.0f);
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        basicShader.use();
        basicShader.setVec3("viewPos", cameraPos.x, cameraPos.y, cameraPos.z);
        basicShader.setInt("shadowMap", depthMap);
        basicShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        drawScene(basicShader, texture1, ball);

        //---second framebuffer pass---
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);

        //set framebuffer kernel/offset
        FBShader.use();
        float side = .5f * cos(3.1415f * glfwGetTime() / 10.0f) + 1.5f;
        float corner = .5f * cos(3.1415f * glfwGetTime() / 10.0f + 3.1415f) + 1.5f;
        FBShader.setVec3("kernelTop", corner/16.0f, side/16.0f, corner/16.0f);
        FBShader.setVec3("kernelMid", side/16.0f, 4.0f/16.0f, side/16.0f);
        FBShader.setVec3("kernelBottom", corner/16.0f, side/16.0f, corner/16.0f);
        //FBShader.setVec3("kernelTop", 1.0f, 1.0f, 1.0f);
        //FBShader.setVec3("kernelMid", 1.0f, 1.0f, 1.0f);
        //FBShader.setVec3("kernelBottom", 1.0f, 1.0f, 1.0f);
        FBShader.setFloat("offset", 1.0f / (300.0f * sin(glfwGetTime()) + 301.0f));
        //FBShader.setFloat("offset", 1.0f / 1000.0f);
        glBindVertexArray(FB_VAO);
        glBindTexture(GL_TEXTURE_2D, fb_texture_color);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        //drawIMGUI();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteFramebuffers(1, &FBO);
    glDeleteFramebuffers(1, &depthFBO);

    system->release();

    glfwTerminate();
    exit(0);
}

void drawPlane()
{
    float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
    };

    //generate vertex buffer object
    unsigned int VBO;
    glGenBuffers(1, &VBO);

    //create vertex array object to store our configuration
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    //copy vertices into buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //copy vertex data into buffer memory
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //interpret vertex data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //texture coordinates
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void drawScene(Shader shader, unsigned int texture1, Object ball)
{
    /*
    //draw planes
    shader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, -6.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0, 0));
    model = glm::scale(model, glm::vec3(5.0f, 5.0f, 1.0f));

    //---misc shader values---
    glm::mat4 modelNormal = glm::transpose(glm::inverse(model));

    shader.use();
    
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    shader.setMat4("model", model);
    shader.setMat4("normalMatrix", modelNormal);

    shader.setVec3("lightColor", .7f, 1.0f, .8f);
    shader.setVec3("light.position", lightPos.x, lightPos.y, lightPos.z);
    shader.setVec3("viewPos", cameraPos);
    shader.setVec3("globalPos", 0, -1.0f, -3.0f);

    shader.setVec3("material.ambient", .05f, .05f, .05f);
    shader.setVec3("material.specular", .7f, .7f, .7f);
    shader.setInt("material.diffuse0", texture1);
    shader.setFloat("material.shininess", 1.0f);

    shader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
    shader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
    shader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

    shader.setInt("material.diffuse", 0);

    //drawPlane();

    //cube 2
    shader.use();
    glBindTexture(GL_TEXTURE_2D, texture1);
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, -5.0f));
    model = glm::scale(model, glm::vec3(5.0f, 5.0f, 0.5f));
    modelNormal = glm::transpose(glm::inverse(model));

    shader.setMat4("model", model);
    shader.setMat4("normalMatrix", modelNormal);
    shader.setVec3("globalPos", 0, -1.0f, -3.0f);

    //drawPlane();
    */

    ball.setCameraPos(cameraPos, cameraFront);
    float bounceHeight = 3.0f;
    float bounceRepeat = 2 * (1 / bounceTime) * sqrt(bounceHeight);

    float xPos = 0, vertPos = 2;

    ballPos = glm::vec3(xPos, vertPos, -3.0f);
    //shader.setFloat("time", cursor / totalTime);
    shader.setVec3("globalPos", ballPos.x, ballPos.y, ballPos.z);
    ball.translate(xPos, vertPos, -3.0f);
    ballRotation -= 20 * deltaTime;
    ball.rotate(ballRotation, 0.0f, 1.0f, 0.0f);
    ball.scale(.8f, .8f, .8f);

    //fragment uniforms
    ball.setMaterial(glm::vec3(.05f, .05f, .05f), .078125f * 10);
    ball.setLight(lightPos, glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f));

    ball.Draw();
}

void processInput(GLFWwindow* window)
{
    const float cameraSpeed = 1.0f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += glm::vec3(0.0f, cameraSpeed, 0.0f);
    //cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= glm::vec3(0.0f, cameraSpeed, 0.0f);
    //cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xPos, double yPos)
{
    float xOffset = xPos - lastX;
    float yOffset = lastY - yPos;
    lastX = xPos;
    lastY = yPos;

    if (firstMouse)
    {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }

    float sensitivity = 0.1f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    yaw += xOffset;
    pitch += yOffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}
