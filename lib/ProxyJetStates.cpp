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

#include <string>

#include "json/value.h"
#include "jet/defines.h"
#include "jet/peerasync.hpp"

#include "jetproxy/ProxyJetStates.hpp"

namespace hbk::jetproxy
{
    ProxyJetStates::ProxyJetStates(hbk::jet::PeerAsync& peer, const std::string& path, const Json::Value& initialValue, const hbk::jet::stateCallback_t& Cb)
        : m_jetPeer(peer)
        , m_path(path)
        , m_introspection(peer, path)
    {
        m_jetPeer.addStateAsync(m_path, initialValue, hbk::jet::responseCallback_t(), Cb);
    }

    ProxyJetStates::~ProxyJetStates()
    {
        m_methods.clear(); // remove methods first
        m_jetPeer.removeStateAsync(m_path);
    }

    void ProxyJetStates::addMethod(const std::string& methodName, const hbk::jet::methodCallback_t& callback, const Method::MethodDescription& description)
    {
        // create the method in place
        auto mthd = std::make_unique < Method > (m_jetPeer, m_path + "/" + methodName, callback, description);
        m_methods.emplace_back(std::move(mthd));
    }

    void ProxyJetStates::addMethod(const std::string& methodName, const hbk::jet::methodCallback_t& callback)
    {
        // create the method in place
        auto mthd = std::make_unique < Method > (m_jetPeer, m_path + "/" + methodName, callback);
        m_methods.emplace_back(std::move(mthd));
    }

    void ProxyJetStates::addToIntrospection(const std::string& nodeName, const AnalogVariableValue &value) {
        m_introspection.setAnalogValueIntrospection(nodeName, value);
    }
    void ProxyJetStates::addToIntrospection(const std::string &nodeName, const EnumValueHandler &enumValueHandler) {
        m_introspection.setEnumVariant(nodeName, enumValueHandler);
    }

    void ProxyJetStates::addToIntrospection(const std::string& nodeName, const SelectionValueHandler& selectionValueHandler) {
        m_introspection.setSelectionValues(nodeName, selectionValueHandler);
    }

    void ProxyJetStates::addToIntrospection(const std::string &nodeName, const std::string &propertyName, const Json::Value& introspectionDetails) {
        m_introspection.insertNodeIntrospection(nodeName, propertyName, introspectionDetails);
    }

    void ProxyJetStates::updateIntrospection() {
        m_introspection.update();
    }

    std::string ProxyJetStates::getReferenceVariableValue(const std::string &nodeName) const
    {
        return m_introspection.getReferenceVariableValue(nodeName);
    }

}
