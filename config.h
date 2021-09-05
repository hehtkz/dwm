/* See LICENSE file for copyright and license details. */

#include "gaplessgrid.c"
#include "fibonacci.c"
#include "movestack.c"
#include "mpdcontrol.c"
#include <X11/XF86keysym.h>

/* appearance */
static const char *fonts[] = {
    "Inconsolata:size=11"
};
static const char dmenufont[]       = "Inconsolata:size=11";
static const char normbordercolor[] = "#be0000";
static const char normbgcolor[]     = "#101010";
static const char normfgcolor[]     = "#bbbbbb";
static const char selbordercolor[]  = "#383636";
static const char selbgcolor[]      = "#383636";
static const char selfgcolor[]      = "#eeeeee";
static const unsigned int borderpx  = 0;        /* border pixel of windows */
static const unsigned int gappx     = 8;        /* gaps */
static const unsigned int snap      = 32;       /* snap pixel */
static const int showbar            = 2;        /* 0 means no bar */
static const int topbar             = 2;        /* 0 means bottom bar */

#define NUMCOLORS         4
static const char colors[NUMCOLORS][MAXCOLORS][8] = {
  // border   foreground background
  { "#131313", "#6e6e6e", "#101010" },  // normal
  { "#383636", "#ffffff", "#101010" },  // selected
  { "#212121", "#000000", "#ff3d00" },  // urgent/warning  (black on yellow)
  { "#212121", "#ffffff", "#ff0000" },  // error (white on red)
  // add more here
};


/* tagging */
static const char *tags[] = { "dev", "web", "media", "games", "misc" };

static const Rule rules[] = {
   /* xprop(1):
   *    WM_CLASS(STRING) = instance, class
   *    WM_NAME(STRING) = title
   */
      /* class        instance    title       tags mask     isfloating   monitor */
      { "Gimp",         NULL,       NULL,       0,            1,           -1 },
      { "firefox",      NULL,       NULL,       1 << 1,       0,           -1 },
      { "Chromium",     NULL,       NULL,       1 << 1,       0,           -1 },
      { "Steam",        NULL,       NULL,       1 << 3,       1,           -1 },
      { "Nautilus",     NULL,       NULL,       0,            0,           -1 },
      { "keepassxc",    NULL,       NULL,       1 << 4,       0,           -1 },
      { "Spotify",      NULL,       NULL,       1 << 2,       0,           -1 }
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const Layout layouts[] = {
  /* symbol     arrange function */
  { "[]=",      tile },    /* first entry is default */
  { "><>",      NULL },    /* no layout function means floating behavior */
  { "[M]",      monocle },
  { "[@]",      spiral },
  { "[\\]",     dwindle },
  { "###",      gaplessgrid },
  { "c",        centeredmaster },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
    { MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
    { MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
    { MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
    { MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
//#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbgcolor, "-sf", selfgcolor, NULL };
static const char *termcmd[]  = { "alacritty", NULL };
static const char *pscreen[] = { "screenshot", NULL };
static const char *piscreen[] = { "screenshot", "-c", NULL };
static const char *upvol[]   = { "pactl", "set-sink-volume", "@DEFAULT_SINK@", "+3%",     NULL };
static const char *downvol[] = { "pactl", "set-sink-volume", "@DEFAULT_SINK@", "-3%",     NULL };
static const char *mutevol[] = { "pactl", "set-sink-mute", "@DEFAULT_SINK@", "toggle", NULL };
// static const char *playrandomsong[] = {"randomsong", NULL};
// static const char *playrandomsongs[] = {"randomsong", "-s", NULL};
static const char *rofi[] = { "rofi", "-show", "run", "-fullscreen", NULL };
static const char *openmail[] = { "thunderbird", NULL };
static const char *openbrowser[] = { "firefox", NULL };
static const char *openwebsearch[] = { "firefox", "https://google.com", NULL };
static const char *redshiftonoff[] = { "killall", "-USR1", "redshift", NULL };
static const char *toggleplaypausesong[] = { "spot", "pp" };
static const char *stopsong[] = { "spot", "pause" };
static const char *nextsong[] = { "spot", "next" };
static const char *previoussong[] = { "spot", "prev" };
static const char *systray[] = { "stalonetray" };

static Key keys[] = {
    /* modifier                     key        function        argument */
    { MODKEY,                       XK_p,      spawn,          {.v = dmenucmd } },
    { MODKEY|ShiftMask,             XK_Return, spawn,          {.v = termcmd } },
    { MODKEY,                       XK_F12,    spawn,          {.v = upvol   } },
    { MODKEY,                       XK_F11,    spawn,          {.v = downvol } },
    { MODKEY,                       XK_F9,     spawn,          {.v = mutevol } },
    { MODKEY,                       XK_b,      togglebar,      {0} },
    { MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
    { MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
    { MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
    { MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
    { MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
    { MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
    { MODKEY|ShiftMask,             XK_j,      movestack,      {.i = +1 } },
    { MODKEY|ShiftMask,             XK_k,      movestack,      {.i = -1 } },
    { MODKEY,                       XK_Return, zoom,           {0} },
    { MODKEY,                       XK_Tab,    view,           {0} },
    { MODKEY|ShiftMask,             XK_c,      killclient,     {0} },
    { MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
    { MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
    { MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
    { MODKEY,                       XK_s,      setlayout,      {.v = &layouts[3]} },
    { MODKEY,                       XK_v,      setlayout,      {.v = &layouts[4]} },
    { MODKEY,                       XK_g,      setlayout,      {.v = &layouts[5]} },
//  { MODKEY,                       XK_c,      setlayout,      {.v = &layouts[6]} },
    { MODKEY,                       XK_space,  setlayout,      {0} },
    { MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
    { MODKEY,                       XK_0,      view,           {.ui = ~0 } },
    { MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
    { MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
    { MODKEY,                       XK_period, focusmon,       {.i = +1 } },
    { MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
    { MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
    { 0,                            XK_Print,  spawn,          {.v = pscreen } },
    { MODKEY,                       XK_Print,  spawn,          {.v = piscreen } },
    { MODKEY,                       XK_F2,     spawn,          {.v = rofi } },
    { MODKEY|ShiftMask,             XK_F2,     spawn,          {.v = systray } },
    { MODKEY,                       XK_r,      spawn,          {.v = redshiftonoff } },
    TAGKEYS(                        XK_1,                      0)
    TAGKEYS(                        XK_2,                      1)
    TAGKEYS(                        XK_3,                      2)
    TAGKEYS(                        XK_4,                      3)
    TAGKEYS(                        XK_5,                      4)
    TAGKEYS(                        XK_6,                      5)
    TAGKEYS(                        XK_7,                      6)
    TAGKEYS(                        XK_8,                      7)
    TAGKEYS(                        XK_9,                      8)
    { MODKEY|ShiftMask,             XK_q,      quit,           {0} },
    // { MODKEY,                       XK_F1,     mpdchange,      {.i = -1} },
    // { MODKEY,                       XK_F2,     mpdchange,      {.i = +1} },
    // { MODKEY,                       XK_Escape, mpdcontrol,     {0} },
    // { MODKEY,                       XK_F3,     spawn,          {.v = playrandomsong } },
    // { MODKEY|ShiftMask,             XK_F3,     spawn,          {.v = playrandomsongs } },
    { 0,                            XF86XK_Mail,                spawn,          {.v = openmail } },
    { 0,                            XF86XK_HomePage,            spawn,          {.v = openbrowser } },
    { 0,                            XF86XK_Search,              spawn,          {.v = openwebsearch } },
    { 0,                            XF86XK_AudioNext,           spawn,          {.v = nextsong } },
    { 0,                            XF86XK_AudioPrev,           spawn,          {.v = previoussong } },
    { 0,                            XF86XK_AudioPlay,           spawn,          {.v = toggleplaypausesong } },
    { 0,                            XF86XK_AudioStop,           spawn,          {.v = stopsong } },
    { 0,                            XF86XK_AudioRaiseVolume,    spawn,          {.v = upvol   } },
    { 0,                            XF86XK_AudioLowerVolume,    spawn,          {.v = downvol } },
    { 0,                            XF86XK_AudioMute,           spawn,          {.v = mutevol } }
};

/* button definitions */
/* click can be ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
    /* click                event mask      button          function        argument */
    { ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
    { ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
    { ClkWinTitle,          0,              Button2,        zoom,           {0} },
    { ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
    { ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
    { ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
    { ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
    { ClkTagBar,            0,              Button1,        view,           {0} },
    { ClkTagBar,            0,              Button3,        toggleview,     {0} },
    { ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
    { ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};
