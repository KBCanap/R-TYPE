/*
** EPITECH PROJECT, 2025
** main.cpp
** File description:
** Entry point for R-Type lobby server with multi-instance game management
*/

#include "StartServer.hpp"
#include <cstring>
#include <iostream>
#include <map>
#include <string>

struct ServerConfig {
    int tcp_port;
    int base_udp_port;
    bool valid;
};

static void show_helper() {
    std::cout << "USAGE:" << std::endl;
    std::cout << "./r-type_server [-p <tcp_port>] [-u <base_udp_port>]" << std::endl;
    std::cout << "\nDESCRIPTION:" << std::endl;
    std::cout << "  Starts a lobby server that manages multiple game instances." << std::endl;
    std::cout << "  Each game instance will use a unique UDP port starting from base_udp_port." << std::endl;
    std::cout << "\nOPTIONS:" << std::endl;
    std::cout << "  -p <port>    TCP port number for lobby server (1024-65535)" << std::endl;
    std::cout << "  -u <port>    Base UDP port number for game instances (1024-65535)" << std::endl;
    std::cout << "  -h           Show this help message" << std::endl;
    std::cout << "\nEXAMPLE:" << std::endl;
    std::cout << "  ./r-type_server -p 8000 -u 8080" << std::endl;
    std::cout << "  This will:" << std::endl;
    std::cout << "    - Start lobby server on TCP port 8000" << std::endl;
    std::cout << "    - Allocate game instances starting at UDP port 8080, 8081, 8082..." << std::endl;
}

static bool is_a_valid_port(const char *str) {
    if (str == nullptr || *str == '\0')
        return false;

    for (int i = 0; str[i]; ++i) {
        if (!std::isdigit(str[i]))
            return false;
    }

    int port = std::atoi(str);
    return port >= 1024 && port <= 65535;
}

static ServerConfig parse_arguments(int ac, char **av) {
    ServerConfig config = {0, 0, false};

    if (ac == 1 || (ac == 2 && std::strcmp("-h", av[1]) == 0)) {
        show_helper();
        return config;
    }

    std::map<std::string, int> ports;

    for (int i = 1; i < ac; i++) {
        if (std::strcmp(av[i], "-p") == 0) {
            if (i + 1 >= ac || !is_a_valid_port(av[i + 1])) {
                std::cerr << "Error: Invalid or missing TCP port after -p flag" << std::endl;
                show_helper();
                return config;
            }
            ports["-p"] = std::atoi(av[i + 1]);
            i++;
        } else if (std::strcmp(av[i], "-u") == 0) {
            if (i + 1 >= ac || !is_a_valid_port(av[i + 1])) {
                std::cerr << "Error: Invalid or missing base UDP port after -u flag"
                          << std::endl;
                show_helper();
                return config;
            }
            ports["-u"] = std::atoi(av[i + 1]);
            i++;
        } else if (std::strcmp(av[i], "-h") == 0) {
            show_helper();
            return config;
        } else {
            std::cerr << "Error: Unknown flag '" << av[i] << "'" << std::endl;
            show_helper();
            return config;
        }
    }

    if (ports.find("-p") == ports.end() || ports.find("-u") == ports.end()) {
        std::cerr << "Error: Both -p and -u flags are required" << std::endl;
        show_helper();
        return config;
    }

    config.tcp_port = ports["-p"];
    config.base_udp_port = ports["-u"];
    config.valid = true;

    return config;
}

int main(int ac, char **av) {
    ServerConfig config = parse_arguments(ac, av);

    if (!config.valid) {
        return (ac == 1 || (ac == 2 && std::strcmp("-h", av[1]) == 0)) ? 0 : 84;
    }

    std::cout << "\nConfiguration:" << std::endl;
    std::cout << "  TCP Port (Lobby):  " << config.tcp_port << std::endl;
    std::cout << "  UDP Port (Base):   " << config.base_udp_port << std::endl;
    std::cout << "\nPress Ctrl+C to stop the server\n" << std::endl;

    try {
        StartServer server(config.tcp_port, config.base_udp_port);
        
        server.run();
        
        std::cout << "\nServer stopped gracefully." << std::endl;
        
    } catch (const std::exception &e) {
        std::cerr << "\n✗ Fatal Error: " << e.what() << std::endl;
        return 84;
    }

    return 0;
}