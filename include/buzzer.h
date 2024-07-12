#pragma once
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef void (*buzzer_alarm_callback)(void* arg);
typedef struct {
  uint64_t duration;   //@param duration buzzer beep duration uint: ms
  uint8_t percentage;  //@param percentage 0-100 sound volume
  uint64_t interval;   //@param interval time(ms) interval between beeps
  TaskHandle_t task_handle;
  int64_t start;
  uint64_t end;  //@param timeout After how many seconds to close. If it is  0,
                 // it will continue until you manually close it.
  void* callback_arg;              // @param arg it will be pass to callback
  buzzer_alarm_callback callback;  //@param callback callback function
} buzzer_alarm_t;

esp_err_t buzzer_init(int buzzer_pin);
esp_err_t buzzer_beep(uint64_t duration, uint8_t percentage);
// buzzer_alarm_t* buzzer_alarm(uint64_t duration, uint64_t interval,
//                              uint8_t percentage, uint64_t timeout, void* arg,
//                              buzzer_alarm_callback callback);
void buzzer_alarm(buzzer_alarm_t* alarm);
esp_err_t buzzer_close(buzzer_alarm_t* alarm);