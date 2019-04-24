#pragma once

#include <iostream>

namespace tsa {

    constexpr auto& print_stream = std::cout;
    constexpr auto& error_stream = std::cerr;

    template<class... T>
    inline decltype(print_stream) print(T&&... args) {
        return (print_stream<< ...<< std::forward<T&&>(args));
    }

    template<class... T>
    inline decltype(print_stream) println(T&&... args) {
        return ((print()) << ...<< std::forward<T&&>(args)) << std::endl;
    }

    template<class... T>
    inline decltype(error_stream) error(T&&... args) {
        return (error_stream << ...<< std::forward<T&&>(args));
    }

    template<class... T>
    inline decltype(error_stream) errorln(T&&... args) {
        return ((error()) << ...<< std::forward<T&&>(args)) << std::endl;
    }


}