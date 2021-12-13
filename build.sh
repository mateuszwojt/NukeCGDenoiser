# download OIDN
wget -q -P /tmp/ https://github.com/OpenImageDenoise/oidn/releases/download/v1.4.2/oidn-1.4.2.x86_64.linux.tar.gz
mkdir -p /opt/oidn
tar -C /opt/oidn -xvzf /tmp/oidn-1.4.2.x86_64.linux.tar.gz

# download and extract Nuke
wget -q -P /tmp/ https://thefoundry.s3.amazonaws.com/products/nuke/releases/12.2v5/Nuke-12.2v5-linux-x86-64-installer.tgz
tar -C /tmp -xvzf /tmp/Nuke-12.2v5-linux-x86-64-installer.tgz
cd /tmp
./Nuke-12.2v5-linux-x86-64-installer.run --accept-foundry-eula --prefix=/usr/local
rm -vf /tmp/*

# build plugin
cd $CI_PROJECT_DIR
mkdir build && cd build
cmake -DOPENIMAGEDENOISE_ROOT_DIR=/opt/oidn ..
make
make install
cd ..