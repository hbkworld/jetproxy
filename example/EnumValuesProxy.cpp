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

#include "example/EnumValuesProxy.hpp"


namespace hbk::jetproxy
{

const std::string EnumValuesProxy::TYPE = "unittestFunctionBlockType";

const std::string EnumValuesProxy::PROP_EVEN = "even";
const std::string EnumValuesProxy::PROP_ENUM_VARIANT_VARIABLE = "multiStateValueVariable";

const EnumValues EnumValuesProxy::evenChoices = {
   { 0, { .description = "description0", .name = "name0"}},
   { 2, { .description = "description2", .name = "name2"}},
   { 100, { .description = "description100", .name = "name100"}}
};

const EnumValues EnumValuesProxy::unevenChoices = {
   { 1, { .description = "description1", .name = "name1"}},
   { 3, { .description = "description3", .name = "name3"}},
   { 101, { .description = "description101", .name = "name101"}}
};

EnumValuesProxy::EnumValuesProxy(hbk::jet::PeerAsync &jetPeer, const std::string &path)
    : JetProxy::JetProxy(jetPeer, TYPE, path, true)
    , evenProperty(false)
    , possibilitiesProperty(unevenChoices)
{
    m_state = std::make_unique<hbk::jetproxy::ProxyJetStates>(m_jetPeer, m_path, EnumValuesProxy::compose(), std::bind(&EnumValuesProxy::setFromJet, this, std::placeholders::_1));
    m_state->addToIntrospection(PROP_ENUM_VARIANT_VARIABLE, possibilitiesProperty);
    setEven(false);
}

void EnumValuesProxy::setEven(bool value) {
    EnumValues choices;

    evenProperty = value;
    if (evenProperty) {
        possibilitiesProperty.setEnumValues(evenChoices);
    } else {
        possibilitiesProperty.setEnumValues(unevenChoices);
    }
    m_state->updateIntrospection();
}

Json::Value EnumValuesProxy::composePropertiesTypes()
{
    Json::Value type;

    type[PROP_EVEN] = hbk::jetproxy::JsonSchema::createJsonSchemaType<decltype(evenProperty)>("A bool value that determines the possibilities of another property.");
    type[PROP_ENUM_VARIANT_VARIABLE] = hbk::jetproxy::JsonSchema::createEnumValuetType("Property with possibilities selected by 'even'.");

    return type;
}

void EnumValuesProxy::restoreDefaults()
{
    evenProperty = false;
    possibilitiesProperty.setEnumValues(unevenChoices);
}

void EnumValuesProxy::composeProperties(Json::Value &composition) const
{
    composition[PROP_EVEN] = evenProperty;
    composition[PROP_ENUM_VARIANT_VARIABLE] = possibilitiesProperty.get();
}

hbk::jet::SetStateCbResult EnumValuesProxy::setFromJet(const Json::Value &request)
{
    // by returning the request, we accept it as is
    Json::Value::Members members = request.getMemberNames();
    for(const std::string& member : members) {
        if (member == PROP_EVEN) {
            setEven(request[PROP_EVEN].asBool());
        } else if (member == PROP_ENUM_VARIANT_VARIABLE) {
            int64_t requestedValue = request[PROP_ENUM_VARIANT_VARIABLE].asInt64();
            if (possibilitiesProperty.set(requestedValue)==-1) {
                throw std::runtime_error("Not a valid value");
            }
        }
    }
    return request;
}

}
