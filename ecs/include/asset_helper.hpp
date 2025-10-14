/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** asset_helper
*/

#pragma once
#include <string>

inline std::string getAssetPath(const std::string &filename) {
#ifdef ASSETS_PATH
    return std::string(ASSETS_PATH) + filename;
#else
    return std::string("./assets/") + filename;
#endif
}