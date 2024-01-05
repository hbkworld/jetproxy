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

#include <iostream>
#include <thread>
#include <cmath>

#include <sys/poll.h>

#include "hbk/sys/eventloop.h"
#include "jet/peerasync.hpp"
#include "jetproxy/DelayedSaver.hpp"
#include "jetproxy/JetProxy.hpp"
#include "jetproxy/TypeFactory.hpp"
#include "JetObjectProxy.hpp"
#include "JetObjectProxyWithType.hpp"
#include "JetObjectProxyWithSubObjectType.hpp"
#include "JetStaticObjectProxyWithType.hpp"
#include "CustomDefinedDataTypes.hpp"

// Important: the Order of this file is important
// 1. Define DataTypes which you would like to use
// 2. Define ObjectTypes which you would like to use
// 3. Instantiate your Object Types
// 4. Woking with you Instantiated Object. In Addition there is shown, how to store and save objects.


int main()
{
    hbk::sys::EventLoop eventloop;
    hbk::jet::PeerAsync peer(eventloop, hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);

    auto m_workerThread = std::thread(std::bind(&hbk::sys::EventLoop::execute, std::ref(eventloop)));
    hbk::jetproxy::TypeFactory factory(peer);
    hbk::jetproxy::DelayedSaver delayedSaver(eventloop, peer);

    static const std::string testPrefix = "/jetproxyTest/";
    static const std::string configFile = "jetproxyExample.cfg";

    /*************************************************************************************************/
    /**********************   Section 1 **************************************************************/
    /*************************************************************************************************/

    // Add the two enum data types defined in
    factory.addDataType(JetProxyExample::CustomDefinedDataType::getMyEnumDataTypeDescription());
    factory.addDataType(JetProxyExample::CustomDefinedDataType::getMyMethodsEnumDataTypeDescription());

    /*************************************************************************************************/
    /**********************   Section 2 **************************************************************/
    /*************************************************************************************************/

    // Working with types means that a special object type is defined, so that this type is present next to the general object type.
    factory.addObjectType<JetProxyExample::JetObjectProxyWithType>(objectmodel::constants::objectTypeId);
    factory.addObjectType<JetProxyExample::JetObjectProxyWithSubObjectType>(objectmodel::constants::objectTypeId);

    // A static object type is a specialization. These types cannot be accessed via a factory, so an instance of it cannot be
    // created during runtime. The advantage is that no general valid constructor has to be used.
    factory.addStaticObjectType<JetProxyExample::JetStaticObjectProxyWithType>(objectmodel::constants::objectTypeId);

    /*************************************************************************************************/
    /**********************   Section 2 **************************************************************/
    /*************************************************************************************************/

    // Working without types means that a object is directly created without defining special object types. No step 2 is needed.
    JetProxyExample::JetObjectProxy object(peer, "object", "/myObject");
    JetProxyExample::JetObjectProxy object2(peer, "object", "/myObject/bla");

    // Initialization of objects that have a special definition with the help of object types
    auto objectWithType = std::make_unique<JetProxyExample::JetObjectProxyWithType>(peer, testPrefix + "myObjectWithType");

    // The JetObjectProxyWithSubObjectType is a very complex ObjectType definition. But it shows all possibilities.
    // This is needed if the new defined object contains objects that should not be stored as type.
    auto objectWithSubObjectType = std::make_unique<JetProxyExample::JetObjectProxyWithSubObjectType>(peer, testPrefix + "myObjectWithSubObjectType");

    // A Static Object can not be created via factory, as mentioned before. But it is possible to handover props as an exta constructor parameter
    JetProxyExample::JetStaticObjectProxyWithType::Properties props;
    props.boolProperty = true;
    for (int i = 0; i < 32; i++)
    {
        props.floatArrayProperty[i] = i*0.5f*std::pow(-1,i);
    }

    props.floatProperty = 1.5f;
    props.intProperty = -2;
    auto staticObjectWithType = std::make_unique<JetProxyExample::JetStaticObjectProxyWithType>(peer, testPrefix + "myStaticObjectWithType", props);


    // now all jet proxies are constructed. Restore previously saved config and start delayed save mechanism!
    hbk::jetproxy::JetProxy::restoreAllFromFile(configFile);
    hbk::jetproxy::DelayedSaver::Matchers matchers(1);
    matchers[0].startsWith = testPrefix;
    delayedSaver.start(matchers, configFile);

    /*************************************************************************************************/
    /**********************   Section 4 **************************************************************/
    /*************************************************************************************************/


    // now we wait for the user to press enter to change some functionblock properties of siggen1 and scaler1
    // We cycle through the different modes of the scaler and change the amplitude of the signal generator (0, 1, ..., 9, 0, 1...)
    struct pollfd fds;
    int ret;
    char readbuffer[128];
    fds.fd = 0; // this is stdin
    fds.events = POLLIN;

    JetProxyExample::JetObjectProxy::Properties objectProperties;
    objectProperties.boolProperty = false;
    objectProperties.intProperty = -10;
    objectProperties.floatProperty = 0.5;
    objectProperties.floatArrayProperty[0] = 0;
    objectProperties.floatArrayProperty[1] = 0;

    JetProxyExample::JetObjectProxyWithSubObjectType::Properties objectWithSubObjectProperties;
    objectWithSubObjectProperties.subObject.intProperty = 0;
    objectWithSubObjectProperties.intProperty = -20;

    object.setProperties(objectProperties);
    objectWithSubObjectType->setProperties(objectWithSubObjectProperties);

    do {
        std::cout << "press enter to change values... or q for quit" << std::endl;
        ret = poll(&fds, 1, -1);
        if (ret == 1) {
            if (read(0, readbuffer, sizeof(readbuffer)) ==-1) {
                break;
            }
            if (readbuffer[0] == 'q')
                break;
            objectProperties.boolProperty = !objectProperties.boolProperty;
            objectProperties.intProperty++;
            objectProperties.floatProperty+= 2.51;
            objectProperties.floatArrayProperty[0]+= 1.1;
            objectProperties.floatArrayProperty[1]+= 3.1;
            if (objectProperties.floatVectorProperty.size() > 8) {
                objectProperties.floatVectorProperty.clear();
            }
            objectProperties.floatVectorProperty.push_back(objectProperties.floatVectorProperty.size() * 0.3);

            objectWithSubObjectProperties.subObject.intProperty +=2;
            objectWithSubObjectProperties.intProperty+=1;


            object.setProperties(objectProperties);
            objectWithSubObjectType->setProperties(objectWithSubObjectProperties);
        } else if (ret == -1) {
            break;
        }
    } while (true);


    // !! Important all Instances have to be deleted before the type can be deleted !!
    objectWithType.reset();

    eventloop.stop();
    m_workerThread.join();

    return EXIT_SUCCESS;
}
