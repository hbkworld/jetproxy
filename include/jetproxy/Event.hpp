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
#include "jet/peerasync.hpp"
#include <jet/defines.h>
#include <future>
#include <functional>
#include <iostream>

#include "StringEnum.hpp"
#include "JsonSchema.hpp"

//TODO: Maybe uuid_t instead of string in constructor
//
namespace hbk::jetproxy {
    /// @class Event
    /// @brief Base class for all events
    class Event
    {
    public:
        enum class Severity {
            High   = 900,       ///< Unrecoverable error (e.g. internal error or hardware failure)
            Medium = 500,       ///< Recoverable error (e.g. cable break)
            Low    =   1        ///< Informational (e.g. overload, card inserted etc.)
        };


        /// Constructor
        /// @param peer The jet peer to use when signalling the event
        /// @param source The source of the event
        /// @param type The type of the event. The type MUST be known by the opc-ua bridge
        /// @param message Message for the event
        /// @param severity Severity of the event
        Event(hbk::jet::PeerAsync& peer,
                const std::string& subSystem,
                const std::string& type,
                const std::string& sourceNode,
                const std::string& sourceName,
                const std::string& message,
                Severity severity);
        ~Event();

        /// @brief Add the state to jet.
        void Trigger();

        std::string getPath() const
        {
            return m_path;
        }

        /// Delay jet state deletion until destruction of event. This is for use in tests
        static void SetDelayedDeletion() {m_delayedDeletion = true;}
    protected:
        void SetSeverity(Severity severity);
        void SetMessage(const std::string& message);
        virtual void composeProperties(Json::Value& composition) const;

    private:
        hbk::jet::PeerAsync& m_jetPeer;
    protected:
        Json::Value m_state;
        std::string m_path;
        std::string m_subSystem;
        std::string m_type;
        std::string m_sourceNode;
        std::string m_sourceName;
        std::string m_message;
        Severity m_severity;
    private:
        bool m_stateExists;
        std::string m_typePath;
        static inline bool m_delayedDeletion = false; // If true, then state will be deleted at event destruction (for use in test code)
    }; // class Event

} 
