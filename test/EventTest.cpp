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

#include "json/value.h"
#include "json/writer.h"

#include "jetproxy/Event.hpp"
#include "jet/peer.hpp"


namespace objc = objectmodel::constants;

namespace hbk::jetproxy
{
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
        if (event==hbk::jet::REMOVE) {
            s_states.erase(path);
        } else {
            if (event==hbk::jet::CHANGE) {
                s_states[path].changeCount++;
            }
            s_states[path].value = notification[hbk::jet::VALUE];
            //		std::cout << "path: " << path << ", event: " << event << " " << s_states[path].changeCount << std::endl;
        }
    }


    class DerivedEvent : public Event {
    public:
        DerivedEvent(hbk::jet::PeerAsync& peer,
                     const std::string& subSystem,
                     const std::string& type,
                     const std::string& sourceNode,
                     const std::string& sourceName,
                     const std::string& message,
                     Severity severity)
            : Event(peer, subSystem, type, sourceNode, sourceName, message, severity)
        {
        }

        void set(const std::string& message, Severity severity)
        {
            SetMessage(message);
            SetSeverity(severity);
        }
    };


    class EventTest : public ::testing::Test {

    protected:
        hbk::sys::EventLoop m_eventloop;

        hbk::jet::PeerAsync peer;
        std::thread m_workerThread;
        hbk::jet::fetchId_t m_fetchId;



        EventTest()
            : peer(m_eventloop, hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0)
        {
            m_workerThread = std::thread(std::bind(&hbk::sys::EventLoop::execute, std::ref(m_eventloop)));
        }

        virtual ~EventTest()
        {
            m_eventloop.stop();
            m_workerThread.join();
        }

        virtual void SetUp()
        {
            hbk::jet::matcher_t matchFbs;
            matchFbs.startsWith = "";
            m_fetchId = peer.addFetchAsync(matchFbs, fetchFbCb);
        }

        virtual void TearDown()
        {
            peer.removeFetchAsync(m_fetchId);
        }

        void waitForPath(const std::string& path)
        {
            unsigned int count = 0;
            while(s_states.find(path) == s_states.end()) {
                ++count;
                ASSERT_TRUE(count < maxWaitTime_ms);
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

    };


    TEST_F(EventTest, event_NoDelay)
    {
        hbk::jet::Peer syncPeer(hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);

        Event event(peer, "theSubSystem", "theType", "theSourceNode", "theSourceName", "theMessage", Event::Severity::Medium);
        event.Trigger();

        hbk::jet::matcher_t matchMethod;
        matchMethod.startsWith = "/notifications/theSubSystem/theType";
        auto result = syncPeer.get(matchMethod);
        ASSERT_EQ(result["result"].size(), 0); // notification already deleted
    }

    TEST_F(EventTest, event_create_test)
    {
        static const std::string subsystem = "theSubSystem";
        static const std::string type = "theType";
        static const std::string sourceNode = "theSourceNode";
        static const std::string sourceName = "theSourceName";
        static const std::string message = "theMessage";
        static const Event::Severity severity = Event::Severity::Medium;
        {
            Event::SetDelayedDeletion(); // Needed for test of events, otherwise the jet state is deleted befor we can check it
            hbk::jet::Peer syncPeer(hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);

            Event event(peer, subsystem, type, "theSourceNode", "theSourceName", message, severity);
            event.Trigger();

            hbk::jet::matcher_t matchMethod;
            matchMethod.startsWith = "/notifications/theSubSystem/theType";
            waitForPath(event.getPath());
            auto result = syncPeer.get(matchMethod);
            ASSERT_GT(result["result"].size(), 0);
            ASSERT_EQ(result["result"][0]["path"], "/notifications/" + subsystem + '/' + type);
            ASSERT_EQ(result["result"][0]["value"][objc::jsonTypeMemberId], type);
            ASSERT_EQ(result["result"][0]["value"]["sourceNode"], sourceNode);
            ASSERT_EQ(result["result"][0]["value"]["sourceName"], sourceName);
            ASSERT_EQ(result["result"][0]["value"]["message"], message);
            ASSERT_EQ(result["result"][0]["value"]["severity"], static_cast<int>(severity));
        }

        {
            // this variant sets message and severity later
            Event::SetDelayedDeletion(); // Needed for test of events, otherwise the jet state is deleted befor we can check it
            hbk::jet::Peer syncPeer(hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);

            DerivedEvent event(peer, subsystem, type, "theSourceNode", "theSourceName", "", Event::Severity::Low);
            event.set(message, severity);
            event.Trigger();

            hbk::jet::matcher_t matchMethod;
            matchMethod.startsWith = "/notifications/theSubSystem/theType";
            waitForPath(event.getPath());
            auto result = syncPeer.get(matchMethod);
            ASSERT_GT(result["result"].size(), 0);
            ASSERT_EQ(result["result"][0]["path"], "/notifications/" + subsystem + '/' + type);
            ASSERT_EQ(result["result"][0]["value"][objc::jsonTypeMemberId], type);
            ASSERT_EQ(result["result"][0]["value"]["sourceNode"], sourceNode);
            ASSERT_EQ(result["result"][0]["value"]["sourceName"], sourceName);
            ASSERT_EQ(result["result"][0]["value"]["message"], message);
            ASSERT_EQ(result["result"][0]["value"]["severity"], static_cast<int>(severity));
        }
    }

    TEST_F(EventTest, event_destroy_test)
    {
        Event::SetDelayedDeletion(); // Needed for test of events, otherwise the jet state is deleted befor we can check it
        hbk::jet::Peer syncPeer(hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);

        {
            Event event(peer, "theSubSystem", "theType", "theSourceNode", "theSourceName", "theMessage", Event::Severity::Medium);
            event.Trigger();

            hbk::jet::matcher_t matchMethod;
            matchMethod.startsWith = "/notifications/theSubSystem/theType";
            waitForPath(event.getPath());
            auto result = syncPeer.get(matchMethod);
            ASSERT_GT(result["result"].size(), 0);
            ASSERT_EQ(result["result"][0]["path"], "/notifications/theSubSystem/theType");
            ASSERT_EQ(result["result"][0]["value"][objc::jsonTypeMemberId], "theType");
            ASSERT_EQ(result["result"][0]["value"]["sourceNode"], "theSourceNode");
            ASSERT_EQ(result["result"][0]["value"]["sourceName"], "theSourceName");
            ASSERT_EQ(result["result"][0]["value"]["message"], "theMessage");
            ASSERT_EQ(result["result"][0]["value"]["severity"], static_cast<int>(Event::Severity::Medium));
        }

        hbk::jet::matcher_t matchMethod;
        matchMethod.startsWith = "/notifications/theSubSystem/theType";
        auto result = syncPeer.get(matchMethod);
        ASSERT_EQ(result["result"].size(), 0);
    }

    TEST_F(EventTest, event_severity_test)
    {
        Event::SetDelayedDeletion(); // Needed for test of events, otherwise the jet state is deleted befor we can check it
        hbk::jet::Peer syncPeer(hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);

        Event event(peer, "theSubSystem", "theType", "theSourceNode", "theSourceName", "theMessage", Event::Severity::High);
        event.Trigger();

        hbk::jet::matcher_t matchMethod;
        matchMethod.startsWith = "/notifications/theSubSystem/theType";
        waitForPath(event.getPath());
        auto result = syncPeer.get(matchMethod);
        ASSERT_GT(result["result"].size(), 0);
        ASSERT_EQ(result["result"][0]["value"]["severity"], static_cast<int>(Event::Severity::High));

        Event event2(peer, "theSubSystem2", "theType2", "theSourceNode", "theSourceName", "theMessage", Event::Severity::Low);
        event2.Trigger();

        matchMethod.startsWith = "/notifications/theSubSystem2/theType2";
        auto result_error = syncPeer.get(matchMethod);
        ASSERT_GT(result_error["result"].size(), 0);
        ASSERT_EQ(result_error["result"][0]["value"]["severity"], static_cast<int>(Event::Severity::Low));
    }

} // namespace
