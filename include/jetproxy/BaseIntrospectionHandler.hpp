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

#include "json/value.h"

#include "objectmodel/ObjectModelConstants.hpp"

namespace hbk::jetproxy
{

// All base structurs to add as intropsection values
struct Range
{
    double minValue;
    double maxValue;

    Json::Value compose() const
    {
        Json::Value rangeJson;
        rangeJson[objectmodel::constants::jsonMinValueMemberId] = minValue;
        rangeJson[objectmodel::constants::jsonMaxValueMemberId] = maxValue;
        return rangeJson;
    }
} typedef Range;

struct UnitInformation
{
    std::string quantity;
    std::string displayName;
    std::string description;
    std::uint32_t unitId;
    std::string namespaceUri;

    Json::Value compose() const
    {
        Json::Value unitJson;
        unitJson[objectmodel::constants::jsonNameSpaceUriId] = namespaceUri;
        unitJson[objectmodel::constants::jsonUnitIdMemberId] = unitId;
        unitJson[objectmodel::constants::jsonQuantityMemberId] = quantity;

        unitJson[objectmodel::constants::displayName][objectmodel::constants::locale] = "";
        unitJson[objectmodel::constants::displayName][objectmodel::constants::text] = displayName;

        unitJson[objectmodel::constants::description][objectmodel::constants::locale] = "";
        unitJson[objectmodel::constants::description][objectmodel::constants::text] = description;

        return unitJson;
    }
} typedef UnitInformation;

class BaseIntrospectionHandler
{
public:

    virtual ~BaseIntrospectionHandler() = default;

    /// Composition depends on the introspection type
    virtual Json::Value composeIntrospection() const = 0;

protected:
    /// abstract
    BaseIntrospectionHandler() = default;

private:
};
} // namespace hbk::jetproxy
