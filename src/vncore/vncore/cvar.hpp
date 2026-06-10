#pragma once

#include <functional>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "singleton.hpp"
#include "vncore/util.hpp"

namespace selwonk::core {
// CVar system, declare vars in .cpp, they will be registered here
// TODO: Save/load to disk
class Cvar : public AutoSingleton<Cvar> {
public:
  enum class Flags {
    None = 0,
    // Var can only be set during initialization, and requires a restart
    // to take effect
    InitOnly = 1 << 0,
    // Var must be >= 0. Only applies to ints and floats
    Unsigned = 1 << 1,
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
    virtual std::optional<std::string> validatePending() const = 0;
    // Set value from a string, returning false on error
    virtual bool setString(std::string_view value) = 0;
    virtual std::string toString() const = 0;

    const std::string& getName() const { return mName; }
    const std::string& getDescription() const { return mDescription; }

    virtual constexpr TypeEnum getType() const = 0;

    // Set pending change to default value
    virtual void setResetPending() = 0;

    constexpr bool hasFlag(Flags flag) const {
      return util::hasFlag(mFlags, flag);
    }

  protected:
    std::string mName;
    std::string mDescription;
    Flags mFlags;
  };

  template <typename T> struct Store {
    // Function called when a change is applied
    using ChangeCallback = std::function<void(T)>;
    // Function that returns an error message if the value is invalid
    using ValidationCallback = std::function<std::optional<std::string>(T)>;

    Store(const T& defaultValue)
        : mValue(defaultValue), mDefault(defaultValue), mPending(defaultValue) {
    }

    void addChange(ChangeCallback cb) { mOnChange.emplace_back(cb); }
    void addValidate(ValidationCallback cb) { mOnValidate.emplace_back(cb); }

    void fireChange(const T& value) const {
      for (auto& cb : mOnChange) {
        cb(value);
      }
    }
    std::optional<std::string> fireValidate(const T& value) const {
      for (auto& cb : mOnValidate) {
        if (auto err = cb(value))
          return err;
      }
      return std::nullopt;
    }

    T mValue;   // Current value, from either runtime or config
    T mDefault; // Hardcoded default value
    T mPending; // Pending edit from user

    std::vector<ChangeCallback> mOnChange;
    std::vector<ValidationCallback> mOnValidate;
  };

  template <typename T, TypeEnum Type> class Var : public VarBase {
  public:
    Var(std::string_view name, T defaultValue, std::string_view description,
        Flags flags = Flags::None)
        : VarBase(name, description, flags), mStore(defaultValue) {}

    Store<T>& getStore() { return mStore; }

    void setValue(T newValue) {
      assert(validate(newValue) == std::nullopt);
      mStore.mValue = newValue;
      mStore.mPending = newValue;
      mStore.fireChange(newValue);
    }

    std::optional<std::string> validatePending() const override {
      return validate(mStore.mPending);
    }

    std::optional<std::string> validate(T newValue) const {
      if constexpr (Type == TypeEnum::Int || Type == TypeEnum::Float) {
        if (hasFlag(Flags::Unsigned) && newValue < 0) {
          return "Unsigned value must be >= 0";
        }
      }

      return mStore.fireValidate(newValue);
    }

    bool dirty() const override { return mStore.mPending != mStore.mValue; }
    void apply() override { setValue(mStore.mPending); }
    bool setString(std::string_view value) override {
      std::stringstream ss((std::string(value)));
      T val;
      ss >> val;
      if (!ss.eof())
        return false;

      setValue(val);
      return true;
    }
    std::string toString() const override {
      std::ostringstream ss;
      ss << mStore.mValue;
      return ss.str();
    }

    const T& value() const { return mStore.mValue; }
    T* getPendingValue() { return &mStore.mPending; }
    void setPendingValue(const T& v) { mStore.mPending = v; }

    constexpr TypeEnum getType() const override { return Type; }

    void setResetPending() override { mStore.mPending = mStore.mDefault; }

  protected:
    Store<T> mStore;
  };

  using Int = Var<int, TypeEnum::Int>;
  using Float = Var<float, TypeEnum::Float>;
  using Bool = Var<bool, TypeEnum::Bool>;

  class EnumBase : public VarBase {
  public:
    EnumBase(std::string_view name, std::string_view description, Flags flags)
        : VarBase(name, description, flags) {}

    // Inherited via VarBase
    constexpr TypeEnum getType() const override { return TypeEnum::Enum; }

    virtual void optionInfo(int i, int* intValue, const std::string** name,
                            const std::string** description) const = 0;
    virtual int optionCount() const = 0;
    virtual int getPendingInt() const = 0;
    virtual void setPendingInt(int v) = 0;
  };

  template <typename T> class Enum : public EnumBase {
  public:
    struct Option {
      std::string name;
      std::string description;
      T value;
    };
    using Backing = std::underlying_type_t<T>;

    Store<T>& getStore() { return mStore; }

    std::string generateDescription(std::string_view base,
                                    const std::vector<Option>& options) const {
      std::stringstream ss;
      ss << base;
      for (const auto& opt : options) {
        ss << "\n\t" << opt.name;
        if (opt.description != "") {
          ss << ": " << opt.description;
        }
      }
      return ss.str();
    }

    std::string toString() const override {
      for (auto& opt : mOptions) {
        if (opt.value == mStore.mValue) {
          return opt.name;
        }
      }
      std::unreachable();
    }
    bool setString(std::string_view value) override {
      for (auto& opt : mOptions) {
        if (opt.name == value) {
          mStore.mValue = opt.value;
          return true;
        }
      }
      return false;
    }

    Enum(std::string_view name, T defaultValue, std::string_view description,
         std::vector<Option> options, Flags flags = Flags::None)
        : EnumBase(name, generateDescription(description, options), flags),
          mStore(defaultValue), mOptions(std::move(options)) {}

    T value() { return mStore.mValue; }

    const std::vector<Option>& getOptions() { return mOptions; }

  private:
    Store<T> mStore;
    std::vector<Option> mOptions;

    // Inherited via EnumBase
    void apply() override { mStore.mValue = mStore.mPending; }
    bool dirty() const override { return mStore.mValue == mStore.mPending; }
    std::optional<std::string> validatePending() const override {
      return mStore.fireValidate(mStore.mPending);
    }
    void setResetPending() override { mStore.mPending = mStore.mDefault; }

    void optionInfo(int i, int* intValue, const std::string** name,
                    const std::string** description) const override {
      *intValue = (int)mOptions[i].value;
      *name = &mOptions[i].name;
      *description = &mOptions[i].description;
    }
    int optionCount() const override { return mOptions.size(); }
    int getPendingInt() const override { return (int)mStore.mPending; }
    void setPendingInt(int v) override { mStore.mPending = (T)v; }
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
