#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "crack_gui.h"
#include "thread.h"
// 哈希算法库
#include "md5.h"
#include "sha1.h"
#include "sha3.h"
#include "sha256.h"

#define IM_CLAMP(V, MN, MX)     ((V) < (MN) ? (MN) : (V) > (MX) ? (MX) : (V))

// *******************************************************************************
// **************** hashcat execute the crack process in thread mode *************
// *******************************************************************************
pthread_t hashcat_thread;
typedef struct hashcat_crack_struct{
    CrackHash *hashcat;
    const char *hash_value;
    CRACK_ALGO_TYPE *type;
    hashcat_crack_struct(CrackHash *hash, const char *value, CRACK_ALGO_TYPE *algo_type)
     : hashcat(hash), hash_value(value), type(algo_type)
     {}
} hashcat_crack_struct;

// thread to run the hashcat crack
void *hashcat_crack_hash(void *arg){
    hashcat_crack_struct *crack = (hashcat_crack_struct *) arg;
    CrackHash *hashcat = crack->hashcat;
    const char *hash_value = crack->hash_value;
    CRACK_ALGO_TYPE *type = crack->type;
    hashcat->crack(hash_value, *type);
}


// *******************************************************************************
// **************** IMGUI Windows Shader Class Implementations *******************
// *******************************************************************************
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

CrackGUIShader::CrackGUIShader(const char *result_save_path)
    : hashcat(result_save_path)
{
    init();
}

CrackGUIShader::~CrackGUIShader(){
    shutdown();
}

void CrackGUIShader::init(){
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()){
        std::cerr << "glfwInit() failed\n";
        exit(-1);
    }
    window = glfwCreateWindow(glfw_window_width, glfw_window_height, "Crack Password by HY DCU", nullptr, nullptr);
    if (window == nullptr){
        std::cerr << "Failed to create GLFW window!\n";
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    const char* glsl_version = "#version 140";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    static Shader m_shader("../shader/vShader.vs", "../shader/fShader.fs");
    m_shader.initArgument("../image/dcu_crack.png");
    p_shader = &m_shader;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.Fonts->AddFontFromFileTTF("../fonts/Cousine-Regular.ttf", 25.0f);
    ImGui::StyleColorsLight();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void CrackGUIShader::shutdown(){
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    p_shader->release();
    glfwTerminate();
}

void CrackGUIShader::shaderBegin(){
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 3);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 30));
}

void CrackGUIShader::shaderEnd(){
    ImGui::PopStyleVar(4);
    ImGui::Render();
    glfwGetFramebufferSize(window, &glfw_window_width, &glfw_window_height);
    glViewport(0, 0, glfw_window_width, glfw_window_height);
    p_shader->use();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}

void CrackGUIShader::showPasswordInput(){
    if(!ImGui::Begin("Password Input Board")){
        ImGui::End();
        return;
    }
    ImGui::InputText("Input Password", password, IM_ARRAYSIZE(password), ImGuiInputTextFlags_Password);
    ImGui::InputText("Confirm Password", re_password, IM_ARRAYSIZE(re_password), ImGuiInputTextFlags_Password);
    if(ImGui::Button("Enter", ImVec2(100, 40))){
        if(strcmp(password, re_password) == 0)
            password_entered = true;
        else
            password_correct_input = false;
    }
    if(!password_correct_input){
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s",
            "The passwords you entered are inconsistent!");
    }
    ImGui::End();
}

void CrackGUIShader::showPasswordCrackProgress() {
    if(!ImGui::Begin("Password Crack Board")){
        ImGui::End();
        return;
    }
    if(!hashcat_crack_started){
        hashcat_crack_started = true;
        // 获得该密码的哈希值
        CRACK::MD5 algo;
        type = CRACK_ALGO_TYPE_MD5;
        hash_value = algo(password);
        hashcat_crack_struct args(&hashcat, hash_value.c_str(), &type);
        int ret = hc_thread_create(hashcat_thread, hashcat_crack_hash, &args);
        if(ret != 0){
            perror("pthread_create");
            shutdown();
            exit(-1);
        }
        ret = pthread_join (hashcat_thread, NULL);
        if (ret != 0) {
            perror ("pthread_join");
            shutdown();
            exit(-1);
        }
    }
    if(hashcat.isCracking()){
        auto guess_base = hashcat.get_guess_base();
        float guess_base_progress = (1.0f * guess_base.first) / guess_base.second;
        char buf[32];
        sprintf(buf, "%d/%d", guess_base.first, guess_base.second);
        ImGui::ProgressBar(guess_base_progress, ImVec2(0.0f, 0.0f), buf);
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::Text("Crack Guess Progress");
    }
    if(hashcat.isCracked())
        password_cracked = true;
    ImGui::End();
}

void CrackGUIShader::showPasswordCracked(){
    if(!ImGui::Begin("Password Display Board")){
        ImGui::End();
        return;
    }
    std::string cracked_password = hashcat.getPassword();
    ImGui::Text("Cracked Password: %s", cracked_password.c_str());
    ImGui::End();
}


void CrackGUIShader::shader(){
    while (!glfwWindowShouldClose(window)){
        shaderBegin();
        setNextWindowFormat();
        if(!password_entered)
            showPasswordInput();
        setNextWindowFormat();
        if(password_entered && !password_cracked)
            showPasswordCrackProgress();
        setNextWindowFormat();
        if(password_entered && password_cracked)
            showPasswordCracked();
        shaderEnd();
    }
}

void CrackGUIShader::setNextWindowFormat(){
    double dcu_occupy = (170.0f / 720.0f);
    double ident_x = 0.2f;
    double ident_y = 0.1f;
    int window_pos_x = glfw_window_width * ident_x;
    int window_pos_y = glfw_window_height * (ident_y + dcu_occupy) ;
    int window_size_x = glfw_window_width * (1 - 2 * ident_x);
    int window_size_y = glfw_window_height * (1 - dcu_occupy) * (1 - 2 * ident_y);
    ImGui::SetNextWindowSize(ImVec2(window_size_x, window_size_y));
    ImGui::SetNextWindowPos(ImVec2(window_pos_x, window_pos_y));
}


// *******************************************************************************
// **************** OpenGL Windows Shader Class Implementations ******************
// *******************************************************************************
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
//从文件中读取顶点着色器程序和片段着色器程序
Shader::Shader(const char *vertexPath, const char *fragmentPath) {
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        vShaderFile.close();
        fShaderFile.close();
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch(std::ifstream::failure failure){
        std::cerr << "ERROR:SHADER::FILE_NOT_SUCESSFULLY_READ" << std::endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vShaderCode, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    ID = glCreateProgram();
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragmentShader);
    glLinkProgram(ID);
    // check for linking errors
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}
//使用着色器并绘制图形
void Shader::use()
{
    //使用着色器程序画出矩形
    glUseProgram(ID);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}


void Shader::setBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}
//初始化渲染参数
void Shader::initArgument(const char *imagePath) {
    float vertices[] = {
        //     ---- 位置 ------纹理坐标----
        //使矩形的面积占满窗口显示面积
        1.0f,  1.0f, 0.0f,   1.0f, 1.0f,   // 右上
        1.0f, -1.0f, 0.0f,   1.0f, 0.0f,   // 右下
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,   // 左下
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f    // 左上
    };
    //绘制两个三角形组成矩形
    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(imagePath, &width, &height, &nrChannels, 0);
    loadTexture(data, width, height);
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
//创建纹理
void Shader::loadTexture(unsigned char *data, int width, int height)
{
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if(data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        printf("Failed to load texture");
    }
}

void Shader::release(){
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

Shader::~Shader(){
    
}