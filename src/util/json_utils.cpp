#include "json_utils.h"
#include "logstream.h"

const DLString JsonUtils::JSON_ERROR = "ERROR";

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
        if (value.isNull())
            return DLString::emptyString;
            
        if (value.isString())
            return value.asString();

        if (value.isNumeric())
            return DLString(value.asInt());

        if (value.isBool())
            return DLString(value.asBool());

    } catch (const std::exception &ex) {
        LogStream::sendError() << "JSON: " << ex.what() << endl;
    }
    
    return JSON_ERROR;
}

DLString JsonUtils::asQuotedString(const Json::Value &value)
{
    ostringstream buf;

    if (value.isString())
        buf << "\"" << value.asString() << "\"";
    else 
        buf << asString(value);
    
    return buf.str();
}

bool JsonUtils::findValue(const Json::Value &jsonObj, const DLString &key, DLString &value)
{
    if (jsonObj.isNull())
        return false;

    if (!jsonObj.isObject()) {
        LogStream::sendError() << "JsonUtils::findValue for " << key << " called on non-obj " << toString(jsonObj) << endl;
        return false;
    }

    // Look for string values in obj["key"] as well as in obj["blah"]["key"].
    for (auto p1 = jsonObj.begin(); p1 != jsonObj.end(); p1++) {
        if (p1->isObject()) {
            for (auto p2 = p1->begin(); p2 != p1->end(); p2++) {
                if (p2.key().asString() == key) {
                    value = asString(*p2);
                    return true;
                }
            }
        } 
        else if (p1.key().asString() == key) {
            value = asString(*p1);
            return true;
        }
    }

    return false;
}

