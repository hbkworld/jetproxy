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
#include "JetStaticObjectProxyWithType.hpp"

namespace JetProxyExample
{

    const char JetStaticObjectProxyWithType::TYPE[] = "jetStaticObjectProxyWithType";

    const char JetStaticObjectProxyWithType::BOOL_PROPERTY[] = "boolProperty";
    const char JetStaticObjectProxyWithType::INT_PROPERTY[] = "intProperty";
    const char JetStaticObjectProxyWithType::FLOAT_PROPERTY[] = "floatProperty";
    const char JetStaticObjectProxyWithType::FLOAT_ARRAY_PROPERTY[] = "floatArrayProperty";


    JetStaticObjectProxyWithType::JetStaticObjectProxyWithType(hbk::jet::PeerAsync& jetPeer, const std::string path, Properties props):
        hbk::jetproxy::JetProxy(jetPeer, JetStaticObjectProxyWithType::TYPE, path, true, hbk::jetproxy::RoleLevel::ADMIN)
    {
        m_state = std::make_unique<hbk::jetproxy::ProxyJetStates>(m_jetPeer, m_path, JetStaticObjectProxyWithType::compose(), std::bind(&JetStaticObjectProxyWithType::setFromJet, this, std::placeholders::_1));
        setProperties(props);
    }

    void JetStaticObjectProxyWithType::composeProperties(Json::Value& composition) const
    {
        composition[BOOL_PROPERTY] = m_properties.boolProperty;
        composition[INT_PROPERTY] = m_properties.intProperty;
        composition[FLOAT_PROPERTY] = m_properties.floatProperty;
        composition[FLOAT_ARRAY_PROPERTY] =  hbk::jetproxy::JetProxy::composeArray(&m_properties.floatArrayProperty[0], m_properties.floatArrayProperty.size());
    }


    hbk::jet::SetStateCbResult JetStaticObjectProxyWithType::setFromJet(const Json::Value& request)
    {
        Properties properties;
        Json::Value::Members members = request.getMemberNames();
        for (const std::string& member : members)
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
                size_t i = 0;
                for (const auto& val : request[FLOAT_ARRAY_PROPERTY])
                {
                    if (i < properties.floatArrayProperty.size())
                    {
                        properties.floatArrayProperty[i] = val.asFloat();
                        ++i;
                    }
                }
            }
        }
        setProperties(properties);
        // Return Value empty because everything is okay.
        return Json::Value();
    }


    void JetStaticObjectProxyWithType::setProperties(const Properties& properties)
    {
        m_properties = properties;
        notify();
    }

    // is needed for working with Types
    Json::Value JetStaticObjectProxyWithType::composePropertiesTypes()
    {
        Json::Value type;

        type[BOOL_PROPERTY] = hbk::jetproxy::JsonSchema::createJsonSchemaType<decltype(m_properties.boolProperty)>("A bool value.");
        type[INT_PROPERTY] = hbk::jetproxy::JsonSchema::createJsonSchemaType<decltype(m_properties.intProperty)>("An int value.");
        type[FLOAT_PROPERTY] = hbk::jetproxy::JsonSchema::createJsonSchemaType<decltype(m_properties.floatProperty)>("A float value.");
        type[FLOAT_ARRAY_PROPERTY] = hbk::jetproxy::JsonSchema::createJsonArrayType<float>("A float array value.", sizeof(m_properties.floatArrayProperty) / sizeof(float));

        return type;
    }

    void JetStaticObjectProxyWithType::restoreDefaults()
    {
        m_properties = Properties();
    }
}
