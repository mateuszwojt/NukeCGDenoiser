# download OIDN
Invoke-WebRequest -Uri "https://github.com/OpenImageDenoise/oidn/releases/download/v1.4.2/oidn-1.4.2.x64.vc14.windows.zip" -OutFile "C:\oidn-1.4.2.x64.vc14.windows.zip"
Expand-Archive "C:\oidn-1.4.2.x64.vc14.windows.zip" -DestinationPath "C:\oidn-1.4.2.x64.vc14.windows"

# build plugin
cd $CI_PROJECT_DIR
mkdir build
cd build
cmake -G "Visual Studio 16 2019" -DOPENIMAGEDENOISE_ROOT_DIR="C:\oidn-1.4.2.x64.vc14.windows" -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
cd ..