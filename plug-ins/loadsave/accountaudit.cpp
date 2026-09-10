/* Dream Land, passwordless account layer, 2026.
 *
 * See ACCOUNTS_NANNY_ROADMAP.md / Trello 2zFpQBoW.
 */
#include <ctime>
#include <fstream>

#include "accountaudit.h"
#include "json_utils.h"
#include "dreamland.h"
#include "dlfile.h"
#include "logstream.h"

using namespace std;

void AccountAudit::record(const DLString &event, const Json::Value &fields)
{
    try {
        Json::Value row;
        row["ts"] = Json::Int64(time(0));
        row["event"] = event;

        if (fields.isObject())
            for (Json::Value::const_iterator i = fields.begin(); i != fields.end(); ++i)
                row[i.name()] = *i;

        DLString path = DLFile(dreamland->getBasePath(), "var/log/account.jsonl").getPath();
        ofstream out(path.c_str(), ios::app);
        if (!out) {
            LogStream::sendError() << "AccountAudit: cannot open " << path << endl;
            return;
        }
        // JsonUtils::toString (FastWriter) already ends the line with '\n', so no
        // endl -- that would leave a blank line between every two records.
        out << JsonUtils::toString(row);
    } catch (const std::exception &e) {
        LogStream::sendError() << "AccountAudit: " << e.what() << endl;
    }
}

void AccountAudit::record(const DLString &event)
{
    record(event, Json::Value());
}
