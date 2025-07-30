#include "stdin_decoder.h"
#include "rapidxml.h"

StdinDecoder::StdinDecoder(std::shared_ptr<ThreadPool> pool) : pool(pool) {
    this->terminate = false;
}

void StdinDecoder::decode(std::string config){
    rapidxml::xml_document<> doc;
    try{
        doc.parse<0>(config.data());
    }
    catch(...){
        return;
    }
    rapidxml::xml_node<>* root = doc.first_node();
    if (root == nullptr) {
        return;
    }
    std::string root_name(root->name());
    if(root_name != "xml"){
        return;
    }

    std::string qasm_file = "";
    std::string genome_id = "";
    for(auto curr = root->first_node(); curr != nullptr; curr = curr->next_sibling()){
        std::string node_name(curr->name());
        if(node_name == "flag"){
            std::string flag_value(curr->value());
            if(flag_value == "terminate"){
                this->terminate = true;
                return;
            }
        }
        else if(node_name == "qasm"){
            qasm_file = this->add_newlines(curr->value());
        }   
        else if(node_name == "id"){
            genome_id = curr->value();
        }
    }
    if(qasm_file != "" && genome_id != ""){
        this->pool->enqueue(Job(qasm_file, std::stoi(genome_id)));
    }
}

std::string StdinDecoder::add_newlines(std::string qasm){
    std::string new_qasm = "";
    for(char ch : qasm){
        new_qasm += ch;
        if(ch == ';'){
            new_qasm += "\n";
        }
    }
    return new_qasm;
}

void StdinDecoder::loop(){
    std::string current = "";
    std::string end = "</xml>";
    while(1){
        if(this->terminate){
            this->pool->terminate();
            return;
        }
        std::string line;
        while (std::getline(std::cin, line)) {
            current += line + "\n";     
            size_t pos = current.find(end);
            while(pos != std::string::npos){
                this->current_config += current.substr(0, pos + end.size());
                this->decode(this->current_config);
                if(this->terminate){
                    this->pool->terminate();
                    return;
                }
                this->current_config = "";
                current = current.substr(pos + end.size());
                pos = current.find(end);
            }
        }
    }
}