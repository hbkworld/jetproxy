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

#include <exception>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <utility>

#include "json/value.h"
#include "json/writer.h"

#include "jet/peerasync.hpp"

#include "jetproxy/JetProxy.hpp"
#include "objectmodel/ObjectModelConstants.hpp"

namespace objModel = objectmodel::constants;

namespace hbk::jetproxy
{

    JetProxy::JetProxies JetProxy::m_jetProxies;

    JetProxy::JetProxy(hbk::jet::PeerAsync& jetPeer, const std::string& type, const std::string& path, bool fixed, const RoleLevel roleLevel, bool persistent):
        m_jetPeer(jetPeer),
        m_type(type),
        m_path(path),
        m_fixed(fixed),
        m_roleLevel(roleLevel),
        m_persistent(persistent)
    {
        if (m_jetProxies.find(m_path) != m_jetProxies.end())
        {
            throw std::runtime_error("Could not create jetProxy. Path '" + m_path + "' already in use!");
        }

        m_jetProxies[m_path] = this;
    }

    JetProxy::~JetProxy()
    {
        m_jetProxies.erase(m_path);
    }

    JetProxy::JetProxy(JetProxy &&other) noexcept
        : m_jetPeer(other.m_jetPeer)
        , m_type(std::move(other.m_type))
        , m_path(std::move(other.m_path))
        , m_roleLevel(other.m_roleLevel)
        , m_state(std::move(other.m_state))
    {
    }

    void JetProxy::addReferenceByTarget(const std::string& referenceId, const std::string& targetId)
    {
        References typeReferences = m_referencesByTarget[referenceId];

        for (const auto &element : typeReferences) {
            if (element == targetId) {
                return;
            }
        }
        typeReferences.emplace_back(targetId);
        m_referencesByTarget[referenceId] = typeReferences;
        m_jetPeer.notifyState(m_path, compose());
    }

    void JetProxy::addReferenceBySource(const std::string &referenceId, const std::string &sourceId)
    {
        References typeReferences = m_referencesBySource[referenceId];

        for (const auto &element : typeReferences) {
            if (element == sourceId) {
                return;
            }
        }
        typeReferences.emplace_back(sourceId);
        m_referencesBySource[referenceId] = typeReferences;
        m_jetPeer.notifyState(m_path, compose());
    }

    void JetProxy::deleteReferenceByTarget(const std::string& referenceId, const std::string& targetId)
    {
        auto referencesIt = m_referencesByTarget.find(referenceId);
        if (referencesIt == m_referencesByTarget.end()) {
            return;
        }
        for (auto it = referencesIt->second.begin(); it != referencesIt->second.end(); it++) {
            if (*it == targetId) {
                m_referencesByTarget[referenceId].erase(it);
                m_jetPeer.notifyState(m_path, compose());
                break;
            }
        }
    }

    void JetProxy::deleteReferenceBySource(const std::string &referenceId, const std::string &sourceId)
    {
        auto referencesIt = m_referencesBySource.find(referenceId);
        if (referencesIt == m_referencesBySource.end()) {
            return;
        }
        for (auto it = referencesIt->second.begin(); it != referencesIt->second.end(); it++) {
            if (*it == sourceId) {
                m_referencesBySource[referenceId].erase(it);
                m_jetPeer.notifyState(m_path, compose());
                break;
            }
        }
    }

    void JetProxy::notify() const
    {
        m_jetPeer.notifyState(m_path, compose());
    }

    Json::Value JetProxy::compose() const
    {
        Json::Value composition;

        for (const auto& referencesIter : m_referencesByTarget) {
            for (const auto & element : referencesIter.second) {
                composition[objModel::jsonReferenceByTargetMemberId][referencesIter.first].append(element);
            }
        }
        for (const auto& referencesIter : m_referencesBySource) {
            for (const auto & element : referencesIter.second) {
                composition[objModel::jsonReferenceBySourceMemberId][referencesIter.first].append(element);
            }
        }
        composition[objectmodel::constants::jsonFixedMemberId] = m_fixed;
        composition[objectmodel::constants::jsonPersistentMemberId] = m_persistent;
        composition[objectmodel::constants::jsonRoleLevelMemberId] = getRoleLevel();
        composition[objModel::jsonTypeMemberId] = m_type;
        composeProperties(composition);

        return composition;
    }
    
    std::string JetProxy::getRoleLevel() const
    {
        switch (m_roleLevel) {
        case RoleLevel::ADMIN :
            return ROLE_ADMIN;
        case RoleLevel::SUPPORT :
            return ROLE_SUPPORT;
        case RoleLevel::FACTORY :
            return ROLE_FACTORY;
        case RoleLevel::DEVELOPER :
            return ROLE_DEVELOPER;
        default :
            // all others are treated as normal user
            return ROLE_USER;
        }
    }


    void JetProxy::setRoleLevel(RoleLevel roleLevel)
    {
        m_roleLevel = roleLevel;
        notify();
    }

    bool JetProxy::isPersistent() const
    {
        return m_persistent;
    }

    void JetProxy::setPersistent(bool persistent)
    {
        m_persistent = persistent;
    }

    int JetProxy::saveAllToFile(const std::string& fileName)
    {
        // we compose to a temporary file and move to the real destination when finished.
        const std::string tmpName = fileName + ".tmp";
        std::ofstream tmpFile;
        tmpFile.open(tmpName);
        
        if (!tmpFile) {
            std::cerr << "could not open file '" << fileName << "' for writing" << std::endl;
            return -1;
        }
        
        Json::Value config;
        /// walk to all existing jet proxies, that are marked as persistent, and save configurations as one json document to file
        for (const auto &iter: m_jetProxies) {
            if (iter.second->isPersistent()) {
                config[iter.first] = iter.second->composeAll();
            }
        }
        
        {
            Json::StreamWriterBuilder builder;
            builder["indentation"] = "  ";
            std::unique_ptr<Json::StreamWriter> writer(
                        builder.newStreamWriter());
            writer->write(config, &tmpFile);
        }
        tmpFile.close();
        ::sync();
        
        try {
            std::filesystem::rename(tmpName, fileName);
        } catch (std::filesystem::filesystem_error& e) {
            std::remove(fileName.c_str());
            std::cerr << "Could not move config file to " << fileName << e.what() << '\n';
            return -1;
        }
        ::sync();
        return 0;
    }

    int JetProxy::restoreAllDefaults()
    {
        for(auto& proxiesIter : m_jetProxies) {
            try {
                proxiesIter.second->restoreDefaults();
            } catch(const std::exception& e) {
                std::cerr << "could not restore defaults for " << proxiesIter.first << ": " << e.what() << std::endl;
            } catch(...) {
                std::cerr << "could not restore defaults for " << proxiesIter.first << std::endl;
            }
        }
        return 0;
    }
    
    int JetProxy::restoreAllFromFile(const std::string& fileName)
    {
        std::ifstream file;
        file.open(fileName);
        if (!file) {
            std::cerr << "could not open file '" << fileName << "' for reading. Restoring defaults for the complete service" << std::endl;
            restoreAllDefaults();
            return -1;
        }

        if ( file.peek() == std::ifstream::traits_type::eof()) {
            std::cerr << "Json file '" << fileName << "': Empty file for reading. Restoring defaults for the complete service" << std::endl;
            restoreAllDefaults();
            return -1;
        }

        Json::Value config;
        try
        {
            file >> config;
        } catch (const Json::RuntimeError& e)
        {
            std::cerr << "Json file '" << fileName << "' has invalid content: " << e.what() << ". Restoring defaults for the complete service" << std::endl;
            restoreAllDefaults();
            return -1;
        }

        if ( config.isNull() ) {
            std::cerr << "Json file '" << fileName << "' has no json content. Restoring defaults for the complete service" << std::endl;
            restoreAllDefaults();
            return -1;
        }

        for(Json::Value::iterator it = config.begin(); it !=config.end(); ++it)
        {
            const std::string jetPath = it.key().asString();
            const Json::Value& jsonConfig = (*it);
            const auto& jetIter = m_jetProxies.find(jetPath);
            if (jetIter==m_jetProxies.end()) {
                std::cout << "could not restore " << jetPath << ": fbproxy does not exist\n";
            } else {
                if (!jetIter->second->isPersistent()) {
                    std::cerr << jetPath << " is marked as non-persistent and won't be restored!" << std::endl;
                } else {
                    try {
                        jetIter->second->setAll(jsonConfig);
                    } catch(const std::runtime_error& excRestore) {
                        std::cerr << "could not restore " << jetPath << ": " << excRestore.what() << ". Restoring defaults!" << std::endl;
                        try {
                            jetIter->second->restoreDefaults();
                        } catch(const std::exception& excRestoreDefaults) {
                            std::cerr << "could not restore defaults for " << jetIter->first << ": " << excRestoreDefaults.what() << std::endl;
                        } catch(...) {
                            std::cerr << "could not restore defaults for " << jetIter->first << std::endl;
                        }
                    }
                }
            }
        }
        return 0;
    }
}
