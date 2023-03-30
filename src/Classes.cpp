#include "Classes.h"

/*
Allowed time class methods
*/
AllowedTime::AllowedTime() {}
AllowedTime::AllowedTime(std::string ST, std::string ET)
{
    this->StartTime_string = ST;
    this->EndTime_string = ET;

    int Start_Hour = std::stoi(ST.substr(0, 2));
    int Start_Min = std::stoi(ST.substr(3, 2));
    int End_Hour = std::stoi(ET.substr(0, 2));
    int End_Min = std::stoi(ET.substr(3, 2));

    this->Start_time = Start_Hour * 60 + Start_Min;
    this->End_time = End_Hour * 60 + End_Min;
}

AllowedTime::~AllowedTime()
{
}

// destructor
bool AllowedTime::WithInTime(std::string CurrTime) const
{
    int hour = std::stoi(CurrTime.substr(0, 2));
    int min = std::stoi(CurrTime.substr(3, 2));
    int time = hour * 60 + min;

    if (Start_time < time < End_time)
    {
        return true;
    }
    else
    {
        return false;
    }
}

std::string AllowedTime::GetStartTime()
{
    return StartTime_string;
}

std::string AllowedTime::GetEndTime()
{
    return EndTime_string;
}

bool AllowedTime::operator==(const AllowedTime &other) const
{
    return (Start_time == other.Start_time && End_time == other.End_time);
}

/*
Allowed time class methods
*/
AllowedID::AllowedID() {}

// Constructor takes a string argument for the ID and initializes the AllowedID_str member variable
AllowedID::AllowedID(const std::string &id, const std::string &start_time, const std::string &end_time) : AllowedID_str(id)
{
    // Create an AllowedTime object with the given start and end times
    AllowedTime allowed_time(start_time, end_time);
    // Add the AllowedTime object to the back of the allowed_times_list using the push_back function
    allowed_times_list.push_back(allowed_time);
}

AllowedID::~AllowedID()
{
}
// Function to add an AllowedTime object to the allowed_times_list
// Takes two string arguments, start_time and end_time, which are used to create the AllowedTime object
void AllowedID::addAllowedTime(const std::string &start_time, const std::string &end_time)
{
    // Create an AllowedTime object with the given start and end times
    AllowedTime allowed_time(start_time, end_time);
    // Add the AllowedTime object to the back of the allowed_times_list using the push_back function
    allowed_times_list.push_back(allowed_time);
}

// Function to check if a given time string is allowed in any of the AllowedTime objects in the allowed_times_list
// Takes a string argument, time_str, for the time to be checked
bool AllowedID::isTimeAllowed(const std::string time_str) const
{
    // Iterate through the allowed_times_list using a range-based for loop
    for (const AllowedTime &allowed_time : allowed_times_list)
    {
        // Check if the time is allowed in the current AllowedTime object
        if (allowed_time.WithInTime(time_str))
        {
            return true; // If the time is allowed, return true
        }
    }
    return false; // If the time is not allowed in any of the AllowedTime objects, return false
}

// This method compares the current ID object with another ID object to see if they are identical or not.
bool AllowedID::operator==(const AllowedID &other) const
{
    // Compare the ID strings
    if (AllowedID_str != other.AllowedID_str)
    {
        return false;
    }
    // Compare the number of AllowedTime objects
    if (allowed_times_list.size() != other.allowed_times_list.size())
    {
        return false;
    }
    // Compare each AllowedTime object
    auto it1 = allowed_times_list.begin();
    auto it2 = other.allowed_times_list.begin();
    for (; it1 != allowed_times_list.end(); ++it1, ++it2)
    {
        if (!(*it1 == *it2))
        {
            return false;
        }
    }
    // All checks passed, the ID objects are identical
    return true;
}

// This method assigns the values of another ID object to the current ID object.
AllowedID &AllowedID::operator=(const AllowedID &other)
{
    // check for self-assignment
    if (this != &other)
    {
        // Assign the ID string and the list of AllowedTime objects
        AllowedID_str = other.AllowedID_str;
        allowed_times_list = other.allowed_times_list;
    }
    return *this;
}