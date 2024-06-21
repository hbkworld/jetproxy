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

#include "hbk/jsonrpc/jsonrpc_defines.h"

#include <jet/defines.h>
#include <jet/peerasync.hpp>

#include "jetproxy/JsonSchema.hpp"
#include "jetproxy/Event.hpp"

#include "objectmodel/ObjectModelConstants.hpp"

namespace hbk::jetproxy {
    Event::Event(hbk::jet::PeerAsync& peer,
                 const std::string& subSystem,
                 const std::string& type,
                 const std::string& sourceNode,
                 const std::string& sourceName,
                 const std::string& message,
                 Event::Severity severity) :
        m_jetPeer(peer),
        m_path(objectmodel::constants::absoluteNotificationsId + '/' + subSystem + '/' + type),
        m_subSystem(subSystem),
        m_type(type),
        m_sourceNode(sourceNode),
        m_sourceName(sourceName),
        m_message(message),
        m_severity(severity),
        m_stateExists(false)
    {
        m_state[JsonSchema::TYPE] = m_type;
        m_state[objectmodel::constants::sourceNode] = m_sourceNode;
        m_state[objectmodel::constants::sourceName] = m_sourceName;
        m_state[hbk::jsonrpc::MESSAGE] = m_message;
        m_state[objectmodel::constants::severity] = static_cast<int>(m_severity);
    } // constructor

    Event::~Event()
    {
        if (m_stateExists)
        {
            m_jetPeer.removeStateAsync(m_path);
        }
    } // destructor

    void Event::SetSeverity(Severity severity)
    {
        m_severity = severity;
        m_state[objectmodel::constants::severity] = static_cast<int>(m_severity);
    } // SetSeverity

    void Event::SetMessage(const std::string& message)
    {
        m_message = message;
        m_state[hbk::jsonrpc::MESSAGE] = m_message;
    }

    void Event::composeProperties(Json::Value &composition) const {} // SetSeverity

    void Event::Trigger()
    {
        m_jetPeer.addStateAsync(m_path, m_state, hbk::jet::responseCallback_t(), hbk::jet::stateCallback_t());
        m_stateExists = true;

        if (!m_delayedDeletion)
        {
            m_jetPeer.removeStateAsync(m_path);
            m_stateExists = false;
        }
    } // Trigger

} // namespace
