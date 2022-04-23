# download OIDN
Invoke-WebRequest -Uri "https://github.com/OpenImageDenoise/oidn/releases/download/v1.4.3/oidn-1.4.3.x64.vc14.windows.zip" -OutFile "C:\oidn-1.4.3.x64.vc14.windows.zip"
Expand-Archive "C:\oidn-1.4.3.x64.vc14.windows.zip" -DestinationPath "C:\" -Verbose

# build plugin
cd $CI_PROJECT_DIR
mkdir build
cd build
cmake -G "Visual Studio 16 2019" -DNUKE_ROOT="C:\Program Files\Nuke12.2v10" -DNuke_INCLUDE_DIR="C:\Program Files\Nuke12.2v10\include" -DOPENIMAGEDENOISE_ROOT_DIR="C:\oidn-1.4.3.x64.vc14.windows.zip" -DCMAKE_VERBOSE_MAKEFILE=ON -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
cd ..