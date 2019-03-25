#include "quantum.h"
#include "led_matrix.h"

extern issi3733_led_t *led_cur;
extern uint8_t led_per_run;
extern issi3733_led_t *lede;
extern issi3733_led_t led_map[];

static uint16_t last_boost_update;
static uint8_t led_boosts[ISSI3733_LED_COUNT];
static uint8_t led_boost_index;
static uint8_t led_cur_index;

float led_boost_decay = 0.87;
float led_boost_propagate = 0.45;
uint8_t led_boost_refresh_ms = 50;

#define LED_BOOST_REFRESH_INTERVAL_IN_MS 40
#define LED_BOOST_DECAY 0.8
#define LED_BOOST_PROPAGATE 0.5
#define LED_BOOST_PEAK 100

#define MIN_RGB 0x000000
#define MIN_R (MIN_RGB >> 16 & 0xff)
#define MIN_G (MIN_RGB >> 8 & 0xff)
#define MIN_B (MIN_RGB & 0xff)

#define MAX_RGB 0xff0000
#define MAX_R (MAX_RGB >> 16 & 0xff)
#define MAX_G (MAX_RGB >> 8 & 0xff)
#define MAX_B (MAX_RGB & 0xff)
uint8_t max_r = (MAX_RGB >> 16 & 0xff);
uint8_t max_g = (MAX_RGB >> 8 & 0xff);
uint8_t max_b = (MAX_RGB & 0xff);

uint32_t underglow_rgb = 0x000000;
#define UNDERGLOW_RGB 0x000000
#define UNDERGLOW_R (underglow_rgb >> 16 & 0xff)
#define UNDERGLOW_G (underglow_rgb >> 8 & 0xff)
#define UNDERGLOW_B (underglow_rgb & 0xff)

#define UNDERGLOW_SCAN_CODE 255

uint8_t layer = 0;
uint8_t mod_layer = 0;

#define max(a, b) (((a) > (b)) ? (a) : (b))

#define __ -1
// static const uint8_t KEY_TO_LED_MAP[MATRIX_ROWS][MATRIX_COLS] = {
//   { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14},
//   {15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29},
//   {30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, __, 42, 43},
//   {44, __, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57},
//   {58, 59, 60, __, __, __, 61, __, __, __, 62, 63, 64, 65, 66},
// };

static const uint8_t KEY_TO_LED_MAP[MATRIX_ROWS][MATRIX_COLS] = {
  { __,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, __, __},
  {__, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, __},
  {__, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, __, __, __},
  {__, __, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, __, __, __},
  {__, __, __, __, __, __, 61, __, __, __, __, __, __, __, __},
};

typedef struct rbg_s {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    bool active;
} rgb_t;

#define RGB(x, y, z) { .r = x, .g = y, .b = z, .active = 1}
#define OFF {.active = 0}
#define ___ {.active = 0}
#define RED RGB(0x90, 0x00, 0x00)
#define BLU RGB(0x00, 0x00, 0x80)
#define GRN RGB(0x00, 0x80, 0x00)
#define YEL RGB(0x80, 0x80, 0x00)
#define ORN RGB(0x70, 0x20, 0x00)
#define PUR RGB(0x80, 0x00, 0x80)
#define CYA RGB(0x00, 0x80, 0x80)
#define WHT RGB(0xB0, 0xB0, 0xB0)

static const rgb_t CONST_LED_MAP[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = {
    { RED,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  BLU,  BLU }, 
    { BLU,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  BLU },
    { RED,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  ___,  PUR,  BLU },
    { BLU,  ___,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  BLU,  YEL,  BLU },
    { RED,  RED,  RED,  ___,  ___,  ___,  OFF,  ___,  ___,  ___,  RED,  WHT,  YEL,  YEL,  YEL }
    },
    [1] = {
    { BLU,  RED,  RED,  RED,  RED,  RED,  RED,  RED,  RED,  RED,  RED,  RED,  RED,  OFF,  PUR }, 
    { OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  ORN,  ORN,  ORN,  OFF,  BLU },
    { OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  ___,  OFF,  GRN },
    { OFF,  ___,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  PUR,  OFF },
    { OFF,  OFF,  OFF,  ___,  ___,  ___,  CYA,  ___,  ___,  ___,  WHT,  WHT,  CYA,  PUR,  CYA }
    },
    [2] = {
    { OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF }, 
    { OFF,  OFF,  YEL,  OFF,  YEL,  YEL,  YEL,  RED,  RED,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF },
    { OFF,  OFF,  BLU,  OFF,  BLU,  BLU,  BLU,  OFF,  OFF,  OFF,  OFF,  OFF,  ___,  OFF,  OFF },
    { OFF,  ___,  OFF,  CYA,  OFF,  OFF,  ORN,  GRN,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF },
    { OFF,  OFF,  OFF,  ___,  ___,  ___,  OFF,  ___,  ___,  ___,  OFF,  OFF,  OFF,  OFF,  OFF }
    },
    [3] = {
    { OFF,  CYA,  CYA,  CYA,  CYA,  CYA,  CYA,  CYA,  CYA,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF }, 
    { RED,  OFF,  GRN,  GRN,  PUR,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF },
    { GRN,  YEL,  GRN,  RED,  PUR,  OFF,  PUR,  YEL,  YEL,  YEL,  YEL,  OFF,  ___,  RED,  OFF },
    { WHT,  ___,  RED,  OFF,  OFF,  PUR,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  WHT,  YEL,  OFF },
    { OFF,  WHT,  WHT,  ___,  ___,  ___,  YEL,  ___,  ___,  ___,  WHT,  OFF,  YEL,  YEL,  YEL }
    },
    [4] = {
    { OFF,  BLU,  BLU,  BLU,  BLU,  BLU,  BLU,  BLU,  BLU,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF }, 
    { OFF,  RED,  OFF,  RED,  RED,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF },
    { PUR,  YEL,  OFF,  OFF,  OFF,  OFF,  OFF,  BLU,  BLU,  BLU,  BLU,  OFF,  ___,  OFF,  OFF },
    { WHT,  ___,  OFF,  OFF,  RED,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  WHT,  BLU,  OFF },
    { OFF,  WHT,  OFF,  ___,  ___,  ___,  GRN,  ___,  ___,  ___,  OFF,  OFF,  BLU,  BLU,  BLU }
    },
    [5] = {
    { OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF }, 
    { OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF },
    { OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  RED,  OFF,  OFF,  ___,  OFF,  OFF },
    { OFF,  ___,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF,  OFF },
    { OFF,  WHT,  WHT,  ___,  ___,  ___,  OFF,  ___,  ___,  ___,  OFF,  OFF,  OFF,  OFF,  OFF }
    },
};

#define KEY_LED_COUNT 67
#define KP(c, r) { .col = c, .row = r } // shorthand for keypos_t
static const keypos_t LED_TO_KEY_MAP[KEY_LED_COUNT] = {
  KP(0, 0), KP(1, 0), KP(2, 0), KP(3, 0), KP(4, 0), KP(5, 0), KP(6, 0), KP(7, 0), KP(8, 0), KP(9, 0), KP(10, 0), KP(11, 0), KP(12, 0), KP(13, 0), KP(14, 0),
  KP(0, 1), KP(1, 1), KP(2, 1), KP(3, 1), KP(4, 1), KP(5, 1), KP(6, 1), KP(7, 1), KP(8, 1), KP(9, 1), KP(10, 1), KP(11, 1), KP(12, 1), KP(13, 1), KP(14, 1),
  KP(0, 2), KP(1, 2), KP(2, 2), KP(3, 2), KP(4, 2), KP(5, 2), KP(6, 2), KP(7, 2), KP(8, 2), KP(9, 2), KP(10, 2), KP(11, 2),            KP(13, 2), KP(14, 2),
  KP(0, 3),           KP(2, 3), KP(3, 3), KP(4, 3), KP(5, 3), KP(6, 3), KP(7, 3), KP(8, 3), KP(9, 3), KP(10, 3), KP(11, 3), KP(12, 3), KP(13, 3), KP(14, 3),
  KP(0, 4), KP(1, 4), KP(2, 4),                               KP(6, 4),                               KP(10, 4), KP(11, 4), KP(12, 4), KP(13, 4), KP(14, 4),
};


static void update_led_boosts(void);
static void update_led_cur_rgb_values(void);
static void set_nearest_led_to_max(uint8_t col, uint8_t row);
static uint8_t calculate_new_color_component_value(uint8_t max, uint8_t min);
static void calculate_new_led_boosts(uint8_t new_led_boosts[]);
static uint8_t calculate_new_led_boost_at(int index);
static uint8_t get_propagated_boost_from_neighbors(int led_position);
static uint8_t get_led_boost_at_keypos(uint8_t row, uint8_t col);
static void set_new_led_boosts(uint8_t* new_led_boosts);
static uint8_t map_key_position_to_led_index(uint8_t col, uint8_t row);


void rgb_matrix_init_user(void) {
  for (int i = 0; i < ISSI3733_LED_COUNT; i++) {
    led_boosts[i] = 0;
  }
  last_boost_update = timer_read();
  led_boost_index = 0;
  led_cur_index = 0;
}

void led_matrix_run(void) {
  uint8_t led_this_run = 0;

  if (led_cur == 0) { //Denotes start of new processing cycle in the case of chunked processing
    led_cur = led_map;
    led_cur_index = 0;
  }
  update_led_boosts();

  while (led_cur < lede && led_this_run < led_per_run) {
    update_led_cur_rgb_values();

    led_cur++;
    led_cur_index++;
    led_this_run++;
  }
}

void rgb_matrix_record_key_press(uint16_t keycode, keyrecord_t *record) {
  uint8_t mods = get_mods();
  if (record->event.pressed) {
     switch (keycode) {
      case KC_LGUI:
      case KC_RGUI:
        if (!mods)
          mod_layer = 3;
        else if ( (mods & MOD_MASK_SHIFT) && !(mods & ~MOD_MASK_SHIFT) )
          mod_layer = 4;
        else if ( (mods & MOD_MASK_ALT) && !(mods & ~MOD_MASK_ALT) )
          mod_layer = 5;
        else
          mod_layer = 0;
        break;
     case KC_LSFT:
     case KC_RSFT:
        if ( (mods & MOD_MASK_GUI) && !(mods & ~MOD_MASK_GUI) )
          mod_layer = 4;
        else
          mod_layer = 0;
        break;
     case KC_LALT:
     case KC_RALT:
        if ( (mods & MOD_MASK_GUI) && !(mods & ~MOD_MASK_GUI) )
          mod_layer = 5;
        else
          mod_layer = 0;
        break;
      default:
        mod_layer = 0;
        break;
     }
  } else if ( IS_RELEASED(record->event) ) {
    mods = get_mods();
    switch (keycode) {
      case KC_LGUI:
      case KC_RGUI:
        mod_layer = 0;
        break;
      case KC_LSFT:
      case KC_RSFT:
        if ( (mods & MOD_MASK_GUI) && !(mods & ~(MOD_MASK_GUI | MOD_MASK_SHIFT) ) )
          mod_layer = 3;
        else if ( (mods & MOD_MASK_GUI) && (mods & MOD_MASK_ALT) )
          mod_layer = 5;
        break;
      case KC_LALT:
      case KC_RALT:
        if ( (mods & MOD_MASK_GUI) && !(mods & ~(MOD_MASK_GUI | MOD_MASK_ALT) ) )
          mod_layer = 3;
        else if ( (mods & MOD_MASK_GUI) && (mods & MOD_MASK_SHIFT) )
          mod_layer = 4;
        break;
      default:
        if ( (mods & MOD_MASK_GUI) && (mods & MOD_MASK_ALT) )
          mod_layer = 5;
        else if ( (mods & MOD_MASK_GUI) && (mods & MOD_MASK_SHIFT) )
          mod_layer = 4;
        else if ( (mods & MOD_MASK_GUI) )
          mod_layer = 3;
        break;
    }
  }

  if (record->event.pressed) {
    keypos_t key = record->event.key;
    set_nearest_led_to_max(key.col, key.row);
  }
}


static void update_led_boosts(void) {
  if (timer_elapsed(last_boost_update) > led_boost_refresh_ms) {
    last_boost_update = timer_read();

    uint8_t new_led_boosts[ISSI3733_LED_COUNT];
    calculate_new_led_boosts(new_led_boosts);
    set_new_led_boosts(new_led_boosts);
  }
}

static void update_led_cur_rgb_values(void) {
    keypos_t led_keypos = LED_TO_KEY_MAP[led_cur->scan];
    uint8_t  curr_layer;
    if (layer == 0)
      curr_layer = mod_layer;
    else
      curr_layer = layer; 

    if (led_cur->scan == UNDERGLOW_SCAN_CODE) {
        *led_cur->rgb.r = UNDERGLOW_R;
        *led_cur->rgb.g = UNDERGLOW_G;
        *led_cur->rgb.b = UNDERGLOW_B;
    } else if ( ( CONST_LED_MAP[curr_layer][led_keypos.row][led_keypos.col].active == 1 ) ) {
        *led_cur->rgb.r = CONST_LED_MAP[curr_layer][led_keypos.row][led_keypos.col].r;
        *led_cur->rgb.g = CONST_LED_MAP[curr_layer][led_keypos.row][led_keypos.col].g;
        *led_cur->rgb.b = CONST_LED_MAP[curr_layer][led_keypos.row][led_keypos.col].b;
    } else {
        *led_cur->rgb.r = calculate_new_color_component_value(max_r, MIN_R);
        *led_cur->rgb.g = calculate_new_color_component_value(max_g, MIN_G);
        *led_cur->rgb.b = calculate_new_color_component_value(max_b, MIN_B);
    }
}

static void set_nearest_led_to_max(uint8_t col, uint8_t row) {
  uint8_t led_index = map_key_position_to_led_index(col, row);
  if (led_index >= 0 && led_index < ISSI3733_LED_COUNT) {
    led_boosts[led_index] = LED_BOOST_PEAK;
  }
}

static uint8_t calculate_new_color_component_value(uint8_t max, uint8_t min) {
  uint8_t current_boost = led_boosts[led_cur_index];
  return (float)(max - min) * current_boost / LED_BOOST_PEAK + min;
}

static void calculate_new_led_boosts(uint8_t new_led_boosts[]) {
  for (int i = 0; i < ISSI3733_LED_COUNT; i++) {
    new_led_boosts[i] = calculate_new_led_boost_at(i);
  }
}

static uint8_t calculate_new_led_boost_at(int index) {
  uint8_t decayed_boost = led_boosts[index] * led_boost_decay;
  uint8_t propagated_boost = get_propagated_boost_from_neighbors(index);
  uint8_t new_boost = (propagated_boost > decayed_boost) ? propagated_boost : decayed_boost;
  if (new_boost > LED_BOOST_PEAK) {
    new_boost = LED_BOOST_PEAK;
  }
  return new_boost;
}

static uint8_t get_propagated_boost_from_neighbors(int led_position) {
  if (led_position < 0 || led_position >= KEY_LED_COUNT) {
    return 0;
  }
  keypos_t led_keypos = LED_TO_KEY_MAP[led_position];
  uint8_t top_boost    = get_led_boost_at_keypos(led_keypos.row - 1, led_keypos.col);
  uint8_t bottom_boost = get_led_boost_at_keypos(led_keypos.row + 1, led_keypos.col);
  uint8_t left_boost   = get_led_boost_at_keypos(led_keypos.row, led_keypos.col - 1);
  uint8_t right_boost  = get_led_boost_at_keypos(led_keypos.row, led_keypos.col + 1);
  uint8_t max_boost = max(max(top_boost, bottom_boost), max(left_boost, right_boost));
  if (max_boost > LED_BOOST_PEAK) {
    max_boost = LED_BOOST_PEAK;
  }
  return max_boost * led_boost_propagate;
}

static uint8_t get_led_boost_at_keypos(uint8_t row, uint8_t col) {
  if (row < 0 || row >= MATRIX_ROWS || col < 0 || col >= MATRIX_COLS) {
    return 0;
  }
  uint8_t led_index = KEY_TO_LED_MAP[row][col];
  if (led_index < 0) {
    return 0;
  }
  return led_boosts[led_index];
}

static void set_new_led_boosts(uint8_t* new_led_boosts) {
  for (int i = 0; i < ISSI3733_LED_COUNT; i++) {
    led_boosts[i] = new_led_boosts[i];
  }
}

static uint8_t map_key_position_to_led_index(uint8_t col, uint8_t row) {
  if (row >= 0 && row < MATRIX_ROWS && col >= 0 && col < MATRIX_COLS) {
    return KEY_TO_LED_MAP[row][col];
  }
  return -1;
}
