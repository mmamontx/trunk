| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C5 | ESP32-C6 | ESP32-C61 | ESP32-H2 | ESP32-H21 | ESP32-H4 | ESP32-P4 | ESP32-S2 | ESP32-S3 | ESP32-S31 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | --------- | -------- | --------- | -------- | -------- | -------- | -------- | --------- |

# 3dfx

NB: The project integrates the following code:

https://github.com/kdienes/madgwick-ahrs/tree/master/MadgwickAHRS

## How to Use 3dfx

Before project configuration and build, be sure to set the correct chip target using `idf.py set-target <chip_name>`.

The build has to fail when building the project for the first time.
Once it has collected the components (during the build), introduce the following change to managed_components/espressif__mpu6050/CMakeLists.txt:

```
#REQUIRES ${REQ}
REQUIRES driver esp_driver_gpio
```

### Build and Flash

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]``.)
