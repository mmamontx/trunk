#ifndef MAIN_H
#define MAIN_H

/* i2c */

#define I2C_MASTER_NUM     I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 400000

#define PIN_NUM_SCL 6
#define PIN_NUM_SDA 7

/* Wi-Fi */

#define WIFI_SSID "3dfx"
#define WIFI_PASS "Voodoo_1"

#define HOST_IP_ADDR "10.3.13.1"
#define HOST_PORT     0x3d3d

#define TO_STR(x) #x
#define STR(x)    TO_STR(x)

//#define IS_LEFT_WHEEL // NB: Comment/uncomment

#ifndef IS_LEFT_WHEEL
#define DEVICE_ID 1
#define ALIGN_AMP_DIR_LEFT  0
#define ALIGN_AMP_DIR_RIGHT 1
#else
#define DEVICE_ID 2
#define ALIGN_AMP_DIR_LEFT  1
#define ALIGN_AMP_DIR_RIGHT 0
#endif

#define DEVICE_IP_ADDR_TOP                "10.3.13.1"
#define DEVICE_IP_ADDR DEVICE_IP_ADDR_TOP STR(DEVICE_ID) // DEVICE_ID 1 -> *.11

// TODO: Integrate HOST_IP_ADDR to the macro below
#define OTA_URL        "http://10.3.13.1:8000/3dfx.bin"
#define OTA_TIMEOUT_MS 60000

/* Etc. */

#define RAD_TO_DEG 57.29577951308232087679815481410517059327f
#define DEG_TO_RAD  0.01745329251994329576923690768488612713f

#define GPIO_NUM_BUTTON 0
#define GPIO_NUM_LED    21

#define LED_CHANGES_PER_SECOND 2
#define NET_FRAMES_PER_SECOND  25

#define IDLE_SECONDS 5

#define IDLE_LOOPS  ((1000 / MAIN_LOOP_PERIOD_MS) * IDLE_SECONDS)
#define CALIB_LOOPS 200

#define FC_MIN       0.05f
#define RC_ALPHA_MAX 0.998f
#define SPEED_MARGIN 5.0f

#define RNG_PERC(rng, perc) ((rng) * ((perc) * 0.01f))

// Leveraged by MadgwickAHRS.c:sampleFreq
#define MAIN_LOOP_PERIOD_MS 5 // 200 FPS

typedef struct __attribute__((packed)) {
    uint8_t device_id;
    float temp;
    float x_roll; // Unused
    float y_pitch; // Unused
    float z_yaw;
    float align_amp;
    float acce_x;
    float acce_y;
    float acce_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;
    float rpm;
    uint32_t loop_time_ms;
} imu_packet_t;

#endif // MAIN_H

