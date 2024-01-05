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
#include "json/value.h"

//#include "EnumValueHandler.hpp"

#include "objectmodel/ObjectModelConstants.hpp"

#include "StringEnum.hpp"

namespace objmod = objectmodel::constants;

namespace hbk::jetproxy
{
class JsonSchema
{
public:
    inline static const std::string ID = "$id";
    inline static const std::string SCHEMA = "$schema";

    inline static const std::string TITLE = objmod::jsonTitleMemberId;
    inline static const std::string TYPE = objmod::jsonTypeMemberId;
    inline static const std::string DESCRIPTION = objmod::jsonDescriptionMemberId;
    inline static const std::string MANDATORY = objmod::jsonMandatoryMemberId;
    inline static const std::string ITEMS =  objmod::jsonItemsMemberId;
    inline static const std::string MAX_ITEMS = objmod::jsonMaxItemsMemberId;
    inline static const std::string PROPERTIES = objmod::jsonPropertiesMemberId;

    inline static const std::string TYPE_ENUM = "enum";
    inline static const std::string TYPE_STRING = "string";
    inline static const std::string TYPE_BOOL = "bool";
    inline static const std::string TYPE_UINT16 = "uint16";
    inline static const std::string TYPE_INT16 = "int16";
    inline static const std::string TYPE_UINT32 = "uint32";
    inline static const std::string TYPE_INT32  = "int32";
    inline static const std::string TYPE_UINT64 = "uint64";
    inline static const std::string TYPE_INT64  = "int64";
    inline static const std::string TYPE_FLOAT = "float";
    inline static const std::string TYPE_DOUBLE = "double";
    inline static const std::string TYPE_ARRAY = "array";
    inline static const std::string TYPE_VECTOR = "vector";
    /// This type holds introspection data of a multistate discrete item type
    inline static const std::string TYPE_ENUM_VALUE_TYPE = "enumValueType";
    inline static const std::string TYPE_OBJECT = "object";
    inline static const std::string TYPE_FUNCTIONBLOCK = "functionblock";
    /// This type holds introspection data of an openDAQ selection variable type
    inline static const std::string TYPE_SELECTION_ENTRY_STRUCTURE = "selectionEntryStructure";

    /// Creates a Json type description based on the template argument
    /// The type should implement a getTypeString template, that returns the name of the type
    /// For example getTypeString<uint32_t>() returns uint32
    template<typename T>
    static Json::Value createJsonSchemaType(std::string description, bool mandatory = true)
    {
        Json::Value type;
        type[DESCRIPTION] = description;
        type[MANDATORY] = mandatory;
        type[TYPE] = getTypeString<T>();
        return type;
    }
    
    //static Json::Value createListType(std::string description, bool mandatory = true);

    static Json::Value createCustomizedType(std::string customizedTypeName, std::string description, bool mandatory = true)
    {
        Json::Value type;
        type[DESCRIPTION] = description;
        type[MANDATORY] = mandatory;
        type[TYPE] = customizedTypeName;
        return type;
    }

    template<typename T>
    static Json::Value createJsonArrayType(std::string description, std::size_t size, bool mandatory = true)
    {
        Json::Value type;
        type[DESCRIPTION] = description;
        type[MANDATORY] = mandatory;
        type[TYPE] = TYPE_ARRAY;
        type[MAX_ITEMS] = size;
        type[ITEMS] = {};
        type[ITEMS][TYPE] = getTypeString<T>();
        return type;
    }
    
    template<typename T>
    static Json::Value createJsonVectorType(std::string description, bool mandatory = true)
    {
        Json::Value type;
        type[DESCRIPTION] = description;
        type[MANDATORY] = mandatory;
        type[TYPE] = TYPE_VECTOR;
        type[ITEMS] = {};
        type[ITEMS][TYPE] = getTypeString<T>();
        return type;
    }

    static Json::Value createEnumValuetType(const std::string &description, bool mandatory = true);
    static Json::Value createSelectionEntryStructureType(const std::string& description, bool mandatory = true);

    template<typename T>
    static std::string getTypeString();
};

template<>
std::string JsonSchema::getTypeString<bool>();
template<>
std::string JsonSchema::getTypeString<std::uint16_t>();
template<>
std::string JsonSchema::getTypeString<std::int16_t>();
template<>
std::string JsonSchema::getTypeString<std::uint32_t>();
template<>
std::string JsonSchema::getTypeString<std::int32_t>();
template<>
std::string JsonSchema::getTypeString<std::uint64_t>();
template<>
std::string JsonSchema::getTypeString<std::int64_t>();
template<>
std::string JsonSchema::getTypeString<float>();
template<>
std::string JsonSchema::getTypeString<double>();
template<>
std::string JsonSchema::getTypeString<const char*>();
template<>
std::string JsonSchema::getTypeString<std::string>();
template<>
std::string JsonSchema::getTypeString<StringEnum>();
template<> 
std::string JsonSchema::getTypeString<uint32_t>();
template<typename T>
std::string JsonSchema::getTypeString()
{
    return TYPE_OBJECT;
}

}
