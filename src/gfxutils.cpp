#include "gfxutils.h"

#include <tice.h>
#include <string.h>
#include <limits.h>

#define isbuf (gfx_GetDraw() == gfx_buffer)
#define doblit() \
    if (isbuf)   \
        gfx_BlitBuffer();

static uint8_t foreground = 0, background = 255;
static uint8_t cbak;

static const gfx_sprite_t *bgsprite;

static sk_key_t getk()
{
    sk_key_t k;
    while (!(k = os_GetCSC()))
        ;
    return k;
}

static void setcol(uint8_t color)
{
    cbak = gfx_SetColor(color);
}

static void restcol()
{
    gfx_SetColor(cbak);
}

static const char *chars[] = {
    // normal
    "+-*/^\0\0-369)\0\0\0.258(\0\0\0000147,\0\0X\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",
    // 2nd
    "\0][e\0\0\0\0\0\0w}\0\0\0i\0\0v{\0\0\0\0\0\0u\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",
    // alpha
    "\"WRMH\0\0?\0VQLG\0\0:ZUPKFC\0 YTOJEB\0\0XSNIDA\0\0\0\0\0\0\0\0"};

const char *capital_chars = chars[2];
const char *yes_no[2] = {"Yes", "No"};
const char *no_yes[2] = {yes_no[1], yes_no[0]};

static char lower(char c)
{
    if (c >= 'A' && c <= 'Z')
        return c - 'A' + 'a';
    return c;
}

static unsigned int max(unsigned int a, unsigned int b) { return a > b ? a : b; }

void gfxutils_TextBox(const char *title, uint8_t lines, const char **texts)
{
    const unsigned int height = 10 + 16 + 9 * lines;
    unsigned int width, x, y;

    width = gfx_GetStringWidth(title) * 2;
    for (uint8_t i = 0; i < lines; i++)
        width = max(width, gfx_GetStringWidth(texts[i]));
    width += 10;
    setcol(background);
    x = (LCD_WIDTH - width) / 2, y = (LCD_HEIGHT - height) / 2;
    gfx_FillRectangle(x, y, width, height);
    gfx_SetTextScale(2, 2);
    gfx_PrintStringXY(title, (LCD_WIDTH - gfx_GetStringWidth(title)) / 2, y + 3);
    gfx_SetTextScale(1, 1);
    for (uint8_t i = 0; i < lines; i++)
        gfx_PrintStringXY(texts[i], (LCD_WIDTH - gfx_GetStringWidth(texts[i])) / 2, y + 3 + 16 + 4 + 9 * i);
}

uint16_t gfxutils_GetTextInput(const char *title, char *buffer, uint16_t length, uint8_t spmode)
{
    int16_t width, x;
    uint16_t titlew, ind;
    const uint8_t height = 16 + 8 + 10, y = (LCD_HEIGHT - height) / 2;
    // uint8_t spmode; // 0=normal, 1=2nd, 2=alpha-lwr, 3=alpha, 5=alpha-lwr+2nd, 9=alpha+2nd

    width = titlew = gfx_GetStringWidth(title) * 2;
    if (length * 8 > width)
        width = length * 8;

    width += 10;
    x = (LCD_WIDTH - width) / 2;

    setcol(background);
    ind = 0;
    buffer[0] = 0;
    // spmode = 0;
    for (;;)
    {
        gfx_FillRectangle(x, y, width, height);

        gfx_SetTextScale(2, 2);
        gfx_PrintStringXY(title, (LCD_WIDTH - titlew) / 2, y + 3);

        gfx_SetTextScale(1, 1);
        gfx_PrintStringXY(buffer, (LCD_WIDTH - gfx_GetStringWidth(buffer)) / 2, y + 3 + 16 + 5);
        doblit();

        sk_key_t k = getk();
        if (k == sk_Enter)
            break;
        if (k == sk_Clear)
        {
            ind = -1;
            break;
        }
        switch (k)
        {
        case sk_Del:
            if (ind)
                buffer[--ind] = 0;
            break;
        case sk_Clear:
            ind = 0;
            buffer[0] = 0;
            break;
        case sk_2nd:
            switch (spmode)
            {
            case 0:
            case 1:
                spmode = 1 - spmode;
                break;
            case 5:
                spmode = 2;
                break;
            case 9:
                spmode = 3;
                break;
            }
            break;
        case sk_Alpha:
            switch (spmode)
            {
            case 0:
            case 1:
                spmode = 3;
                break;
            case 2:
            case 5:
                spmode = 0;
                break;
            case 3:
            case 9:
                spmode = 2;
                break;
            }
            break;
        default:
            if (ind == length)
                break;
            spmode %= 4;
            if (spmode == 2)
                buffer[ind] = lower(chars[2][k - 10]);
            else if (spmode == 3)
                buffer[ind] = chars[2][k - 10];
            else
                buffer[ind] = chars[spmode][k - 10];
            if (buffer[ind])
                buffer[++ind] = 0;
            break;
        }
    }
    restcol();
    return ind;
}

uint8_t gfxutils_Select(const char *title, uint8_t selections, const char **texts)
{
    const uint16_t height = 10 + 16 + 11 * selections;
    uint16_t width, twidth, x, y;
    uint8_t sel;

    width = twidth = gfx_GetStringWidth(title) * 2;
    for (uint16_t i = 0; i < selections; i++)
        width = max(width, gfx_GetStringWidth(texts[i]));
    width += 10;

    x = (LCD_WIDTH - width) / 2;
    y = (LCD_HEIGHT - height) / 2;

    sel = 0;
    setcol(background);
    for (;;)
    {
        gfx_FillRectangle(x, y, width, height);

        gfx_SetTextScale(2, 2);
        gfx_PrintStringXY(title, (LCD_WIDTH - twidth) / 2, y + 3);

        gfx_SetTextScale(1, 1);
        for (uint8_t i = 0; i < selections; i++)
        {
            if (i == sel)
            {
                gfxutils_InvertColors();
                gfx_SetColor(background);
            }
            gfx_FillRectangle(x, y + 3 + 16 + 4 + 11 * i, width, 11);
            gfx_PrintStringXY(texts[i], (LCD_WIDTH - gfx_GetStringWidth(texts[i])) / 2, y + 3 + 16 + 5 + 11 * i);
            if (i == sel)
            {
                gfxutils_InvertColors();
                gfx_SetColor(background);
            }
        }
        // gfx_PrintStringXY(texts[i], (LCD_WIDTH - gfx_GetStringWidth(texts[i])) / 2, y + 3 + 16 + 4 + 9 * i);

        doblit();

        sk_key_t k = getk();
        if (k == sk_Enter)
            break;
        if (k == sk_Up)
        {
            if (sel == 0)
                sel = selections;
            sel--;
        }
        else if (k == sk_Down)
        {
            sel++;
            if (sel == selections)
                sel = 0;
        }
        else if (k == sk_Clear)
            return -1;
    }
    restcol();

    return sel;
}

void gfxutils_ColoredSprite(const gfx_sprite_t *sprite, uint16_t x, uint8_t y, uint8_t color, uint8_t keep)
{
    gfx_FillRectangle(x, y, sprite->width, sprite->height);
    uint8_t c = gfx_SetColor(color);
    for (uint8_t i = 0; i < sprite->width; i++)
        for (uint8_t j = 0; j < sprite->height; j++)
            if (sprite->data[i + j * sprite->width] != keep)
                gfx_SetPixel(x + i, y + j);
    gfx_SetColor(c);
}

uint8_t gfxutils_SetBackgroundColor(uint8_t color)
{
    background = color;
    gfx_SetTextTransparentColor(color);
    return gfx_SetTextBGColor(color);
}

uint8_t gfxutils_SetForegroundColor(uint8_t color)
{
    foreground = color;
    return gfx_SetTextFGColor(color);
}

void gfxutils_InvertColors(void)
{
    uint8_t f = gfxutils_SetForegroundColor(background);
    gfxutils_SetBackgroundColor(f);
}

const gfx_sprite_t *gfxutils_SetBackgroundSprite(const gfx_sprite_t *sprite)
{
    const gfx_sprite_t *s;
    s = bgsprite;
    bgsprite = sprite;
    return s;
}

uint8_t gfxutils_FullScreenSprite(const gfx_sprite_t *sprite)
{
    uint8_t scale = LCD_WIDTH / sprite->width;
    if (LCD_HEIGHT / sprite->height < scale)
        scale = LCD_HEIGHT / sprite->height;
    gfx_ScaledSprite_NoClip(sprite, (LCD_WIDTH - sprite->width * scale) / 2, 0, scale, scale);
    return scale;
}

void gfxutils_ClearScreen(void)
{
    gfx_FillScreen(background);
    if (bgsprite != NULL)
        gfxutils_FullScreenSprite(bgsprite);
}

uint8_t gfxutils_FindClosestColor(uint8_t r, uint8_t g, uint8_t b)
{
    unsigned int mindiff = 1U + INT_MAX + INT_MAX;
    uint8_t col;
    uint8_t i = 0;
    do
    {
        gfxutils_rgb888_t c = gfxutils_565To888(gfx_palette[i]);
        unsigned int rd = r - c.r, gd = g - c.g, bd = b - c.b;
        unsigned int dif = rd * rd + gd * gd + bd * bd;
        if (dif < mindiff)
        {
            mindiff = dif;
            col = i;
        }
        i++;
    } while (i);
    return col;
}

// rgb565: rrrrrggg gggbbbbb
gfxutils_rgb888_t gfxutils_565To888(uint16_t rgb565)
{
    gfxutils_rgb888_t ret;
    ret.r = ((uint8_t)(rgb565 >> 8)) & 0b11111000;
    ret.g = ((uint8_t)(rgb565 >> 3)) & 0b11111100;
    ret.b = ((uint8_t)(rgb565 << 3)) & 0b11111000;
    return ret;
}
