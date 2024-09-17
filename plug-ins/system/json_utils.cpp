#include "json_utils.h"
#include "register-impl.h"
#include "idcontainer.h"
#include "regcontainer.h"
#include "lex.h"
#include "logstream.h"

static const DLString JSON_ERROR = "ERROR";

DLString JsonUtils::toString(const Json::Value &value)
{
    try {
        Json::FastWriter writer;
        DLString text = writer.write(value);
        return text;
    
    } catch (const std::exception &ex) {
        LogStream::sendError() << "JSON: " << ex.what() << endl;
    }

    return JSON_ERROR;
}

void JsonUtils::fromString(const DLString &text, Json::Value &value)
{
    try {
        Json::Reader reader;
        reader.parse(text, value);

    } catch (const std::exception &ex) {
        LogStream::sendError() << "JSON: " << ex.what() << endl;
    }
}

bool JsonUtils::validate(const DLString &text, ostringstream &errbuf)
{
    try {
        Json::Value value;
        Json::Reader reader;

        if (reader.parse(text, value))
            return true;

        errbuf << reader.getFormattedErrorMessages();

    } catch (const std::exception &ex) {
        errbuf << ex.what();
    }

    return false;  
}

void JsonUtils::copy(Json::Value &dest, const Json::Value &source)
{
    fromString(
        toString(source),
        dest
    );
}

DLString JsonUtils::asString(const Json::Value &value)
{
    try {
        ostringstream buf;
    
        if (value.isString())
            buf << "\"" << value.asString() << "\"";
        else if (value.isNumeric())
            buf << value.asInt();
        else if (value.isBool())
            buf << value.asBool();
        
        return buf.str();

    } catch (const std::exception &ex) {
        LogStream::sendError() << "JSON: " << ex.what() << endl;
    }

    return JSON_ERROR;    
}

using namespace Scripting;

Scripting::Register JsonUtils::toRegister(const Json::Value &value)
{
   try {
        if (value.isNull())
            return Register();

        if (value.isBool())
            return Register(value.asBool());

        if (value.isNumeric())
            return Register(value.asInt());

        if (value.isArray())
            return JsonUtils::toRegContainer(value);
            
        if (value.isObject())
            return JsonUtils::toIdContainer(value);

        return Register(value.asString());        

    } catch (const std::exception &ex) {
        LogStream::sendError() << "JSON: " << ex.what() << endl;
    }

    return JSON_ERROR;    
}

Scripting::Register JsonUtils::toRegContainer(const Json::Value &jsonArray)
{
    if (!jsonArray.isArray())
        return JSON_ERROR;

    Scripting::Register result = Register::handler<RegContainer>();
    RegContainer *array = result.toHandler().getDynamicPointer<RegContainer>();
    int cnt = 0;

    for (auto &value: jsonArray) {
        array->setField(cnt++, JsonUtils::toRegister(value));
    }

    return result;
}

DLString JsonUtils::findValue(const Json::Value &jsonObj, const DLString &key)
{
    if (!jsonObj.isObject())
        return JSON_ERROR;

    // Look for string values in obj["key"] as well as in obj["blah"]["key"].
    for (auto p1 = jsonObj.begin(); p1 != jsonObj.end(); p1++) {
        if (p1->isObject()) {
            for (auto p2 = p1->begin(); p2 != p1->end(); p2++) {
                if (p2->isString() && p2.key().asString() == key)
                    return p2->asString();
            }
        } else if (p1->isString()) {
            if (p1.key().asString() == key)
                return p1->asString();
        }
    }

    return JSON_ERROR;
}

Scripting::Register JsonUtils::toIdContainer(const Json::Value &jsonObj)
{
    if (!jsonObj.isObject())
        return JSON_ERROR;

    try {
        Register mapReg = Register::handler<IdContainer>();
        IdContainer *map = mapReg.toHandler().getDynamicPointer<IdContainer>();

        for (auto p = jsonObj.begin(); p != jsonObj.end(); p++) {
            map->setField(
                IdRef(p.key().asString()), 
                JsonUtils::toRegister(*p));
        }

        return mapReg;    

    } catch (const std::exception &ex) {
        LogStream::sendError() << "JSON: " << ex.what() << endl;
    }

    return JSON_ERROR; 
}

