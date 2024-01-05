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

#include "jetproxy/Method.hpp"
#include <chrono>
#include <iostream>
namespace hbk::fb
{
    class MethodTest : public ::testing::Test {
        
    protected:
        hbk::sys::EventLoop eventloop;

        hbk::jet::PeerAsync peer;
        std::thread m_workerThread;
        hbk::jet::fetchId_t m_fetchId;
        
        MethodTest()
            : peer(eventloop, hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0)
        {
            m_workerThread = std::thread(std::bind(&hbk::sys::EventLoop::execute, std::ref(eventloop)));
        }
        
        virtual ~MethodTest()
        {
            eventloop.stop();
            m_workerThread.join();
        }
        
        virtual void SetUp()
        {
        }
        
        virtual void TearDown()
        {
        }
    };
    
    
    
    TEST_F(MethodTest, fb_method_test)
    {
        using namespace std::chrono_literals;

        bool gotCalled;
        
        hbk::jet::methodCallback_t callback( [&gotCalled]( const Json::Value& parameters) 
            {
                gotCalled = true;
                double summand1 = parameters["a"].asDouble();
                int summand2 = parameters["b"].asInt();
                double summand3 = std::strtod(parameters["c"].asCString(), nullptr);
                double sum = summand1 + summand2 + summand3;
                return sum;
            } );
        jetproxy::Method::MethodDescription desc {
            .title = "sumOfThree",
            .description = "calculate sum of three numbers",
            .args = {
                {
                    .name = "a",
                    .description = "summand 1",
                    .type = jetproxy::JsonSchema::TYPE_DOUBLE
                },
                {
                    .name = "b",
                    .description = "summand 2",
                    .type = jetproxy::JsonSchema::TYPE_INT32
                },
                {
                    .name = "c",
                    .description = "summand 3",
                    .type = jetproxy::JsonSchema::TYPE_STRING
                }
            },
            .return_type = {
                .description = "sum",
                .type = jetproxy::JsonSchema::TYPE_DOUBLE
            }
        };

        std::string methodPath = "somepath";
        Json::Value v;
        double summand1 = -3.5;
        int summand2 = 3;
        double summand3 = 10.0;
        v["a"] = summand1;
        v["b"] = summand2;
        v["c"] = std::to_string(summand3);

        gotCalled = false;
        {
            // with description
            jetproxy::Method m(peer, methodPath, callback, desc);
            jetproxy::RemoteMethod rm(peer, methodPath);

            std::future<Json::Value> ret = rm(v);
            ASSERT_EQ(ret.wait_for(2s), std::future_status::ready);
            ASSERT_TRUE(ret.valid());
            Json::Value jret = ret.get();
            ASSERT_NEAR(jret["result"].asDouble(), summand1 + summand2 + summand3, 0.0001);

            ASSERT_TRUE( gotCalled );
        }

        gotCalled = false;
        {
            // without description
            jetproxy::Method m(peer, methodPath, callback);
            jetproxy::RemoteMethod rm(peer, methodPath);

            std::future<Json::Value> ret = rm(v);
            ASSERT_EQ(ret.wait_for(2s), std::future_status::ready);
            ASSERT_TRUE(ret.valid());
            Json::Value jret = ret.get();
            ASSERT_NEAR(jret["result"].asDouble(), summand1 + summand2 + summand3, 0.0001);

            ASSERT_TRUE( gotCalled );
        }
    }
}
