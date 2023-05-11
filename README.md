# ESP32 Buzzer Component
Components Used To Drive Passive Electromagnetic Buzzer

## Dependences
- [ESP32 Timer Component](https://github.com/myfreax/esp32-timer)

## Usage
```
git submodule add git@github.com:myfreax/esp32-buzzer.git components/buzzer
```
```c
#include "buzzer.h"

void app_main(void) {
  ESP_ERROR_CHECK(buzzer_config(gpio_pin));
  buzzer_once(100 * 1000);
}
```