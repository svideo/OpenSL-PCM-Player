Complie
------
- make

Install
------
- adb shell
- cd /sdcard
- mkdir test
- exit
- adb push armeabi/audio-test /sdcard/test
- adb push test.mp3 /sdcard/test
- adb push test_pcm_s16le.pcm /sdcard/test

Run
------
- adb shell
- cd /sdcard/test/
- ./audio-test test.mp3
