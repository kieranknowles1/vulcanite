#pragma once

namespace selwonk::core {
template <typename T> class Singleton {
public:
  static T& get() {
    assert(instance != nullptr && "Singleton instance is not initialized");
    return *instance;
  }

  Singleton() {
    assert(instance == nullptr && "Singleton instance already exists");
    instance = static_cast<T*>(this);
  }

  ~Singleton() { instance = nullptr; }

  // No copy/move
  Singleton(const Singleton&) = delete;
  Singleton& operator=(const Singleton&) = delete;
  Singleton(Singleton&&) = delete;
  Singleton& operator=(Singleton&&) = delete;

private:
  static T* instance;
};

// Do some template bullshit to define the static member variable
template <typename T> T* Singleton<T>::instance = nullptr;

} // namespace selwonk::core
