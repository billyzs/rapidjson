//----------------------------------------------------------------------------
//  Copyright (C) 2018, iRobot Corporation.
//  This material contains trade secrets and confidential information
//  of iRobot Corporation.  Any use, reproduction, disclosure or
//  dissemination is strictly prohibited without the explicit written
//  permission of iRobot Corporation.  All rights reserved.
//----------------------------------------------------------------------------

/// @addtogroup json
/// @{
/// @file json-getters.hpp
/// @brief helper for noexcept getters; not meant to be used on its own, always include "json.hpp"
/// @}
#pragma once
#include "json-getters-impl.hpp"
#include <rapidjson/document.h>
#include <type_traits>
namespace json {

/// @brief overload getters for ConstObject
/// @note chose to return an optional because there's no default ctor for ConstObject
inline
boost::optional<rapidjson::Value::ConstObject>
get(const rapidjson::Value& val) noexcept {
    return details::get_impl<rapidjson::Value::ConstObject>(val);
}

/// @brief overload getters for Object
/// @note chose to return an optional because there's no default ctor for Object
inline
boost::optional<rapidjson::Value::Object>
get(rapidjson::Value& val) noexcept {
    return details::get_obj_impl(val);
}

/// @brief if @parem val is convertible to @tparam RetType, return its data; else return @param default_retval
/// @note this is type-checked & noexcept, whereas rapidjson"s GetXyz() family of functions will assert on type mismatch
/// so prefer this in our code & supply sensible defaults
template <typename RetType, typename Val = rapidjson::Value>
RetType get(Val&& val, RetType default_retval = RetType{}) noexcept {
    return details::get_impl<RetType>(val).value_or(default_retval);
}

/// @brief if val contains key and is convertible to RetType, return the value pointed to; else return a default val
/// @note not meant to return rapidjson::Value::Object or ::ConstObject; user the other overload for that
/// @note O(n) amortized to find the member
template <typename RetType, typename KeyType>
RetType try_get(const rapidjson::Value& val, KeyType&& key, RetType default_retval) noexcept {
    if (!val.IsObject()) return default_retval;
    const auto member_iter = val.FindMember(std::forward<KeyType>(key));
    if (member_iter != val.MemberEnd()) {
        return get<RetType>(member_iter->value, default_retval);
    } else {
        return default_retval;
    }
}

/// @brief if val contains key and is convertible to RetType, return the value pointed to; else return boost::none
/// @note O(n) amortized to find the member
template <typename RetType, typename KeyType, typename Val = rapidjson::Value>
boost::optional<RetType> try_get(Val&& val, KeyType&& key) noexcept {
    if (!val.IsObject()) return boost::none;
    const auto member_iter = std::forward<Val>(val).FindMember(std::forward<KeyType>(key));
    if (member_iter != val.MemberEnd()) {
        return details::get_proxy<RetType>(member_iter->value);
    } else {
        return boost::none;
    }
}

/// @brief overload that returns (optional of) rapidjson::Value::Object
/// @note O(n) amortized to find the member
template <typename RetType, typename KeyType,
          typename std::enable_if<std::is_same<RetType, rapidjson::Value::Object>::value, int>::type = 0>
boost::optional<RetType> try_get(rapidjson::Value& val, KeyType&& key) noexcept {
    if (!val.IsObject()) return boost::none;
    const auto member_iter = val.FindMember(std::forward<KeyType>(key));
    if (member_iter != val.MemberEnd()) {
        return details::get_obj_impl(member_iter->value);
    } else {
        return boost::none;
    }
}
} // namespace json

