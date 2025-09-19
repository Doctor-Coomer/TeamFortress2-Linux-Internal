#pragma once
#include "imgui.h"

#include <string>

using ImHotkeyFlags = int;

/**
 * @brief Flags to control the behavior of an ImHotkey widget
 */
enum ImHotkeyFlags_
{
    ImHotkeyFlags_None = 0,
    ImHotkeyFlags_NoModifiers = 1,     // Ignore modifiers keys (shift, alt, ctrl..)
    ImHotkeyFlags_NoKeyboard = 1 << 1, // Ignore the keyboard (modifiers not included)
    ImHotkeyFlags_NoMouse = 1 << 2,    // Ignore the mouse (including side buttons)

    ImHotkeyFlags_Default = ImHotkeyFlags_None // Default flags to use
};


/**
 * @brief Bitmasks to check whether a modifier is set, check bitfield & (AND) bitmask
 * if the modifier is enabled, the result will be nonzero.
 */
enum ImHotkeyModifiers_
{
    ImHotkeyModifier_Shift = 0x1,  // Shift modifier is part of the hotkey
    ImHotkeyModifier_Alt = 0x10,   // Alt modifier is part of the hotkey
    ImHotkeyModifier_Ctrl = 0x100, // Ctrl modifier is part of the hotkey
};

namespace ImGui
{
    /**
     * @brief Container for the relevant data of a hotkey.
     */
    struct ImHotkeyData_t
    {
    public:
        ImHotkeyData_t(unsigned short t_scanCode, unsigned short t_vkCode,
                       unsigned short t_mouseButtons = 0, unsigned short t_modifiers = 0);

        // The scan code of the key, scan codes are keyboard hardware dependent
        unsigned short scanCode = 0;

        // The virtual keycode of the the key, constant and set based on the scancode
        unsigned short vkCode = 0;

        // Either none, left, right, middle, mouse4 or mouse5 (0 - 5)
        unsigned short mouseButton = 0;

        // Could be shift, ctrl or alt (0x1, 0x10 or 0x100)
        unsigned short modifiers = 0;

        /**
         * @brief Get the label for this hotkey data, made up of the hotkeys set.
         *
         * @return Label of this hotkey, for example `Alt + F4##0`, `Ctrl + C##0` or `Z##0`
         *
         * @remark To ensure two instances sharing the same data wont end up using the
         * same label, causing the widgets to stay in sync, a generated ID is appended to
         * the label behind the ##, that way it will not be displayed.
         */
        [[nodiscard]] const char* GetLabel();

        /**
         * @brief Resets the keybind (sets all values back to 0).
         */
        void Reset();

    private:
        // The label of the widget is made up from the buttons and modifiers, for example
        // "Alt + F4" or "Ctrl + Z" or "A". Be mindful of label collisions.
        std::string label_;

        // A unique ID for this instance to generate a unique label, preventing collisions.
        int32_t id_;

        // The sum of all fields together to determine whether the values have changed
        // since the last time the label has been converted to a string representation
        int32_t labelCacheSum_ = -1;

        static inline int32_t instanceCount = 0;
    };

    IMGUI_API bool ImHotkey(ImHotkeyData_t* v);

    IMGUI_API bool ImHotkey(ImHotkeyData_t* v, ImHotkeyFlags flags);

    IMGUI_API bool ImHotkey(ImHotkeyData_t* v, const ImVec2& size);

    IMGUI_API bool ImHotkey(ImHotkeyData_t* v, const ImVec2& size, ImHotkeyFlags flags);
}
