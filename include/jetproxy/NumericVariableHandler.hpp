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
#include <optional>

#include "json/value.h"

#include "objectmodel/ObjectModelConstants.hpp"

#include "IntrospectionVariableHandler.hpp"

namespace hbk::jetproxy
{

/// Numeric Variable Items
/// All members are optionals, those that are not set will not be composed!
template <class T>
struct NumericVariableValue : IntrospectionVariableValue<T>
{
    std::optional<T> minValue;
    std::optional<T> maxValue;

    Json::Value compose() const
    {
        Json::Value composition = IntrospectionVariableValue<T>::compose();
        if(minValue) {
            composition[objectmodel::constants::jsonMinValueMemberId] = minValue.value();
        }
        if(maxValue) {
            composition[objectmodel::constants::jsonMaxValueMemberId] = maxValue.value();
        }
        return composition;
    }
};

template <class T>
class NumberVariableHandler : public BaseIntrospectionHandler
{
public:
    NumberVariableHandler(const NumericVariableValue<T> &numericVariableValue)
        : m_numericVariableValue(numericVariableValue)
    {
    }

    NumberVariableHandler(NumericVariableValue<T> &&numericVariableValue);

    void setIntrospectionValues(const NumericVariableValue<T> &numericVariableValue)
    {
        m_numericVariableValue = numericVariableValue;
    }

    void setIntrospectionValues(NumericVariableValue<T> &&numericVariableValue)
    {
        m_numericVariableValue = std::move(numericVariableValue);
    }

    Json::Value composeIntrospection() const override
    {
        return m_numericVariableValue.compose();
    }

private:
    NumericVariableValue<T> m_numericVariableValue;
};
} // namespace hbk::jetproxy
