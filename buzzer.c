#include "buzzer.h"

#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "timer.h"

static const char* TAG = "BUZZER";

esp_err_t buzzer_config(int buzzer_pin) {
  ledc_timer_config_t timer = {.speed_mode = LEDC_LOW_SPEED_MODE,
                               .timer_num = LEDC_TIMER_0,
                               .duty_resolution = LEDC_TIMER_10_BIT,
                               .freq_hz = 2700,
                               .clk_cfg = LEDC_AUTO_CLK};
  esp_err_t timer_err = ledc_timer_config(&timer);

  if (timer_err != ESP_OK) {
    return timer_err;
  }

  ledc_channel_config_t channel = {.speed_mode = LEDC_LOW_SPEED_MODE,
                                   .channel = LEDC_CHANNEL_0,
                                   .timer_sel = LEDC_TIMER_0,
                                   .intr_type = LEDC_INTR_DISABLE,
                                   .gpio_num = buzzer_pin,
                                   .duty = 0,
                                   .hpoint = 0};
  return ledc_channel_config(&channel);
}

esp_err_t buzzer_once(uint64_t time_us, uint8_t percentage) {
  esp_err_t ledc_set_duty_err = ledc_set_duty(
      LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0,
      (uint32_t)(((1 << LEDC_TIMER_10_BIT) - 1) * (percentage / 100.f)));
  if (ledc_set_duty_err != ESP_OK) {
    return ledc_set_duty_err;
  }

  esp_err_t ledc_update_duty_err =
      ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

  if (ledc_update_duty_err != ESP_OK) {
    return ledc_update_duty_err;
  }

  static esp_timer_handle_t handle;
  void callback(void* arg) {
    esp_timer_handle_t* timer = arg;
    ESP_ERROR_CHECK(ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 50));
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
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0,
                                  (uint32_t)(((1 << LEDC_TIMER_10_BIT) - 1) *
                                             (alarm->percentage / 100.f))));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
    vTaskDelay(alarm->interval_time / portTICK_PERIOD_MS);
    ESP_ERROR_CHECK(ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 50));
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
  return ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 50);
}