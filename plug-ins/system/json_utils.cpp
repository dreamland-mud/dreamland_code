#include "json_utils.h"
#include "register-impl.h"

DLString JsonUtils::toString(const Json::Value &value)
{
    Json::FastWriter writer;
    DLString text = writer.write(value);
    return text;
}

void JsonUtils::fromString(const DLString &text, Json::Value &value)
{
    Json::Reader reader;
    reader.parse(text, value);
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
    ostringstream buf;

    if (value.isString())
        buf << "\"" << value.asString() << "\"";
    else if (value.isNumeric())
        buf << value.asInt();
    else if (value.isBool())
        buf << value.asBool();
        
    return buf.str();
}

using namespace Scripting;

Scripting::Register JsonUtils::toRegister(const Json::Value &jv)
{
    if (jv.isNull())
        return Register();

    if (jv.isBool())
        return Register(jv.asBool());

    if (jv.isInt())
        return Register(jv.asInt());

    return Register(jv.asString());        
}

