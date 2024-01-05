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
#include "JetObjectProxyWithSubObjectType.hpp"

namespace JetProxyExample
{
    const char JetObjectProxyWithSubObjectType::TYPE[] = "jetObjectProxyWithSubObjectType";

    const char JetObjectProxyWithSubObjectType::BOOL_PROPERTY[] = "boolProperty";
    const char JetObjectProxyWithSubObjectType::INT_PROPERTY[] = "intProperty";
    const char JetObjectProxyWithSubObjectType::FLOAT_PROPERTY[] = "floatProperty";
    const char JetObjectProxyWithSubObjectType::FLOAT_ARRAY_PROPERTY[] = "floatArrayProperty";

    const char JetObjectProxyWithSubObjectType::SUB_OBJECT[] = "subObject";


    JetObjectProxyWithSubObjectType::JetObjectProxyWithSubObjectType(hbk::jet::PeerAsync& jetPeer, const std::string& path, bool fixed)
        : hbk::jetproxy::JetProxy(jetPeer, JetObjectProxyWithSubObjectType::TYPE, path, fixed, hbk::jetproxy::RoleLevel::ADMIN)
    {
        m_state = std::make_unique<hbk::jetproxy::ProxyJetStates>(m_jetPeer, m_path, JetObjectProxyWithSubObjectType::compose(), std::bind(&JetObjectProxyWithSubObjectType::setFromJet, this, std::placeholders::_1));
        m_subObject = std::make_unique < hbk::jetproxy::ProxyJetStates > (m_jetPeer, m_path + "/" + SUB_OBJECT , composeSubObjectProperties(), std::bind(&JetObjectProxyWithSubObjectType::setSubObjectFromJet, this, std::placeholders::_1));
    }

    JetObjectProxyWithSubObjectType::JetObjectProxyWithSubObjectType(hbk::jet::PeerAsync& jetPeer, const std::string& path)
        : JetObjectProxyWithSubObjectType(jetPeer, path, true)
    {
    }

    void JetObjectProxyWithSubObjectType::composeProperties(Json::Value& composition) const
    {
        composition[BOOL_PROPERTY] = m_properties.boolProperty;
        composition[INT_PROPERTY] = m_properties.intProperty;
        composition[FLOAT_PROPERTY] = m_properties.floatProperty;
    }


    Json::Value JetObjectProxyWithSubObjectType::composeSubObjectProperties() const
    {
        Json::Value composition;
        composition[BOOL_PROPERTY] = m_properties.subObject.boolProperty;
        composition[INT_PROPERTY] = m_properties.subObject.intProperty;
        return composition;

    }


    hbk::jet::SetStateCbResult JetObjectProxyWithSubObjectType::setFromJet(const Json::Value& request)
    {
        Properties properties = m_properties;
        Json::Value::Members members = request.getMemberNames();
        for (const std::string& member : members)
        {
            if (member == BOOL_PROPERTY)
            {
                properties.boolProperty = request[BOOL_PROPERTY].asBool();
            }
            else if (member == INT_PROPERTY)
            {
                properties.intProperty = request[INT_PROPERTY].asInt();
            }
            else if (member == FLOAT_PROPERTY)
            {
                properties.floatProperty = request[FLOAT_PROPERTY].asFloat();
            }
            else if (member == FLOAT_ARRAY_PROPERTY)
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
        }

        setProperties(properties);
        // Return Value empty because everything is okay.
        return Json::Value();
    }

    Json::Value JetObjectProxyWithSubObjectType::setSubObjectFromJet(const Json::Value& request)
    {
        Properties properties = m_properties;
        Json::Value::Members members = request.getMemberNames();
        for (const std::string& member : members) {
            if (member ==BOOL_PROPERTY) {
                properties.subObject.boolProperty = request[BOOL_PROPERTY].asBool();
            } else if (member == INT_PROPERTY) {
                properties.subObject.intProperty = request[INT_PROPERTY].asInt();
            }
        }
        setProperties(properties);
        // Return Value empty because everything is okay.
        return Json::Value();
    }


    void JetObjectProxyWithSubObjectType::setProperties(const Properties& properties)
    {
        m_properties = properties;
        // Notify changes to Jet
        notify();
        // Notify the the sub State as well...
        m_jetPeer.notifyState(m_path + "/" + SUB_OBJECT, composeSubObjectProperties());
    }

    // is needed for working with Types
    Json::Value JetObjectProxyWithSubObjectType::composePropertiesTypes()
    {
        Json::Value type;

        type[BOOL_PROPERTY] = hbk::jetproxy::JsonSchema::createJsonSchemaType<decltype(m_properties.boolProperty)>("A bool value.");
        type[INT_PROPERTY] = hbk::jetproxy::JsonSchema::createJsonSchemaType<decltype(m_properties.intProperty)>("An int value.");
        type[FLOAT_PROPERTY] = hbk::jetproxy::JsonSchema::createJsonSchemaType<decltype(m_properties.floatProperty)>("A float value.");
        type[FLOAT_ARRAY_PROPERTY] = hbk::jetproxy::JsonSchema::createJsonArrayType<float>("A float array value.", sizeof(m_properties.floatArrayProperty) / sizeof(float));

            // Struct Object creates per default an Object
        type[SUB_OBJECT] = hbk::jetproxy::JsonSchema::createJsonSchemaType<decltype(m_properties.subObject)>("A Sub Object.");
        type[SUB_OBJECT][BOOL_PROPERTY] = hbk::jetproxy::JsonSchema::createJsonSchemaType<decltype(m_properties.subObject.boolProperty)>("A Bool Property");
        type[SUB_OBJECT][INT_PROPERTY] = hbk::jetproxy::JsonSchema::createJsonSchemaType<decltype(m_properties.subObject.intProperty)>("Offset Factor");

        return type;
    }

    /// Here, root object and sub object are handeled
    Json::Value JetObjectProxyWithSubObjectType::composeAll() const
    {
        Json::Value storeValue;
        storeValue[m_path] = compose();
        storeValue[m_path + "/" + SUB_OBJECT] = composeSubObjectProperties();
        return storeValue;
    }

    /// Here, root object and sub object are handeled
    void JetObjectProxyWithSubObjectType::setAll(const Json::Value& composition)
    {
        setFromJet(composition[m_path]);
        setSubObjectFromJet(composition[m_path + "/" + SUB_OBJECT]);
        notifyAll();
    }

    void JetObjectProxyWithSubObjectType::notifyAll()
    {
        m_jetPeer.notifyState(m_path, compose());
        m_jetPeer.notifyState(m_path + "/" + SUB_OBJECT, composeSubObjectProperties());
    }
} // namespace MsBridge
