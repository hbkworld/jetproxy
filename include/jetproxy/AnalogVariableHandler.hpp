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


#include <map>
#include <optional>

#include "json/value.h"

#include "objectmodel/ObjectModelConstants.hpp"

#include "BaseIntrospectionHandler.hpp"

namespace hbk::jetproxy
{

/// All members are optionals, those that are not set will not be composed!
struct AnalogVariableValue
{
    std::optional<Range> euRange;
    std::optional<UnitInformation> engineeringUnits;

    /// @code
    /// {
    ///    range {
    ///     minValue : 
    ///     maxValue :
    ///    }
    ///    unit {
    ///     ...
    ///    }
    /// }
    /// @endcode
    /// @note here we would need specializations
    Json::Value compose() const
    {
        Json::Value composition;

        if (engineeringUnits) {
            Json::Value unitJson = engineeringUnits.value().compose();
            composition[objectmodel::constants::jsonEngineeringUnitsMemberId] = unitJson;
        }

        if (euRange) {
            Json::Value rangeJson = euRange.value().compose();
            composition[objectmodel::constants::jsonEURangeMemberId] = rangeJson;
        }
        return composition;
    }
};

class AnalogVariableHandler : public BaseIntrospectionHandler
{
public:
    AnalogVariableHandler(const AnalogVariableValue &analogVariableValue)
        : m_analogVariableValue(analogVariableValue)
    {
    }

    AnalogVariableHandler(AnalogVariableValue &&analogVariableValue);

    void setIntrospectionValues(const AnalogVariableValue &analogVariableValue)
    {
        m_analogVariableValue = analogVariableValue;
    }

    void setIntrospectionValues(AnalogVariableValue &&analogVariableValue)
    {
        m_analogVariableValue = std::move(analogVariableValue);
    }

    Json::Value composeIntrospection() const override
    {
        return m_analogVariableValue.compose();
    }

private:
    AnalogVariableValue m_analogVariableValue;
};
} // namespace hbk::jetproxy
