# ImGui Addons from ZynLab

Here are some useful ImGui addons mainly designed for audio applications, mainly including knob and check button.

- **Author:** Wouter Saaltink ([@wtrsltnk](https://github.com/wtrsltnk))
- **Source:** <https://github.com/wtrsltnk/zynlab/tree/master/lib.imgui/imgui_addons>

## Usage

To include them into your project:

**First,** include `imgui_common.h` and addon's header file into your own source file. For example, if you prefer knobs:

```c++
// Addons from Zynlab
#include "imgui_addons/imgui_knob.h"
```

Remember to set `target_include_directories()` when using CMake.

**Second,** add corresponding C++ files to your source file list in build systems (`Makefile` or `CMakeLists.txt`), together with Dear ImGui's official source files.

