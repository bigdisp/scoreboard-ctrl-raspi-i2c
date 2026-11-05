Main I2C Scoreboard Control
===========================

This project provides basic low level scoreboard control features for communication via the i2c interface. It is meant to control atmegas flashed with the `scoreboard_atmega8_i2c` software. 
The communication acts on a low level wich means:

 * Can only send commands to the atmega
 * States are not remembered
 * Currently, states are not checked, either
 
Requirements:
-------------
 * RaspberryPi of any version (tested with RaspberryPi Model B)
 * `wiringpi` library:  
   run `sudo apt-get install wiringpi`
        or alternatively:  
        ```
        git clone git://git.drogon.net/wiringPi  
        cd wiringPi  
        ./build
        ```  
        If you use Arch Linux instead of Raspian, you can also install wiringpi via its [AUR makepkg script](https://aur.archlinux.org/packages/wiringpi-git/).
        
Build:
------
Navigate to the debug or release folders and run `make`.

i2c protocol:
-------------
The program sends the following bytes to the atmega (default address `0x10`):

| Command                 | i2c transfer                                                       |
|-------------------------|--------------------------------------------------------------------|
| `target digit`          | single byte: the ASCII value of the first symbol of `digit` (blank `' '` if omitted) |
| `pwm on`                | single byte `0x03` (enable pwm)                                    |
| `pwm off`               | single byte `0x02` (disable pwm, LEDs permanently on)              |
| `pwm step` / `pwm a b`  | `0x00` + 16 bit on-time, `0x01` + 16 bit off-time, then `0x04` (apply timing) |

Brightness steps are clamped to the range 0 - 10.

Note: these opcodes belong to the single channel pwm protocol of the
`scoreboard_atmega8_i2c` firmware this tool was written for (2016-04).
The later RGB firmware uses a different opcode layout (see its README);
the PHP API (`scoreboard-php-api`) uses that newer protocol via `i2cset`.

Return codes:
-------------
| Code | Meaning                                  |
|------|------------------------------------------|
| -2   | could not connect to the i2c device      |
| -1   | not enough arguments (usage is printed)  |
| 0    | success                                  |
| 2    | writing a pwm command failed             |
| 3    | writing a symbol failed                  |
| 4    | illegal file descriptor                  |

Tests:
------
The unit tests simulate the i2c bus with a stub of the wiringPi API and check
the exact bytes sent for every command. No hardware or wiringPi installation
is needed:

    make -C test


Usage:
------

    scoreboard target [digit]
    scoreboard pwm value|on|off [value]
        target: The Target number to be changed.
                Abbreviations from list below.
        digit:  The ASCII Symbol to write. Can be any ASCII value,
                but unknown values are replaced by a '-'.
                If string is provided, only the first symbol is written.
        pwm:    Change the PWM settings.
                If a single value is given, it is interpreted as brightness
                steps between 0 and 10. 10 is brightest
                If two values are given, they are interpreted as clock cycles.
                The first value specifies the on-time, the second the off-time.
                If 'off' is given instead of the first value, pwm is disabled.
                In that case, the led are enabled permanently.
                Replacing the value by 'on' reenables pwm.
                This is not done automatically if values are changed.
    
        Abbreviation list:
                H1:  Home ones
                H10: Home tens
                R1:  Road ones
                R10: Road tens
                I1:  Inning ones
                I10: Inning tens

Alternatively, you can use the system i2c commands from the command line:  
    ```  
    addr=0x10  
    i2cset -y 1 $addr 0x48 0x00
    ```
    Here, `0x48` and `0x00` are the register to be written and the data.

TODO
----
 * Integrate pwm.c as well
 * integrate additional special commands if necessary
 