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
    void showPasswordCracked();
    void setNextWindowFormat();
public:
    GLFWwindow *window;
    CrackHash hashcat;
    char password[61] = {'p', 'a', 's', 's', 'w', 'o', 'r', 'd'};
    char re_password[61] = {'p', 'a', 's', 's', 'w', 'o', 'r', 'd'};
    std::string hash_value;
    CRACK_ALGO_TYPE type;
    bool password_entered = false;
    bool password_correct_input = true;
    bool hashcat_crack_started = false;
    bool password_cracked = false;
    int glfw_window_width = 1280;
    int glfw_window_height = 720;
    Shader *p_shader;
};



#endif // _CRACK_GUI_H_