# download OIDN
Invoke-WebRequest -Uri "https://github.com/OpenImageDenoise/oidn/releases/download/v1.4.2/oidn-1.4.2.x64.vc14.windows.zip" -OutFile "C:\oidn-1.4.2.x64.vc14.windows.zip"
Expand-Archive "C:\oidn-1.4.2.x64.vc14.windows.zip" -DestinationPath "C:\oidn-1.4.2.x64.vc14.windows"

# build plugin
cd $CI_PROJECT_DIR
mkdir build
cd build
cmake -G "Visual Studio 16 2019" -DNuke_INCLUDE_DIR="C:/Program Files/Nuke12.2v10/include" -DOPENIMAGEDENOISE_ROOT_DIR="C:/oidn-1.4.2.x64.vc14.windows" -DOPENIMAGEDENOISE_LIBRARY="C:/oidn-1.4.2.x64.vc14.windows/lib/OpenImageDenoise.lib" -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
cd ..