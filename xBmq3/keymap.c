#include QMK_KEYBOARD_H
#include "version.h"
#define MOON_LED_LEVEL LED_LEVEL
#ifndef ZSA_SAFE_RANGE
#define ZSA_SAFE_RANGE SAFE_RANGE
#endif

#include "oneshot.h"
#include "swapper.h"

enum layers {
    DEF = 0,
    SYM = 1,
    NAV = 2,
    NUM = 3,
};

enum custom_keycodes {
    RGB_SLD = ZSA_SAFE_RANGE,
    OS_SHFT,
    OS_CTRL,
    OS_ALT,
    OS_CMD,
    SW_WIN,
    SW_LANG,
};

#define LA_SYM MO(SYM)
#define LA_NAV MO(NAV)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [DEF] = LAYOUT_voyager(
    KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT,                                 KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT,
    KC_TRANSPARENT, KC_Q,           KC_W,           KC_E,           KC_R,           KC_T,                                           KC_Y,           KC_U,           KC_I,           KC_O,           KC_P,           KC_TRANSPARENT,
    KC_TRANSPARENT, KC_A,           KC_S,           KC_D,           KC_F,           KC_G,                                           KC_H,           KC_J,           KC_K,           KC_L,           KC_SCLN,        KC_TRANSPARENT,
    KC_TRANSPARENT, KC_Z,           KC_X,           KC_C,           KC_V,           KC_B,                                           KC_N,           KC_M,           KC_COMMA,       KC_DOT,         KC_SLASH,       KC_TRANSPARENT,
                                                    LA_NAV,         KC_LEFT_SHIFT,                                  KC_SPACE,       LA_SYM
  ),
  [SYM] = LAYOUT_voyager(
    KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT,                                 KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT,
    KC_TRANSPARENT, KC_ESCAPE,      KC_LBRC,        KC_LCBR,        KC_LPRN,        KC_TILD,                                        KC_CIRC,        KC_RPRN,        KC_RCBR,        KC_RBRC,        KC_GRAVE,       KC_TRANSPARENT,
    KC_TRANSPARENT, KC_MINUS,       KC_ASTR,        KC_EQUAL,       KC_UNDS,        KC_DLR,                                         KC_HASH,        OS_CMD,         OS_ALT,         OS_CTRL,        OS_SHFT,        KC_TRANSPARENT,
    KC_TRANSPARENT, KC_PLUS,        KC_PIPE,        KC_AT,          KC_SLASH,       KC_PERC,                                        KC_TRANSPARENT, KC_BSLS,        KC_AMPR,        KC_QUES,        KC_EXLM,        KC_TRANSPARENT,
                                                    KC_TRANSPARENT, KC_TRANSPARENT,                                 KC_TRANSPARENT, KC_TRANSPARENT
  ),
  [NAV] = LAYOUT_voyager(
    KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT,                                 KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT,
    KC_TRANSPARENT, KC_TAB,         SW_WIN,         LGUI(LSFT(KC_LBRC)),LGUI(LSFT(KC_RBRC)),KC_AUDIO_VOL_UP,                        CW_TOGG,        KC_HOME,        KC_UP,          KC_END,         KC_DELETE,      KC_TRANSPARENT,
    KC_TRANSPARENT, OS_SHFT,        OS_CTRL,        OS_ALT,         OS_CMD,         KC_AUDIO_VOL_DOWN,                              KC_CAPS,        KC_LEFT,        KC_DOWN,        KC_RIGHT,       KC_BSPC,        KC_TRANSPARENT,
    KC_TRANSPARENT, KC_MEDIA_PREV_TRACK,KC_MEDIA_NEXT_TRACK,LGUI(KC_LBRC),  LGUI(KC_RBRC),  KC_MEDIA_PLAY_PAUSE,                    RGB_TOG,        KC_PGDN,        KC_PAGE_UP,     KC_AUDIO_MUTE,  KC_ENTER,       KC_TRANSPARENT,
                                                    KC_TRANSPARENT, KC_TRANSPARENT,                                 KC_TRANSPARENT, KC_TRANSPARENT
  ),
  [NUM] = LAYOUT_voyager(
    KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT,                                 KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT,
    KC_TRANSPARENT, KC_1,           KC_2,           KC_3,           KC_4,           KC_5,                                           KC_6,           KC_7,           KC_8,           KC_9,           KC_0,           KC_TRANSPARENT,
    KC_TRANSPARENT, OS_SHFT,        OS_CTRL,        OS_ALT,         OS_CMD,         KC_F11,                                         KC_F12,         OS_CMD,         OS_ALT,         OS_CTRL,        OS_SHFT,        KC_TRANSPARENT,
    KC_TRANSPARENT, KC_F1,          KC_F2,          KC_F3,          KC_F4,          KC_F5,                                          KC_F6,          KC_F7,          KC_F8,          KC_F9,          KC_F10,         KC_TRANSPARENT,
                                                    KC_TRANSPARENT, KC_TRANSPARENT,                                 KC_TRANSPARENT, KC_TRANSPARENT
  ),
};

bool is_oneshot_cancel_key(uint16_t keycode) {
    switch (keycode) {
    case LA_SYM:
    case LA_NAV:
        return true;
    default:
        return false;
    }
}

bool is_oneshot_ignored_key(uint16_t keycode) {
    switch (keycode) {
    case LA_SYM:
    case LA_NAV:
    case KC_LSFT:
    case OS_SHFT:
    case OS_CTRL:
    case OS_ALT:
    case OS_CMD:
        return true;
    default:
        return false;
    }
}

static bool sw_win_active  = false;
static bool sw_lang_active = false;

static oneshot_state os_shft_state = os_up_unqueued;
static oneshot_state os_ctrl_state = os_up_unqueued;
static oneshot_state os_alt_state  = os_up_unqueued;
static oneshot_state os_cmd_state  = os_up_unqueued;

extern rgb_config_t rgb_matrix_config;

RGB hsv_to_rgb_with_value(HSV hsv) {
  RGB rgb = hsv_to_rgb( hsv );
  float f = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
  return (RGB){ f * rgb.r, f * rgb.g, f * rgb.b };
}

void keyboard_post_init_user(void) {
  rgb_matrix_enable();
}

const uint8_t PROGMEM ledmap[][RGB_MATRIX_LED_COUNT][3] = {
    [0] = { {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {41,255,255}, {0,0,0}, {41,255,255}, {41,255,255}, {41,255,255}, {169,255,84}, {41,255,255}, {41,255,255}, {41,255,255}, {41,255,255}, {41,255,255}, {169,255,84}, {41,255,255}, {41,255,255}, {41,255,255}, {41,255,255}, {41,255,255}, {0,255,255}, {0,0,0}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {41,255,255}, {41,255,255}, {41,255,255}, {41,255,255}, {41,255,255}, {169,255,84}, {41,255,255}, {41,255,255}, {41,255,255}, {41,255,255}, {41,255,255}, {169,255,84}, {41,255,255}, {41,255,255}, {41,255,255}, {41,255,255}, {41,255,255}, {169,255,84}, {0,0,0}, {131,255,255} },

    [1] = { {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {131,255,255}, {131,255,255}, {131,255,255}, {131,255,255}, {131,255,255}, {169,255,84}, {131,255,255}, {131,255,255}, {131,255,255}, {131,255,255}, {131,255,255}, {169,255,84}, {131,255,255}, {131,255,255}, {131,255,255}, {131,255,255}, {131,255,255}, {0,0,0}, {0,0,0}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {131,255,255}, {131,255,255}, {131,255,255}, {131,255,255}, {131,255,255}, {169,255,84}, {131,255,255}, {0,0,0}, {131,255,255}, {131,255,255}, {131,255,255}, {169,255,84}, {131,255,255}, {131,255,255}, {131,255,255}, {131,255,255}, {131,255,255}, {169,255,84}, {0,0,0}, {0,0,0} },

    [2] = { {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {0,255,255}, {0,255,255}, {0,255,255}, {0,255,255}, {0,255,255}, {169,255,84}, {0,255,255}, {0,255,255}, {0,255,255}, {0,255,255}, {0,255,255}, {169,255,84}, {0,255,255}, {0,255,255}, {0,255,255}, {0,255,255}, {0,255,255}, {0,0,0}, {0,0,0}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {0,255,255}, {0,255,255}, {0,255,255}, {0,255,255}, {0,255,255}, {169,255,84}, {0,255,255}, {0,255,255}, {0,255,255}, {0,255,255}, {0,255,255}, {169,255,84}, {0,255,255}, {0,255,255}, {0,255,255}, {0,255,255}, {0,255,255}, {169,255,84}, {0,0,0}, {0,0,0} },

    [3] = { {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {152,255,255}, {152,255,255}, {152,255,255}, {152,255,255}, {152,255,255}, {169,255,84}, {152,255,255}, {152,255,255}, {152,255,255}, {152,255,255}, {152,255,255}, {169,255,84}, {152,255,255}, {152,255,255}, {152,255,255}, {152,255,255}, {152,255,255}, {152,255,255}, {152,255,255}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {152,255,255}, {152,255,255}, {152,255,255}, {152,255,255}, {152,255,255}, {169,255,84}, {152,255,255}, {152,255,255}, {152,255,255}, {152,255,255}, {152,255,255}, {169,255,84}, {152,255,255}, {152,255,255}, {152,255,255}, {152,255,255}, {152,255,255}, {169,255,84}, {152,255,255}, {152,255,255} },

};

void set_layer_color(int layer) {
  for (int i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
    HSV hsv = {
      .h = pgm_read_byte(&ledmap[layer][i][0]),
      .s = pgm_read_byte(&ledmap[layer][i][1]),
      .v = pgm_read_byte(&ledmap[layer][i][2]),
    };
    if (!hsv.h && !hsv.s && !hsv.v) {
        rgb_matrix_set_color( i, 0, 0, 0 );
    } else {
        RGB rgb = hsv_to_rgb_with_value(hsv);
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
  }
}

bool rgb_matrix_indicators_user(void) {
  if (rawhid_state.rgb_control) {
      return false;
  }
  if (!keyboard_config.disable_layer_led) {
    switch (biton32(layer_state)) {
      case 0:
        set_layer_color(0);
        break;
      case 1:
        set_layer_color(1);
        break;
      case 2:
        set_layer_color(2);
        break;
      case 3:
        set_layer_color(3);
        break;
     default:
        if (rgb_matrix_get_flags() == LED_FLAG_NONE) {
          rgb_matrix_set_color_all(0, 0, 0);
        }
    }
  } else {
    if (rgb_matrix_get_flags() == LED_FLAG_NONE) {
      rgb_matrix_set_color_all(0, 0, 0);
    }
  }

  return true;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    update_swapper(
        &sw_win_active, KC_LGUI, KC_TAB, SW_WIN,
        keycode, record
    );
    update_swapper(
        &sw_lang_active, KC_LCTL, KC_SPC, SW_LANG,
        keycode, record
    );

    update_oneshot(
        &os_shft_state, KC_LSFT, OS_SHFT,
        keycode, record
    );
    update_oneshot(
        &os_ctrl_state, KC_LCTL, OS_CTRL,
        keycode, record
    );
    update_oneshot(
        &os_alt_state, KC_LALT, OS_ALT,
        keycode, record
    );
    update_oneshot(
        &os_cmd_state, KC_LCMD, OS_CMD,
        keycode, record
    );

    switch (keycode) {
    case QK_MODS ... QK_MODS_MAX:
        if (IS_CONSUMER_KEYCODE(QK_MODS_GET_BASIC_KEYCODE(keycode))) {
            if (record->event.pressed) {
                add_mods(QK_MODS_GET_MODS(keycode));
                send_keyboard_report();
                wait_ms(2);
                register_code(QK_MODS_GET_BASIC_KEYCODE(keycode));
                return false;
            } else {
                wait_ms(2);
                del_mods(QK_MODS_GET_MODS(keycode));
            }
        }
        break;

    case RGB_SLD:
        if (record->event.pressed) {
            rgblight_mode(1);
        }
        return false;
    }
    return true;
}

layer_state_t layer_state_set_user(layer_state_t state) {
    return update_tri_layer_state(state, SYM, NAV, NUM);
}
