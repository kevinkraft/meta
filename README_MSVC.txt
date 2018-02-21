Get the git repo, then after run:

	git submodule update --init --recursive

to get the submodules.

Need to have the following setup in CMakeSettings.json

    {
      "name": "meta-release",
      "generator": "Ninja",
      "configurationType": "Release",
      "inheritEnvironments": [ "msvc_x64_x64" ],
      "buildRoot": "${projectDir}\\build\\${name}",
      "installRoot": "${projectDir}\\install\\${name}",
      "cmakeCommandArgs": "",
      "buildCommandArgs": "-v",
      "ctestCommandArgs": "",
      "variables": [
        {
          "name": "CMAKE_BUILD_TYPE",
          "value": "Release"
        },
        {
          "name": "ICU_ROOT",
          "value": "D:\\Programs\\meta-toolkit\\deps\\icu-60.2"
        },
        {
          "name": "CMAKE_VERBOSE_MAKEFILE:BOOL",
          "value": "ON"
        },
        {
          "name": "META_STATIC_UTF",
          "value": "On"

        }
      ]
    }

Also dont for get to download the models and point to them in the config.toml file.

When building the shared libraries, it doesnt make the .lib files if BUILD_SHARED_LIBS is ON.
So run with it set to OFF, then run again with it ON, and the .libs shouldbe picked up.
BUILD_STATIC_ICU must remain off as it gives an error about not compiling ICU on windows.
I downloaded the binaries and it seem able t ofind them, so I don't think it matters.