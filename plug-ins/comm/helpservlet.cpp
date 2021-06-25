#include <jsoncpp/json/json.h>
#include "skillmanager.h"
#include "skill.h"
#include "jsonservlet.h"
#include "servlet_utils.h"
#include "commandtemplate.h"
#include "pcharacter.h"
#include "mudtags.h"
#include "merc.h"
#include "def.h"

/**
 * Public servlet: /skill <name> -- provides skill summary description
 * Args: message
 */
JSONSERVLET_HANDLE(cmd_skill, "/skill")
{
    DLString skillName;

    if (!servlet_get_arg(params, "message", skillName))
        throw Exception("'message' parameter not found");
    
    int sn = SkillManager::getThis( )->unstrictLookup(skillName);
    Skill *skill = SkillManager::getThis( )->find( sn );

    if (!skill) {
        body["error"] = "Такого умения нет.";
        return;
    }

    PCharacter dummy;
    ostringstream inBuf, outBuf;
    dummy.config.setBit(CONFIG_RUCOMMANDS|CONFIG_RUSKILLS);
    skill->show(&dummy, inBuf);

    // Strip all colour and other tags from the output.
    mudtags_convert(inBuf.str().c_str(), outBuf, 
        TAGS_CONVERT_VIS|TAGS_ENFORCE_NOWEB|
        TAGS_CONVERT_COLOR|TAGS_ENFORCE_NOCOLOR|TAGS_ENFORCE_RAW);
    body["text"] = outBuf.str();
}

