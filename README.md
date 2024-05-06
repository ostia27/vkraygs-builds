# pygs
Gaussian Splatting

## Desktop Viewer

![](/media/screenshot-fast2.jpg)

~230FPS with 3M visible splats, 4x MSAA, and 1600x900 screen size, on NVIDIA GeForce RTX 4090 (~340FPS with no MSAA.)

FPS may vary depending on splat size, screen size, splat distribution, etc.

Like other web based viewer, it uses traditional graphics pipeline, drawing splats projected in 2D screen space.

One of benefits of using graphics pipeline rather than compute pipeline is that splats can be drawn together with other objects and graphics pipeline features such as MSAA.
The screenshot shows splats rendered with render pass, with depth buffer and 4x MSAA. Splats are drawn over grid and axis.

Tested only on my desktop PC with NVIDIA GeForce RTX 4090, on Windows and Linux. Let me know if there is any issue building or running the viewer.

Current Onesweep radix sort implementation doesn't seem to work on MacOS.

https://raphlinus.github.io/gpu/2021/11/17/prefix-sum-portable.html

So I've implemented reduce-then-scan radix sort. No big performance difference even on NVidia GPU.


### Performance
- NVidia GeForce RTX 4090, Windows
  - garden.ply (30000 iters), 1600x900, 4xMSAA, ~230FPS with 3M visible splats. (2~3x faster than 3DGS.cpp)
  - garden.ply (30000 iters), 1600x900, No MSAA, ~340FPS with 3M visible splats. (3~5x faster than 3DGS.cpp)
- Apple M2 Pro
  - garden.ply (30000 iters), 3200x1800 (retina display), 4xMSAA, ~20FPS with 3M visible splats. 9.5ms (20%) sort, 42ms (80%) rendering.
  - garden.ply (30000 iters), 1600x900, 4xMSAA, ~29FPS with 3M visible splats. 9.5ms (27%) sort, 24.5ms (73%) rendering.
  - bicycle.ply (30000 iters), 1200x800, 4xMSAA, ~40FPS with 2M visible splats. 6.6ms (27%) sort, 17.5ms (73%) rendering.
    - Similar performance reported by [UnityGaussianSplatting](https://github.com/aras-p/UnityGaussianSplatting), 46FPS with Apple M1 Max.


### Rendering Algorithm Details
1. (COMPUTE) rank: cull splats outside view frustum, create key-value pairs to sort, based on view space depth.
1. (COMPUTE) sort: perform 32bit key-value radix sort.
1. (COMPUTE) inverse: create inverse index map from splat order from sorted index. This is for sequential memory access pattern in the next step.
1. (COMPUTE) projection: calculate 3D-to-2D gaussian splat projection, and color using spherical harmonics.
1. (GRAPHICS) rendering: simply draw 2D guassian quads.

Rendering is bottleneck.
The number of visible points is the main factor.
Drawing 2-5 millions points with the simplest point shader (even with depth test/write on and color blend off) already costs 2-3ms.
Without reducing the number of points to draw (e.g. deleting less important splts during training, using octree, early stopping in tile-based rendering), it seems hard to make more dramatic improvements in rendering speed.


### Requirements
- `VulkanSDK>=1.2`
  - Download the latest version from https://vulkan.lunarg.com/ and follow install instruction.
  - `1.3` is recommended, but `1.2` should also work.
- `cmake>=3.15`


### Dependencies
- submodules
  ```bash
  $ git submodule update --init --recursive
  ```
  - VulkanMemoryAllocator
  - glm
  - glfw
  - imgui
  - argparse
  - [vulkan_radix_sort](https://github.com/jaesung-cs/vulkan_radix_sort): my Vulkan/GLSL implementation of [Onesweep](https://research.nvidia.com/publication/2022-06_onesweep-faster-least-significant-digit-radix-sort-gpus) and Reduce-then-scan.


### Build
```bash
$ cmake . -B build
$ cmake --build build --config Release -j
```


### Run
```bash
$ ./build/pygs_base
$ ./build/pygs_base -i <ply_filepath>
```
Drag and drop pretrained .ply file from [official gaussian splatting](https://github.com/graphdeco-inria/gaussian-splatting), Pre-trained Models (14 GB).

Left drag to rotate.

Right drag to translate.

Left+Right drag to zoom in/out.

WASD, Space to move.


### References
- https://github.com/aras-p/UnityGaussianSplatting : Performance report, probably similar rendering pipeline
- https://github.com/shg8/3DGS.cpp : Vulkan viewer, but tile-based rendering with compute shader.


### Notes
- Order Independent Transparency (OIT) doesn't work. There are many nearly-opaque splats overlapped in a pixel, thus colors are blended in unsatisfactory manner.

- Rendering guassian splats with 4x MSAA is slow. Turning MSAA off gives about 2x rendering time boost.

- I've tried 4x MSAA and depth resolve for opaque objects in the first subpass and gaussian splat rendering with no MSAA in the second subpass, where 4x MSAA color/depth images are resolved to 1x MSAA images. Multisample colors are blended with background color into a pixel.

    ![](/media/depth_resolve.jpg)


## Python and CUDA (WIP)

GUI is created in an off thread.
According to GLFW documentation, the user should create window in main thread.
However, managing windows off-thread seems working in Windows and Linux somehow.

Unfortunately, Apple doesn't allow this.
Apple’s UI frameworks can only be called from the main thread.
Here's a related [thread](https://forums.developer.apple.com/forums/thread/659010) by Apple staff.


### Requirements

- Windows or Linux (Doesn't work for MacOS.)
- conda: cmake, pybind11, cuda-toolkit (cuda WIP, not necessary yet)
```bash
$ conda create -n pygs python=3.10
$ conda activate pygs
$ conda install conda-forge::cmake
$ conda install conda-forge::pybind11
$ conda install nvidia/label/cuda-12.2.2::cuda-toolkit  # or any other version
```


### Build

The python package dynamically links to c++ shared library file.

So, first build the shared library first, then install python package.

```bash
$ cmake . -B build
$ cmake --build build --config Release -j
$ pip install -e binding/python
```


### Test

```bash
$ python
>>> import pygs
>>> pygs.show()
>>> pygs.load("./models/bicycle_30000.ply")  # asynchronously load model to viewer
>>> pygs.load("./models/garden_30000.ply")
>>> pygs.close()
```


### Notes
- Directly updating to vulkan-cuda mapped mempry in kernel is slower than memcpy (3.2ms vs. 1ms for 1600x900 rgba32 image)
