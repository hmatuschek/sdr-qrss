# sdr-qrss - A simple QRSS receiver using libsdr
_sdr-qrss_ is a rather trivial QRSS receiver application based on Qt5 and <a href="https://github.com/hmatuschek/libsdr" target="_blank">libsdr</a>. Hence it can (in principle) use any input source that is supported by libsdr, i.e. the sound card or a RTL2832 dongle. 

<img src="http://i57.tinypic.com/eiuiw0.png" alt="sdr-qrss">


## Usage
The application is controlled completely via command line arguments. 

```
sdr-qrss [OPTIONS]
```

### Options

`--source NAME` or `-s NAME` Specifies the source. `audio` will select the sound card for input and `rtl` will select an RTL2832 dongle as the source. (Default: `audio`)

`--frequency FREQ` or `-f FREQ` Specifies the frequency of the RTL2832 receiver.

`--dot-length LEN` Specifies the dot-length in seconds. (Default: `3`s)

`--bfo-frequency FREQ` Specifies the BFO frequency in Hz. (Default: `700`Hz)

`--width WIDTH` Specifies the frequency width of the spectrum view. (Default: `300`Hz)

`--agc` Enables an AGC.

`--monitor` Enables the audio monitoring. If present, the received signal is also played back to the sound-card.

`--help` Displays a short description of the available options.


## License 
sdr-qrss - Copyright (C) 2014 Hannes Matuschek

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

