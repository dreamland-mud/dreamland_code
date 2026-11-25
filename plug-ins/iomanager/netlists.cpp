/* Dream Land, 2026.
 *
 * Network tags for IP addresses -- see netlists.h.
 */
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include "netlists.h"
#include "dreamland.h"
#include "dldirectory.h"
#include "dlfile.h"
#include "logstream.h"

using namespace std;

namespace {

// An IPv6 address as two host-order halves, so ranges compare like integers.
struct V6 {
    uint64_t h, l;
    bool operator<(const V6 &o) const { return h < o.h || (h == o.h && l < o.l); }
    bool operator==(const V6 &o) const { return h == o.h && l == o.l; }
};

inline uint32_t succ(uint32_t a) { return a + 1; }
inline V6 succ(const V6 &a) { V6 r = a; if (++r.l == 0) ++r.h; return r; }

// Inclusive range; org indexes AsnTable::orgs (unused by the tag lists).
template <class A>
struct Range {
    A lo, hi;
    uint32_t org;
};

template <class A>
bool rangeLess(const Range<A> &a, const Range<A> &b)
{
    return a.lo < b.lo;
}

struct ListTable {
    vector<Range<uint32_t> > v4;
    vector<Range<V6> > v6;
};

struct AsnTable {
    vector<Range<uint32_t> > v4;
    vector<Range<V6> > v6;
    vector<string> orgs;
};

enum { SRC_VPN4, SRC_VPN6, SRC_DC4, SRC_DC6, SRC_TOR, SRC_ASN, SRC_COUNT };

struct Source {
    const char *file;
    time_t mtime;
    bool checked;
};

Source sources[SRC_COUNT] = {
    { "vpn-ipv4.txt", 0, false },
    { "vpn-ipv6.txt", 0, false },
    { "datacenter-ipv4.txt", 0, false },
    { "datacenter-ipv6.txt", 0, false },
    { "tor-exit.txt", 0, false },
    { "ip2asn-combined.tsv", 0, false },
};

ListTable lists[SRC_ASN];
AsnTable asn;
time_t lastCheck = 0;

const int CHECK_EVERY = 60;

bool parse4(const char *s, uint32_t &a)
{
    struct in_addr x;
    if (inet_pton(AF_INET, s, &x) != 1)
        return false;
    a = ntohl(x.s_addr);
    return true;
}

bool parse6(const char *s, V6 &a)
{
    struct in6_addr x;
    if (inet_pton(AF_INET6, s, &x) != 1)
        return false;
    a.h = a.l = 0;
    for (int i = 0; i < 8; i++)
        a.h = (a.h << 8) | x.s6_addr[i];
    for (int i = 8; i < 16; i++)
        a.l = (a.l << 8) | x.s6_addr[i];
    return true;
}

// One address in either family. An IPv4-mapped IPv6 address (::ffff:a.b.c.d)
// answers as IPv4, which is how the lists carry it.
bool parseAddr(const char *s, bool &isV4, uint32_t &a4, V6 &a6)
{
    if (parse4(s, a4)) {
        isV4 = true;
        return true;
    }
    if (!parse6(s, a6))
        return false;
    if (a6.h == 0 && (a6.l >> 32) == 0xffff) {
        isV4 = true;
        a4 = (uint32_t)(a6.l & 0xffffffffu);
        return true;
    }
    isV4 = false;
    return true;
}

// "addr" or "addr/len" into an inclusive range of its family.
bool parseCidr(char *s, bool &isV4, Range<uint32_t> &r4, Range<V6> &r6)
{
    char *slash = strchr(s, '/');
    int len = -1;
    if (slash) {
        *slash = 0;
        char *end;
        long l = strtol(slash + 1, &end, 10);
        if (end == slash + 1 || l < 0 || l > 128)
            return false;
        len = (int)l;
    }

    uint32_t a4;
    V6 a6;
    if (!parseAddr(s, isV4, a4, a6))
        return false;

    if (isV4) {
        if (len < 0)
            len = 32;
        if (len > 32)
            return false;
        uint32_t mask = len == 0 ? 0 : (0xffffffffu << (32 - len));
        r4.lo = a4 & mask;
        r4.hi = r4.lo | ~mask;
        r4.org = 0;
        return true;
    }

    if (len < 0)
        len = 128;
    uint64_t mh, ml;
    if (len >= 64) {
        int bits = len - 64;
        mh = ~0ULL;
        ml = bits == 0 ? 0 : (~0ULL << (64 - bits));
    } else {
        mh = len == 0 ? 0 : (~0ULL << (64 - len));
        ml = 0;
    }
    r6.lo.h = a6.h & mh;
    r6.lo.l = a6.l & ml;
    r6.hi.h = r6.lo.h | ~mh;
    r6.hi.l = r6.lo.l | ~ml;
    r6.org = 0;
    return true;
}

// Sort, then fold ranges that overlap or touch (same org only, when asked),
// so one binary search answers each lookup.
template <class A>
void mergeRanges(vector<Range<A> > &v, bool sameOrg)
{
    sort(v.begin(), v.end(), rangeLess<A>);
    size_t out = 0;
    for (size_t i = 0; i < v.size(); i++) {
        if (out > 0) {
            Range<A> &last = v[out - 1];
            bool touches = !(last.hi < v[i].lo) || succ(last.hi) == v[i].lo;
            if (touches && (!sameOrg || last.org == v[i].org)) {
                if (last.hi < v[i].hi)
                    last.hi = v[i].hi;
                continue;
            }
        }
        v[out++] = v[i];
    }
    v.resize(out);
    v.shrink_to_fit();
}

template <class A>
const Range<A> *findRange(const vector<Range<A> > &v, const A &x)
{
    // Last range starting at or before x.
    size_t lo = 0, hi = v.size();
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (x < v[mid].lo)
            hi = mid;
        else
            lo = mid + 1;
    }
    if (lo == 0)
        return 0;
    const Range<A> &r = v[lo - 1];
    return (r.hi < x) ? 0 : &r;
}

void chomp(char *s)
{
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r' || s[n - 1] == ' ' || s[n - 1] == '\t'))
        s[--n] = 0;
}

// A provider name goes straight into game output: plain ASCII, and no '{',
// which the renderer would read as a colour code.
string cleanOrg(const char *s)
{
    string out;
    for (; *s; s++) {
        unsigned char c = (unsigned char)*s;
        if (c == '{')
            out += '(';
        else if (c >= 0x20 && c < 0x7f)
            out += (char)c;
        else
            out += '?';
    }
    return out;
}

bool loadList(const string &path, ListTable &t, size_t &count)
{
    FILE *f = fopen(path.c_str(), "r");
    if (!f)
        return false;

    ListTable fresh;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        chomp(line);
        char *s = line;
        while (*s == ' ' || *s == '\t')
            s++;
        if (!*s || *s == '#')
            continue;

        bool isV4;
        Range<uint32_t> r4;
        Range<V6> r6;
        if (!parseCidr(s, isV4, r4, r6))
            continue;
        if (isV4)
            fresh.v4.push_back(r4);
        else
            fresh.v6.push_back(r6);
    }
    fclose(f);

    mergeRanges(fresh.v4, false);
    mergeRanges(fresh.v6, false);
    count = fresh.v4.size() + fresh.v6.size();
    t.v4.swap(fresh.v4);
    t.v6.swap(fresh.v6);
    return true;
}

// ip2asn-combined.tsv: range_start, range_end, asn, country, description.
// ASN 0 rows ("Not routed") carry nothing to show and are skipped.
bool loadAsn(const string &path, AsnTable &t, size_t &count)
{
    FILE *f = fopen(path.c_str(), "r");
    if (!f)
        return false;

    AsnTable fresh;
    unordered_map<string, uint32_t> index;
    char line[2048];
    while (fgets(line, sizeof(line), f)) {
        char *fld[5];
        int n = 0;
        fld[n++] = line;
        for (char *p = line; *p && n < 5; p++)
            if (*p == '\t') {
                *p = 0;
                fld[n++] = p + 1;
            }
        if (n < 5)
            continue;
        chomp(fld[4]);

        char *end;
        unsigned long number = strtoul(fld[2], &end, 10);
        if (end == fld[2] || number == 0)
            continue;

        bool v4lo, v4hi;
        uint32_t lo4, hi4;
        V6 lo6, hi6;
        if (!parseAddr(fld[0], v4lo, lo4, lo6) || !parseAddr(fld[1], v4hi, hi4, hi6) || v4lo != v4hi)
            continue;

        string org = "AS" + to_string(number);
        string desc = cleanOrg(fld[4]);
        if (!desc.empty())
            org += " " + desc;

        unordered_map<string, uint32_t>::iterator i = index.find(org);
        uint32_t idx;
        if (i == index.end()) {
            idx = (uint32_t)fresh.orgs.size();
            fresh.orgs.push_back(org);
            index[org] = idx;
        } else {
            idx = i->second;
        }

        if (v4lo) {
            Range<uint32_t> r = { lo4, hi4, idx };
            fresh.v4.push_back(r);
        } else {
            Range<V6> r = { lo6, hi6, idx };
            fresh.v6.push_back(r);
        }
    }
    fclose(f);

    mergeRanges(fresh.v4, true);
    mergeRanges(fresh.v6, true);
    fresh.orgs.shrink_to_fit();
    count = fresh.v4.size() + fresh.v6.size();
    t.v4.swap(fresh.v4);
    t.v6.swap(fresh.v6);
    t.orgs.swap(fresh.orgs);
    return true;
}

void clearSource(int i)
{
    if (i == SRC_ASN) {
        AsnTable empty;
        asn.v4.swap(empty.v4);
        asn.v6.swap(empty.v6);
        asn.orgs.swap(empty.orgs);
    } else {
        ListTable empty;
        lists[i].v4.swap(empty.v4);
        lists[i].v6.swap(empty.v6);
    }
}

// Reload whatever file changed since the last look, at most once a minute.
void refresh()
{
    time_t now = time(0);
    if (lastCheck != 0 && now - lastCheck < CHECK_EVERY)
        return;
    lastCheck = now;

    DLDirectory dir(dreamland->getDbDir(), "netlists");
    for (int i = 0; i < SRC_COUNT; i++) {
        string path = DLFile(dir, sources[i].file).getPath();
        struct stat st;
        time_t mtime = stat(path.c_str(), &st) == 0 ? st.st_mtime : 0;

        if (sources[i].checked && mtime == sources[i].mtime)
            continue;
        sources[i].checked = true;
        sources[i].mtime = mtime;

        if (mtime == 0) {
            clearSource(i);
            continue;
        }

        try {
            size_t count = 0;
            bool ok = i == SRC_ASN ? loadAsn(path, asn, count) : loadList(path, lists[i], count);
            if (ok)
                LogStream::sendNotice() << "NetLists: " << sources[i].file << ", " << count << " ranges." << endl;
            else
                LogStream::sendError() << "NetLists: cannot read " << path << endl;
        } catch (const std::exception &e) {
            LogStream::sendError() << "NetLists: " << sources[i].file << ": " << e.what() << endl;
        }
    }
}

bool inList(int src, bool isV4, uint32_t a4, const V6 &a6)
{
    return isV4 ? findRange(lists[src].v4, a4) != 0 : findRange(lists[src].v6, a6) != 0;
}

// 3 tor, 2 vpn, 1 datacenter, 0 none. The datacenter list includes the VPN
// ranges, so VPN is asked first.
int rankOf(const string &ip)
{
    bool isV4;
    uint32_t a4;
    V6 a6;
    if (!parseAddr(ip.c_str(), isV4, a4, a6))
        return 0;
    if (inList(SRC_TOR, isV4, a4, a6))
        return 3;
    if (inList(SRC_VPN4, isV4, a4, a6) || inList(SRC_VPN6, isV4, a4, a6))
        return 2;
    if (inList(SRC_DC4, isV4, a4, a6) || inList(SRC_DC6, isV4, a4, a6))
        return 1;
    return 0;
}

string orgOf(const string &ip)
{
    bool isV4;
    uint32_t a4;
    V6 a6;
    if (!parseAddr(ip.c_str(), isV4, a4, a6))
        return string();

    uint32_t idx;
    if (isV4) {
        const Range<uint32_t> *r = findRange(asn.v4, a4);
        if (!r)
            return string();
        idx = r->org;
    } else {
        const Range<V6> *r = findRange(asn.v6, a6);
        if (!r)
            return string();
        idx = r->org;
    }
    return idx < asn.orgs.size() ? asn.orgs[idx] : string();
}

// "a, b" -> {"a", "b"}, blanks dropped.
vector<string> chainOf(const DLString &host)
{
    vector<string> out;
    size_t start = 0;
    while (start <= host.size()) {
        size_t comma = host.find(',', start);
        if (comma == string::npos)
            comma = host.size();
        string part = host.substr(start, comma - start);
        size_t b = part.find_first_not_of(" \t");
        size_t e = part.find_last_not_of(" \t");
        if (b != string::npos)
            out.push_back(part.substr(b, e - b + 1));
        start = comma + 1;
    }
    return out;
}

} // namespace

DLString NetLists::tag(const DLString &ip)
{
    static const char *names[] = { "", "dc", "vpn", "tor" };

    refresh();
    int best = 0;
    vector<string> chain = chainOf(ip);
    for (size_t i = 0; i < chain.size(); i++)
        best = max(best, rankOf(chain[i]));
    return names[best];
}

DLString NetLists::org(const DLString &ip)
{
    refresh();
    string out;
    vector<string> chain = chainOf(ip);
    for (size_t i = 0; i < chain.size(); i++) {
        string o = orgOf(chain[i]);
        if (o.empty())
            continue;
        if (!out.empty())
            out += " / ";
        out += o;
    }
    return out;
}
