#ifndef _CRACK_HASH_H_
#define _CRACK_HASH_H_
/*
 * 对 hashcat 破解哈希值的整个过程的封装
 */
#include <string>
#include <utility>
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
    ~CrackHash();
    void crack(const char *hash_value, CRACK_ALGO_TYPE type);
    std::string get_hashcat_status_string();
    std::string getPassword();
    bool isInitilizing();
    bool isCracking();
    bool isCracked();
    bool deviceFound();
    std::pair<int, int> get_guess_base();
    double get_progress_percent();

protected:
    void init();
    void destroy();
    void init_hashcat_options(const char *hash_value, CRACK_ALGO_TYPE type);
private:
    hashcat_ctx_t *hashcat_ctx = nullptr;
    const char *install_folder = NULL;
    const char *shared_folder = NULL;
    std::string result_file_names;
    int argc;
    char **argv;
    bool m_cracked = false;
};
#endif