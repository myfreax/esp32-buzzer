idf_component_register(
  SRCS "buzzer.c"
  INCLUDE_DIRS "include"
  REQUIRES timer driver pwm datetime
)