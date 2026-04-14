# tool-common

Shared GUI libraries for LU-Rebuilt tools.

> **Note:** This project was developed with significant AI assistance (Claude by Anthropic). All code has been reviewed and validated by the project maintainer, but AI-generated code may contain subtle issues. Contributions and reviews are welcome.

Part of the [LU-Rebuilt](https://github.com/LU-Rebuilt) project.

## Libraries

| Library | Description | Dependencies |
|---------|-------------|--------------|
| **qt_common** | Qt6 file browser widget | Qt6::Widgets |
| **gl_viewport** | OpenGL 3D viewport with orbit camera, ray picking, mesh rendering | Qt6, OpenGL |

## Usage

### Via FetchContent

```cmake
FetchContent_Declare(tool_common
    GIT_REPOSITORY https://github.com/LU-Rebuilt/tool-common.git
    GIT_TAG main
)
FetchContent_MakeAvailable(tool_common)

# File browser widget only:
target_link_libraries(your_tool PRIVATE qt_common)

# 3D viewport with orbit camera and mesh rendering:
target_link_libraries(your_tool PRIVATE gl_viewport qt_common)
```

## License

[GNU Affero General Public License v3.0](https://www.gnu.org/licenses/agpl-3.0.html) (AGPLv3)

