# download OIDN
cd /tmp
wget -q -P /tmp/ https://github.com/OpenImageDenoise/oidn/releases/download/v1.4.3/oidn-1.4.3.x86_64.linux.tar.gz
mkdir -p /opt
tar -C /opt -xvzf /tmp/oidn-1.4.3.x86_64.linux.tar.gz

# clean-up temp
rm -vf /tmp/*

# build plugin
cd $CI_PROJECT_DIR
mkdir build && cd build
cmake -DNUKE_ROOT=/usr/local/Nuke12.2v10 -DOIDN_ROOT=/opt/oidn-1.4.3.x86_64.linux -DCMAKE_VERBOSE_MAKEFILE=ON ..
make
make install
cd ..