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


#include "hbk/sys/eventloop.h"

#include "jet/peer.hpp"

#include "jetproxy/DelayedSaver.hpp"

#include "example/EnumValuesProxy.hpp"


int main()
{
    static const std::string basePath = "/exampleEnumVariantProxy";
    
    hbk::sys::EventLoop eventLoop;
    hbk::jet::PeerAsync clientJetPeer(eventLoop, hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);
    hbk::jetproxy::DelayedSaver delayedSaver(eventLoop, clientJetPeer);
    hbk::jetproxy::DelayedSaver::Matchers matchers(1);
    matchers[0].startsWith = basePath;
    delayedSaver.start(matchers, "EnumVariantExamle.cfg");

    hbk::jetproxy::EnumValuesProxy testProxy(clientJetPeer, basePath);

    eventLoop.execute();

    return 0;
}
