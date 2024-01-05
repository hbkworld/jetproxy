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

#include "jet/peer.hpp"

#include "jetproxy/JetProxy.hpp"
#include "jetproxy/ProxyJetStates.hpp"

#include "objectmodel/ObjectModelConstants.hpp"

#include <iostream>
#include <fstream>

namespace objModel = objectmodel::constants;


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

//    std::cout << path << " " << event << " " << notification[hbk::jet::VALUE] << std::endl;
    if (event == hbk::jet::ADD) {
        StateInformation stateInformation;
        stateInformation.value = notification[hbk::jet::VALUE];
        s_states.insert({path, stateInformation});
    } else if (event==hbk::jet::CHANGE) {
        s_states[path].changeCount++;
        s_states[path].value = notification[hbk::jet::VALUE];
    } else if (event == hbk::jet::REMOVE) {
        s_states.erase(path);
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


namespace hbk::jetproxy
{
const char TYPE[] = "TestProxy";

static const double NUMBER_DEFAULT_VALUE = 42.0;
static const std::string pathPrefix = "/JetProxyTest/";
static const char PROPERTY_NUMBER[] = "number";
static const char PROXY_ID[] = "aProxy";
static const char METHOD_WITH_DESCRIPTION_NAME[] = "sumWithDescription";
static const char METHOD_NAME[] = "sum";
static const std::string proxyPath = pathPrefix + PROXY_ID;
static const std::string methodWithDescriptionPath = proxyPath + "/" + METHOD_WITH_DESCRIPTION_NAME;
static const std::string methodPath = proxyPath + "/" + METHOD_NAME;



class TestProxy : public JetProxy
{
public:
    Method::MethodDescription desc {
        .title = "sumOfThree",
                .description = "calculate sum of three numbers",
                .args = {
            {
                .name = "a",
                .description = "summand 1",
                .type = hbk::jetproxy::JsonSchema::TYPE_DOUBLE
            },
            {
                .name = "b",
                .description = "summand 2",
                .type = hbk::jetproxy::JsonSchema::TYPE_DOUBLE
            },
            {
                .name = "c",
                .description = "summand 3",
                .type = hbk::jetproxy::JsonSchema::TYPE_DOUBLE
            }
        },
                .return_type = {
            .description = "sum",
            .type = hbk::jetproxy::JsonSchema::TYPE_DOUBLE
        }
    };

    TestProxy(hbk::jet::PeerAsync& peer, const std::string& path, RoleLevel roleLevel = RoleLevel::USER, bool persistent = true)
        : JetProxy(peer, TYPE, path, true, roleLevel, persistent)
        , m_number(NUMBER_DEFAULT_VALUE)
    {
        m_state = std::make_unique<hbk::jetproxy::ProxyJetStates>(m_jetPeer, m_path, TestProxy::compose(), std::bind(&TestProxy::setFromJet, this, std::placeholders::_1));
        m_state->addMethod(METHOD_NAME, std::bind(&TestProxy::sumCb, this, std::placeholders::_1));
        m_state->addMethod(METHOD_WITH_DESCRIPTION_NAME, std::bind(&TestProxy::sumCb, this, std::placeholders::_1), desc);
    }

    TestProxy(const TestProxy& src) = delete;
    TestProxy(TestProxy&& src) = default;

    Json::Value sumCb(const Json::Value& parameters)
    {
        double sum = parameters["a"].asDouble() + parameters["b"].asDouble() + parameters["c"].asDouble();
        return sum;
    }

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

    void setNumber(double number)
    {
        m_number = number;
    }

    double getNumber() const
    {
        return m_number;
    }

private:

    double squareCb(const Json::Value& parameters)
    {
        double number = parameters[0].asDouble();
        return number * number;
    }
    double m_number;
};


class TestExceptionProxy : public TestProxy
{
public:
    TestExceptionProxy(hbk::jet::PeerAsync& peer, const std::string& path, RoleLevel roleLevel = RoleLevel::USER, bool persistent = true)
        : TestProxy(peer, path, roleLevel, persistent )
    {
    }

    void restoreDefaults() override
    {
        throw std::runtime_error("Could not restore default!");
    }

    void composeProperties(Json::Value &composition) const override
    {
        throw std::runtime_error("Could not compose properties!");
    }

};


class JetProxy_test : public ::testing::Test {

protected:
    /// This jet peer represents the external client that sends requests to the service running in the device
    hbk::jet::Peer clientJetPeer;

    hbk::sys::EventLoop eventloop;
    hbk::jet::PeerAsync peer;
    std::thread workerThread;

    JetProxy_test()
        : clientJetPeer(hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0)
        , peer(eventloop, hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0)
    {
    }

    virtual ~JetProxy_test() = default;

    virtual void SetUp() override
    {
        hbk::jet::matcher_t matchFbs;
        matchFbs.startsWith = pathPrefix;
        peer.addFetchAsync(matchFbs, fetchFbCb);

        workerThread = std::thread(std::bind(&hbk::sys::EventLoop::execute, std::ref(eventloop)));
    }

    virtual void TearDown() override
    {
        eventloop.stop();
        workerThread.join();
    }
};



TEST_F(JetProxy_test, create_and_destroy)
{
    {
        TestProxy testProxy(peer, proxyPath);
        ASSERT_EQ(testProxy.getPath(), proxyPath);

        // since there are two methods, 3 states will be created per instance
        waitForPath(testProxy.getPath());
        waitForPath(methodWithDescriptionPath);
        waitForPath(methodPath);

        ASSERT_EQ(s_states.size(), 3);
        // When leaving scope, the type is to disappear.
    }

    waitForStateCount(0);
}

TEST_F(JetProxy_test, double_type)
{
    TestProxy testProxy(peer, proxyPath);
    /// using the same type twice has to fail!
    ASSERT_THROW(TestProxy testProxy2(peer, proxyPath), std::runtime_error);
}

TEST_F(JetProxy_test, save_and_restore)
{
    static const std::string fileName = "save_test.sav";
    double defaultNumber;
    double expectedNumber;
    {
        // If a file does not exist, default settings are restored!
        int ret;
        TestProxy testProxy(peer, proxyPath);
        hbk::jetproxy::JetProxy::restoreAllDefaults();
        defaultNumber = testProxy.getNumber();
        expectedNumber = defaultNumber + 2;
        testProxy.setNumber(expectedNumber);
        ret = TestProxy::restoreAllFromFile("no_existe");
        ASSERT_EQ(ret,-1);
        // value should be back to default!
        ASSERT_EQ(testProxy.getNumber(), defaultNumber);
    }
    {
        std::ofstream nojson;
        nojson.open ("nojson.txt");
        nojson << "Writing this to a file.\n";
        nojson.close();
        int ret;
        ret = TestProxy::restoreAllFromFile("nojson.txt");
        ASSERT_EQ(ret,-1);
    }
    {
        std::ofstream empty;
        empty.open ("empty.txt");
        empty.close();
        int ret;
        ret = TestProxy::restoreAllFromFile("empty.txt");
        ASSERT_EQ(ret,-1);
    }

    {
        std::ofstream nulljson;
        nulljson.open ("nulljson.txt");
        nulljson << "null\n";
        nulljson.close();
        int ret;
        ret = TestProxy::restoreAllFromFile("nulljson.txt");
        ASSERT_EQ(ret,-1);
    }

    {
        TestProxy testProxy(peer, proxyPath);
        // check default values
        ASSERT_EQ(testProxy.getNumber(), NUMBER_DEFAULT_VALUE);
        // manipulate property
        expectedNumber = testProxy.getNumber() * 2;
        testProxy.setNumber(expectedNumber);
        TestProxy::saveAllToFile(fileName);
    }

    {
        double number;
        std::string displayName;
        TestProxy testProxy(peer, proxyPath);
        ASSERT_EQ(testProxy.getPath(), proxyPath);
        TestProxy::restoreAllFromFile(fileName);
        // check whether the manipulated value was restored
        number = testProxy.getNumber();
        ASSERT_NEAR(number, expectedNumber, 0.0001);
        // now restore default and check them
        TestProxy::restoreAllDefaults();
        number = testProxy.getNumber();
        ASSERT_EQ(number, NUMBER_DEFAULT_VALUE);
    }

    {
        /// Non persistent value should not be touche when restoring.
        /// Even if the saved file has a configuration!
        double number;
        std::string displayName;
        TestProxy testProxy(peer, proxyPath, RoleLevel::USER, false);
        ASSERT_EQ(testProxy.getPath(), proxyPath);
        TestProxy::restoreAllFromFile(fileName);
        // value should not be restored!
        number = testProxy.getNumber();
        ASSERT_EQ(number, NUMBER_DEFAULT_VALUE);
    }

    {
        /// Non persistent value should not be touche when restoring.
        /// Even if the saved file has a configuration!
        double number;
        std::string displayName;
        TestProxy testProxy(peer, proxyPath, RoleLevel::USER, true);
        ASSERT_EQ(testProxy.getPath(), proxyPath);
        testProxy.setPersistent(false);
        TestProxy::restoreAllFromFile(fileName);
        // value should not be restored!
        number = testProxy.getNumber();
        ASSERT_EQ(number, NUMBER_DEFAULT_VALUE);
    }

    {
        // provoke error when opening file
        ASSERT_EQ(TestProxy::saveAllToFile(""), -1);
    }

    {
        /// A jet proxy that throws an exception when restoring defaults.
        /// All exception a to be catched internally!
        TestExceptionProxy testProxy(peer, proxyPath, RoleLevel::USER, true);
        TestProxy::restoreAllDefaults();
    }
}

TEST_F(JetProxy_test, userlevels)
{
    static const std::string proxyPathUser = pathPrefix + PROXY_ID + ROLE_USER;
    static const std::string proxyPathAdmin = pathPrefix + PROXY_ID + ROLE_ADMIN;
    static const std::string proxyPathService = pathPrefix + PROXY_ID + ROLE_SUPPORT;
    static const std::string proxyPathFactory = pathPrefix + PROXY_ID + ROLE_FACTORY;
    static const std::string proxyPathDeveloper = pathPrefix + PROXY_ID + ROLE_DEVELOPER;
    TestProxy testProxyUser(peer, proxyPathUser, RoleLevel::USER);
    TestProxy testProxyAdmin(peer, proxyPathAdmin, RoleLevel::ADMIN);
    TestProxy testProxyService(peer, proxyPathService, RoleLevel::SUPPORT);
    TestProxy testProxyFactory(peer, proxyPathFactory, RoleLevel::FACTORY);
    TestProxy testProxyDeveloper(peer, proxyPathDeveloper, RoleLevel::DEVELOPER);

    waitForPath(testProxyUser.getPath());
    waitForPath(testProxyAdmin.getPath());
    waitForPath(testProxyService.getPath());
    waitForPath(testProxyFactory.getPath());
    waitForPath(testProxyDeveloper.getPath());


    ASSERT_EQ(s_states[proxyPathUser].value[objectmodel::constants::jsonRoleLevelMemberId], ROLE_USER);
    ASSERT_EQ(s_states[proxyPathAdmin].value[objectmodel::constants::jsonRoleLevelMemberId], ROLE_ADMIN);
    ASSERT_EQ(s_states[proxyPathService].value[objectmodel::constants::jsonRoleLevelMemberId], ROLE_SUPPORT);
    ASSERT_EQ(s_states[proxyPathFactory].value[objectmodel::constants::jsonRoleLevelMemberId], ROLE_FACTORY);
    ASSERT_EQ(s_states[proxyPathDeveloper].value[objectmodel::constants::jsonRoleLevelMemberId], ROLE_DEVELOPER);

    testProxyUser.setRoleLevel(RoleLevel::ADMIN);
    ASSERT_EQ(testProxyUser.getRoleLevel(), ROLE_ADMIN);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_EQ(s_states[proxyPathUser].value[objectmodel::constants::jsonRoleLevelMemberId], ROLE_ADMIN);

}

TEST_F(JetProxy_test, create_and_move)
{
    TestProxy testProxySrc(peer, proxyPath);
    TestProxy testProxyDst(std::move(testProxySrc));

    waitForPath(testProxyDst.getPath());
    waitForPath(methodWithDescriptionPath);
    waitForPath(methodPath);

    ASSERT_EQ(s_states.size(), 3);
    Json::Value instanceComposition = s_states[proxyPath].value;
    ASSERT_EQ(instanceComposition[PROPERTY_NUMBER].asDouble(), NUMBER_DEFAULT_VALUE);
    ASSERT_EQ(instanceComposition[objModel::jsonTypeMemberId].asString(), TYPE);
}

TEST_F(JetProxy_test, references)
{
    static const char REFERENCE_ID[] = "refId";
    static const char TARGET_ID_1[] = "targetId1";
    static const char TARGET_ID_2[] = "targetId2";
    static const char SOURCE_ID_1[] = "sourceId1";
    static const char SOURCE_ID_2[] = "sourceId2";

    TestProxy testProxy(peer, proxyPath);

    waitForPath(testProxy.getPath());
    waitForPath(methodWithDescriptionPath);
    waitForPath(methodPath);

    ASSERT_EQ(s_states.size(), 3);
    Json::Value instanceComposition = s_states[proxyPath].value;
    // there should be no references yet
    ASSERT_TRUE(instanceComposition[objModel::jsonReferenceByTargetMemberId].isNull());


    // add 2 references by target and check the result
    testProxy.addReferenceByTarget(REFERENCE_ID, TARGET_ID_1);
    testProxy.addReferenceByTarget(REFERENCE_ID, TARGET_ID_2);
    // add an existing target has to be ignored
    testProxy.addReferenceByTarget(REFERENCE_ID, TARGET_ID_2);

    // add 2 references by source and check the result
    testProxy.addReferenceBySource(REFERENCE_ID, SOURCE_ID_1);
    testProxy.addReferenceBySource(REFERENCE_ID, SOURCE_ID_2);
    // add an existing target has to be ignored
    testProxy.addReferenceBySource(REFERENCE_ID, SOURCE_ID_2);


    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    instanceComposition = s_states[proxyPath].value;
    ASSERT_EQ(instanceComposition[objModel::jsonReferenceByTargetMemberId][REFERENCE_ID].size(), 2);
    ASSERT_EQ(instanceComposition[objModel::jsonReferenceByTargetMemberId][REFERENCE_ID][0], TARGET_ID_1);
    ASSERT_EQ(instanceComposition[objModel::jsonReferenceByTargetMemberId][REFERENCE_ID][1], TARGET_ID_2);
    ASSERT_EQ(instanceComposition[objModel::jsonReferenceBySourceMemberId][REFERENCE_ID].size(), 2);
    ASSERT_EQ(instanceComposition[objModel::jsonReferenceBySourceMemberId][REFERENCE_ID][0], SOURCE_ID_1);
    ASSERT_EQ(instanceComposition[objModel::jsonReferenceBySourceMemberId][REFERENCE_ID][1], SOURCE_ID_2);

    // remove one and check result
    testProxy.deleteReferenceByTarget(REFERENCE_ID, TARGET_ID_1);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    instanceComposition = s_states[proxyPath].value;
    ASSERT_EQ(instanceComposition[objModel::jsonReferenceByTargetMemberId][REFERENCE_ID].size(), 1);
    ASSERT_EQ(instanceComposition[objModel::jsonReferenceByTargetMemberId][REFERENCE_ID][0], TARGET_ID_2);

    // remove one and check result
    testProxy.deleteReferenceBySource(REFERENCE_ID, SOURCE_ID_1);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    instanceComposition = s_states[proxyPath].value;
    ASSERT_EQ(instanceComposition[objModel::jsonReferenceBySourceMemberId][REFERENCE_ID].size(), 1);
    ASSERT_EQ(instanceComposition[objModel::jsonReferenceBySourceMemberId][REFERENCE_ID][0], SOURCE_ID_2);
}

TEST_F(JetProxy_test, method)
{
    TestProxy testProxy(peer, proxyPath);

    waitForPath(testProxy.getPath());
    Json::Value instanceComposition = s_states[proxyPath].value;
    // there should be no references yet
    ASSERT_TRUE(instanceComposition[objModel::jsonReferenceByTargetMemberId].isNull());

    Json::Value summands;
    summands["a"] = 1.0;
    summands["b"] = 10.0;
    summands["c"] = 100.0;
    Json::Value sum = clientJetPeer.callMethod(methodWithDescriptionPath, summands);
    ASSERT_NEAR(sum.asDouble(), 111.0, 000.1);

    sum = clientJetPeer.callMethod(methodPath, summands);
    ASSERT_NEAR(sum.asDouble(), 111.0, 000.1);
}
}
