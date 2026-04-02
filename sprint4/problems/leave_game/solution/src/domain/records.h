#pragma once


namespace model {

struct Record {
    std::string name;
    int score;
    double play_time;
};

using Records = std::vector<Record>;

class RecordRepositoryI {
public:
    virtual const Records GetRecords(int start, int max_items) const = 0;
    virtual void SaveRecord(std::string name, int score, double play_time_seconds) = 0;

    virtual ~RecordRepositoryI() = default;
protected:
};



} // namespace model {