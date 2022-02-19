# download OIDN
cd /tmp
curl -OL https://github.com/OpenImageDenoise/oidn/releases/download/v1.4.2/oidn-1.4.2.x86_64.macos.tar.gz
mkdir -p /opt
tar -C /opt -xvzf /tmp/oidn-1.4.2.x86_64.macos.tar.gz

# clean-up temp
rm -vf /tmp/*

# build plugin
cd $CI_PROJECT_DIR
mkdir build && cd build
cmake -DOpenImageDenoise_ROOT_DIR=/opt/oidn-1.4.2.x86_64.macos ..
make
make install
cd ..