#include "buzzer.h"

#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "pwm.h"
#include "timer.h"
static const char* TAG = "BUZZER";

esp_err_t buzzer_config(int buzzer_pin) {
  return pwm_config(LEDC_TIMER_3, LEDC_TIMER_10_BIT, 2700, LEDC_AUTO_CLK,
                    LEDC_CHANNEL_7, buzzer_pin);
}

esp_err_t buzzer_once(uint64_t time_us, uint8_t percentage) {
  esp_err_t error = pwm_set_duty(LEDC_CHANNEL_7, LEDC_TIMER_10_BIT, percentage);
  if (error != ESP_OK) {
    return error;
  }

  static esp_timer_handle_t handle;
  void callback(void* arg) {
    esp_timer_handle_t* timer = arg;
    ESP_ERROR_CHECK(ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_7, 50));
    esp_err_t err = esp_timer_delete(*timer);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Timer Delete Failed: %s", esp_err_to_name(err));
    }
  }

  ESP_ERROR_CHECK(
      set_timeout("buzzer_once", callback, &handle, &handle, time_us));
  return ESP_OK;
}

static void alarm_task(void* arg) {
  buzzer_params_t* alarm = arg;
  while (1) {
    ESP_ERROR_CHECK(
        pwm_set_duty(LEDC_CHANNEL_7, LEDC_TIMER_10_BIT, alarm->percentage));
    vTaskDelay(alarm->interval_time / portTICK_PERIOD_MS);
    ESP_ERROR_CHECK(pwm_stop(LEDC_CHANNEL_7));
    vTaskDelay(alarm->interval_time / portTICK_PERIOD_MS);
  }
}

buzzer_params_t* buzzer_alarm(uint64_t interval_time_ms, uint8_t percentage) {
  TaskHandle_t xtask_id;
  buzzer_params_t* alarm = malloc(sizeof(buzzer_params_t));
  alarm->percentage = percentage;
  alarm->interval_time = interval_time_ms;
  xTaskCreate(&alarm_task, "alarm_task", 2048, alarm, 10, &xtask_id);
  alarm->xtask_id = xtask_id;
  return alarm;
}

esp_err_t buzzer_close(buzzer_params_t* alarm) {
  vTaskDelete(alarm->xtask_id);
  free(alarm);
  return pwm_stop(LEDC_CHANNEL_7);
}