#pragma once

#include <functional>
#include <utility>

/*
 * Wrapper of std::function to ease handling of callbacks. Usage:
 *
 * Declare a callback the same as a std::function:
 *
 *    Callback<void(int)> onEvent;
 *
 * Invoke the callback:
 *
 *    onEvent(5);
 *
 * Connect a standalone function to the callback:
 *
 *    static void myEventHandler(int v) {
 *        printf("i got %d\n", v);
 *    }
 *
 *    onEvent.connect(myEventHandler);
 *
 * Connect a class member function to the callback:
 *
 *    class Handler
 *    {
 *    public:
 *        void eventHandler(int v) {
 *            printf("i got %d\n", v);
 *        }
 *    };
 *
 *    Handler h;
 *    onEvent.connect(&Handler::eventHandler, &h);
 *
 * Connect a lambda:
 *
 *    onEvent.connect([](int v) {
 *        printf("i got %d\n", v);
 *    });
 *
 * Disable the callback:
 *
 *    onEvent.disconnect();
 */

namespace commkit
{

template <typename RT>
class Callback;
template <typename RT, typename... Args>
class Callback<RT(Args...)>
{
public:
    Callback() : target(nullptr)
    {
    }
    Callback(const Callback &other) = delete;       // non construction-copyable
    Callback &operator=(const Callback &) = delete; // non copyable

    using Target = std::function<RT(Args...)>;

    void connect(Target t)
    {
        target = t;
    }

    template <class T>
    void connect(RT (T::*memberfunc)(Args...), T *t)
    {
        target = do_bind<T, Args...>(memberfunc, t);
    }

    void disconnect()
    {
        target = nullptr;
    }

    void operator()(Args... args)
    {
        if (target) {
            target(std::forward<Args>(args)...);
        }
    }

private:
    Target target;

    /*
     * specializations of std::bind(), based on arity.
     * saves callers from having the specify the number
     * of placeholders manually.
     *
     * I'm not sure how to expand Args... directly within std::bind,
     * so do this manually for the max number of params we want to support.
     */

    template <class T>
    static Target do_bind(RT (T::*memberfunc)(), T *t)
    {
        return std::bind(memberfunc, t);
    }

    template <class T, typename P1>
    static Target do_bind(RT (T::*memberfunc)(P1), T *t)
    {
        return std::bind(memberfunc, t, std::placeholders::_1);
    }

    template <class T, typename P1, typename P2>
    static Target do_bind(RT (T::*memberfunc)(P1, P2), T *t)
    {
        return std::bind(memberfunc, t, std::placeholders::_1, std::placeholders::_2);
    }

    template <class T, typename P1, typename P2, typename P3>
    static Target do_bind(RT (T::*memberfunc)(P1, P2, P3), T *t)
    {
        return std::bind(memberfunc, t, std::placeholders::_1, std::placeholders::_2,
                         std::placeholders::_3);
    }

    template <class T, typename P1, typename P2, typename P3, typename P4>
    static Target do_bind(RT (T::*memberfunc)(P1, P2, P3, P4), T *t)
    {
        return std::bind(memberfunc, t, std::placeholders::_1, std::placeholders::_2,
                         std::placeholders::_3, std::placeholders::_4);
    }

    template <class T, typename P1, typename P2, typename P3, typename P4, typename P5>
    static Target do_bind(RT (T::*memberfunc)(P1, P2, P3, P4, P5), T *t)
    {
        return std::bind(memberfunc, t, std::placeholders::_1, std::placeholders::_2,
                         std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    }
};

} // namespace commkit
