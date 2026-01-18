#pragma once

#include <fmt/base.h>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "singleton.hpp"

namespace selwonk::core {
// CVar system, declare vars in .cpp, they will be registered here
class Cvar : public AutoSingleton<Cvar> {
  class VarBase {
  public:
    VarBase(std::string_view name, std::string_view description)
        : mName(name), mDescription(description) {
      Cvar::get().registerVar(this);
    }

    virtual ~VarBase() = default;
    virtual void displayEdit() = 0;
    virtual void apply() = 0;
    virtual bool dirty() const = 0;

    const std::string& getName() const { return mName; }

  protected:
    std::string mName;
    std::string mDescription;
  };

  template <typename T> class Var : public VarBase {
  public:
    // Function called when a change is applied
    using ChangeCallback = std::function<void(T)>;
    // Function that returns an error message if the value is invalid
    using ValidationCallback = std::function<std::optional<std::string>(T)>;

    Var(std::string_view name, T defaultValue, std::string_view description)
        : VarBase(name, description), mDefault(defaultValue),
          mPendingChange(defaultValue), mValue(defaultValue) {}

    void addChangeCallback(ChangeCallback callback) {
      mCallbacks.push_back(callback);
    }
    void addValidationCallback(ValidationCallback callback) {
      mValidationCallbacks.push_back(callback);
    }

    void setValue(T newValue) {
      assert(validate(newValue) == std::nullopt);
      mValue = newValue;
      mPendingChange = newValue;
      for (auto& callback : mCallbacks) {
        callback(newValue);
      }
    }

    std::optional<std::string> validate(T newValue) {
      for (auto& callback : mValidationCallbacks) {
        if (auto error = callback(newValue)) {
          return error;
        }
      }
      return std::nullopt;
    }

    bool dirty() const override { return mPendingChange != mValue; }
    void apply() override { setValue(mPendingChange); }

    // Implemented manually for each specialisation
    void displayEdit() override;
    const T& value() const { return mValue; }

  private:
    std::vector<ChangeCallback> mCallbacks;
    std::vector<ValidationCallback> mValidationCallbacks;
    T mDefault;       // Hardcoded default value
    T mPendingChange; // Pending edit from user
    T mValue;         // Current value, from either runtime or config
  };

public:
  using Int = Var<int>;

  void displayUi();

private:
  void registerVar(VarBase* var) { mVars[var->getName()] = var; }

  std::map<std::string, VarBase*> mVars;
};

} // namespace selwonk::core
