#ifndef _CRACK_HASH_H_
#define _CRACK_HASH_H_
/*
 * 对 hashcat 破解哈希值的整个过程的封装
 */
#include <string>
#include <utility>
#include <time.h>
#include "types.h"

enum CRACK_ALGO_TYPE{
    CRACK_ALGO_TYPE_MD5,
    CRACK_ALGO_TYPE_SHA1,
    CRACK_ALGO_TYPE_SHA256,
    CRACK_ALGO_TYPE_LAST
};


class CrackHash{
public:
    CrackHash(const std::string &filename = "result.txt");
    virtual ~CrackHash();
    virtual void crack(const char *hash_value, CRACK_ALGO_TYPE type) = 0;
    std::string get_hashcat_status_string();
    std::string getPassword();
    bool isInitilizing();
    bool isExhausted();
    bool isCracking();
    bool isCracked();
    bool deviceFound();
    std::pair<int, int> get_guess_base();
    double get_progress_percent();
    std::string get_backend_device_speed();
    std::string get_total_time_consume();
    time_t crack_begin;                         // crack 起始时间
    time_t crack_end;                           // crack 终止时间

protected:
    void init();
    void destroy();
protected:
    hashcat_ctx_t *hashcat_ctx = nullptr;
    const char *install_folder = NULL;
    const char *shared_folder = NULL;
    std::string result_file_names;
    int argc;
    char **argv;
    bool m_cracked = false;
};

class DCUCrackHash : public CrackHash{
public:
    DCUCrackHash(const std::string &filename = "dcu_result.txt");
    void crack(const char *hash_value, CRACK_ALGO_TYPE type);
    ~DCUCrackHash();
protected:
    void init_hashcat_options(const char *hash_value, CRACK_ALGO_TYPE type);
};

class IntelCPUCrackHash : public CrackHash{
public:
    IntelCPUCrackHash(const std::string &filename = "cpu_result.txt");
    ~IntelCPUCrackHash();
    void crack(const char *hash_value, CRACK_ALGO_TYPE type);
protected:
    void init_hashcat_options(const char *hash_value, CRACK_ALGO_TYPE type);
};
#endif