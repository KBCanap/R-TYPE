/*
** EPITECH PROJECT, 2025
** main.cpp
** File description:
** created by dylan adg
*/

#include "StartServer.hpp"
#include <cstring>
#include <iostream>

static void show_helper()
{
    std::cout << "USAGE:" << std::endl;
    std::cout << "./r-type_server [-p] <port_number>" << std::endl;
    std::cout << "Example: ./r-type_server -p 8080" << std::endl;
}

bool is_a_valid_port(char *str)
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

int main(int ac, char **av)
{
    if (ac == 1 || (ac == 2 && std::strcmp("-h", av[1]) == 0)) {
        show_helper();
        return 0;
    } else if (ac == 3 && std::strcmp("-p", av[1]) == 0 && is_a_valid_port(av[2])) {
        int port = std::atoi(av[2]);
        
        try {
            StartServer server(port);
            std::cout << "Server running on port " << port << ". Press Ctrl+C to stop" << std::endl;
            server.networkLoop();
            
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 84;
        }
        
    } else {
        show_helper();
        return 84;
    }
    return 0;
}
