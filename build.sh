# download and extract Nuke
wget -q -P /tmp/ https://thefoundry.s3.amazonaws.com/products/nuke/releases/12.1v2/Nuke-12.1v2-linux-x86-64-installer.tgz
tar -C /tmp -xvzf /tmp/Nuke-12.1v2-linux-x86-64-installer.tgz
cd /tmp
./Nuke-12.1v2-linux-x86-64-installer.run --accept-foundry-eula --prefix=/usr/local
rm -vf /tmp/*

# build plugin
cd $CI_PROJECT_DIR
mkdir build && cd build
cmake ..
make
make install
cd ..