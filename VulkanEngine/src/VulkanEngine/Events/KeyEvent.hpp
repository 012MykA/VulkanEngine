#pragma once

#include "VulkanEngine/Events/Event.hpp"
#include "VulkanEngine/Core/KeyCodes.hpp"

namespace ve
{
    class KeyEvent : public Event
    {
    public:
        KeyCode GetKeyCode() const { return m_KeyCode; }

        EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

    protected:
        KeyEvent(KeyCode keycode) : m_KeyCode(keycode) {}

        KeyCode m_KeyCode;
    };

    class KeyPressedEvent : public KeyEvent
    {
    public:
        KeyPressedEvent(KeyCode keycode, int repeatCount) : KeyEvent(keycode), m_RepeatCount(repeatCount) {}

        int GetRepeatCount() const { return m_RepeatCount; }

        EVENT_CLASS_TYPE(KeyPressed)

    private:
        int m_RepeatCount;
    };

    class KeyReleasedEvent : public KeyEvent
    {
    public:
        KeyReleasedEvent(KeyCode keycode) : KeyEvent(keycode) {}

        EVENT_CLASS_TYPE(KeyReleased)
    };

    class KeyTypedEvent : public KeyEvent
    {
    public:
        KeyTypedEvent(KeyCode codepoint) : KeyEvent(codepoint) {}

        EVENT_CLASS_TYPE(KeyTyped)
    };

} // namespace ve
