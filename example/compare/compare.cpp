// JSON simple example
// This example does not handle errors.

#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <type_traits>


using namespace rapidjson;
using namespace std;


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
        bool same;
        if (A.Size() == B.Size() && !(A.Empty() || B.Empty())) {
            auto eIterA = A.Begin();
            while (B.End() != std::find_if(B.Begin(), B.End(),
                                           [&eIterA](const Value& elemB) {
                                               return compare(*(++eIterA), elemB);
                                           }));

            // done searching;
            same =  (eIterA = A.End()); // that means all elements in A are in B
        } else {
            same = (A.Empty() && B.Empty());
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
}




int main() {
    cout << boolalpha;

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

    return 0;
}
