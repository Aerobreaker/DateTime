#include <iostream>

#include "DateTime.h"
#include <filesystem>

using namespace std;
using namespace datetime;

int main() {
	const std::string appdatadir = std::getenv("APPDATA");
	const std::string settingdir = (appdatadir == "" ? std::filesystem::temp_directory_path().string() : appdatadir) + "\\DateTime";
	const std::string tz_dir = settingdir + "\\timezones";
	if (!std::filesystem::exists(settingdir)) {
		std::filesystem::create_directories(settingdir);
	}
	date::set_install(tz_dir);

	time_point_seconds now = date::floor<seconds>(system_clock::now());

	cout << "Current time is:" << endl << now << endl << endl;

	DateTime tmp{ now, false };
	
	cout << "Date: " << tmp.GetDate() << endl;
	cout << "Time: " << tmp.GetTime() << endl;
	cout << "Year: " << tmp.GetYear() << endl;
	cout << "Month: " << tmp.GetMonth() << endl;
	cout << "Day: " << tmp.GetDay() << endl;
	cout << "Hour: " << tmp.GetHour() << endl;
	cout << "Minute: " << tmp.GetMinute() << endl;
	cout << "Second: " << tmp.GetSecond() << endl;
	cout << endl;

	cout << "Formatted output:" << endl;
	cout << "  12 hour:" << endl;
	cout << "    " << tmp << endl;
	tmp.toggle_24h();
	cout << "  24 hour:" << endl;
	cout << "    " << tmp << endl;
	cout << endl;
	tmp.make_24h(false);

	auto tmp2 = tmp;
	tmp2.set_timezone("EDT");

	cout << "Time in time zone " << tmp2.get_timezone() << " is:" << endl;
	cout << "  " << tmp2 << "(" << tmp2.get_offset_from(tmp) << ")" << endl;
	cout << endl;

	tmp2 += days(3);
	tmp2.set_timezone(tmp.get_timezone());
	
	cout << "Time adjusted 3 days (no timezone):" << endl;
	cout << "  " << tmp2 << endl;
	cout << "Offset is:" << endl;
	cout << "  " << tmp2.get_offset_from(tmp) << endl;

	tmp2.set_timezone("us/eastern");
	tmp2 += hours(4) + minutes(13) + seconds(5);

	cout << "Time adjusted 4 hours, 13 minutes, 5 seconds:" << endl;
	cout << "  " << tmp2 << endl;
	cout << "Total offset is:" << endl;
	cout << "  " << tmp2.get_offset_from(tmp) << endl;

	tmp2 -= days(4);

	cout << "Time adjusted 4 days:" << endl;
	cout << "  " << tmp2 << endl;
	cout << "New offset is:" << endl;
	cout << "  " << tmp2.get_offset_from(tmp);
	cout << endl;

	cout << endl;

	cout << "End" << endl;
}
