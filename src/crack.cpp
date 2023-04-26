#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "types.h"
#include "user_options.h"
#include "usage.h"
#include "memory.h"
#include "hashcat.h"
#include "terminal.h"
#include "thread.h"
#include "status.h"
#include "shared.h"
#include "event.h"
#include "hashcat_main.h"
#include "crack.h"
#include <assert.h>
#include <iostream>
#include <fstream>
#ifdef WITH_BRAIN
#include "brain.h"
#endif

#if defined (__MINGW64__) || defined (__MINGW32__)
int _dowildcard = -1;
#endif

CrackHash::CrackHash(const std::string &filename) : result_file_names(filename.c_str()){
    init();
}

CrackHash::~CrackHash(){
    destroy();
}

void CrackHash::init(){
    hashcat_ctx = (hashcat_ctx_t *) hcmalloc (sizeof (hashcat_ctx_t));
    if (hashcat_init (hashcat_ctx, event) == -1) {
        std::cerr << "hashcat_init error(file = " << __FILE__ << ", line = " << __LINE__ << ")\n";
    }
#if defined (INSTALL_FOLDER)
    install_folder = INSTALL_FOLDER;
#endif

#if defined (SHARED_FOLDER)
    shared_folder = SHARED_FOLDER;
#endif
    if (user_options_init (hashcat_ctx) == -1) {
        std::cerr << "user_options_init (file = " << __FILE__ << ", line = " << __LINE__ << ")\n";
    }
}

void CrackHash::destroy(){
    hashcat_session_destroy (hashcat_ctx);
    hashcat_destroy (hashcat_ctx);
    hcfree (hashcat_ctx);
}



std::string CrackHash::getPassword(){
    if(!m_cracked){
        return "Cracked Failed!"; 
    }
    std::ifstream result_file(result_file_names.c_str());
    if(!result_file.is_open()){
        std::cerr << result_file_names << " file not opened!\n";
    }
    std::string crack_value;
    result_file >> crack_value;
    return crack_value;
}


bool CrackHash::isCracking(){
    std::string status(get_hashcat_status_string());
    return status == "Running";
}
bool CrackHash::isInitilizing(){
    int device_status = hashcat_ctx->status_ctx->devices_status;
    return device_status == STATUS_INIT;
}

bool CrackHash::deviceFound(){
    int cnt = hashcat_ctx->backend_ctx->backend_devices_active;
    return cnt != 0;
}

std::string CrackHash::get_hashcat_status_string(){
    const status_ctx_t *status_ctx = hashcat_ctx->status_ctx;
    assert(status_ctx != nullptr);
    const int devices_status = status_ctx->devices_status;

    switch (devices_status){
        case STATUS_INIT:               return "Initializing";
        case STATUS_AUTOTUNE:           return "Autotuning";
        case STATUS_SELFTEST:           return "Selftest";
        case STATUS_RUNNING:            return "Running";
        case STATUS_PAUSED:             return "Paused";
        case STATUS_EXHAUSTED:          return "Exhausted";
        case STATUS_CRACKED:            return "Cracked";
        case STATUS_ABORTED:            return "Aborted";
        case STATUS_QUIT:               return "Quit";
        case STATUS_BYPASS:             return "Bypass";
        case STATUS_ABORTED_CHECKPOINT: return "Aborted (Checkpoint)";
        case STATUS_ABORTED_RUNTIME:    return "Aborted (Runtime)";
        case STATUS_ERROR:              return "Error";
        case STATUS_ABORTED_FINISH:     return "Aborted (Finish)";
        case STATUS_AUTODETECT:         return "Autodetect";
  }
  return "Unknown! Bug!";
}

bool CrackHash::isCracked(){
    return m_cracked;
}
 bool CrackHash::isExhausted(){
    std::string status(get_hashcat_status_string());
    return status == "Exhausted";
 }
std::pair<int, int> CrackHash::get_guess_base(){
    int guess_base_offset = status_get_guess_base_offset(hashcat_ctx);
    int guess_base_count = status_get_guess_base_count(hashcat_ctx);
    return {guess_base_offset, guess_base_count};
}

 double CrackHash::get_progress_percent(){
    return status_get_progress_finished_percent(hashcat_ctx);
 }

std::string CrackHash::get_backend_device_speed(){
    const backend_ctx_t *backend_ctx = hashcat_ctx->backend_ctx;
    int device_cnt = backend_ctx->backend_devices_cnt;
    double speed = 0.0f;
    for(int cnt = 0; cnt < device_cnt; ++cnt){
        speed = status_get_hashes_msec_dev(hashcat_ctx, cnt);
        if(speed != 0)
            break;
    }
    char speed_string[40];
    format_speed_display (speed * 1000, speed_string, sizeof(speed_string));
    return {speed_string};
}

DCUCrackHash::DCUCrackHash(const std::string &filename) : CrackHash(filename){

}

IntelCPUCrackHash::IntelCPUCrackHash(const std::string &filename) : CrackHash(filename){
    
}

 void DCUCrackHash::init_hashcat_options(const char *hash_value, CRACK_ALGO_TYPE type){
    const char *hash_mode;
    if(type == CRACK_ALGO_TYPE_MD5)
        hash_mode = "0";
    else if(type == CRACK_ALGO_TYPE_SHA1)
        hash_mode = "100";
    else if(type = CRACK_ALGO_TYPE_SHA256)
        hash_mode = "1400";
    else{
        std::cerr << "Algorithm (" << type << ") not support!\n";
        exit(-1);
    }
    const char *filename = result_file_names.c_str();
    static const char *argv_temp_1[] = {
        "./crack_hash",
        "-a",
        "3",                                // 掩码攻击
        "-m",
        hash_mode,                          // 要破解的算法
        "--session=DCU",
        "-o",
        filename,                           // 保存破解结果的文件
        "--potfile-disable",                // disable potfile
        "--outfile-format=2",               // 破解哈希值的格式
        "-D",
        "2",                                // 使用 GPU
        "--increment",
        "--increment-min=6",                // 密码最小长度 6
        "--increment-max=12",               // 密码最大长度 12
        hash_value,                         // 要破解的哈希值
        "?l?l?l?l?l?l?l?l?l?l?l?l"
    };
    argc = sizeof(argv_temp_1) / sizeof(char *);
    argv = (char **)argv_temp_1;
    if (user_options_getopt (hashcat_ctx, argc, argv) == -1){
        user_options_destroy (hashcat_ctx);
        std::cerr << "user_options_getopt (file = " << __FILE__ << ", line = " << __LINE__ << ")\n";
    }
    if (user_options_sanity (hashcat_ctx) == -1){
        user_options_destroy (hashcat_ctx);
        std::cerr << "user_options_sanity (file = " << __FILE__ << ", line = " << __LINE__ << ")\n";
    }
}

void IntelCPUCrackHash::init_hashcat_options(const char *hash_value, CRACK_ALGO_TYPE type){
    const char *hash_mode;
    if(type == CRACK_ALGO_TYPE_MD5)
        hash_mode = "0";
    else if(type == CRACK_ALGO_TYPE_SHA1)
        hash_mode = "100";
    else if(type = CRACK_ALGO_TYPE_SHA256)
        hash_mode = "1400";
    else{
        std::cerr << "Algorithm (" << type << ") not support!\n";
        exit(-1);
    }
    const char *filename = result_file_names.c_str();
    static const char *argv_temp_2[] = {
        "./crack_hash",
        "-a",
        "3",                                // 掩码攻击
        "-m",
        hash_mode,                          // 要破解的算法
        "--session=IntelCPU",
        "-o",
        filename,                           // 保存破解结果的文件
        "--potfile-disable",                // disable potfile
        "--outfile-format=2",               // 破解哈希值的格式
        "-D",
        "1",                                // 使用 CPU
        "--increment",
        "--increment-min=6",                // 密码最小长度 6
        "--increment-max=12",               // 密码最大长度 12
        hash_value,                         // 要破解的哈希值
        "?l?l?l?l?l?l?l?l?l?l?l?l"
    };
    argc = sizeof(argv_temp_2) / sizeof(char *);
    argv = (char **)argv_temp_2;
    if (user_options_getopt (hashcat_ctx, argc, argv) == -1){
        user_options_destroy (hashcat_ctx);
        std::cerr << "user_options_getopt (file = " << __FILE__ << ", line = " << __LINE__ << ")\n";
    }
    if (user_options_sanity (hashcat_ctx) == -1){
        user_options_destroy (hashcat_ctx);
        std::cerr << "user_options_sanity (file = " << __FILE__ << ", line = " << __LINE__ << ")\n";
    }
}

void DCUCrackHash::crack(const char *hash_value, CRACK_ALGO_TYPE type){
    m_cracked = false;
    if(hashcat_ctx == nullptr){
        std::cerr << "hashcat_ctx_t does't malloced\n";
    }
    init_hashcat_options(hash_value, type);
    int rc_final = -1;
    if (hashcat_session_init (hashcat_ctx, install_folder, shared_folder, argc, argv, COMPTIME) == 0){
        backend_info_compact (hashcat_ctx);
        user_options_info (hashcat_ctx);
        rc_final = hashcat_session_execute (hashcat_ctx);
    }
    if(rc_final == -1)
        m_cracked = false;
    else
        m_cracked = true;
}

void IntelCPUCrackHash::crack(const char *hash_value, CRACK_ALGO_TYPE type){
    m_cracked = false;
    if(hashcat_ctx == nullptr){
        std::cerr << "hashcat_ctx_t does't malloced\n";
    }
    init_hashcat_options(hash_value, type);
    int rc_final = -1;
    if (hashcat_session_init (hashcat_ctx, install_folder, shared_folder, argc, argv, COMPTIME) == 0){
        backend_info_compact (hashcat_ctx);
        user_options_info (hashcat_ctx);
        rc_final = hashcat_session_execute (hashcat_ctx);
    }
    if(rc_final == -1)
        m_cracked = false;
    else
        m_cracked = true;
}