#pragma once
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
typedef struct {
  uint8_t percentage;
  uint64_t interval_time;
  TaskHandle_t xtask_id;
} buzzer_params_t;

esp_err_t buzzer_config(int buzzer_pin);
esp_err_t buzzer_once(uint64_t time_us, uint8_t percentage);
buzzer_params_t* buzzer_alarm(uint64_t interval_time_ms, uint8_t percentage);
esp_err_t buzzer_close(buzzer_params_t* alarm);