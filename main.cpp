/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: jocke
 *
 * Created on den 21 februari 2016, 12:00
 */

#include <cstdlib>
#include <iostream>
#include <thread>
#include <fstream>
#include <regex>
#include <vector>
#include <string>
#include <sstream>
#include <chrono>
#include <map>



using namespace std;

/* Memorybank :)
 * regex numbers("[0-9]+");
 * regex letters("([a-z,A-Z]+)");
 * vector = "array of strings" 
 * list = "vector of ints"
 * array = array of ints.
 */

class Helpers {
public:
    string tokenize(string str, int pos = 1) {
        string buf;
        stringstream ss(str);
        vector<string> tokens;
        while (ss >> buf) {
            tokens.push_back(buf);
        }
        return tokens[pos];
    }
    string get_hostname_from_file() {
        ifstream fil;
        string hostname;
        fil.open("/etc/hostname");
        if (fil.is_open()) {
            while (!fil.eof()) {
                fil >> hostname;
            }
            fil.close();
        } else {
            hostname = "Err..";
        }
        return hostname;
    }
    string get_env_hostname() {
        string hostname = getenv("HOSTNAME");
        return hostname;
    }
    unsigned long int get_epoch(string type = "seconds") {
        regex seconds("^[sS][eE][cC].*");
        chrono::system_clock::time_point tp = chrono::system_clock::now();
        chrono::system_clock::duration dura = tp.time_since_epoch();
        unsigned long int returner;
        if (regex_match(type, seconds)) {
            returner = dura.count() / 1000000000;
        } else {
            returner = dura.count();
        }
        return returner;
    }
    string find_config_key(string config_key, string config_file = "/etc/janitor/config.cfg") {
         string s, key, value, returner = "NULL";
         ifstream file;
         regex remove_comment("(^[#; ]|($)).*");
         file.open(config_file);
         if (file.is_open()) {
             while (!file.eof()) {
                 getline(file, s);
                 if (!regex_match(s, remove_comment)) {
                     key = s.substr(0, s.find("="));
                     value = s.substr(s.find("=") + 1);
                     if ( key == config_key ) {
                         returner = value;
                     }
                 }
             }
             file.close();
         }
         return returner;
     }
    int read_config_to_map(map<string, string>& config_map, string config_path = "/etc/janitor/config.cfg") {
        string s, k, v;
        ifstream config_file;
        regex remove_comment("(^[#; ]|($)).*");
        config_file.open(config_path);
        if (config_file.is_open()) {
            while (!config_file.eof()) {
                getline(config_file, s);
                if (!regex_match(s, remove_comment)) {
                    k = s.substr(0, s.find("="));
                    v = s.substr(s.find("=") + 1);
                    config_map[k] = v;
                }
            }
        } else {
            return 1;
        }
        return 0;
    }
    void wait_for(unsigned int seconds) {
        clock_t time_end;
        time_end = clock() + seconds * CLOCKS_PER_SEC;
        while (clock() < time_end) {
            
        }
    }
};


class Proc{
public:
    string memory_info(string reggy) {
        regex expression(reggy);
        string line_check;
        string memory_info;
        ifstream fil;
        fil.open("/proc/meminfo");
        if (fil.is_open()) {
            while (!fil.eof()) {
                getline(fil, line_check);
                if (regex_match(line_check, expression)) {
                    memory_info = line_check;
                }
            }
        }
        fil.close();
        return memory_info;
    }
    int avg_cpu_usage() {
        regex expression("^cpu.MHz.*");
        regex numbers("[0-9]+");
        int num_of_cores = 0;
        int sum_of_mhz;
        vector<string> cpu_lines;
        vector<float> cpu_stats;
        string line_check, a, b, c;
        float d;
        int avg_mhz, returner;
        ifstream cpuinfo;
        cpuinfo.open("/proc/cpuinfo");
        if (cpuinfo.is_open()) {
            while (!cpuinfo.eof()) {
                getline(cpuinfo, line_check);
                if (regex_match(line_check, expression)) {
                    cpu_lines.push_back(line_check);
                }
                num_of_cores = cpu_lines.size();
            }
            // remove junk from cpu_lines and add to cpu_stats
            while (cpu_lines.size() > 0 ) {
                istringstream iss(cpu_lines.back());
                iss >> a >> b >> c >> d;
                cpu_stats.push_back(d);
                cpu_lines.pop_back();
            }
            if ( num_of_cores > 0 ) {
                sum_of_mhz = accumulate(cpu_stats.begin(), cpu_stats.end(), 0);
                avg_mhz = sum_of_mhz / num_of_cores;
            } 
        }
        returner = avg_mhz;
        return returner;
    }
    string load_avg(string what = "all") {
        regex special("([.])");
        ifstream fil;
        stringstream ftfss;
        string line_check, five, ten, fifteen, ftfs, junk_a, junk_b, returner;
        fil.open("/proc/loadavg");
        if (fil.is_open()) {
            while (!fil.eof()) {
                getline(fil, line_check);
                line_check = regex_replace(line_check, special, "");
                istringstream iss(line_check);
                // Split linecheck to 5 different strings
                iss >> five >> ten >> fifteen >> junk_a >> junk_b;
            }
            if ( what == "all" ) {
                ftfss << five << " " << ten << " " << fifteen;
                ftfs = ftfss.str();
                returner = ftfs;
            } else if ( what == "five" ) {
                returner = five;
            } else if ( what == "ten" ) {
                returner = ten;
            } else if ( what == "fifteen" ) {
                returner = fifteen;
            }
        }
        return returner;
    }
    string network_status(string interface, string what = "all") {
        string base_path = "/sys/class/net/", rx, tx, rx_tx;
        ifstream netinfo, net_rx, net_tx;
        regex numbers("[0-9]+");
        net_rx.open(base_path + interface + "/statistics/rx_packets");
        net_tx.open(base_path + interface + "/statistics/tx_packets");
        if (net_rx.is_open() && net_tx.is_open()) {
            while (!net_rx.eof()) {
                getline(net_rx, rx);
                if (regex_match(rx, numbers)) {
                    break;
                }
            }
            while (!net_tx.eof()) {
                getline(net_tx, tx);
                if (regex_match(tx, numbers)) {
                    break;
                }
            }
            rx_tx = rx + " " + tx;
        } else {
            rx_tx = "Err";
        }
        return rx_tx;
    }
};

    
int main(int argc, char** argv) {
    // Class objects
    Proc Proc;
    Helpers Help;
    
    // regex
    regex reg_numbers("[0-9]+");
    
    // All Vectors to be used
    //vector<string> arguments;
    vector<string> plugins_save, plugins_run, used, results;
    //vector<string> used;
    //vector<string> results;
    
    // Bools
    bool daemon = false;
    
    // ints
    int refresh = 10;
    
    // Maps
    map<string, string> config_map;
    
    // More static stuff - move this?
    string hostname = Help.get_env_hostname();
    string buf_plugins, interface;
            
    // Config the string for "GetEpoch" in a configfile: TODO
    unsigned long int current_time = Help.get_epoch();
    
    // Are there any CLI-args?
    if (argv[1] == "-h" || argv[1] == "--help") {
        cout << "====== Need help? =====" << endl;
        cout << "Use any combination of below arguments" << endl;
        cout << argv[0] << " MemFree|MemTotal|CpuAvg|LoadAvgFive|LoadAvgTen|LoadAvgFifteen" << endl;
        return 0;
    } else if (argv[1] == "-s" || argv[1] == "--service") {
        bool daemon = true;
    }
    
    // Add all arguments to the vector "arguments"
    /*
    if ( (string)argv[1] == "TryAll" ) {
        for (int i = 0; i < all.size(); i++) {
            arguments.push_back(all[i]);
        }
    } else { 
        for (int i = 1; i < argc; i++) {
            arguments.push_back(argv[i]);
        }
    }    
    !!! - OLD - Might not be used at all..
    if (Help.find_config_key("plugins") != "NULL") {
        string buf;
        stringstream ss(Help.find_config_key("plugins"));
        //vector<string> plugins;
        while (ss >> buf) {
            arguments.push_back(buf);
        }
    }*/
    // Read the configfile
    Help.read_config_to_map(config_map);
    
    // Check if the user wants this a service/daemon)
    if (config_map.at("daemon") == "true") {
        daemon = true;
    }
    // Set refreshrate from config-file
    if (!config_map.at("refresh").empty()) {
        string refresh_rate_pre_match = config_map.at("refresh");
        if (regex_match(refresh_rate_pre_match, reg_numbers)) {
            refresh = stoi(config_map.at("refresh"));
        }
    }
    
    // Set interface from config-file
    /*if (!config_map.at("interface").empty()) {
        interface = config_map.at("interface");
    } else {
        interface = "Empty";
    }*/
    
    // Put the configured plugins to the plugin vector
    stringstream ss_plugins(config_map.at("plugins"));
    while (ss_plugins >> buf_plugins) {
        plugins_save.push_back(buf_plugins);
    }
    plugins_run = plugins_save;
    
    
    // Magic
    if (daemon) {
        while (daemon) {
            while (plugins_run.size() >= 1) {
                if ( plugins_run.back() == "MemFree") {
                    results.push_back(Help.tokenize(Proc.memory_info("MemFree.*"), 1));
                    used.push_back(plugins_run.back());
                } else if ( plugins_run.back() == "MemTotal" ) {
                    results.push_back(Help.tokenize(Proc.memory_info("MemTotal.*"), 1));
                    used.push_back(plugins_run.back());
                } else if ( plugins_run.back() == "CpuAvgMhz" ) {
                    results.push_back(to_string(Proc.avg_cpu_usage()));
                    used.push_back(plugins_run.back());
                } else if ( plugins_run.back() == "LoadAvgFive" ) {
                    results.push_back(Proc.load_avg("five"));
                    used.push_back(plugins_run.back());
                } else if ( plugins_run.back() == "LoadAvgTen" ) {
                    results.push_back(Proc.load_avg("ten"));
                    used.push_back(plugins_run.back());
                } else if ( plugins_run.back() == "LoadAvgFifteen" ) {
                    results.push_back(Proc.load_avg("fifteen"));
                    used.push_back(plugins_run.back());
                } else if ( plugins_run.back() == "Network" && !config_map.at("interface").empty() && config_map.at("interface") != "Err") {
                    string tx_rx = Proc.network_status(config_map.at("interface"), "all");
                    used.push_back(config_map.at("interface") + "_tx");
                    results.push_back(Help.tokenize(tx_rx, 0));
                    used.push_back(config_map.at("interface") + "_rx");
                    results.push_back(Help.tokenize(tx_rx, 1));
                }
                plugins_run.pop_back();
            }
            
            // Print the result - I want this to be sent to a graphite-server
            while (results.size() != 0 ) {
                cout << hostname << "." 
                        << used.back()
                        <<" "
                        << results.back() 
                        << " "
                        << current_time
                        << endl;
                used.pop_back();
                results.pop_back();
            }
            Help.wait_for(refresh);
            plugins_run = plugins_save;
            current_time = Help.get_epoch();
        }
    } else {
        // Go through all the arguments one by one
        // Maybe move this to a funciton later on?
        while (plugins_run.size() >= 1) {
            if ( plugins_run.back() == "MemFree") {
                results.push_back(Help.tokenize(Proc.memory_info("MemFree.*"), 1));
                used.push_back(plugins_run.back());
            } else if ( plugins_run.back() == "MemTotal" ) {
                results.push_back(Help.tokenize(Proc.memory_info("MemTotal.*"), 1));
                used.push_back(plugins_run.back());
            } else if ( plugins_run.back() == "CpuAvgMhz" ) {
                results.push_back(to_string(Proc.avg_cpu_usage()));
                used.push_back(plugins_run.back());
            } else if ( plugins_run.back() == "LoadAvgFive" ) {
                results.push_back(Proc.load_avg("five"));
                used.push_back(plugins_run.back());
            } else if ( plugins_run.back() == "LoadAvgTen" ) {
                results.push_back(Proc.load_avg("ten"));
                used.push_back(plugins_run.back());
            } else if ( plugins_run.back() == "LoadAvgFifteen" ) {
                results.push_back(Proc.load_avg("fifteen"));
                used.push_back(plugins_run.back());
            }
            plugins_run.pop_back();
        }
        // Print them pretty in graphite-style
        while (results.size() != 0 ) {
            cout << hostname << "." 
                    << used.back()
                    <<" "
                    << results.back() 
                    << " "
                    << current_time
                    << endl;
            used.pop_back();
            results.pop_back();
        }
    }
    
    return 0;
}

