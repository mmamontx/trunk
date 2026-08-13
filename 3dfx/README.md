| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C5 | ESP32-C6 | ESP32-C61 | ESP32-H2 | ESP32-H21 | ESP32-H4 | ESP32-P4 | ESP32-S2 | ESP32-S3 | ESP32-S31 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | --------- | -------- | --------- | -------- | -------- | -------- | -------- | --------- |

# 3dfx

NB: The project integrates the following code:

https://github.com/kdienes/madgwick-ahrs/tree/master/MadgwickAHRS

## How to Use 3dfx

Before project configuration and build, be sure to set the correct chip target using `idf.py set-target <chip_name>`.

Patch the gyro lib:

```
patch -p0 <mpu6050.patch
```

### Build and Flash

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]``.)

OR

1. Run `idf.py build`, `cd build`, `python3 -m http.server 8000 --bind 0.0.0.0`.
2. Bring-up a Wi-Fi AP with address 10.3.13.1.
3. Power-on the device, wait for the LED to blink green, then push the BOOT button (the left one).
