# download OIDN
Invoke-WebRequest -Uri "https://github.com/OpenImageDenoise/oidn/releases/download/v2.1.0/oidn-2.1.0.x64.windows.zip" -OutFile "C:\oidn-2.1.0.x64.windows.zip"
Expand-Archive "C:\oidn-2.1.0.x64.windows.zip" -DestinationPath "C:\" -Verbose

# build plugin
cd $CI_PROJECT_DIR
mkdir build
cd build
cmake -G "Visual Studio 16 2019" -DNUKE_ROOT="C:\Program Files\Nuke12.2v10" -DNuke_INCLUDE_DIR="C:\Program Files\Nuke12.2v10\include" -DOIDN_ROOT="C:\oidn-2.1.0.x64.windows" -DCMAKE_VERBOSE_MAKEFILE=ON -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
cd ..