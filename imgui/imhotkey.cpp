#include "imhotkey.h"
#include <iostream>

#define WIN32_LEAN_AND_MEAN
#include <array>
#include <map>
#include <windows.h>


namespace ImGui
{
    namespace
    {
        ImHotkeyData_t* capturing = nullptr;
        ImHotkeyFlags currentFlags = ImHotkeyFlags_None;

        HHOOK gMouseHook = nullptr;
        HHOOK gKeyboardHook = nullptr;

        int32_t inputStack = 0;

        std::map<std::string, unsigned short> modNameToFlag{
            {"Shift", ImHotkeyModifier_Shift}, {"Alt", ImHotkeyModifier_Alt},
            {"Ctrl", ImHotkeyModifier_Ctrl},
        };

        std::array<std::string, 6> mouseNames
                {"", "Left", "Right", "Middle", "Mouse4", "Mouse5"};

        LRESULT CALLBACK KeyboardEventProc(const int nCode, const WPARAM wParam,
                                           const LPARAM lParam)
        {
            const auto event = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
            static std::map<DWORD, bool> keyStates;

            switch (wParam) {
                case WM_KEYDOWN:
                case WM_SYSKEYDOWN: {
                    // key is being held down and is already registered
                    if (keyStates[event->vkCode]) { break; }

                    inputStack++;
                    keyStates[event->vkCode] = true;

                    if (event->vkCode == VK_LCONTROL || event->vkCode == VK_RCONTROL) {
                        if (!(currentFlags & ImHotkeyFlags_NoModifiers)) {
                            capturing->modifiers |= ImHotkeyModifier_Ctrl;
                        }
                    } else if (event->vkCode == VK_LSHIFT || event->vkCode == VK_RSHIFT) {
                        if (!(currentFlags & ImHotkeyFlags_NoModifiers)) {
                            capturing->modifiers |= ImHotkeyModifier_Shift;
                        }
                    } else if (event->vkCode == VK_LMENU || event->vkCode == VK_RMENU) {
                        if (!(currentFlags & ImHotkeyFlags_NoModifiers)) {
                            capturing->modifiers |= ImHotkeyModifier_Alt;
                        }
                    } else {
                        if (!(currentFlags & ImHotkeyFlags_NoKeyboard)) {
                            capturing->vkCode = event->vkCode;
                        }
                    }
                    break;
                }
                case WM_KEYUP:
                case WM_SYSKEYUP:
                    inputStack--;
                    keyStates[event->vkCode] = false;
                default:
                    break;
            }
            return CallNextHookEx(gKeyboardHook, nCode, wParam, lParam);
        }

        LRESULT CALLBACK MouseEventProc(const int nCode, const WPARAM wParam,
                                        const LPARAM lParam)
        {
            switch (wParam) {
                case WM_LBUTTONDOWN:
                case WM_RBUTTONDOWN:
                case WM_MBUTTONDOWN:
                case WM_XBUTTONDOWN:
                    inputStack++;
                    break;
                case WM_LBUTTONUP:
                case WM_RBUTTONUP:
                case WM_MBUTTONUP:
                case WM_XBUTTONUP:
                    inputStack--;
                default:
                    break;
            }

            if (wParam == WM_LBUTTONDOWN) {
                capturing->mouseButton = 1;
            } else if (wParam == WM_RBUTTONDOWN) {
                capturing->mouseButton = 2;
            } else if (wParam == WM_MBUTTONDOWN) {
                capturing->mouseButton = 3;
            } else if (wParam == WM_XBUTTONDOWN) {
                const MSLLHOOKSTRUCT* event = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
                capturing->mouseButton = event->mouseData == 65536 ? 4 : 5;
            }
            return CallNextHookEx(gMouseHook, nCode, wParam, lParam);
        }

        DWORD ApplyEventHook(const int id, const HOOKPROC& eventProc, HHOOK& handle)
        {
            handle = SetWindowsHookExA(id, eventProc, GetModuleHandleA(nullptr), 0);

            std::thread([handle] {
                MSG message;
                while (capturing) {
                    if (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)) {
                        TranslateMessage(&message);
                        DispatchMessage(&message);
                    }
                }
                UnhookWindowsHookEx(handle);
            }).detach();
            return 0;
        }
    }

    ImHotkeyData_t::ImHotkeyData_t(const unsigned short t_scanCode,
                                   const unsigned short t_vkCode,
                                   const unsigned short t_mouseButtons,
                                   const unsigned short t_modifiers)
        : scanCode(t_scanCode), vkCode(t_vkCode), mouseButton(t_mouseButtons),
          modifiers(t_modifiers), id_(instanceCount++) {}

    const char* ImHotkeyData_t::GetLabel()
    {
        const int32_t sum = scanCode + vkCode + mouseButton + modifiers;
        if (labelCacheSum_ != sum) {
            std::string new_;
            // track how many things we added to determine if we must place " + " first.
            int32_t additions = 0;

            // Place the mods first since the standart is Ctrl + Z rather than Z + Ctrl
            for (const auto& [name, flag]: modNameToFlag) {
                if (modifiers & flag) {
                    new_ += additions++ ? " + " + name : name;
                }
            }

            // There really is no "right" way to place mouse or keys first, either way
            // looks strange and you have to be a psycho to set a hotkey like that anyway.
            if (mouseButton) {
                const std::string name = mouseNames[mouseButton];
                new_ += additions++ ? " + " + name : name;
            }

            if (vkCode) {
                const std::string character(1, static_cast<char>(vkCode));
                new_ += additions++ ? " + " + character : character;
            }
            label_ = new_ + std::format("##{}", id_);
        }
        labelCacheSum_ = sum;
        return label_.c_str();
    }

    void ImHotkeyData_t::Reset()
    {
        modifiers = vkCode = scanCode = mouseButton = 0;
    }

    bool ImHotkey(ImHotkeyData_t* v)
    {
        return ImHotkey(v, {100, 30}, ImHotkeyFlags_Default);
    }

    bool ImHotkey(ImHotkeyData_t* v, const ImHotkeyFlags flags)
    {
        return ImHotkey(v, {100, 30}, flags);
    }

    bool ImHotkey(ImHotkeyData_t* v, const ImVec2& size, const ImHotkeyFlags flags)
    {
        static bool hadAnyInput = false;

        // What are we supposed to be capturing if any type of input is disabled..?
        assert(!(flags & ImHotkeyFlags_NoKeyboard)
            || !(flags & ImHotkeyFlags_NoMouse)
            || !(flags & ImHotkeyFlags_NoModifiers));

        if (!capturing) {
            if (Button(v->GetLabel(), size)) {
                capturing = v;
                currentFlags = flags;
                hadAnyInput = false;
                capturing->Reset();
                if (!(flags & ImHotkeyFlags_NoKeyboard) || !(
                        flags & ImHotkeyFlags_NoModifiers)) {
                    ApplyEventHook(WH_KEYBOARD_LL, KeyboardEventProc, gKeyboardHook);
                }
                if (!(flags & ImHotkeyFlags_NoMouse)) {
                    ApplyEventHook(WH_MOUSE_LL, MouseEventProc, gMouseHook);
                }
                // We reset the keybind so indicate the change
                return true;
            }
            return false;
        }

        // No more keys being held should end the capture of the widget
        hadAnyInput |= inputStack > 0;
        if (hadAnyInput && inputStack <= 0) {
            // TODO: Not threadsafe, could be set to null as input hooks try to access it
            capturing = nullptr;
        }

        BeginDisabled(true);
        if (v == capturing) {
            // Change the text to orange to indicate that it is being captured
            PushStyleColor(ImGuiCol_Text, ImVec4{1, 0.5, 0, 1});
            Button(hadAnyInput ? v->GetLabel() : "...", size);
            PopStyleColor();
        } else { Button(v->GetLabel(), size); }
        EndDisabled();
        return !capturing;
    }
}
