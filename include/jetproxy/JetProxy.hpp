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

#pragma once

#include <map>
#include <string>
#include <vector>

#include "jet/peerasync.hpp"

#include "jetproxy/JsonSchema.hpp"
#include "ProxyJetStates.hpp"
#include "Method.hpp"
#include "objectmodel/ObjectModelConstants.hpp"


namespace hbk::jetproxy
{
    static const std::string ROLE_USER = "user";
    static const std::string ROLE_ADMIN = "admin";
    static const std::string ROLE_SUPPORT = "support";
    static const std::string ROLE_FACTORY = "factory";
    static const std::string ROLE_DEVELOPER = "developer";

    enum class RoleLevel {
        USER = 0,
        ADMIN = 1,
        SUPPORT = 2,
        FACTORY = 3,
        DEVELOPER = 4
    };

    class JetProxy
    {
    public:

        virtual ~JetProxy();

        /// Used by the factory to create a dynamic object of any type.
        /// Since object is created by the factory, it can also be removed again => fixed = false
        template<typename objectType>
        static std::unique_ptr< JetProxy > createMethod(hbk::jet::PeerAsync& jetPeer, std::string path)
        {
            return std::make_unique < objectType >(jetPeer, path, false);
        }

        /// This function composes a type description of the template argument in our JsonSchema variant.
        ///
        /// @warning It is used when the type is added dynamically to opc-ua.
        /// If the type is defined in an opc-ua-companion specification, This method should not be used!
        ///
        /// Template parameter: ObjectType, the type that needs to be described
        ///     Type Requirements:
        ///         - ObjectType::Type, name of the type
        ///         - ObjectType::composePropertiesTypes(), A function returning a Json::Value with the Properties in that Type
        ///             Properties are the variables and methods that the type has
        template<typename ObjectType> /* We could make functionblocktype in to a concept*/
        static Json::Value composeType(std::string& super_type)
        {
            /// \todo: Im taking stuff from the json schema tutorial, we might need to clean up later
            Json::Value type;
            type[JsonSchema::SCHEMA] = "http://json-schema.org/draft-07/schema#";
            type[JsonSchema::ID] = "https://hbk-world.com/schemas/" + std::string(ObjectType::TYPE) + ".json"; // http://example.com/product.schema.json
            type[JsonSchema::TITLE] = ObjectType::TYPE;
            type[JsonSchema::TYPE] = super_type; //These types will be objects
            type[JsonSchema::PROPERTIES] = ObjectType::composePropertiesTypes();
            return type;
        }

        /// Arrays have a fixed number of elements
        template<typename T>
        static Json::Value composeArray(const T* array, std::size_t size)
        {
            // An array with size 0 is a bit strange. Nevertheless initialize as empty array
            Json::Value arrayValue = Json::arrayValue;
            for (std::size_t i = 0; i < size; ++i)
            {
                arrayValue.append(array[i]);
            }
            return arrayValue;
        }

        /// Vectors have a variable number of elements
        template<typename T>
        static Json::Value composeVector(const std::vector <T >& vector)
        {
            // Initialize as empty array
            Json::Value arrayValue = Json::arrayValue;
            for (const auto& iter : vector) {
                arrayValue.append(iter);
            }
            return arrayValue;
        }

        // Structs in a Vector need a own compose function so that
        // the content of a struct is changed to a Json::Value
        template<typename T>
        static Json::Value composeVectorOfStructs(const std::vector <T >& vector)
        {
            // Initialize as empty array
            Json::Value arrayValue = Json::arrayValue;
            for (const auto& iter : vector) {
                arrayValue.append(iter.compose());
            }
            return arrayValue;
        }


        void addReferenceByTarget(const std::string& referenceId, const std::string& targetId);
        void addReferenceBySource(const std::string& referenceId, const std::string& sourceId);
        void deleteReferenceByTarget(const std::string& referenceId, const std::string& targetId);
        void deleteReferenceBySource(const std::string& referenceId, const std::string& sourceId);
        std::string getPath() const
        {
            return m_path;
        }

        /// Saves complete configuration of all existing jet proxies to json file.
        /// Existing file is overwritten.
        /// Each jet proxy configuration is saved under its jet path.
        /// \code
        /// {
        ///   <jet proxy path ("/fb/scaler1")> :
        ///   {
        ///     "connections" : <the connections>,
        ///     "properties" : <the properties>
        ///   }
        ///   <another jet proxy path> :
        ///   {
        ///     "connections" : <the connections>,
        ///     "properties" : <the properties>
        ///   }
        ///   ...
        /// }
        /// \endcode
        static int saveAllToFile(const std::string& fileName);

        /// Read saved configuration and configure all jet proxies with matching jet path.
        ///
        /// If the requested configuration file does not exist or is invalid,
        /// default values are loaded for all jet proxies.
        ///
        /// If restoration of a single jet proxy fails, it is set to default values.
        ///
        /// File entries for which no existing jet proxies are found are ignored.
        /// They are not created! Only existing jet proxies are configured.
        static int restoreAllFromFile(const std::string& fileName);

        /// Load default settings for all jet proxies
        /// They are not created! Only existing jet proxies are configured
        /// \warning If operation fails on a jetproxy, the problem will belogged.
        /// Operation will not be aborted but will continue with the remaining jet proxies.
        static int restoreAllDefaults();

        /// It set the object on default value
        /// Each object is validated on its own.
        /// \warning The method is also resonsible to notify all affected jet states.
        virtual void restoreDefaults() = 0;

        std::string getRoleLevel() const;

        void setRoleLevel(RoleLevel roleLevel);

        bool isPersistent() const;

        void setPersistent(bool);

    protected:

        /// \param fixed If true, the jet proxy can not be removed by external clients
        /// \throws std::exception
        JetProxy(hbk::jet::PeerAsync& jetPeer, const std::string& type, const std::string& path, bool fixed, RoleLevel roleLevel = RoleLevel::USER, bool persistent = true);

        /// move constructable is usefull to move an existing object into a container
        JetProxy(JetProxy&& other) noexcept;
        /// jet proxies may not be copied!
        JetProxy(JetProxy& src) = delete;
        /// jet proxies may not be assigned!
        JetProxy& operator=(const JetProxy&) = delete;


        /// The current state of the object is notified to jet
        void notify() const;

        /// Composes complete root object
        /// This should create a Json document with each member of the Object
        virtual Json::Value compose() const;

        /// Composes all properties of the root object.
        /// That is: Add all values of the object to the root object
        virtual void composeProperties(Json::Value& composition) const = 0;

        /// Composes the root object and all sub objects.
        ///
        /// @warning This is done by overriding this method and calling compose() for the
        /// root object all sub objects.
        /// The default implementation just calls compose() which is the
        /// correct handling when there is a root object only and no sub objects!
        virtual Json::Value composeAll() const
        {
            return compose();
        }

        /// Root object and all sub objects are validated and set one after the other.
        /// All corresponding states on jet are also updated accordingly.
        ///
        /// @warning This is done by overriding this method and calling setFromJet() and notify() for the
        /// root object all sub objects.
        /// The default implementation just calls setFromJet() and notify() which is the
        /// correct handling when there is a root object only and no sub objects!
        ///
        /// @warning The method is also resonsible to notify all affected jet states.
        virtual void setAll(const Json::Value& composition)
        {
            setFromJet(composition);
            notify();
        }

        /// @todo I'm not happy with this name. Should we call it validateAndSet() instead?
        /// Called from jet to change configuration of this object.
        /// The whole request will be validated first. If this succeeds and the request contains any changes to the current state, the configuration is changed accordingly.
        /// \throw std::runtime_error if validation fails
        /// \return an empty json object if nothing changed or the complete new state of the object
        virtual hbk::jet::SetStateCbResult setFromJet(const Json::Value& request) = 0;

        hbk::jet::PeerAsync& m_jetPeer;
        std::string m_type;
        std::string m_path;
        bool m_fixed;
        RoleLevel m_roleLevel;
        bool m_persistent;

        std::unique_ptr <hbk::jetproxy::ProxyJetStates > m_state;

    private:

        /// jet path is the key
        using JetProxies = std::unordered_map<std::string, JetProxy*>;

        /// Reference id is the key. "this" is source of the reference, value is the target of the reference
        std::map<std::string, std::vector<std::string>> m_referencesByTarget;

        /// Reference id is the key. value is the source of the reference, "this" is target of the reference
        std::map<std::string, std::vector<std::string>> m_referencesBySource;

        /// Collection of all existing jet proxies.
        /// It is used for:
        /// - Save/Restore complete configuration
        static JetProxies m_jetProxies;
    };
} // namespace hbk::jetproxy
