#pragma once

#include <stdexcept>
#include <utility>

namespace Common::Utils {

// Tag type for unexpect value
struct unexpect_t {
    explicit unexpect_t() = default;
};

inline constexpr unexpect_t unexpect{};

// Helper function to create unexpected value
template <typename E>
class unexpected {
  public:
    using value_type = E;
    
    explicit unexpected(const E &error) : error_(error) {}
    explicit unexpected(E &&error) : error_(std::move(error)) {}
    
    const E &value() const & { return error_; }
    E &value() & { return error_; }
    const E &&value() const && { return std::move(error_); }
    E &&value() && { return std::move(error_); }
    
  private:
    E error_;
};

template <typename E>
unexpected<E> make_unexpected(const E &error) {
    return unexpected<E>(error);
}

// Simple expected implementation for C++23 std::expected
template <typename T, typename E>
class expected {
  public:
    using value_type = T;
    using error_type = E;

    // Constructors
    expected() : has_value_(true) {
        new (&storage_.value) T();
    }

    expected(const T &value) : has_value_(true) {
        new (&storage_.value) T(value);
    }

    expected(T &&value) : has_value_(true) {
        new (&storage_.value) T(std::move(value));
    }

    expected(unexpect_t, const E &error) : has_value_(false) {
        new (&storage_.error) E(error);
    }

    expected(unexpect_t, E &&error) : has_value_(false) {
        new (&storage_.error) E(std::move(error));
    }

    expected(const unexpected<E> &e) : has_value_(false) {
        new (&storage_.error) E(e.value());
    }

    expected(unexpected<E> &&e) : has_value_(false) {
        new (&storage_.error) E(std::move(e.value()));
    }

    // Copy constructor
    expected(const expected &other) : has_value_(other.has_value_) {
        if (has_value_) {
            new (&storage_.value) T(other.value());
        } else {
            new (&storage_.error) E(other.error());
        }
    }

    // Move constructor
    expected(expected &&other) noexcept : has_value_(other.has_value_) {
        if (has_value_) {
            new (&storage_.value) T(std::move(other.value()));
        } else {
            new (&storage_.error) E(std::move(other.error()));
        }
    }

    // Destructor
    ~expected() {
        if (has_value_) {
            storage_.value.~T();
        } else {
            storage_.error.~E();
        }
    }

    // Assignment operators
    expected &operator=(const expected &other) {
        if (this != &other) {
            this->~expected();
            new (this) expected(other);
        }
        return *this;
    }

    expected &operator=(expected &&other) noexcept {
        if (this != &other) {
            this->~expected();
            new (this) expected(std::move(other));
        }
        return *this;
    }

    // Value access
    T &value() & {
        if (!has_value_) {
            throw std::runtime_error("Bad expected access");
        }
        return storage_.value;
    }

    const T &value() const & {
        if (!has_value_) {
            throw std::runtime_error("Bad expected access");
        }
        return storage_.value;
    }

    T &&value() && {
        if (!has_value_) {
            throw std::runtime_error("Bad expected access");
        }
        return std::move(storage_.value);
    }

    const T &&value() const && {
        if (!has_value_) {
            throw std::runtime_error("Bad expected access");
        }
        return std::move(storage_.value);
    }

    // Error access
    E &error() & {
        if (has_value_) {
            throw std::runtime_error("Bad expected access");
        }
        return storage_.error;
    }

    const E &error() const & {
        if (has_value_) {
            throw std::runtime_error("Bad expected access");
        }
        return storage_.error;
    }

    E &&error() && {
        if (has_value_) {
            throw std::runtime_error("Bad expected access");
        }
        return std::move(storage_.error);
    }

    const E &&error() const && {
        if (has_value_) {
            throw std::runtime_error("Bad expected access");
        }
        return std::move(storage_.error);
    }

    // Observers
    explicit operator bool() const noexcept { return has_value_; }
    bool has_value() const noexcept { return has_value_; }

    T value_or(T &&default_value) const & {
        return has_value_ ? value() : std::forward<T>(default_value);
    }

    T value_or(T &&default_value) && {
        return has_value_ ? std::move(value()) : std::forward<T>(default_value);
    }

  private:
    union storage_t {
        T value;
        E error;
        storage_t() {}
        ~storage_t() {}
    };

    storage_t storage_;
    bool has_value_;
};

// Specialization for void value type
template <typename E>
class expected<void, E> {
  public:
    using value_type = void;
    using error_type = E;

    expected() : has_value_(true) {}

    expected(unexpect_t, const E &error) : has_value_(false), error_(error) {}

    expected(unexpect_t, E &&error) : has_value_(false), error_(std::move(error)) {}

    expected(const expected &other) = default;
    expected(expected &&other) = default;
    expected &operator=(const expected &other) = default;
    expected &operator=(expected &&other) = default;

    // Observers
    explicit operator bool() const noexcept { return has_value_; }
    bool has_value() const noexcept { return has_value_; }

    // Error access
    E &error() & {
        if (has_value_) {
            throw std::runtime_error("Bad expected access");
        }
        return error_;
    }

    const E &error() const & {
        if (has_value_) {
            throw std::runtime_error("Bad expected access");
        }
        return error_;
    }

    E &&error() && {
        if (has_value_) {
            throw std::runtime_error("Bad expected access");
        }
        return std::move(error_);
    }

    const E &&error() const && {
        if (has_value_) {
            throw std::runtime_error("Bad expected access");
        }
        return std::move(error_);
    }

  private:
    bool has_value_;
    E error_;
};

} // namespace Common::Utils

// Convenience aliases
namespace Common::Utils {

template <typename T, typename E>
using Expected = expected<T, E>;

} // namespace Common::Utils
