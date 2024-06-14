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

#include <stdexcept>

#include "objectmodel/ObjectModelConstants.hpp"


#include "jetproxy/SelectionValueHandler.hpp"

namespace hbk::jetproxy
{

SelectionValueHandler::SelectionValueHandler(const SelectionValues &selectionValues)
    : m_selectionValues(selectionValues)
{
    if (m_selectionValues.empty()) {
        throw std::runtime_error("Selection values type may not be constructed with empty selection values!");
    }
    m_key = m_selectionValues.cbegin()->first;
}

SelectionValueHandler::SelectionValueHandler(SelectionValues &&selectionValues)
    : m_selectionValues(std::move(selectionValues))
{
    if (m_selectionValues.empty()) {
        throw std::runtime_error("Selection values type may not be constructed with empty selection values!");
    }
    m_key = m_selectionValues.cbegin()->first;
}

void SelectionValueHandler::setSelectionValues(const SelectionValues &selectionValues)
{
    if (selectionValues.empty()) {
        throw std::runtime_error("Selection values type may not be set with empty selection values!");
    }

    m_selectionValues = selectionValues;
    updateValue();
}

void SelectionValueHandler::setSelectionValues(SelectionValues &&entries)
{
    if (entries.empty()) {
        throw std::runtime_error("Selection values type may not be set with empty selection values!");
    }

    m_selectionValues.clear();
    m_selectionValues.merge(entries);
    updateValue();
}

bool SelectionValueHandler::exists(int64_t key) const
{
    return (m_selectionValues.find(key)!=m_selectionValues.end());
}

int SelectionValueHandler::set(int64_t key)
{
    if (m_selectionValues.find(key)==m_selectionValues.end()) {
        // not available, can not set value!
        return -1;
    }
    m_key = key;
    return 0;
}

int64_t SelectionValueHandler::get() const
{
    return m_key;
}

Json::Value SelectionValueHandler::composeIntrospection() const
{
    Json::Value entries;
    for (const auto& iter : m_selectionValues) {
        Json::Value member;
        member[objectmodel::constants::jsonKeyId] = iter.first;
        member[objectmodel::constants::jsonValueMemberId] = iter.second;
        entries[objectmodel::constants::jsonSelectionValuesMemberId].append(member);
    }
    return entries;
}

std::string SelectionValueHandler::composeReferenceProperties() const
{
    std::string entries;
    for (const auto& iter : m_selectionValues) {
        if (iter.second.isString()) {
            // only values of type strings are supported!
            entries += std::to_string(iter.first) + ", %" + iter.second.asString() + ", ";
        }
    }

    if (!entries.empty()) {
        entries.pop_back();
        entries.pop_back();
    }

    return entries;
}

void SelectionValueHandler::updateValue()
{
    // see whether the current value is available in the new entries
    if (m_selectionValues.find(m_key)==m_selectionValues.end()) {
        // not available, use value of first choice then first
        m_key = m_selectionValues.cbegin()->first;
    }
}
} // namespace hbk::jetproxy
