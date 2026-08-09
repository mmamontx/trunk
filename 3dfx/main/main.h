#ifndef MAIN_H
#define MAIN_H

/* i2c */

#define I2C_MASTER_NUM     I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 100000
#define I2C_ACK_CHECK_EN   1

#define PIN_NUM_SCL 6
#define PIN_NUM_SDA 7

/* Magnetometer */

#define QMC5883P_DEFAULT_ADDR  0x2c

#define QMC5883P_REG_CHIPID    0x0
#define QMC5883P_REG_XOUT_LSB  0x1
#define QMC5883P_REG_XOUT_MSB  0x2
#define QMC5883P_REG_YOUT_LSB  0x3
#define QMC5883P_REG_YOUT_MSB  0x4
#define QMC5883P_REG_ZOUT_LSB  0x5
#define QMC5883P_REG_ZOUT_MSB  0x6
#define QMC5883P_REG_STATUS    0x9
#define QMC5883P_REG_CONTROL1  0xa
#define QMC5883P_REG_CONTROL2  0xb

#define QMC5883P_REG_OUT       QMC5883P_REG_XOUT_LSB

// CONTROL1 bits 0-1: Mode
#define QMC5883P_MODE_SUSPEND    0x00
#define QMC5883P_MODE_NORMAL     0x01
#define QMC5883P_MODE_SINGLE     0x02
#define QMC5883P_MODE_CONTINUOUS 0x03

// CONTROL1 bits 2-3: Output Data Rate (ODR)
#define QMC5883P_ODR_SHIFT 2
#define QMC5883P_ODR_10HZ  0x00
#define QMC5883P_ODR_50HZ  0x01
#define QMC5883P_ODR_100HZ 0x02
#define QMC5883P_ODR_200HZ 0x03

// CONTROL1 bits 4-5: Over Sample Ratio 1 (OSR1)
#define QMC5883P_OSR1_SHIFT 4
#define QMC5883P_OSR1_8     0x00
#define QMC5883P_OSR1_4     0x01
#define QMC5883P_OSR1_2     0x02
#define QMC5883P_OSR1_1     0x03

// CONTROL1 bits 6-7: Over Sample Ratio 2 (OSR2) / 'Down Sampling Ratio' (DSR)
#define QMC5883P_OSR2_SHIFT 6
#define QMC5883P_OSR2_1     0x00
#define QMC5883P_OSR2_2     0x01
#define QMC5883P_OSR2_4     0x02
#define QMC5883P_OSR2_8     0x03

// CONTROL2 bits 0-1: Set Reset
#define QMC5883P_SETRESET_ON      0x00 // Set and reset on
#define QMC5883P_SETRESET_SETONLY 0x01 // Set only on
#define QMC5883P_SETRESET_OFF     0x02 // Set and reset off

// CONTROL2 bits 2-3: Range
#define QMC5883P_RANGE_SHIFT 2
#define QMC5883P_RANGE_30G   0x00 // +-30 Gauss
#define QMC5883P_RANGE_12G   0x01 // +-12 Gauss
#define QMC5883P_RANGE_8G    0x02 // +-8 Gauss
#define QMC5883P_RANGE_2G    0x03 // +-2 Gauss

#define QMC5883P_MIN_CALIB_TIME_MS    1000
#define QMC5883P_AUTO_CALIB_PERIOD_MS 5000

#define QMC5883P_NVS_NAMESPACE "mag_cal"

/* Wi-Fi */

#define WIFI_SSID "3dfx"
#define WIFI_PASS "Voodoo_1"

#define HOST_IP_ADDR "10.3.13.1"
#define HOST_PORT     0x3d3d

#define TO_STR(x) #x
#define STR(x)    TO_STR(x)

#define DEVICE_ID                         1
#define DEVICE_IP_ADDR_TOP                "10.3.13.1"
#define DEVICE_IP_ADDR DEVICE_IP_ADDR_TOP STR(DEVICE_ID) // DEVICE_ID 1 -> *.11

/* Etc. */

#define RAD_TO_DEG 57.29577951308232087679815481410517059327f
#define DEG_TO_RAD  0.01745329251994329576923690768488612713f

#define GPIO_NUM_BUTTON 0
#define GPIO_NUM_LED    21

#define LED_CHANGES_PER_SECOND 1

// Leveraged by MadgwickAHRS.c:sampleFreq
#define MAIN_LOOP_PERIOD_MS 20 // 50 FPS

typedef struct __attribute__((packed)) {
    uint8_t device_id;
    float temp;
    float x_roll;
    float y_pitch;
    float z_yaw;
} imu_packet_t;

#endif // MAIN_H

