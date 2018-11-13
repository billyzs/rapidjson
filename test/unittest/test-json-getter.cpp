#include "json-getters.hpp"
#include <gtest/gtest.h>
#include <cstring>
#include <limits>
#include <string>
#include <type_traits>

using namespace rapidjson;
using namespace json;
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

TEST(ctv_get, const_char_ptr) {

    Value s{"string"};
    ASSERT_TRUE(s.IsString());
    auto ss = get<const char*>(s);
    ASSERT_EQ(std::strcmp(ss, "string"), 0);

    Value b{true};
    ASSERT_TRUE(b.IsBool());
    const char* blah = "blah";
    ASSERT_EQ(get<const char*>(b, blah),blah);
    auto bb = get<const char*>(b, blah);
    ASSERT_EQ(bb, blah);

    bb = get<const char*>(b);
    ASSERT_FALSE(bb);

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

    ASSERT_TRUE(obj.IsObject());
    ASSERT_EQ(obj.MemberCount(), 1);
    ASSERT_NE(obj.FindMember("foo"), obj.MemberEnd());
    ASSERT_TRUE(get(obj));
    const Value::Object o = get(obj).value();
    ASSERT_NE(o.FindMember("foo"), o.MemberEnd());
    EXPECT_EQ(o["foo"], "bar");

//    const Document doc = copy_from(obj);
//    const Value::ConstObject co = get(doc).value();
//    ASSERT_NE(co.FindMember("foo"), co.MemberEnd());
//    EXPECT_EQ(co["foo"], "bar");
}

TEST(ctv_get, try_get_value) {
    Value obj{kObjectType};
    Value::AllocatorType alloc{};
    obj.AddMember("foo", "bar", alloc);

    ASSERT_TRUE(obj.IsObject());
    ASSERT_EQ(obj.MemberCount(), 1);
    ASSERT_NE(obj.FindMember("foo"), obj.MemberEnd());

    // try_get with default
    ASSERT_FALSE(try_get<bool>(obj, "foo", false)); 
    ASSERT_TRUE(try_get<std::string>(obj, "baz", "").empty()); 
    auto bar = try_get<std::string>(obj, "foo", ""); 
    ASSERT_EQ(bar, "bar");

    auto o = try_get<std::string>(obj, "foo");
    static_assert(std::is_same<decltype(o), boost::optional<std::string> >::value, "o is a boost::optional<std::string>");
    ASSERT_EQ(*o, std::string("bar"));
}

TEST(ctv_get, try_get_obj) {
    Document doc{kObjectType};
    Value obj2{kObjectType};
    obj2.AddMember("foo", "bar", doc.GetAllocator());
    doc.AddMember("obj", obj2, doc.GetAllocator());

    const Document& cdoc = doc;
    auto cobj = try_get<Value::ConstObject>(cdoc, "obj");
    ASSERT_FALSE(cobj->ObjectEmpty()) << "try_get returns an optional which has value semantics but pointer syntax";
    ASSERT_EQ(cobj->MemberCount(), 1);
    auto bar = get<std::string>((*cobj)["foo"]);
    EXPECT_EQ(bar, std::string("bar"));
    
    auto nc_obj = *(try_get<Value::Object>(doc, "obj"));
    static_assert(std::is_same<decltype(nc_obj), Value::Object>::value, "nc_object is rapidjson::Value::Object");
    ASSERT_FALSE(nc_obj.ObjectEmpty()) 
        << "or you can 'dereference' the call to get a value; but it will assert if the call returned boost::none";
    ASSERT_EQ(nc_obj.MemberCount(), 1);
    bar = get<std::string>(nc_obj["foo"]);
    EXPECT_EQ(bar, std::string{"bar"});
}

