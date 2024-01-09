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

namespace objectmodel::constants
{


    // Defined Json Members for internal Usage. for the reason that we
    // that we do not want to have in an assignment of words, all internal
    // identifiers (for the jet cache) must start with _<keyword>
    // We had the problem for example, when the object model uses a variable
    // called type, but it is already a keyword.
    // ------------------- Start ------------------------------------------
    static const std::string jsonTypeMemberId =                     "_type";        // responsible for giving a jet state a specific type
    static const std::string jsonTitleMemberId =                    "_title";       // represent the title / name of a own defined object, data type, etc.
    static const std::string jsonDescriptionMemberId =              "_description"; // Shown
    static const std::string jsonDisplayNameMemberId =              "_displayName"; // The display name which is shown up in opc-ua. May differ from the ID!
    static const std::string jsonMandatoryMemberId =                "_mandatory";   // describes if something is mandatory in a type definition
    static const std::string jsonRoleLevelMemberId =                "_roleLevel";   // Gives the role name for an object
    static const std::string jsonKeyId =                            "_key";         // Introspection data
    static const std::string jsonValueMemberId =                    "_value";       // Values (for example enum values) in a data type definition
    static const std::string jsonFixedMemberId =                    "_fixed";       // Indication that a state is fixed and can not be deleted.
    static const std::string jsonItemsMemberId =                    "_items";       // items in an array
    static const std::string jsonMaxItemsMemberId =                 "_maxItems";    // the max items of an array
    static const std::string jsonPropertiesMemberId =               "_properties";  // The properties within an object type
    static const std::string jsonNameMemberId =                     "_name";        // Name of an argument
    static const std::string jsonReturnsMemberId =                  "_returns";     // Return value a method
    static const std::string jsonEnumValuesMemberId =               "_enumValues";  // Introspection data
    static const std::string jsonSelectionValuesMemberId =          "_selectionValues"; // Introspection data
    static const std::string jsonPersistentMemberId =               "_persistent";  // Introspection data
    static const std::string jsonNumberInListMemberId =             "NumberInList";// Introspection data
    static const std::string jsonDefaultValueMemberId =             "DefaultValue";// Introspection data
    static const std::string jsonCoercionExpressionMemberId =       "CoercionExpression";// Introspection data
    static const std::string jsonValidationExpressionMemberId =     "ValidationExpression";// Introspection data
    static const std::string jsonNameSpaceUriId =                   "NamespaceUri"; // Introspection data
    static const std::string jsonEngineeringUnitsMemberId =         "EngineeringUnits"; // Introspection data
    static const std::string jsonUnitIdMemberId =                   "UnitId";       // Introspection data
    static const std::string jsonQuantityMemberId =                 "Quantity";    // Introspection data
    static const std::string jsonEURangeMemberId =                  "EuRange";    // Introspection data
    static const std::string jsonMinValueMemberId =                 "MinValue";    // Introspection data
    static const std::string jsonMaxValueMemberId =                 "MaxValue";    // Introspection data

    // For the future perhaps needed, when dynamic connection are in focus more again
    static const std::string jsonDestinationsMemberId =             "_destinations";
    static const std::string jsonPathMemberId =                     "_path";
    static const std::string jsonPortMemberId =                     "_port";
    static const std::string jsonReferenceByTargetMemberId =        "_referencesByTarget"; // reference owner is the source
    static const std::string jsonReferenceBySourceMemberId =        "_referencesBySource"; // reference owner is the target
    static const std::string jsonConnectionsMemberId =              "_connections";
    static const std::string jsonInputsMemberId =                   "_inputs";
    static const std::string jsonOutputsMemberId =                  "_outputs";
    // ------------------- END ------------------------------------------

    // Defined meta objects types at jet
    static const std::string baseObjectTypeTypeId =                 "BaseObjectType";
    static const std::string listTypeId =                           "List";

    // names as used in OPC-UA standard
    static const std::string objectTypeId =                         "Object";
    static const std::string description =                          "Description";
    static const std::string displayName =                          "DisplayName";
    static const std::string locale =                               "locale";
    static const std::string text =                                 "text";
    static const std::string connectedToReferenceType =             "ConnectedTo";

    // Fix Ids
    static const std::string idSeparator =                          "/";

    // Absolute Node Fix Ids
    static const std::string rootId =                               idSeparator;

    // Absolute Types Ids
    /// private area that is not published to public configuration interface
    static const std::string absoluteTypesId =                      rootId + "types";
    static const std::string absoluteNotificationsId =              rootId + "notifications";
    // define Types Absolute Paths

    // Absolutes Pathes
    /// private area that is not published to public configuration interface
    static const std::string typesPath =                            absoluteTypesId + idSeparator;
    /// private area that is not published to public configuration interface
    static const std::string methodTypesPath =                      typesPath + "method" + idSeparator;
    /// private area that is not published to public configuration interface
    static const std::string objectTypesPath =                      typesPath + "objectTypes" + idSeparator;
    /// private area that is not published to public configuration interface
    static const std::string dataTypesPath =                        typesPath + "dataTypes" + idSeparator;
    static const std::string absoluteNotificationsPath =            absoluteNotificationsId + idSeparator;



    // Fix Browse Names Nodes for Connections (this information is needed for special functionality)
    static const std::string portObjectTypeId =                     "portType";
    static const std::string outputPortObjectTypeId =               "outputPortType";
    static const std::string inputPortObjectTypeId =                "inputPortType";

    static const std::string inputsBrowseName =                     (char) (std::toupper(jsonInputsMemberId[0])) + jsonInputsMemberId.substr(1);
    static const std::string outputsBrowseName =                    (char) (std::toupper(jsonOutputsMemberId[0])) + jsonOutputsMemberId.substr(1);

    static const std::string portTypeBrowseName =                   (char) (std::toupper(portObjectTypeId[0])) + portObjectTypeId.substr(1);
    static const std::string inputPortTypeBrowseName =              (char) (std::toupper(inputPortObjectTypeId[0])) + inputPortObjectTypeId.substr(1);
    static const std::string ouputtPortTypeBrowseName =             (char) (std::toupper(outputPortObjectTypeId[0])) + outputPortObjectTypeId.substr(1);

    // Internal Relative Functionblock Pathes
    static const std::string relFbConnectionInputsPath =            "/connections/inputs/";
    static const std::string relFbConnectionOutputsPath =           "/connections/outputs/";

    /// everything in the "internal" space won't show up in the public interface (OPC-UA)
    static const std::string internalPath = rootId + "internal" + idSeparator;

    /// Introspection data is placed under this path
    static const std::string introspectionPath = rootId + "introspection" + idSeparator;

    /// Avahi service descriptions are expected in the internal area.
    /// There will be one object for each service to be shown in avahi.
    /// See the manpage avahi.service for details
    /// Each object will look like this:
    /// \code
    /// {
    ///   "port" : < ip port>
    ///   "textRecords" : {
    ///     "<key>" : "<value>",
    ///     ...
    ///     "<another key>" : "<value>"
    /// }
    /// \endcode
    /// The service id (example: _http._tcp) will be part of the path of each object. The complete path is:
    /// \code
    /// avahiBasePath + serviceId
    /// \endcode
    static const std::string avahiBasePath = internalPath + "avahi" + idSeparator;
    static const std::string avahiServicePort = "port";
    static const std::string avahiServiceTextRecords = "textRecords";

    /// All devices and their nodes are located under this.
    static const std::string deviceRootPath = rootId;

    /// All functionblocks are located under this node.
    static const std::string functionBlocks = "FB";
    static const std::string functionBlocksPath = rootId + functionBlocks + idSeparator;

    /// '/system' contains general stuff that should be standardized over different device types and even device families.
    /// System services are responsible for those states.
    static const std::string systemPath = rootId + "System" + idSeparator;
//    static const std::string locationPath = systemPath + "location";

    /// '/slots' contains everyhting about slots of a frame.
    /// Frame services are responsible for those states
    static const std::string slotsPath = rootId + "Slots" + idSeparator;
    /// Slots are located under slotsPath. Slots will be named 'slot<n>' with n >= 1.
    static const std::string slotPrefix = "Slot";

    static const std::string serviceIdStreamingTcp = "_streaming._tcp";
    static const std::string serviceIdStreamingWebsocket = "_streaming-ws._tcp";

    static const std::string serviceIdOpcUaTcp = "_opcua-tcp._tcp";
    static const std::string serviceIdOpcUaWebsocket = "_opcua-tls._tcp";

    static const std::string severity = "severity";
    static const std::string sourceNode = "sourceNode";
    static const std::string sourceName = "sourceName";

}
