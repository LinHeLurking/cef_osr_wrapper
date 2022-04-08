# CEF OSR Wrapper

This project wraps CEF in off-screen redering mode.
And sends out the frame buffers by Socket API.

## Build Instructions

Currently some implementations are only available on Windows.

On Windows, make sure to build and link the project with toolchain
`msvc_x64_x64`. Toolchain `msvc_x86_x64` won't work.