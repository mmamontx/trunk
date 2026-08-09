#include <math.h>

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/sockets.h"

#include "esp_event.h"
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
        ESP_LOGI(TAG, "Received event_id = WIFI_EVENT_STA_START");
        ESP_LOGI(TAG, "Connecting...");
        esp_wifi_connect();
    } else if ((event_base == WIFI_EVENT) &&
               (event_id == WIFI_EVENT_STA_DISCONNECTED)) {
        ESP_LOGI(TAG, "Received event_id = WIFI_EVENT_STA_DISCONNECTED");
        ESP_LOGI(TAG, "Reconnecting...");
        esp_wifi_connect();
    } else if ((event_base == IP_EVENT) && (event_id == IP_EVENT_STA_GOT_IP)) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Received event_id = IP_EVENT_STA_GOT_IP");
        ESP_LOGI(TAG, "IP-address: " IPSTR, IP2STR(&event->ip_info.ip));
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

    // TODO: Determine proper values for our usage scenario
    ret = mpu6050_config(mpu6050, ACCE_FS_4G, GYRO_FS_500DPS);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "mpu6050_config() returned %d", ret);
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

static esp_err_t qmc5883p_write_byte(uint8_t reg_addr, uint8_t value)
{
    esp_err_t ret;
    i2c_cmd_handle_t cmd;

    cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (QMC5883P_DEFAULT_ADDR << 1) | I2C_MASTER_WRITE,
                          I2C_ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_addr, I2C_ACK_CHECK_EN);
    i2c_master_write_byte(cmd, value, I2C_ACK_CHECK_EN);
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));

    i2c_cmd_link_delete(cmd);

    return ret;
}

static esp_err_t
qmc5883p_read_bytes(uint8_t start_reg, uint8_t *data_buf, size_t len)
{
    esp_err_t ret;
    i2c_cmd_handle_t cmd;

    if (len == 0) return ESP_OK;

    cmd = i2c_cmd_link_create();

    // 1. Select the register to be read
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (QMC5883P_DEFAULT_ADDR << 1) | I2C_MASTER_WRITE,
                          I2C_ACK_CHECK_EN);
    i2c_master_write_byte(cmd, start_reg, I2C_ACK_CHECK_EN);

    // 2. Re-start to read the data
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (QMC5883P_DEFAULT_ADDR << 1) | I2C_MASTER_READ,
                          I2C_ACK_CHECK_EN);

    // 3. Read len-1 bytes with ACK-reply
    if (len > 1) i2c_master_read(cmd, data_buf, len - 1, I2C_MASTER_ACK);
    // 4. The last byte must be read with NACK in order to complete the transfer
    i2c_master_read_byte(cmd, data_buf + len - 1, I2C_MASTER_NACK);

    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));

    i2c_cmd_link_delete(cmd);

    return ret;
}

static esp_err_t qmc5883p_init(void)
{
    uint8_t mag_chip_id;
    esp_err_t ret;

    ret = qmc5883p_read_bytes(QMC5883P_REG_CHIPID, &mag_chip_id, 1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Can't find the magnetometer on i2c bus.");
        return ret;
    }
    ESP_LOGI(TAG, "mag_chip_id = 0x%x", mag_chip_id);

    ret = qmc5883p_write_byte(QMC5883P_REG_CONTROL2, QMC5883P_SETRESET_SETONLY);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "qmc5883p_write_byte() returned %d", ret);
        return ret;
    }

    vTaskDelay(pdMS_TO_TICKS(10)); // Wait for the reset

    // TODO: Determine proper values for our usage scenario
    ret = qmc5883p_write_byte(QMC5883P_REG_CONTROL1, QMC5883P_MODE_NORMAL |
        (QMC5883P_ODR_50HZ << QMC5883P_ODR_SHIFT) |
        (QMC5883P_OSR1_2 << QMC5883P_OSR1_SHIFT));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "qmc5883p_write_byte() returned %d", ret);
        return ret;
    }

    vTaskDelay(pdMS_TO_TICKS(10)); // Just in case

    return ESP_OK;
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

static void nvs_save_mag_cal(nvs_handle_t mag_nvs_handle,
                             int16_t mag_x_min, int16_t mag_x_max,
                             int16_t mag_y_min, int16_t mag_y_max,
                             int16_t mag_z_min, int16_t mag_z_max)
{
    esp_err_t ret;

    ESP_LOGI(TAG, "mag_x_min = %d mag_x_max = %d",
             mag_x_min, mag_x_max);
    ESP_LOGI(TAG, "mag_y_min = %d mag_y_max = %d",
             mag_y_min, mag_y_max);
    ESP_LOGI(TAG, "mag_z_min = %d mag_z_max = %d",
             mag_z_min, mag_z_max);

    nvs_set_i16(mag_nvs_handle, "mag_x_min", mag_x_min);
    nvs_set_i16(mag_nvs_handle, "mag_x_max", mag_x_max);

    nvs_set_i16(mag_nvs_handle, "mag_y_min", mag_y_min);
    nvs_set_i16(mag_nvs_handle, "mag_y_max", mag_y_max);

    nvs_set_i16(mag_nvs_handle, "mag_z_min", mag_z_min);
    nvs_set_i16(mag_nvs_handle, "mag_z_max", mag_z_max);

    ret = nvs_commit(mag_nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_commit() returned %d", ret);
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

    // Magnetometer
    bool button_pressed = false;
    bool button_pressed_prev = false;
    bool static_calib = false;
    bool auto_calib_changed = false;
    uint32_t current_time;
    uint32_t static_calib_changed_time;
    uint32_t auto_calib_changed_time = 0;
    uint8_t mag_buffer[6];
    int16_t mag_x, mag_y, mag_z;
    uint8_t mag_sts;
    /*float heading;
    float heading_degrees;*/
    int16_t mag_x_min = SHRT_MAX;
    int16_t mag_x_max = SHRT_MIN;
    int16_t mag_y_min = SHRT_MAX;
    int16_t mag_y_max = SHRT_MIN;
    int16_t mag_z_min = SHRT_MAX;
    int16_t mag_z_max = SHRT_MIN;
    float x_offset, y_offset, z_offset;
    float x_delta, y_delta, z_delta;
    float cal_x, cal_y, cal_z;
    nvs_handle_t mag_nvs_handle;

    // Net
    int sock, err;
    struct sockaddr_in dest_addr;
    imu_packet_t packet;

    // Etc.
    esp_err_t ret;
    int loop_count = 0;
    bool configured = false;
    /*const float dt = (float)MAIN_LOOP_PERIOD_MS / 1000;
    float x_roll_acc, y_pitch_acc;*/
    float x_roll, y_pitch, z_yaw;
    float gx_rad, gy_rad, gz_rad;

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

    ret = qmc5883p_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "qmc5883p_init() returned %d", ret);
        goto main_loop;
    }

    ret = nvs_flash_init();
    if ((ret != ESP_OK) || (ret == ESP_ERR_NVS_NO_FREE_PAGES) ||
        (ret == ESP_ERR_NVS_NEW_VERSION_FOUND)) {
      ESP_LOGE(TAG, "nvs_flash_init() returned %d", ret);
    }

    ret = nvs_open(QMC5883P_NVS_NAMESPACE, NVS_READWRITE, &mag_nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open() returned %d (%s)", ret, esp_err_to_name(ret));
    }

    nvs_get_i16(mag_nvs_handle, "mag_x_min", &mag_x_min);
    nvs_get_i16(mag_nvs_handle, "mag_x_max", &mag_x_max);

    nvs_get_i16(mag_nvs_handle, "mag_y_min", &mag_y_min);
    nvs_get_i16(mag_nvs_handle, "mag_y_max", &mag_y_max);

    nvs_get_i16(mag_nvs_handle, "mag_z_min", &mag_z_min);
    nvs_get_i16(mag_nvs_handle, "mag_z_max", &mag_z_max);

    ESP_LOGI(TAG, "Calibrated values (loaded):");

    ESP_LOGI(TAG, "mag_x_min = %d mag_x_max = %d", mag_x_min, mag_x_max);
    ESP_LOGI(TAG, "mag_y_min = %d mag_y_max = %d", mag_y_min, mag_y_max);
    ESP_LOGI(TAG, "mag_z_min = %d mag_z_max = %d", mag_z_min, mag_z_max);

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

        ret = qmc5883p_read_bytes(QMC5883P_REG_OUT, mag_buffer, 6);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "qmc5883p_read_bytes returned %d (%s)",
                     ret, esp_err_to_name(ret));
        }

        ret = qmc5883p_read_bytes(QMC5883P_REG_STATUS, &mag_sts, 1);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "qmc5883p_read_bytes returned %d (%s)",
                     ret, esp_err_to_name(ret));
        }

        mag_x = (int16_t)((mag_buffer[1] << 8) | mag_buffer[0]);
        mag_y = (int16_t)((mag_buffer[3] << 8) | mag_buffer[2]);
        mag_z = (int16_t)((mag_buffer[5] << 8) | mag_buffer[4]);

        //ESP_LOGI(TAG, "mag_x = %d mag_y = %d mag_z = %d", mag_x, mag_y, mag_z);
        //ESP_LOGI(TAG, "mag_sts = 0x%x", mag_sts);

        if (!static_calib) {
            if (mag_x < mag_x_min) { mag_x_min = mag_x; auto_calib_changed = true; }
            if (mag_x > mag_x_max) { mag_x_max = mag_x; auto_calib_changed = true; }

            if (mag_y < mag_y_min) { mag_y_min = mag_y; auto_calib_changed = true; }
            if (mag_y > mag_y_max) { mag_y_max = mag_y; auto_calib_changed = true; }

            if (mag_z < mag_z_min) { mag_z_min = mag_z; auto_calib_changed = true; }
            if (mag_z > mag_z_max) { mag_z_max = mag_z; auto_calib_changed = true; }

            current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
            if (auto_calib_changed &&
                (current_time - auto_calib_changed_time >
                    QMC5883P_AUTO_CALIB_PERIOD_MS)) {
                ESP_LOGI(TAG, "Calibrated values (auto):");

                nvs_save_mag_cal(mag_nvs_handle,
                                 mag_x_min, mag_x_max,
                                 mag_y_min, mag_y_max,
                                 mag_z_min, mag_z_max);

                auto_calib_changed_time = current_time;
                auto_calib_changed = false;
            }

            x_offset = (mag_x_max + mag_x_min) / 2.0;
            y_offset = (mag_y_max + mag_y_min) / 2.0;
            z_offset = (mag_z_max + mag_z_min) / 2.0;
            x_delta  = (mag_x_max - mag_x_min) / 2.0;
            y_delta  = (mag_y_max - mag_y_min) / 2.0;
            z_delta  = (mag_z_max - mag_z_min) / 2.0;

            if (x_delta == 0) x_delta = 1;
            if (y_delta == 0) y_delta = 1;
            if (z_delta == 0) z_delta = 1;

            cal_x = (mag_x - x_offset) / x_delta;
            cal_y = (mag_y - y_offset) / y_delta;
            cal_z = (mag_z - z_offset) / y_delta;

            /*heading = atan2(cal_y, cal_x);
            heading_degrees = heading * (180.0 / M_PI);
            if (heading_degrees < 0) {
                heading_degrees += 360.0;
            }*/

            //ESP_LOGI(TAG, "heading_degrees = %3.1f°", heading_degrees);

            ret = mpu6050_get_acce(mpu6050, &acce);
            if (ret != ESP_OK) continue;

            /*ESP_LOGI(TAG, "acce_x = %.2f acce_y = %.2f acce_z = %.2f",
                     acce.acce_x, acce.acce_y, acce.acce_z); // g*/

            ret = mpu6050_get_gyro(mpu6050, &gyro);
            if (ret != ESP_OK) continue;

            /*ESP_LOGI(TAG, "gyro_x = %.2f gyro_x = %.2f gyro_z = %.2f",
                     gyro.gyro_x, gyro.gyro_y, gyro.gyro_z); // °/s*/

            ret = mpu6050_get_temp(mpu6050, &temp);
            if (ret != ESP_OK) continue;

            //ESP_LOGI(TAG, "temp = %.2f", temp.temp); // °C

            /* Legacy calculations */

            /*x_roll_acc  = atan2f(acce.acce_y, acce.acce_z) * RAD_TO_DEG;
            y_pitch_acc = atan2f(acce.acce_x, sqrtf(acce.acce_y * acce.acce_y +
                                                    acce.acce_z * acce.acce_z))
                          * RAD_TO_DEG;*/

            /*ESP_LOGI(TAG, "x_roll_acc = %.2f y_pitch_acc = %.2f",
                     x_roll_acc, y_pitch_acc); // °*/

            /*x_roll  = 0.98f * (x_roll  + gyro.gyro_x * dt) + 0.02f * x_roll_acc;
            y_pitch = 0.98f * (y_pitch + gyro.gyro_y * dt) +
                      0.02f * y_pitch_acc;*/

            // Software z_yaw
            /*z_yaw = z_yaw + gyro.gyro_z * dt;
            if (z_yaw >= 360.0f) z_yaw -= 360.0f;
            if (z_yaw < 0.0f)    z_yaw += 360.0f;*/

            /*ESP_LOGI(TAG, "x_roll = %.1f° y_pitch = %.1f° z_yaw = %.1f°",
                     x_roll, y_pitch, z_yaw);*/


            gx_rad = gyro.gyro_x * DEG_TO_RAD;
            gy_rad = gyro.gyro_y * DEG_TO_RAD;
            gz_rad = gyro.gyro_z * DEG_TO_RAD;

            MadgwickAHRSupdate(gx_rad, gy_rad, gz_rad,
                               acce.acce_x, acce.acce_y, acce.acce_z,
                               cal_x, cal_y, -cal_z); // NB: -cal_z

            x_roll = atan2f(2.0f * (q0 * q1 + q2 * q3),
                            q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3) *
                     RAD_TO_DEG;
            y_pitch = -asinf(2.0f * (q1 * q3 - q0 * q2)) * RAD_TO_DEG;
            z_yaw = atan2f(2.0f * (q1 * q2 + q0 * q3),
                           q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3) *
                    RAD_TO_DEG;

            /*ESP_LOGI(TAG, "x_roll = %.1f° y_pitch = %.1f° z_yaw = %.1f°",
                     x_roll, y_pitch, z_yaw);*/

            packet.temp = temp.temp;
            packet.x_roll = x_roll;
            packet.y_pitch = y_pitch;
            packet.z_yaw = z_yaw;

            err = sendto(sock, &packet, sizeof(packet), 0,
                         (struct sockaddr *)&dest_addr, sizeof(dest_addr));
            if (err < 0) {
                //ESP_LOGE(TAG, "sendto() returned %d, errno = %d", err, errno);
                led_strip_set_pixel(led_strip, 0, 0, 0, 16); // Blue
            } else {
                led_strip_set_pixel(led_strip, 0, 0, 16, 0); // Green
            }
        } else { // Calibrating
            if (mag_x < mag_x_min) mag_x_min = mag_x;
            if (mag_x > mag_x_max) mag_x_max = mag_x;
            if (mag_y < mag_y_min) mag_y_min = mag_y;
            if (mag_y > mag_y_max) mag_y_max = mag_y;
            if (mag_z < mag_z_min) mag_z_min = mag_z;
            if (mag_z > mag_z_max) mag_z_max = mag_z;

            led_strip_set_pixel(led_strip, 0, 16, 16, 16); // White
        }

loop_count_inc:
        loop_count++;

        if (loop_count == ((1000 / MAIN_LOOP_PERIOD_MS) /
                           LED_CHANGES_PER_SECOND)) {
            blink_led(led_strip, led_state);
            led_state = !led_state;
            loop_count = 0;
        }

        button_pressed = button_check();

        if (button_pressed && button_pressed_prev) {
            current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
            if (!static_calib) {
                if (current_time - static_calib_changed_time >
                    QMC5883P_MIN_CALIB_TIME_MS) {
                    ESP_LOGI(TAG, "Calibrating...");

                    mag_x_min = SHRT_MAX;
                    mag_x_max = SHRT_MIN;

                    mag_y_min = SHRT_MAX;
                    mag_y_max = SHRT_MIN;

                    mag_z_min = SHRT_MAX;
                    mag_z_max = SHRT_MIN;

                    static_calib = true;
                    static_calib_changed_time = current_time;
                }
            } else { // Calibrating already
                if (current_time - static_calib_changed_time >
                    QMC5883P_MIN_CALIB_TIME_MS) {
                    ESP_LOGI(TAG, "Calibrated values (static):");

                    nvs_save_mag_cal(mag_nvs_handle,
                                     mag_x_min, mag_x_max,
                                     mag_y_min, mag_y_max,
                                     mag_z_min, mag_z_max);

                    static_calib = false;
                    static_calib_changed_time = current_time;
                }
            }
        }

        button_pressed_prev = button_pressed;

        vTaskDelay(pdMS_TO_TICKS(MAIN_LOOP_PERIOD_MS));
    }
}

