# JET OPC-UA Proxy

This library creates Objects on Jet, which are then interpreted by the opc-ua jet Bridge directly.

## Opc-UA Node Id Conventions

We use string ids für the OPC-UA Nodes. This has an effect of how the state id on jet should look like. These naming patterns are described here:

**Naming Patters for the Jet State ID:**

We are using Camel Case like,

``` shell
/connectors/connector/voltageCurrent
/system/blaBlub
```

If in the naming units are used, but it should be avoided, the full name is used in the signal ID and not the shortcut to avoid problems with Camel Case.

``` shell
testCurrentMilliApmere
```

For numbering we uses the pattern:

``` shell
/connectors/connector_1/...
/connectors/connector_2/...
.....
/connectors/connector_10/...
...
/connectors/connector_100/...
```

## Definition of the jet states (and how it effects OPC-UA)

On jet are states which are described in json format. These jet states can be interpreted at run time by other services. Therefore there are conventions how these jet states have to look like, so that other services like the OPC-UA bridge can benefit from them.


### Data Type Jet State Definitions

Data Types have the following prefix in the jet state id "/types/dataTypes/". Afterwards the new defined base type of the data type is added to the path, like Enum or Struct. At the end the title is added to the path.
Afterwards the data type itself is described. It has a description, which explains why this new data is needed. A title or rather the name of the new data type and the super or base data type where it is derived from. In the value field the data type is defined in more detail. The following code snippet shows an example of an enum:


``` shell
state '/types/dataTypes/enum/myEnumDataType' added
{
	"$id" : "https://hbk-world.com/schemas/myEnumDataType.json",
	"$schema" : "http://json-schema.org/draft-07/schema#",
	"_description" : "This is my enum data Type",
	"_title" : "myEnumDataType",
	"_type" : "enum",
	"_value" :
	{
		"HBK" : 1,
		"HBM" : 0
	}
}
```

### Object Type Jet State Definitions

Object Types have the following prefix in the jet state id "/types/objectTypes/". Afterwards the title is added again to the path. Under properties all properties which are bounded to the object are defined. They could have basis types or custom defined types, which were discussed before. When the object is an base object it is from the type object. The following code snippet shows an example of an object:

``` shell
state '/types/objectTypes/jetObjectProxyWithType' added
{
	"$id" : "https://hbk-world.com/schemas/jetObjectProxyWithType.json",
	"$schema" : "http://json-schema.org/draft-07/schema#",
	"_properties" :
	{
		"boolProperty" :
		{
			"_description" : "A bool value.",
			"_mandatory" : true,
			"_type" : "bool"
		},
		"customDataProperty" :
		{
			"_description" : "Setting a custom data type.",
			"_mandatory" : true,
			"_type" : "myEnumDataType"
		},
		"floatArrayProperty" :
		{
			"_description" : "A float array value.",
			"_items" :
			{
				"_type" : "float"
			},
			"_mandatory" : true,
			"_maxItems" : 32,
			"_type" : "array"
		},
		"floatProperty" :
		{
			"_description" : "A float value.",
			"_mandatory" : true,
			"_type" : "float"
		},
		"intProperty" :
		{
			"_description" : "An int value.",
			"_mandatory" : true,
			"_type" : "int32"
		}
	},
	"_title" : "jetObjectProxyWithType",
	"_type" : "object"
}

```

It is also possible to use inheritance like it is the case in this code example.

``` shell
state '/types/objectTypes/jetObjectProxyWithType' added
{
	"$id" : "https://hbk-world.com/schemas/jetObjectProxyWithType.json",
	"$schema" : "http://json-schema.org/draft-07/schema#",
	"_properties" :
	{
		"boolProperty" :
		{
			"_description" : "A bool value.",
			"_mandatory" : true,
			"_type" : "bool"
		},
	},
	"_title" : "jetObjectProxyWithType",
	"_type" : "jetObjectProxyWithType"
}

```

### Object Instances Jet State Definitions

At the end, instances of the objects can be created. Such an instance then also holds the values of an object as well as some general runtime variables, such as user level, which manages permissions for read and write access.

``` shell
state '/myObjectWithType' added
{
	"boolProperty" : true,
	"customDataProperty" : 21915,
	"_fixed" : true,
	"floatArrayProperty" :
	[
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	],
	"floatProperty" : 0,
	"intProperty" : 2,
	"_type" : "jetObjectProxyWithType",
	"_userLevel" : "admin"
}

#### Introspection

Some variables can have the type EnumVariant. Variables of this type are like enums except that the possible values are specified runtime by the function block.
Note that the possible values are specified per instance of the type, not for the type.

In OPC-UA EnumVariant is mapped to MultiStateValueDiscreteType.

This is done by creating a state under /introspection. Here is an example:


``` json
state '/introspection/myObjectWithType' added
{
    "someMultiStateValueProperty": {
        "_enumValues": [
            {
                "_name": "A",
                "_description": "Description of A",
                "_value": 100
            },
            {
                "_name": "B",
                "_description": "Description of B",
                "_value": 200
            }
        ]
    },
    "anotherMultiStateValueProperty": { ... }

}
```

`value` is of type int64



openDAQ introduces the selection variable type. It has the following introspection format:

``` json
state '/introspection/myObjectWithType' added
{
    "someSelectionVariableTypeProprty": {
        "_selectionValues": [
            {
                "_key": 100,
                "_value": { "Description of A" }
            },
            {
                "_key": 200,
                "_value": { "Description of B" }
            }
        ]

    },
    "anotherSelectionVariableTypeProprty: { ... }
}
```

`key` is of type int64



### Events
NOTE: The event code will probably be moved to another repo, but for now it is in the JetProxy repo

Events are described in Confluence:
    https://confluence.hbkworld.com/pages/editpage.action?pageId=207585319

They are triggered by instantiating one of the event classes (see include/jetproxy/Event.hpp)

You can see examples on triggering events in example/JetObjectProxy.cpp

Here is an example:



``` shell
// Trigger event telling that there is an analog overload on a given connector
hbk::jetproxy::ChannelStatusEvent(
    peer,                      // jetPeer
    "JetProxyExample",         // subSystem
    "/Connectors/Connector_1", // sourceNode
    "Connector 1",             // sourceName
    hbk::jetproxy::ChannelStatusEvent::ChannelStatusType::Analog,
    hbk::jetproxy::ChannelStatusEvent::ChannelStatus::Bad);

// Trigger event telling that there no longer is an analog overload
hbk::jetproxy::ChannelStatusEvent(
    peer,                     // jetPeer
    "JetProxyExample",        // subSystem
    "/Connectors/Connector_1", // sourceNode
    "Connector 1",            // sourceName
    hbk::jetproxy::ChannelStatusEvent::ChannelStatusType::Analog,
    hbk::jetproxy::ChannelStatusEvent::ChannelStatus::Ok);
```

Another example:

``` shell
// Report TEDS detectedA on a connector
hbk::jetproxy::TEDSDetectedEvent(
    m_jetPeer,                 // jetPeer
    "JetProxyExample",         // subSystem
    "/Connectors/Connector_3", // sourceNode
    "theConnector");           // sourceName
```
