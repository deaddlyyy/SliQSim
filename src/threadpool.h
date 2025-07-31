#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <functional>
#include <chrono>
#include "Simulator.h"
#include <atomic>
#include <condition_variable>
#include <fstream>
#include <tuple>

extern std::ofstream debug_file;
extern std::mutex debug_file_mutex;

class Job{
    public:
        Job(std::string qasm_file, int genome_id);
        Job(std::string statevector_0, std::string statevector_1, std::string statevector_plus, std::string statevector_minus, int genome_id);
        std::string to_xml();
        void print();
        std::string get_file();
        int get_id();
    private:

        std::string qasm_file;
        int genome_id;
        std::string statevector_0;
        std::string statevector_1;
        std::string statevector_plus;
        std::string statevector_minus;
};

class ThreadPool{
    public:
        ThreadPool(int number_of_threads);
        void enqueue(Job job);
        void print_result(Job result);
        Job dequeue();
        void terminate();

    private:
        int get_qubits(std::string qasm);
        std::tuple<std::string, std::string, std::string, std::string> sim_evo(std::string qasm);

        std::vector<std::thread> workers;
        std::queue<Job> jobs;
        std::mutex queue_mutex;
        std::condition_variable cv;
        std::atomic<bool> terminate_flag;
        std::mutex print_mutex;
};


#endif