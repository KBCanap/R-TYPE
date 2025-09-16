/*
** EPITECH PROJECT, 2025
** main.cpp
** File description:
** created by dylan adg
*/

#include <iostream>
#include "asio.hpp"

int main() {
    try {
        asio::io_context io_context;
        
        std::cout << "R-Type Server démarré !" << std::endl;
        std::cout << "Version Asio: " << ASIO_VERSION << std::endl;
        
        // Test basique d'un acceptor
        asio::ip::tcp::acceptor acceptor(io_context, 
            asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 8080));
        
        std::cout << "Serveur en écoute sur le port 8080" << std::endl;
        std::cout << "Appuyez sur Ctrl+C pour arrêter" << std::endl;
        
        io_context.run();
        
    } catch (std::exception& e) {
        std::cerr << "Erreur: " << e.what() << std::endl;
    }
    
    return 0;
}