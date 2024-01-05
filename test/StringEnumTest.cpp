// This code is licenced under the MIT license:
//
// Copyright (c) 2024 Hottinger Brüel & Kjær
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "jetproxy/StringEnum.hpp"
#include <gtest/gtest.h>


namespace hbk::jetproxy
{
    static const hbk::jetproxy::StringEnum::Members members = {{0, "zero"}, {1, "one"}, {2, "two"}};
    TEST(fb_test, fb_stringenum_test)
    {
        int result;
        int key;
        std::string string;
        StringEnum stringEnum(members);
        key = stringEnum.getKey();
        string = stringEnum.getString();
        ASSERT_EQ(key, members.begin()->first);
        ASSERT_EQ(string, members.begin()->second);
    
        stringEnum.setKey(0);
        // change
        result = stringEnum.setKey(2);
        ASSERT_EQ(result, 1);
        // no change
        result = stringEnum.setKey(2);
        ASSERT_EQ(result, 0);
        // not valid!
        result = stringEnum.setKey(99);
        ASSERT_EQ(result, -1);
    
        stringEnum.setString("zero");
        // change
        result = stringEnum.setString("one");
        ASSERT_EQ(result, 1);
        // no change
        result = stringEnum.setString("one");
        ASSERT_EQ(result, 0);
        // not valid!
        result = stringEnum.setString("none");
        ASSERT_EQ(result, -1);
    
        // set by key
        for (const auto& iter : members)
        {
            stringEnum.setKey(iter.first);
            key = stringEnum.getKey();
            string = stringEnum.getString();
            ASSERT_EQ(key, iter.first);
            ASSERT_EQ(string, iter.second);
        }
    
        // set by value
        for (const auto& iter : members)
        {
            stringEnum.setString(iter.second);
            key = stringEnum.getKey();
            string = stringEnum.getString();
            ASSERT_EQ(key, iter.first);
            ASSERT_EQ(string, iter.second);
        }
    }
    
    TEST(fb_test, fb_stringenum_copy_test)
    {
        int resultKey;
        int requestedKey = 1;
    
        StringEnum stringEnumSrc(members);
        stringEnumSrc.setKey(requestedKey);
        StringEnum stringEnumDest(stringEnumSrc);
        resultKey = stringEnumDest.getKey();
        ASSERT_EQ(requestedKey, resultKey);
    
        // changing the source should affect the destination!
        stringEnumSrc.setKey(2);
        resultKey = stringEnumDest.getKey();
        ASSERT_EQ(requestedKey, resultKey);
    }
    
    TEST(fb_test, fb_stringenum_invalid_initial_selection_test)
    {
        ASSERT_THROW(StringEnum stringEnum(members, 99), std::runtime_error);
    }
}
