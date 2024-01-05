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
#include "jetproxy/Method.hpp"
#include "jetproxy/TypeFactory.hpp"
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

    std::cout << "path: " << path << ", event: " << event << " " << s_states[path].changeCount << std::endl;

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
    class DynamicDummyProxy : public jetproxy::JetProxy
    {
    public:
        static const char TYPE[];

        DynamicDummyProxy(hbk::jet::PeerAsync& jetPeer, const std::string& path, bool fixed)
            : jetproxy::JetProxy(jetPeer, TYPE, path, fixed)
            , m_number(0)
        {
        }

        virtual ~DynamicDummyProxy() = default;

        virtual hbk::jet::SetStateCbResult setFromJet(const Json::Value& request) override
        {
            // by returning the request, we accept it as is
            return request;
        }

        Json::Value compose() const override
        {
            Json::Value composition;
            composeProperties(composition);
            return composition;
        }

        void composeProperties(Json::Value& composition) const override
        {
            composition["number"] = m_number;
        }

        void restoreDefaults() override
        {
            m_number = 0;
        }

        static Json::Value composePropertiesTypes()
        {
            Json::Value type;

            type["number"] = hbk::jetproxy::JsonSchema::createJsonSchemaType<decltype(m_number)>("A double value.");

            return type;
        }

    private:
        double m_number;

    };

    const char DynamicDummyProxy::TYPE[] = "dynamicDummy";



    class StaticDummyProxy : public jetproxy::JetProxy
    {
    public:
        static const char TYPE[];

        StaticDummyProxy(hbk::jet::PeerAsync& jetPeer, const std::string& path)
            : jetproxy::JetProxy(jetPeer, TYPE, path, true)
        {
        }

        virtual ~StaticDummyProxy() = default;

        virtual hbk::jet::SetStateCbResult setFromJet(const Json::Value& request) override
        {
            // by returning the request, we accept it as is
            return request;
        }

        Json::Value compose() const override
        {
            return Json::Value();
        }

        void restoreDefaults() override
        {
        }

        static Json::Value composePropertiesTypes()
        {
            return Json::Value();
        }

    };

    const char StaticDummyProxy::TYPE[] = "staticDummy";




    class ObjectTypeTest : public ::testing::Test {

    protected:

        /// This jet peer represents the external client that sends requests to the service running in the device
        hbk::jet::Peer fetchingJetPeer;
        hbk::sys::EventLoop eventloop;
        hbk::jet::PeerAsync peer;
        std::thread m_workerThread;




        ObjectTypeTest()
            : fetchingJetPeer(hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0)
            , peer(eventloop, hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0)
        {
        }

        virtual ~ObjectTypeTest() = default;

        virtual void SetUp()
        {
            hbk::jet::matcher_t match;
            match.startsWith = objectmodel::constants::absoluteTypesId;
            fetchingJetPeer.addFetch(match, fetchFbCb);
            m_workerThread = std::thread(std::bind(&hbk::sys::EventLoop::execute, std::ref(eventloop)));
        }

        virtual void TearDown()
        {
            eventloop.stop();
            m_workerThread.join();
        }
    };


    TEST_F(ObjectTypeTest, object_type_test)
    {
        using namespace std::chrono_literals;
        int result;

        {
            jetproxy::TypeFactory factory(peer);
            // erase has to fail
            result = factory.eraseObjectType("invalid");
            ASSERT_EQ(result, 0);

            ASSERT_EQ(s_states.size(), 0);

            result = factory.addObjectType< DynamicDummyProxy >(objectmodel::constants::objectTypeId);
            ASSERT_EQ(result, 0);
            // adding a product twice has to fail!
            result = factory.addObjectType< DynamicDummyProxy >(objectmodel::constants::objectTypeId);
            ASSERT_EQ(result, -1);

            result = factory.addStaticObjectType< StaticDummyProxy >(objectmodel::constants::objectTypeId);
            ASSERT_EQ(result, 0);
            // adding a product twice has to fail!
            result = factory.addStaticObjectType< StaticDummyProxy >(objectmodel::constants::objectTypeId);
            ASSERT_EQ(result, -1);

            waitForPath(objectmodel::constants::objectTypesPath + StaticDummyProxy::TYPE);
            waitForPath(objectmodel::constants::objectTypesPath + DynamicDummyProxy::TYPE);

            result = factory.eraseObjectType(StaticDummyProxy::TYPE);
            ASSERT_EQ(result, 1);

            waitForStateCount(1);
            auto iter = s_states.find(objectmodel::constants::objectTypesPath + StaticDummyProxy::TYPE);
            ASSERT_EQ(iter, s_states.end());

            factory.eraseObjectType(DynamicDummyProxy::TYPE);
            ASSERT_EQ(result, 1);

            waitForStateCount(0);

            // add the products again and leave scope which causes destruction of the factory which causes removal again.
            factory.addObjectType< DynamicDummyProxy >(objectmodel::constants::objectTypeId);
            factory.addStaticObjectType< StaticDummyProxy >(objectmodel::constants::objectTypeId);
        }
        waitForStateCount(0);
    }

    TEST_F(ObjectTypeTest, create_dynamically)
    {
        std::unique_ptr< DynamicDummyProxy > dummyObj;

        jetproxy::TypeFactory factory(peer);
        // creating ungeistered has to fail!
        dummyObj = factory.createObject < DynamicDummyProxy > (peer, "/ObjectTypeTest/dummyObj", false);
        ASSERT_EQ(dummyObj, nullptr);


        // register product
        factory.addObjectType< DynamicDummyProxy >(objectmodel::constants::objectTypeId);
        dummyObj = factory.createObject < DynamicDummyProxy > (peer, "/ObjectTypeTest/dummyObj", false);
        ASSERT_NE(dummyObj, nullptr);
    }
}
