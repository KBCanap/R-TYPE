#pragma once
#include <string>

// Fonction helper pour construire les chemins d'assets
inline std::string getAssetPath(const std::string& filename) {
#ifdef ASSETS_PATH
    return std::string(ASSETS_PATH) + filename;
#else
    // Fallback par défaut si ASSETS_PATH n'est pas défini
    return std::string("./assets/") + filename;
#endif
}