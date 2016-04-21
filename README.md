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

Usage:
------
    ```
    scoreboard target [value]
        target: The Target number to be changed.
                Abbreviations from list below.
        value:  The ASCII Symbol to write. Can be any ASCII value,
                but unknown values are replaced by a '-'.
                If string is provided, only the first symbol is written.
    
        Abbreviation list:
                H1:  Home ones
                H10: Home tens
                R1:  Road ones
                R10: Road tens
                I1:  Inning ones
                I10: Inning tens
    ```
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
 