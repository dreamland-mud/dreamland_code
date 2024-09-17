#ifndef JSONUTILS_H
#define JSONUTILS_H

#include <sstream>
#include <jsoncpp/json/json.h>
#include "dlstring.h"
#include "register-decl.h"

namespace JsonUtils {
    /** Convert JSON object into one-line presentation. */
    DLString toString(const Json::Value &value);

    /** Populate JSON object from a string representation. */
    void fromString(const DLString &text, Json::Value &value);

    
    /** Attempt to parse JSON contained within a string, recording error messages in the buffer.
    *  Return true if parsing is successful.
    */
    bool validate(const DLString &text, ostringstream &errbuf);

    /** A replacement for Json::Value::copy not available in the current 1.7.4 version. Should
     * not be needed after upgrading libjsoncpp to 1.9.5
     */
    void copy(Json::Value &dest, const Json::Value &source);

    /** Return string representation of the value (with quotes for actual strings). */
    DLString asString(const Json::Value &value);

    /** Get a Register with string/numeric/array/struct or boolean value of this JSON. */
    Scripting::Register toRegister(const Json::Value &value);

    /** Get a representation of a JSON object as a Fenia structure with fields. */
    Scripting::Register toIdContainer(const Json::Value &jsonObj);

    /** Get a representation of a JSON array as a Fenia array. */
    Scripting::Register toRegContainer(const Json::Value &jsonArray);

    /** Find string value with given key in the object or in a obj within this object. */
    DLString findValue(const Json::Value &jsonObj, const DLString &key);

}

#endif