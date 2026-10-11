#define COLOR(hex) { ((hex >> 24) & 0xFF) / 255.0f, \
                     ((hex >> 16) & 0xFF) / 255.0f, \
                     ((hex >> 8) & 0xFF) / 255.0f, \
                     (hex & 0xFF) / 255.0f }

static const int sloppyfocus = 1;
static const int bypass_surface_visibility = 0;
static const unsigned int borderpx = 1;
static const unsigned int snap = 32;
static const float rootcolor[] = COLOR(0x0c0e15ff);
static const float bordercolor[] = COLOR(0x283347ff);
static const float focuscolor[] = COLOR(0x41a7fcff);
static const float urgentcolor[] = COLOR(0xf65866ff);
static const float fullscreen_bg[] = { 0.0f, 0.0f, 0.0f, 1.0f };

#define TAGCOUNT (9)
static int log_level = WLR_ERROR;

static const Layout layouts[] = {
    { "[]", frame_layout },
};

static const MonitorRule monrules[] = {
    { NULL, 1.0f, .rr =  WL_OUTPUT_TRANSFORM_NORMAL, -1, -1 },
};

static const struct xkb_rule_names xkb_rules = { .options = NULL };
static const int repeat_rate = 35;
static const int repeat_delay = 250;

#define MODKEY WLR_MODIFIER_LOGO
#define TAGKEYS(KEY, TAG) \
    { MODKEY, KEY, view, {.ui = 1 << TAG} }, \
    { MODKEY|WLR_MODIFIER_CTRL, KEY, toggleview, {.ui = 1 << TAG} }, \
    { MODKEY|WLR_MODIFIER_SHIFT, KEY, tag, {.ui = 1 << TAG} }, \
    { MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT, KEY, toggletag, {.ui = 1 << TAG} }

static const char *termcmd[] = { "foot", NULL };

static const Key keys[] = {
    { MODKEY, XKB_KEY_Return, spawn, {.v = termcmd} },
    { MODKEY, XKB_KEY_a, zoom, {0} },
    { MODKEY, XKB_KEY_Tab, view, {0} },
    { MODKEY, XKB_KEY_c, killclient, {0} },
    { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_space, togglefloating, {0} },
    { MODKEY, XKB_KEY_e, togglefullscreen, {0} },
    { MODKEY, XKB_KEY_0, view, {.ui = ~0} },
    { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_0, tag, {.ui = ~0} },
    { MODKEY, XKB_KEY_comma, focusmon, {.i = WLR_DIRECTION_LEFT} },
    { MODKEY, XKB_KEY_period, focusmon, {.i = WLR_DIRECTION_RIGHT} },
    { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_comma, tagmon, {.i = WLR_DIRECTION_LEFT} },
    { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_period, tagmon, {.i = WLR_DIRECTION_RIGHT} },

    { MODKEY, XKB_KEY_h, frame_focus, {.i = FRAME_LEFT} },
    { MODKEY, XKB_KEY_j, frame_focus, {.i = FRAME_DOWN} },
    { MODKEY, XKB_KEY_k, frame_focus, {.i = FRAME_UP} },
    { MODKEY, XKB_KEY_l, frame_focus, {.i = FRAME_RIGHT} },

    { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_H, frame_swap, {.i = FRAME_LEFT} },
    { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_J, frame_swap, {.i = FRAME_DOWN} },
    { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_K, frame_swap, {.i = FRAME_UP} },
    { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_L, frame_swap, {.i = FRAME_RIGHT} },

    { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_plus, frame_resize, {.i = FRAME_DOWN} },
    { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_underscore, frame_resize, {.i = FRAME_UP} },
    { MODKEY, XKB_KEY_equal, frame_resize, {.i = FRAME_LEFT} },
    { MODKEY, XKB_KEY_minus, frame_resize, {.i = FRAME_RIGHT} },

    { MODKEY, XKB_KEY_i, frame_set_split, {.i = SPLIT_LEFT_RIGHT} },
    { MODKEY, XKB_KEY_n, frame_set_split, {.i = SPLIT_TOP_BOTTOM} },

    TAGKEYS(XKB_KEY_1, 0),
    TAGKEYS(XKB_KEY_2, 1),
    TAGKEYS(XKB_KEY_3, 2),
    TAGKEYS(XKB_KEY_4, 3),
    TAGKEYS(XKB_KEY_5, 4),
    TAGKEYS(XKB_KEY_6, 5),
    TAGKEYS(XKB_KEY_7, 6),
    TAGKEYS(XKB_KEY_8, 7),
    TAGKEYS(XKB_KEY_9, 8),

    { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_q, quit, {0} },
    { WLR_MODIFIER_CTRL|WLR_MODIFIER_ALT, XKB_KEY_BackSpace, quit, {0} },
#define CHVT(n) { WLR_MODIFIER_CTRL|WLR_MODIFIER_ALT, XKB_KEY_F##n, chvt, {.ui = (n)} }
    CHVT(1), CHVT(2), CHVT(3), CHVT(4), CHVT(5), CHVT(6),
    CHVT(7), CHVT(8), CHVT(9), CHVT(10), CHVT(11), CHVT(12),
};

static const Button buttons[] = {
    { MODKEY, BTN_LEFT, moveresize, {.ui = CurMove} },
    { MODKEY, BTN_MIDDLE, togglefloating, {0} },
    { MODKEY, BTN_RIGHT, moveresize, {.ui = CurResize} },
};

