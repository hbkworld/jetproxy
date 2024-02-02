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
#include <unordered_map>

#include "json/value.h"

#include "jet/peerasync.hpp"

#include "AnalogVariableHandler.hpp"
#include "BaseIntrospectionHandler.hpp"
#include "EnumValueHandler.hpp"
#include "SelectionValueHandler.hpp"
#include "IntrospectionVariableHandler.hpp"
#include "NumericVariableHandler.hpp"


namespace hbk::jetproxy {

/// holds references to all properties that are relevant to create the introspection of an jet proxy object
class Introspection
{
public:
    /// node name is the key
    using EnumValueHandlers = std::unordered_map < std::string, EnumValueHandler >;
    /// node name is the key
    using SelectionValueHandlers = std::unordered_map < std::string, SelectionValueHandler >;

    /// node name is the key
    using NumericValueHandlers = std::unordered_map < std::string, SelectionValueHandler >;

    ///node name is key
    using IntrospectionVariableHandlers = std::unordered_map < std::string, std::unique_ptr<BaseIntrospectionHandler>>;

    /// introspection property name is the key
    using Details = std::unordered_map < std::string, Json::Value >;
    /// node name is the key
    using Entries = std::unordered_map < std::string, Details >;

    Introspection(hbk::jet::PeerAsync& peer, const std::string& jetProxyPath);

    ~Introspection();

    std::string getPath() const
    {
        return m_introspectionPath;
    }
    
    /// Will overwrite values set already by other calls!
    /// \param introspectionPropertyName Name of the property within the jet object (This equals the node in OPC-UA)
    void insertNodeIntrospection(const std::string& nodeName, const std::string& introspectionPropertyName, const Json::Value& introspectionDetails);
    
    size_t eraseNodeIntrospection(const std::string& nodeName, const std::string& introspectionPropertyName);

    /// set introspection property "_enumValues" for node "nodeName"
    void setEnumVariant(const std::string& nodeName, const EnumValueHandler& enumValues);

    /// set introspection property "SelectionValues" for node "nodeName"
    void setSelectionValues(const std::string& nodeName, const SelectionValueHandler &selectionValues);

    /// \returns Reference variable value depending on a selection values of selection variable it relates to
    /// "switch($<Reference variable name>, <reference properties of the reference variable>)"
    /// example:
    /// "switch($measurement, 0, %voltage_range, 1, %bridge_range")
    /// \param selectionValueNodeName Node name of the selection variable
    std::string getReferenceVariableValue(const std::string& selectionValueNodeName) const;

    /// set introspection property "Numeric Values" for node "nodeName"
    template <class T>
    void setIntrospectionVariable(const std::string& nodeName, const IntrospectionVariableHandler<T> &introspectionVariableValues)
    {
        // if this is the first time we add somthing, we want to have the jet state created
        bool first = m_enumValueHandlers.empty() && m_entries.empty() && m_selectionValueHandlers.empty();

        // existing entry will be replaced
        m_introspectionVariableHandlers[nodeName] = std::make_unique < IntrospectionVariableHandler<T> > (introspectionVariableValues);

        if (first) {
            // This is the first introspecton entry created. Now we need the jet state to present the introspection.
            m_peer.addStateAsync(m_introspectionPath, compose(), hbk::jet::responseCallback_t(), hbk::jet::stateCallback_t());
        } else {
            m_peer.notifyState(m_introspectionPath, compose());
        }
    }

    template <class T>
    void setIntrospectionVariable(const std::string& nodeName, const NumberVariableHandler<T> &introspectionVariableValues)
    {
        // if this is the first time we add somthing, we want to have the jet state created
        bool first = m_enumValueHandlers.empty() && m_entries.empty() && m_selectionValueHandlers.empty();

        // existing entry will be replaced
        m_introspectionVariableHandlers[nodeName] = std::make_unique < NumberVariableHandler<T> > (introspectionVariableValues);

        if (first) {
            // This is the first introspecton entry created. Now we need the jet state to present the introspection.
            m_peer.addStateAsync(m_introspectionPath, compose(), hbk::jet::responseCallback_t(), hbk::jet::stateCallback_t());
        } else {
            m_peer.notifyState(m_introspectionPath, compose());
        }
    }

    void setAnalogValueIntrospection(const std::string& nodeName, const AnalogVariableValue &introspectionData);

    void update() const;

private:

    /// @code
    /// {
    ///   "Node" : {
    ///     "IntrospectionProperty Name 1" : { < oject describing introspection property 1 > },
    ///     "IntrospectionProperty Name 2" : { < oject describing introspection property 2 > }
    ///   },
    ///   "someEnumVariantProperty": {
    ///     "_enumValues": [
    ///       {
    ///         "_name": "A",
    ///         "_description": "Description of A",
    ///         "_value": 100
    ///       },
    ///       {
    ///         "_name": "B",
    ///         "_description": "Description of B",
    ///         "_value": 200
    ///       }
    ///    ]
    ///   },
    ///   "anotherEnumVariantProperty":
    ///   {
    ///      ...
    ///   }
    /// }
    /// @endcode
    ///
    /// @return The complete introspection object or an empty object when there is no introspection
    Json::Value compose() const;

    hbk::jet::PeerAsync& m_peer;
    std::string m_introspectionPath;
    EnumValueHandlers m_enumValueHandlers;
    SelectionValueHandlers m_selectionValueHandlers;
    IntrospectionVariableHandlers m_introspectionVariableHandlers;
    Entries m_entries;
};

}
