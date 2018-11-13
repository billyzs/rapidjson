//----------------------------------------------------------------------------
//  Copyright (C) 2018, iRobot Corporation.
//  This material contains trade secrets and confidential information
//  of iRobot Corporation.  Any use, reproduction, disclosure or
//  dissemination is strictly prohibited without the explicit written
//  permission of iRobot Corporation.  All rights reserved.
//----------------------------------------------------------------------------

/// @addtogroup json
/// @{
/// @file json-getter-impl.hpp
/// @brief implementation of json-getters.hpp; not meant to be used on its own, always include "json.hpp"
/// @}
#pragma once

#include <rapidjson/document.h>
#include <boost/optional.hpp>

namespace json {
namespace details {
using namespace rapidjson;

    template <typename T>
    using Opt = boost::optional<T>;

    /// forward decl
    template <typename RetType>
    inline Opt<RetType> get_impl(const Value& val) noexcept;
    template <typename RetType, typename std::enable_if_t<!std::is_same_v<RetType, Value::Object>> >
    inline Opt<RetType> get_proxy(const Value& val) noexcept {
        return get_impl<RetType>(val);
    }

    /// @brief helper method to check for type consistency, before attempting go get value from rapidjson::Value
    /// @tparam RetType the type of data to get
    /// @tparam Pred predicate that returns true if a rapidjson Value contains data convertible to RetType
    /// @tparam Getter returns RetType from a rapidjson Value
    template <typename Pred, typename Getter>
    auto get_helper(const Pred& pred, const Getter& getter) noexcept -> Opt<typename std::decay<decltype(getter())>::type> {
        return pred() ? boost::make_optional(getter()) : boost::none;
    }

    template <typename Val = rapidjson::Value, typename Ret>
    Opt<Ret> my_get(Val&& val) noexcept {
        return get_impl<Ret>(std::forward<Val>(val));
    }

    /// specifically for non-const Value returning Value::Object
    inline
    Opt<Value::Object> get_obj_impl(Value& val) noexcept {
        return get_helper([&val](){return val.IsObject();}, [&val](){return val.GetObject();});
    }

    inline
    Opt<Value::Object> get_impl(Value& val) noexcept {
        return get_helper([&val](){return val.IsObject();}, [&val](){return val.GetObject();});
    }

    /// specialization for bool
    template <>
    inline
    Opt<bool> get_impl(const Value& val) noexcept {
        return get_helper([&val](){return val.IsBool();}, [&val](){return val.GetBool();});
    }

    /// specialization for std::string
    template <>
    inline
    Opt<std::string> get_impl(const Value& val) noexcept {
        return get_helper([&val](){return val.IsString();}, [&val](){return std::string{val.GetString()};});
    }

    /// specialization for const char*
    template <>
    inline
    Opt<const char*> get_impl(const Value& val) noexcept {
        return get_helper([&val](){return val.IsString();}, [&val](){return val.GetString();});
    }

    /// specialization for int
    template <>
    inline
    Opt<int> get_impl(const Value& val) noexcept {
        return get_helper([&val](){return val.IsInt();}, [&val](){return val.GetInt();});
    }

    /// specialization for int64_t
    template <>
    inline
    Opt<int64_t> get_impl(const Value& val) noexcept {
        return get_helper([&val](){return val.IsInt64();}, [&val](){return val.GetInt64();});
    }

    /// specialization for uint
    template <>
    inline
    Opt<uint32_t> get_impl(const Value& val) noexcept {
        return get_helper([&val](){return val.IsUint();}, [&val](){return val.GetUint();});
    }

    /// specialization for uint64_t
    template <>
    inline
    Opt<uint64_t> get_impl(const Value& val) noexcept {
        return get_helper([&val](){return val.IsUint64();}, [&val](){return val.GetUint64();});
    }

    /// specialization for float
    template <>
    inline
    Opt<float> get_impl(const Value& val) noexcept {
        return get_helper([&val](){return val.IsLosslessFloat();}, [&val](){return val.GetFloat();});
    }

    /// specialization for double
    template <>
    inline
    Opt<double> get_impl(const Value& val) noexcept {
        return get_helper([&val](){return val.IsLosslessDouble();}, [&val](){return val.GetDouble();});
    }

    /// specialization for ConstObject
    template <>
    inline
    Opt<Value::ConstObject> get_impl(const Value& val) noexcept {
        return get_helper([&val](){return val.IsObject();}, [&val](){return val.GetObject();});
    }

} // namespace details
} // namespace json
