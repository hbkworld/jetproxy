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

#include "jet/defines.h"
#include "jetproxy/Method.hpp"
#include "objectmodel/ObjectModelConstants.hpp"

namespace objModel = objectmodel::constants;

namespace hbk::jetproxy {
    Method::Method(hbk::jet::PeerAsync& peer, const std::string& path, hbk::jet::methodCallback_t callback, const MethodDescription& description)
        : m_jetPeer(peer)
        , m_methodPath(path)
    {
        m_typePath = objModel::methodTypesPath;
        /// \warning path may start with '/' => remove it from prefix
        if (path[0] == '/') {
            m_typePath.pop_back();
        }
        m_typePath += path;

        Json::Value jsonDescription;
        jsonDescription[objModel::jsonTitleMemberId] = description.title;
        jsonDescription[objModel::jsonDescriptionMemberId] = description.description;
        jsonDescription[objModel::jsonTypeMemberId] = hbk::jsonrpc::METHOD;
        jsonDescription[hbk::jet::ARGS] = Json::Value(Json::arrayValue);
        for(const auto& arg : description.args) {
            jsonDescription[hbk::jet::ARGS].append(arg_to_json(arg));
        }
        jsonDescription[objModel::jsonReturnsMemberId] = return_type_to_json(description.return_type);
        m_jetPeer.addStateAsync(m_typePath, jsonDescription, hbk::jet::responseCallback_t(), hbk::jet::stateCallback_t());
        
        // the method description has to be available before the actual method. This is why we don't call the base constructor here!
        createMethod(callback);
    }

    Method::Method(hbk::jet::PeerAsync& peer, const std::string& path, hbk::jet::methodCallback_t callback)
        : m_jetPeer(peer)
        , m_methodPath(path)
    {
        createMethod(callback);
    }

    Method::~Method()
    {
        m_jetPeer.removeMethodAsync(m_methodPath);
        if (!m_typePath.empty()) {
            m_jetPeer.removeStateAsync(m_typePath);
        }
    }
    
    void Method::createMethod(hbk::jet::methodCallback_t callback)
    {
        // nobody cares whether adding the method failed.
        // This is why we provide an empty response callback
        m_jetPeer.addMethodAsync(m_methodPath,
                                 hbk::jet::responseCallback_t(),
                                 callback
                                 );
    }

//    Json::Value Method::enum_to_json(const StringEnum::Members &members)
//    {
//        Json::Value jsonArray;
//        for(const auto& v : members)
//        {
//            jsonArray[v.second] = v.first;
//        }
//        return jsonArray;
//    }

    Json::Value Method::arg_to_json(const MethodArg &arg)
    {
        Json::Value v;
        v[objectmodel::constants::jsonNameMemberId] = arg.name;
        v[JsonSchema::DESCRIPTION] = arg.description;
        v[JsonSchema::TYPE] = arg.type;
        return v;
    }

    Json::Value Method::return_type_to_json(const MethodReturnType& rtype)
    {
        Json::Value v;
        v[JsonSchema::DESCRIPTION] = rtype.description;
        v[JsonSchema::TYPE] = rtype.type;
        return v;
    }

    RemoteMethod::RemoteMethod(hbk::jet::PeerAsync &peer, const std::string &path)
        : m_methodPath(path), m_jetPeer(peer)
    {
    }

    std::future<Json::Value> RemoteMethod::operator()(const Json::Value &value)
    {
        auto cb = [this](const Response &response) {
            method_return.set_value(response);
        };
        m_jetPeer.callMethodAsync(m_methodPath, value, cb);
        return method_return.get_future();
    }
}
