/*
 * aimctl - AlphaIM command line tool.
 *
 * Make databases from tdf files, query the running input method, diagnose
 * broken applications and manage the phrase db.
 *
 * See Doc/dictionary_inner.pdf for more inner format detail.
 */

#ifdef WIN32
#include <Windows.h>
#include <mbctype.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stddef.h>
#include <locale.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#ifdef WIN32
#include "win32/pgetopt.h"
#endif

#include "type.h"
#include "Util.h"
#include "CharUtil.h"
#include "config.h"
#include "aim.h"
#include "tinyxml2/tinyxml2.h"

#include "PinyinTDFParser.h"
#include "HanTDFParser.h"
#include "PhTDFParser.h"

#include "indextree/IndexTree.h"
#include "indextree/IndexTreeWriter.h"
#include "indextree_item.h"

using namespace std;
using namespace tinyxml2;

struct AimPaths {
    string home;
    string data;
    string config;
};

struct PhraseEntry {
    string key;    // e.g. "cong-从我做起"
    string py;     // e.g. "cong"
    string phrase; // e.g. "从我做起"
    int priority;
};

AimPaths aimPaths();

bool readPhraseDb(const string& path, vector<PhraseEntry>& out);
bool writePhraseDb(const string& path, const vector<PhraseEntry>& entries);
int  imdbEntries(const string& path);
bool loadHanPinyinMap(const string& dbPath, map<string, string>& out);
string firstHanChar(const string& s);
bool alphaimdRunning();

int cmdPhrase(int argc, char* argv[]);
int cmdPinyin(int argc, char* argv[]);


#define AIMCTL_VERSION "0.2"

static void usage();
static void make_pinyinDB(const string& pinyinPath, const string& dbPath);
static void make_hanDB(const string& hanPath, const string& dbPath);
static void make_phDB(const string& hanPath, const string& dbPath);
static int  cmdInfo(int argc, char* argv[]);
static int  cmdCheck(int argc, char* argv[]);
static int  cmdMake(int argc, char* argv[]);

/* ------------------------------------------------------------------ */
/* helpers                                                             */
/* ------------------------------------------------------------------ */

AimPaths aimPaths()
{
    AimPaths p;
    p.home = Util::usrProfileDir(IM_ID);

    string dir = Util::execDir();
    while (!dir.empty() && dir[dir.size() - 1] == '/')
        dir.erase(dir.size() - 1);
    p.data = Util::isFileExist(dir + "/system") ? dir + "/system" : DATADIR;

    p.config = p.home + "/configure.xml";
    return p;
}

static string trim(const string& s)
{
    size_t b = s.find_first_not_of(" \t\r\n");
    if (b == string::npos)
        return "";
    size_t e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

static string runCmd(const string& cmd)
{
    string out;
    FILE* f = popen((cmd + " 2>/dev/null").c_str(), "r");
    if (!f)
        return out;
    char buf[256];
    while (fgets(buf, sizeof(buf), f))
        out += buf;
    pclose(f);
    return trim(out);
}

static bool hasCmd(const string& name)
{
    return !runCmd("command -v " + name).empty();
}

/* return the input method name selected by im-config, e.g. "alphaim" */
static string readRunIm(const string& path)
{
    FILE* f = fopen(path.c_str(), "r");
    if (!f)
        return "";
    char buf[512];
    string im;
    while (fgets(buf, sizeof(buf), f)) {
        string line = trim(buf);
        if (line.find("run_im ") == 0) {
            im = trim(line.substr(7));
            break;
        }
    }
    fclose(f);
    return im;
}

/* recursively look for the first element with this name */
static XMLElement* findElement(XMLElement* parent, const char* name)
{
    if (!parent)
        return NULL;
    XMLElement* e = parent->FirstChildElement(name);
    if (e)
        return e;
    for (e = parent->FirstChildElement(); e; e = e->NextSiblingElement()) {
        XMLElement* found = findElement(e, name);
        if (found)
            return found;
    }
    return NULL;
}

static void printShortcut(const string& path)
{
    const char* activate = "Ctrl+Space";
    XMLDocument doc;
    if (doc.LoadFile(path.c_str()) == XML_SUCCESS) {
        XMLElement* root = doc.FirstChildElement();
        XMLElement* shortcut = root ? findElement(root, "shortcut") : NULL;
        int act = -1;
        if (shortcut && shortcut->QueryIntAttribute("activate", &act) == XML_SUCCESS) {
            if (act == 0)
                activate = "Shift+Space";
            else if (act == 2)
                activate = "Super+Space";
        }
    }

    printf("\n快捷键:\n");
    printf("  输入法开关: %s（可改配置） | Ctrl+Shift\n", activate);
    printf("  中英文切换: 左 Shift\n");
    printf("  符号切换:   右 Shift\n");
    printf("  提交:       Space | Enter\n");
    printf("  取消:       Esc\n");
    printf("  向前翻页:   PgUp | 左 | 上 | -\n");
    printf("  向后翻页:   PgDn | 右 | 下 | =\n");
}

static int cmdMake(int argc, char* argv[])
{
    if (argc < 5) {
        printf("usage: aimctl make <pinyin|han|phrase> <tdf> <db>\n");
        return 1;
    }

    string kind = argv[2];
    if (kind == "pinyin")
        make_pinyinDB(argv[3], argv[4]);
    else if (kind == "han")
        make_hanDB(argv[3], argv[4]);
    else if (kind == "phrase")
        make_phDB(argv[3], argv[4]);
    else {
        printf("unknown kind: %s (pinyin | han | phrase)\n", kind.c_str());
        return 1;
    }
    return 0;
}

static int cmdInfo(int argc, char* argv[])
{
    AimPaths paths = aimPaths();

    printf("%s %s (aimctl %s)\n", IM_NAME, VERSION, AIMCTL_VERSION);
    printf("用户目录: %s\n", paths.home.c_str());
    printf("数据目录: %s\n", paths.data.c_str());
    printf("\n词库条目:\n");
    const char* dbNames[] = { "拼音", "汉字", "词组", "用户词组" };
    const char* dbFiles[] = { "pinyin-utf8.imdb", "han-utf8.imdb",
                              "phrase-utf8.imdb", "user_phrase-utf8.imdb" };
    for (int i = 0; i < 4; ++i) {
        int n = imdbEntries(paths.home + "/" + dbFiles[i]);
        if (n < 0)
            n = imdbEntries(paths.data + "/" + dbFiles[i]);
        if (n < 0)
            printf("  %s: -\n", dbNames[i]);
        else
            printf("  %s: %d\n", dbNames[i], n);
    }

    printShortcut(paths.config);

    printf("\n运行:\n");
    string pid = runCmd("pgrep -x alphaimd | head -1");
    if (pid.empty())
        printf("  alphaimd: 未运行（执行 alphaimd & 或重新登录）\n");
    else
        printf("  alphaimd: 运行中 (pid %s)\n", pid.c_str());

    string userRc = readRunIm(string(getenv("HOME") ? getenv("HOME") : "") + "/.xinputrc");
    string sysRc = readRunIm("/etc/X11/xinit/xinputrc");
    printf("\nim-config:\n");
    printf("  ~/.xinputrc: %s\n", userRc.empty() ? "（无）" : userRc.c_str());
    printf("  /etc/X11/xinit/xinputrc: %s\n", sysRc.empty() ? "（无）" : sysRc.c_str());
    string current = userRc.empty() ? sysRc : userRc;
    if (!current.empty() && current != IM_ID)
        printf("  当前未选中 alphaim，请运行 im-config 选择 alphaim 后重新登录。\n");

    const char* envs[] = { "XMODIFIERS", "GTK_IM_MODULE", "QT_IM_MODULE" };
    printf("\n环境变量:\n");
    for (int i = 0; i < 3; ++i) {
        const char* v = getenv(envs[i]);
        printf("  %-14s %s\n", envs[i], v ? v : "未设置");
    }
    (void)argv;

    printf("\n提示: 输入法异常时运行 'aimctl check' 查看诊断。\n");
    return 0;
}

/* ------------------------------------------------------------------ */
/* diagnostics                                                         */
/* ------------------------------------------------------------------ */

static void checkCommon()
{
    AimPaths paths = aimPaths();

    printf("通用检查:\n");

    string pid = runCmd("pgrep -x alphaimd | head -1");
    if (pid.empty()) {
        printf("  [问题] alphaimd 未运行。\n");
        printf("         启动: alphaimd &   或重新登录桌面会话。\n");
    } else {
        printf("  [正常] alphaimd 运行中 (pid %s)\n", pid.c_str());
    }

    if (!Util::isFileExist(paths.config)) {
        printf("  [问题] 配置文件不存在: %s\n", paths.config.c_str());
        printf("         首次运行 alphaimd 会自动生成。\n");
    } else {
        printf("  [正常] 配置文件: %s\n", paths.config.c_str());
    }

    int han = imdbEntries(paths.home + "/han-utf8.imdb");
    int phrase = imdbEntries(paths.home + "/phrase-utf8.imdb");
    if (han < 0 || phrase < 0) {
        printf("  [问题] 词库缺失或损坏（han:%d phrase:%d）\n", han, phrase);
        printf("         可用 aimctl -a/-w 重新生成，或从 %s 复制。\n",
               paths.data.c_str());
    } else {
        printf("  [正常] 词库: 汉字 %d 条, 词组 %d 条\n", han, phrase);
    }

    const char* others[] = { "fcitx5", "fcitx", "ibus-daemon" };
    string found;
    for (int i = 0; i < 3; ++i) {
        if (!runCmd(string("pgrep -x ") + others[i]).empty()) {
            if (!found.empty()) found += ", ";
            found += others[i];
        }
    }
    if (!found.empty())
        printf("  [提醒] 检测到其它输入法进程: %s\n"
               "         同时运行多种输入法容易冲突，建议在 im-config 中只选 alphaim。\n",
               found.c_str());

    string userRc = readRunIm(string(getenv("HOME") ? getenv("HOME") : "") + "/.xinputrc");
    string sysRc = readRunIm("/etc/X11/xinit/xinputrc");
    string current = userRc.empty() ? sysRc : userRc;
    if (!current.empty() && current != IM_ID) {
        printf("  [问题] 当前输入法不是 alphaim: %s\n", current.c_str());
        printf("         运行 im-config，选择 AlphaIM，然后重新登录。\n");
    } else if (!current.empty()) {
        printf("  [正常] im-config: %s\n", current.c_str());
    }

    const char* envs[] = { "XMODIFIERS", "GTK_IM_MODULE", "QT_IM_MODULE" };
    for (int i = 0; i < 3; ++i) {
        const char* v = getenv(envs[i]);
        if (v && strstr(v, "alphaim"))
            printf("  [正常] %s=%s\n", envs[i], v);
    }
}

static void checkWechat()
{
    printf("\nWeChat:\n");
    bool installed = hasCmd("wechat") || hasCmd("wechat-universal") ||
                     hasCmd("weixin");
    if (!installed)
        printf("  [信息] 未检测到 WeChat。\n");
    printf("  Linux 微信使用静态编译的私有 Qt，alphaim 不能作为输入法注入。\n");
    printf("  解决办法: 使用托盘菜单的“输入并复制”，再粘贴到聊天窗口。\n");
}

static void checkWps()
{
    AimPaths paths = aimPaths();
    const char* dir = "/opt/kingsoft/wps-office/office6/qt/plugins/platforminputcontexts";
    string plugin = string(dir) + "/libalphaimplatforminputcontextplugin.so";
    string src = paths.data + "/im-modules/wps/Qt5.12/libalphaimplatforminputcontextplugin.so";

    printf("\nWPS:\n");
    bool installed = hasCmd("wps") || Util::isFileExist("/opt/kingsoft/wps-office");
    if (!installed) {
        printf("  [信息] 未检测到 WPS。\n");
        return;
    }
    if (Util::isFileExist(plugin)) {
        printf("  [正常] 已安装 alphaim 输入插件: %s\n", plugin.c_str());
        return;
    }
    printf("  [问题] WPS 使用自带的私有 Qt，未安装 alphaim 输入插件，无法输入中文。\n");
    printf("  解决办法: 复制插件到 WPS 插件目录后重启 WPS:\n");
    printf("    mkdir -p %s\n", dir);
    printf("    cp %s %s/\n", src.c_str(), dir);
    if (!Util::isFileExist(src))
        printf("  注意: 插件文件不存在（%s），请先编译 im-modules/wps。\n",
               src.c_str());
}

static void checkOthers()
{
    printf("\n其它应用:\n");
    printf("  GTK/Qt 程序通常可直接使用 alphaim。若某个程序无法输入中文，\n");
    printf("  先运行 im-config 选择 alphaim 并重新登录，再检查该程序的\n");
    printf("  QT_IM_MODULE / GTK_IM_MODULE 环境变量是否被覆盖。\n");
}

static int cmdCheck(int argc, char* argv[])
{
    string target;
    if (argc > 2 && argv[2][0] != '-')
        target = argv[2];

    if (target.empty()) {
        checkCommon();
        checkWechat();
        checkWps();
        checkOthers();
        return 0;
    }
    if (target == "wechat" || target == "weixin") {
        checkWechat();
        return 0;
    }
    if (target == "wps") {
        checkWps();
        return 0;
    }
    printf("未知应用: %s（支持 wechat, wps）\n", target.c_str());
    return 1;
}

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */

static void usage()
{
    printf("Usage: aimctl <command> [options]\n"
           "\n"
           "commands:\n"
           "    info                    show data dirs, dictionaries, shortcuts\n"
           "    check [app]             diagnose input problems (app: wechat, wps)\n"
           "    make pinyin tdf db      create Pinyin database\n"
           "    make han tdf db         create Han database\n"
           "    make phrase tdf db      create Phrase database\n"
           "    pinyin <字>...          show pinyin of hanzi\n"
           "    phrase <sub>            manage phrase databases\n"
           "    help                    this message\n"
           "\n"
           "phrase subcommands:\n"
           "    stat | list | find | add | set | del | clean | export | save | backup\n"
           "    (see 'aimctl phrase help')\n"
           "\n"
           "aimctl -h    this message\n"
           "aimctl -v    version\n");
}

/* ------------------------------------------------------------------ */
/* phrase / pinyin commands                                            */
/* ------------------------------------------------------------------ */

/* IndexTree prints debug lines to stdout; silence them while reading. */
static int g_quiet = 0;
static int g_savedOut = -1;

struct QuietScope {
    QuietScope() {
        if (g_quiet++ == 0) {
            fflush(stdout);
            g_savedOut = dup(1);
            int fd = open("/dev/null", O_WRONLY);
            if (fd >= 0) {
                dup2(fd, 1);
                close(fd);
            }
        }
    }
    ~QuietScope() {
        if (--g_quiet == 0 && g_savedOut >= 0) {
            fflush(stdout);
            dup2(g_savedOut, 1);
            close(g_savedOut);
            g_savedOut = -1;
        }
    }
};

#define IMDB_MAGIC    0xB4B3
#define PH_SEP        '-'
#define BASE_PRIORITY 20
#define MAX_PRIORITY  255

/* ------------------------------------------------------------------ */
/* database access                                                     */
/* ------------------------------------------------------------------ */

int imdbEntries(const string& path)
{
    FILE* f = fopen(path.c_str(), "rb");
    if (!f)
        return -1;
    struct inxtree_header h;
    size_t n = fread(&h, sizeof(h), 1, f);
    fclose(f);
    if (n != 1 || inxtree_read_u16(h.magic) != IMDB_MAGIC)
        return -1;
    return (int)inxtree_read_u32(h.d_entries);
}

bool readPhraseDb(const string& path, vector<PhraseEntry>& out)
{
    out.clear();
    QuietScope quiet;
    IndexTree tree;
    if (!tree.load(path, IMDB_MAGIC, true))
        return false;

    IndexList list;
    tree.getIndexList(list, "", true, 0, -1);
    for (size_t i = 0; i < list.size(); ++i) {
        PhraseEntry e;
        e.key = list[i]->index;
        e.priority = list[i]->d.len_data > 0 ? list[i]->d.buf[0] : 0;
        size_t pos = e.key.find(PH_SEP);
        if (pos != string::npos) {
            e.py = e.key.substr(0, pos);
            e.phrase = e.key.substr(pos + 1);
        } else {
            e.phrase = e.key;
        }
        out.push_back(e);
    }
    IndexTree::freeItems(list);
    return true;
}

bool writePhraseDb(const string& path, const vector<PhraseEntry>& entries)
{
    bool existed = Util::isFileExist(path);
    if (existed)
        Util::copyFile(path, path + ".bak");

    string tmp = path + ".aimctl-tmp";
    IndexTreeWriter w(1, INXTREE_NOT_HAS_DUPINX);
    if (existed) {
        QuietScope quiet;
        if (!w.load(path, IMDB_MAGIC, false))
            return false;
        w.clear();
    } else {
        w.open();
        memset(&w.m_header, 0, sizeof(w.m_header));
        w.m_header.magic[0] = 0xB3;
        w.m_header.magic[1] = 0xB4;
        w.m_header.d_coding[0] = INXTREE_UTF8;
        inxtree_write_u16(w.m_header.i_size, 1);
    }

    for (size_t i = 0; i < entries.size(); ++i) {
        u32* key = NULL;
        int len = CharUtil::utf8StrToUcs4Str(entries[i].key.c_str(), &key);
        if (len <= 0) {
            free(key);
            continue;
        }
        int p = entries[i].priority;
        if (p < 0) p = 0;
        if (p > MAX_PRIORITY) p = MAX_PRIORITY;
        u8 pri = (u8)p;
        w.add(key, len, &pri, 1);
        free(key);
    }

    bool ok = false;
    {
        QuietScope quiet;
        ok = w.write(tmp);
    }
    if (!ok) {
        unlink(tmp.c_str());
        return false;
    }
    if (rename(tmp.c_str(), path.c_str()) != 0) {
        unlink(tmp.c_str());
        return false;
    }
    return true;
}

bool loadHanPinyinMap(const string& dbPath, map<string, string>& out)
{
    out.clear();
    QuietScope quiet;
    IndexTree tree;
    if (!tree.load(dbPath, IMDB_MAGIC, true))
        return false;

    IndexList list;
    tree.getIndexList(list, "", true, 0, -1);
    map<string, int> best;
    for (size_t i = 0; i < list.size(); ++i) {
        HanItem h(list[i]->d.ptr);
        if (h.py.empty())
            continue;
        map<string, int>::iterator it = best.find(list[i]->index);
        if (it == best.end() || h.priority > it->second) {
            best[list[i]->index] = h.priority;
            out[list[i]->index] = h.py;
        }
    }
    IndexTree::freeItems(list);
    return true;
}

string firstHanChar(const string& s)
{
    if (s.empty())
        return "";
    unsigned char lead = (unsigned char)s[0];
    size_t w = lead < 0x80 ? 1 : (lead < 0xE0 ? 2 : (lead < 0xF0 ? 3 : 4));
    if (w > s.size())
        w = s.size();
    return s.substr(0, w);
}

bool alphaimdRunning()
{
    FILE* f = popen("pgrep -x alphaimd 2>/dev/null", "r");
    if (!f)
        return false;
    char buf[64];
    bool running = fgets(buf, sizeof(buf), f) != NULL;
    pclose(f);
    return running;
}

static void writeWarning()
{
    if (alphaimdRunning())
        printf("注意: alphaimd 正在运行，修改可能被输入法下次保存时覆盖。\n"
               "      建议先退出输入法再修改词库。\n");
}

/* ------------------------------------------------------------------ */
/* phrase commands                                                     */
/* ------------------------------------------------------------------ */

static void dbPaths(const AimPaths& p, string& user, string& sys)
{
    user = p.home + "/user_phrase-utf8.imdb";
    sys = p.home + "/phrase-utf8.imdb";
    if (!Util::isFileExist(sys))
        sys = p.data + "/phrase-utf8.imdb";
}

static bool byPriority(const PhraseEntry& a, const PhraseEntry& b)
{
    return a.priority != b.priority ? a.priority > b.priority : a.key < b.key;
}

static bool byPriorityAsc(const PhraseEntry& a, const PhraseEntry& b)
{
    return a.priority != b.priority ? a.priority < b.priority : a.key < b.key;
}

static bool loadMerged(const string& user, const string& sys,
                       bool all, bool system, const string& dbOpt,
                       vector<PhraseEntry>& out)
{
    out.clear();
    if (!dbOpt.empty())
        return readPhraseDb(dbOpt, out);

    vector<PhraseEntry> tmp;
    if (system || all) {
        if (!readPhraseDb(sys, tmp)) {
            printf("无法读取系统词组库: %s\n", sys.c_str());
            return false;
        }
        out = tmp;
    }
    if (!system) {
        if (!readPhraseDb(user, tmp)) {
            printf("无法读取用户词组库: %s\n", user.c_str());
            return false;
        }
        if (all) {
            map<string, size_t> index;
            for (size_t i = 0; i < out.size(); ++i)
                index[out[i].key] = i;
            for (size_t i = 0; i < tmp.size(); ++i) {
                map<string, size_t>::iterator f = index.find(tmp[i].key);
                if (f == index.end()) {
                    index[tmp[i].key] = out.size();
                    out.push_back(tmp[i]);
                } else if (tmp[i].priority > out[f->second].priority) {
                    out[f->second] = tmp[i];
                }
            }
        } else {
            out = tmp;
        }
    }
    return true;
}

/* replace control chars so a multi-line key cannot break output */
static string cleanKey(const string& key)
{
    string s = key;
    for (size_t i = 0; i < s.size(); ++i)
        if ((unsigned char)s[i] < 0x20)
            s[i] = ' ';
    return s;
}

static bool ensureDir(const string& dir)
{
    if (dir.empty() || Util::isDirExist(dir))
        return true;
    size_t pos = dir.find_last_of('/');
    if (pos != string::npos && pos > 0 && !ensureDir(dir.substr(0, pos)))
        return false;
    return mkdir(dir.c_str(), 0755) == 0 || Util::isDirExist(dir);
}

static void printEntries(const vector<PhraseEntry>& entries, int limit)
{
    if (limit <= 0 || limit > (int)entries.size())
        limit = (int)entries.size();
    for (int i = 0; i < limit; ++i)
        printf("%3d  %s\n", entries[i].priority, cleanKey(entries[i].key).c_str());
    if (limit < (int)entries.size())
        printf("... 共 %d 条，显示前 %d 条\n", (int)entries.size(), limit);
}

static void printStat(const string& path, const char* title)
{
    printf("%s: %s\n", title, path.c_str());
    if (!Util::isFileExist(path)) {
        printf("  文件不存在\n");
        return;
    }
    vector<PhraseEntry> entries;
    if (!readPhraseDb(path, entries)) {
        printf("  读取失败（格式错误或不可读）\n");
        return;
    }
    int low = 0, mid = 0, high = 0, top = 0;
    for (size_t i = 0; i < entries.size(); ++i) {
        int p = entries[i].priority;
        if (p <= 10) ++low;
        else if (p <= 20) ++mid;
        else if (p <= 50) ++high;
        else ++top;
    }
    printf("  条目: %d\n", (int)entries.size());
    printf("  优先级: <=10:%d  11-20:%d  21-50:%d  >50:%d\n",
           low, mid, high, top);
}

int cmdPhrase(int argc, char* argv[])
{
    AimPaths paths = aimPaths();
    string user, sys;
    dbPaths(paths, user, sys);

    string sub;
    string dbOpt;
    vector<string> rest;
    for (int i = 2; i < argc; ++i) {
        string a = argv[i];
        if (a == "--db" && i + 1 < argc) {
            dbOpt = argv[++i];
        } else if (sub.empty() && !a.empty() && a[0] != '-') {
            sub = a;
        } else {
            rest.push_back(a);
        }
    }

    if (sub.empty() || sub == "help") {
        printf("用法: aimctl phrase <stat|list|find|add|set|del|clean|export|save|backup> [参数]\n"
               "  --db <文件>  直接指定库文件\n"
               "  --system     使用系统词组库（默认用户库）\n"
               "  --all        合并用户库和系统库\n");
        return sub.empty() ? 1 : 0;
    }

    bool all = false, system = false;
    int limit = -1;
    int minPriority = -1;
    bool asc = false;
    bool preview = false;
    int below = 10;
    vector<string> args;
    for (size_t i = 0; i < rest.size(); ++i) {
        string a = rest[i];
        if (a == "--all" || a == "-a") all = true;
        else if (a == "--system" || a == "-s") system = true;
        else if (a == "-r") asc = true;
        else if (a == "-n" && i + 1 < rest.size()) preview = true, limit = atoi(rest[++i].c_str());
        else if (a == "--min" && i + 1 < rest.size()) minPriority = atoi(rest[++i].c_str());
        else if (a == "--below" && i + 1 < rest.size()) below = atoi(rest[++i].c_str());
        else if (a == "-d" || a == "--dry-run") preview = true;
        else args.push_back(a);
    }

    if (sub == "stat") {
        if (!dbOpt.empty()) {
            printStat(dbOpt, "词组库");
            return 0;
        }
        printStat(user, "用户库");
        printStat(sys, "系统库");
        return 0;
    }

    if (sub == "list") {
        vector<PhraseEntry> entries;
        if (!loadMerged(user, sys, all, system, dbOpt, entries))
            return 1;
        if (minPriority >= 0) {
            vector<PhraseEntry> tmp;
            for (size_t i = 0; i < entries.size(); ++i)
                if (entries[i].priority >= minPriority)
                    tmp.push_back(entries[i]);
            entries.swap(tmp);
        }
        sort(entries.begin(), entries.end(), asc ? byPriorityAsc : byPriority);
        printEntries(entries, limit < 0 ? 20 : limit);
        return 0;
    }

    if (sub == "find") {
        if (args.empty()) {
            printf("用法: aimctl phrase find <前缀> [--all]\n");
            return 1;
        }
        string q = args[0];
        vector<PhraseEntry> entries;
        if (!loadMerged(user, sys, all, system, dbOpt, entries))
            return 1;
        vector<PhraseEntry> hit;
        for (size_t i = 0; i < entries.size(); ++i) {
            const string& ph = entries[i].phrase.empty() ? entries[i].key : entries[i].phrase;
            if (ph.compare(0, q.size(), q) == 0 ||
                entries[i].key.compare(0, q.size(), q) == 0)
                hit.push_back(entries[i]);
        }
        sort(hit.begin(), hit.end(), byPriority);
        printf("以“%s”开头: %d 条\n", q.c_str(), (int)hit.size());
        printEntries(hit, limit < 0 ? 50 : limit);
        return 0;
    }

    if (sub == "add") {
        if (args.empty() || args.size() > 2) {
            printf("用法: aimctl phrase add <短语> [优先级]\n");
            return 1;
        }
        string phrase = args[0];
        int pri = args.size() > 1 ? atoi(args[1].c_str()) : BASE_PRIORITY;
        map<string, string> pinyin;
        if (!loadHanPinyinMap(paths.home + "/han-utf8.imdb", pinyin)) {
            printf("无法读取汉字库: %s/han-utf8.imdb\n", paths.home.c_str());
            return 1;
        }
        map<string, string>::iterator p = pinyin.find(firstHanChar(phrase));
        if (p == pinyin.end() || p->second.empty()) {
            printf("找不到首字拼音: %s\n", firstHanChar(phrase).c_str());
            return 1;
        }
        string key = p->second + PH_SEP + phrase;
        string db = dbOpt.empty() ? user : dbOpt;
        vector<PhraseEntry> entries;
        if (Util::isFileExist(db) && !readPhraseDb(db, entries)) {
            printf("无法读取词组库: %s\n", db.c_str());
            return 1;
        }
        bool found = false;
        for (size_t i = 0; i < entries.size(); ++i) {
            if (entries[i].key == key) {
                if (pri > entries[i].priority)
                    entries[i].priority = pri;
                found = true;
                break;
            }
        }
        if (!found) {
            PhraseEntry e;
            e.key = key;
            e.py = p->second;
            e.phrase = phrase;
            e.priority = pri;
            entries.push_back(e);
        }
        writeWarning();
        if (!writePhraseDb(db, entries)) {
            printf("写入失败: %s\n", db.c_str());
            return 1;
        }
        printf("%s: %s (%d)\n", found ? "已更新" : "已添加", key.c_str(), pri);
        return 0;
    }

    if (sub == "set" || sub == "del") {
        if (args.size() < (sub == "set" ? 2u : 1u)) {
            printf("用法: aimctl phrase %s <短语|key>%s\n",
                   sub.c_str(), sub == "set" ? " <优先级>" : "");
            return 1;
        }
        string q = args[0];
        int pri = sub == "set" ? atoi(args[1].c_str()) : 0;
        if (pri < 0) pri = 0;
        if (pri > MAX_PRIORITY) pri = MAX_PRIORITY;
        string db = dbOpt.empty() ? (system ? sys : user) : dbOpt;
        vector<PhraseEntry> entries;
        if (!readPhraseDb(db, entries)) {
            printf("无法读取词组库: %s\n", db.c_str());
            return 1;
        }
        vector<PhraseEntry> out;
        int hit = 0;
        for (size_t i = 0; i < entries.size(); ++i) {
            if (entries[i].key == q || entries[i].phrase == q) {
                ++hit;
                if (sub == "set") {
                    entries[i].priority = pri;
                    out.push_back(entries[i]);
                }
                continue;
            }
            out.push_back(entries[i]);
        }
        if (hit == 0) {
            printf("没有匹配: %s（可用 aimctl phrase find 查询）\n", q.c_str());
            return 1;
        }
        writeWarning();
        if (!writePhraseDb(db, out)) {
            printf("写入失败: %s\n", db.c_str());
            return 1;
        }
        printf("%s %d 条\n", sub == "set" ? "已调整" : "已删除", hit);
        return 0;
    }

    if (sub == "clean") {
        string db = dbOpt.empty() ? (system ? sys : user) : dbOpt;
        vector<PhraseEntry> entries;
        if (!readPhraseDb(db, entries)) {
            printf("无法读取词组库: %s\n", db.c_str());
            return 1;
        }
        vector<PhraseEntry> out;
        int removed = 0;
        int shown = 0;
        for (size_t i = 0; i < entries.size(); ++i) {
            if (entries[i].priority <= below) {
                if (preview && (limit <= 0 || shown < limit)) {
                    printf("  %3d  %s\n", entries[i].priority, entries[i].key.c_str());
                    ++shown;
                }
                ++removed;
            } else {
                out.push_back(entries[i]);
            }
        }
        printf("优先级 <= %d 的词组: %d 条（总 %d）\n",
               below, removed, (int)entries.size());
        if (preview || removed == 0)
            return 0;
        writeWarning();
        if (!writePhraseDb(db, out)) {
            printf("写入失败: %s\n", db.c_str());
            return 1;
        }
        printf("已清理，剩余 %d 条。\n", (int)out.size());
        return 0;
    }

    if (sub == "export") {
        if (args.empty()) {
            printf("用法: aimctl phrase export <文件>\n");
            return 1;
        }
        vector<PhraseEntry> entries;
        if (!loadMerged(user, sys, all, system, dbOpt, entries))
            return 1;
        sort(entries.begin(), entries.end(), byPriority);
        FILE* f = fopen(args[0].c_str(), "w");
        if (!f) {
            printf("无法写入: %s\n", args[0].c_str());
            return 1;
        }
        for (size_t i = 0; i < entries.size(); ++i)
            fprintf(f, "%s\t%d\n", cleanKey(entries[i].key).c_str(), entries[i].priority);
        fclose(f);
        printf("已导出 %d 条到 %s\n", (int)entries.size(), args[0].c_str());
        return 0;
    }

    if (sub == "save") {
        if (args.empty()) {
            printf("用法: aimctl phrase save <文件>\n");
            return 1;
        }
        vector<PhraseEntry> entries;
        if (!loadMerged(user, sys, all, system, dbOpt, entries))
            return 1;
        sort(entries.begin(), entries.end(), byPriority);
        if (!writePhraseDb(args[0], entries)) {
            printf("写入失败: %s\n", args[0].c_str());
            return 1;
        }
        printf("已保存 %d 条到 %s\n", (int)entries.size(), args[0].c_str());
        return 0;
    }

    if (sub == "backup") {
        string dir = args.empty() ? paths.data : args[0];
        string dest = dir + "/user_phrase-utf8.imdb";
        vector<PhraseEntry> entries;
        if (!readPhraseDb(dbOpt.empty() ? user : dbOpt, entries)) {
            printf("无法读取用户词组库。\n");
            return 1;
        }
        if (!ensureDir(dir)) {
            printf("无法创建目录: %s\n", dir.c_str());
            return 1;
        }
        if (!writePhraseDb(dest, entries)) {
            printf("备份失败: %s\n", dest.c_str());
            printf("系统目录需要 root 权限，可执行:\n"
                   "  sudo aimctl phrase backup%s\n",
                   args.empty() ? "" : (" " + args[0]).c_str());
            return 1;
        }
        printf("已备份 %d 条到 %s\n", (int)entries.size(), dest.c_str());
        return 0;
    }

    printf("未知子命令: %s\n", sub.c_str());
    return 1;
}

/* ------------------------------------------------------------------ */
/* pinyin lookup                                                       */
/* ------------------------------------------------------------------ */

int cmdPinyin(int argc, char* argv[])
{
    if (argc < 3) {
        printf("用法: aimctl pinyin <汉字>...\n");
        return 1;
    }

    AimPaths paths = aimPaths();
    vector<string> lines;
    bool ok;
    {
        QuietScope quiet;
        IndexTree tree;
        ok = tree.load(paths.home + "/han-utf8.imdb", IMDB_MAGIC, true);
        if (ok) {
            for (int a = 2; a < argc; ++a) {
                IndexList list;
                tree.getIndexList(list, argv[a], true, 0, -1);
                for (size_t i = 0; i < list.size(); ++i) {
                    HanItem h(list[i]->d.ptr);
                    char buf[256];
                    snprintf(buf, sizeof(buf), "%-6s %-14s %d",
                             list[i]->index.c_str(), h.py.c_str(), h.priority);
                    lines.push_back(buf);
                }
                IndexTree::freeItems(list);
            }
        }
    }

    if (!ok) {
        printf("无法读取汉字库: %s/han-utf8.imdb\n", paths.home.c_str());
        return 1;
    }
    if (lines.empty()) {
        printf("没有找到。\n");
        return 1;
    }
    printf("%-6s %-14s %s\n", "字", "拼音", "优先级");
    for (size_t i = 0; i < lines.size(); ++i)
        printf("%s\n", lines[i].c_str());
    return 0;
}

int main(int argc, char* argv[])
{
    int c;

#ifdef _LINUX
    setlocale(LC_ALL, "C.UTF-8");
#endif

    if (argc > 1 && argv[1][0] != '-') {
        string cmd = argv[1];
        if (cmd == "info")
            return cmdInfo(argc, argv);
        if (cmd == "check")
            return cmdCheck(argc, argv);
        if (cmd == "make")
            return cmdMake(argc, argv);
        if (cmd == "pinyin")
            return cmdPinyin(argc, argv);
        if (cmd == "phrase")
            return cmdPhrase(argc, argv);
        if (cmd == "help") {
            usage();
            return 0;
        }
        printf("unknown command: %s\n", cmd.c_str());
        usage();
        return 1;
    }

    while (( c = getopt(argc, argv, "hv")) != -1) {
        switch (c) {
        case 'h':
            usage();
            return 0;

        case 'v':
            printf("version: %s (aimctl %s)\n", VERSION, AIMCTL_VERSION);
            return 0;
        }

        usage();
        return 0;
    }

    usage();
    return 0;
}

static void make_pinyinDB(const string& pinyinPath, const string& dbPath)
{
    PinyinTDFParser pyParser;
    Util::getTimeMS();

    pyParser.parser(pinyinPath);
    pyParser.write(dbPath);

    printf("write db file, done: \n");
    printf("    entries: %u\n", pyParser.getTotalEntry());
    printf("    costs: (%u)s:(%u)ms\n", Util::getTimeMS()/1000, Util::getTimeMS()%1000);
}

static void make_hanDB(const string& hanPath, const string& dbPath)
{
    HanTDFParser hanParser;
    Util::getTimeMS();

    hanParser.parser(hanPath);
    hanParser.write(dbPath);

    printf("write db file, done: \n");
    printf("    entries: %u\n", hanParser.getTotalEntry());
    printf("    costs: (%u)s:(%u)ms\n", Util::getTimeMS()/1000, Util::getTimeMS()%1000);
}

static void make_phDB(const string& path, const string& dbPath)
{
    PhTDFParser phParser;
    Util::getTimeMS();

    phParser.parser(path);
    phParser.write(dbPath);

    printf("write db file, done: \n");
    printf("    entries: %u\n", phParser.getTotalEntry());
    printf("    costs: (%u)s:(%u)ms\n", Util::getTimeMS()/1000, Util::getTimeMS()%1000);
}
