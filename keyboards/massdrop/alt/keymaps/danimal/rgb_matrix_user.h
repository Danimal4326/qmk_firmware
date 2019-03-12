#pragma once

#define MAX_LED_BOOST_DECAY         1.0
#define MIN_LED_BOOST_DECAY         0.0
#define LED_BOOST_DECAY_STEP        0.01

#define MAX_LED_BOOST_PROPAGATE     1.0
#define MIN_LED_BOOST_PROPAGATE     0.0
#define LED_BOOST_PROPAGATE_STEP    0.01

#define LED_BOOST_REFRESH_STEP      5
#define MAX_LED_BOOST_REFRESH       100
#define MIN_LED_BOOST_REFRESH       5

float       led_boost_decay;
float       led_boost_propagate;
uint8_t     led_boost_refresh_ms;
uint32_t    underglow_rgb;

void rgb_matrix_record_key_press(keyrecord_t *record);
