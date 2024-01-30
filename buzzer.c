#include "buzzer.h"

#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "pwm.h"
#include "timer.h"
static const char* TAG = "BUZZER";

static void callback(void* arg) {
  esp_timer_handle_t* timer = arg;
  ESP_ERROR_CHECK(ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_7, 50));
  esp_err_t err = esp_timer_delete(*timer);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Timer Delete Failed: %s", esp_err_to_name(err));
  }
}

esp_err_t buzzer_init(int buzzer_pin) {
  return pwm_config(LEDC_TIMER_3, LEDC_TIMER_10_BIT, 2700, LEDC_AUTO_CLK,
                    LEDC_CHANNEL_7, buzzer_pin);
}

esp_err_t buzzer_beep(uint64_t time_us, uint8_t percentage) {
  esp_err_t error = pwm_set_duty(LEDC_CHANNEL_7, LEDC_TIMER_10_BIT, percentage);
  if (error != ESP_OK) {
    return error;
  }

  static esp_timer_handle_t handle;

  ESP_ERROR_CHECK(
      set_timeout("buzzer_beep", callback, &handle, &handle, time_us));
  return ESP_OK;
}

static void alarm_task(void* arg) {
  buzzer_alarm_t* alarm = arg;
  while (1) {
    ESP_ERROR_CHECK(
        pwm_set_duty(LEDC_CHANNEL_7, LEDC_TIMER_10_BIT, alarm->percentage));
    vTaskDelay(alarm->interval / portTICK_PERIOD_MS);
    ESP_ERROR_CHECK(pwm_stop(LEDC_CHANNEL_7));
    vTaskDelay(alarm->interval / portTICK_PERIOD_MS);
  }
}

/**
 * @brief
 * @param interval ms
 * @param percentage 0-100
 */
buzzer_alarm_t* buzzer_alarm(uint64_t interval, uint8_t percentage) {
  buzzer_alarm_t* alarm = malloc(sizeof(buzzer_alarm_t));
  alarm->percentage = percentage;
  alarm->interval = interval;
  alarm->task_handle = malloc(sizeof(TaskHandle_t));
  xTaskCreate(&alarm_task, "alarm_task", 2048, alarm, 10, alarm->task_handle);
  return alarm;
}

esp_err_t buzzer_close(buzzer_alarm_t* alarm) {
  vTaskDelete(*alarm->task_handle);
  free(alarm);
  return pwm_stop(LEDC_CHANNEL_7);
}