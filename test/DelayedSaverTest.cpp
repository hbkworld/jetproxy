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

#include <fstream>
#include <thread>
#include <unistd.h>

#include <gtest/gtest.h>

#include "jet/peer.hpp"
#include "hbk/sys/eventloop.h"

#include "jetproxy/DelayedSaver.hpp"
#include "jetproxy/JetProxy.hpp"

#include "example/JetObjectProxyWithSubObjectType.hpp"


namespace hbk::jetproxy {

    static const std::string CONFIG_FILE = "delayedDaverTest.cfg";
    const char TYPE[] = "TestProxy";
    static const double NUMBER_DEFAULT_VALUE = 42.0;
    static const char PROPERTY_NUMBER[] = "number";
    static const std::string PATH_PREFIX = "/DelayedSaverTest/";
    static const char PROXY_ID[] = "aProxy";
    static const std::string PROXY_PATH = PATH_PREFIX + PROXY_ID;
    static const std::string ANOTHER_PROXY_PATH = PATH_PREFIX + PROXY_ID + "2";

    class TestProxy : public JetProxy
    {
    public:



        TestProxy(hbk::jet::PeerAsync& peer, const std::string& path, RoleLevel userLevel = RoleLevel::USER, bool persistent = true)
            : JetProxy(peer, TYPE, path, true, userLevel, persistent)
            , m_number(NUMBER_DEFAULT_VALUE)
        {
            m_state = std::make_unique<hbk::jetproxy::ProxyJetStates>(m_jetPeer, m_path, TestProxy::compose(), std::bind(&TestProxy::setFromJet, this, std::placeholders::_1));
        }

        TestProxy(const TestProxy& src) = delete;
        TestProxy(TestProxy&& src) = default;

        Json::Value compose() const override
        {
            Json::Value composition;
            composition = hbk::jetproxy::JetProxy::compose();
            composeProperties(composition);
            return composition;
        }

        void restoreDefaults() override
        {
            m_number = NUMBER_DEFAULT_VALUE;
        }

        void composeProperties(Json::Value &composition) const override
        {
            composition[PROPERTY_NUMBER] = m_number;
        }

        hbk::jet::SetStateCbResult setFromJet(const Json::Value& request) override
        {
            bool changed = false;
            const Json::Value numberNode = request[PROPERTY_NUMBER];
            if (!numberNode.isNull()) {
                m_number = numberNode.asDouble();
                changed = true;
            }
            if (!changed) {
                return Json::Value();
            }
            return composeAll();
        }

        double getNumber() const
        {
            return m_number;
        }

    private:
        double m_number;
    };


        TEST(DelayedSaverTest, error_test)
        {
            hbk::sys::EventLoop eventloop;
            hbk::jet::PeerAsync peer(eventloop, hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);

            hbk::jetproxy::DelayedSaver delayedSaver(eventloop, peer);

            // there must be at least on matcher
            hbk::jetproxy::DelayedSaver::Matchers noMatchers;

            ASSERT_EQ(delayedSaver.start(noMatchers, "fileName"), -1);

            // file name must not be empty
            hbk::jetproxy::DelayedSaver::Matchers matcher(1);
            matcher[0].startsWith = PROXY_PATH;
            ASSERT_EQ(delayedSaver.start(matcher, ""), -1);
        }


        /// Test save upon destruction. Even stopped eventloop should not hinder that!
        TEST(DelayedSaverTest, forced_save_test)
        {
            double requestedValue;
            {
                hbk::sys::EventLoop eventloop;
                hbk::jet::PeerAsync peer(eventloop, hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);
                hbk::jet::Peer callingPeer(hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);
                auto m_workerThread = std::thread(std::bind(&hbk::sys::EventLoop::execute, std::ref(eventloop)));

                TestProxy aproxy(peer, PROXY_PATH);
                double originalValue = aproxy.getNumber();

                hbk::jetproxy::DelayedSaver delayedSaver(eventloop, peer);
                hbk::jetproxy::DelayedSaver::Matchers matchers(1);
                matchers[0].startsWith = PROXY_PATH;
                delayedSaver.start(matchers, CONFIG_FILE);

                requestedValue = ++originalValue;
                {
                    Json::Value requestedValueJson;
                    requestedValueJson[PROPERTY_NUMBER] = requestedValue;
                    callingPeer.setStateValue(aproxy.getPath(), requestedValueJson);
                }

                // make another change...
                ++requestedValue;
                {
                    Json::Value requestedValueJson;
                    requestedValueJson[PROPERTY_NUMBER] = requestedValue;
                    callingPeer.setStateValue(aproxy.getPath(), requestedValueJson);
                }

                // When leaving this scope, the delayed saver gets destroyed.
                // Pending save request will be performed first.
                eventloop.stop();
                m_workerThread.join();
            }

            {
                // start all over, load what was saved before and check whether change was saved.
                hbk::sys::EventLoop eventloop;
                hbk::jet::PeerAsync peer(eventloop, hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);
                hbk::jet::Peer callingPeer(hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);
                auto m_workerThread = std::thread(std::bind(&hbk::sys::EventLoop::execute, std::ref(eventloop)));

                TestProxy aproxy(peer, PROXY_PATH);
                hbk::jetproxy::JetProxy::restoreAllFromFile(CONFIG_FILE);
                double restoredValue = aproxy.getNumber();
                ASSERT_EQ(restoredValue, requestedValue);
                eventloop.stop();
                m_workerThread.join();
            }
        }


        TEST(DelayedSaverTest, delayed_save_test)
        {
            hbk::sys::EventLoop eventloop;
            hbk::jet::PeerAsync peer(eventloop, hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);
            hbk::jet::Peer callingPeer(hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);
            auto m_workerThread = std::thread(std::bind(&hbk::sys::EventLoop::execute, std::ref(eventloop)));

            double requestedValue;
            static const std::chrono::milliseconds delay(5);
            {
                TestProxy aproxy(peer, PROXY_PATH);
                double originalValue = aproxy.getNumber();


                // remove the file, request a change wait fot the timeout and check whether file does exist.
                unlink(CONFIG_FILE.c_str());

                hbk::jetproxy::DelayedSaver delayedSaver(eventloop, peer);
                delayedSaver.setDelay(delay);

                hbk::jetproxy::DelayedSaver::Matchers matchers(1);
                matchers[0].startsWith = PROXY_PATH;
                delayedSaver.start(matchers, CONFIG_FILE);

                std::this_thread::sleep_for(delay * 2);
                {
                    // there should still be no file since there was no change
                    std::ifstream savedFile(CONFIG_FILE);
                    ASSERT_EQ(savedFile.good(), false);
                }


                requestedValue = ++originalValue;
                Json::Value requestedValueJson;
                requestedValueJson[PROPERTY_NUMBER] = requestedValue;
                callingPeer.setStateValue(aproxy.getPath(), requestedValueJson);
                // leaving this scope destoys delayedSaver, which causes ::sync to have the file on disk!
            }
            std::ifstream savedFile(CONFIG_FILE);
            ASSERT_EQ(savedFile.good(), true);
            eventloop.stop();
            m_workerThread.join();
        }
        
        TEST(DelayedSaverTest, nonpersistent_test)
        {
            hbk::sys::EventLoop eventloop;
            hbk::jet::PeerAsync peer(eventloop, hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);
            hbk::jet::Peer callingPeer(hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);
            auto m_workerThread = std::thread(std::bind(&hbk::sys::EventLoop::execute, std::ref(eventloop)));

            double requestedValue;
            {
                TestProxy nonpersistent(peer, PROXY_PATH, RoleLevel::USER, false);
                TestProxy persistent(peer, ANOTHER_PROXY_PATH, RoleLevel::USER, true);
                double originalValue = nonpersistent.getNumber();


                // Remove the file, request a change for non-persistent property. Wait fot the timeout and check whether file does exist.
                // File may not have been saved
                unlink(CONFIG_FILE.c_str());
                static const std::chrono::milliseconds delay(5);

                hbk::jetproxy::DelayedSaver delayedSaver(eventloop, peer);
                delayedSaver.setDelay(delay);

                hbk::jetproxy::DelayedSaver::Matchers matchers(1);
                matchers[0].startsWith = PROXY_PATH;
                delayedSaver.start(matchers, CONFIG_FILE);

                std::this_thread::sleep_for(delay * 2);
                {
                    // file should still be no file since there was no change
                    std::ifstream savedFile(CONFIG_FILE);
                    ASSERT_EQ(savedFile.good(), false);
                }


                requestedValue = ++originalValue;
                Json::Value requestedValueJson;
                requestedValueJson[PROPERTY_NUMBER] = requestedValue;
                callingPeer.setStateValue(nonpersistent.getPath(), requestedValueJson);

                std::this_thread::sleep_for(delay * 2);
                {
                    std::ifstream savedFile(CONFIG_FILE);
                    ASSERT_EQ(savedFile.good(), false);
                }
            }
            eventloop.stop();
            m_workerThread.join();
        }

        TEST(DelayedSaverTest, subobject_delayed_save_test)
        {
            // Construct type, request new configuration and save.
            JetProxyExample::JetObjectProxyWithSubObjectType::Properties requestedProperties;
            {
                hbk::sys::EventLoop eventloop;
                hbk::jet::PeerAsync peer(eventloop, hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);
                hbk::jet::Peer callingPeer(hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);
                auto m_workerThread = std::thread(std::bind(&hbk::sys::EventLoop::execute, std::ref(eventloop)));

                JetProxyExample::JetObjectProxyWithSubObjectType jetProxyExample(peer, PROXY_PATH);
                JetProxyExample::JetObjectProxyWithSubObjectType::Properties originalProperties = jetProxyExample.getProperties();

                // remove the file, request a change wait for the timeout and check whether file does exist.
                unlink(CONFIG_FILE.c_str());
                static const std::chrono::milliseconds delay(5);

                hbk::jetproxy::DelayedSaver delayedSaver(eventloop, peer);
                delayedSaver.setDelay(delay);

                hbk::jetproxy::DelayedSaver::Matchers matchers(1);
                matchers[0].startsWith = PROXY_PATH;
                delayedSaver.start(matchers, CONFIG_FILE);

                requestedProperties = originalProperties;
                requestedProperties.floatArrayProperty = originalProperties.floatArrayProperty;
                ++requestedProperties.intProperty;
                --requestedProperties.subObject.intProperty;
                {
                    // manipulate the root object
                    Json::Value requestedValueJson;
                    requestedValueJson[JetProxyExample::JetObjectProxyWithSubObjectType::INT_PROPERTY] = requestedProperties.intProperty;
                    callingPeer.setStateValue(jetProxyExample.getPath(), requestedValueJson);
                }
                {
                    // manipulate the sub object
                    Json::Value requestedValueJson;
                    requestedValueJson[JetProxyExample::JetObjectProxyWithSubObjectType::INT_PROPERTY] = requestedProperties.subObject.intProperty;
                    callingPeer.setStateValue(jetProxyExample.getPath() + + "/" + JetProxyExample::JetObjectProxyWithSubObjectType::SUB_OBJECT, requestedValueJson);
                }
                eventloop.stop();
                m_workerThread.join();
            }

            // Now construct the same type, restore saved configuration and compare with prior request.
            {
                hbk::sys::EventLoop eventloop;
                hbk::jet::PeerAsync peer(eventloop, hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);
                hbk::jet::Peer callingPeer(hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);
                auto m_workerThread = std::thread(std::bind(&hbk::sys::EventLoop::execute, std::ref(eventloop)));

                JetProxyExample::JetObjectProxyWithSubObjectType jetProxyExample(peer, PROXY_PATH);
                hbk::jetproxy::JetProxy::restoreAllFromFile(CONFIG_FILE);
                JetProxyExample::JetObjectProxyWithSubObjectType::Properties currentProperties = jetProxyExample.getProperties();
                ASSERT_EQ(currentProperties, requestedProperties);

                eventloop.stop();
                m_workerThread.join();

            }
            unlink(CONFIG_FILE.c_str());
        }
}
