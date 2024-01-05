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

#include <string>
#include <map>

#include "jet/peerasync.hpp"

#include "jetproxy/StringEnum.hpp"
#include "jetproxy/DataType.hpp"

namespace JetProxyExample
{

    // Class defining custom enums:
    //   myEnumDataType
    //   myMethodEnumDataType
    class CustomDefinedDataType
    {
    public:
        // My Enum Data Type
        enum MyEnumDataType {
            HBM,
            HBK
        };

        inline static const std::string MY_ENUM_DATA_TYPE = "myEnumDataType";

        static const hbk::jetproxy::DataType::DataDescription getMyEnumDataTypeDescription()
        {
            hbk::jetproxy::DataType::DataDescription dataDescription;
            hbk::jetproxy::StringEnum::Members myEnumDataType = {
                {HBM, "HBM"},
                {HBK, "HBK"}
            };
            dataDescription.title = MY_ENUM_DATA_TYPE;
            dataDescription.superDataType = hbk::jetproxy::DataType::SuperDataType::ENUM;
            dataDescription.description = "This is my enum data Type";
            dataDescription.stringEnumMembers = myEnumDataType;
            return dataDescription;
        };

        // My Method Enum Data Type
        enum MethodEnums {
            EN_METHOD_ENUM_VALUE_1,
            EN_METHOD_ENUM_VALUE_2,
        };

        inline static const std::string MY_METHOD_ENUM_DATA_TYPE = "myMethodEnumDataType";

        static const hbk::jetproxy::DataType::DataDescription getMyMethodsEnumDataTypeDescription()
        {
            hbk::jetproxy::DataType::DataDescription dataDescription;
            static const hbk::jetproxy::StringEnum::Members methodEnums = {
                {EN_METHOD_ENUM_VALUE_1, "EnumValue_1"},
                {EN_METHOD_ENUM_VALUE_2, "EnumValue_2"}
            };
            dataDescription.title = MY_METHOD_ENUM_DATA_TYPE;
            dataDescription.superDataType = hbk::jetproxy::DataType::SuperDataType::ENUM;
            dataDescription.description = "This is my enum data Type for Methods";
            dataDescription.stringEnumMembers = methodEnums;
            return dataDescription;
        };

        // Other Customized Enum Definitions

        CustomDefinedDataType() {};
        ~CustomDefinedDataType() = default;
    };

}
