/**
 * GfxUtils - utils to use in conjuction with the GRAPHX library
 * @author david-why
 */

#ifndef _GFXUTILS_H
#define _GFXUTILS_H

#include <graphx.h>

extern const char *capital_chars;
extern const char *yes_no[2];
extern const char *no_yes[2];

/**
 * Displays a textbox in the middle of the screen.
 * 
 * @param title The title to display in the box.
 * @param lines The number of lines in the box.
 * @param texts The lines to display in the box.
 */
void gfxutils_TextBox(const char *title, uint8_t lines, const char **texts);

/**
 * Displays a box in the middle of the screen to get text input from the user. 
 * Uses the background color set via \c gfxutils_SetBackgroundColor() and the 
 * foreground color set via \c gfxutils_SetForegroundColor(). Returns -1 when 
 * the user presses [clear] to quit.
 * 
 * @param title The title to display in the box.
 * @param buffer The buffer for the user to type in data.
 * @param length Maximum text length for the user to input. Does not include
 * string terminator.
 * @param spmode Special mode: 0=normal, 1=2nd, 2=alpha lowercase, 3=alpha 
 * uppercase, 5=alpha lowercase+2nd, 9=alpha uppercase+2nd. Default 0.
 * @returns Number of characters the user typed in.
 */
uint16_t gfxutils_GetTextInput(const char *title, char *buffer, uint16_t length, uint8_t spmode = 0);

/**
 * Displays a box in the middle of the screen to prompt the user to make a 
 * selection. Uses the background color set via 
 * \c gfxutils_SetBackgroundColor() and the foreground color set via 
 * \c gfxutils_SetForegroundColor(). Returns -1 when the user presses [clear] 
 * to quit.
 * 
 * @param title The title to display in the box.
 * @param selections The number of selections to display.
 * @param texts The text to display for each selection.
 * @returns The zero-based index of the user's selection.
 */
uint8_t gfxutils_Select(const char *title, uint8_t selections, const char **texts);

/**
 * Displays a sprite that has all its pixels not equal to \p keep filled with 
 * the \p color.
 * 
 * @param sprite The sprite to color and display.
 * @param x The X coordinates.
 * @param y The Y coordinates.
 * @param color The fill color.
 * @param keep The color to not fill. Usually, this is the background color.
 */
void gfxutils_ColoredSprite(const gfx_sprite_t *sprite, uint16_t x, uint8_t y, uint8_t color, uint8_t keep);

/**
 * Sets the global background color used in various gfxutils functions. 
 * This also calls \c gfx_SetTextBGColor() and 
 * \c gfx_SetTextTransparentcolor().
 * 
 * @param color The background color to set.
 * @returns The previous text background color.
 */
uint8_t gfxutils_SetBackgroundColor(uint8_t color);

/**
 * Sets the global foreground color used in various gfxutils functions. 
 * This also calls \c gfx_SetTextFGColor().
 * 
 * @param color The foreground color to set.
 * @returns The previous text foreground color.
 */
uint8_t gfxutils_SetForegroundColor(uint8_t color);

/**
 * Inverts the foreground and background colors.
 */
void gfxutils_InvertColors(void);

/**
 * Sets the background image. 
 * This image will display when you call \c gfxutils_ClearScreen().
 * 
 * @param sprite The background image to set, or \c NULL to disable background
 * image. Image will be scaled to fit.
 * @returns The previously set background image.
 * @see gfxutils_ClearScreen
 */
const gfx_sprite_t *gfxutils_SetBackgroundSprite(const gfx_sprite_t *sprite);

/**
 * Displays an image in fullscreen, scaling as required.
 * 
 * @param sprite The image to draw.
 * @returns The scale used.
 */
uint8_t gfxutils_FullScreenSprite(const gfx_sprite_t *sprite);

/**
 * Clears the screen. If a background image is set, display the image. Else 
 * fill the screen with background color.
 * 
 * @see gfxutils_SetBackgroundSprite
 * @see gfxutils_SetBackgroundColor
 */
void gfxutils_ClearScreen(void);

/**
 * Structure of a 24-bit RGB color, with 8 bits of red, green, and blue.
 */
typedef struct
{
    uint8_t r, g, b;
} gfxutils_rgb888_t;

/**
 * Finds the closest RGB color in the current palette.
 * 
 * @param r The red value.
 * @param g The green value.
 * @param b The blue value.
 * @returns The closest color index in the current palette.
 */
uint8_t gfxutils_FindClosestColor(uint8_t r, uint8_t g, uint8_t b);

/**
 * Convert a 16-bit RGB565 color to a RGB888 color.
 * 
 * @param rgb565 The RGB565 color, typically found in the GRAPHX palette.
 * @returns The RGB888 color.
 */
gfxutils_rgb888_t gfxutils_565To888(uint16_t rgb565);

#endif