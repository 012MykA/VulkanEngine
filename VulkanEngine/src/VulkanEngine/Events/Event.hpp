#pragma once

#include "VulkanEngine/Core/Base.hpp"

#include <functional>

namespace VE
{
    enum class EventType
    {
        None = 0,
        WindowClose,
        WindowResize,
        WindowFocus,
        WindowLostFocus,
        WindowMoved,
        KeyPressed,
        KeyReleased,
        KeyTyped,
        MouseButtonPressed,
        MouseButtonReleased,
        MouseMoved,
        MouseScrolled
    };

    enum EventCategory
    {
        None = 0,
        EventCategoryApplication = BIT(0),
        EventCategoryInput = BIT(1),
        EventCategoryKeyboard = BIT(2),
        EventCategoryMouse = BIT(3),
        EventCategoryMouseButton = BIT(4)
    };

#define EVENT_CLASS_TYPE(type)                                   \
    static EventType GetStaticType() { return EventType::type; } \
    virtual EventType GetEventType() const override { return GetStaticType(); }

#define EVENT_CLASS_CATEGORY(category) \
    virtual int GetCategoryFlags() const override { return category; }

    class Event
    {
    public:
        virtual ~Event() = default;

        virtual EventType GetEventType() const = 0;
        virtual int GetCategoryFlags() const = 0;

        bool IsInCategory(EventCategory category)
        {
            return GetCategoryFlags() & category;
        }

    protected:
        bool m_Handled = false;
    };

    class EventDispatcher
    {
        template <typename T>
        using EventFn = std::function<bool(T &)>;

    public:
        EventDispatcher(Event &event) : m_Event(event) {}

        template <typename T>
        bool Dispatch(EventFn<T> func)
        {
            static_assert(std::is_base_of<Event, T>::value, "Dispathed type is not a subclass of Event!");
            if (m_Event.GetEventType() == T::GetStaticType())
            {
                m_Event.m_Handled = func(*reinterpret_cast<T *>(&m_Event));
                return true;
            }
            return false;
        }

    private:
        Event &m_Event;
    };

} // namespace VE
