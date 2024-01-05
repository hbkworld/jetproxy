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

#include <algorithm>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "jetproxy/StringEnum.hpp"

namespace hbk::jetproxy
{
	StringEnum::StringEnum(const Members& enumMembers)
		: m_enumMembers(enumMembers)
		, m_iter(m_enumMembers.begin())
	{
	}
	
	StringEnum::StringEnum(const Members& enumMembers, int selectedMember)
		: m_enumMembers(enumMembers)
		, m_iter(enumMembers.find(selectedMember))
	{
		if (m_iter == enumMembers.end()) {
			throw std::runtime_error("invalid member selection on construction!");
		}
	}	

	int StringEnum::setKey(int key)
	{
		auto iter = m_enumMembers.find(key);
		if (iter == m_enumMembers.end()) {
			return -1;
		}
		if (m_iter != iter) {
			m_iter = iter;
			return 1;
		}
		// not changed
		return 0;
	}

	int StringEnum::setString(const std::string& string)
	{
		auto lambda = [string](std::pair<int, const std::string&> element) { return element.second == string; };
		const auto iter = std::find_if(m_enumMembers.begin(), m_enumMembers.end(), lambda);
		if (iter == m_enumMembers.end()) {
			return -1;
		}
		if (m_iter != iter) {
			m_iter = iter;
			return 1;
		}
		// not changed
		return 0;
	}

	int StringEnum::getKey() const
	{
		return m_iter->first;
	}

	const std::string& StringEnum::getString() const
	{
		return m_iter->second;
	}
} // namespace hbk::jetproxy
