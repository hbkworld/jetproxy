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

#include <chrono>
#include <vector>
#include <string>
#include <syslog.h>

#include "json/value.h"

#include "hbk/sys/eventloop.h"
#include "hbk/sys/timer.h"
#include "jetproxy/DelayedSaver.hpp"
#include "jetproxy/JetProxy.hpp"
#include "jet/defines.h"
#include "jet/peerasync.hpp"

#include "objectmodel/ObjectModelConstants.hpp"

namespace hbk::jetproxy {
    
    DelayedSaver::DelayedSaver(hbk::sys::EventLoop& eventloop, hbk::jet::PeerAsync& peer)
        : m_doSaveOnChange(false)
        , m_delay(std::chrono::milliseconds(3000))
        , m_peer(peer)
        , m_delayedSaveTimer(eventloop)
    {
    }
    
    DelayedSaver::~DelayedSaver()
    {
        stop();
    }
    
    void DelayedSaver::saveDelayedHandler(bool fired) {
        if (!fired) {
            syslog(LOG_INFO, "Saving current configuration before shutting down...");
        } else {
            syslog(LOG_INFO, "Saving current configuration...");
        }
        JetProxy::saveAllToFile(m_configFile);
    }
    
    /// @param one or more fetch conditions that acivate the delayed save mechanism.
    int DelayedSaver::start(const Matchers& matchers, const std::string &configFile)
    {
        if (matchers.empty()) {
            syslog(LOG_ERR, "Can not start delayed saver without a matcher to listen to");
            return -1;
        }
        
        if (configFile.empty()) {
            syslog(LOG_ERR, "Can not start delayed saver without a config file name");
            return -1;
        }
        
        m_configFile = configFile;
        
        auto notificationCb = [this](const Json::Value& notification, int status) {
            // Called upon change on states we have a watch on. This triggers the timer
            // for a delayed save.
            if (!m_doSaveOnChange) {
                return;
            }
            if (status < 0) {
                return;
            }
            try {
                const std::string event = notification[hbk::jet::EVENT].asString();
                if (event != hbk::jet::REMOVE) {
                    // arm delayed saving for changes on persistent elements only
                    const bool persistent = notification[hbk::jet::VALUE][objectmodel::constants::jsonPersistentMemberId].asBool();
                    if (!persistent) {
                        return;
                    }
                    auto timeoutCb = [this](bool fired) {
                      saveDelayedHandler(fired);
                    };
                    m_delayedSaveTimer.set(m_delay, false, timeoutCb);
                }
            }  catch (...) {
            }
        };
        
        auto responseCb = [this](const Json::Value&) {
            // When registering a fetch, cjet notifies all matching state before giving the the response to the registration.
            // As a result we know everything that matches before this callback is called.
            // All ADD and CHANGE events coming afterwards are to trigger the delayed save mechanism!
            m_doSaveOnChange = true;
        };
        
        for (const auto& matcher : matchers) {
            m_fetchIds.push_back(m_peer.addFetchAsync(matcher, notificationCb, responseCb));
        }
        return 0;
    }
    
    void DelayedSaver::setDelay(std::chrono::milliseconds delay)
    {
        m_delay = delay;
    }
    
    /// If there is an deleayed save in flight, we save at once by canceling the delay timer
    void DelayedSaver::stop()
    {
        for (auto fetchId : m_fetchIds) {
            m_peer.removeFetchAsync(fetchId);
        }
        m_delayedSaveTimer.cancel();
    }
}
