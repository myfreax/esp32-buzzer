#pragma once
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef void (*buzzer_alarm_callback)(void* arg);
typedef struct {
  uint64_t duration;
  uint8_t percentage;
  uint64_t interval;
  TaskHandle_t task_handle;
  uint64_t timeout;
  void* arg;
  buzzer_alarm_callback callback;
} buzzer_alarm_t;

esp_err_t buzzer_init(int buzzer_pin);
esp_err_t buzzer_beep(uint64_t duration, uint8_t percentage);
buzzer_alarm_t* buzzer_alarm(uint64_t duration, uint64_t interval,
                             uint8_t percentage, uint64_t timeout, void* arg,
                             buzzer_alarm_callback callback);
esp_err_t buzzer_close(buzzer_alarm_t* alarm);