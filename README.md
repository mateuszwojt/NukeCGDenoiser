# NukeCGDenoiser

This is a Nuke plugin for denoising CG renders using two different backends - CPU and GPU.

GPU yields faster results (well, obviously). But you can use CPU whenever GPU option is not feasable.

## Requirements

In order to compile this plugin, you're gonna need:

- CMake 3.9 or later
- Nuke (tested on 12.1v2, but any modern version of Nuke should work)
- [Intel's OpenImageDenoise](https://github.com/OpenImageDenoise/oidn)
- [NVIDIA's Optix 7.0](https://developer.nvidia.com/designworks/optix/download)

This plugin should compile on Linux and Windows, since both Intel and NVIDIA backends are supported on these platforms.

## Building

_**Important**_: If OIDN library is not found, `oidn` submodule will be used to compile this dependency. Note that it requires `git-lfs`, building it without LFS installed will likely fail.

- TODO...



## Performance tests

- TODO...

## References

I'd like to say "big thanks" to:
- Marta Nowak, for help in figuring out the contiguos pixel packing
- Hendrik Proosa, for looking through the code and suggesting some great improvements in image buffer access