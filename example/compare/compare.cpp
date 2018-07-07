// JSON simple example
// This example does not handle errors.

#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <type_traits>


using namespace rapidjson;
using namespace std;
static const string A {R"JSON(
{
    "foo" : false,
    "obj": {"a" : 1},
    "bar" : "baz"
})JSON"};

static const string B {R"JSON(
{
    "bar": "baz",
    "foo" : false
})JSON"};

struct KeyExtractor : public BaseReaderHandler<UTF8<>, KeyExtractor> {
    vector<string> m_keys{};
    uint32_t level = 0;

    bool Key(const char* str, SizeType length, bool copy) {
        cout << "Key " << str  << endl;
        (void) length; (void) copy;
        if (level  == 1)
            m_keys.emplace_back(str);
        return true;
    }

    bool StartObject() {
        ++level;
        cout << "StartObject (level " << level << ")" << endl;
        return true;
    }

    bool EndObject(SizeType) {
        --level;
        cout << "EndObject (level " << level << ")" << endl;
        return true;
    }

    const vector<string>& get_keys() {
        cout << "keys are: ";
        for (const auto& k : m_keys)
        {
            cout << k << " ";
        }
        cout << endl;
        return m_keys;
    }
};

bool compare(const Value& A, const Value& B);

namespace details {
    using Obj = Value::ConstObject;

    bool compare_obj(const Obj& A, const Obj& B){
        bool same = !B.ObjectEmpty();
        if (!A.ObjectEmpty()) {
            for (auto mIterA = A.MemberBegin(); same && (A.MemberEnd() != mIterA); ++mIterA) {
                auto mIterB = B.FindMember(mIterA->name);
                if (mIterB != B.MemberEnd()) {
                    same = compare(mIterA->value, mIterB->value);
                }
            }

        } else {
            same = !same;
        }
        return same;
    }

    using Arr = Value::ConstArray;
    /**
     * trivially one can cross compare all elems of A with all elems of B
     * in O(m*n) time complexity where m = A.Size(), n = B.size();
     *
     * to do better than this, we notice that there are 4 types in a json: string, arithmetic, object, array;
     * array elements that are string and arithmetic types can be compared easily by using std::set or map;
     *  array elements that are objects or arrays 
     */

    bool compare_arr(const Arr& A, const Arr& B) {
        bool same = !B.Empty();
        if (!A.Empty()) {
            return same;
        } else {
            same = !same;
        }
        return same;
    }
}

bool compare(const Value& A, const Value& B) {

    // trivial case
    if (A.IsNull()) {
        return B.IsNull();
    }
    // both Objects
    else if (A.IsObject() && B.IsObject()) {
        return details::compare_obj(A.GetObject(), B.GetObject());
    }
    // both arrays
    else if (A.IsArray() && B.IsArray()) {
        return details::compare_arr(A.GetArray(), B.GetArray());
    }
    // fundamental types (char, bool, arithmetic) have overloaded operator==
    else {
        return A == B;
    }

    // assert? output diagnostic info?
}



// it only ever makes sense to compare 2 of the same type; comparisoin between different types always return false
//
//template <typename ArrayT = GenericArray<>>
//bool compare(const ArrayT)

//    StringStream ssA(A.c_str()), ssB(B.c_str());
//    KeyExtractor aKeys, bKeys;
//    Reader aReader, bReader;
//    aReader.Parse(ssA, aKeys);
//    bReader.Parse(ssB, bKeys);


void json(){
    Document aDoc, bDoc;
    aDoc.Parse(A.c_str());
    bDoc.Parse(B.c_str());

    StringStream ssA(A.c_str()), ssB(B.c_str());
    KeyExtractor aKeys, bKeys;
    Reader aReader, bReader;
    aReader.Parse(ssA, aKeys);
    bReader.Parse(ssB, bKeys);
    aKeys.get_keys();
    bKeys.get_keys();

    // GerArray()
    const string ARR {R"JSON({"foo" : [1,2,3]})JSON"};
    Document jARR; jARR.Parse(ARR.c_str());
    auto jarr = jARR["foo"].GetArray();
    static_assert(std::is_same<decltype(jarr), typename Value::Array>::value, "type must be same");

    // what is the type of the element of an array? Value?
    Document bjARR; bjARR.Parse("{\"foo\" : [3,2,1]}");
    cout << "A, B equal? " << boolalpha << (bjARR == jARR) << endl;

    // test operator== for number, bool and string
    cout << "********* operator==(number, number) *********" << endl;

    // number:
    {
        Value num1(static_cast<uint8_t>(3)), num2(3ul);
        cout << "is number object? " << num1.IsObject() << endl;
        if (num1.GetUint() == num2.GetUint()) {
            cout << "compare number Values:: " << (num1 == num2) << endl;
            cout << "compare number & string: " << (num1 == Value("3")) << endl;
        }
    }
    // bool
    {
        Value b1(true), b3(true), b4(1ul);
        cout << "is bool object? "<< b1.IsObject() << endl;
        if (b1.GetBool() == b3.GetBool()) {
            cout << "compare bool Values: " << (b1 == b3) << endl;
            cout << "compare bool & uint " << (b1 == b4) << endl;
        }
    }
    // string
    {
        Value s1("sds"), s2("sds"), s3("sds\t");
        cout << "is string object? " << s1.IsObject() << endl;
        if (s1.GetString() == s2.GetString()) {
            cout << "compare string Values: " << (s1 == s2) << endl;
            cout << "compare string Values with whitespace: " << (s1 == s3) << endl;
        }
    }

    // can you call ObjectEmpty on something else? probably not
    {
        Value foo("3");
        // cout << "object empty? " << foo.ObjectEmpty() << endl; // will assert
    }

    // how exactly does member iterator work?
    {
        string d {R"JSON(
        {
            "foo" : {
                "bar" : false,
                "baz" : 3
            },
            "woof" : true
        })JSON"};
        Document doc; doc.Parse(d.c_str());
        StringBuffer sb;
        Writer<StringBuffer> w(sb); doc.Accept(w);
        cout << "********* inspecting obj *********" << endl;
        cout << "doc : " << sb.GetString() << endl;
        auto obj = doc.GetObject();
        cout << "inspecting obj " << endl;
        cout << "does obj have \"foo\" ? " << obj.HasMember("foo") << endl;
        cout << "does obj have \"woof\" ? " << obj.HasMember("woof") << endl;
        cout << "obj has : " << endl;
        for (auto m = doc.MemberBegin(); m != doc.MemberEnd(); ++m) {
            cout << m->name.GetString() << endl;
        }

    }
    // compare() works on everything besides Array
    {
        string A_ {R"JSON(
        {
            "foo" : {
                "bar" : {
                    "boo" : []
                },
                "baz" : 3
            },
            "woof" : true
        })JSON"
        };

        string B_ {R"JSON(
        {
            "foo" : {
                "baz" : 3,
                "bar" : {
                    "boo" : []
                }
            },
            "woof" : true
        })JSON"
        };
        // ["boo", 7, {"woof" : false, "ts" : 1}]
        Document A, B;
        A.Parse(A_.c_str()); B.Parse(B_.c_str());
        cout << "compare(A, B): " << compare(A, B) << endl;
    }

}

int main() {
    cout << boolalpha;

    // KeyExtractor: array with nested objects

    return 0;
}
