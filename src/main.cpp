#include "crack_gui.h"

int main(){
    CrackGUIShader *gui = new CrackGUIShader;
    gui->shader();
    delete gui;
    return 0;
}