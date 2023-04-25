#ifndef _CRACK_GUI_H_
#define _CRACK_GUI_H_
#include "imgui.h"
#include "crack.h"
#include <GLFW/glfw3.h>
#include <string>
class Shader {
public:
    unsigned int ID;
    unsigned int VAO, VBO, EBO;
    unsigned int texture;
    Shader(const char* vertexPath, const char* fragmentPath);
    void use();
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void initArgument(const char *imagePath);
    void loadTexture(unsigned char *data, int width, int height);
    void release();
    ~Shader();
};

class CrackGUIShader{
public:
    CrackGUIShader(const char *result_save_path = "result.txt");
    ~CrackGUIShader();
    void shader();
protected:
    void init();
    void shutdown();
    void shaderBegin();
    void shaderEnd();
    void showPasswordInput();
    void showPasswordCrackProgress();
    void showPasswordCrackResults();
    void setNextWindowFormat();
public:
    GLFWwindow *window;
    CrackHash *hashcat;
    // input windows control variables
    bool buttoned = false;
    bool hash_correct_input = true;
    bool password_correct_input = true;
    bool input_complete = false;    // 指示输入窗口的所有工作是否完成
    char password[16] = {'p', 'a', 's', 's', 'w', 'o', 'r', 'd'};
    char re_password[16] = {'p', 'a', 's', 's', 'w', 'o', 'r', 'd'};
    char hash_input[128] = {};
    int crack_mode = 1;             // 默认破解模式是输入哈希值进行破解
    int hash_algo = 1;              // 默认的哈希算法是 MD5 算法
    CRACK_ALGO_TYPE type;

    // crack process windows control variables
    int timer = 0;                      // hashcat 的启动需要时间，在这个过程中需要等待一段时间
    std::string hash_value;
    bool initlizing = true;
    bool hashcat_crack_started = false; // 标志着 hashcat 的启动与否，毕竟只需要运行一次 hashcat 进行破解
    bool crack_finished = false;
    bool crack_successed = false;       // 用于指示 hashcat 破解该哈希值是否成功
    int guess_base_offset = 0;          // 当前破解的阶段
    int guess_base_count = 1;           // 破解该哈希密码所需要总的阶段
    double progress = 0;                // 当前阶段所处的进度
    // crack result display windows control variables


    // 窗口属性
    int glfw_window_width = 1280;
    int glfw_window_height = 720;
    // 背景渲染工具
    Shader *p_shader;
};



#endif // _CRACK_GUI_H_