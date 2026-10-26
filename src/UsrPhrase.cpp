#include "UsrPhrase.h"
#include "CharUtil.h"

#include <map>

static bool isHan(u32 character)
{
    return (character >= 0x3400 && character <= 0x4DBF) ||
        (character >= 0x4E00 && character <= 0x9FFF) ||
        (character >= 0xF900 && character <= 0xFAFF) ||
        (character >= 0x20000 && character <= 0x2FA1F) ||
        (character >= 0x30000 && character <= 0x323AF);
}

void UsrPhrase::flush(vector<string>& phrases)
{
    if (m_sentence.size() > 320) {
        parseUsrInput(m_sentence);
        m_sentence.clear();
    }
    phrases.insert(phrases.end(), m_phrases.begin(), m_phrases.end());
    clear();
}

void UsrPhrase::clear()
{
    m_phrases.clear();
}

void UsrPhrase::parseUsrInput(const string& text)
{
    struct HanChar {
        string text;
    };

    map<string, int> occurrences;
    vector<HanChar> run;
    const auto collectRun = [&]() {
        for (size_t begin = 0; begin + 1 < run.size(); ++begin) {
            string phrase;
            for (size_t finish = begin; finish < run.size(); ++finish) {
                phrase += run[finish].text;
                if (finish - begin + 1 >= 2 && phrase.size() < USRPH_MAX_SIZE)
                    ++occurrences[phrase];
            }
        }
        run.clear();
    };
    const char* cursor = text.c_str();
    const char* end = cursor + text.size();

    while (cursor < end) {
        const char* start = cursor;
        unsigned char lead = static_cast<unsigned char>(*cursor);
        int width = lead < 0x80 ? 1 :
            lead >= 0xC2 && lead <= 0xDF ? 2 :
            lead >= 0xE0 && lead <= 0xEF ? 3 :
            lead >= 0xF0 && lead <= 0xF4 ? 4 : 0;

        if (width == 0 || end - cursor < width) {
            collectRun();
            ++cursor;
            continue;
        }

        bool valid = true;
        for (int i = 1; i < width; ++i) {
            unsigned char continuation = static_cast<unsigned char>(cursor[i]);
            if (continuation < 0x80 || continuation > 0xBF) {
                valid = false;
                break;
            }
        }
        if (!valid) {
            collectRun();
            ++cursor;
            continue;
        }

        u32 character = CharUtil::utf8byteToUCS4Char(&cursor);
        if (!isHan(character)) {
            collectRun();
            continue;
        }
        run.push_back({string(start, cursor - start)});
    }
    collectRun();

    for (map<string, int>::const_iterator it = occurrences.begin();
         it != occurrences.end(); ++it) {
        if (it->second >= 2)
            m_phrases.insert(it->first);
    }
}

void UsrPhrase::trackUsrInput(const string& word)
{
    if (word.size() >= MIN_HAN_U8BYTES * 2 &&
        word.size() <= USRPH_MAX_SIZE)
            m_phrases.insert(word);

    m_sentence += word;
}
