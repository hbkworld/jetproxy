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

#include "BaseIntrospectionHandler.hpp"

namespace hbk::jetproxy
{

template <class T>
/// All members are optionals, those that are not set will not be composed!
struct IntrospectionVariableValue
{
    std::optional<std::uint32_t> numberInList;
    std::optional<std::string> coercionExpression;
    std::optional<std::string> validationExpression;
    std::optional<T> defaultValue;
    std::optional<UnitInformation> unit;

    /// @code
    /// {
    ///    [
    ///     "defaultValue": 0
    ///     "coercionExpression":0
    ///     "validationExpression":0
    ///     "unit"
    ///     {
    ///       ...
    ///     }
    ///    ],
    ///    [
    ///    ...
    ///    ]
    /// }
    /// @endcode
    /// @note here we would need specializations
    Json::Value compose() const
    {
        Json::Value composition;
        if (numberInList) composition[objectmodel::constants::jsonNumberInListMemberId] = numberInList.value();
        if (defaultValue) composition[objectmodel::constants::jsonDefaultValueMemberId] = defaultValue.value();
        if (coercionExpression) composition[objectmodel::constants::jsonCoercionExpressionMemberId] = coercionExpression.value();
        if (validationExpression) composition[objectmodel::constants::jsonValidationExpressionMemberId] = validationExpression.value();
        if (unit) {
            composition[objectmodel::constants::jsonEngineeringUnitsMemberId] = unit.value().compose();
        }
        return composition;
    }
};

template <class T>
class IntrospectionVariableHandler : public BaseIntrospectionHandler
{
public:
    IntrospectionVariableHandler(const IntrospectionVariableValue<T> &introspectionVariable)
        : m_introspectionVariable(introspectionVariable)
    {
    }

    IntrospectionVariableHandler(IntrospectionVariableValue<T> &&introspectionVariable);

    void setIntrospectionValues(const IntrospectionVariableValue<T> &introspectionVariable)
    {
        m_introspectionVariable = introspectionVariable;
    }

    void setIntrospectionValues(IntrospectionVariableValue<T> &&introspectionVariable)
    {
        m_introspectionVariable = std::move(introspectionVariable);
    }

    Json::Value composeIntrospection() const override
    {
        return m_introspectionVariable.compose();
    }

private:
    IntrospectionVariableValue<T> m_introspectionVariable;
};
} // namespace hbk::jetproxy
