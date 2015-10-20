Building for linux
------------------

    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Debug ..
    make

Building for android
--------------------

To build the native library:

    mkdir build
    cd build
    ANDROID_NDK=~/opt/android-ndk-r10e ANDROID_NATIVE_API_LEVEL=14 cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/android.toolchain.cmake -DANDROID_ABI=armeabi -DCMAKE_BUILD_TYPE=Debug ..
    make

`ANDROID_ABI=armeabi` will build the library for ARMv5TE with SW floating point (or else it will be built for ARMv7 with HW floating point and crash on the emulator).

To build the APK:

    cd android
    android update project -p . --target 2
    mkdir src
    ant debug

The `android update` step only needs to be done once. The argument for `--target` should be the id of an Android target with at least API level 14 (`android list target` may be useful).

Actually I've just lied, these instructions are not enough to get a working APK. You will also need to build the `assets` dir and put it under `android` before building. More on this later.
