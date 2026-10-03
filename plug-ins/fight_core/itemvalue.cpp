#include <jsoncpp/json/json.h>
#include "itemvalue.h"
#include "configurable.h"

static Json::Value itemValue;

CONFIGURABLE_LOADED(fight, item_value)
{
    itemValue = value;
}

double item_value(const char *section, const char *key, double def, int idx)
{
    if (!itemValue.isObject() || !itemValue.isMember(section))
        return def;

    const Json::Value &sec = itemValue[section];
    if (!sec.isObject() || !sec.isMember(key))
        return def;

    const Json::Value &v = sec[key];
    if (idx >= 0) {
        if (!v.isArray() || idx >= (int)v.size() || !v[idx].isNumeric())
            return def;
        return v[idx].asDouble();
    }

    return v.isNumeric() ? v.asDouble() : def;
}

double item_value_sub(const char *section, const char *key, const char *sub, double def)
{
    if (!itemValue.isObject() || !itemValue.isMember(section))
        return def;

    const Json::Value &sec = itemValue[section];
    if (!sec.isObject() || !sec.isMember(key))
        return def;

    const Json::Value &obj = sec[key];
    if (!obj.isObject() || !obj.isMember(sub) || !obj[sub].isNumeric())
        return def;

    return obj[sub].asDouble();
}

const Json::Value & item_value_object(const char *section, const char *key)
{
    static const Json::Value nullValue;

    if (!itemValue.isObject() || !itemValue.isMember(section))
        return nullValue;

    const Json::Value &sec = itemValue[section];
    if (!sec.isObject() || !sec.isMember(key))
        return nullValue;

    return sec[key];
}
