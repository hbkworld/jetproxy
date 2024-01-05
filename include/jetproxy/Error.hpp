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

#include "json/value.h"

#include "hbk/jsonrpc/jsonrpc_defines.h"
#include "jet/defines.h"
#include "jetproxy/ErrorCode.hpp"

namespace hbk::jetproxy
{
    /// Abstract base class for all error classes.
    class Error : public hbk::jet::jsoncpprpcException
    {
    protected:
        Error(hbk::jetproxy::ErrorCode ec, const std::string& sc, const std::string& msg, const std::string property) :
            hbk::jet::jsoncpprpcException(static_cast<int>(ec), msg, makeJson(sc, property)),
            m_ec(ec),
            m_sc(sc)
        {}

    public:
        hbk::jetproxy::ErrorCode getErrorCode() { return m_ec; }
        std::string getSecondaryErrorCode() { return m_sc; }

    private:
        static Json::Value makeJson(const std::string& sc, const std::string& property);
        hbk::jetproxy::ErrorCode m_ec;
        std::string m_sc;

    }; // class Error
} // namespace
