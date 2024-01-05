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

#include "example/SelectionValuesProxy.hpp"


namespace hbk::jetproxy
{

const std::string SelectionValuesProxy::TYPE = "unittestFunctionBlockType";

const std::string SelectionValuesProxy::PROP_EVEN = "even";
const std::string SelectionValuesProxy::PROP_SELECTION_VALUE_VARIABLE = "selectionValueVariable";

const SelectionValues SelectionValuesProxy::evenChoices = {
   { 0, "name0"},
   { 2, "name2"},
   { 100, "name100"}
};

const SelectionValues SelectionValuesProxy::unevenChoices = {
   { 1, "name1"},
   { 3, "name3"},
   { 101, "name101"}
};

SelectionValuesProxy::SelectionValuesProxy(hbk::jet::PeerAsync &jetPeer, const std::string &path)
    : JetProxy::JetProxy(jetPeer, TYPE, path, true)
    , evenProperty(false)
    , possibilitiesProperty(unevenChoices)
{
    m_state = std::make_unique<hbk::jetproxy::ProxyJetStates>(m_jetPeer, m_path, SelectionValuesProxy::compose(), std::bind(&SelectionValuesProxy::setFromJet, this, std::placeholders::_1));
    m_state->addToIntrospection(PROP_SELECTION_VALUE_VARIABLE, possibilitiesProperty);
    setEven(false);
}

void SelectionValuesProxy::setEven(bool value) {
    SelectionValues choices;

    evenProperty = value;
    if (evenProperty) {
        possibilitiesProperty.setSelectionValues(evenChoices);
    } else {
        possibilitiesProperty.setSelectionValues(unevenChoices);
    }
    m_state->updateIntrospection();
}

Json::Value SelectionValuesProxy::composePropertiesTypes()
{
    Json::Value type;

    type[PROP_EVEN] = hbk::jetproxy::JsonSchema::createJsonSchemaType<decltype(evenProperty)>("A bool value that determines the possibilities of another property.");
    type[PROP_SELECTION_VALUE_VARIABLE] = hbk::jetproxy::JsonSchema::createSelectionEntryStructureType("Property with possibilities selected by 'even'.");

    return type;
}

void SelectionValuesProxy::restoreDefaults()
{
    evenProperty = false;
    possibilitiesProperty.setSelectionValues(unevenChoices);
}

void SelectionValuesProxy::composeProperties(Json::Value &composition) const
{
    composition[PROP_EVEN] = evenProperty;
    composition[PROP_SELECTION_VALUE_VARIABLE] = possibilitiesProperty.get();
}

hbk::jet::SetStateCbResult SelectionValuesProxy::setFromJet(const Json::Value &request)
{
    // by returning the request, we accept it as is
    Json::Value::Members members = request.getMemberNames();
    for(const std::string& member : members) {
        if (member == PROP_EVEN) {
            setEven(request[PROP_EVEN].asBool());
        } else if (member == PROP_SELECTION_VALUE_VARIABLE) {
            int64_t requestedValue = request[PROP_SELECTION_VALUE_VARIABLE].asInt64();
            if (possibilitiesProperty.set(requestedValue)==-1) {
                throw std::runtime_error("Not a valid value");
            }
        }
    }
    return request;
}

}
