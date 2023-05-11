#pragma once
#include "esp_err.h"

esp_err_t buzzer_config(int buzzer_pin);
esp_err_t buzzer_once(uint64_t time_us);