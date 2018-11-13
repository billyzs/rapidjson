#ifndef RAPIDJSON_HAS_STDSTRING
#define RAPIDJSON_HAS_STDSTRING
#endif
#ifndef RAPIDJSON_HAS_CXX11_RVALUE_REFS
#define RAPIDJSON_HAS_CXX11_RVALUE_REFS 1
#endif
#include <rapidjson/document.h>
#include <gtest/gtest.h>
#include <utility>
#include <limits>
#include <optional>

namespace details {
    using namespace rapidjson;

    /// forward decl
    template <typename RetType> RetType get_impl(const Value& val, RetType default_retval) noexcept;

    /// @brief helper method to check for type consistency, before attempting go get value from rapidjson::Value
    /// @tparam RetType the type of data to get
    /// @tparam Pred predicate that returns true if a rapidjson Value contains data convertible to @tparam RetType
    /// @tparam Getter returns @tparam RetType from a rapidjson Value
    template <typename RetType, typename Pred, typename Getter>
    RetType get_helper(RetType default_retval, const Pred& pred, const Getter& getter) noexcept {
        return pred() ? getter() : default_retval;
    }

    /// specialization for bool
    template <>
    bool get_impl(const Value& val, bool default_retval) noexcept {
        return get_helper(default_retval, [&val](){return val.IsBool();}, [&val](){return val.GetBool();});
    }

    /// specialization for std::string
    template <>
    std::string get_impl(const Value& val, std::string default_retval) noexcept {
        return get_helper(std::move(default_retval), [&val](){return val.IsString();}, [&val](){return val.GetString();});
    }

    /// specialization for int
    template <>
    int get_impl(const Value& val, int default_val) noexcept {
        return get_helper(default_val, [&val](){return val.IsInt();}, [&val](){return val.GetInt();});
    }

    /// specialization for int64_t
    template <>
    int64_t get_impl(const Value& val, int64_t default_val) noexcept {
        return get_helper(default_val, [&val](){return val.IsInt64();}, [&val](){return val.GetInt64();});
    }

    /// specialization for uint
    template <>
    uint32_t get_impl(const Value& val, uint32_t default_val) noexcept {
        return get_helper(default_val, [&val](){return val.IsUint();}, [&val](){return val.GetUint();});
    }

    /// specialization for uint64_t
    template <>
    uint64_t get_impl(const Value& val, uint64_t default_val) noexcept {
        return get_helper(default_val, [&val](){return val.IsUint64();}, [&val](){return val.GetUint64();});
    }

    /// specialization for float
    template <>
    float get_impl(const Value& val, float default_val) noexcept {
        return get_helper(default_val, [&val](){return val.IsLosslessFloat();}, [&val](){return val.GetFloat();});
    }

    /// specialization for double
    template <>
    double get_impl(const Value& val, double default_val) noexcept {
        return get_helper(default_val, [&val](){return val.IsLosslessDouble();}, [&val](){return val.GetDouble();});
    }

}
/// @brief value semantics helper for creating copies of common rapidjson types
/// @tparam SrcType the type of data to copy; usually either rapidjson::Value or rapidjson::Document
/// @param value to be copied
/// @note we don't use GenericValue or GenericDocument, so I didn't bother to make this more generic
/// @note there's no meaningful way to copy Value without an allocator; for that, use the copy ctor From document.h
template <typename SrcType>
[[nodiscard]]
rapidjson::Document copy_from(const SrcType& value) {
    rapidjson::Document retval;
    retval.CopyFrom(value, retval.GetAllocator());
    return retval;
}

/// @brief overload getters for ConstObject
/// @note chose to return an optional because there's no default ctor for ConstObjects
std::optional<rapidjson::Value::ConstObject>
get(const rapidjson::Value& val) noexcept {
    return val.IsObject() ? std::make_optional(val.GetObject()) : std::nullopt;
}

std::optional<rapidjson::Value::Object>
get(rapidjson::Value& val) noexcept {
    return val.IsObject() ? std::make_optional(val.GetObject()) : std::nullopt;
}

/// @brief if @parem val is convertible to @tparam RetType, return its data; else return @param default_retval
/// @note this is type-checked & noexcept, whereas rapidjson"s GetXyz() family of functions will assert on type mismatch
/// so prefer this in our code & supply sensible defaults
template <typename RetType>
RetType get(const rapidjson::Value& val, RetType default_retval = RetType{}) noexcept {
    return details::get_impl<RetType>(val, default_retval);
}

using namespace rapidjson;
TEST(ctv, copy_from_value_to_doc) {
    Value val;
    val.SetInt(1234);
    Document val2 = copy_from(val);
    ASSERT_EQ(val, val2);
    val.SetInt(2222);
    ASSERT_NE(val, val2);

    Document obj{kObjectType};
    obj.AddMember("foo", "bar", obj.GetAllocator());

    Document obj2{kNumberType};
    obj2 = copy_from(obj);
    ASSERT_TRUE(obj.IsObject());
    ASSERT_EQ(obj.MemberCount(), 1);
    ASSERT_TRUE(obj2.IsObject());
    ASSERT_EQ(obj2.MemberCount(), 1);
    ASSERT_NE(obj2.FindMember("foo"), obj2.MemberEnd());
}

TEST(ctv, copy_from_doc_to_doc) {
    Document val;
    val.SetInt(1234);
    Document val2 = copy_from(val);
    ASSERT_EQ(val, val2);
}

TEST(ctv_get, boolean) {
    Value b{true};
    ASSERT_TRUE(b.IsBool());
    auto bb = get<bool>(b, false);
    ASSERT_EQ(bb, b.GetBool());

    Value s{"string"};
    ASSERT_TRUE(s.IsString());
    auto ss = get<bool>(s);
    ASSERT_FALSE(ss);
}

TEST(ctv_get, std_string) {

    Value s{"string"};
    ASSERT_TRUE(s.IsString());
    auto ss = get<std::string>(s);
    ASSERT_EQ(ss, std::string{"string"});

    Value b{true};
    ASSERT_TRUE(b.IsBool());
    auto bb = get<std::string>(b, "default");
    ASSERT_EQ(bb, std::string{"default"});
}

TEST(ctv_get, int_and_uint) {
    // happy path:
    Value i(std::numeric_limits<int>::max());
    ASSERT_TRUE(i.IsInt());
    ASSERT_EQ(get<int>(i), std::numeric_limits<int>::max());
    ASSERT_EQ(get<int64_t>(i), std::numeric_limits<int>::max()); // interestingly, this works

    Value i64{int64_t{std::numeric_limits<uint32_t>::max()}+1};
    ASSERT_TRUE(i64.IsInt64());
    ASSERT_EQ(get<int64_t>(i64), int64_t{std::numeric_limits<uint32_t>::max()}+1);
    ASSERT_EQ(get<uint64_t>(i64), int64_t{std::numeric_limits<uint32_t>::max()}+1); // interestingly, this works

    Value ui(std::numeric_limits<uint32_t>::max());
    ASSERT_TRUE(ui.IsUint());
    ASSERT_EQ(get<uint32_t>(ui), std::numeric_limits<uint32_t>::max());
    ASSERT_EQ(get<int64_t>(ui), std::numeric_limits<uint32_t>::max()); // interestingly, this works

    Value ui64(std::numeric_limits<uint64_t>::max());
    ASSERT_TRUE(ui64.IsUint64());
    ASSERT_EQ(get<uint64_t>(ui64), std::numeric_limits<uint64_t>::max());

    // sad path:
    ASSERT_EQ(get<int>(ui), 0);
    ASSERT_EQ(get<int>(i64), 0);
    ASSERT_EQ(get<uint32_t>(i64), 0);
    ASSERT_EQ(get<int>(ui64), 0);
    ASSERT_EQ(get<int>(ui64), 0);
    ASSERT_EQ(get<uint32_t>(ui64), 0);
}

TEST(ctv_get, float_double) {
    Value f{std::numeric_limits<float>::max()};
    ASSERT_TRUE(f.IsLosslessFloat());
    ASSERT_EQ(get<float>(f), std::numeric_limits<float>::max());
    ASSERT_EQ(get<double>(f), std::numeric_limits<float>::max());

    Value d{std::numeric_limits<double>::max()};
    ASSERT_TRUE(d.IsLosslessDouble());
    ASSERT_EQ(get<double>(d), std::numeric_limits<double>::max());
    ASSERT_EQ(get<float>(d), 0);
}

TEST(ctv_get, object) {
    Value obj{kObjectType};
    Value::AllocatorType alloc{};
    obj.AddMember("foo", "bar", alloc);
    // const Value cobj = copy_from(obj);
    ASSERT_TRUE(obj.IsObject());
    ASSERT_EQ(obj.MemberCount(), 1);
    ASSERT_NE(obj.FindMember("foo"), obj.MemberEnd());
    ASSERT_TRUE(get(obj));
    const Value::Object o = get(obj).value();
    ASSERT_NE(o.FindMember("foo"), o.MemberEnd());
    EXPECT_EQ(o["foo"], "bar");

    const Document doc = copy_from(obj);
    const Value::ConstObject co = get(doc).value();
    ASSERT_NE(co.FindMember("foo"), co.MemberEnd());
    EXPECT_EQ(co["foo"], "bar");
}
