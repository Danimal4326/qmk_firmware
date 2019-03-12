#include QMK_KEYBOARD_H
#include "rgb_matrix_user.h"

enum alt_keycodes {
    L_BRI = SAFE_RANGE, //LED Brightness Increase
    L_BRD,              //LED Brightness Decrease
    L_PTN,              //LED Pattern Select Next
    L_PTP,              //LED Pattern Select Previous
    L_PSI,              //LED Pattern Speed Increase
    L_PSD,              //LED Pattern Speed Decrease
    L_T_MD,             //LED Toggle Mode
    L_T_ONF,            //LED Toggle On / Off
    L_ON,               //LED On
    L_OFF,              //LED Off
    L_T_BR,             //LED Toggle Breath Effect
    L_T_PTD,            //LED Toggle Scrolling Pattern Direction
    U_T_AUTO,           //USB Extra Port Toggle Auto Detect / Always Active
    U_T_AGCR,           //USB Toggle Automatic GCR control
    DBG_TOG,            //DEBUG Toggle On / Off
    DBG_MTRX,           //DEBUG Toggle Matrix Prints
    DBG_KBD,            //DEBUG Toggle Keyboard Prints
    DBG_MOU,            //DEBUG Toggle Mouse Prints
    MD_BOOT,            //Restart into bootloader after hold timeout
    L_DCY_UP,           // LED Decay Increase
    L_DCY_DN,           // LED Decay Decrease
    L_PRP_UP,           // LED Propoagate Increase
    L_PRP_DN,           // LED Propoagate Decrease
    L_REF_UP,           // LED Refresh Speed Increase 
    L_REF_DN,           // LED Refresh Speed Decrease
};

#define TG_NKRO MAGIC_TOGGLE_NKRO //Toggle 6KRO / NKRO mode
#define ______ KC_TRNS
#define _BL 0
#define _FL 1
#define _KB 2

keymap_config_t keymap_config;

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BL] = LAYOUT(
       KC_GESC,        KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC,  KC_DEL,  \
       KC_TAB,         KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,  KC_BSLS,  KC_HOME, \
       CTL_T(KC_ESC),  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,            KC_ENT,   KC_PGUP, \
       KC_LSFT,        KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,  KC_RSFT,            KC_UP,    KC_PGDN, \
       KC_LCTL,        KC_LGUI,  KC_LALT,                                KC_SPC,                                 KC_RGUI,  MO(_FL),  KC_LEFT,  KC_DOWN,  KC_RGHT  \
    ),
    [_FL] = LAYOUT(
       KC_GRV,         KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,   ______,   KC_MUTE, \
       ______,         ______,   ______,   ______,   ______,   ______,   ______,   ______,   ______,   ______,   KC_PSCR,  KC_SLCK,  KC_PAUS,  ______,   KC_END, \
       ______,         ______,   ______,   ______,   ______,   ______,   ______,   ______,   ______,   ______,   ______,   ______,             ______,   LSFT(KC_INS), \
       ______,         ______,   ______,   ______,   ______,   ______,   ______,   ______,   ______,   ______,   ______,   ______,             KC_VOLU,  ______, \
       ______,         ______,   ______,                                 KC_MPLY,                                MO(_KB),  ______,   KC_MPRV,  KC_VOLD,  KC_MNXT  \
    ),
    [_KB] = LAYOUT(
       ______,         ______,  ______,    ______,   ______,   ______,   ______,   ______,   ______,   ______,   ______,   ______,   ______,   ______,   ______, \
       L_T_BR,         L_PTN,   L_BRI,     L_PSI,    L_DCY_UP, L_PRP_UP, L_REF_UP, U_T_AUTO, U_T_AGCR, ______,   ______,   ______,   ______,   ______,   ______, \
       L_T_PTD,        L_PTP,   L_BRD,     L_PSD,    L_DCY_DN, L_PRP_DN, L_REF_DN, ______,   ______,   ______,   ______,   ______,             ______,   ______, \
       ______,         L_T_MD,  L_T_ONF,   ______,   ______,   MD_BOOT,  TG_NKRO,  ______,   ______,   ______,   ______,   ______,             ______,   ______, \
       ______,         ______,  ______,                                  ______,                                 ______,   ______,   ______,   ______,   ______  \
    ),
};

const uint16_t PROGMEM fn_actions[] = {

};

// Runs just one time when the keyboard initializes.
void matrix_init_user(void) {
    // Set default brightness
    //gcr_desired = 32;
};

// Runs constantly in the background, in a loop.
void matrix_scan_user(void) {
};

#define MODS_SHIFT  (get_mods() & MOD_BIT(KC_LSHIFT)    || get_mods() & MOD_BIT(KC_RSHIFT)) 
#define MODS_CTRL   (get_mods() & MOD_BIT(KC_LCTL)      || get_mods() & MOD_BIT(KC_RCTRL))
#define MODS_ALT    (get_mods() & MOD_BIT(KC_LALT)      || get_mods() & MOD_BIT(KC_RALT))

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    static uint32_t key_timer;

    rgb_matrix_record_key_press(record);

    switch (keycode) {
       case L_DCY_UP:
            if (record->event.pressed) {
                if (LED_BOOST_DECAY_STEP > MAX_LED_BOOST_DECAY - led_boost_decay) led_boost_decay = MAX_LED_BOOST_DECAY;
                else led_boost_decay += LED_BOOST_DECAY_STEP;
            }
            return false;
       case L_DCY_DN:
            if (record->event.pressed) {
                if (LED_BOOST_DECAY_STEP > led_boost_decay) led_boost_decay = MIN_LED_BOOST_DECAY;
                else led_boost_decay -= LED_BOOST_DECAY_STEP;
            }
            return false;
       case L_PRP_UP:
            if (record->event.pressed) {
                if (LED_BOOST_PROPAGATE_STEP > MAX_LED_BOOST_PROPAGATE - led_boost_propagate) led_boost_propagate = MAX_LED_BOOST_PROPAGATE;
                else led_boost_propagate += LED_BOOST_PROPAGATE_STEP;
            }
            return false;
       case L_PRP_DN:
            if (record->event.pressed) {
                if (LED_BOOST_PROPAGATE_STEP > led_boost_propagate) led_boost_propagate = MIN_LED_BOOST_PROPAGATE;
                else led_boost_propagate -= LED_BOOST_PROPAGATE_STEP;
            }
            return false;
       case L_REF_UP:
            if (record->event.pressed) {
                if (LED_BOOST_REFRESH_STEP > MAX_LED_BOOST_REFRESH - led_boost_refresh_ms) led_boost_refresh_ms = MAX_LED_BOOST_REFRESH;
                else led_boost_refresh_ms += LED_BOOST_REFRESH_STEP;
            }
            return false;
       case L_REF_DN:
            if (record->event.pressed) {
                if (LED_BOOST_REFRESH_STEP > led_boost_refresh_ms) led_boost_refresh_ms = MIN_LED_BOOST_REFRESH;
                else led_boost_refresh_ms -= LED_BOOST_REFRESH_STEP;
            }
            return false;
       case L_BRI:
            if (record->event.pressed) {
                if (LED_GCR_STEP > LED_GCR_MAX - gcr_desired) gcr_desired = LED_GCR_MAX;
                else gcr_desired += LED_GCR_STEP;
                if (led_animation_breathing) gcr_breathe = gcr_desired;
            }
            return false;
        case L_BRD:
            if (record->event.pressed) {
                if (LED_GCR_STEP > gcr_desired) gcr_desired = 0;
                else gcr_desired -= LED_GCR_STEP;
                if (led_animation_breathing) gcr_breathe = gcr_desired;
            }
            return false;
        case L_PTN:
            if (record->event.pressed) {
                if (led_animation_id == led_setups_count - 1) led_animation_id = 0;
                else led_animation_id++;
            }
            return false;
        case L_PTP:
            if (record->event.pressed) {
                if (led_animation_id == 0) led_animation_id = led_setups_count - 1;
                else led_animation_id--;
            }
            return false;
        case L_PSI:
            if (record->event.pressed) {
                led_animation_speed += ANIMATION_SPEED_STEP;
            }
            return false;
        case L_PSD:
            if (record->event.pressed) {
                led_animation_speed -= ANIMATION_SPEED_STEP;
                if (led_animation_speed < 0) led_animation_speed = 0;
            }
            return false;
        case L_T_MD:
            if (record->event.pressed) {
                led_lighting_mode++;
                if (led_lighting_mode > LED_MODE_MAX_INDEX) led_lighting_mode = LED_MODE_NORMAL;
            }
            return false;
        case L_T_ONF:
            if (record->event.pressed) {
                led_enabled = !led_enabled;
                I2C3733_Control_Set(led_enabled);
            }
            return false;
        case L_ON:
            if (record->event.pressed) {
                led_enabled = 1;
                I2C3733_Control_Set(led_enabled);
            }
            return false;
        case L_OFF:
            if (record->event.pressed) {
                led_enabled = 0;
                I2C3733_Control_Set(led_enabled);
            }
            return false;
        case L_T_BR:
            if (record->event.pressed) {
                led_animation_breathing = !led_animation_breathing;
                if (led_animation_breathing) {
                    gcr_breathe = gcr_desired;
                    led_animation_breathe_cur = BREATHE_MIN_STEP;
                    breathe_dir = 1;
                }
            }
            return false;
        case L_T_PTD:
            if (record->event.pressed) {
                led_animation_direction = !led_animation_direction;
            }
            return false;
        case U_T_AUTO:
            if (record->event.pressed && MODS_SHIFT && MODS_CTRL) {
                TOGGLE_FLAG_AND_PRINT(usb_extra_manual, "USB extra port manual mode");
            }
            return false;
        case U_T_AGCR:
            if (record->event.pressed && MODS_SHIFT && MODS_CTRL) {
                TOGGLE_FLAG_AND_PRINT(usb_gcr_auto, "USB GCR auto mode");
            }
            return false;
        case DBG_TOG:
            if (record->event.pressed) {
                TOGGLE_FLAG_AND_PRINT(debug_enable, "Debug mode");
            }
            return false;
        case DBG_MTRX:
            if (record->event.pressed) {
                TOGGLE_FLAG_AND_PRINT(debug_matrix, "Debug matrix");
            }
            return false;
        case DBG_KBD:
            if (record->event.pressed) {
                TOGGLE_FLAG_AND_PRINT(debug_keyboard, "Debug keyboard");
            }
            return false;
        case DBG_MOU:
            if (record->event.pressed) {
                TOGGLE_FLAG_AND_PRINT(debug_mouse, "Debug mouse");
            }
            return false;
        case MD_BOOT:
            if (record->event.pressed) {
                key_timer = timer_read32();
            } else {
                if (timer_elapsed32(key_timer) >= 500) {
                    reset_keyboard();
                }
            }
            return false;
        default:
            return true; //Process all other keycodes normally
    }
}

uint32_t layer_state_set_user(uint32_t state) {
    switch (biton32(state)) {
        case _BL:
            underglow_rgb = 0x000000;
            break;
        case _FL:
            underglow_rgb = 0x000010;
            break;
        case _KB:
            underglow_rgb = 0x100000;
            break;
        default:
            underglow_rgb = 0x000000;
            break;
    }
    return state;
}

