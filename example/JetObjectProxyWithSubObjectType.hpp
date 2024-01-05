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

#include <array>
#include <string>
#include <map>

#include "jet/peerasync.hpp"

#include "jetproxy/ProxyJetStates.hpp"
#include "jetproxy/JetProxy.hpp"

namespace JetProxyExample
{
	class JetObjectProxyWithSubObjectType final : public hbk::jetproxy::JetProxy
	{
	public:

		// Is needed for using it with a type definition
		static const char TYPE[];

		static const char BOOL_PROPERTY[];
		static const char INT_PROPERTY[];
		static const char FLOAT_PROPERTY[];
		static const char FLOAT_ARRAY_PROPERTY[];

		static const char SUB_OBJECT[];

		struct SubObjectProperties {
			SubObjectProperties()
				: boolProperty(1)
				, intProperty(2)
			{
			}
			bool boolProperty;
			int intProperty;
			bool operator==(const SubObjectProperties& other) const
			{
				return ((boolProperty == other.boolProperty) &&	(intProperty == other.intProperty));
			}

		};

		struct Properties {
			/// default settings
			Properties()
				: boolProperty(1)
				, intProperty(2)
				, floatProperty(0)
				, floatArrayProperty({0})
			{
			}
			bool boolProperty;
			int intProperty;
			float floatProperty;
			std::array < float, 32 > floatArrayProperty;
			SubObjectProperties subObject;
			bool operator==(const Properties& other) const
			{
				return ((boolProperty == other.boolProperty) &&
								(intProperty == other.intProperty) &&
								(floatProperty == other.floatProperty) &&
								(floatArrayProperty == other.floatArrayProperty) &&
								//								(memcmp(floatArrayProperty, other.floatArrayProperty, sizeof(floatArrayProperty)) == 0) &&
								(subObject == other.subObject));
			}
		};

		/// \throws std::exception
		JetObjectProxyWithSubObjectType(hbk::jet::PeerAsync& jetPeer, const std::string& path);
		JetObjectProxyWithSubObjectType(hbk::jet::PeerAsync& jetPeer, const std::string& path, bool fixed);

		virtual ~JetObjectProxyWithSubObjectType() = default;

		void setProperties(const Properties& properties);
		Properties getProperties() const
		{
			return m_properties;
		}

		// Is needed for working with a type definition
		static Json::Value composePropertiesTypes();

		void restoreDefaults() override
		{
			m_properties = Properties();
		}

	private:

		virtual void composeProperties(Json::Value& composition) const override;

		virtual Json::Value composeAll() const override;
		virtual void setAll(const Json::Value& composition) override;
		void notifyAll();

		Json::Value composeSubObjectProperties() const;

        hbk::jet::SetStateCbResult setFromJet(const Json::Value& request) override;
		Json::Value setSubObjectFromJet(const Json::Value& request);

		Properties m_properties;

		std::unique_ptr<hbk::jetproxy::ProxyJetStates> m_subObject;

	};
} // namespace JetProxy
