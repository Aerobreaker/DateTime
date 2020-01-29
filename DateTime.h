#pragma once

#include "date/tz.h"
#include <filesystem>

/*
Include local copies of the source for date.h and tz.h for 2 reasons:
1. The copy of tz.h installed via vcpkg isn't compiled with auto download, and there doesn't seem to be a way to change this
2. The standard copy has problems with changing the install directory to something which does not end in "tzdata" when auto installing

Using the following commands from the developer command prompt to download local copies of the source and place the
  relevant files into the local directory:
	git clone https://github.com/HowardHinnant/date
	move date\src\tz.cpp tz.cpp
	move date\include\date date2
	rd /s /q date
	move date2 date

Lines 3118 and 3119 can then be replaced with this line:
	file += file.substr(file.rfind(folder_delimiter));

Automatic timezone database updates require curl installed.  To install curl:
	1. Open a command prompt
	2. CD to the directory where vcpkg (a C++ package manager written by
	   Microsoft) is to be installed
	3. git clone git clone https://github.com/Microsoft/vcpkg.git
	4. cd vcpkg
	5. bootstrap-vcpkg.bat
	6. vcpkg integrate install
	7. vcpkg install curl[tool] curl[tool]:x64-windows
*/

namespace datetime {
	using std::chrono::time_point;
	using std::chrono::duration;
	using std::chrono::hours;
	using std::chrono::minutes;
	using std::chrono::seconds;
	using std::chrono::system_clock;
	using system_time_point = system_clock::time_point;
	using system_duration = system_clock::duration;
	using time_point_seconds = time_point<system_clock, seconds>;
	using date::sys_days;
	using date::local_days;
	using date_type = date::year_month_day;
	using date::time_of_day;
	using date::year;
	using date::month;
	using date::day;
	using date::days;
	using date::months;
	using date::years;
	using date::operator <<;
	using date::time_zone;
	using date::make_zoned;
	using date::zoned_time;
	using date::current_zone;
	
	const time_zone* const default_zone = current_zone();

	class hour {
	private:
		unsigned char _value;
		bool _h24;
	public:
		hour(bool h24 = true);
		explicit constexpr hour(unsigned h, bool h24 = true);
		constexpr hour& operator ++ ();
		constexpr hour operator ++ (int);
		constexpr hour operator + (const hour& h);
		constexpr hour operator + (const hours& h);
		constexpr hour& operator -- ();
		constexpr hour operator -- (int);
		constexpr hour operator - (const hours& h);
		constexpr hours operator - (const hour& h);
		constexpr hour& operator += (const hours& h);
		constexpr hour& operator -= (const hours& h);
		constexpr bool operator == (const hour& h);
		constexpr bool operator != (const hour& h);
		constexpr bool operator < (const hour& h);
		constexpr bool operator <= (const hour& h);
		constexpr bool operator > (const hour& h);
		constexpr bool operator >= (const hour& h);
		constexpr explicit operator int() const;
		constexpr explicit operator unsigned() const;
		bool is_24h();
		bool toggle_24h();
		void make_24h(bool new_24h);
		friend std::ostream& operator << (std::ostream& out, const hour& h);
	};

	class minute {
	private:
		unsigned char _value;
	public:
		minute();
		explicit constexpr minute(unsigned m);
		constexpr minute& operator ++ ();
		constexpr minute operator ++ (int);
		constexpr minute operator + (const minute& m);
		constexpr minute operator + (const minutes& m);
		constexpr minute& operator -- ();
		constexpr minute operator -- (int);
		constexpr minute operator - (const minutes& m);
		constexpr minutes operator - (const minute& m);
		constexpr minute& operator += (const minutes& m);
		constexpr minute& operator -= (const minutes& m);
		constexpr bool operator == (const minute& m);
		constexpr bool operator != (const minute& m);
		constexpr bool operator < (const minute& m);
		constexpr bool operator <= (const minute& m);
		constexpr bool operator > (const minute& m);
		constexpr bool operator >= (const minute& m);
		constexpr explicit operator int() const;
		constexpr explicit operator unsigned() const;
		friend std::ostream& operator << (std::ostream& out, const minute& m);
	};

	class second {
	private:
		unsigned char _value;
	public:
		second();
		explicit constexpr second(unsigned s);
		constexpr second& operator ++ ();
		constexpr second operator ++ (int);
		constexpr second operator + (const second& s);
		constexpr second operator + (const seconds& s);
		constexpr second& operator -- ();
		constexpr second operator -- (int);
		constexpr second operator - (const seconds& s);
		constexpr seconds operator - (const second& s);
		constexpr second& operator += (const seconds& s);
		constexpr second& operator -= (const seconds& s);
		constexpr bool operator == (const second& s);
		constexpr bool operator != (const second& s);
		constexpr bool operator < (const second& s);
		constexpr bool operator <= (const second& s);
		constexpr bool operator > (const second& s);
		constexpr bool operator >= (const second& s);
		constexpr explicit operator int() const;
		constexpr explicit operator unsigned() const;
		friend std::ostream& operator << (std::ostream& out, const second& s);
	};

	class DateTime {
	private:
		system_time_point* _time_point;
		bool _h24;
		const time_zone* _tz;
		std::string _tz_name;
		void _constructor_proxy(system_time_point* tp, bool h24, std::string tz);
		void _no_h24_proxy(system_time_point* tp, std::string tz);
	public:
		std::string h12format = "%A %B %d, %Y %I:%M:%S %p";
		std::string h24format = "%A %B %d, %Y %H:%M:%S";
		DateTime(const DateTime& other);
		DateTime(bool h24 = true, std::string time_zone="");
		DateTime(const sys_days& timeval, bool h24 = true, std::string time_zone = "");
		DateTime(const system_duration& timeval, bool h24 = true, std::string time_zone = "");
		DateTime(const system_time_point& timeval, bool h24 = true, std::string time_zone = "");
		DateTime(const time_point_seconds& timeval, bool h24 = true, std::string time_zone = "");
		DateTime(std::string time_zone);
		DateTime(const sys_days& timeval, std::string time_zone);
		DateTime(const system_duration& timeval, std::string time_zone);
		DateTime(const system_time_point& timeval, std::string time_zone);
		DateTime(const time_point_seconds& timeval, std::string time_zone);
		sys_days GetDays() const;
		date_type GetDate() const;
		time_of_day<seconds> GetTime() const;
		year GetYear() const;
		month GetMonth() const;
		day GetDay() const;
		hour GetHour() const;
		minute GetMinute() const;
		second GetSecond() const;
		zoned_time<system_duration> GetZonedTime() const;
		bool is_24h();
		bool toggle_24h();
		void make_24h(bool new_24h);
		DateTime operator + (const system_duration& dur) const;
		DateTime operator - (const system_duration& dur) const;
		system_duration operator - (const system_time_point& other) const;
		system_duration operator - (const DateTime& other) const;
		DateTime& operator += (const system_duration& dur);
		DateTime& operator -= (const system_duration& dur);
		bool operator < (const system_time_point& other) const;
		bool operator > (const system_time_point& other) const;
		bool operator == (const system_time_point& other) const;
		bool operator <= (const system_time_point& other) const;
		bool operator >= (const system_time_point& other) const;
		bool operator != (const system_time_point& other) const;
		bool operator < (const DateTime& other) const;
		bool operator > (const DateTime& other) const;
		bool operator == (const DateTime& other) const;
		bool operator <= (const DateTime& other) const;
		bool operator >= (const DateTime& other) const;
		bool operator != (const DateTime& other) const;
		operator system_time_point() const;
		operator time_point_seconds() const;
		friend std::ostream& operator << (std::ostream& out, const DateTime& dt);
		void set_timezone(std::string new_tz);
		std::string get_timezone() const;
		std::string to_string() const;
		std::string get_offset_from(const DateTime& other) const;
		template <typename T>
		T get_difference(const sys_days& other) const {
			return date::floor<T>(GetDays() - other);
		}
		template <typename T>
		T get_difference(const system_time_point& other) const {
			return date::floor<T>(*_time_point - other);
		}
		template <typename T>
		T get_difference(const DateTime& other) const {
			return date::floor<T>(*this - other);
		}
		template <typename T>
		T get_difference(const date_type& other) const {
			return date::floor<T>(GetDays() - (sys_days)other);
		}
	};

	std::pair<std::string,  const time_zone*> get_zone(std::string search);
	void build_zones_map();

	void set_install_dir(std::string new_dir);
}

#pragma comment(lib, "DateTime.lib")
