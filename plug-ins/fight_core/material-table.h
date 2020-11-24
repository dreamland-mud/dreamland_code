#ifndef MATERIAL_TABLE_H
#define MATERIAL_TABLE_H

#include "jsoncpp/json/json.h"
#include "configurable.h"

namespace Json { class Value; }

extern const FlagTable material_types;
extern const FlagTable material_flags;
extern const FlagTable imm_flags;
struct material_t {
    DLString name;
    int burns;
    int floats;
    json_flag<&material_types> type;
    json_flag<&material_flags> flags;
    json_flag<&imm_flags> vuln;
    DLString rname;

    void fromJson(const Json::Value &value);
};

extern json_vector<material_t> material_table;
const material_t * material_by_name(const DLString &name);
vector<const material_t *> materials_by_type(bitstring_t type);

#endif