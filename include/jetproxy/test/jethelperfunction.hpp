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

#include <fstream>
#include <iostream>
#include <thread>

#include <gtest/gtest.h>


#include <jet/peerasync.hpp>


static const unsigned int maxWaitTime_ms = 30;

struct StateInformation {
    StateInformation()
        : changeCount(0)
    {
    }
    Json::Value value;
    unsigned int changeCount;
};

/// jet path is the key
static std::unordered_map < std::string, StateInformation > s_states;

/// Used for holding the current values of the states and for counting change events
void fetchFbCb( const Json::Value& notification, int status )
{
    if (status<0) {
        // if jet peer is destroyed, we remove all content
        s_states.clear();
        return;
    }
    std::string event = notification[hbk::jet::EVENT].asString();
    std::string path = notification[hbk::jet::PATH].asString();
    if (event==hbk::jet::REMOVE) {
        s_states.erase(path);
    } else {
        if (event==hbk::jet::CHANGE) {
            s_states[path].changeCount++;
        }
        s_states[path].value = notification[hbk::jet::VALUE];
    }
}

void waitForPath(const std::string& path)
{
    unsigned int count = 0;
    while(s_states.find(path) == s_states.end()) {
        ++count;
        ASSERT_TRUE(count < maxWaitTime_ms) << std::string(__FUNCTION__) +": '" + path + "' wait timeout";
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void waitForStateCount(size_t stateCount)
{
    unsigned int count = 0;
    while(s_states.size() != stateCount) {
        ++count;
        ASSERT_TRUE(count < maxWaitTime_ms) << std::string(__FUNCTION__) +": wait timeout";
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void waitForChangeCount(const std::string& path, unsigned int expectedCount)
{
    unsigned int count = 0;
    const auto iter = s_states.find(path);
    ASSERT_NE(iter, s_states.end());
    while(iter->second.changeCount < expectedCount) {
        ++count;
        ASSERT_TRUE(count < maxWaitTime_ms) << std::string(__FUNCTION__) +": '" + path + "' " + std::to_string(expectedCount) + " wait timeout";
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
