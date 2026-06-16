# webwindowed

A header-only library for building C/C++ applications with a web-based GUI.
Unlike technologies like electron, it does not use a shipped browser but instead makes use of other browser-libraries on
the target platform.

This is my personal variant of [webview/webview](https://github.com/webview/webview) which is mostly fit for my purposes
and not designed to necessarily be useful or fit for anything else.
For something that is probably a better idea to use, see [Tauri](https://tauri.app)
or [saucer](https://github.com/saucer/saucer).

## Platform Support

| Platform | Technologies                                      |
|----------|---------------------------------------------------|
| Linux    | [GTK][gtk], [WebKitGTK][webkitgtk]                |
| Windows  | [Windows API][win32-api], [WebView2][ms-webview2] |

## Documentation

The most up-to-date documentation is right in the source code. Improving the documentation is a continuous effort and
you are more than welcome to contribute.

## Prerequisites

Your compiler must support minimum C++23.

This project provides its code as headers only.
All you need to do is to include the headers

### Linux

The [GTK][gtk] and [WebKitGTK][webkitgtk] libraries are required for development and distribution. You need to check
your package repositories regarding which packages to install.

#### Packages

- Debian:
    - WebKitGTK 6.0, GTK 4:
        - Development: `apt install libgtk-4-dev libwebkitgtk-6.0-dev`
        - Production: `apt install libgtk-4-1 libwebkitgtk-6.0-4`
- Fedora:
    - WebKitGTK 6.0, GTK 4:
        - Development: `dnf install gtk4-devel webkitgtk6.0-devel`
        - Production: `dnf install gtk4 webkitgtk6.0`
- FreeBSD:
    - GTK 4: `pkg install webkit2-gtk4`

#### Library Dependencies

- Linux:
    - Use `pkg-config` with `--cflags` and `--libs` to get the compiler/linker options for one of these sets of modules:
        - `gtk4 webkitgtk-6.0`
    - Link libraries: `dl`
- Windows:
    - [WebView2 from NuGet](https://www.nuget.org/packages/Microsoft.Web.WebView2).
    - Windows libraries: `advapi32 ole32 shell32 shlwapi user32 version`

### Windows

Your compiler must support C++23 and it is recommended to pair it with an up-to-date Windows 10 SDK.

For Visual C++ we recommend Visual Studio 2026 or later.

Developers and end-users must have the [WebView2 runtime][ms-webview2-rt] installed on their system for any version of
Windows before Windows 11.

## Getting Started

If you are a developer of this project then please go to the [development section](#development).

You will have a working app, but you are encouraged to explore the [available examples](./examples).

Create the following files in a new directory:

`.gitignore`:

```
# Build artifacts
/build
```

### Building the Example

Build the project:

```sh
premake5 gmake
./scripts/build.sh
```

Find the executable in the `build/bin` directory.

## MS WebView2 Loader

Linking the WebView2 loader part of the Microsoft WebView2 SDK is not a hard requirement when using our webview library,
and neither is distributing `WebView2Loader.dll` with your app.

If, however, `WebView2Loader.dll` is loadable at runtime, e.g. from the executable's directory, then it will be used;
otherwise our minimalistic implementation will be used instead.

Should you wish to use the official loader then remember to distribute it along with your app unless you link it
statically. Linking it statically is possible with Visual C++.

Here are some of the noteworthy ways our implementation of the loader differs from the official implementation:

- Does not support configuring WebView2 using environment variables such as `WEBVIEW2_BROWSER_EXECUTABLE_FOLDER`.
- Microsoft Edge Insider (preview) channels are not supported.

## Thread Safety

Since library functions generally do not have thread safety guarantees,
`webwindowed::dispatch()` can be used to schedule code to execute on the main/GUI thread and thereby make that
execution safe in multi-threaded applications.

`webwindowed::resolve()` uses `*dispatch()` internally and is therefore safe to call from
another thread.

The main/GUI thread should be the thread that calls `webwindowed::run()`.

## Development

This project uses the premake5 build system.

## License

Code is distributed under MIT license.

[gtk]: https://gtk.org/

[webkitgtk]: https://webkitgtk.org/

[ms-webview2]: https://developer.microsoft.com/en-us/microsoft-edge/webview2/

[ms-webview2-rt]: https://developer.microsoft.com/en-us/microsoft-edge/webview2/

[win32-api]: https://docs.microsoft.com/en-us/windows/win32/apiindex/windows-api-list
