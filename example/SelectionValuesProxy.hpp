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


#include "json/value.h"

#include "jet/peer.hpp"

#include "jetproxy/JetProxy.hpp"

namespace hbk::jetproxy
{


class SelectionValuesProxy : public JetProxy::JetProxy
{
public:
    static const SelectionValues evenChoices;
    static const SelectionValues unevenChoices;


    static const std::string TYPE;
    static const std::string PROP_EVEN;
    static const std::string PROP_SELECTION_VALUE_VARIABLE;


    SelectionValuesProxy(hbk::jet::PeerAsync& jetPeer, const std::string& path);

    virtual ~SelectionValuesProxy() = default;

    hbk::jet::SetStateCbResult setFromJet(const Json::Value& request) override;

    /// By changig the node, a depending MultiStateDiscreteValueType is to be updated
    void setEven(bool value);

    void composeProperties(Json::Value& composition) const override;

    void restoreDefaults() override;

    static Json::Value composePropertiesTypes();

    bool evenProperty;
    SelectionValueHandler possibilitiesProperty;
};
}
