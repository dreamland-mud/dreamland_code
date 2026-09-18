#include "json_utils_ext.h"
#include "register-impl.h"
#include "idcontainer.h"
#include "regcontainer.h"
#include "reglist.h"
#include "fenia/object.h"
#include "lex.h"
#include "logstream.h"

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


/** A Fenia structure keys its fields by identifier; the JSON object wants the
 *  name behind it. */
static Json::Value fromIdContainer(const IdContainer *map, int depth)
{
    Json::Value result(Json::objectValue);

    for (auto i = map->idmap.begin(); i != map->idmap.end(); i++)
        result[Lex::getThis()->getName(i->first).c_str()]
            = JsonUtils::fromRegister(i->second, depth - 1);

    return result;
}

/** A Fenia array is a map keyed by number, and its keys can be anything. Sent
 *  as JSON it is an array when the keys are the numbers 0..n, and an object
 *  otherwise -- dropping a field because it was keyed by a string would be a
 *  silent loss. */
static Json::Value fromRegContainer(const RegContainer *array, int depth)
{
    bool plain = true;
    int expected = 0;

    for (auto i = array->map.begin(); i != array->map.end(); i++, expected++)
        if (i->first.type != Register::NUMBER || i->first.toNumber() != expected) {
            plain = false;
            break;
        }

    if (plain) {
        Json::Value result(Json::arrayValue);
        for (auto i = array->map.begin(); i != array->map.end(); i++)
            result.append(JsonUtils::fromRegister(i->second, depth - 1));
        return result;
    }

    Json::Value result(Json::objectValue);
    for (auto i = array->map.begin(); i != array->map.end(); i++)
        result[i->first.toString().c_str()] = JsonUtils::fromRegister(i->second, depth - 1);
    return result;
}

static Json::Value fromRegList(const RegList *list, int depth)
{
    Json::Value result(Json::arrayValue);

    for (auto i = list->begin(); i != list->end(); i++)
        result.append(JsonUtils::fromRegister(*i, depth - 1));

    return result;
}

Json::Value JsonUtils::fromRegister(const Scripting::Register &value, int depth)
{
    if (depth <= 0)
        return Json::Value::null;

    switch (value.type) {
    case Register::NONE:
        return Json::Value::null;

    case Register::NUMBER:
        return Json::Value(value.toNumber());

    case Register::IDENTIFIER:
    case Register::STRING:
        return Json::Value(value.toString().c_str());

    case Register::OBJECT:
        break;

    default:
        // A function has no JSON form at all.
        return Json::Value::null;
    }

    Scripting::Object *obj = value.toObject();
    if (!obj)
        return Json::Value::null;

    Handler::Pointer handler = obj->getHandler();
    if (!handler)
        return Json::Value::null;

    if (IdContainer *map = handler.getDynamicPointer<IdContainer>())
        return fromIdContainer(map, depth);

    if (RegContainer *array = handler.getDynamicPointer<RegContainer>())
        return fromRegContainer(array, depth);

    if (RegList *list = handler.getDynamicPointer<RegList>())
        return fromRegList(list, depth);

    // A wrapper around a mob, a room, a skill: no JSON form, and guessing at one
    // would put a random number where the client expects a value.
    return Json::Value::null;
}
