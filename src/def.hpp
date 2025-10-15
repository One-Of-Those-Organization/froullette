#ifndef DEF_HPP_
#define DEF_HPP_

#include <memory>

template <typename T>
using sptr_t = std::shared_ptr<T>;

template <typename T, typename... Args>
sptr_t<T> mk_sptr(Args &&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

#endif // DEF_HPP_
