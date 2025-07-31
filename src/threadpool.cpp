#include "threadpool.h"
#include <fstream>
#include <regex>

void ThreadPool::terminate(){
    this->terminate_flag = true;
    this->cv.notify_all();
    for(auto& worker : this->workers){
        worker.join();
    }
}

Job ThreadPool::dequeue(){
    std::unique_lock<std::mutex> lock(this->queue_mutex);
    while (1) {
        this->cv.wait(lock, [this]() {
            return !this->jobs.empty() || this->terminate_flag;
        });

        if (!this->jobs.empty()) {
            Job job = this->jobs.front();
            this->jobs.pop();
            return job;
        }

        if (this->terminate_flag && this->jobs.empty()) {
            return Job("", -1);
        }
    }
}


void ThreadPool::print_result(Job result){
    std::unique_lock<std::mutex> lock(this->print_mutex);
    result.print();
}

ThreadPool::ThreadPool(int number_of_threads){
    this->terminate_flag = false;
    for(int i = 0; i < number_of_threads; i++){
        this->workers.emplace_back([this](){
            while (true) {
                Job job = this->dequeue();
                if(job.get_id() == -1){
                    return;
                }
                else{
                    std::tuple<std::string, std::string, std::string, std::string> statevectors = this->sim_evo(job.get_file());
                    this->print_result(Job(std::get<0>(statevectors), std::get<1>(statevectors), std::get<2>(statevectors), std::get<3>(statevectors), job.get_id()));
                }
            }
        });
    }
}

void ThreadPool::enqueue(Job job){
    std::unique_lock<std::mutex> lock(this->queue_mutex);
    this->jobs.push(job);
    this->cv.notify_one();
}

std::tuple<std::string, std::string, std::string, std::string> ThreadPool::sim_evo(std::string qasm){
    int shots = 1;
    int type = 1;
    int r = 32;
    std::random_device rd;
    int seed = rd();
    Simulator simulator_0(type, shots, seed, r, false, false, false);
    Simulator simulator_1(type, shots, seed, r, false, false, false);
    Simulator simulator_plus(type, shots, seed, r, false, false, false);
    Simulator simulator_minus(type, shots, seed, r, false, false, false);
    int qubits = this->get_qubits(qasm);
    std::string basis_state_0 = "";
    std::string basis_state_1 = "";
    std::string basis_state_plus = "";
    std::string basis_state_minus = "";
    for(int i = 0; i < qubits-1; i++){
        basis_state_0 += "0";
        basis_state_1 += "0";
        basis_state_plus += "0";
        basis_state_minus += "0";
    }
    basis_state_0 += "0";
    basis_state_1 += "1";
    basis_state_plus += "+";
    basis_state_minus += "-";
    simulator_0.sim_qasm_file(qasm, basis_state_0);
    simulator_0.getStatevector();
    std::string statevector_0 = simulator_0.get_state_vector();
    simulator_1.sim_qasm_file(qasm, basis_state_1);
    simulator_1.getStatevector();
    std::string statevector_1 = simulator_1.get_state_vector();
    simulator_plus.sim_qasm_file(qasm, basis_state_plus);
    simulator_plus.getStatevector();
    std::string statevector_plus = simulator_plus.get_state_vector();
    simulator_minus.sim_qasm_file(qasm, basis_state_minus);
    simulator_minus.getStatevector();
    std::string statevector_minus = simulator_minus.get_state_vector();
    return std::make_tuple(statevector_0, statevector_1, statevector_plus, statevector_minus);
}

int ThreadPool::get_qubits(std::string qasm){
    std::regex qreg_regex(R"(qreg q\[(\d+)\])");
    std::smatch match;
    if(std::regex_search(qasm, match, qreg_regex)){
        return std::stoi(match[1].str());
    }
    else{
        return 0;
    }
}






Job::Job(std::string qasm_file, int genome_id){
    this->qasm_file = qasm_file;
    this->genome_id = genome_id;
}

Job::Job(std::string statevector_0, std::string statevector_1, std::string statevector_plus, std::string statevector_minus, int genome_id){
    this->statevector_0 = statevector_0;
    this->statevector_1 = statevector_1;    
    this->statevector_plus = statevector_plus;
    this->statevector_minus = statevector_minus;
    this->genome_id = genome_id;
}

std::string Job::to_xml(){
    std::string xml = "<xml><id>" + std::to_string(this->genome_id) + "</id>";
    xml += "<statevector_0>" + this->statevector_0 + "</statevector_0>";
    xml += "<statevector_1>" + this->statevector_1 + "</statevector_1>";
    xml += "<statevector_plus>" + this->statevector_plus + "</statevector_plus>";
    xml += "<statevector_minus>" + this->statevector_minus + "</statevector_minus>";
    xml += "</xml>";
    return xml;
}

void Job::print(){
    std::cout << this->to_xml() << std::endl;
}

std::string Job::get_file(){
    return this->qasm_file;
}

int Job::get_id(){
    return this->genome_id;
}