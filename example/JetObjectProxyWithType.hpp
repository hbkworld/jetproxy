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
#include <map>

#include "jet/peerasync.hpp"

#include "jetproxy/ProxyJetStates.hpp"
#include "jetproxy/JetProxy.hpp"
#include "jetproxy/JetProxy.hpp"
#include "CustomDefinedDataTypes.hpp"

namespace JetProxyExample
{
	class JetObjectProxyWithType final : public hbk::jetproxy::JetProxy
	{
	public:

		// Is needed for using it with a type definition
		static const char TYPE[];

		static const char BOOL_PROPERTY[];
		static const char INT_PROPERTY[];
		static const char FLOAT_PROPERTY[];
		static const char FLOAT_ARRAY_PROPERTY[];
		static const char FLOAT_VECTOR_PROPERTY[];
		static const char CUSTOM_DATATYPE_PROPERTY[];

		struct Properties {
			/// default settings
			Properties()
				: boolProperty(1)
				, intProperty(2)
				, floatProperty(0)
				, enumProperty(CustomDefinedDataType::HBM)
			{
			}
			bool boolProperty;
			int intProperty;
			float floatProperty;
			CustomDefinedDataType::MyEnumDataType enumProperty;
			float floatArrayProperty[32];
			std::vector < float > floatVectorProperty;
		};

		/// \throws std::exception
		JetObjectProxyWithType(hbk::jet::PeerAsync& jetPeer, const std::string path);
		JetObjectProxyWithType(hbk::jet::PeerAsync& jetPeer, const std::string path, bool fixed);

		virtual ~JetObjectProxyWithType() = default;

		void setProperties(const Properties& properties);

		void restoreDefaults() override
		{
			m_properties = Properties();
		}

		// Is needed for working with a type definition
		static Json::Value composePropertiesTypes();

	private:

		virtual void composeProperties(Json::Value& composition) const override;
        hbk::jet::SetStateCbResult setFromJet(const Json::Value& request) override;

		Properties m_properties;

	};
} // namespace JetProxy
