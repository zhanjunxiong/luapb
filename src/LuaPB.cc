
#include "LuaPB.h"
#include "ProtoImporter.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor_database.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/compiler/importer.h>

static int pb__repeated_add(lua_State* L);
static int pb_repeated_len(lua_State* L);
static const struct luaL_reg repeatedlib[] =
{
		{"add", pb__repeated_add},
		{"len", pb_repeated_len},
		{NULL, NULL}
};

static int push_repeated_msg(lua_State* L, google::protobuf::Message* msg, google::protobuf::FieldDescriptor *field)
{
	lua_pushlightuserdata(L, field);
	lua_gettable(L, LUA_REGISTRYINDEX);
	if (lua_isnil(L, -1))
	{
		lua_newtable(L);

		lua_repeated_msg* repeated = static_cast<lua_repeated_msg*>(lua_newuserdata(L, sizeof(lua_repeated_msg)));
		repeated->msg = msg;
		repeated->field = field;
		lua_rawseti(L, -2, 0); //

		lua_pushvalue(L, -1);
		luaL_openlib(L, NULL, repeatedlib, 1);

		luaL_getmetatable(L, PB_REPEATED_MESSAGE_META);
		lua_setmetatable(L, -2);

		lua_pushlightuserdata(L, field);
		lua_pushvalue(L, -2);
		lua_settable(L, LUA_REGISTRYINDEX);
	}
	return 1;
}

lua_repeated_msg* get_repeated_msg(lua_State* L)
{
	luaL_checktype(L, lua_upvalueindex(1), LUA_TTABLE);
	lua_rawgeti(L, lua_upvalueindex(1), 0);
	if (!lua_isnil(L, -1))
	{
		lua_repeated_msg* msg = static_cast<lua_repeated_msg*>(lua_touserdata(L, -1));
		return msg;
	}
	luaL_argerror(L, (0), "get_repeated_msg(lua_State* L), repeated msg is nill\n");
	return NULL;
}

lua_repeated_msg* get_repeated_msg(lua_State* L, int stackindex)
{
	luaL_checktype(L, stackindex, LUA_TTABLE);
	lua_rawgeti(L, stackindex, 0);
	if (!lua_isnil(L, -1))
	{
		lua_repeated_msg* msg = static_cast<lua_repeated_msg*>(lua_touserdata(L, -1));
		return msg;
	}
	luaL_argerror(L, (0), "get_repeated_msg(lua_State* L, int stackindex), repeated msg is nill\n");
	return NULL;
}

static int pb__repeated_add(lua_State* L)
{
	lua_repeated_msg* repeated = get_repeated_msg(L);
    google::protobuf::Message* message = repeated->msg;
    if (!message)
    {
    	luaL_argerror(L, 1, "pb__repeated_add, pb msg is nil");
    	return 0;
    }
    google::protobuf::FieldDescriptor* field = repeated->field;
    const google::protobuf::Reflection* reflection = message->GetReflection();
    luaL_argcheck(L, field != NULL, 1, "pb_repeated_add, field is null");

    if(field->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE)
   {
    	google::protobuf::Message* msg = reflection->AddMessage(message, field);
    	push_message(L, msg);
    	return 1;
    }
    else if(field->type() == google::protobuf::FieldDescriptor::TYPE_INT32)
    {
    	int val = static_cast<int>(luaL_checkinteger(L, 1));
        reflection->AddInt32(message, field, val);
    }
    else if (field->type() == google::protobuf::FieldDescriptor::TYPE_INT64)
    {
        long val = static_cast<long>(luaL_checknumber(L, 1));
        reflection->AddInt64(message, field, val);
    }
    else if(field->type() == google::protobuf::FieldDescriptor::TYPE_UINT32)
    {
        unsigned int val = static_cast<unsigned int>(luaL_checknumber(L, 1));
        reflection->AddUInt32(message, field, val);
    }
    else if (field->type() == google::protobuf::FieldDescriptor::TYPE_UINT64)
    {
        unsigned long val = static_cast<unsigned long>(luaL_checknumber(L, 1));
        reflection->AddUInt64(message, field, val);
    }
    else if(field->type() == google::protobuf::FieldDescriptor::TYPE_FLOAT)
    {
        float val = static_cast<float>(luaL_checknumber(L, 1));
        reflection->AddFloat(message, field, val);
    }
    else if(field->type() == google::protobuf::FieldDescriptor::TYPE_DOUBLE)
    {
        double val =  static_cast<double>(luaL_checknumber(L, 1));
        reflection->AddDouble(message, field, val);
    }
    else if(field->type() == google::protobuf::FieldDescriptor::TYPE_BOOL)
    {
        int val = static_cast<int>(luaL_checkinteger(L, 1));
        reflection->AddBool(message, field, val);
    }
    else if(field->type() == google::protobuf::FieldDescriptor::TYPE_STRING)
    {
    	size_t strlen;
    	const char *str = luaL_checklstring(L, 1, &strlen);
        reflection->AddString(message, field, str);
    }
    else if (field->type() == google::protobuf::FieldDescriptor::TYPE_BYTES)
    {
    	size_t strlen;
    	const char *str = luaL_checklstring(L, 1, &strlen);
        reflection->AddString(message, field, str);
    }
    else
    {
        luaL_argerror(L, (1), "pb_repeated_add field name type for add  is not support!!");
     }
    return 0;
}

static int pb_repeated_get(lua_State* L)
{
	lua_repeated_msg* repeated = get_repeated_msg(L, 1);
    google::protobuf::Message* message = repeated->msg;
    if (!message)
    {
    	luaL_argerror(L, 1, "pb_repeated_get, pb msg is nil");
    	return 0;
    }
    google::protobuf::FieldDescriptor *field = repeated->field;
    const google::protobuf::Reflection* reflection = message->GetReflection();
    luaL_argcheck(L, field != NULL, 1, "pb_repeated_get field not exist");

    // -1 为了和lua的下标从一开始保持一致
	int index = static_cast<int>(luaL_checkinteger(L, 2)) - 1;
	luaL_argcheck(L, index >= 0, 2, "pb_repeated_get index expected >= 1");
	if(field->type() == google::protobuf::FieldDescriptor::TYPE_INT32)
	{
		lua_pushinteger(L, reflection->GetRepeatedInt32(*message, field, index));
	}
	else if(field->type() == google::protobuf::FieldDescriptor::TYPE_STRING)
	{
		lua_pushstring(L, reflection->GetRepeatedString(*message, field, index).data());
	}
    else if (field->type() == google::protobuf::FieldDescriptor::TYPE_BYTES)
    {
		lua_pushstring(L, reflection->GetRepeatedString(*message, field, index).data());
    }
	else if(field->type() == google::protobuf::FieldDescriptor::TYPE_UINT32)
	{
		lua_pushinteger(L, reflection->GetRepeatedUInt32(*message, field, index));
	}
	else if(field->type() == google::protobuf::FieldDescriptor::TYPE_FLOAT)
	{
		lua_pushnumber(L, reflection->GetRepeatedFloat(*message, field, index));
	}
	else if(field->type() == google::protobuf::FieldDescriptor::TYPE_DOUBLE)
	{
		lua_pushnumber(L, reflection->GetRepeatedDouble(*message, field, index));
	}
	else if(field->type() == google::protobuf::FieldDescriptor::TYPE_BOOL)
	{
		lua_pushboolean(L, reflection->GetRepeatedBool(*message, field, index));
	}
	else if(field->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE)
	{
		google::protobuf::Message* msg = reflection->MutableRepeatedMessage(message, field, index);
		push_message(L, msg);
	}
	else
	{
		luaL_argerror(L, 0, "pb_repeated_get, field type for get not support!!!");
		return 0;
	}
	return 1;
}

static int pb_repeated_set(lua_State* L)
{
	lua_repeated_msg* repeated = get_repeated_msg(L, 1);
    google::protobuf::Message *message = repeated->msg;
    if (!message)
    {
    	luaL_argerror(L, 1, "pb_repeated_set, pb msg is nil");
    	return 0;
    }

    const google::protobuf::Reflection* reflection = message->GetReflection();
    google::protobuf::FieldDescriptor *field = repeated->field;
    luaL_argcheck(L, field != NULL, 1, "pb_repeated_set field not exist");

	int index = static_cast<int>(luaL_checkinteger(L, 2)) - 1;
	luaL_argcheck(L, index >= 0, 2, "pb_repeated_set index expected >= 1");

	if(field->type() == google::protobuf::FieldDescriptor::TYPE_INT32)
	{
		int val = static_cast<int>(luaL_checkinteger(L, 3));
		reflection->SetRepeatedInt32(message, field, index, val);
	}
	else if(field->type() == google::protobuf::FieldDescriptor::TYPE_UINT32)
	{
		unsigned int val = static_cast<unsigned int>(luaL_checkinteger(L, 3));
		reflection->SetRepeatedUInt32(message, field, index, val);
	}
	else if(field->type() == google::protobuf::FieldDescriptor::TYPE_FLOAT)
	{
		float val = static_cast<float>(luaL_checknumber(L, 3));
		reflection->SetRepeatedFloat(message, field, index, val);
	}
	else if(field->type() == google::protobuf::FieldDescriptor::TYPE_DOUBLE)
	{
		double val = static_cast<double>(luaL_checknumber(L, 3));
		reflection->SetRepeatedDouble(message, field, index, val);
	}
	else if(field->type() == google::protobuf::FieldDescriptor::TYPE_BOOL)
	{
		int val = static_cast<int>(lua_toboolean(L, 3));
		reflection->SetRepeatedBool(message, field, index, val);
	}
	else if(field->type() == google::protobuf::FieldDescriptor::TYPE_STRING)
	{
		size_t strlen;
		const char *str = static_cast<const char *>(luaL_checklstring(L, 3, &strlen));
		reflection->SetRepeatedString(message, field, index, str);
	}
	else if(field->type() == google::protobuf::FieldDescriptor::TYPE_BYTES)
	{
		size_t strlen;
		const char *str = static_cast<const char *>(luaL_checklstring(L, 3, &strlen));
		reflection->SetRepeatedString(message, field, index, str);
	}
	else
	{
		luaL_argerror(L, (2), "pb_repeated_set type for set not support!!!");
	}
	return 0;
}

static int pb_repeated_len(lua_State* L)
{
	lua_repeated_msg* repeated = get_repeated_msg(L);
    google::protobuf::Message *message = repeated->msg;
    if (!message)
    {
    	luaL_argerror(L, 1, "pb_repeated_len, pb msg is nil");
    	return 0;
    }

    const google::protobuf::Reflection* reflection = message->GetReflection();
    google::protobuf::FieldDescriptor *field = repeated->field;
    luaL_argcheck(L, field != NULL, 1, "pb_repeated_len field not exist");

    int fieldsize = reflection->FieldSize(*message, field);
    lua_pushinteger(L, fieldsize);
    return 1;
}
////////////////////////////////////////////////////////////
static int pb_import(lua_State* L)
{
	const char* filename = luaL_checkstring(L, 1);
	sProtoImporter.Import(filename);
	return 0;
}

static int pb_new(lua_State* L)
{
	const char* type_name = luaL_checkstring(L, 1);
	google::protobuf::Message *message = sProtoImporter.createDynamicMessage(type_name);
	if (!message)
	{
		fprintf(stderr, "pb_new error, result is typename(%s) not found!\n", type_name);
		return 0;
	}

	push_message(L, message, true);
	return 1;
}

static int pb_delete(lua_State* L)
{
	lua_pbmsg* luamsg = (lua_pbmsg*)luaL_checkudata(L, 1, PB_MESSAGE_META);

    if (luamsg->isDelete && luamsg->msg)
    {
    	google::protobuf::Message*message = luamsg->msg;
    	delete message;
    	luamsg->msg = NULL;
    }
    return 0;
}

static int pb_tostring(lua_State* L)
{
	lua_pbmsg* luamsg = (lua_pbmsg*)luaL_checkudata(L, 1, PB_MESSAGE_META);
    google::protobuf::Message *message = luamsg->msg;
	if (!message)
	{
		luaL_argerror(L, 1, "pb_tostring,  pb msg is nil");
		return 0;
	}
    std::string msg(message->DebugString());
    lua_pushlstring(L, msg.c_str(), msg.length());
	return 1;
}

static int pb_get(lua_State* L)
{
	lua_pbmsg* luamsg = (lua_pbmsg*)luaL_checkudata(L, 1, PB_MESSAGE_META);
	const char* field_name = luaL_checkstring(L, 2);

    google::protobuf::Message *message = luamsg->msg;
    if (!message)
    {
    	luaL_argerror(L, 1, "pb_get,  pb msg is nil");
    	return 0;
    }

    const google::protobuf::Descriptor* descriptor = message->GetDescriptor();
    const google::protobuf::Reflection* reflection = message->GetReflection();
    const google::protobuf::FieldDescriptor *field = descriptor->FindFieldByName(field_name);
    luaL_argcheck(L, (field != NULL), 2, "pb_get, field_name error");

    if (field->is_repeated())
    {
    	push_repeated_msg(L, message, const_cast<google::protobuf::FieldDescriptor *>(field));
    }
    else if(field->type() == google::protobuf::FieldDescriptor::TYPE_INT32)
	{
		lua_pushinteger(L, reflection->GetInt32(*message, field));
	}
    else if (field->type() == google::protobuf::FieldDescriptor::TYPE_INT64)
    {
        lua_pushnumber(L, reflection->GetInt64(*message, field));
    }
	else if(field->type() == google::protobuf::FieldDescriptor::TYPE_UINT32)
	{
		lua_pushinteger(L, reflection->GetUInt32(*message, field));
	}
    else if (field->type() == google::protobuf::FieldDescriptor::TYPE_UINT64)
    {
        lua_pushnumber(L, reflection->GetUInt64(*message, field));
    }
	else if(field->type() == google::protobuf::FieldDescriptor::TYPE_FLOAT)
	{
		 lua_pushnumber(L, reflection->GetFloat(*message, field));
	}
	else if(field->type() == google::protobuf::FieldDescriptor::TYPE_DOUBLE)
	{
		 lua_pushnumber(L, reflection->GetDouble(*message, field));
	}
	else if(field->type() == google::protobuf::FieldDescriptor::TYPE_BOOL)
	{
		lua_pushboolean(L, reflection->GetBool(*message, field));
	}
	else if(field->type() == google::protobuf::FieldDescriptor::TYPE_STRING)
	{
		std::string str(reflection->GetString(*message, field));
		lua_pushlstring(L, str.c_str(), str.length());
	}
	else if(field->type() == google::protobuf::FieldDescriptor::TYPE_BYTES)
	{
		std::string str(reflection->GetString(*message, field));
		lua_pushlstring(L, str.c_str(), str.length());
	}
	if(field->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE)
	{
    	google::protobuf::Message* msg = reflection->MutableMessage(message, field);
    	push_message(L, msg, false);
	}
    return 1;
}

static int pb_set(lua_State* L)
{
	lua_pbmsg* luamsg = (lua_pbmsg*)luaL_checkudata(L, 1, PB_MESSAGE_META);
	const char* field_name = luaL_checkstring(L, 2);

    google::protobuf::Message *message = luamsg->msg;
    if (!message)
    {
    	luaL_argerror(L, 1, "pb_set, pb msg is nil");
    	return 0;
    }

    const google::protobuf::Descriptor* descriptor = message->GetDescriptor();
    const google::protobuf::Reflection* reflection = message->GetReflection();
    const google::protobuf::FieldDescriptor *field = descriptor->FindFieldByName(field_name);

    luaL_argcheck(L, field != NULL, 2, "LuaPB::set field_name error");
    luaL_argcheck(L, !field->is_repeated(), 2, "LuaPB::set field_name is repeated");

    if(field->type() == google::protobuf::FieldDescriptor::TYPE_STRING)
    {
    	size_t strlen;
    	const char *str = luaL_checklstring(L, 3, &strlen);
        reflection->SetString(message, field, str);
    }
    else if (field->type() == google::protobuf::FieldDescriptor::TYPE_BYTES)
    {
    	size_t strlen;
    	const char *str = luaL_checklstring(L, 3, &strlen);
        reflection->SetString(message, field, str);
    }
    else if(field->type() == google::protobuf::FieldDescriptor::TYPE_INT32)
    {
        int val = static_cast<int>(luaL_checkinteger(L, 3));
        reflection->SetInt32(message, field, val);
    }
    else if(field->type() == google::protobuf::FieldDescriptor::TYPE_INT64)
    {
        long val = static_cast<long>(luaL_checknumber(L, 3));
        reflection->SetInt64(message, field, val);
    }
    else if(field->type() == google::protobuf::FieldDescriptor::TYPE_UINT32)
    {
        unsigned int val = static_cast<unsigned int>(luaL_checkinteger(L, 3));
        reflection->SetUInt32(message, field, val);
    }
    else if(field->type() == google::protobuf::FieldDescriptor::TYPE_UINT64)
    {
        unsigned long val = static_cast<unsigned long>(luaL_checknumber(L, 3));
        reflection->SetUInt64(message, field, val);
    }
    else if(field->type() == google::protobuf::FieldDescriptor::TYPE_FLOAT)
    {
        float val = static_cast<float>(luaL_checknumber(L, 3));
        reflection->SetFloat(message, field, val);
    }
    else if(field->type() == google::protobuf::FieldDescriptor::TYPE_DOUBLE)
    {
        double val = static_cast<double>(luaL_checknumber(L, 3));
        reflection->SetDouble(message, field, val);
    }
    else if(field->type() == google::protobuf::FieldDescriptor::TYPE_BOOL)
    {
        int val = static_cast<int>(luaL_checkinteger(L, 3));
        reflection->SetBool(message, field, val);
    }
    else
    {
    	luaL_argerror(L, 2, "pb_set field_name type error");
    }
    return 0;
}

static int pb_parseFromString(lua_State* L)
{
	lua_pbmsg* luamsg = (lua_pbmsg*)luaL_checkudata(L, 1, PB_MESSAGE_META);
    google::protobuf::Message *message = luamsg->msg;

    luaL_checktype(L, 2, LUA_TSTRING);

    size_t bin_len;
    const char* bin = static_cast<const char*>(	luaL_checklstring(L, 2, &bin_len));
    message->ParseFromArray(bin, bin_len);
    return 0;
}

static int pb_serializeToString(lua_State* L)
{
	lua_pbmsg* luamsg = (lua_pbmsg*)luaL_checkudata(L, 1, PB_MESSAGE_META);
    google::protobuf::Message *message = luamsg->msg;

    std::string msg;
    message->SerializeToString(&msg);
    lua_pushlstring(L, msg.c_str(), msg.length());
	return 1;
}

static const struct luaL_reg lib[] =
{
		{"new", pb_new},
		{"import", pb_import},
		{"tostring", pb_tostring},
		{"parseFromString", pb_parseFromString},
		{"serializeToString", pb_serializeToString},
		{NULL, NULL}
};

static const struct luaL_reg libm[] =
{
		{"__index", pb_get},
		{"__newindex", pb_set},
		{"__gc", pb_delete},
		{NULL, NULL}
};

static const struct luaL_reg repeatedlibm[] =
{
		{"__index", pb_repeated_get},
		{"__newindex", pb_repeated_set},
		{NULL, NULL}
};

int luaopen_luapb(lua_State* L)
{
	luaL_newmetatable(L, PB_MESSAGE_META);
	lua_pushvalue(L, -1);
	lua_pushstring(L, "v");
	lua_setfield(L, -2, "__mode");
	luaL_register(L, NULL, libm);
	lua_setmetatable(L, -2);

	luaL_newmetatable(L, PB_REPEATED_MESSAGE_META);
	luaL_register(L, NULL, repeatedlibm);

	luaL_register(L, PB_MESSAGE, lib);
	return 1;
}

int push_message(lua_State* L, google::protobuf::Message* message, bool del)
{
	lua_pushlightuserdata(L, message);
	lua_gettable(L, LUA_REGISTRYINDEX);
	if (lua_isnil(L, -1))
	{
		lua_pbmsg* tmp = static_cast<lua_pbmsg*>(lua_newuserdata(L, sizeof(lua_pbmsg)));
		tmp->msg = message;
		tmp->isDelete = del;
		luaL_getmetatable(L, PB_MESSAGE_META);
		lua_setmetatable(L, -2);

		//
		lua_pushlightuserdata(L, message);
		lua_pushvalue(L, -2);
		lua_settable(L, LUA_REGISTRYINDEX);
	}
	return 1;
}
