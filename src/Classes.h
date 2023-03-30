#ifndef CLASSES_H
#define CLASSES_H
#include <iostream>
#include <string>
#include <list>

class AllowedTime
{
public:
    AllowedTime();
    AllowedTime(std::string StartTime, std::string EndTime); // parameterized constructor
    ~AllowedTime();                                          // destructor
    bool WithInTime(std::string CurrTime) const;             // checks if we are within time
    std::string GetStartTime();
    std::string GetEndTime();
    bool operator==(const AllowedTime &other) const; // compares two AllowedTime objects

private:
    // we save the time in minuets from 00:00:00
    int Start_time;
    std::string StartTime_string;

    int End_time;
    std::string EndTime_string;
};

class AllowedID
{
public:
    AllowedID();
    AllowedID(const std::string &AllowedID, const std::string &start_time, const std::string &end_time);
    ~AllowedID();

    void addAllowedTime(const std::string &start_time, const std::string &end_time);
    bool isTimeAllowed(const std::string time_str) const; // function to check if a specific time is allowed in any of the AllowedTime objects in the list
    bool operator==(const AllowedID &other) const;        // compares two AllowedID objects
    AllowedID &operator=(const AllowedID &other);

private:
    std::string AllowedID_str;
    std::list<AllowedTime> allowed_times_list;
};

#endif // CLASSES_H