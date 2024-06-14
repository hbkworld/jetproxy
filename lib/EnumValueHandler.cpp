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

#include <stdexcept>

#include "objectmodel/ObjectModelConstants.hpp"


#include "jetproxy/EnumValueHandler.hpp"

namespace hbk::jetproxy
{

EnumValueHandler::EnumValueHandler(const EnumValues &enumValues)
    : m_enumValues(enumValues)
{
    if (m_enumValues.empty()) {
        throw std::runtime_error("MultiStateValueDiscreteType may not be constructed with empty enum values!");
    }
    m_value = m_enumValues.cbegin()->first;
}

EnumValueHandler::EnumValueHandler(EnumValues &&enumValues)
    : m_enumValues(std::move(enumValues))
{
    if (m_enumValues.empty()) {
        throw std::runtime_error("MultiStateValueDiscreteType may not be constructed with empty enum values!");
    }
    m_value = m_enumValues.cbegin()->first;
}

void EnumValueHandler::setEnumValues(const EnumValues &enumValues)
{
    if (enumValues.empty()) {
        throw std::runtime_error("MultiStateValueDiscreteType may not be set with empty enum values!");
    }

    m_enumValues = enumValues;
    updateValue();
}

void EnumValueHandler::setEnumValues(EnumValues &&enumValues)
{
    if (enumValues.empty()) {
        throw std::runtime_error("MultiStateValueDiscreteType may not be set with empty enum values!");
    }

    m_enumValues.clear();
    m_enumValues.merge(enumValues);
    updateValue();
}

bool EnumValueHandler::exists(int64_t value) const
{
    return (m_enumValues.find(value)!=m_enumValues.end());
}

int EnumValueHandler::set(int64_t value)
{
    if (m_enumValues.find(value)==m_enumValues.end()) {
        // not available, can not set value!
        return -1;
    }
    m_value = value;
    return 0;
}

int64_t EnumValueHandler::get() const
{
    return m_value;
}

Json::Value EnumValueHandler::composeIntrospection() const
{
    Json::Value choices;
    for (const auto& iter : m_enumValues) {
        Json::Value member;
        member[objectmodel::constants::jsonNameMemberId] = iter.second.name;
        member[objectmodel::constants::jsonDescriptionMemberId] = iter.second.description;
        member[objectmodel::constants::jsonValueMemberId] = iter.first;
        choices[objectmodel::constants::jsonEnumValuesMemberId].append(member);
    }
    return choices;
}

void EnumValueHandler::updateValue()
{
    // see whether the current value is available in the new choices
    if (m_enumValues.find(m_value)==m_enumValues.end()) {
        // not available, use value of first choice then first
        m_value = m_enumValues.cbegin()->first;
    }
}
} // namespace hbk::jetproxy
