/*
TrigaLog is a software for GNU operating system to get the real-time values of the
Nuclear Reator Triga IPR-R1 from the TrigaSever software and storage in hard-drive.
Copyright (C) 2024 Thalles Campagnani

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
//main.cpp
#include "TrigaLog.h"
#include <cxxopts.hpp>
#include <fstream>
#include <string>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
//#include <unistd.h>
//#include <cstring>
//#include <sstream>
#include <iomanip>

void showVersion()
{
    std::cout << "Development Version" << std::endl;
}

void showLicense()
{
    std::cout << "TrigaLog        Copyright (C) 2023-2024       Thalles Campagnani" << std::endl;
    std::cout << "This    program    comes    with    ABSOLUTELY    NO   WARRANTY;" << std::endl;
    std::cout << "This is free software,    and you are welcome to redistribute it" << std::endl;
    std::cout << "under certain conditions; For more details read the file LICENSE" << std::endl;
    std::cout << "that came together with the source code." << std::endl << std::endl;
}

struct CONFIG
{
    std::string server_ip   = "127.0.0.1"; //CLP IP
    short       server_port = 1234; //Port of server
    std::string log_path    = "TrigaLog/"; //Path of the log files
    int         amo         = 10;
};

CONFIG readConfigFile(std::string filename)
{
    CONFIG config;
    
    std::ifstream configFile(filename);
    
    if (!configFile.is_open()) 
    {
        std::cerr << "[readConfigFile] Erro ao abrir o arquivo de configuração: " << filename << std::endl;
        filename = "TrigaLog.conf";
        std::ifstream configFile(filename);
        std::cerr << "[readConfigFile] Tentando abrir: " << filename << std::endl;
        if (!configFile.is_open()) 
        {
            std::cerr << "[readConfigFile] Erro ao abrir o arquivo de configuração: " << filename << std::endl;
            std::cerr << "[readConfigFile] Configurações setadas como padrão" << std::endl;
            return config;
        }
    }

    std::string line;
    while (std::getline(configFile, line)) {
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            // Remova espaços em branco extras
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);

            if (key == "server_ip") {
                config.server_ip = value.c_str();
            } else if (key == "server_port") {
                config.server_port = std::stoi(value);
            } else if (key == "log_path") {
                config.log_path = value.c_str();
            } else if (key == "amo") {
                config.amo = std::stoi(value);
            }
        }
    }

    configFile.close();
    return config;

}

std::string genFileNamePath(std::string path)
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_c), "%Y_%m_%d_%H_%M_%S");
    std::string timestamp = ss.str();

    return path + "TrigaLog_" + timestamp + ".log";
}

int main(int argc, char* argv[])
{
    std::string filename = "/etc/TrigaLog.conf";
    cxxopts::Options options("TrigaLog","TrigaLog is a software for GNU operating system to get the real-time values of the Nuclear Reator Triga IPR-R1 from the TrigaSever software and storage in hard-drive.");

    options.add_options()
        ("v,version", "Show the program version")
        ("h,help", "Show this help message")
        ("l,license", "Show info of the license");

    auto result = options.parse(argc, argv);

    if (argc >1)
    {
        if (result.count("version") || result.count("v")){
            showVersion();
            return 0;
        } else if (result.count("help") || result.count("h")){
            std::cout << options.help() << std::endl;
            return 0;
        } else if (result.count("license") || result.count("l")) {
            showLicense();
            return 0;
        } else if (result.count("config") || result.count("c")){
            if (result["config"].as<std::string>().empty()) {
                std::cerr << "Error: Please provide a filename for --config option." << std::endl;
                return 1;
            }
            filename = result["config"].as<std::string>();
        } else {
            std::cout << options.help() << std::endl;
            return 1;
        }
    }


    CONFIG config   = readConfigFile(filename);
    config.log_path = genFileNamePath(config.log_path);

    TrigaLog server(config.server_ip,config.server_port,config.log_path);
    server.startRead(config.amo);

    return 0;
}
