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

#include <vector>

#include "hbk/sys/eventloop.h"
#include "hbk/sys/timer.h"
#include "jet/peerasync.hpp"

namespace hbk::jetproxy {
    /// Implements an automatic, rate limited mechanism to save all jetproxies created within this process upon any change on matching jet states.
    class DelayedSaver
    {
    public:
        using Matchers = std::vector < hbk::jet::matcher_t >;
        
        DelayedSaver(hbk::sys::EventLoop& eventloop, hbk::jet::PeerAsync& peer);
        
        /// stop() is being called
        ~DelayedSaver();
        
        /// @param one or more fetch conditions that acivate the delayed save mechanism.
        /// @param configFile name of the file to write to.
        /// @return -1 When there is no machter or configFile is empty
        int start(const Matchers &matchers, const std::string& configFile);
        
        /// @param delay The desired delay in milliseconds
        /// @warning Will not affected running delay! Change takes effect on next notification from a selected matcher.
        void setDelay(std::chrono::milliseconds delay);
        
        /// If there is an delayed save in flight, we save at once by cancelling the delay timer
        void stop();
        
    private:
        
        /// @param fired false when stopped to force immediate save, true if delay has elapsed.
        void saveDelayedHandler(bool fired);
        
        bool m_doSaveOnChange;
        std::string m_configFile;
        std::chrono::milliseconds m_delay;
        hbk::jet::PeerAsync &m_peer;
        hbk::sys::Timer m_delayedSaveTimer;
        std::vector < hbk::jet::fetchId_t > m_fetchIds;
    };
}
