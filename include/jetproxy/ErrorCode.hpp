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

#include <string>

namespace hbk::jetproxy
{

    // @enum Enum of possible error codes for reporting errors on client requests.
    enum class ErrorCode
    {
        Ok                  =  0,  ///< The request succeeded
        OkOverridden        =  1,  ///< The request was performed, but the value was overridden and changed.

        InternalError       =  2,  ///< Fatal internal error occured.
        InvalidArgument     =  3,  ///< Argument or value is invalid
        WrongState          =  4,  ///< Request an action that is not allowed in the device's current state.
        NotFound            =  5,  ///< Access to a resource that is not found.
        AlreadyExists       =  6,  ///< Creation of a resource that already exists
        NotImplementedYet   =  7,  ///< Acccess of functionality that hasn't been implemented yet.
        NotSupported        =  8,  ///< Access functionality that is not supported on this device
        ReadOnly            =  9,  ///< Write to a read-only value
        ResourceUnavailable = 10,  ///< Access of a resource that currently is not available
        NoPermission        = 11,  ///< No permission to access the resource
    };

    std::string errorCodeToString(ErrorCode errorCode);
} // namespace hbk::jetproxy
