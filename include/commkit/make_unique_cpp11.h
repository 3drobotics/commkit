#pragma once

/*
 * C++11 support for make_unique() from https://isocpp.org/files/papers/N3656.txt,
 * pointed to from
 * http://stackoverflow.com/questions/17902405/how-to-implement-make-unique-function-in-c11
 *
 * Herb Sutter sez (http://herbsutter.com/gotw/_102/):
 *
 * "That C++11 doesn’t include make_unique is partly an oversight,
 * and it will almost certainly be added in the future.
 * In the meantime, use the one provided below."
 */

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

// only implement for C++11 - C++14 provides this
#if __cplusplus == 201103L

// option to disable
#ifndef COMMKIT_DISABLE_MAKE_UNIQUE

namespace std
{

template <class T>
struct _Unique_if {
    typedef unique_ptr<T> _Single_object;
};

template <class T>
struct _Unique_if<T[]> {
    typedef unique_ptr<T[]> _Unknown_bound;
};

template <class T, size_t N>
struct _Unique_if<T[N]> {
    typedef void _Known_bound;
};

template <class T, class... Args>
typename _Unique_if<T>::_Single_object make_unique(Args &&... args)
{
    return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <class T>
typename _Unique_if<T>::_Unknown_bound make_unique(size_t n)
{
    typedef typename remove_extent<T>::type U;
    return unique_ptr<T>(new U[n]());
}

template <class T, class... Args>
typename _Unique_if<T>::_Known_bound make_unique(Args &&...) = delete;

} // namespace std

#endif // COMMKIT_DISABLE_MAKE_UNIQUE
#endif // __cplusplus
