#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <string>
#include <list>
#include <functional>
#include <ctime>
#include <queue>
#include <numeric>


using namespace std;

string timeTtoTimeStamp(time_t t)
{
    char buffer [80];
    struct tm *tm_info = localtime (&t);
    strftime(buffer, 80, "%d/%b/%Y:%H:%M:%S %z", tm_info);
    return string(buffer);
}

time_t timeStampToTimeT(string timeStamp)
{
    struct tm tm;
    strptime(timeStamp.c_str(), "%d/%b/%Y:%H:%M:%S %z", &tm);
    time_t  timet = mktime(&tm);
    return timet;
}

bool pairComparator(pair<string,long long int> p1, pair<string,long long int> p2)
{
    return p1.second > p2.second;
}

std::map<string, long long int> feature1;
void feature1_input(vector<string> tokens) {
    string IP = tokens[0];

    if (feature1.find(IP) != feature1.end()) {
        feature1[IP] = feature1[IP] + 1;
    } else {
        feature1.insert(make_pair(IP, 1)); 
    }
}

std::map<string, long long int> feature2;
void feature2_input(vector<string> tokens) {

    string resource = tokens[6];
    long long int bytes;

    try{
        if (tokens[9] == "-")
            bytes = 0;
        else 
            bytes = stoll(tokens[9]);
    }
    catch(std::invalid_argument& e)
    {
    }

    catch(std::out_of_range& e){
        // if the converted value would fall out of the range of the result type 
        // or if the underlying function (std::strtol or std::strtoull) sets errno 
        // to ERANGE.
    }

    catch(...) {
        // everything else
    }

    if (feature2.find(resource) != feature2.end()) {
        feature2[resource] = feature2[resource] + bytes;
    } else {
        feature2.insert(make_pair(resource, bytes));
    }
}

string getTimeStamp(string token3, string token4)
{
    string date = token3;
    date = date.substr(1);
    string timezone = token4;
    timezone = timezone.substr(0,5);

    string timeStamp = date + " " + timezone;
    return timeStamp;
}

vector<time_t> feature3_timeStamp;
vector<string> feature3_timeStampString;

void feature3_input(vector<string> tokens) {
    string curr_time_string = getTimeStamp(tokens[3], tokens[4]);
    feature3_timeStamp.push_back(timeStampToTimeT(curr_time_string));
    feature3_timeStampString.push_back(curr_time_string);
}

struct feature3_struct
{
    time_t timeStamp;
    string timeStampString;
    unsigned int cnt;
    unsigned int hourcnt;
};

bool hourcntComparator(feature3_struct s1, feature3_struct s2)
{
    return s1.hourcnt > s2.hourcnt;
}

void feature3_process() {

    fstream outFile;
    string outputFileName = "./log_output/hours.txt";

    int length = feature3_timeStamp.size();

    if (length == 1)
    {
        //handle this.
    }

    std::vector<time_t> :: iterator it = feature3_timeStamp.begin();

    time_t begin_timeStamp = *it;
    time_t end_timeStamp = *(it + (length - 1));
    unsigned long int noOfSeconds = (end_timeStamp - begin_timeStamp + 1);

    vector<feature3_struct> feature3 (noOfSeconds);

    for (vector<feature3_struct> :: iterator it = feature3.begin();
                                             it != feature3.end();
                                             ++it )
    {
        it->timeStampString = "";
        it->timeStamp = 0;
        it->cnt = 0;
        it->hourcnt = 0;
    }

    begin_timeStamp = feature3_timeStamp[0];
    for (int i = 0;i < feature3_timeStamp.size();i++)
    {
        time_t t = feature3_timeStamp[i];
        string tString = feature3_timeStampString[i];

        time_t offset = t - begin_timeStamp;
        feature3[offset].cnt++;
        feature3[offset].timeStamp = t;
        feature3[offset].timeStampString = tString;
    }

    unsigned long int sum = 0;
    for (int i = 0; i < noOfSeconds; ++i)
    {
        if (i >= 3600) {
            feature3[i - 3600].hourcnt = sum;
            feature3[i - 3600].timeStamp = begin_timeStamp + (i - 3600);

            sum -= feature3[i - 3600].cnt;
            sum += feature3[i].cnt;
        } else {
            sum += feature3[i].cnt;
        }
    }

    int remaining = noOfSeconds % 3600;

    sum = 0;
    for (int i = remaining - 1; i >= 0; --i)
    {
        sum += feature3[i].cnt;
        feature3[i].hourcnt = sum;
        feature3[i].timeStamp = begin_timeStamp + i;
    }
    
    outFile.open(outputFileName, ios::out);

    if (!outFile.is_open()) {
        cout << "Could not create file:" << outputFileName << endl;
        return;
    }

    std::sort(feature3.begin(), feature3.end(), hourcntComparator); 

    for (int i = 0; i < 10; ++i)
    {
        outFile << timeTtoTimeStamp(feature3[i].timeStamp) << "," << feature3[i].hourcnt << endl; 
    }

}

struct inputData
{
    unsigned int hostIndex;
    string timeStamp;
    string request;
    string replyCode;
    string bytes;
};

std::vector< pair<string, inputData >> feature4;

void feature4_input(vector<string> tokens) {

    inputData i;

    i.timeStamp = getTimeStamp(tokens[3], tokens[4]);
    i.request = tokens[5] + " " + tokens[6] + " " + tokens[7];
    i.replyCode = tokens[8];
    i.bytes = tokens[9];

    feature4.push_back(make_pair(tokens[0],i)); 
}


bool hostComparator(pair<string, inputData> p1, pair<string, inputData> p2)
{
    return p1.first < p2.first; 
}

bool timeStampComparator(pair<string, inputData> p1, pair<string, inputData> p2)
{
    return timeStampToTimeT(p1.second.timeStamp) < timeStampToTimeT(p2.second.timeStamp);
}

// detect three failed login attempts for a given IP
std::vector< pair<string, inputData >> :: iterator detectThreeFaiedLoginAttempts(unsigned int currHostNum, 
                                                          std::vector< pair<string, inputData >> :: iterator it)
{
    time_t prev_time = timeStampToTimeT(it->second.timeStamp);

    std::vector< pair<string, inputData >> :: iterator startBlockIter = feature4.end();

    // We need to sort each block wrt timeStamp
    std::vector< pair<string, inputData >> :: iterator end_it = it;
    while (end_it -> second.hostIndex == currHostNum)
    {
        ++end_it;
    }
    std::sort(it, end_it, timeStampComparator);
    int cnt =0;

    for (std::vector< pair<string, inputData >> :: iterator myit = it; myit->second.hostIndex == currHostNum && (stoi(myit->second.replyCode) == 401) ; ++myit)
    {
        
        time_t curr_time = timeStampToTimeT(myit->second.timeStamp);
        time_t diff_time = 0;

        cnt++;

        diff_time = diff_time + difftime(prev_time, curr_time);

        prev_time = curr_time;

        if ((diff_time <= 20) && cnt == 3) 
        {
            return myit; // return the 3rd attempt iterator
        }
    }

    return feature4.end(); // didn't find any 3 consecutive attempts
}
    
void blockAccessForNext5Mins(unsigned int currHostNum,
                            std::vector< pair<string, inputData >> :: iterator startBlockIter)
{
    fstream outFile;
    string outputFileName = "./log_output/blocked.txt";

    if ((startBlockIter + 1) == feature4.end()) {
        // if there are no lines after 3 failed attemps, we cannot block anything.
        return;
    }

    outFile.open(outputFileName, ios::out);

    if (!outFile.is_open()) {
        cout << "Could not create file:" << outputFileName << endl;
        return;
    }

    // We need to sort each block wrt timeStamp
    std::vector< pair<string, inputData >> :: iterator end_it = (startBlockIter + 1);
    while (end_it -> second.hostIndex == currHostNum)
    {
        ++end_it;
    }
    std::sort(startBlockIter+1, end_it, timeStampComparator);

    time_t prev_time = timeStampToTimeT((startBlockIter + 1) ->second.timeStamp);

    for (std::vector< pair<string, inputData >> :: iterator myit = startBlockIter + 1; // start from the next line
                                                        myit->second.hostIndex == currHostNum; ++myit) {

        time_t curr_time = timeStampToTimeT(myit->second.timeStamp);
        time_t diff_time = 0;
        diff_time = diff_time + difftime(prev_time, curr_time);

        prev_time = curr_time;

        if ((diff_time <= 300))
        {
            // block this entry
            // Write into blocked.txt

            outFile << myit-> first << " ";
            outFile << "-";
            outFile << " ";
            outFile << "-";
            outFile << " ";
            outFile << "[";
            outFile << myit -> second.timeStamp;
            outFile << "]";
            outFile << " ";
            outFile << myit -> second.request << " ";
            outFile << myit -> second.replyCode << " ";
            outFile << myit -> second.bytes << endl;
            // return the final line of this blocked IP to the caller
            // so that the caller can do the next round of check starting from this point.
        }

       // if (diff_time > 300)
       //     return myit + 1; // return the next iterator.
        
    }
    outFile.close();
    
    return;
}

void detectIllegalLogin()
{
    unsigned int prevHostNum = 0;
    for (std::vector< pair<string, inputData >> :: iterator it = feature4.begin(); it != feature4.end(); ++it)
    {
        unsigned int currHostNum = (it)->second.hostIndex;
        
        std::vector< pair<string, inputData >> :: iterator startBlockIter;

        if (prevHostNum != currHostNum) {
            //cout << "currHostNum is " << currHostNum << endl;
            startBlockIter = detectThreeFaiedLoginAttempts(currHostNum, it);
            if (startBlockIter == feature4.end())
            {
                continue;
            }
            blockAccessForNext5Mins(currHostNum, startBlockIter);
        }
        prevHostNum = currHostNum;
    }
}

void feature4_process() {

    cout << "bs" << endl;
    std::sort(feature4.begin(), feature4.end(), hostComparator);
    cout << "as" << endl;

    string prevHost = "";
    int cnt = 0;
    for (std::vector< pair<string, inputData >> :: iterator it = feature4.begin(); it != feature4.end() ; ++it)
    {
        string curHost = it -> first;

        if (curHost != prevHost) {
            it->second.hostIndex = ++cnt;
        } else {
            it->second.hostIndex = cnt;
        }
        prevHost = curHost;
    }
    // for each host, detect illegal login

    detectIllegalLogin();
}

void feature1_process() {
    vector<pair<string,long long int>> vectorOfPairs;
    fstream outFile;
    string outputFileName = "./log_output/hosts.txt";
    
    outFile.open(outputFileName, ios::out);

    if (!outFile.is_open()) {
        cout << "Could not create file:" << outputFileName << endl;
        return;
    }

    for (map<string, long long int> :: iterator it1 = feature1.begin(); it1 != feature1.end() ; ++it1) 
    {
        vectorOfPairs.push_back(make_pair(it1->first, it1->second));
    }
    std::sort(vectorOfPairs.begin(), vectorOfPairs.end(), pairComparator);

    for (vector<pair<string,long long int>> :: iterator it2 = vectorOfPairs.begin(); it2 != vectorOfPairs.end() ; ++it2) 
    {
        outFile << it2->first << "," << it2->second << endl;
    }
    outFile.close();
}

void feature2_process() {
    vector<pair<string,long long int>> vectorOfPairs;
    fstream outFile;
    string outputFileName = "./log_output/resources.txt";

    outFile.open(outputFileName, ios::out);

    if (!outFile.is_open()) {
        cout << "Could not create file:" << outputFileName << endl;
        return;
    }

    for (map<string, long long int> :: iterator it1 = feature2.begin(); it1 != feature2.end() ; ++it1) 
    {
        vectorOfPairs.push_back(make_pair(it1->first, it1->second));
    }

    std::sort(vectorOfPairs.begin(), vectorOfPairs.end(), pairComparator);
    
    for (vector<pair<string,long long int>> :: iterator it2 = vectorOfPairs.begin(); it2 != vectorOfPairs.end() ; ++it2) 
    {
        outFile << it2->first << endl;
    }
    outFile.close();
}

int main(int argc, const char * argv[]) {
    string inFileName = argv[1];
    ifstream inFile;

    setenv("TZ", "America/Aruba", 1);
    tzset();

    inFile.open(inFileName);

    if (!inFile.is_open()) {
        cout << "Cannot open file: " << inFileName << endl;
    }
    string line;
    int cnt = 0;
    while (getline(inFile, line)) {
        istringstream iss(line);

        vector<string> tokens;
        copy(istream_iterator<string>(iss),
                        istream_iterator<string>(),
                        back_inserter(tokens));
        if (tokens.size() != 10)
            continue;    
/*
       feature1_input(tokens);
       feature2_input(tokens);
       feature3_input(tokens);
*/
       feature4_input(tokens);

    }
    inFile.close();
    feature1_process();
    cout << "Feature 1 OVER " << endl;
    /*
    feature2_process();
    cout << "Feature 2 OVER " << endl;
    feature3_process();
    cout << "Feature 3 OVER " << endl;
    */
    feature4_process();
    cout << "Feature 4 OVER " << endl;

    return 0;
}
