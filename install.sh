#!/usr/bin/env bash

set -euo pipefail

# Script for installing shareIT (locally). Run without sudo.

EXEC_NAME="shareit"

EXEC_TARGET_DIR="$HOME/.local/bin"

SHAREIT_DIR="$HOME/.shareit"
CONFIG_DIR_NAME="config"
LOG_DIR_NAME="log"

echo "[INFO] Compiling..."
make build
echo "[INFO] Program compiled successfully."

echo "[INFO] Adding binary to $EXEC_TARGET_DIR..."
install -vd $EXEC_TARGET_DIR
install -vm 755 shareit $EXEC_TARGET_DIR

echo "[INFO] .shareit folder..."

install -vd $SHAREIT_DIR
install -vd $SHAREIT_DIR/$CONFIG_DIR_NAME
touch $SHAREIT_DIR/$CONFIG_DIR_NAME/config.json

cat config_template.json > $SHAREIT_DIR/$CONFIG_DIR_NAME/config.json

install -vd $SHAREIT_DIR/$LOG_DIR_NAME

echo "[INFO] installation completed."




if ! echo ":$PATH:" | grep -q ":$EXEC_TARGET_DIR:" ; then
    echo "[WARNING] $EXEC_TARGET_DIR is not in PATH."
fi


echo "run the cli by typing shareit in your terminal."