# evmpd

Sends commands to mpd based on media-key events.

``` bash
$ make
$ sudo ./evmpd localhost 6600 \
   /dev/input/by-id/usb-xxxx_xxxx-event-kbd \
   'shutdown -h now'
...
```

Requires:

- [libmpdclient](http://www.musicpd.org/libs/libmpdclient/)
- [libevdev](http://www.freedesktop.org/wiki/Software/libevdev/)

Supports:

- `F4` = `HALT_COMMAND`
- Play/Pause
- Stop
- Previous
- Next
- Volume Up (increments by 5%)
- Volume Down (decrements by 5%)


## Formatting

Code formatting using GNU indent:

``` bash
$ indent -kr -ci2 -cli2 -i2 -l80 -nut
```
