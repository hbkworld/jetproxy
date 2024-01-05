/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
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
/* vim: set ts=4 et sw=4 tw=80: */

#pragma once

#include <memory>
#include <vector>

#include "json/value.h"

#include "jet/defines.h"
#include "jet/peerasync.hpp"

#include "Introspection.hpp"
#include "Method.hpp"

#include "EnumValueHandler.hpp"
#include "IntrospectionVariableHandler.hpp"
#include "NumericVariableHandler.hpp"
#include "SelectionValueHandler.hpp"

namespace hbk::jetproxy {

/// holds all jet states describing a fb proxy instance:
/// -A jet state containing all properties and connections of a fb proxy
/// -An optional state with introspection information about state properties
/// -All additional states describing the methods of this fb proxy.
class ProxyJetStates
{
public:
    /// \param path path of the object value state
    ProxyJetStates(hbk::jet::PeerAsync& peer, const std::string& path, const Json::Value& initialValue, const hbk::jet::stateCallback_t& Cb);
    virtual ~ProxyJetStates();

    ProxyJetStates(const ProxyJetStates& src) = delete;
    ProxyJetStates& operator= (const ProxyJetStates& src) = delete;
    ProxyJetStates(ProxyJetStates&& src) = default;
    ProxyJetStates& operator= (ProxyJetStates&& src) = delete;

    /// \param methodName To be appended to path of the function block value path
    void addMethod(const std::string& methodName, const hbk::jet::methodCallback_t& callback, const Method::MethodDescription& description);

    /// a method withoud descriptions adds not method type to jet, because is added to opc-ua with the companion spec
    void addMethod(const std::string& methodName, const hbk::jet::methodCallback_t& callback);

    void addToIntrospection(const std::string& nodeName, const AnalogVariableValue& value);

    /// Add a reference to a MultiStateDiscreteValueType to the introspection
    void addToIntrospection(const std::string& nodeName, const EnumValueHandler &handler);

    template<typename T>
    void addToIntrospection(const std::string& nodeName, const IntrospectionVariableHandler<T> &handler) {
        m_introspection.setIntrospectionVariable<T>(nodeName, handler);
    }

    /// \returns Reference variable value depending on a selection values of selection variable it relates to
    /// "switch($<Reference variable name>, <reference properties of the reference variable>)"
    /// example:
    /// "switch($measurement, 0, %voltage_range, 1, %bridge_range")
    /// \param nodeName Node name of the selection variable
    std::string getReferenceVariableValue(const std::string& nodeName) const;

    template<typename T>
    void addToIntrospection(const std::string& nodeName, const NumberVariableHandler<T> &handler) {
        m_introspection.setIntrospectionVariable<T>(nodeName, handler);
    }

    /// Add a reference to a openDAQ selection variable to the introspection
    void addToIntrospection(const std::string& nodeName, const SelectionValueHandler& handler);


    /// Add a reference to a MultiStateDiscreteValueType to the introspection
    void addToIntrospection(const std::string &nodeName, const std::string &propertyName, const Json::Value& introspectionDetails);


    /// All added references are taken into account
    void updateIntrospection();

private:
    hbk::jet::PeerAsync& m_jetPeer;
    std::string m_path;
    std::vector < std::unique_ptr < Method > > m_methods;
    Introspection m_introspection;
};
}
