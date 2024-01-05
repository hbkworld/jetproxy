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

#include "hbk/jsonrpc/jsonrpc_defines.h"
#include <jet/defines.h>
#include "jetproxy/ErrorCode.hpp"
#include "jetproxy/Error.hpp"

//= READ THIS: ====================================================================================
//
// This file implements support for error reporting in the Test domain.
//
// See confluence:
//     https://confluence.hbkworld.com/display/DAQ2020/Error+reporting+and+error+codes
//
// The developers are currently expected to expand this file as needed.
//
// HOW TO EXTEND:
// For each new error the following should be done:
// * Add an entry to the enum just bove the text "INSERT POINT 1"
// * Add a new class for the error just abobe the text "INSERT POINT 2" near the
//   end of the file. Copy one of the previous classes and modify it
//
// HOW TO USE:
// To report an error of the given kind you throw an instance of the class, e.g.
//
//     throw DemoError("frequency");
//
// The optional argument is to specify the name of the argument or struct member
// that is causing the error.
//
//=================================================================================================
namespace JetProxyExample
{
    /// Base class for all error classes. There will be one base class for each
    /// domain. E.g. for bridge, CCLD and general.
    /// Instances of these classes, that all are based on exceptions, should be
    /// thrown in case of errors when parsing data from jet.
	class TestErrorBase : public hbk::jetproxy::Error
	{
    public:
        /// Constructor
        /// \param ec This specifies the general type of error that occured (e.g. InvalidArgument, ReadOnly etc.)
        /// \param sc A string uuid that specifies the error exactly. Use "uuidgen" to generate a unique id for the error.
        /// \param msg A description of the error in english.
        /// \param property The name of the argument or object member that caused the problem. Leave as empty string if not relevant.
	    TestErrorBase(hbk::jetproxy::ErrorCode ec, const std::string& sc, const std::string& msg, const std::string property) :
                        hbk::jetproxy::Error(ec, sc, msg, property)
        {}

	}; // class TestErrorBase

    // ========== Implementation of error classes start here, one for east secondary code ==========

    class DemoError1 : public TestErrorBase
    {
    public:
        DemoError1(const std::string& property) : TestErrorBase(hbk::jetproxy::ErrorCode::InvalidArgument,
                "523218be-f3ba-44d2-a4cb-d1c4de05ba2e", "First demo error", property)
        {};
    };

    class DemoError2 : public TestErrorBase
    {
    public:
        DemoError2(const std::string& property) : TestErrorBase(hbk::jetproxy::ErrorCode::WrongState,
                "6b7ebecf-68c2-422f-bc50-4543d72ed987", "Second demo error", property)
        {};
    };
        //======== INSERT POINT 2: ==================================================================
        //
        // For each new secondary error, create a new class above by copy/pasting and modifying it.
        // Remember to use "uuidgen" to generate a new uuid to use for the secondary error code of the new error.
        //
        //===========================================================================================

} // namespace
