#ifndef JSON_UTILS_EXT_H
#define JSON_UTILS_EXT_H

#include "json_utils.h"
#include "register-decl.h"

namespace JsonUtils {
    /** Get a Register with string/numeric/array/struct or boolean value of this JSON. */
    Scripting::Register toRegister(const Json::Value &value);

    /** Get a representation of a JSON object as a Fenia structure with fields. */
    Scripting::Register toIdContainer(const Json::Value &jsonObj);

    /** Get a representation of a JSON array as a Fenia array. */
    Scripting::Register toRegContainer(const Json::Value &jsonArray);
}

#endif
