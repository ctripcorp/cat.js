#!/bin/sh

echo "[build script]start build addon"
node-gyp configure build

echo "[build script]auto generate doc"
apidoc -i src/ -o doc/catjsdoc