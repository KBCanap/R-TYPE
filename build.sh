#!/bin/bash
# Script de construction automatique
set -e

BUILD_TYPE=${1:-Release}
BUILD_DIR="build_${BUILD_TYPE,,}"

echo "Construction du projet R-TYPE..."
echo "Type de build: $BUILD_TYPE"
echo "Répertoire: $BUILD_DIR"

# Création du répertoire de build
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configuration et construction
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..
make -j$(nproc)

echo "Construction terminée!"
echo "Exécutables disponibles dans: $BUILD_DIR/bin/"
echo "  - r-type_server"
echo "  - r-type_app (client avec ECS)"
