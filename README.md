# NukeCGDenoiser

This is a Nuke plugin for denoising CG renders using Intel's OpenImageDenoise library.

[Documentation](https://mateuszwojt.gitlab.io/nukecgdenoiser)

## Requirements

In order to compile this plugin, you're gonna need:

- CMake 3.9 or later
- Nuke (tested on 12.2v5, can't say if it works with 13.x yet)
- [Intel's OpenImageDenoise](https://github.com/OpenImageDenoise/oidn)

This plugin should compile fine on both Linux and Windows.

## Building

> OIDN is no longer provided as third party library withing this repository. You need to download the binaries or compile it yourself before building this plugin.

Just be sure to specify path to the OpenImageDenoise library using `OPENIMAGEDENOISE_ROOT_DIR` variable.

### Linux

```
git clone https://gitlab.com/mateuszwojt/nukecgdenoiser.git
cd nukecgdenoiser
mkdir build && cd build
cmake -DOPENIMAGEDENOISE_ROOT_DIR=/path/to/oidn ..
make && make install
```

### Windows

```
git clone https://gitlab.com/mateuszwojt/nukecgdenoiser.git
cd nukecgdenoiser
mkdir build && cd build
cmake -G "Visual Studio 14 2015 Win64" -DOPENIMAGEDENOISE_ROOT_DIR=/path/to/oidn ..
cmake --build . --config Release
```

## Troubleshooting

If for some reason plugin cannot be loaded inside Nuke, make sure that you have the path to OpenImageDenoise library appened to your system's `PATH` variable.

## Limitations

Currently there's no internal validation of the incoming AOV resolution. If your passes are different size, it can introduce artifacts into the denoised image. Make sure to reformat such AOVs, so all passes are at the same image resolution.

This plugin works on a limited number of channels at the moment. Only RGB components are processed, alpha is ignored.

## Contributions

I'd like to say "big thanks" to:
- Marta Nowak, for help in figuring out the contiguos pixel packing
- Hendrik Proosa, for looking through the code and suggesting some great improvements in image buffer access
- Christophe Moreau, for pointing out that the old version was broken ;)
