#!/usr/bin/env bash
# Flash MeshDeck to a T-Deck over USB. Run ./build.sh first.
set -e
cd "$(dirname "$0")"

ENV_NAME="MeshDeck_TDeck_868"

if [ ! -d build/meshcore ]; then
  echo "!! Run ./build.sh first."
  exit 1
fi

export PATH="$PATH:$HOME/.local/bin:$HOME/Library/Python/3.9/bin:$HOME/Library/Python/3.11/bin:$HOME/Library/Python/3.12/bin:$HOME/Library/Python/3.13/bin"

echo "== Flashing MeshDeck =="
echo "Plug the T-Deck in via USB and switch it ON."
echo "If flashing fails to start: switch it off, HOLD the trackball button,"
echo "switch on while holding, then run this script again."
echo ""

cd build/meshcore
pio run -e "$ENV_NAME" -t upload

echo ""
echo "== Flashed! The T-Deck should reboot into MeshDeck. =="
