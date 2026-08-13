#include <float.h>
#include <math.h>

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/sockets.h"

#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "esp_wifi.h"

#include "led_strip.h"
#include "mpu6050.h"
#include "nvs_flash.h"

#include "sdkconfig.h"

#include "main.h"
#include "MadgwickAHRS.h"

static const char *TAG = "3dfx";

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    if ((event_base == WIFI_EVENT) && (event_id == WIFI_EVENT_STA_START)) {
        esp_wifi_connect();
    } else if ((event_base == WIFI_EVENT) &&
               (event_id == WIFI_EVENT_STA_DISCONNECTED)) {
        esp_wifi_connect();
    }
}

static esp_err_t wifi_init_sta(void)
{
    esp_err_t ret;
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    esp_netif_ip_info_t ip_info;
    esp_netif_t *sta_netif;

    ret = esp_netif_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2c_netif_init() returned %d", ret);
        return ret;
    }

    ret = esp_event_loop_create_default();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_event_loop_create_default() returned %d", ret);
        return ret;
    }

    sta_netif = esp_netif_create_default_wifi_sta();

    esp_netif_dhcpc_stop(sta_netif);

    inet_pton(AF_INET, DEVICE_IP_ADDR, &ip_info.ip);
    inet_pton(AF_INET, "255.255.255.0", &ip_info.netmask);
    inet_pton(AF_INET, HOST_IP_ADDR, &ip_info.gw);
    esp_netif_set_ip_info(sta_netif, &ip_info);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); // ?
    ret = esp_wifi_init(&cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_init() returned %d", ret);
        return ret;
    }

    ret = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
        &wifi_event_handler, NULL, &instance_any_id);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_event_handler_instance_register() returned %d", ret);
        return ret;
    }

    ret = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
        &wifi_event_handler, NULL, &instance_got_ip);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_event_handler_instance_register() returned %d", ret);
        return ret;
    }

    ret = esp_wifi_set_mode(WIFI_MODE_STA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_mode() returned %d", ret);
        return ret;
    }

    ret = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_config() returned %d", ret);
        return ret;
    }

    ret = esp_wifi_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_start() returned %d", ret);
        return ret;
    }

    return ESP_OK;
}

static void blink_led(led_strip_handle_t led_strip, bool led_state)
{
    if (led_state) {
        led_strip_refresh(led_strip);
    } else {
        led_strip_clear(led_strip);
    }
}

static esp_err_t configure_led(led_strip_handle_t *led_strip)
{
    esp_err_t ret;
    led_strip_config_t strip_config = {
        .strip_gpio_num = GPIO_NUM_LED,
        .max_leds = 1,
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000,
        .flags.with_dma = false,
    };

    ret = led_strip_new_rmt_device(&strip_config, &rmt_config, led_strip);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "led_strip_new_rmt_device() returned %d", ret);
        return ret;
    }

    ret = led_strip_clear(*led_strip);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "led_strip_clear() returned %d", ret);
        return ret;
    }

    return ESP_OK;
}

static esp_err_t i2c_bus_init(void)
{
    esp_err_t ret;
    i2c_config_t conf;

    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = (gpio_num_t)PIN_NUM_SDA;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = (gpio_num_t)PIN_NUM_SCL;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    conf.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;

    ret = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2c_param_config() returned %d", ret);
        return ret;
    }

    ret = i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2c_driver_install() returned %d", ret);
        return ret;
    }

    return ESP_OK;
}

static mpu6050_handle_t mpu6050_init(void)
{
    esp_err_t ret;
    uint8_t mpu6050_deviceid;
    mpu6050_handle_t mpu6050;

    mpu6050 = mpu6050_create(I2C_MASTER_NUM, MPU6050_I2C_ADDRESS);
    if (mpu6050 == NULL) {
        ESP_LOGE(TAG, "mpu6050_create() returned NULL");
        return NULL;
    }

    // NB: Must be changed for auto
    // acce = w^2 * r, w is in rad/s, r is in meters
    // w = 34.9 rad/s (2000 DPS ~= 39.7 km/h), r = 0.02 m => ~2.5g
    //ret = mpu6050_config(mpu6050, ACCE_FS_16G, GYRO_FS_2000DPS); // For auto
    ret = mpu6050_config(mpu6050, ACCE_FS_2G, GYRO_FS_250DPS);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "mpu6050_config() returned %d", ret);
        return NULL;
    }

    ret = mpu6050_dlpf_cfg(mpu6050, 0x2); // 42 Hz
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "mpu6050_dlpf_cfg() returned %d", ret);
        return NULL;
    }

    ret = mpu6050_wake_up(mpu6050);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "mpu6050_wake_up() returned %d", ret);
        return NULL;
    }

    ret = mpu6050_get_deviceid(mpu6050, &mpu6050_deviceid);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "mpu6050_get_deviceid() returned %d", ret);
        return NULL;
    }
    if (mpu6050_deviceid != MPU6050_WHO_AM_I_VAL) {
        ESP_LOGE(TAG, "mpu6050_deviceid (0x%x) != MPU6050_WHO_AM_I_VAL (0x%x)",
                 mpu6050_deviceid, MPU6050_WHO_AM_I_VAL);
        return NULL;
    }

    return mpu6050;
}

static void button_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_NUM_BUTTON),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&io_conf);
}

static bool button_check(void)
{
    return (gpio_get_level(GPIO_NUM_BUTTON) == 0) ? true : false;
}

void ota_update(void)
{
    esp_http_client_config_t config = {
        .url = OTA_URL,
        .timeout_ms = OTA_TIMEOUT_MS,
        .keep_alive_enable = true,
    };
    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };

    esp_err_t ret = esp_https_ota(&ota_config);
    if (ret == ESP_OK) {
        esp_restart();
    } else {
        ESP_LOGE(TAG, "esp_https_ota() returned %d (%s)",
                 ret, esp_err_to_name(ret));
    }
}

void app_main(void)
{
    // LED
    led_strip_handle_t led_strip;
    bool led_state = false;

    // Gyroscope/Accelerometer
    mpu6050_handle_t mpu6050;
    mpu6050_acce_value_t acce;
    mpu6050_gyro_value_t gyro;
    mpu6050_temp_value_t temp;

    float gx_rad, gy_rad, gz_rad;

    double calib_total_x = 0;
    double calib_total_y = 0;
    double calib_total_z = 0;
    uint16_t idle_loops_remaining = IDLE_LOOPS;
    uint16_t calib_loops_remaining = CALIB_LOOPS;
    float gyro_x_offset = 0.0;
    float gyro_y_offset = 0.0;
    float gyro_z_offset = 0.0;

    float r11;
    float r12;
    float r13;

    float gz_deg;
    float rps;
    float rpm;
    float fc;
    float rc_alpha;

    float raw_roll_prev = FLT_MAX;
    float raw_roll;
    float raw_roll_delta;
    float clean_roll;
    float max_roll;
    float min_roll;
    float acce_y_min = -1.0;
    float acce_y_max = 1.0;
    float range_acce_y = acce_y_max - acce_y_min;
    float align_amp = 0.0;
    float clean_align_amp = 0.0f;
    bool half_turn_flag = false;
    uint8_t align_amp_dir = ALIGN_AMP_DIR_LEFT;

    // Net
    int sock, err;
    struct sockaddr_in dest_addr;
    imu_packet_t packet;
    bool button_pressed = false;
    bool button_pressed_prev = false;
    bool frame_ready = false;

    // Etc.
    esp_err_t ret;
    bool configured = false;

    int loop_count = 0;
    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t period_ticks = pdMS_TO_TICKS(MAIN_LOOP_PERIOD_MS);
    uint32_t current_time = 0;
    uint32_t prev_time = 0;

    dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(HOST_PORT);

    packet.device_id = DEVICE_ID;

    button_init();

    ret = configure_led(&led_strip);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "configure_led() returned %d", ret);
        goto main_loop;
    }

    ret = i2c_bus_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2c_bus_init() returned %d", ret);
        goto main_loop;
    }

    mpu6050 = mpu6050_init();
    if (mpu6050 == NULL) {
        ESP_LOGE(TAG, "mpu6050_init() returned NULL");
        goto main_loop;
    }

    // TODO: Check if NVS needed: for Wi-Fi, for our scenario
    ret = nvs_flash_init();
    if ((ret != ESP_OK) || (ret == ESP_ERR_NVS_NO_FREE_PAGES) ||
        (ret == ESP_ERR_NVS_NEW_VERSION_FOUND)) {
        ESP_LOGE(TAG, "nvs_flash_init() returned %d", ret);

        ret = nvs_flash_erase();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "nvs_flash_erase() returned %d", ret);
            goto main_loop;
        }

        ret = nvs_flash_init();
        if ((ret != ESP_OK) || (ret == ESP_ERR_NVS_NO_FREE_PAGES) ||
            (ret == ESP_ERR_NVS_NEW_VERSION_FOUND)) {
            ESP_LOGE(TAG, "nvs_flash_init() returned %d", ret);
            goto main_loop;
        }
    }

    ret = wifi_init_sta();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "wifi_init_sta() returned %d", ret);
        goto main_loop;
    }

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
        ESP_LOGE(TAG, "socket() returned %d", sock);
        goto main_loop;
    }

    configured = true;

main_loop:
    while (1) {
        if (!configured) {
            led_strip_set_pixel(led_strip, 0, 16, 0, 0); // Red
            goto loop_count_inc;
        }

        if (idle_loops_remaining > 0) {
            idle_loops_remaining--;
            led_strip_set_pixel(led_strip, 0, 16, 16, 16); // White
            goto loop_count_inc;
        }

        if (calib_loops_remaining > 0) {
            ret = mpu6050_get_gyro(mpu6050, &gyro);
            if (ret != ESP_OK) goto loop_count_inc;

            calib_total_x += gyro.gyro_x;
            calib_total_y += gyro.gyro_y;
            calib_total_z += gyro.gyro_z;
            calib_loops_remaining--;

            if (calib_loops_remaining == 0) {
                gyro_x_offset = (float)(calib_total_x / CALIB_LOOPS);
                gyro_y_offset = (float)(calib_total_y / CALIB_LOOPS);
                gyro_z_offset = (float)(calib_total_z / CALIB_LOOPS);
            }

            led_strip_set_pixel(led_strip, 0, 16, 16, 0); // Yellow
            goto loop_count_inc;
        }

        ret = mpu6050_get_acce(mpu6050, &acce);
        if (ret != ESP_OK) goto loop_count_inc;

        ret = mpu6050_get_gyro(mpu6050, &gyro);
        if (ret != ESP_OK) goto loop_count_inc;

        ret = mpu6050_get_temp(mpu6050, &temp);
        if (ret != ESP_OK) goto loop_count_inc;

        /* Madgwick */

        gx_rad = (gyro.gyro_x - gyro_x_offset) * DEG_TO_RAD;
        gy_rad = (gyro.gyro_y - gyro_y_offset) * DEG_TO_RAD;
        gz_rad = (gyro.gyro_z - gyro_z_offset) * DEG_TO_RAD;

#ifndef IS_LEFT_WHEEL
        MadgwickAHRSupdateIMU(-gx_rad, -gz_rad, -gy_rad,
                              -acce.acce_x, -acce.acce_z, -acce.acce_y);
#else
        MadgwickAHRSupdateIMU(gx_rad, gz_rad, -gy_rad,
                              acce.acce_x, acce.acce_z, -acce.acce_y);
#endif

        r11 = q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3;
        r12 = 2.0f * (q1 * q2 - q0 * q3);
        r13 = 2.0f * (q1 * q3 + q0 * q2);

        // The angle between the intended movement vector and the dev axises
        // r11 - forward projection len
        // r12 - right projection len
        // r13 - up projection len
        raw_roll = atan2f(-r12, // = left projection
                                // Below is the same as forward projection;
                                // r11 and r13 always change from '-' to '+'
                                // interchangeably. The sqrt() below gets rid
                                // of it.
                                // This resulting vector is 1.0 (max) when the
                                // wheel goes perfectly forward along X axis.
                                sqrtf(r11 * r11 + r13 * r13)) * RAD_TO_DEG;

        /* Update HPF filter roughness */

        gz_deg = fabsf(gyro.gyro_z - gyro_z_offset); // Degrees per second
        rps = gz_deg / 360.0f; // Rounds per second (Hz)
        rpm = rps * 60.0f; // Rounds per minute

        // Frequency cutoff (fc)
        // Changes less frequent than this (e.g. drift) are ignored
        fc = rps / SPEED_MARGIN;
        // Cutoff the drift even if the rotation is stopped
        if (fc < FC_MIN) fc = FC_MIN;

        rc_alpha = 1.0f / (1.0f + 2.0f * M_PI * fc *
                           (MAIN_LOOP_PERIOD_MS * 0.001f));
        if (rc_alpha > RC_ALPHA_MAX) rc_alpha = RC_ALPHA_MAX;

        /* Filter Z (X in Madgwick) drift */

        if (raw_roll_prev == FLT_MAX) {
            raw_roll_prev = raw_roll;
            clean_roll = 0.0;
            max_roll = clean_roll;
            min_roll = clean_roll;
            goto loop_count_inc;
        }

        raw_roll_delta = raw_roll - raw_roll_prev;
        raw_roll_prev = raw_roll;

        if (raw_roll_delta > 180.0f)  raw_roll_delta -= 360.0f;
        if (raw_roll_delta < -180.0f) raw_roll_delta += 360.0f;

        // HPF - High-Pass Filter
        // The point is to care only about high frequency changes so that the
        // slow gyro drift is neglected.
        clean_roll = (clean_roll + raw_roll_delta) *
                     // rc_alpha is always < 1, and the closer it to 1 - the
                     // softer the filtering.
                     rc_alpha;

        if (clean_roll > max_roll) max_roll = clean_roll;
        if (clean_roll < min_roll) min_roll = clean_roll;

        /* Update Y range */

        if (acce.acce_y < acce_y_min) {
            acce_y_min = acce.acce_y;
            range_acce_y = acce_y_max - acce_y_min;
        }
        if (acce.acce_y > acce_y_max) {
            acce_y_max = acce.acce_y;
            range_acce_y = acce_y_max - acce_y_min;
        }

        /* Detect wheel rotation */

        // NB: Original Y corresponds to g vector
        if ((acce.acce_y > (acce_y_max - RNG_PERC(range_acce_y, 10))) &&
            !half_turn_flag) {
            // Hit the bottom
            align_amp = max_roll - min_roll;

            // 'Asphalt' filter
            if (clean_align_amp == 0.0f) {
                clean_align_amp = align_amp;
            } else {
                // Take only 10% of the new value
                clean_align_amp = (0.1f * align_amp) +
                                  (0.9f * clean_align_amp);
                align_amp = clean_align_amp;
            }

            // clean_roll > 0 -> the wheel is moved to the LEFT
            // (inward for the right wheel, or outward for the left)
            if (clean_roll > 0) align_amp_dir = ALIGN_AMP_DIR_RIGHT;
            // clean_roll < 0 -> the wheel is moved to the RIGHT
            // (outward for the right wheel, or inward for the left)
            if (clean_roll < 0) align_amp_dir = ALIGN_AMP_DIR_LEFT;

            max_roll = clean_roll;
            min_roll = clean_roll;

            half_turn_flag = true;
        } else if (acce.acce_y < (acce_y_min + RNG_PERC(range_acce_y, 10))) {
            // Hit the top
            half_turn_flag = false;
        }

        frame_ready = true;

loop_count_inc:
        loop_count++;

        if (((loop_count % ((1000 / MAIN_LOOP_PERIOD_MS) /
                            NET_FRAMES_PER_SECOND)) == 0) &&
            configured && frame_ready) {
            packet.temp = temp.temp;
            packet.x_roll = 0; // Unused
            packet.y_pitch = 0; // Unused
            packet.z_yaw = clean_roll; // X is treated as Z
            packet.align_amp = align_amp_dir ? clean_align_amp : -clean_align_amp;
            packet.acce_x = acce.acce_x;
            packet.acce_y = acce.acce_y;
            packet.acce_z = acce.acce_z;
            packet.gyro_x = gyro.gyro_x - gyro_x_offset;
            packet.gyro_y = gyro.gyro_y - gyro_y_offset;
            packet.gyro_z = gyro.gyro_z - gyro_z_offset;
            packet.rpm = rpm;
            packet.loop_time_ms = current_time - prev_time;

            err = sendto(sock, &packet, sizeof(packet), 0,
                         (struct sockaddr *)&dest_addr, sizeof(dest_addr));
            if (err < 0) {
                //ESP_LOGE(TAG, "sendto() returned %d, errno = %d", err, errno);
                led_strip_set_pixel(led_strip, 0, 0, 0, 16); // Blue
            } else {
                led_strip_set_pixel(led_strip, 0, 0, 16, 0); // Green
            }

            frame_ready = false;
        }

        if ((loop_count % ((1000 / MAIN_LOOP_PERIOD_MS) /
                           LED_CHANGES_PER_SECOND)) == 0) {
            blink_led(led_strip, led_state);
            led_state = !led_state;
        }

        button_pressed = button_check();

        if (button_pressed && button_pressed_prev) {
            led_strip_set_pixel(led_strip, 0, 16, 16, 16); // White

            blink_led(led_strip, true); // LED on

            ota_update();
        }

        button_pressed_prev = button_pressed;

        if (loop_count == (1000 / MAIN_LOOP_PERIOD_MS)) { // 1 second
            prev_time = current_time;
            current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;

            loop_count = 0;
        }

        vTaskDelayUntil(&last_wake_time, period_ticks);
    }
}

