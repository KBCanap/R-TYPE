/*
** EPITECH PROJECT, 2025
** main.cpp
** File description:
** created by dylan adg
*/

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>

int main() {
    std::cout << "R-Type Client démarré !" << std::endl;

    // Test SFML Graphics
    sf::RenderWindow window(sf::VideoMode(800, 600), "R-Type Client Test");
    sf::CircleShape shape(50);
    shape.setFillColor(sf::Color::Green);

    std::cout << "Fenêtre SFML créée (800x600)" << std::endl;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.draw(shape);
        window.display();
    }

    std::cout << "Client fermé" << std::endl;
    return 0;
}
