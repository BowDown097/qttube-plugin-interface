#pragma once
#include "qttube-plugin/components/reply.h"
#include <bitset>
#include <QMutexLocker>

namespace QtTubePlugin
{
    template<typename T>
    inline constexpr bool is_reply_v = false;

    template<typename T>
    inline constexpr bool is_reply_v<Reply<T>> = true;

    // this should be used when returning results immediately in a reply.
    // this is quite spaghetti tho, i don't like it :(
    template<typename Object, typename Function>
        requires std::is_member_function_pointer_v<Function> && std::derived_from<Object, QObject>
    void invokeQueued(Object* obj, Function func, auto&&... args)
    {
        QMetaObject::invokeMethod(obj, [func, obj, ...args = std::forward<decltype(args)>(args)]() mutable {
            (obj->*func)(std::forward<decltype(args)>(args)...);
        }, Qt::QueuedConnection);
    }

    // small utility class to handle completion tracking of multiple concurrent tasks
    template<size_t N>
    class MultiCompletionState
    {
    public:
        bool hit()
        {
            QMutexLocker locker(&mutex);
            fbits.set(position++);
            return fbits.all();
        }
    private:
        std::bitset<N> fbits;
        QMutex mutex;
        size_t position{};
    };
}
