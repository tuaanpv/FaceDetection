
#include    "externalFuntion.h"

using namespace std;

#define MAX_COUNT	1000000000

char* smsNumber	=	"0912798807";
string SMSName  = 	"PT";
char* devUSB	=	"/dev/ttyUSB0";
int   hour_to_report = 22;
int   min_to_report = 00;
int   NewPeopleReport = 1;

const string  main_path				= "PT/";
const string  guests    			= "guests_";
const string  smsconfig				= "sms.conf";

string daily_guests_counter_file;


int main(int argc, char* argv[])
{
	cout << "This is external function" << endl;
	
	long count;
	int old_people_count = 0;
	int daily_report = 0, old_iday = 0;

	vector<string> labels;
	vector<string> parameter;
	read_file(smsconfig, labels, parameter);

	for(size_t i = 0; i < labels.size(); ++i)
	{
		cout << labels[i] << ": " << parameter[i] << endl;
		if(labels[i] == "SMSNumber")
		{
			//strcpy(smsNumber, parameter[i].c_str());
			smsNumber = new char[parameter[i].size() + 1];
    		memcpy(smsNumber, parameter[i].c_str(), parameter[i].size() + 1);
		}
		else if(labels[i] == "SMSName")
		{
			SMSName = parameter[i];
		}
		else if(labels[i] == "devUSB")
		{
			devUSB = new char[parameter[i].size() + 1];
    		memcpy(devUSB, parameter[i].c_str(), parameter[i].size() + 1);
		}
		else if(labels[i] == "HourReport")
		{
			hour_to_report = stoi(parameter[i]);
		}
		else if(labels[i] == "NewPeopleReport")
		{
			NewPeopleReport = stoi(parameter[i]);
		}
	}

	//cout << "Config from file: " << smsNumber << " | " << devUSB << endl;
	init();

	while(1)
	{
		//string tempText;
		//std::vector<string>	smsText;
		//int people_count = 0;

		count++;
		if(count >= MAX_COUNT)
		{
			time_t now = get_date_time();
			int people_count = 0;

		    daily_guests_counter_file = main_path + guests + syear + smonth + sday + ".pt";
		    ifstream dailyGuest((daily_guests_counter_file).c_str());
		    dailyGuest >> people_count;
		    dailyGuest.close();
			
		    cout << "Number of sec since January 1,1970: " << now << endl;
		    cout << "Date: " << iday << "/" << imonth << "/"  << iyear << endl;
		    cout << "Time: "<< ihour << ":" << imin << ":" << isec << endl;
		    cout << CGREEN << "Total people: " << people_count << CRESET << endl;

			if(NewPeopleReport && (old_people_count != people_count))
			{
				std::vector<string>	smsText;
				int new_people = people_count - old_people_count;
				smsText.push_back("Dear " + SMSName);
				smsText.push_back("\nNew guest appeared: " + to_string(new_people));
				smsText.push_back("\nTotal guest: " + to_string(people_count));
				smsText.push_back("\n" + sday + "/" + smonth + "/" + syear + " - " + shour + ":" + smin + ":" + ssec + "\n");
				cout << "Sending SMS to: " << SMSName << "(" << smsNumber << ")" << endl;
				sendSMS(devUSB, smsNumber, smsText);
		    
		    	old_people_count = people_count;
			}

			//cout << "Break out it!" << endl;

			if((ihour >= hour_to_report) && (!daily_report) && (old_iday != iday))
			{
				std::vector<string>	smsText;
				smsText.push_back("Dear " + SMSName);
				smsText.push_back("\nTotal gest today: " + to_string(people_count));
				smsText.push_back("\n" + sday + "/" + smonth + "/" + syear + " - " + shour + ":" + smin + ":" + ssec);

				cout << "Sending SMS to: " << SMSName << "(" << smsNumber << ")" << endl;
				sendSMS(devUSB, smsNumber, smsText);

				daily_report = 1;
				old_iday = iday;
			}
			else if((ihour < hour_to_report) && daily_report && (old_iday != iday))
			{
				daily_report = 0;
			}

			count = 0;
		}
	}

	free(smsNumber);
	free(devUSB);
	return 1;
}

void init()
{
	std::vector<string>	smsText;

	smsText.push_back("Face counting \n");
	smsText.push_back("System starting up \n");
	smsText.push_back("@PT: +84943999344");
	smsText.push_back("@PT: tuaanpv@gmail.com");

	sendSMS(devUSB, smsNumber, smsText);
}

time_t get_date_time()
{
    time_t now = time(0);

    //cout << "Number of sec since January 1,1970:" << now << endl;
    tm *ltm = localtime(&now);
    //cout << "Date: " << 1 + ltm->tm_mday << "/" << 1 + ltm->tm_mon << "/"  << 1900 + ltm->tm_year << endl;
    //cout << "Time: "<< 1 + ltm->tm_hour << ":" << 1 + ltm->tm_min << ":" << 1 + ltm->tm_sec << endl;
    iday    = ltm->tm_mday;
    sday    = to_string(iday);
    imonth  = 1 + ltm->tm_mon;
    smonth  = to_string(imonth);
    iyear   = 1900 + ltm->tm_year;
    syear   = to_string(iyear);
    ihour   = ltm->tm_hour;
    shour   = to_string(ihour);
    imin    = ltm->tm_min;
    smin    = to_string(imin);
    isec    = ltm->tm_sec;
    ssec    = to_string(isec);

    return now;
}

static void read_file(const string& filename, vector<string>& labels, vector<string>& parameter)
{
	char separator = ':';

    std::ifstream file(filename.c_str(), ifstream::in);
    if (!file)
    {
        cout << "No valid input file was given, please check the given filename." << endl;
        //return 1;
    }

    string line, path, classlabel;
    while (getline(file, line))
    {
        stringstream liness(line);
        getline(liness, path, separator);
        getline(liness, classlabel);
        if(!path.empty() && !classlabel.empty()) 
        {
            labels.push_back(path);
            parameter.push_back(classlabel);
        }
    }
}