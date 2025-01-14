#pragma once

#include <systemd/sd-journal.h>
#include <map>
#include <string>
#include <vector>
#include <memory>

struct JournalEntry {
    std::map<std::string, std::string> fields;
    uint64_t timestamp;
};

class SystemdJournalMock {
public:
    static void SetNextError(int error) {
        s_nextError = error;
    }

    static void Reset() {
        s_entries.clear();
        s_currentIndex = 0;
        s_nextError = 0;
        s_matches.clear();
    }

    static void AddEntry(const JournalEntry& entry) {
        s_entries.push_back(entry);
    }

    static int GetMatchCount() {
        return s_matches.size();
    }

    static const std::vector<std::string>& GetMatches() {
        return s_matches;
    }

    static size_t GetCurrentIndex() {
        return s_currentIndex;
    }

private:
    static std::vector<JournalEntry> s_entries;
    static size_t s_currentIndex;
    static int s_nextError;
    static std::vector<std::string> s_matches;

    friend int sd_journal_open(sd_journal**, int);
    friend int sd_journal_next(sd_journal*);
    friend int sd_journal_previous(sd_journal*);
    friend int sd_journal_seek_head(sd_journal*);
    friend int sd_journal_seek_tail(sd_journal*);
    friend int sd_journal_get_data(sd_journal*, const char*, const void**, size_t*);
    friend int sd_journal_get_realtime_usec(sd_journal*, uint64_t*);
    friend int sd_journal_add_match(sd_journal*, const char*, size_t);
    friend void sd_journal_flush_matches(sd_journal*);
};
