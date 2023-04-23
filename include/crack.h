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
    std::string getPassword();
    bool isCracking();
    bool isCracked();
    std::pair<int, int> get_guess_base();

protected:
    void init();
    void destroy();
    void init_hashcat_options(const char *hash_value, CRACK_ALGO_TYPE type);
    void get_hashcat_status();
private:
    hashcat_ctx_t *hashcat_ctx;
    hashcat_status_t *hashcat_status;
    int progress_mode = PROGRESS_MODE_KEYSPACE_KNOWN;
    const char *install_folder = NULL;
    const char *shared_folder = NULL;
    std::string result_file_names;
    int argc;
    char **argv;
    bool m_cracked = false;
};
#endif