
#ifndef PROTOIMPORTER_H_
#define PROTOIMPORTER_H_

#include "Singleton.h"

#include <string>

#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/compiler/importer.h>

class ProtoImporter
{
public:
	ProtoImporter();
public:
	bool Import(const std::string& filename);
	google::protobuf::Message* createDynamicMessage(const std::string& typeName);
public:
	google::protobuf::compiler::Importer importer;
	google::protobuf::DynamicMessageFactory factory;
};

#define sProtoImporter Singleton<ProtoImporter>::instance()

#endif
