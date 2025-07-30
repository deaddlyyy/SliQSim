#ifndef __STDIN_DECODER_H__
#define __STDIN_DECODER_H__

#include <iostream>
#include <string>
#include <vector>
#include "threadpool.h"
#include <memory>
#include <sys/select.h>
#include <fstream>
#include <mutex>

extern std::ofstream debug_file;
extern std::mutex debug_file_mutex;

class StdinDecoder{
    public:
        StdinDecoder(std::shared_ptr<ThreadPool> pool);
        void decode(std::string config);
        void loop();
        std::string add_newlines(std::string qasm);

    private:
        std::string current_config;
        std::shared_ptr<ThreadPool> pool;
        bool terminate;
};

#endif