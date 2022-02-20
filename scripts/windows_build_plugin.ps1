# download OIDN
Invoke-WebRequest -Uri "https://github.com/OpenImageDenoise/oidn/releases/download/v1.4.2/oidn-1.4.2.x64.vc14.windows.zip" -OutFile "C:\oidn-1.4.2.x64.vc14.windows.zip"
Expand-Archive "C:\oidn-1.4.2.x64.vc14.windows.zip" -DestinationPath "C:\" -Verbose

# build plugin
cd $CI_PROJECT_DIR
mkdir build
cd build
cmake -G "Visual Studio 16 2019" -DNUKE_ROOT="C:\Program Files\Nuke12.2v10" -DOIDN_ROOT="C:\oidn-1.4.2.x64.vc14.windows" -DCMAKE_VERBOSE_MAKEFILE=ON -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
cd ..