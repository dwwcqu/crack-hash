#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "crack_gui.h"
#include <pthread.h>
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
    int res = 0;
    pthread_exit((void *)&res);
}


// *******************************************************************************
// **************** IMGUI Windows Shader Class Implementations *******************
// *******************************************************************************
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

CrackGUIShader::CrackGUIShader(const char *result_save_path)
    : hashcat(new CrackHash(result_save_path))
{
    init();
}

CrackGUIShader::~CrackGUIShader(){
    delete hashcat;
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
    const char *project_dir = "";
#ifdef CRACK_PROJECT_DIR
    project_dir = CRACK_PROJECT_DIR;
#endif
    std::string dir(project_dir);
    static Shader m_shader((dir + "/shader/vShader.vs").c_str(), (dir + "/shader/fShader.fs").c_str());
    m_shader.initArgument((dir + "/image/dcu_crack.png").c_str());
    p_shader = &m_shader;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.Fonts->AddFontFromFileTTF((dir + "/fonts/Cousine-Regular.ttf").c_str(), 25.0f);
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
    ImGui::Text("Mode Select: ");
    ImGui::RadioButton("Hash Mode", &crack_mode, 1);
    ImGui::SameLine();
    ImGui::RadioButton("Password Mode", &crack_mode, 2);
    ImGui::Text("Hash Algorithm Select: ");
    ImGui::RadioButton("MD5", &hash_algo, 1);
    ImGui::SameLine();
    ImGui::RadioButton("SHA1", &hash_algo, 2);
    ImGui::SameLine();
    ImGui::RadioButton("SHA256", &hash_algo, 3);
    switch (hash_algo)
    {
        case 1 : type = CRACK_ALGO_TYPE_MD5; break;
        case 2 : type = CRACK_ALGO_TYPE_SHA1; break;
        case 3 : type = CRACK_ALGO_TYPE_SHA256; break;
        default: break;
    }
    if(crack_mode == 1){
        ImGui::InputText("Hash Value", hash_input, IM_ARRAYSIZE(hash_input), ImGuiInputTextFlags_CharsHexadecimal);
    }
    else{
        ImGui::InputText("Input Password", password, IM_ARRAYSIZE(password), ImGuiInputTextFlags_Password);
        ImGui::InputText("Confirm Password", re_password, IM_ARRAYSIZE(re_password), ImGuiInputTextFlags_Password);
    }
    if(ImGui::Button("Enter", ImVec2(100, 40))){
        // 哈希值输入模式
        if(crack_mode == 1){
            int len = strlen(hash_input);
            if((hash_algo == 1 && len == 32) || (hash_algo == 2 && len == 40)
               || (hash_algo == 2 && len == 64)){
                    hash_correct_input = true;
                    input_complete = true;
               }
            else 
                hash_correct_input = false;
        }
        // 密码输入模式
        else{
            if(strcmp(password, re_password) != 0)
                password_correct_input = false;
            else
                input_complete = true;
        }
    }
    if(crack_mode == 1 && !hash_correct_input){
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s",
            "The hash value you entered does not match the hash algorithm!");
    }
    if(crack_mode == 2 && !password_correct_input){
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
    // 在单个线程中运行 hashcat 进行破解
    if(!hashcat_crack_started){
        hashcat_crack_started = true;       // 只需要运行一次
        // 密码输入模式，需要先计算密码的哈希值
        if(crack_mode == 2){
            if(hash_algo == 1){
                CRACK::MD5 algo;
                hash_value = algo(password);
            }
            else if(hash_algo == 2){
                CRACK::SHA1 algo;
                hash_value = algo(password);
            }
            else {
                CRACK::SHA256 algo;
                hash_value = algo(password);
            }
        }
        // 根据不同的模式修改要破解的哈希值
        hashcat_crack_struct args(hashcat, NULL, &type);
        if(crack_mode == 1)
            args.hash_value = hash_input;
        else
            args.hash_value = hash_value.c_str();
        pthread_attr_t pAttr;
        pthread_attr_init(&pAttr);
        pthread_attr_setdetachstate(&pAttr, PTHREAD_CREATE_DETACHED);
        int ret = pthread_create(&hashcat_thread, NULL, hashcat_crack_hash, &args);
        if(ret != 0){
            perror("pthread_create");
            shutdown();
            exit(-1);
        }
        pthread_attr_destroy(&pAttr);
    }
    initlizing = hashcat->isInitilizing();
    if(initlizing){
        std::string dot(timer, '.');
        ImGui::Text("Crack Environments Initializing%s", dot.c_str());
        timer = (timer + 1) % 7; 
    }
    if(!initlizing && !hashcat->deviceFound()){
        ImGui::TextColored(ImVec4(1.0f, 0, 0, 1.0f), "No devices found/left");
        timer = 6;
    }
    if(hashcat->isCracking()){
        auto guess_base = hashcat->get_guess_base();
        guess_base_offset = guess_base.first;
        guess_base_count = guess_base.second;
        float guess_base_progress = (1.0f * guess_base.first) / guess_base.second;
        char buf[32];
        sprintf(buf, "%d/%d", guess_base.first, guess_base.second);
        ImGui::Text("Crack Level Progress:");
        ImGui::ProgressBar(guess_base_progress, ImVec2(0.0f, 0.0f), buf);
        progress = hashcat->get_progress_percent() / 100;
        ImGui::Text("Crack Progress:");
        ImGui::ProgressBar(progress);
    }
    if((guess_base_offset == guess_base_count && ((int) progress) == 1) || hashcat->isCracked()){
        crack_finished = true;
        if(hashcat->isCracked())
            crack_successed = true;
    }
    ImGui::End();
}

void CrackGUIShader:: showPasswordCrackResults(){
    if(!ImGui::Begin("Password Display Board")){
        ImGui::End();
        return;
    }
    // 等待 hashcat 将结果写回
    if(timer < 1100){
        std::string dot(timer % 7, '.');
        ImGui::Text("Waiting Results Write-back%s", dot.c_str());
        timer++;
    }
    if(timer >= 1100) {
        if(crack_successed){
            std::string cracked_password = hashcat->getPassword();
            ImGui::Text("Cracked Password: ");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", cracked_password.c_str());
        }
        else
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", "Cracked Failed!");
    }
    ImGui::End();
}


void CrackGUIShader::shader(){
    while (!glfwWindowShouldClose(window)){
        shaderBegin();
        setNextWindowFormat();
        if(!input_complete)
            showPasswordInput();
        setNextWindowFormat();
        if(input_complete && !crack_finished)
            showPasswordCrackProgress();
        setNextWindowFormat();
        if(input_complete && crack_finished)
            showPasswordCrackResults();
        shaderEnd();
    }
}

void CrackGUIShader::setNextWindowFormat(){
    double dcu_occupy = (170.0f / 720.0f);
    double ident_x = 0.05f;
    double ident_y = 0.025f;
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