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

#include <cstddef>
#include <iostream>
#include <string>

#include "jetproxy/TypeFactory.hpp"

#include "objectmodel/ObjectModelConstants.hpp"


namespace hbk::jetproxy {
	TypeFactory::~TypeFactory()
	{
		for (const auto& iter : m_products) {
			m_jetPeer.removeStateAsync(objectmodel::constants::objectTypesPath + iter.first);
		}
		for (const auto& iter : m_staticObjectTypes) {
			m_jetPeer.removeStateAsync(objectmodel::constants::objectTypesPath + iter);
		}
		for (const auto& iter : m_dataTypes) {
			m_jetPeer.removeStateAsync(objectmodel::constants::dataTypesPath + iter);
		}

		m_products.clear();
	}
	
	size_t TypeFactory::eraseObjectType(const std::string &type)
	{
		// try dynamic and static types
		size_t result = m_products.erase(type);
		if (result == 0) {
			result = m_staticObjectTypes.erase(type);
			if (result == 0) {
				std::cerr << "Unknown function block type " << type << " can not be unregsitered" << std::endl;
				return 0;
			}
		}
		m_jetPeer.removeStateAsync(objectmodel::constants::objectTypesPath + type);
		return 1;
	}
} // namespace hbk::jetproxy
