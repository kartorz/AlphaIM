#include "UsrPhrase.h"
#include "CharUtil.h"

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
	printf("flush sentencs %s\n",  m_sentence.c_str());
    if (m_sentence.size() >= MIN_HAN_U8BYTES * 2)
        m_phrases.insert(m_sentence);
    phrases.insert(phrases.end(), m_phrases.begin(), m_phrases.end());
    clear();
}

void UsrPhrase::clear()
{
    m_sentence.clear();
    m_phrases.clear();
}

void UsrPhrase::trackUsrInput(const string& text)
{
    string word;
    const char* cursor = text.c_str();
    const char* end = cursor + text.size();
    while (*cursor) {
        const char* start = cursor;
        unsigned char lead = *cursor;
        int width = lead < 0x80 ? 1 : lead >= 0xC2 && lead <= 0xDF ? 2 :
            lead >= 0xE0 && lead <= 0xEF ? 3 : lead >= 0xF0 && lead <= 0xF4 ? 4 : 0;
        u32 character = 0;
        if (width && end - cursor >= width)
            character = CharUtil::utf8byteToUCS4Char(&cursor);
        else
            ++cursor;
        if (isHan(character)) {
            word.append(start, cursor - start);
            m_sentence.append(start, cursor - start);
        }
        if (!isHan(character) || m_sentence.size() >= USRPH_MAX_SIZE) {
            if (word.size() >= MIN_HAN_U8BYTES * 2)
                m_phrases.insert(word);
            word.clear();
        }
    }
    if (word.size() >= MIN_HAN_U8BYTES * 2)
        m_phrases.insert(word);
}
