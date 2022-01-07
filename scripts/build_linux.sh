# download OIDN
cd /tmp
wget -q -P /tmp/ https://github.com/OpenImageDenoise/oidn/releases/download/v1.4.2/oidn-1.4.2.x86_64.linux.tar.gz
mkdir -p /opt
tar -C /opt -xvzf /tmp/oidn-1.4.2.x86_64.linux.tar.gz

# clean-up temp
rm -vf /tmp/*

# build plugin
cd $CI_PROJECT_DIR
mkdir build && cd build
cmake -DOPENIMAGEDENOISE_ROOT_DIR=/opt/oidn-1.4.2.x86_64.linux ..
make
make install
cd ..