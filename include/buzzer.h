#pragma once
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
typedef struct {
  uint8_t percentage;
  uint64_t interval;
  TaskHandle_t* task_handle;
} buzzer_alarm_t;

esp_err_t buzzer_init(int buzzer_pin);
esp_err_t buzzer_beep(uint64_t time_us, uint8_t percentage);
buzzer_alarm_t* buzzer_alarm(uint64_t interval_time_ms, uint8_t percentage);
esp_err_t buzzer_close(buzzer_alarm_t* alarm);