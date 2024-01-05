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
#include <unordered_map>

namespace hbk::jetproxy
{
	class StringEnum
	{
	  public:
		using Members = std::unordered_map<int, std::string>;
		StringEnum(const Members& enumMembers);
		/// \throw std::exception if selectedMember is not member of enumMembers
		StringEnum(const Members& enumMembers, int selectedMember);

		/// \return -1 error, 0 no change, 1 changed
		int setKey(int key);
		/// \return -1 error, 0 no change, 1 changed
		int setString(const std::string& string);

		int getKey() const;
		const std::string& getString() const;

	  private:
		const Members& m_enumMembers;
		// since m_enumMembers is a reference copying the iterator is ok.
		Members::const_iterator m_iter;
	};
} // namespace hbk::jetproxy
