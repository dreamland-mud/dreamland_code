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
    // INTO A TEMPORARY, AND ONLY THEN OVER THE CALLER'S VALUE.
    //
    // The result of parse() was ignored, and Json::Reader fills what it is
    // given with everything it managed to read before the error. A world file
    // cut short -- a half-finished edit, a truncated copy, a disk that filled --
    // therefore loaded as a smaller file that looked perfectly valid: no
    // complaint anywhere, and whoever read it downstream worked from half the
    // data. On config/settings.json that showed up as a settings dialog with one
    // option in it instead of twenty-five.
    //
    // Parsing straight into the caller's value and clearing it on failure would
    // trade that bug for another one: 'value' is in-out, and Configurable::
    // setText hands over the member that holds the CURRENT configuration. A
    // broken edit saved through fedit would then wipe a table that was working
    // a moment ago. A failed parse must change nothing at all -- the caller
    // keeps whatever it had, which at boot is nothing, and the log says why.
    Json::Value parsed;

    try {
        Json::Reader reader;

        if (!reader.parse(text, parsed)) {
            LogStream::sendError() << "JSON: " << reader.getFormattedErrorMessages() << endl;
            return;
        }

    } catch (const std::exception &ex) {
        LogStream::sendError() << "JSON: " << ex.what() << endl;
        return;
    }

    value = parsed;
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

