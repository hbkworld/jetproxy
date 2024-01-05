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

#include "jetproxy/JsonSchema.hpp"


namespace hbk::jetproxy
{
template<>
std::string JsonSchema::getTypeString<bool>(){
    return TYPE_BOOL;
}
template<>
std::string JsonSchema::getTypeString<std::uint16_t>(){
    return TYPE_UINT16;
}
template<>
std::string JsonSchema::getTypeString<std::int16_t>(){
    return TYPE_INT16;
}
template<>
std::string JsonSchema::getTypeString<std::uint32_t>(){
    return TYPE_UINT32;
}
template<>
std::string JsonSchema::getTypeString<std::int32_t>(){
    return TYPE_INT32;
}
template<>
std::string JsonSchema::getTypeString<std::uint64_t>(){
    return TYPE_UINT64;
}
template<>
std::string JsonSchema::getTypeString<std::int64_t>(){
    return TYPE_INT64;
}
template<>
std::string JsonSchema::getTypeString<float>(){
    return TYPE_FLOAT;
}
template<>
std::string JsonSchema::getTypeString<double>(){
    return TYPE_DOUBLE;
}
template<>
std::string JsonSchema::getTypeString<const char*>()
{
   return TYPE_STRING;
}
template<>
std::string JsonSchema::getTypeString<std::string>()
{
    return TYPE_STRING;
}
template<>
std::string JsonSchema::getTypeString<StringEnum>()
{
    return TYPE_STRING;
}

//Json::Value JsonSchema::createListType(std::string description, bool mandatory)
//{
//    Json::Value type;
//    type[DESCRIPTION] = description;
//    type[MANDATORY] = mandatory;
//    type[TYPE] = "list";
//    return type;
//}

Json::Value JsonSchema::createEnumValuetType(const std::string& description, bool mandatory)
{
    Json::Value type;
    type[DESCRIPTION] = description;
    type[MANDATORY] = mandatory;
    type[TYPE] = TYPE_ENUM_VALUE_TYPE;
    return type;
}

Json::Value JsonSchema::createSelectionEntryStructureType(const std::string& description, bool mandatory)
{
    Json::Value type;
    type[DESCRIPTION] = description;
    type[MANDATORY] = mandatory;
    type[TYPE] = TYPE_SELECTION_ENTRY_STRUCTURE;
    return type;
}
}
