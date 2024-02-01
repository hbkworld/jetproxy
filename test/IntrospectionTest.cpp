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

#include "jet/peer.hpp"

#include "jetproxy/AnalogVariableHandler.hpp"
#include "jetproxy/Introspection.hpp"
#include "jetproxy/IntrospectionVariableHandler.hpp"
#include "jetproxy/NumericVariableHandler.hpp"


#include "objectmodel/ObjectModelConstants.hpp"


struct StateInformation {
    StateInformation()
    {
    }
    Json::Value value;
    /// https://releases.llvm.org/16.0.0/tools/clang/tools/extra/docs/clang-tidy/checks/modernize/use-default-member-init.html
    unsigned int changeCount{0};
};



static std::unordered_map < std::string, StateInformation > s_states;
static const unsigned int maxWaitTime_ms = 30;

/// Used for holding the current values of the states and for counting change events
static void fetchFbCb( const Json::Value& notification, int status )
{
    if (status<0) {
        // if jet peer is destroyed, we remove all content
        s_states.clear();
        return;
    }
    
    const std::string event = notification[hbk::jet::EVENT].asString();
    const std::string path = notification[hbk::jet::PATH].asString();
    
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

static void waitForPath(const std::string& path, unsigned int minChangeCount)
{
    unsigned int count = 0;

    do {
        auto iter = s_states.find(path);
        if (iter != s_states.end()) {
            //  exists, now check for desired minimal change count
            if (iter->second.changeCount >= minChangeCount) {
                // all checks passed!
                return;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        ++count;
    } while(count < maxWaitTime_ms);
}


namespace hbk::jetproxy
{




class IntrospectionTest : public ::testing::Test {

protected:

    /// This jet peer represents the external client that sends requests to the service running in the device
    hbk::jet::Peer clientJetPeer;



    IntrospectionTest()
        : clientJetPeer(hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0)
    {
    }

    ~IntrospectionTest() = default;

    void SetUp() override
    {
        hbk::jet::matcher_t match;
        match.startsWith = objectmodel::constants::introspectionPath;
        clientJetPeer.addFetch(match, fetchFbCb);
    }

    void TearDown() override
    {
    }
};


TEST_F(IntrospectionTest, fb_object_type_test)
{
    static const std::string propName = "offset";
    Json::Value offsetValue;
    offsetValue = 7;

    static const std::string complexPropName = "complexPropName";
    Json::Value introspectionDetails;
    introspectionDetails[objectmodel::constants::jsonUnitIdMemberId] = 2;
    introspectionDetails["Description"] = "descr";

    static const std::string basePath = "/test/introspectionTest";
    Introspection introspection(clientJetPeer.getAsyncPeer(), basePath);
    introspection.insertNodeIntrospection("prop", propName, offsetValue);
    introspection.insertNodeIntrospection("prop", complexPropName, introspectionDetails);
    
    hbk::jet::matcher_t matcher;
    
    std::string introspectionPath;
    
    introspectionPath = objectmodel::constants::introspectionPath;
    introspectionPath.pop_back();
    introspectionPath += basePath;
    
    matcher.startsWith = introspectionPath;
    Json::Value response = clientJetPeer.get(matcher);
    Json::Value result = response[hbk::jsonrpc::RESULT];
    ASSERT_EQ(result.size(), 1);
    ASSERT_EQ(result[0][hbk::jet::PATH].asString(), introspectionPath);
    
    ASSERT_EQ(introspection.eraseNodeIntrospection("prop", propName), 1);
    ASSERT_EQ(introspection.eraseNodeIntrospection("doesn'texist", propName), 0);
}

static void checkAnalogVariableIntrospection(const Json::Value& composition, const AnalogVariableValue& analogVariable)
{
    ASSERT_EQ(composition[objectmodel::constants::jsonEngineeringUnitsMemberId]["Description"]["text"], analogVariable.engineeringUnits->description);
    ASSERT_EQ(composition[objectmodel::constants::jsonEngineeringUnitsMemberId]["DisplayName"]["text"], analogVariable.engineeringUnits->displayName);

    ASSERT_EQ(composition[objectmodel::constants::jsonEngineeringUnitsMemberId][objectmodel::constants::jsonUnitIdMemberId].asUInt(), analogVariable.engineeringUnits->unitId);
    ASSERT_EQ(composition[objectmodel::constants::jsonEngineeringUnitsMemberId][objectmodel::constants::jsonQuantityMemberId], analogVariable.engineeringUnits->quantity);
    ASSERT_EQ(composition[objectmodel::constants::jsonEngineeringUnitsMemberId][objectmodel::constants::jsonNameSpaceUriId], analogVariable.engineeringUnits->namespaceUri);

    ASSERT_EQ(composition[objectmodel::constants::jsonEURangeMemberId][objectmodel::constants::jsonMinValueMemberId].asDouble(), analogVariable.euRange->minValue);
    ASSERT_EQ(composition[objectmodel::constants::jsonEURangeMemberId][objectmodel::constants::jsonMaxValueMemberId].asDouble(), analogVariable.euRange->maxValue);
}

TEST_F(IntrospectionTest, AnalogVariableTest)
{
    AnalogVariableValue analogVariable;
    
    Range range;
    range.minValue = 100;
    range.maxValue = 300.5;
    analogVariable.euRange = range;
    
    UnitInformation unitUnformation;
    unitUnformation.unitId = 12;
    unitUnformation.description = "descr";
    unitUnformation.displayName = "a name";
    unitUnformation.namespaceUri = "ns";
    unitUnformation.quantity = "quant";
    analogVariable.engineeringUnits = unitUnformation;

    AnalogVariableValue anotherAnalogVariable;
    anotherAnalogVariable.euRange = {-10, 78};

    AnalogVariableHandler analogVariableHandler(analogVariable);
    Json::Value composition = analogVariableHandler.composeIntrospection();    
    checkAnalogVariableIntrospection(composition, analogVariable);
    
    static const std::string basePath = "/test/proxy";
    Introspection introspection(clientJetPeer.getAsyncPeer(), basePath);
    static const std::string VariableName = "theVariable";
    static const std::string OtherVariableName = "theOtherVariable";
    introspection.setAnalogValueIntrospection(VariableName, analogVariable);
    // setAnalogValueIntrospection currently causes two changes. => Could be optimized!
    waitForPath(introspection.getPath(), 2);
    auto stateInformation = s_states[introspection.getPath()];
    composition = stateInformation.value[VariableName];
    checkAnalogVariableIntrospection(composition, analogVariable);


    introspection.setAnalogValueIntrospection(OtherVariableName, anotherAnalogVariable);
    // two more changes
    waitForPath(introspection.getPath(), 4);
    stateInformation = s_states[introspection.getPath()];
    composition = stateInformation.value[OtherVariableName];

    ASSERT_EQ(composition[objectmodel::constants::jsonEURangeMemberId][objectmodel::constants::jsonMinValueMemberId].asDouble(), anotherAnalogVariable.euRange->minValue);
    ASSERT_EQ(composition[objectmodel::constants::jsonEURangeMemberId][objectmodel::constants::jsonMaxValueMemberId].asDouble(), anotherAnalogVariable.euRange->maxValue);
    // unit is not set!
    ASSERT_FALSE(composition.isMember(objectmodel::constants::jsonEngineeringUnitsMemberId));
}

TEST_F(IntrospectionTest, IntrospectionVariableTest)
{
    IntrospectionVariableValue<int32_t> introspectionVariable;
    introspectionVariable.defaultValue = 42;
    introspectionVariable.coercionExpression = "don't";

    IntrospectionVariableValue<int32_t> anotherIntrospectionVariable;
    anotherIntrospectionVariable.defaultValue = 9;

    IntrospectionVariableHandler<int32_t> introspectionVariableHandler(introspectionVariable);

    Json::Value composition = introspectionVariableHandler.composeIntrospection();
    ASSERT_EQ(composition[objectmodel::constants::jsonDefaultValueMemberId], introspectionVariable.defaultValue);
    ASSERT_EQ(composition[objectmodel::constants::jsonCoercionExpressionMemberId], introspectionVariable.coercionExpression);

    static const std::string basePath = "/test/proxy";
    Introspection introspection(clientJetPeer.getAsyncPeer(), basePath);
    introspection.setIntrospectionVariable("theVariable", introspectionVariableHandler);
}

TEST_F(IntrospectionTest, NumericVariableTest)
{
    NumericVariableValue<int32_t> introspectionVariable;
    introspectionVariable.defaultValue = 42;
    introspectionVariable.minValue = -10;
    introspectionVariable.maxValue = 10;
    introspectionVariable.suggestedValues = {1, 2, 3};

    NumericVariableValue<int32_t> anotherIntrospectionVariable;
    anotherIntrospectionVariable.defaultValue = 9;
    anotherIntrospectionVariable.minValue = -100;
    anotherIntrospectionVariable.maxValue = -10;

    NumberVariableHandler<int32_t> numberVariableHandler(introspectionVariable);
    NumberVariableHandler<int32_t> anotherNumberVariableHandler(anotherIntrospectionVariable);

    Json::Value composition = numberVariableHandler.composeIntrospection();
    ASSERT_EQ(composition[objectmodel::constants::jsonDefaultValueMemberId], introspectionVariable.defaultValue);
    ASSERT_EQ(composition[objectmodel::constants::jsonMaxValueMemberId], introspectionVariable.maxValue);
    ASSERT_EQ(composition[objectmodel::constants::jsonMinValueMemberId], introspectionVariable.minValue);

    ASSERT_EQ(composition[objectmodel::constants::jsonSuggestedValuesMemberId].size(), introspectionVariable.suggestedValues.size());
    for (unsigned int index = 0; index < introspectionVariable.suggestedValues.size(); ++index) {
        ASSERT_EQ(composition[objectmodel::constants::jsonSuggestedValuesMemberId][index], introspectionVariable.suggestedValues[index]);
    }

    static const std::string basePath = "/test/proxy";
    Introspection introspection(clientJetPeer.getAsyncPeer(), basePath);
    introspection.setIntrospectionVariable("theVariable", numberVariableHandler);
    introspection.insertNodeIntrospection("theVariable", objectmodel::constants::jsonDefaultValueMemberId, 5);
    introspection.setIntrospectionVariable("theOtherVariable", anotherNumberVariableHandler);
}
}
