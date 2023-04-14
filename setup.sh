# Get number of CPU cores
CPUS=$(getconf _NPROCESSORS_ONLN 2>/dev/null || getconf NPROCESSORS_ONLN 2>/dev/null || echo 1)

sudo apt-get update --yes
sudo apt-get upgrade --yes
sudo apt-get install cmake python3-dev python3-pytest doxygen graphviz gcovr llvm g++ pkg-config m4 --yes
sudo curl --proto '=https' --tlsv1.3 https://sh.rustup.rs -sSf | sh -s -- -y #install rust compiler
wget -q https://ftp.mozilla.org/pub/firefox/releases/102.2.0esr/source/firefox-102.2.0esr.source.tar.xz
tar xf firefox-102.2.0esr.source.tar.xz
cd firefox-102.2.0/js
sed -i 's/bool Unbox/JS_PUBLIC_API bool Unbox/g' ./public/Class.h           # need to manually add JS_PUBLIC_API to js::Unbox until it gets fixed in Spidermonkey
sed -i 's/bool js::Unbox/JS_PUBLIC_API bool js::Unbox/g' ./src/vm/JSObject.cpp  # same here
cd src
cp ./configure.in ./configure
chmod +x ./configure
mkdir _build
cd _build
../configure --disable-jemalloc --with-system-zlib --with-intl-api --enable-optimize
make -j$CPUS
sudo make install