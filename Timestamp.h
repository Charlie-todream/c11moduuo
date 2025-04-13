#pragma once
#include <iostream>
#include <string>

class TimeStamp
{
public:
    TimeStamp();
    explicit TimeStamp(int64_t microSecondsSinceEpoch_);
    static TimeStamp now();
    std::string toString() const;

private:
    int64_t microSecondsSinceEpoch_;
};