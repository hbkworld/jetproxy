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

#include "jetproxy/Introspection.hpp"
#include "jetproxy/JetProxy.hpp"
#include "jetproxy/JsonSchema.hpp"

#include "example/EnumValuesProxy.hpp"

#include "objectmodel/ObjectModelConstants.hpp"


struct StateInformation {
    StateInformation()
        : changeCount(0)
    {
    }
    Json::Value value;
    unsigned int changeCount;
};



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


namespace hbk::jetproxy
{




class EnumValuesTest : public ::testing::Test {

protected:

    /// This jet peer represents the external client that sends requests to the service running in the device
    hbk::jet::Peer clientJetPeer;



    EnumValuesTest()
        : clientJetPeer(hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0)
    {
    }

    virtual ~EnumValuesTest() = default;

    virtual void SetUp() override
    {
        hbk::jet::matcher_t match;
        match.startsWith = objectmodel::constants::introspectionPath;
        clientJetPeer.addFetch(match, fetchFbCb);
    }

    virtual void TearDown() override
    {
    }
};



TEST_F(EnumValuesTest, fb_object_type_test)
{
    static const EnumValues choices = {
        { 1, { .description = "description1", .name = "name1"}},
        { 20, { .description = "description20", .name = "name20"}},
        { 30, { .description = "description30", .name = "name30"}}
    };

    static const EnumValues moreChoices = {
        { 0, { .description = "description1", .name = "name1"}},
        { 20, { .description = "description20", .name = "name20"}},
        { 100, { .description = "description100", .name = "name100"}}
    };

    static const EnumValues otherChoices = {
        { 1, { .description = "description1", .name = "name1"}},
        { 20, { .description = "description20", .name = "name20"}},
    };

    static const EnumValues emptychoices;


    std::string basePath = "/test/proxy";
    Introspection introspection(clientJetPeer.getAsyncPeer(), basePath);


    // construction without any choice is forbidden!
    EXPECT_THROW(EnumValueHandler multiStateDiscreteValueType(emptychoices), std::runtime_error);

    EnumValueHandler multiStateDiscreteValueType(choices);

    for (const auto &iter : choices) {
        ASSERT_EQ(multiStateDiscreteValueType.set(iter.first), 0);
        ASSERT_EQ(multiStateDiscreteValueType.exists(iter.first), true);
        ASSERT_EQ(multiStateDiscreteValueType.get(), iter.first);
    }

    ASSERT_EQ(multiStateDiscreteValueType.set(9999), -1);
    ASSERT_EQ(multiStateDiscreteValueType.exists(9999), false);


    // use move contructor
    {
        EnumValues movedChoices = choices;
        EnumValues movedEmptychoices = emptychoices;
        EnumValueHandler tempMultiStateDiscreteValueType(std::move(movedChoices));

        // construction without any choice is forbidden!
        EXPECT_THROW(EnumValueHandler multiStateDiscreteValueType(std::move(movedEmptychoices)), std::runtime_error);

    }



    introspection.setEnumVariant("theProperty", multiStateDiscreteValueType);

    // adding another one with the same name overwrites the existing one!
    introspection.setEnumVariant("theProperty", multiStateDiscreteValueType);

    // Add another one
    EnumValueHandler otherMultiStateDiscreteValueType(otherChoices);
    introspection.setEnumVariant("theOtherProperty", otherMultiStateDiscreteValueType);


    // After that point the jet proxy is constructed. No more types are added. Only existing one may change.

    // Manipulate choices
    // This happens when introspection changes which is caused by change of dependencies.
    // Int this case the new possibilities support the current value. As a result it should be kept.
    multiStateDiscreteValueType.set(20);
    multiStateDiscreteValueType.setEnumValues(moreChoices);
    ASSERT_EQ(multiStateDiscreteValueType.get(), 20);

    // now we select a value and change to possibilities that do not include the value.
    // as a result we switch to the first possible value
    multiStateDiscreteValueType.set(100);
    multiStateDiscreteValueType.setEnumValues(choices);
    ASSERT_EQ(multiStateDiscreteValueType.get(), choices.begin()->first);

    // set to empty choice is forbidden!
    EXPECT_THROW(multiStateDiscreteValueType.setEnumValues(emptychoices), std::runtime_error);
    {
        // same with moved parameter
        EnumValues movedEmptyChoices;
        EXPECT_THROW(multiStateDiscreteValueType.setEnumValues(std::move(movedEmptyChoices)), std::runtime_error);
    }

    {
        // move in a valid collection
        EnumValues movedChoices = {
            { 1, { .description = "description1", .name = "name1"}},
            { 20, { .description = "description20", .name = "name20"}},
            { 30, { .description = "description30", .name = "name30"}}
        };
        multiStateDiscreteValueType.setEnumValues(std::move(movedChoices));
    }



    // Update after all known changes are made
    introspection.update();
}

TEST_F(EnumValuesTest, fb_object_type_description)
{
    static const std::string description = "description";

    {
        Json::Value schema = JsonSchema::createEnumValuetType(description);
        ASSERT_EQ(schema[JsonSchema::DESCRIPTION].asString(), description);
        ASSERT_EQ(schema[JsonSchema::TYPE].asString(), JsonSchema::TYPE_ENUM_VALUE_TYPE);
    }

    {
        Json::Value schema = EnumValuesProxy::composePropertiesTypes();
        ASSERT_EQ(schema[EnumValuesProxy::PROP_ENUM_VARIANT_VARIABLE][JsonSchema::DESCRIPTION].isString(), true);
        ASSERT_EQ(schema[EnumValuesProxy::PROP_ENUM_VARIANT_VARIABLE][JsonSchema::TYPE].asString(), JsonSchema::TYPE_ENUM_VALUE_TYPE);
        ASSERT_EQ(schema[EnumValuesProxy::PROP_EVEN][JsonSchema::DESCRIPTION].isString(), true);
        ASSERT_EQ(schema[EnumValuesProxy::PROP_EVEN][JsonSchema::TYPE].asString(), JsonSchema::TYPE_BOOL);
    }
}


TEST_F(EnumValuesTest, proxy_test)
{
    std::string basePath = "/test/proxy";

    EnumValuesProxy enumVariantProxy(clientJetPeer.getAsyncPeer(), basePath);

    {
        // request with invalid value is rejected!
        Json::Value request;
        request[EnumValuesProxy::PROP_ENUM_VARIANT_VARIABLE] = 9999;
        EXPECT_THROW(clientJetPeer.setStateValue(enumVariantProxy.getPath(), request), std::runtime_error);
    }

    {
        // request with invalid value is rejected!
        Json::Value request;
        request[EnumValuesProxy::PROP_EVEN] = true;
        clientJetPeer.setStateValue(enumVariantProxy.getPath(), request);
    }

    JetProxy::restoreAllDefaults();
}
}
