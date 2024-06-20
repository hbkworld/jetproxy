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

#include <syslog.h>

#include "jetproxy/Introspection.hpp"

namespace hbk::jetproxy
{

Introspection::Introspection(hbk::jet::PeerAsync &peer, const std::string &jetProxyPath)
    : m_peer(peer)
{
    m_introspectionPath = objectmodel::constants::introspectionPath;
    /// \warning jetProxyPath may start with '/' => remove it from prefix
    if (jetProxyPath.front() == '/') {
        m_introspectionPath.pop_back();
    }
    m_introspectionPath += jetProxyPath;
}

Introspection::~Introspection()
{
    if (!m_enumValueHandlers.empty() || !m_entries.empty() || !m_selectionValueHandlers.empty() || !m_introspectionVariableHandlers.empty()) {
        m_peer.removeStateAsync(m_introspectionPath);
    }
}

void Introspection::insertNodeIntrospection(const std::string& nodeName, const std::string& introspectionPropertyName, const Json::Value& introspectionDetails)
{
    // if this is the first time we add somthing, we want to have the jet state created
    bool first = m_enumValueHandlers.empty() && m_entries.empty() && m_selectionValueHandlers.empty() && m_introspectionVariableHandlers.empty();

    m_entries[nodeName][introspectionPropertyName] = introspectionDetails;
    if (first) {
        // This is the first introspecton entry created. Now we need the jet state to present the introspection.
        m_peer.addStateAsync(m_introspectionPath, compose(), hbk::jet::responseCallback_t(), hbk::jet::stateCallback_t());
    } else {
        m_peer.notifyState(m_introspectionPath, compose());
    }
}

size_t Introspection::eraseNodeIntrospection(const std::string& nodeName, const std::string& introspectionPropertyName)
{
    auto iter = m_entries.find(nodeName);
    if (iter == m_entries.end()) {
        return 0;
    }
    return iter->second.erase(introspectionPropertyName);
}

void Introspection::setEnumVariant(const std::string &nodeName, const EnumValueHandler &enumValues)
{
    // if this is the first time we add somthing, we want to have the jet state created
    bool first = m_enumValueHandlers.empty() && m_entries.empty() && m_selectionValueHandlers.empty();

    // existing entry will be replaced
    m_enumValueHandlers.erase(nodeName);
    m_enumValueHandlers.emplace( std::pair < std::string, EnumValueHandler > (nodeName, enumValues));
    if (first) {
        // This is the first introspecton entry created. Now we need the jet state to present the introspection.
        m_peer.addStateAsync(m_introspectionPath, compose(), hbk::jet::responseCallback_t(), hbk::jet::stateCallback_t());
    } else {
        m_peer.notifyState(m_introspectionPath, compose());
    }
}

void Introspection::setSelectionValues(const std::string &nodeName, const SelectionValueHandler &selectionValues)
{
    // if this is the first time we add somthing, we want to have the jet state created
    bool first = m_enumValueHandlers.empty() && m_entries.empty() && m_selectionValueHandlers.empty();

    // existing entry will be replaced
    m_selectionValueHandlers.erase(nodeName);
    m_selectionValueHandlers.emplace( std::pair < std::string, SelectionValueHandler > (nodeName, selectionValues));
    if (first) {
        // This is the first introspecton entry created. Now we need the jet state to present the introspection.
        m_peer.addStateAsync(m_introspectionPath, compose(), hbk::jet::responseCallback_t(), hbk::jet::stateCallback_t());
    } else {
        m_peer.notifyState(m_introspectionPath, compose());
    }
}

std::string Introspection::getReferenceVariableValue(const std::string &selectionValueNodeName) const
{
    const auto iter = m_selectionValueHandlers.find(selectionValueNodeName);
    if (iter==m_selectionValueHandlers.end()) {
        return "";
    }

    std::string value;
    value = "switch($" + selectionValueNodeName + ", " + iter->second.composeReferenceProperties() + ")";
    return value;
}

void Introspection::setAnalogValueIntrospection(const std::string& nodeName, const AnalogVariableValue &introspectionData)
{
    if (introspectionData.engineeringUnits) {
        insertNodeIntrospection(nodeName, objectmodel::constants::jsonEngineeringUnitsMemberId, introspectionData.engineeringUnits.value().compose());
    } else {
        eraseNodeIntrospection(nodeName, objectmodel::constants::jsonEngineeringUnitsMemberId);
    }
    if (introspectionData.euRange) {
        insertNodeIntrospection(nodeName, objectmodel::constants::jsonEURangeMemberId, introspectionData.euRange.value().compose());
    } else {
        eraseNodeIntrospection(nodeName, objectmodel::constants::jsonEURangeMemberId);
    }
}


void Introspection::update() const
{
    m_peer.notifyState(m_introspectionPath, compose());
}

Json::Value Introspection::compose() const
{
    Json::Value composition;
    for (const auto& iter : m_enumValueHandlers) {
        const std::string &nodeName = iter.first;
        composition[nodeName] = iter.second.composeIntrospection();
    }

    for (const auto& iter : m_selectionValueHandlers) {
        const std::string &nodeName = iter.first;
        composition[nodeName] = iter.second.composeIntrospection();
    }

    for (const auto& iter : m_introspectionVariableHandlers) {
        const std::string &nodeName = iter.first;
        composition[nodeName] = iter.second->composeIntrospection();
    }

    // explicitely values overwrite values set by other calls
    for (const auto& iter : m_entries) {
        for (const auto& typeIter : iter.second) {
            const std::string &nodeName = iter.first;
            const std::string &introspectionPropertyName = typeIter.first;
            const Json::Value &details = typeIter.second;
            composition[nodeName][introspectionPropertyName] = details;
        }
    }

    return composition;
}

} // namespace
