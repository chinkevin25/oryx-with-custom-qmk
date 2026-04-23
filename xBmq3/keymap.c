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

#define LA_SYM LT(SYM, KC_ENT)
#define LA_NAV LT(NAV, KC_SPC)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [DEF] = LAYOUT_voyager(
    KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT,                                 KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT,
    KC_TRANSPARENT, KC_Q,           KC_W,           KC_E,           KC_R,           KC_T,                                           KC_Y,           KC_U,           KC_I,           KC_O,           KC_P,           KC_TRANSPARENT,
    KC_TRANSPARENT, LGUI_T(KC_A),   LALT_T(KC_S),   LCTL_T(KC_D),   LSFT_T(KC_F),   KC_G,                                           KC_H,           RSFT_T(KC_J),   RCTL_T(KC_K),   RALT_T(KC_L),   RGUI_T(KC_SCLN),KC_TRANSPARENT,
    KC_TRANSPARENT, KC_Z,           KC_X,           KC_C,           KC_V,           KC_B,                                           KC_N,           KC_M,           KC_COMMA,       KC_DOT,         KC_SLASH,       KC_TRANSPARENT,
                                                    KC_ESCAPE,      LA_NAV,                                         LA_SYM,         KC_BSPC
  ),
  [SYM] = LAYOUT_voyager(
    KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT,                                 KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT,
    KC_TRANSPARENT, KC_ESCAPE,      KC_LBRC,        KC_LCBR,        KC_LPRN,        KC_TILD,                                        KC_CIRC,        KC_RPRN,        KC_RCBR,        KC_RBRC,        KC_GRAVE,       KC_TRANSPARENT,
    KC_TRANSPARENT, KC_MINUS,       KC_ASTR,        KC_EQUAL,       KC_UNDS,        KC_DLR,                                         KC_HASH,        OS_SHFT,        OS_CTRL,        OS_ALT,         OS_CMD,         KC_TRANSPARENT,
    KC_TRANSPARENT, KC_PLUS,        KC_PIPE,        KC_AT,          KC_SLASH,       KC_PERC,                                        KC_TRANSPARENT, KC_BSLS,        KC_AMPR,        KC_QUES,        KC_EXLM,        KC_TRANSPARENT,
                                                    KC_TRANSPARENT, KC_TRANSPARENT,                                 KC_TRANSPARENT, KC_TRANSPARENT
  ),
  [NAV] = LAYOUT_voyager(
    KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT,                                 KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT,
    KC_TRANSPARENT, KC_TAB,         SW_WIN,         LGUI(LSFT(KC_LBRC)),LGUI(LSFT(KC_RBRC)),KC_AUDIO_VOL_UP,                        CW_TOGG,        KC_HOME,        QK_REPEAT_KEY,  KC_END,         KC_DELETE,      KC_TRANSPARENT,
    KC_TRANSPARENT, OS_CMD,         OS_ALT,         OS_CTRL,        OS_SHFT,        KC_AUDIO_VOL_DOWN,                              KC_LEFT,        KC_DOWN,        KC_UP,          KC_RIGHT,       KC_BSPC,        KC_TRANSPARENT,
    KC_TRANSPARENT, KC_MEDIA_PREV_TRACK,KC_MEDIA_NEXT_TRACK,LGUI(KC_LBRC),  LGUI(KC_RBRC),  KC_MEDIA_PLAY_PAUSE,                    RGB_TOG,        KC_PGDN,        KC_PAGE_UP,     KC_AUDIO_MUTE,  KC_ENTER,       KC_TRANSPARENT,
                                                    KC_TRANSPARENT, KC_TRANSPARENT,                                 KC_TRANSPARENT, KC_TRANSPARENT
  ),
  [NUM] = LAYOUT_voyager(
    KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT,                                 KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT,
    KC_TRANSPARENT, KC_1,           KC_2,           KC_3,           KC_4,           KC_5,                                           KC_6,           KC_7,           KC_8,           KC_9,           KC_0,           KC_TRANSPARENT,
    KC_TRANSPARENT, OS_CMD,         OS_ALT,         OS_CTRL,        OS_SHFT,        KC_F11,                                         KC_F12,         OS_SHFT,        OS_CTRL,        OS_ALT,         OS_CMD,         KC_TRANSPARENT,
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
    case KC_LCTL:
    case KC_LALT:
    case KC_LGUI:
    case KC_RSFT:
    case KC_RCTL:
    case KC_RALT:
    case KC_RGUI:
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
    [0] = { {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {41,255,255}, {41,255,255}, {41,255,255}, {41,255,255}, {41,255,255}, {169,255,84}, {41,255,255}, {41,255,255}, {41,255,255}, {41,255,255}, {41,255,255}, {169,255,84}, {41,255,255}, {41,255,255}, {41,255,255}, {41,255,255}, {41,255,255}, {0,255,255}, {0,245,245}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {41,255,255}, {41,255,255}, {41,255,255}, {41,255,255}, {41,255,255}, {169,255,84}, {41,255,255}, {41,255,255}, {41,255,255}, {41,255,255}, {41,255,255}, {169,255,84}, {41,255,255}, {41,255,255}, {41,255,255}, {41,255,255}, {41,255,255}, {169,255,84}, {0,0,0}, {131,255,255} },

    [1] = { {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {131,255,255}, {131,255,255}, {131,255,255}, {131,255,255}, {131,255,255}, {169,255,84}, {131,255,255}, {131,255,255}, {131,255,255}, {131,255,255}, {131,255,255}, {169,255,84}, {131,255,255}, {131,255,255}, {131,255,255}, {131,255,255}, {131,255,255}, {0,0,0}, {0,0,0}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {131,255,255}, {131,255,255}, {131,255,255}, {131,255,255}, {131,255,255}, {169,255,84}, {131,255,255}, {0,245,245}, {41,255,255}, {74,255,255}, {152,255,255}, {169,255,84}, {131,255,255}, {131,255,255}, {131,255,255}, {131,255,255}, {131,255,255}, {169,255,84}, {0,0,0}, {0,0,0} },

    [2] = { {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {0,255,255}, {0,255,255}, {0,255,255}, {0,255,255}, {0,255,255}, {169,255,84}, {152,255,255}, {74,255,255}, {41,255,255}, {0,245,245}, {0,255,255}, {169,255,84}, {0,255,255}, {0,255,255}, {0,255,255}, {0,255,255}, {0,255,255}, {0,0,0}, {0,0,0}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {0,255,255}, {0,255,255}, {0,255,255}, {0,255,255}, {0,255,255}, {169,255,84}, {0,255,255}, {0,255,255}, {0,255,255}, {0,255,255}, {0,255,255}, {169,255,84}, {0,255,255}, {0,255,255}, {0,255,255}, {0,255,255}, {0,255,255}, {169,255,84}, {0,0,0}, {0,0,0} },

    [3] = { {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {152,255,255}, {152,255,255}, {152,255,255}, {152,255,255}, {152,255,255}, {169,255,84}, {131,255,255}, {74,255,255}, {41,255,255}, {0,245,245}, {152,255,255}, {169,255,84}, {152,255,255}, {152,255,255}, {152,255,255}, {152,255,255}, {152,255,255}, {152,255,255}, {152,255,255}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {169,255,84}, {152,255,255}, {152,255,255}, {152,255,255}, {152,255,255}, {152,255,255}, {169,255,84}, {152,255,255}, {0,245,245}, {41,255,255}, {41,255,255}, {152,255,255}, {169,255,84}, {152,255,255}, {152,255,255}, {152,255,255}, {152,255,255}, {152,255,255}, {169,255,84}, {152,255,255}, {152,255,255} },

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

// LED indices for the Voyager (row-major per half: 0-25 left, 26-51 right)
// Left half   row 2 (home): 12(col0) 13(A) 14(S) 15(D) 16(F) 17(G)
// Right half  row 2 (home): 38(H) 39(J) 40(K) 41(L) 42(;) 43(col5)
// Thumbs: 24 LeftOuter, 25 LeftInner, 50 RightInner, 51 RightOuter

bool rgb_matrix_indicators_user(void) {
    if (rawhid_state.rgb_control) {
        return false;
    }
    if (keyboard_config.disable_layer_led) {
        if (rgb_matrix_get_flags() == LED_FLAG_NONE) {
            rgb_matrix_set_color_all(0, 0, 0);
        }
        return true;
    }

    // Tier 1: start clean (blocked keys stay off)
    rgb_matrix_set_color_all(0, 0, 0);

    uint8_t layer = biton32(layer_state);

    switch (layer) {
        case DEF:
            // Row 1 alphas: dim
            for (int i = 7; i <= 11; i++) rgb_matrix_set_color(i, 30, 30, 30);
            for (int i = 32; i <= 36; i++) rgb_matrix_set_color(i, 30, 30, 30);
            // Row 2 HRM: mod color per finger, mirrored
            rgb_matrix_set_color(13, 150, 0, 255);     // A = Cmd
            rgb_matrix_set_color(14, 255, 100, 0);     // S = Alt
            rgb_matrix_set_color(15, 0, 200, 255);     // D = Ctrl
            rgb_matrix_set_color(16, 255, 50, 50);     // F = Shift
            rgb_matrix_set_color(17, 30, 30, 30);      // G
            rgb_matrix_set_color(38, 30, 30, 30);      // H
            rgb_matrix_set_color(39, 255, 50, 50);     // J = Shift
            rgb_matrix_set_color(40, 0, 200, 255);     // K = Ctrl
            rgb_matrix_set_color(41, 255, 100, 0);     // L = Alt
            rgb_matrix_set_color(42, 150, 0, 255);     // ; = Cmd
            // Row 3 alphas: dim
            for (int i = 19; i <= 23; i++) rgb_matrix_set_color(i, 30, 30, 30);
            for (int i = 44; i <= 48; i++) rgb_matrix_set_color(i, 30, 30, 30);
            break;

        case SYM:
            // Row 1: Esc + brackets/symbols
            rgb_matrix_set_color(7, 180, 80, 255);    // Esc
            for (int i = 8; i <= 10; i++) rgb_matrix_set_color(i, 180, 80, 255);   // [ { (
            rgb_matrix_set_color(11, 200, 0, 150);    // ~
            rgb_matrix_set_color(32, 200, 0, 150);    // ^
            for (int i = 33; i <= 35; i++) rgb_matrix_set_color(i, 180, 80, 255);  // ) } ]
            rgb_matrix_set_color(36, 200, 0, 150);    // `
            // Row 2: symbols left, mods right (in base HRM order)
            for (int i = 13; i <= 17; i++) rgb_matrix_set_color(i, 200, 0, 150);
            rgb_matrix_set_color(38, 200, 0, 150);    // #
            rgb_matrix_set_color(39, 255, 50, 50);    // OS_SHFT
            rgb_matrix_set_color(40, 0, 200, 255);    // OS_CTRL
            rgb_matrix_set_color(41, 255, 100, 0);    // OS_ALT
            rgb_matrix_set_color(42, 150, 0, 255);    // OS_CMD
            // Row 3 symbols
            for (int i = 19; i <= 23; i++) rgb_matrix_set_color(i, 200, 0, 150);
            for (int i = 45; i <= 48; i++) rgb_matrix_set_color(i, 200, 0, 150);
            break;

        case NAV:
            // Row 1: nav-history + media
            for (int i = 7; i <= 10; i++) rgb_matrix_set_color(i, 100, 200, 100);  // Tab, SWin, Tab<, Tab>
            rgb_matrix_set_color(11, 255, 100, 0);     // Vol+
            rgb_matrix_set_color(32, 60, 60, 60);      // CW_TOGG
            rgb_matrix_set_color(33, 100, 200, 100);   // Home
            rgb_matrix_set_color(34, 255, 200, 0);     // Repeat — amber
            rgb_matrix_set_color(35, 100, 200, 100);   // End
            rgb_matrix_set_color(36, 255, 50, 50);     // Del — red
            // Row 2: mods left, arrows right, Bspc
            rgb_matrix_set_color(13, 150, 0, 255);     // OS_CMD
            rgb_matrix_set_color(14, 255, 100, 0);     // OS_ALT
            rgb_matrix_set_color(15, 0, 200, 255);     // OS_CTRL
            rgb_matrix_set_color(16, 255, 50, 50);     // OS_SHFT
            rgb_matrix_set_color(17, 255, 100, 0);     // Vol-
            for (int i = 38; i <= 41; i++) rgb_matrix_set_color(i, 0, 255, 100);   // arrows H J K L
            rgb_matrix_set_color(42, 255, 50, 50);     // Bspc — red
            // Row 3: media + nav-history
            rgb_matrix_set_color(19, 255, 100, 0);     // Prev
            rgb_matrix_set_color(20, 255, 100, 0);     // Next
            rgb_matrix_set_color(21, 100, 200, 100);   // Back
            rgb_matrix_set_color(22, 100, 200, 100);   // Fwd
            rgb_matrix_set_color(23, 255, 100, 0);     // Play
            rgb_matrix_set_color(44, 255, 255, 0);     // RGB_TOG — yellow
            rgb_matrix_set_color(45, 100, 200, 100);   // PgDn
            rgb_matrix_set_color(46, 100, 200, 100);   // PgUp
            rgb_matrix_set_color(47, 255, 100, 0);     // Mute
            rgb_matrix_set_color(48, 255, 50, 50);     // Enter — red
            break;

        case NUM:
            // Row 1 numbers
            for (int i = 7; i <= 11; i++) rgb_matrix_set_color(i, 0, 100, 255);
            for (int i = 32; i <= 36; i++) rgb_matrix_set_color(i, 0, 100, 255);
            // Row 2: mods both sides (base HRM order), F11/F12 inner
            rgb_matrix_set_color(13, 150, 0, 255);     // OS_CMD
            rgb_matrix_set_color(14, 255, 100, 0);     // OS_ALT
            rgb_matrix_set_color(15, 0, 200, 255);     // OS_CTRL
            rgb_matrix_set_color(16, 255, 50, 50);     // OS_SHFT
            rgb_matrix_set_color(17, 100, 50, 200);    // F11
            rgb_matrix_set_color(38, 100, 50, 200);    // F12
            rgb_matrix_set_color(39, 255, 50, 50);     // OS_SHFT
            rgb_matrix_set_color(40, 0, 200, 255);     // OS_CTRL
            rgb_matrix_set_color(41, 255, 100, 0);     // OS_ALT
            rgb_matrix_set_color(42, 150, 0, 255);     // OS_CMD
            // Row 3 F-keys
            for (int i = 19; i <= 23; i++) rgb_matrix_set_color(i, 100, 50, 200);
            for (int i = 44; i <= 48; i++) rgb_matrix_set_color(i, 100, 50, 200);
            break;
    }

    // Tier 1: thumb cluster role colors (consistent across layers)
    rgb_matrix_set_color(24, 255, 60, 0);      // ESC — red-orange
    rgb_matrix_set_color(25, 255, 150, 0);     // NAV — amber
    rgb_matrix_set_color(50, 50, 200, 50);     // SYM — green
    rgb_matrix_set_color(51, 150, 150, 150);   // SPC — gray

    // Tier 2: oneshot queued → bright white flash on mod positions
    if (os_shft_state == os_up_queued) {
        rgb_matrix_set_color(16, 255, 255, 255);
        rgb_matrix_set_color(39, 255, 255, 255);
    }
    if (os_ctrl_state == os_up_queued) {
        rgb_matrix_set_color(15, 255, 255, 255);
        rgb_matrix_set_color(40, 255, 255, 255);
    }
    if (os_alt_state == os_up_queued) {
        rgb_matrix_set_color(14, 255, 255, 255);
        rgb_matrix_set_color(41, 255, 255, 255);
    }
    if (os_cmd_state == os_up_queued) {
        rgb_matrix_set_color(13, 255, 255, 255);
        rgb_matrix_set_color(42, 255, 255, 255);
    }

    // Tier 2: HRM mod currently held → brighter in mod color
    uint8_t mods = get_mods();
    if (mods & MOD_MASK_SHIFT) {
        rgb_matrix_set_color(16, 255, 150, 150);
        rgb_matrix_set_color(39, 255, 150, 150);
    }
    if (mods & MOD_MASK_CTRL) {
        rgb_matrix_set_color(15, 150, 230, 255);
        rgb_matrix_set_color(40, 150, 230, 255);
    }
    if (mods & MOD_MASK_ALT) {
        rgb_matrix_set_color(14, 255, 200, 100);
        rgb_matrix_set_color(41, 255, 200, 100);
    }
    if (mods & MOD_MASK_GUI) {
        rgb_matrix_set_color(13, 200, 100, 255);
        rgb_matrix_set_color(42, 200, 100, 255);
    }

    // Tier 2: Caps Word active → amber top row alphas
    if (is_caps_word_on()) {
        for (int i = 7; i <= 11; i++) rgb_matrix_set_color(i, 255, 180, 0);
        for (int i = 32; i <= 36; i++) rgb_matrix_set_color(i, 255, 180, 0);
    }

    // Tier 3: tri-layer NUM active → distinctive cyan on both inner thumbs
    if (layer == NUM) {
        rgb_matrix_set_color(25, 0, 255, 200);
        rgb_matrix_set_color(50, 0, 255, 200);
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
