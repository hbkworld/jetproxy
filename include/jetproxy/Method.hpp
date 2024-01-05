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
#include "jet/peerasync.hpp"
#include <jet/defines.h>
#include <future>
#include <functional>
#include <iostream>

#include "StringEnum.hpp"
#include "JsonSchema.hpp"

namespace hbk::jetproxy {

class Method
{
public:

    /// This struct represents one argument to a method
    struct MethodArg {
        std::string name;
        std::string description;
        std::string type;
    };

    /// This struct represents the return value of a method
    struct MethodReturnType {
        std::string description;
        std::string type;
    };

    /// This struct represents a whole method, it can have 0-n scalar arguments, 1 scalar return type.
    struct MethodDescription {
        std::string title;
        std::string description;
        std::vector<MethodArg> args;
        MethodReturnType return_type;
    };

    /// Constructor for a method object
    /// \param path, jet path of the method, /types/method will be appended
    /// \param callback, callback for when the method is called
    /// \param description, a struct describing the name, arguments and return value of the method
    Method(hbk::jet::PeerAsync& peer, const std::string& path, hbk::jet::methodCallback_t callback, const MethodDescription &description);

    /// Constructor for a method object
    /// \param path, jet path of the method, /types/method will be appended
    /// \param callback, callback for when the method is called
    Method(hbk::jet::PeerAsync& peer, const std::string& path, hbk::jet::methodCallback_t callback);

    ~Method();

private:

    void createMethod(hbk::jet::methodCallback_t callback);

    Json::Value arg_to_json(const MethodArg& arg);
    Json::Value return_type_to_json(const MethodReturnType &rtype);

    hbk::jet::PeerAsync& m_jetPeer;
    std::string m_methodPath;
    std::string m_typePath;
};

/// calls a jet method and waits for the response.
/// \warning keep in mind that the jet peer has to be running in another thread!
class RemoteMethod
{
public:
    using Response = Json::Value;
    RemoteMethod(hbk::jet::PeerAsync& peer, const std::string& path);
    ~RemoteMethod() = default;

    std::future<Json::Value> operator()(const Json::Value& value);

private:
    std::string m_methodPath;
    std::promise<Response> method_return;
    hbk::jet::PeerAsync& m_jetPeer;
};
}
