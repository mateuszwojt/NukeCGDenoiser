# download OIDN
cd $CI_PROJECT_DIR
Invoke-WebRequest -Uri "https://github.com/OpenImageDenoise/oidn/releases/download/v1.4.2/oidn-1.4.2.x64.vc14.windows.zip" -OutFile $CI_PROJECT_DIR\oidn-1.4.2.x64.vc14.windows.zip
Expand-Archive $CI_PROJECT_DIR\oidn-1.4.2.x64.vc14.windows.zip -DestinationPath $CI_PROJECT_DIR\oidn-1.4.2.x64.vc14.windows -Verbose

# build plugin
mkdir build
cd build
cmake -G "Visual Studio 16 2019" -DNUKE_ROOT="C:/Program Files/Nuke12.2v10" -DOIDN_ROOT=$CI_PROJECT_DIR/oidn-1.4.2.x64.vc14.windows -DOpenImageDenoise_INCLUDE_DIR=$CI_PROJECT_DIR/oidn-1.4.2.x64.vc14.windows/include -DOpenImageDenoise_LIBRARIES=$CI_PROJECT_DIR/oidn-1.4.2.x64.vc14.windows/lib/OpenImageDenoise.lib -DCMAKE_VERBOSE_MAKEFILE=ON -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
cd ..