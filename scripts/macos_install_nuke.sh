# download and install Nuke
cd /tmp
curl -OL https://thefoundry.s3.amazonaws.com/products/nuke/releases/12.2v10/Nuke12.2v10-mac-x86_64.dmg
hdiutil convert -quiet /tmp/Nuke12.2v10-mac-x86_64.dmg -format UDTO -o Nuke12.2v10
hdiutil attach -quiet -nobrowse -noverify -noautoopen -mountpoint right_here /tmp/Nuke12.2v10.cdr
pushd /Volumes/Nuke12.2v10-mac-x86_64
cp -R Nuke12.2v10 /Applications/
popd
hdiutil detach /Volumes/Nuke12.2v10-mac-x86_64