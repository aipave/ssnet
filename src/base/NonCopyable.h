#pragma once

namespace ssnet {
class NonCopyable {
public:
    NonCopyable(const NonCopyable &) = delete;

    NonCopyable &operator=(const NonCopyable &) = delete;

protected:
    NonCopyable() = default;

    ~NonCopyable() = default;
};
}