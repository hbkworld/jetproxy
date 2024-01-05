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

#include <gtest/gtest.h>
#include <jet/peerasync.hpp>

#include "hbk/sys/eventloop.h"
#include "jet/peer.hpp"

#include "jetproxy/JetProxy.hpp"
#include "jetproxy/TypeFactory.hpp"
#include "jetproxy/DataType.hpp"
#include <chrono>
#include <iostream>

struct StateInformation {
    StateInformation()
        : changeCount(0)
    {
    }
    Json::Value value;
    unsigned int changeCount;
};


static const unsigned int maxWaitTime_ms = 30;

static std::unordered_map < std::string, StateInformation > s_states;

/// Used for holding the current values of the states and for counting change events
static void fetchFbCb( const Json::Value& notification, int status )
{
    if (status<0) {
        // if jet peer is destroyed, we remove all content
        s_states.clear();
        return;
    }
    
    std::string event = notification[hbk::jet::EVENT].asString();
    std::string path = notification[hbk::jet::PATH].asString();
    
    //std::cout << "path: " << path << ", event: " << event << " " << s_states[path].changeCount << std::endl;
    
    if (event == hbk::jet::REMOVE) {
        s_states.erase(path);
    } else if (event == hbk::jet::ADD) {
        s_states.insert({path, StateInformation()});
    } else {
        if (event == hbk::jet::CHANGE) {
            s_states[path].changeCount++;
        }
        s_states[path].value = notification[hbk::jet::VALUE];
    }
}

static void waitForPath(const std::string& path)
{
    unsigned int count = 0;
    while(s_states.find(path) == s_states.end()) {
        ++count;
        ASSERT_TRUE(count < maxWaitTime_ms);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

static void waitForStateCount(size_t stateCount)
{
    unsigned int count = 0;
    while(s_states.size() != stateCount) {
        ++count;
        ASSERT_TRUE(count < maxWaitTime_ms);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}


namespace hbk
{

    
    enum MyEnumDataType {
        HBM,
        HBK
    };

    inline static const std::string MY_ENUM_DATA_TYPE = "myEnumDataType";

    static const hbk::jetproxy::DataType::DataDescription getMyEnumDataTypeDescription()
    {
        hbk::jetproxy::DataType::DataDescription dataDescription;
        hbk::jetproxy::StringEnum::Members myEnumDataType = {
            {HBM, "HBM"},
            {HBK, "HBK"}
        };
        dataDescription.title = MY_ENUM_DATA_TYPE;
        dataDescription.superDataType = hbk::jetproxy::DataType::SuperDataType::ENUM;
        dataDescription.description = "This is my enum data Type";
        dataDescription.stringEnumMembers = myEnumDataType;
        return dataDescription;
    };

    class DataTypeTest : public ::testing::Test {
        
    protected:
        
        /// This jet peer represents the external client that sends requests to the service running in the device
        hbk::jet::Peer fetchingJetPeer;
        
        
        
        DataTypeTest()
            : fetchingJetPeer(hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0)
        {
        }
        
        virtual ~DataTypeTest() = default;
        
        virtual void SetUp() override
        {
            hbk::jet::matcher_t match;
            match.startsWith = objectmodel::constants::absoluteTypesId;
            fetchingJetPeer.addFetch(match, fetchFbCb);
        }
        
        virtual void TearDown() override
        {
        }
    };


    TEST_F(DataTypeTest, fb_object_type_test)
    {
        using namespace std::chrono_literals;
        int result;

        hbk::sys::EventLoop eventloop;
        hbk::jet::PeerAsync peer(eventloop, hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);
        auto m_workerThread = std::thread(std::bind(&hbk::sys::EventLoop::execute, std::ref(eventloop)));
        
        {
            jetproxy::TypeFactory factory(peer);
            result = factory.addDataType(getMyEnumDataTypeDescription());
            ASSERT_EQ(result, 0);
            // adding a DataType twice has to fail!
            result = factory.addDataType(getMyEnumDataTypeDescription());
            ASSERT_EQ(result, -1);


            {
                // adding a DataType requires a description
                jetproxy::DataType::DataDescription dataDescription;
                result = factory.addDataType(dataDescription);
                ASSERT_EQ(result, -1);
            }

            waitForPath(objectmodel::constants::dataTypesPath + "enum/" + MY_ENUM_DATA_TYPE);

            
        }
        
        waitForStateCount(0);

        eventloop.stop();
        m_workerThread.join();
    }
    

}
