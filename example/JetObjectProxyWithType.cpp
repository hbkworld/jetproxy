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

#include <iostream>

#include "json/value.h"

#include "jetproxy/JsonSchema.hpp"
#include "jetproxy/StringEnum.hpp"
#include "JetObjectProxyWithType.hpp"

namespace JetProxyExample
{

    const char JetObjectProxyWithType::TYPE[] = "jetObjectProxyWithType";

    const char JetObjectProxyWithType::BOOL_PROPERTY[] = "boolProperty";
    const char JetObjectProxyWithType::INT_PROPERTY[] = "intProperty";
    const char JetObjectProxyWithType::FLOAT_PROPERTY[] = "floatProperty";
    const char JetObjectProxyWithType::FLOAT_ARRAY_PROPERTY[] = "floatArrayProperty";
    const char JetObjectProxyWithType::FLOAT_VECTOR_PROPERTY[] = "floatVectorProperty";
    const char JetObjectProxyWithType::CUSTOM_DATATYPE_PROPERTY[] = "customDataProperty";


    JetObjectProxyWithType::JetObjectProxyWithType(hbk::jet::PeerAsync& jetPeer, const std::string path, bool fixed)
        : hbk::jetproxy::JetProxy(jetPeer, JetObjectProxyWithType::TYPE, path, fixed, hbk::jetproxy::RoleLevel::ADMIN)
    {
        m_state = std::make_unique<hbk::jetproxy::ProxyJetStates>(m_jetPeer, m_path, JetObjectProxyWithType::compose(), std::bind(&JetObjectProxyWithType::setFromJet, this, std::placeholders::_1));

    }

    JetObjectProxyWithType::JetObjectProxyWithType(hbk::jet::PeerAsync& jetPeer, const std::string path)
        : JetObjectProxyWithType(jetPeer, path, true)
    {
    }

    void JetObjectProxyWithType::composeProperties(Json::Value& composition) const
    {
        composition[BOOL_PROPERTY] = m_properties.boolProperty;
        composition[INT_PROPERTY] = m_properties.intProperty;
        composition[FLOAT_PROPERTY] = m_properties.floatProperty;
        composition[CUSTOM_DATATYPE_PROPERTY] =  m_properties.enumProperty;
        composition[FLOAT_ARRAY_PROPERTY] =  hbk::jetproxy::JetProxy::composeArray(m_properties.floatArrayProperty, sizeof(m_properties.floatArrayProperty) / sizeof(float));
        composition[FLOAT_VECTOR_PROPERTY] = hbk::jetproxy::JetProxy::composeVector(m_properties.floatVectorProperty);
    }


    hbk::jet::SetStateCbResult JetObjectProxyWithType::setFromJet(const Json::Value& request)
    {
        Properties properties;
        properties.enumProperty = JetProxyExample::CustomDefinedDataType::HBK;
        Json::Value::Members members = request.getMemberNames();
        for (std::string& member : members)
        {
            if (member.compare(BOOL_PROPERTY) == 0)
            {
                properties.boolProperty = request[BOOL_PROPERTY].asBool();
            }
            else if (member.compare(INT_PROPERTY) == 0)
            {
                properties.intProperty = request[INT_PROPERTY].asInt();
            }
            else if (member.compare(FLOAT_PROPERTY) == 0)
            {
                properties.floatProperty = request[FLOAT_PROPERTY].asFloat();
            }
            else if (member.compare(FLOAT_ARRAY_PROPERTY) == 0)
            {
                int i = 0;
                for (const auto& val : request[FLOAT_ARRAY_PROPERTY])
                {
                    if (i < 32)
                    {
                        properties.floatArrayProperty[i] = val.asFloat();
                        ++i;
                    }
                }
            }
            else if (member.compare(FLOAT_VECTOR_PROPERTY) == 0)
            {
                int i = 0;
                for (const auto& val : request[FLOAT_VECTOR_PROPERTY])
                {
                    properties.floatVectorProperty.push_back(val.asFloat());
                    ++i;
                }
            }
        }

        setProperties(properties);
        // Return Value empty because everything is okay.
        return Json::Value();
    }


    void JetObjectProxyWithType::setProperties(const Properties& properties)
    {
        m_properties = properties;
        //Needed to notify the jet that the change is valid
        notify();
    }

    // is needed for working with Types
    Json::Value JetObjectProxyWithType::composePropertiesTypes()
    {
        Json::Value type;

        type[BOOL_PROPERTY] = hbk::jetproxy::JsonSchema::createJsonSchemaType<decltype(m_properties.boolProperty)>("A bool value.");
        type[INT_PROPERTY] = hbk::jetproxy::JsonSchema::createJsonSchemaType<decltype(m_properties.intProperty)>("An int value.");
        type[FLOAT_PROPERTY] = hbk::jetproxy::JsonSchema::createJsonSchemaType<decltype(m_properties.floatProperty)>("A float value.");
        type[CUSTOM_DATATYPE_PROPERTY] = hbk::jetproxy::JsonSchema::createCustomizedType(JetProxyExample::CustomDefinedDataType::MY_ENUM_DATA_TYPE, "Setting a custom data type.");
        type[FLOAT_ARRAY_PROPERTY] = hbk::jetproxy::JsonSchema::createJsonArrayType<float>("A float array value.", sizeof(m_properties.floatArrayProperty) / sizeof(float));
        type[FLOAT_VECTOR_PROPERTY] = hbk::jetproxy::JsonSchema::createJsonVectorType<float>("A float vector value.");

        return type;
    }


} // namespace MsBridge
