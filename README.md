# SuperTuxKart

本仓库用来临时调试 supertuxkart.

```shell
sudo apt-get install build-essential cmake libbluetooth-dev libsdl2-dev \
libcurl4-openssl-dev libenet-dev libfreetype6-dev libharfbuzz-dev \
libjpeg-dev libogg-dev libopenal-dev libpng-dev \
libssl-dev libvorbis-dev libmbedtls-dev pkg-config zlib1g-dev \
git subversion ninja-build

mkdir stk && cd stk
git clone --depth=1 --branch=v1.4-dbg https://github.com/Sakura286/stk-code stk-code

# asset 即 data deb 包里的文件
svn checkout https://svn.code.sf.net/p/supertuxkart/code/stk-assets stk-assets

cd stk-assets
# 几乎必定中断，所以需要重复执行下面一条指令直至所有文件下载完毕
svn cleanup && svn update

# 切换到 v1.4 对应的版本
svn update -r18556

cd ../stk-code
mkdir build && cd build
cmake .. -DBUILD_RECORDER=0 -DCMAKE_BUILD_TYPE="Debug" -DUSE_CRYPTO_OPENSSL=OFF
make -j$(nproc)

# 后面就可以用 gdb 调试了
gdb ./bin/supertuxkart
```
