#!/usr/bin/env bash
set -euo pipefail

URL="https://files.grouplens.org/datasets/movielens/ml-25m.zip"

echo "[HAYAKO] Local environment verification asset pull starting..."
if [ ! -f "ml-25m.zip" ] && [ ! -d "ml-25m" ]; then
    curl -sL "$URL" -o ml-25m.zip
fi

if [ -f "ml-25m.zip" ]; then
    echo "[HAYAKO] Extracting core payload datasets..."
    unzip -qo ml-25m.zip
    rm ml-25m.zip
fi

echo "[HAYAKO] Environment alignment verified."
