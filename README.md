## [Pixel Reader](https://github.com/ealang/pixel-reader)

An ebook reader app for the Miyoo Mini. Supports epub and txt formats.

![Screenshot](resources/demo.gif)

## Miyoo Mini Installation

Supports Onion, MiniUI, and the default/factory OS.

1. [Download the latest release](https://github.com/ealang/pixel-reader/releases). Make sure to get the correct zip file for your OS. For Onion or default/factory OS: `pixel_reader_onion_xxx.zip`. For MiniUI: `pixel_reader_miniui_xxx.zip`. 
2. Extract the zip into the root of your SD card.
3. Boot your device, and the app should now show up in the apps/tools list.

The default location for book files is `Media/Books`.

## Development Reference

### Desktop Build

Install dependencies (Ubuntu):
```
apt install make g++ libxml2-dev libzip-dev libsdl1.2-dev libsdl-ttf2.0-dev libsdl-image1.2-dev
```

Build:
```
make -j
```

Find app in `build/reader`.

### Search Feature

A simple in‑app text search is available in the reader. Press the `Y` button while reading to open the search prompt, type your query and press Enter to search. Use the shoulder buttons (L1 / R1) to move to previous/next matches. After searching, a **Search Results** UI opens so you can pick a result to jump to.

### Highlights UI

While reading, press the `Menu` button (Esc on desktop/mac keyboard) to open a small menu — choose **Highlights** to view saved highlights for the current book. Select a highlight to jump to it or delete it. Highlights are still persisted to `Books/highlights.txt`.
### CI Cross-Compile (Miyoo Mini)

A GitHub Actions workflow has been added that cross-compiles packages for the Miyoo Mini (ARM Cortex-A7 / armv7). On push to `master` or via manual workflow dispatch it will build and upload artifacts such as `pixel_reader_miniui_v1.0.zip` and `pixel_reader_onion_v1.0.zip`.

### Miyoo Mini Cross-Compile

Cross-compile env is provided by [shauninman/union-miyoomini-toolchain](https://github.com/shauninman/union-miyoomini-toolchain). Docker is required.

Fetch git submodules:
```
git submodule init && git submodule update
```

Start shell:
```
make miyoo-mini-shell
```

Create app packages:
```
./cross-compile/miyoo-mini/create_packages.sh <version num>
```

### Run Tests

[Install gtest](https://github.com/google/googletest/blob/main/googletest/README.md).

```
make test
```
