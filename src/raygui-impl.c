#include "raygui-impl.h"
#include "raylib.h"

#define RAYGUI_IMPLEMENTATION 1
#include "raygui.h"

/*************************************************************************************************/

void GuiDrawCustomIcon(Texture2D icons, int iconSize, int index, int posX, int posY, float scale, Color tint)
{
    int iconsPerRow, column, row;
    Rectangle icon, bounds;
    Vector2 origin;

    if (icons.id == 0)
        return;

    iconsPerRow = icons.width / iconSize;
    column = index % iconsPerRow;
    row = index / iconsPerRow;

    icon.x = (float)(iconSize * column);
    icon.y = (float)(iconSize * row);
    icon.width = (float)(iconSize);
    icon.height = (float)(iconSize);

    bounds.x = (float)(posX);
    bounds.y = (float)(posY);
    bounds.width = (float)(iconSize * scale);
    bounds.height = (float)(iconSize * scale);

    origin.x = 0.0f;
    origin.y = 0.0f;

    DrawTexturePro(icons, icon, bounds, origin, 0.0f, tint);
}

int GuiButtonEx(Rectangle bounds, const char* text)
{
    GuiState state = guiState;
    int event = 0;

    // Update control
    //--------------------------------------------------------------------
    if ((state != STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        // Check button state
        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = STATE_PRESSED;
            else if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) state = STATE_PRESSED;
            else state = STATE_FOCUSED;

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) event = 1;
            if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) event = 2;
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    GuiDrawRectangle(bounds, GuiGetStyle(BUTTON, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(BUTTON, BORDER + (state * 3))), guiAlpha), Fade(GetColor(GuiGetStyle(BUTTON, BASE + (state * 3))), guiAlpha));
    GuiDrawText(text, GetTextBounds(BUTTON, bounds), GuiGetStyle(BUTTON, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(BUTTON, TEXT + (state * 3))), guiAlpha));

    if (state == STATE_FOCUSED) GuiTooltip(bounds);
    //------------------------------------------------------------------

    return event;
}

int GuiIconButtonEx(Rectangle bounds, const char* tooltip, Texture2D icons, int iconSize,
    int padding, int icon, float iconScale)
{
    int result;

    GuiSetTooltip(tooltip);
    result = GuiButtonEx(bounds, "");
    GuiDrawCustomIcon(icons, iconSize, icon, (int)bounds.x + padding, (int)bounds.y + padding, iconScale,
        RAYWHITE);

    return result;
}

int GuiInvisibleButtonEx(Rectangle bounds)
{
    GuiState state = guiState;
    int event = 0;

    // Update control
    //--------------------------------------------------------------------
    if ((state != STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        // Check button state
        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = STATE_PRESSED;
            else if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) state = STATE_PRESSED;
            else state = STATE_FOCUSED;

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) event = 1;
            if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) event = 2;
        }
    }

    return event;
}

/*************************************************************************************************/

void IntegerToText(int val, char* text)
{
    int num, length = 0, div = 1000000000;

    if (val == 0)
    {
        memcpy(text, "0", 2);
        return;
    }

    for (; div > 0; div /= 10)
    {
        if (((num = (val / div) % 10)) || (length > 0))
            text[length++] = '0' + num;
    }

    text[length] = 0;
}

Rectangle ClampBounds(Rectangle child, Rectangle parent)
{
    float offset;

    if (child.width > parent.width)
        child.width = parent.width;

    if (child.height > parent.height)
        child.height = parent.height;

    offset = parent.x - child.x;

    if (offset > 0)
        child.x -= offset;

    offset = parent.y - child.y;

    if (offset > 0)
        child.y -= offset;

    offset = (child.x + child.width) - (parent.x + parent.width);

    if (offset > 0)
        child.x -= offset;

    offset = (child.y + child.height) - (parent.y + parent.height);

    if (offset > 0)
        child.y -= offset;

    return child;
}

float ScaleToFit(float childWidth, float childHeight, float parentWidth, float parentHeight)
{
    float widthScale = parentWidth / childWidth;
    float heightScale = parentHeight / childHeight;
    return widthScale < heightScale ? widthScale : heightScale;
}

int IntegerAlignUp(int val, int mod)
{
    int rest = val & (mod - 1);
    return rest ? val - rest + mod : val;
}

int IntegerAlignDown(int val, int mod)
{
    int rest = val & (mod - 1);
    return rest ? val - rest : val;
}