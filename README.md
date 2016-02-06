Building prerequisites
-------------
* [packsprites](https://github.com/mpersano/packsprites) in the `$PATH`.

Building for linux
------------------

    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Debug ..
    make

Building for windows
--------------------

See [this](https://gist.github.com/mpersano/14d2227462ebbfee4b92) for instructions on cross-compiling the prerequisites.

    mkdir build
    cd build
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/mingw-w64.toolchain.cmake ..
    make

Building for android
--------------------

To build the native library:

    mkdir build
    cd build
    ANDROID_NDK=~/opt/android-ndk-r10e ANDROID_NATIVE_API_LEVEL=19 cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/android.toolchain.cmake -DANDROID_ABI=armeabi -DCMAKE_BUILD_TYPE=Debug ..
    make

`ANDROID_NDK` should point to your local NDK root, of course. The `ANDROID_ABI=armeabi` option will build the library for ARMv5TE with SW floating point (without it, the library will be built for ARMv7 with HW floating point and crash on the emulator).

To build the APK:

    cd android
    android update project -p . --target 2
    mkdir src
    ant debug

The `android update` step only needs to be done once. The argument for `--target` should be the id of an Android target with at least API level 19 (`android list target` may be useful).
