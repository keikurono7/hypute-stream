#!/usr/bin/env bash
set -euo pipefail

URL="https://files.grouplens.org/datasets/movielens/ml-1m.zip"

echo "[HAYAKO] Local environment verification asset pull starting..."
if [ ! -f "ml-1m.zip" ] && [ ! -d "ml-1m" ]; then
    curl -sL "$URL" -o ml-1m.zip
fi

if [ -f "ml-1m.zip" ]; then
    echo "[HAYAKO] Extracting core payload datasets..."
    unzip -qo ml-1m.zip
    rm ml-1m.zip
fi

echo "[HAYAKO] Environment alignment verified."
