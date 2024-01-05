/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* vim: set ts=4 et sw=4 tw=80: */
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

#pragma once

#include <map>

#include "json/value.h"

#include "BaseIntrospectionHandler.hpp"

namespace hbk::jetproxy
{

/// key and a variant
using SelectionValues = std::map<int64_t, Json::Value>;

class SelectionValueHandler : public BaseIntrospectionHandler
{
public:
    SelectionValueHandler(const SelectionValues &selectionValues);
    SelectionValueHandler(SelectionValues &&selectionValues);

    void setSelectionValues(const SelectionValues &selectionValues);
    void setSelectionValues(SelectionValues &&selectionValues);

    bool exists(int64_t key) const;
    int set(int64_t key);
    int64_t get() const;

    /// @code
    /// {
    ///    "_selectionValues": [
    ///        {
    ///            "_key": 100,
    ///            "_value": { "bla" }
    ///        },
    ///        {
    ///            "_key": 200,
    ///            "_value": { "blub" }
    ///        }
    ///    ]
    /// }
    /// @endcode
    Json::Value composeIntrospection() const override;

    /// \returns a string of the format: "<key 1>, %<value 1>, ... <key n>, %<value n>"
    std::string composeReferenceProperties() const;

private:
    void updateValue();
    SelectionValues m_selectionValues;
    int64_t m_key;
};
} // namespace hbk::jetproxy
