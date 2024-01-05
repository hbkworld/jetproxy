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

#include <string>

#include <gtest/gtest.h>


#include "jetproxy/ErrorCode.hpp"



namespace hbk::jetproxy
{
        
    TEST(fb_test, errorcode_test)
    {
        std::string string;
        
        ASSERT_EQ(errorCodeToString(ErrorCode::Ok), "Ok");
        ASSERT_EQ(errorCodeToString(ErrorCode::OkOverridden), "OkOverridden");
        ASSERT_EQ(errorCodeToString(ErrorCode::InternalError), "InternalError");
        ASSERT_EQ(errorCodeToString(ErrorCode::InvalidArgument), "InvalidArgument");
        ASSERT_EQ(errorCodeToString(ErrorCode::WrongState), "WrongState");
        ASSERT_EQ(errorCodeToString(ErrorCode::NotFound), "NotFound");
        ASSERT_EQ(errorCodeToString(ErrorCode::AlreadyExists), "AlreadyExists");;
        ASSERT_EQ(errorCodeToString(ErrorCode::NotImplementedYet), "NotImplementedYet");
        ASSERT_EQ(errorCodeToString(ErrorCode::NotSupported), "NotSupported");
        ASSERT_EQ(errorCodeToString(ErrorCode::ReadOnly), "ReadOnly");
        ASSERT_EQ(errorCodeToString(ErrorCode::NoPermission), "NoPermission");
        ASSERT_EQ(errorCodeToString(ErrorCode::ResourceUnavailable), "ResourceUnavailable");
    }
}
