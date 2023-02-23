# download and install Nuke
wget -q -P /tmp/ https://thefoundry.s3.amazonaws.com/products/nuke/releases/12.2v10/Nuke12.2v10-linux-x86_64.tgz
tar -C /tmp -xvzf /tmp/Nuke12.2v10-linux-x86_64.tgz
cd /tmp
./Nuke12.2v10-linux-x86_64.run --accept-foundry-eula --prefix=/usr/local