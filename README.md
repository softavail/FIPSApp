# FIPSApp

FIPSApp is a simple Android application that demonstrates how to use **OpenSSL** and **cURL** from a native library on Android.
The app includes a native library that links against OpenSSL and cURL (built for Android as shared objects).
For simplicity, this sample includes only the **arm64-v8a** architecture, but the process is identical for other ABIs such as `armeabi-v7a`, `x86`, and `x86_64`.

The UI is intentionally minimal: the user enters a URL and the app attempts to establish a **TLS connection** to it using cURL powered by OpenSSL. This makes the project a compact reference for TLS networking through native code on Android.

---

## Build Instructions

### Requirements

To build the application, you will need:

* **Android Studio**
* **Gradle**
* **Android NDK**
* Ability to compile **C and C++** code

### Install the NDK and build tools

To compile and debug the native code, ensure these components are installed:

* [**Android NDK**](https://developer.android.com/ndk) — provides headers, libraries, and toolchains for building C/C++ code on Android.
* [**CMake**](https://cmake.org/) — the external build system used by Gradle for native builds.

You can install these components from:
**Android Studio → Settings → Appearance & Behavior → System Settings → Android SDK → SDK Tools**

For more details, see: [*Install and configure the NDK and CMake*](https://developer.android.com/studio/projects/install-ndk).

### Compile and Run

1. Open the project in **Android Studio**.
2. Let Gradle sync the project.
3. Build and run the app on an Android device or emulator (ARM64 is required, unless you add more ABIs).

The Gradle build scripts automatically download the required OpenSSL and cURL binaries and place them in the correct native library directories.

