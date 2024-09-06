#include "json_utils.h"
#include "register-impl.h"
#include "idcontainer.h"
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

Scripting::Register JsonUtils::toRegister(const Json::Value &jv)
{
    try {
        if (jv.isNull())
            return Register();

        if (jv.isBool())
            return Register(jv.asBool());

        if (jv.isInt())
            return Register(jv.asInt());

        return Register(jv.asString());        

    } catch (const std::exception &ex) {
        LogStream::sendError() << "JSON: " << ex.what() << endl;
    }

    return JSON_ERROR;    
}

Scripting::Register JsonUtils::toIdContainer(const Json::Value &value)
{
    try {
        Register mapReg = Register::handler<IdContainer>();
        IdContainer *map = mapReg.toHandler().getDynamicPointer<IdContainer>();

        for (auto p = value.begin(); p != value.end(); p++) {
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

