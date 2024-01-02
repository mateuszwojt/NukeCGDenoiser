# NukeCGDenoiser

![](images/denoiser_node_usage.png)

This is a Nuke plugin for denoising CG renders using Intel's Open Image Denoise library.

[Documentation](https://mateuszwojt.gitlab.io/nukecgdenoiser)

## Requirements

In order to compile this plugin, you're gonna need:

- CMake 3.13 or later
- Nuke 12.x/13.x/14.x/15.x
- [Intel's OpenImageDenoise 2.x](https://github.com/OpenImageDenoise/oidn)

This plugin should compile fine on Windows, Linux and MacOS. This plugin was tested on Nuke 15.0v2 / MacOS Ventura 13.6.1 (arm64) with Open Image Denoise v2.1.0.

## Building

> OIDN is no longer provided as third party library withing this repository. You need to compile it yourself or download the pre-built binaries from [here](https://github.com/OpenImageDenoise/oidn/releases) before building this plugin.

Just be sure to specify path to the OpenImageDenoise library using `DOIDN_ROOT` variable.

### Linux

```
git clone https://gitlab.com/mateuszwojt/nukecgdenoiser.git
cd nukecgdenoiser
mkdir build && cd build
cmake -DDOIDN_ROOT=/path/to/oidn ..
make && make install
```

### Windows

```
git clone https://gitlab.com/mateuszwojt/nukecgdenoiser.git
cd nukecgdenoiser
mkdir build && cd build
cmake -G "Visual Studio 14 2015 Win64" -DDOIDN_ROOT=/path/to/oidn ..
cmake --build . --config Release
```

## Troubleshooting

If for some reason plugin cannot be loaded inside Nuke, make sure that you have the path to OpenImageDenoise library appened to your system's `PATH` variable.

If the plugin is crashing Nuke, please create a new issue [here on GitLab](https://gitlab.com/mateuszwojt/nukecgdenoiser/-/issues) with the description containing steps to reproduce the crash.

## Limitations

Currently there's no internal validation of the input AOVs resolution. If your passes are different width and height for some reason, it can introduce artifacts into the denoised image. Make sure to reformat such AOVs, so all passes are at the same image resolution.

This plugin works on a limited number of channels at the moment. Only RGB components are processed, alpha is ignored.
