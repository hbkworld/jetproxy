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
//
#include <iostream>

#include "json/value.h"
#include "jetproxy/JsonSchema.hpp"
#include "jetproxy/StringEnum.hpp"
#include "jetproxy/ErrorCode.hpp"
#include "JetObjectProxy.hpp"
#include "CustomDefinedDataTypes.hpp"
#include "TestError.hpp"

namespace JetProxyExample
{
    const char JetObjectProxy::BOOL_PROPERTY[] = "boolProperty";
    const char JetObjectProxy::INT_PROPERTY[] = "intProperty";
    const char JetObjectProxy::FLOAT_PROPERTY[] = "floatProperty";
    const char JetObjectProxy::FLOAT_ARRAY_PROPERTY[] = "floatArrayProperty";
    const char JetObjectProxy::FLOAT_VECTOR_PROPERTY[] = "floatVectorProperty";
    const char JetObjectProxy::METHOD[] = "callMethod";

    static hbk::jetproxy::Method::MethodDescription method1Dec = {
            .title = JetObjectProxy::METHOD,
            .description = "calls a method.",
            .args = {{.name = "enumType", .description = "a specific enum value", .type = JetProxyExample::CustomDefinedDataType::MY_METHOD_ENUM_DATA_TYPE},
                     {.name = "customerName", .description = "The user defined name of the source", .type = hbk::jetproxy::JsonSchema::TYPE_STRING}},
            .return_type = {.description = "Return 0 is valid; -1 failed", .type = hbk::jetproxy::JsonSchema::TYPE_INT32}
    };

    // Constructor
	JetObjectProxy::JetObjectProxy(hbk::jet::PeerAsync& jetPeer, const  std::string type, const std::string path):
        hbk::jetproxy::JetProxy(jetPeer, type, path, true)
	{
        // Initialize the state (which is defined by compose() and assign hbk::jet::SetStateCbResulttFromJet to be called on state changes
        m_state = std::make_unique<hbk::jetproxy::ProxyJetStates>(m_jetPeer, m_path, JetObjectProxy::compose(), std::bind(&JetObjectProxy::setFromJet, this, std::placeholders::_1));
        m_state->addMethod(METHOD, std::bind(&JetObjectProxy::method1, this, std::placeholders::_1), method1Dec);
    }

    // This is called by the JetProxy's compose() method to fill out all values (here called properties) of the oject in a json object
    void JetObjectProxy::composeProperties(Json::Value& composition) const
    {
            composition[BOOL_PROPERTY] = m_properties.boolProperty;
            // Attention:: If you create an object without a type it will be displayed at opc-ua as double -> numeric value!
            composition[INT_PROPERTY] = m_properties.intProperty;
            composition[FLOAT_PROPERTY] = m_properties.floatProperty;
            composition[FLOAT_ARRAY_PROPERTY] = hbk::jetproxy::JetProxy::composeArray(m_properties.floatArrayProperty, sizeof(m_properties.floatArrayProperty) / sizeof(float));
            composition[FLOAT_VECTOR_PROPERTY] = hbk::jetproxy::JetProxy::composeVector(m_properties.floatVectorProperty);
    }


    hbk::jet::SetStateCbResult JetObjectProxy::setFromJet(const Json::Value& request)
	{
        Properties properties = m_properties;
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
                if (properties.intProperty == 666) // Don't allow the value 666
                {
                    throw DemoError1(INT_PROPERTY);
                }
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
                    if (i < 2)
                    {
                        properties.floatArrayProperty[i] = val.asFloat();
                        ++i;
                    }
                }
            }
            else if (member.compare(FLOAT_VECTOR_PROPERTY) == 0)
            {
                for (const auto& val : request[FLOAT_VECTOR_PROPERTY])
                {
                     properties.floatVectorProperty.push_back(val.asFloat());
                }
            }
        }

        setProperties(properties);
        // Return Value empty because everything is okay.
        return Json::Value();
    }

    int JetObjectProxy::method1(const Json::Value& args)
    {
        if (args.size() < 2) {
            return -1;
        }

        std::string id = args[1].asString();
        // Because it is cammel case
        id[0] = std::tolower(id[0]);

        // TODO this must be more generic
        if ( args[0].asInt() == JetProxyExample::CustomDefinedDataType::EN_METHOD_ENUM_VALUE_1)
        {
            std::cout << "enumValue1" << std::endl;
        }
        else if ( args[0].asInt() == JetProxyExample::CustomDefinedDataType::EN_METHOD_ENUM_VALUE_2)
        {
            std::cout << "enumValue2" << std::endl;
        }
        else
        {
            return -1;
        }

        return 0;
    }

    void JetObjectProxy::setProperties(const Properties& properties)
    {
        m_properties = properties;
        notify();
    }
}
