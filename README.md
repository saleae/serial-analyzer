# Saleae Asynchronous Serial Analyzer

Saleae Asynchronous Serial Analyzer

## Getting Started

The following documentation describes getting this analyzer building locally. For more detailed information about the Analyzer SDK, debugging, CI build, and more, checkout the readme from the Sample Analyzer repository:

https://github.com/saleae/SampleAnalyzer

### MacOS

Dependencies:

- XCode with command line tools
- CMake 3.13+
- git

Installing command line tools after XCode is installed:

```
xcode-select --install
```

Then open XCode, open Preferences from the main menu, go to locations, and select the only option under 'Command line tools'.

Installing CMake on MacOS:

1. Download the binary distribution for MacOS, `cmake-*-Darwin-x86_64.dmg`
2. Install the usual way by dragging into applications.
3. Open a terminal and run the following:

```
/Applications/CMake.app/Contents/bin/cmake-gui --install
```

_Note: Errors may occur if older versions of CMake are installed._

Building the analyzer:

```
mkdir build
cd build
cmake ..
cmake --build .
```

### Ubuntu 18.04+

Dependencies:

- CMake 3.13+
- gcc 4.8+
- git

Misc dependencies:

```
sudo apt-get install build-essential
```

Building the analyzer:

```
mkdir build
cd build
cmake ..
cmake --build .
```

### Windows

Dependencies:

- Visual Studio 2019
- CMake 3.13+
- git

**Visual Studio 2019**

_Note - newer and older versions of Visual Studio are likely to work._

Setup options:

- Workloads > Desktop & Mobile > "Desktop development with C++"

Note - if CMake has any problems with the MSVC compiler, it's likely a component is missing.

**CMake**

Download and install the latest CMake release here.
https://cmake.org/download/

**git**

Download and install git here.
https://git-scm.com/

Building the analyzer:

```
mkdir build
cd build
cmake .. -A x64
```

Then, open the newly created solution file located here: `build\serial_analyzer.sln`

The built analyzer DLLs will be located here:

`build\Analyzers\Debug`

`build\Analyzers\Release`

For debug and release builds, respectively.


## Output Frame Format
  
### Frame Type: `"data"`

| Property | Type | Description |
| :--- | :--- | :--- |
| `data` | bytes | The serial word, the width in bits is controlled by the serial settings |
| `error` | str | (optional) Present if an error was detected when decoding this word |
| `address` | bool | (optional) Present if multi-processor or multi-drop bus special modes were selected. True indicates that this is an address byte |

A single serial word

## Bitrate change setting
Sometimes you capture an async signal where the bitrate changes according to certain parameters: for instance a handshake can determine what maximum speed can be used, and then this speed is applied. The 'Bitrate change' field can be used to instruct the analyser to change bitrate at different times in the capture data.

The format of this field is any number of time:bitrate, separated by spaces. For instance if the field contains `0:9600 1.568:115200` it means the beginning of the capture will use 9600 bits/s, and starting at 1.568s, it changes to 115200 bits/s.
The change will be made at any edge after the entered time; a small X marker will be added to this edge to witness the change.
