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

    /** The other direction: a Fenia value as JSON, so a script can hand a whole
     *  structure to code that speaks JSON -- a web client frame, for one.
     *  Numbers, strings and null map one to one; a Fenia structure becomes an
     *  object, a list or an array becomes an array. A function, a wrapper, or
     *  anything else with no JSON meaning becomes null rather than an error:
     *  one odd field must not cost the whole frame.
     *
     *  Structures may point at each other, and a cycle would otherwise recurse
     *  until the stack ends; past 'depth' levels the value comes back null. */
    Json::Value fromRegister(const Scripting::Register &value, int depth = 16);
}

#endif
