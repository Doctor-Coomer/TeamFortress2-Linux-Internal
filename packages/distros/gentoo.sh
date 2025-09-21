sudo emerge media-libs/glew media-libs/libsdl dev-debug/gdb dev-util/vulkan-headers

cd /tmp
wget https://versaweb.dl.sourceforge.net/project/glew/glew/2.1.0/glew-2.1.0.tgz?viasf=1
mkdir glew-2.1.0
tar -zxvf glew-2.1.0.tgz -C ./glew-2.1.0
cd glew-2.1.0/
make
sudo make install.all
