#include "buzzer.h"

#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "pwm.h"
#include "time.h"
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
/**
 * @brief
 * @param duration buzzer beep duration uint: us
 * @param percentage 0-100 sound volume
 */
esp_err_t buzzer_beep(uint64_t duration, uint8_t percentage) {
  esp_err_t error = pwm_set_duty(LEDC_CHANNEL_7, LEDC_TIMER_10_BIT, percentage);
  if (error != ESP_OK) {
    return error;
  }

  static esp_timer_handle_t handle;

  ESP_ERROR_CHECK(
      set_timeout("buzzer_beep", callback, &handle, &handle, duration));
  return ESP_OK;
}

static void alarm_task(void* arg) {
  buzzer_alarm_t* alarm = (buzzer_alarm_t*)arg;
  int64_t start = time_current_us();

  while (1) {
    if (alarm->timeout != 0 &&
        time_current_us() - start > alarm->timeout * 1000 * 1000) {
      if (alarm->callback != NULL) {
        alarm->callback(alarm->arg);
      }
      vTaskDelete(NULL);
    }
    pwm_set_duty(LEDC_CHANNEL_7, LEDC_TIMER_10_BIT, alarm->percentage);
    vTaskDelay(alarm->duration / portTICK_PERIOD_MS);
    pwm_stop(LEDC_CHANNEL_7);
    vTaskDelay(alarm->interval / portTICK_PERIOD_MS);
  }
}

/**
 * @brief
 * @param duration buzzer beep duration uint: ms
 * @param interval time(ms) interval between beeps
 * @param percentage 0-100 sound volume
 * @param timeout After how many milliseconds to close. If it is 0, it will
 * continue until you manually close it.
 * @param arg
 * @note When using automatic shutdown, you need to free buzzer_alarm_t
 */
buzzer_alarm_t* buzzer_alarm(uint64_t duration, uint64_t interval,
                             uint8_t percentage, uint64_t timeout, void* arg,
                             buzzer_alarm_callback callback) {
  buzzer_alarm_t* alarm = malloc(sizeof(buzzer_alarm_t));
  alarm->duration = duration;
  alarm->percentage = percentage;
  alarm->interval = interval;
  alarm->arg = arg;
  alarm->callback = callback;
  alarm->task_handle = NULL;
  alarm->timeout = timeout;
  xTaskCreate(alarm_task, "alarm_task", 2048, alarm, 10, &alarm->task_handle);
  return alarm;
}

esp_err_t buzzer_close(buzzer_alarm_t* alarm) {
  eTaskState state = eTaskGetState(alarm->task_handle);
  if (state < 4) {
    vTaskDelete(alarm->task_handle);
  }
  free(alarm);
  return pwm_stop(LEDC_CHANNEL_7);
}