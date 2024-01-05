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
#include <jet/defines.h>
#include <future>
#include <functional>
#include <iostream>

#include "StringEnum.hpp"
#include "JsonSchema.hpp"
#include "objectmodel/ObjectModelConstants.hpp"

namespace objModel = objectmodel::constants;

namespace hbk::jetproxy {
    class DataType
    {
    public:
        /// \todo in the future structs as well?!
        enum class SuperDataType {
            ENUM
        };

        struct DataDescription {
            std::string title;
            SuperDataType superDataType;
            std::string description;
            hbk::jetproxy::StringEnum::Members stringEnumMembers;
        };
        
        /// \return The super data type this one is derived from or an
        /// empty string when there is no super data type.
        static std::string getSuperDataTypeToString(const SuperDataType superDataType) 
        {
            switch (superDataType)
            {
            // All valid possibilities are to be handled here!
            case SuperDataType::ENUM:
                 return JsonSchema::TYPE_ENUM;
            }
            return "";
        };

        
        static Json::Value composeDataType(DataDescription description)
        {
            Json::Value jsonDescription;
            jsonDescription[JsonSchema::SCHEMA] = "http://json-schema.org/draft-07/schema#";
            jsonDescription[JsonSchema::ID] = "https://hbk-world.com/schemas/" + description.title + ".json";
            jsonDescription[objModel::jsonTitleMemberId] = description.title;
            jsonDescription[objModel::jsonDescriptionMemberId] = description.description;
            

            if (description.superDataType == SuperDataType::ENUM)
            {
                jsonDescription[objModel::jsonTypeMemberId] = JsonSchema::TYPE_ENUM;
                Json::Value jsonArray;
                for(const auto& v : description.stringEnumMembers)
                {
                    jsonArray[v.second] = v.first;
                }
                jsonDescription[objModel::jsonValueMemberId] = jsonArray;
            }
            
            return jsonDescription;
        }

        DataType() = default;
        ~DataType() = default;
    };
}
