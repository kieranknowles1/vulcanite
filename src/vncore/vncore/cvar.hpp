#pragma once

#include <fmt/base.h>
#include <functional>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "singleton.hpp"

namespace selwonk::core {
// CVar system, declare vars in .cpp, they will be registered here
// TODO: Migrate CLI and settings to CVars
// TODO: Save/load to disk
class Cvar : public AutoSingleton<Cvar> {
public:
  enum class Flags {
    None = 0,
    // Var can only be set during initialization, and requires a restart
    // to take effect
    InitOnly = 1 << 0,
  };

  enum class TypeEnum {
    Int,
    Float,
    Bool,
    Enum,
  };

  class VarBase {
  public:
    VarBase(std::string_view name, std::string_view description, Flags flags)
        : mName(name), mDescription(description), mFlags(flags) {
      Cvar::get().registerVar(this);
    }

    virtual ~VarBase() = default;
    virtual void apply() = 0;
    virtual bool dirty() const = 0;
    virtual std::optional<std::string> isPendingValid() const = 0;
    // Set value from a string, returning false on error
    virtual bool setString(std::string_view value) = 0;
    virtual std::string toString() = 0;

    const std::string& getName() const { return mName; }
    const std::string& getDescription() const { return mDescription; }

    virtual TypeEnum getType() = 0;

    // Set pending change to default value
    virtual void setResetPending() = 0;

    bool hasFlag(Flags flag) {
      using FlagBase = std::underlying_type_t<Flags>;
      return (static_cast<FlagBase>(mFlags) & static_cast<FlagBase>(flag)) != 0;
    }

  protected:
    std::string mName;
    std::string mDescription;
    Flags mFlags;
  };

  template <typename T, TypeEnum Type> class Var : public VarBase {
  public:
    // Function called when a change is applied
    using ChangeCallback = std::function<void(T)>;
    // Function that returns an error message if the value is invalid
    using ValidationCallback = std::function<std::optional<std::string>(T)>;

    Var(std::string_view name, T defaultValue, std::string_view description,
        Flags flags = Flags::None)
        : VarBase(name, description, flags), mDefault(defaultValue),
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

    std::optional<std::string> isPendingValid() const override {
      return validate(mPendingChange);
    }

    std::optional<std::string> validate(T newValue) const {
      for (auto& callback : mValidationCallbacks) {
        if (auto error = callback(newValue)) {
          return error;
        }
      }
      return std::nullopt;
    }

    bool dirty() const override { return mPendingChange != mValue; }
    void apply() override { setValue(mPendingChange); }
    bool setString(std::string_view value) override {
      std::stringstream ss((std::string(value)));
      T val;
      ss >> val;
      if (ss.bad())
        return false;

      setValue(val);
      return true;
    }
    std::string toString() override {
      std::ostringstream ss;
      ss << mValue;
      return ss.str();
    }

    const T& value() const { return mValue; }
    T* getPendingValue() { return &mPendingChange; }
    void setPendingValue(const T& v) { mPendingChange = v; }

    TypeEnum getType() override { return Type; }

    void setResetPending() override { mPendingChange = mDefault; }

  protected:
    std::vector<ChangeCallback> mCallbacks;
    std::vector<ValidationCallback> mValidationCallbacks;
    T mDefault;       // Hardcoded default value
    T mPendingChange; // Pending edit from user
    T mValue;         // Current value, from either runtime or config
  };

  using Int = Var<int, TypeEnum::Int>;
  using Float = Var<float, TypeEnum::Float>;
  using Bool = Var<bool, TypeEnum::Bool>;

  template <typename T> class Enum : public Var<int, TypeEnum::Enum> {
  public:
    struct Option {
      std::string name;
      std::string description;
      T value;
    };
    using Backing = std::underlying_type_t<T>;
    static_assert(std::is_same_v<Backing, int>,
                  "Only integer enums are currently supported");

    Enum(std::string_view name, T defaultValue, std::string_view description,
         std::vector<Option> options, Flags flags = Flags::None)
        : Var<Backing, TypeEnum::Enum>(name, static_cast<Backing>(defaultValue),
                                       description, flags),
          mOptions(std::move(options)) {}

    const std::vector<Option>& getOptions() { return mOptions; }

  private:
    std::vector<Option> mOptions;
  };

  // Parse command line options, returns true if we should quit immediately
  // after displaying help or an invalid argument
  bool parseCli(int argc, char** argv);

  std::map<std::string, VarBase*>& getVars() { return mVars; }

private:
  void registerVar(VarBase* var) {
    assert(!mVars.contains(var->getName()) && "Duplicate CVar name");
    mVars[var->getName()] = var;
  }

  std::map<std::string, VarBase*> mVars;
};

template class Cvar::Var<int, Cvar::TypeEnum::Int>;
template class Cvar::Var<float, Cvar::TypeEnum::Float>;
template class Cvar::Var<bool, Cvar::TypeEnum::Bool>;

} // namespace selwonk::core
