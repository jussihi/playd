# playd


playd is a simple MOD player (currently supports only S3M file format) inspired by cool demo scene things (Future Crew), MilkyTracker videos on youtube and bisqwit's work. I also always liked the 8-bit "music".

Its audio interface is based on ALSA, so you should be using Linux. I might also make a future update with Windows support.


# How to use

Build with make. It should build as long as ALSA is present on the machine. Purple Motion's "Starshine" is included as a test track. Here's from theres:
```sh
$ make
$ ./playd strshine.s3m
```
