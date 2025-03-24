# vkraygs

Official code of the CVPR 2025 paper "Hardware Rasterized Ray-Based Gaussian Splatting" [[Project page](https://facebookresearch.github.io/vkraygs/)].

The codebase provides a Ray-based Gaussian splatting viewer written on top of [vkgs](https://github.com/jaesung-cs/vkgs).

## Desktop Viewer

![](/media/screenshot.png)

Viewer works with pre-trained 3DGS-compatible models as input.


### Requirements
- `VulkanSDK>=1.2`
  - Download the latest version from https://vulkan.lunarg.com/ and follow install instruction.
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
  - [vulkan_radix_sort](https://github.com/jaesung-cs/vulkan_radix_sort): my Vulkan/GLSL implementation of reduce-then-scan radix sort.


### Build
```bash
$ cmake . -B build
$ cmake --build build --config Release -j
```


### Run
```bash
$ ./build/vkgs_viewer 
$ ./build/vkgs_viewer -i <ply_filepath>
```
Drag and drop pretrained .ply files.

- Left drag to rotate.

- Right drag to translate.

- Left+Right drag to zoom in/out.

- WASD, Space to move.

- Wheel to zoom in/out.

- Ctrl+wheel to change FOV.

### RayGS models

RayGS models can be created with [Gaussian Opacity Fields](https://github.com/autonomousvision/gaussian-opacity-fields) (GOF).
The trained models should be exported with `create_fused_ply.py` from the same repo before being used with this viewer. This will adapt the primitive's scale and opacity to account for the MIP logic of GOF.
For this reason, in the viewer the MIP bias should be set to 0.0.

## License
vkraygs is MIT licensed, as found in the LICENSE file.
