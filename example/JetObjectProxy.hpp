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
//
#pragma once

#include <string>
#include <map>

#include "jet/peerasync.hpp"

#include "jetproxy/ProxyJetStates.hpp"
#include "jetproxy/Method.hpp"
#include "jetproxy/JetProxy.hpp"
#include "jetproxy/StringEnum.hpp"

    namespace JetProxyExample
    {
        class JetObjectProxy : public hbk::jetproxy::JetProxy
        {
        public:

            // Name of the different members and methods of the object:
            static const char BOOL_PROPERTY[];
            static const char INT_PROPERTY[];
            static const char FLOAT_PROPERTY[];
            static const char FLOAT_ARRAY_PROPERTY[];
            static const char FLOAT_VECTOR_PROPERTY[];
            static const char METHOD[];

            // Struct containing the values of all members
            struct Properties {
                /// default settings
                Properties()
                    : boolProperty(1)
                    , intProperty(2)
                    , floatProperty(0)
                    , floatArrayProperty()
                    , floatVectorProperty()
                {
                    floatArrayProperty[0] = 3.2;
                    floatArrayProperty[1] = -1;
                }
                bool boolProperty;
                int intProperty;
                float floatProperty;
                float floatArrayProperty[2];
                std::vector < float > floatVectorProperty;
            };

            /// Constructors
            /// \throws std::exception
            JetObjectProxy(hbk::jet::PeerAsync& jetPeer, std::string type, const std::string path);
            JetObjectProxy(hbk::jet::PeerAsync& jetPeer, std::string type, const std::string path, bool fixed);

            /// Save the value of all members
            void setProperties(const Properties& properties);

            void restoreDefaults() override
            {
                m_properties = Properties();
            }

        private:

            /// Called from JetProxy's compose() to insert all out all member values into the specified json object
            virtual void composeProperties(Json::Value& composition) const override;

            /// Called from JetProxy when someone requests changes to the object.
            /// @param request Json object containing the requested member values
            /// @returns ???
            virtual hbk::jet::SetStateCbResult setFromJet(const Json::Value& request) override;

            /// Implementation of the example method in this object
            /// @param args A json object containing the arguments for the method
            //  @returns ???
            int method1(const Json::Value& args);

            Properties m_properties; ///< Struct containing the value of all members of the object

        };
    } // namespace JetProxy
