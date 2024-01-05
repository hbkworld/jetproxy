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

#include "hbk/string/split.h"

#include "jet/peer.hpp"

#include "jetproxy/Introspection.hpp"
#include "jetproxy/JetProxy.hpp"
#include "jetproxy/JsonSchema.hpp"

#include "jetproxy/SelectionValueHandler.hpp"

#include "example/SelectionValuesProxy.hpp"

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




class SelectionValuesTest : public ::testing::Test {

protected:

    /// This jet peer represents the external client that sends requests to the service running in the device
    hbk::jet::Peer clientJetPeer;



    SelectionValuesTest()
        : clientJetPeer(hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0)
    {
    }

    virtual ~SelectionValuesTest() = default;

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



TEST_F(SelectionValuesTest, fb_object_type_test)
{
    static const SelectionValues selectionEntries =
    {
        { 1, "name1" },
        { 20, "name20" },
        { 30, "name30" }
    };
    
    static const SelectionValues MoreSelectionEntries =
    {
        { 0, "name0" },
        { 20, "name20" },
        { 100, "name100" }
    };
    
    static const SelectionValues otherSelectionChoices =
    {
        { 1, "name1" },
        { 20, "name20" },
    };

    static const SelectionValues emptySelectionChoices;
    


    std::string basePath = "/test/proxy";
    Introspection introspection(clientJetPeer.getAsyncPeer(), basePath);


    // construction without any choice is forbidden!
    EXPECT_THROW(SelectionValueHandler selectionValuesType(emptySelectionChoices), std::runtime_error);

    
    SelectionValueHandler selectionValuesHandler(selectionEntries);



    for (const auto &iter : selectionEntries) {
        ASSERT_EQ(selectionValuesHandler.set(iter.first), 0);
        ASSERT_EQ(selectionValuesHandler.exists(iter.first), true);
        ASSERT_EQ(selectionValuesHandler.get(), iter.first);
    }
    
    ASSERT_EQ(selectionValuesHandler.set(9999), -1);
    ASSERT_EQ(selectionValuesHandler.exists(9999), false);


    // use move contructor
    {
        SelectionValues movedChoices = selectionEntries;
        SelectionValues movedEmptyChoices = emptySelectionChoices;
        SelectionValueHandler tempSelectionChoices(std::move(movedChoices));

        // construction without any choice is forbidden!
        EXPECT_THROW(SelectionValueHandler selectionVariableType(std::move(movedEmptyChoices)), std::runtime_error);
    }
    


    introspection.setSelectionValues("theProperty", selectionValuesHandler);
    

    // adding another one with the same name overwrites the existing one!
    introspection.setSelectionValues("theProperty", selectionValuesHandler);

    // Add another one
    SelectionValueHandler otherSelectionValuesType(otherSelectionChoices);
    introspection.setSelectionValues("theOtherProperty", otherSelectionValuesType);


    // After that point the jet proxy is constructed. No more types are added. Only existing one may change.

    // Manipulate choices
    // This happens when introspection changes which is caused by change of dependencies.
    // Int this case the new possibilities support the current value. As a result it should be kept.
    selectionValuesHandler.set(20);
    selectionValuesHandler.setSelectionValues(MoreSelectionEntries);
    ASSERT_EQ(selectionValuesHandler.get(), 20);
    

    // now we select a value and change to possibilities that do not include the value.
    // as a result we switch to the first possible value
    selectionValuesHandler.set(100);
    selectionValuesHandler.setSelectionValues(selectionEntries);
    
    ASSERT_EQ(selectionValuesHandler.get(), selectionEntries.begin()->first);

    // set to empty entries is forbidden!
    EXPECT_THROW(selectionValuesHandler.setSelectionValues(emptySelectionChoices), std::runtime_error);
    {
        // same with moved parameter
        SelectionValues movedEmptyChoices;
        EXPECT_THROW(selectionValuesHandler.setSelectionValues(std::move(movedEmptyChoices)), std::runtime_error);
    }

    {
        // move in a valid collection
        SelectionValues movedChoices = {
            { 1, "name1"},
            { 20, "name20"},
            { 30, "name30"}
        };
        selectionValuesHandler.setSelectionValues(std::move(movedChoices));
    }

    // Update after all known changes are made
    introspection.update();
}

TEST_F(SelectionValuesTest, fb_object_type_description)
{
    static const std::string description = "description";

    {
        Json::Value schema = JsonSchema::createSelectionEntryStructureType(description);
        ASSERT_EQ(schema[JsonSchema::DESCRIPTION].asString(), description);
        ASSERT_EQ(schema[JsonSchema::TYPE].asString(), JsonSchema::TYPE_SELECTION_ENTRY_STRUCTURE);
    }

    {
        Json::Value schema = SelectionValuesProxy::composePropertiesTypes();
        ASSERT_EQ(schema[SelectionValuesProxy::PROP_SELECTION_VALUE_VARIABLE][JsonSchema::DESCRIPTION].isString(), true);
        ASSERT_EQ(schema[SelectionValuesProxy::PROP_SELECTION_VALUE_VARIABLE][JsonSchema::TYPE].asString(), JsonSchema::TYPE_SELECTION_ENTRY_STRUCTURE);
        ASSERT_EQ(schema[SelectionValuesProxy::PROP_EVEN][JsonSchema::DESCRIPTION].isString(), true);
        ASSERT_EQ(schema[SelectionValuesProxy::PROP_EVEN][JsonSchema::TYPE].asString(), JsonSchema::TYPE_BOOL);
    }
}

TEST_F(SelectionValuesTest, proxy_test)
{
    std::string basePath = "/test/proxy";

    SelectionValuesProxy proxy(clientJetPeer.getAsyncPeer(), basePath);

    {
        // request with invalid value is rejected!
        Json::Value request;
        request[SelectionValuesProxy::PROP_SELECTION_VALUE_VARIABLE] = 9999;
        EXPECT_THROW(clientJetPeer.setStateValue(proxy.getPath(), request), std::runtime_error);
    }

    {
        // request with invalid value is rejected!
        Json::Value request;
        request[SelectionValuesProxy::PROP_EVEN] = true;
        clientJetPeer.setStateValue(proxy.getPath(), request);
    }

    JetProxy::restoreAllDefaults();
}

TEST_F(SelectionValuesTest, tostring)
{
    static const SelectionValues selectionEntries =
    {
        { 1, "name1" },
        { 20, "name20" },
        { 30, "name30" }
    };


    SelectionValueHandler selectionValuesHandler(selectionEntries);

    // string should be: '1, %name1, 20, %name20, 30, %name30'
    std::string referenceProperties = selectionValuesHandler.composeReferenceProperties();
    hbk::string::tokens tokens = hbk::string::split(referenceProperties, ", ");

    for (size_t pairIndex=0; pairIndex<tokens.size()/2; ++pairIndex)
    {
        int64_t key = std::stoll(tokens[pairIndex*2]);
        auto iter = selectionEntries.find(key);
        ASSERT_NE(iter, selectionEntries.end());
        std::string expectedValue = "%" + iter->second.asString();
        ASSERT_EQ(expectedValue, tokens[(pairIndex*2)+1]);
    }

    std::string propertyName = "theProperty";
    std::string basePath = "/test/proxy";
    Introspection introspection(clientJetPeer.getAsyncPeer(), basePath);

    introspection.setSelectionValues("theProperty", selectionValuesHandler);

    // string should be: 'switch($theProperty 1, %name1, 20, %name20, 30, %name30)'
    std::string referenceVariableValue = introspection.getReferenceVariableValue("theProperty");
    std::cout << referenceVariableValue << std::endl;

    std::string prefix = "switch($theProperty, ";
    std::string::size_type pos = referenceVariableValue.find(prefix);
    ASSERT_EQ(pos, 0);
    ASSERT_EQ(referenceVariableValue.substr(prefix.length()), referenceProperties + ')');
}
}
