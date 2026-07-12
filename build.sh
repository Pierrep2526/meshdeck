#!/usr/bin/env bash
# MeshDeck build script - clones MeshCore, drops the MeshDeck firmware in, compiles.
# Works on macOS and Linux. Needs: git + python3.
set -e
cd "$(dirname "$0")"

MESHCORE_REPO="https://github.com/meshcore-dev/MeshCore.git"
MESHCORE_COMMIT="bbb58cceb852a9190b46d5f6984e8e5140e6991e"   # pinned, known-compatible
ENV_NAME="MeshDeck_TDeck_868"

echo "== MeshDeck build =="

# 1. make sure PlatformIO is available
if ! command -v pio >/dev/null 2>&1; then
  echo "-- installing PlatformIO (one-time)..."
  python3 -m pip install --user platformio
  export PATH="$PATH:$HOME/.local/bin:$HOME/Library/Python/3.9/bin:$HOME/Library/Python/3.11/bin:$HOME/Library/Python/3.12/bin:$HOME/Library/Python/3.13/bin"
fi
if ! command -v pio >/dev/null 2>&1; then
  echo "!! PlatformIO installed but 'pio' is not on your PATH."
  echo "   Close and reopen the terminal, or run: python3 -m platformio run"
  exit 1
fi

# 2. fetch MeshCore (pinned version)
if [ ! -d build/meshcore ]; then
  echo "-- cloning MeshCore..."
  mkdir -p build
  git clone "$MESHCORE_REPO" build/meshcore
fi
git -C build/meshcore fetch --all --quiet || true
git -C build/meshcore checkout --quiet "$MESHCORE_COMMIT"

# 3. overlay the MeshDeck firmware
echo "-- installing MeshDeck sources..."
rm -rf build/meshcore/examples/meshdeck
mkdir -p build/meshcore/examples/meshdeck
cp -r firmware/* build/meshcore/examples/meshdeck/
cp platformio.local.ini build/meshcore/platformio.local.ini

# 4. build
echo "-- compiling (first run downloads the ESP32 toolchain, ~5-10 min)..."
cd build/meshcore
pio run -e "$ENV_NAME"

# 5. collect output
cd ../..
mkdir -p output
cp "build/meshcore/.pio/build/$ENV_NAME/firmware.bin" output/meshdeck.bin
echo ""
echo "== Done! =="
echo "Firmware: output/meshdeck.bin"
echo "Flash it with: ./flash.sh    (T-Deck plugged in via USB)"
