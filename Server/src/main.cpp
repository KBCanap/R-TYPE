/*
** EPITECH PROJECT, 2025
** main.cpp
** File description:
** created by dylan adg
*/

#include "StartServer.hpp"
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include "GameServerLoop.hpp"

struct ServerConfig {
    int tcp_port;
    int udp_port;
    bool valid;
};

static void show_helper()
{
    std::cout << "USAGE:" << std::endl;
    std::cout << "./r-type_server [-p <tcp_port>] [-u <udp_port>]" << std::endl;
    std::cout << "OPTIONS:" << std::endl;
    std::cout << "  -p <port>    TCP port number (1024-65535)" << std::endl;
    std::cout << "  -u <port>    UDP port number (1024-65535)" << std::endl;
    std::cout << "  -h           Show this help message" << std::endl;
    std::cout << "\nExample: ./r-type_server -p 8080 -u 8081" << std::endl;
}

static bool is_a_valid_port(const char *str)
{
    if (str == nullptr || *str == '\0')
        return false;

    for (int i = 0; str[i]; ++i) {
        if (!std::isdigit(str[i]))
            return false;
    }

    int port = std::atoi(str);
    return port >= 1024 && port <= 65535;
}

static ServerConfig parse_arguments(int ac, char **av)
{
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
        }
        else if (std::strcmp(av[i], "-u") == 0) {
            if (i + 1 >= ac || !is_a_valid_port(av[i + 1])) {
                std::cerr << "Error: Invalid or missing UDP port after -u flag" << std::endl;
                show_helper();
                return config;
            }
            ports["-u"] = std::atoi(av[i + 1]);
            i++;
        }
        else if (std::strcmp(av[i], "-h") == 0) {
            show_helper();
            return config;
        }
        else {
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
    config.udp_port = ports["-u"];
    config.valid = true;
    
    return config;
}

int main(int ac, char **av)
{
    ServerConfig config = parse_arguments(ac, av);
    
    if (!config.valid) {
        return (ac == 1 || (ac == 2 && std::strcmp("-h", av[1]) == 0)) ? 0 : 84;
    }

    try {
        StartServer server(config.tcp_port, config.udp_port);

        short nb_client = server.networkLoop();

        if (nb_client == 0) {
            return 0;
        }
        std::cout << "nb client :" << nb_client << std::endl;
        GameServerLoop game_loop(config.udp_port, nb_client);
        game_loop.start();
        while (game_loop.isRunning()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        game_loop.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 84;
    }
    
    return 0;
}