/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
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
/* vim: set ts=4 et sw=4 tw=80: */
#pragma once


#include <string>
#include <map>

#include "json/value.h"

#include "BaseIntrospectionHandler.hpp"

namespace hbk::jetproxy
{

struct EnumValue {
    std::string description;
    std::string name;
};
/// Value is the key. Ordered with ascending value.
using EnumValues = std::map<int64_t, EnumValue>;

class EnumValueHandler : public BaseIntrospectionHandler
{
public:

    EnumValueHandler(const EnumValues &enumValues);
    EnumValueHandler(EnumValues &&enumValues);

    void setEnumValues(const EnumValues &enumValues);
    void setEnumValues(EnumValues &&enumValues);

    bool exists(int64_t value) const;
    int set(int64_t value);
    int64_t get() const;

    /// @code
    /// {
    ///    "_enumValues": [
    ///        {
    ///            "_name": "A",
    ///            "_description": "Description of A",
    ///            "_value": 100
    ///        },
    ///        {
    ///            "_name": "B",
    ///            "_description": "Description of B",
    ///            "_value": 200
    ///        }
    ///    ]
    /// }
    /// @endcode
    Json::Value composeIntrospection() const override;

private:
    void updateValue();
    EnumValues m_enumValues;
    int64_t m_value;
};
} // namespace hbk::jetproxy
