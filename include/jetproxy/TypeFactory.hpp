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

#include <iostream>
#include <memory>
#include <string>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "json/value.h"

#include "jet/peerasync.hpp"
#include "jetproxy/JetProxy.hpp"
#include "jetproxy/Method.hpp"
#include "objectmodel/ObjectModelConstants.hpp"
#include "DataType.hpp"

namespace hbk::jetproxy {
    /// We register all object types that are available to a object factory.
    /// Those types are added dynamically to opc-ua.
    /// @warning It should not be used for types that are defined in an opc-ua-companion specification, This method should not be used!
    /// As a result there will be a jet state with the type definition for each available functionblock proxy type.
    /// For measurement services with dynamic objects the factory will also be used to create functionblocks dynamically.
    class TypeFactory {
    public:
        
        TypeFactory(hbk::jet::PeerAsync& jetPeer)
            : m_jetPeer(jetPeer)
        {
        }
        /// unregister all types on destruction
        virtual ~TypeFactory();
        
        /// Each object type has to implement this as a static public method to create instance of this type.
        /// It always has to look like this
        /// \code
        /// return std::make_unique < object type >(jetPeer, objectId);
        /// \endcode
        using CreateMethod = std::unique_ptr < JetProxy > (*)(hbk::jet::PeerAsync& jetPeer, std::string path);
        
        
        /// Register a new data type
        /// \return 0 success, -1 error because this functionblock type is already registered
        int addDataType(DataType::DataDescription description)
        {
            if (description.title.empty())
                return -1;
            
            std::string localDataTypeId = DataType::getSuperDataTypeToString(description.superDataType) + objectmodel::constants::idSeparator + description.title;
            const auto& iter = m_dataTypes.find(localDataTypeId);
            if (iter != m_dataTypes.end()) {
                std::cerr << "Data Type " << localDataTypeId << " is already registered in factory!" << std::endl;
                return -1;
            }
            Json::Value dataTypeDefinition = DataType::composeDataType(description);
            
            try {
                m_jetPeer.addStateAsync(objectmodel::constants::dataTypesPath + localDataTypeId, 
                                        dataTypeDefinition, hbk::jet::responseCallback_t(), hbk::jet::stateCallback_t());
            } catch(const std::exception& e) {
                std::cerr << e.what() << std::endl;
                return -1;
            }
            m_dataTypes.insert(localDataTypeId);
            
            return 0;
        }
        
        
        /// Register a new object type 
        /// \return 0 success, -1 error because this object type is already registered
        template<typename object_t>
        int addObjectType(std::string super_type)
        {
            std::string objectType = object_t::TYPE;
            const auto& iter = m_products.find(objectType);
            if (iter != m_products.end()) {
                std::cerr << "Object type " << objectType << " is already registered in factory!" << std::endl;
                return -1;
            }
            Json::Value typedefinition = JetProxy::composeType<object_t>(super_type);
            
            try {
                m_jetPeer.addStateAsync(objectmodel::constants::objectTypesPath + objectType, typedefinition, hbk::jet::responseCallback_t(), hbk::jet::stateCallback_t());
            } catch(const std::exception& e) {
                std::cerr << e.what() << std::endl;
                return -1;
            }
            m_products.emplace(std::make_pair(objectType, JetProxy::createMethod< object_t >));
            
            return 0;
        }
        
        /// Register a new object type 
        /// Static object types can not be added or deleted.
        /// \return 0 success, -1 error because this object type is already registered
        template<typename object_t>
        int addStaticObjectType(std::string super_type)
        {
            std::string objectType = object_t::TYPE;
            const auto& iter = m_staticObjectTypes.find(objectType);
            if (iter != m_staticObjectTypes.end()) {
                std::cerr << "Static function block type " << objectType << " is already registered in factory!" << std::endl;
                return -1;
            }
            Json::Value typedefinition = JetProxy::composeType<object_t>(super_type);
            
            try {
                m_jetPeer.addStateAsync(objectmodel::constants::objectTypesPath + objectType, typedefinition, hbk::jet::responseCallback_t(), hbk::jet::stateCallback_t());
            } catch(const std::exception& e) {
                std::cerr << e.what() << std::endl;
                return -1;
            }
            m_staticObjectTypes.insert(objectType);
            
            return 0;
        }
        
        /// Unregister an object type
        /// \return 1 success, 0 unknown object type
        size_t eraseObjectType(const std::string& objectType);
        
        /// Create a new instance of a registered object type type.
        /// The caller takes ownership of the new instance.
        /// object type and object id are combined to et the complete path of the object (fbproxy/<fb type>/<fb id>) to be created
        /// \param path / id of the object
        /// \param name of the object
        /// \return nullptr if object type is not known
        template<typename ObjectType>
        std::unique_ptr< ObjectType > createObject(hbk::jet::PeerAsync& jetPeer, const std::string& path, bool fixed)
        {
            const auto& iter = m_products.find(ObjectType::TYPE);
            if (iter == m_products.end()) {
                std::cerr << "Unknown object type '" << ObjectType::TYPE << "'" << std::endl;
                return nullptr;
            }
            return std::make_unique < ObjectType >(jetPeer, path, fixed);
        }
        
        
    private:
        /// Product id is the key
        /// Method for creating a product instance is value.
        using ProductMap = std::unordered_map < std::string, CreateMethod >;
        
        /// path type is the key
        /// Data Type is the value
        using DataTypes = std::unordered_set < std::string >;
        
        using StaticObjectTypes = std::unordered_set < std::string >;
        /// the registered product types
        ProductMap m_products;
        DataTypes m_dataTypes;
        StaticObjectTypes m_staticObjectTypes;
        
        ///The peer in which the functionblocks are registered
        hbk::jet::PeerAsync& m_jetPeer;
    };
} //namespace hbk::jetproxy
