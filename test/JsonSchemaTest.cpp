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

#include <iostream>

#include <gtest/gtest.h>

#include "json/value.h"
#include "json/writer.h"

#include "jetproxy/JsonSchema.hpp"
#include "jetproxy/StringEnum.hpp"

namespace hbk::jetproxy
{

    TEST(fb_test, jsonchema_schematype_test)
    {
        Json::Value result;
        result = JsonSchema::createJsonSchemaType<const char*>(JsonSchema::TYPE_STRING);
        ASSERT_EQ(result[JsonSchema::TYPE], result[JsonSchema::DESCRIPTION]);
        result = JsonSchema::createJsonSchemaType<std::string>(JsonSchema::TYPE_STRING);
        ASSERT_EQ(result[JsonSchema::TYPE], result[JsonSchema::DESCRIPTION]);
        result = JsonSchema::createJsonSchemaType<StringEnum>(JsonSchema::TYPE_STRING);
        ASSERT_EQ(result[JsonSchema::TYPE], result[JsonSchema::DESCRIPTION]);
        result = JsonSchema::createJsonSchemaType<bool>(JsonSchema::TYPE_BOOL);
        ASSERT_EQ(result[JsonSchema::TYPE], result[JsonSchema::DESCRIPTION]);
        result = JsonSchema::createJsonSchemaType<uint16_t>(JsonSchema::TYPE_UINT16);
        ASSERT_EQ(result[JsonSchema::TYPE], result[JsonSchema::DESCRIPTION]);
        result = JsonSchema::createJsonSchemaType<int16_t>(JsonSchema::TYPE_INT16);
        ASSERT_EQ(result[JsonSchema::TYPE], result[JsonSchema::DESCRIPTION]);
        result = JsonSchema::createJsonSchemaType<uint32_t>(JsonSchema::TYPE_UINT32);
        ASSERT_EQ(result[JsonSchema::TYPE], result[JsonSchema::DESCRIPTION]);
        result = JsonSchema::createJsonSchemaType<int32_t>(JsonSchema::TYPE_INT32);
        ASSERT_EQ(result[JsonSchema::TYPE], result[JsonSchema::DESCRIPTION]);
        result = JsonSchema::createJsonSchemaType<uint64_t>(JsonSchema::TYPE_UINT64);
        ASSERT_EQ(result[JsonSchema::TYPE], result[JsonSchema::DESCRIPTION]);
        result = JsonSchema::createJsonSchemaType<int64_t>(JsonSchema::TYPE_INT64);
        ASSERT_EQ(result[JsonSchema::TYPE], result[JsonSchema::DESCRIPTION]);
        result = JsonSchema::createJsonSchemaType<float>(JsonSchema::TYPE_FLOAT);
        ASSERT_EQ(result[JsonSchema::TYPE], result[JsonSchema::DESCRIPTION]);
        result = JsonSchema::createJsonSchemaType<double>(JsonSchema::TYPE_DOUBLE);
        ASSERT_EQ(result[JsonSchema::TYPE], result[JsonSchema::DESCRIPTION]);
        
        static const size_t size = 8;
        result = JsonSchema::createJsonArrayType< int32_t >(JsonSchema::TYPE_ARRAY, size);
        ASSERT_EQ(result[JsonSchema::TYPE], result[JsonSchema::DESCRIPTION]);
        ASSERT_EQ(result[JsonSchema::MAX_ITEMS], size);
        ASSERT_EQ(result[JsonSchema::ITEMS][JsonSchema::TYPE], JsonSchema::TYPE_INT32);
    }
}
