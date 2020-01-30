#pragma once

#include "date/tz.h"
#include <string>

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

	std::pair<std::string, const time_zone*> get_zone(std::string search);
	void build_zones_map();
	void set_install_dir(std::string new_dir);
	
	constexpr char h12_format [] = "%A %B %d, %Y %I:%M:%S %p";
	constexpr char h24_format [] = "%A %B %d, %Y %H:%M:%S";

	static const time_zone* const default_zone = current_zone();

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

	template <class _Clock=system_clock, class _Duration=_Clock::duration>
	class DateTime {
		typedef std::chrono::time_point<_Clock, _Duration> time_point;
	private:
		time_point* _time_point;
		const time_zone* _tz;
		std::string _tz_name;
		void _constructor_proxy(time_point* tp, std::string tz) {
			_time_point = tp;
			if (tz != "") {
				set_timezone(tz);
			} else {
				_tz = default_zone;
				_tz_name = default_zone->name();
			}
		}
	public:
		std::string format = h12_format;
		DateTime(const DateTime& other) {
			_time_point = new time_point(*(other._time_point));
			_tz = other._tz;
			_tz_name = other._tz_name;
		}
		DateTime(std::string time_zone = "") {
			_constructor_proxy(new system_time_point(), time_zone);
		}
		DateTime(const sys_days& timeval, std::string time_zone = "") {
			_constructor_proxy(new system_time_point(timeval), time_zone);
		}
		DateTime(const _Duration& timeval, std::string time_zone = "") {
			_constructor_proxy(new system_time_point(timeval), time_zone);
		}
		template <class _tp_Clock = system_clock, class _tp_Duration = _tp_Clock::duration>
		DateTime(const std::chrono::time_point<_tp_Clock, _tp_Duration>& timeval, std::string time_zone = "") {
			_constructor_proxy(new system_time_point(timeval), time_zone);
		}
		DateTime(const time_point_seconds& timeval, std::string time_zone = "") {
			_constructor_proxy(new system_time_point(timeval), time_zone);
		}
		sys_days GetDays() const {
			return date::floor<days>(*_time_point);
		}
		date_type GetDate() const {
			return date_type {date::floor<days>(GetZonedTime().get_local_time())};
		}
		template <class _out_Duration=seconds>
		time_of_day<_out_Duration> GetTime() const {
			date::local_time<_Duration> local_tp = GetZonedTime().get_local_time();
			return time_of_day<_out_Duration>(date::floor<_out_Duration>(local_tp - date::floor<days>(local_tp)));
		}
		year GetYear() const {
			return GetDate().year();
		}
		month GetMonth() const {
			return GetDate().month();
		}
		day GetDay() const {
			return GetDate().day();
		}
		hour GetHour() const {
			return hour(GetTime().hours().count());
		}
		minute GetMinute() const {
			return minute(GetTime().minutes().count());
		}
		second GetSecond() const {
			return second(GetTime().seconds().count());
		}
		zoned_time<_Duration> GetZonedTime() const {
			return make_zoned(_tz, *_time_point);
		}
		DateTime operator + (const _Duration& dur) const {
			return DateTime(*_time_point + dur);
		}
		DateTime operator - (const _Duration& dur) const {
			return DateTime(*_time_point - dur);
		}
		template <class _tp_Clock, class _tp_Duration=_tp_Clock::duration>
		_Duration operator - (const std::chrono::time_point<_tp_Clock, _tp_Duration>& other) const {
			return *_time_point - other;
		}
		_Duration operator - (const DateTime& other) const {
			return *_time_point - *(other._time_point);
		}
		DateTime& operator += (const _Duration& dur) {
			*_time_point += dur;
			return *this;
		}
		DateTime& operator -= (const _Duration& dur) {
			*_time_point -= dur;
			return *this;
		}
		bool operator < (const time_point& other) const {
			return *_time_point < other;
		}
		bool operator > (const time_point& other) const {
			return *_time_point > other;
		}
		bool operator == (const time_point& other) const {
			return *_time_point == other;
		}
		bool operator <= (const time_point& other) const {
			return *_time_point <= other;
		}
		bool operator >= (const time_point& other) const {
			return *_time_point >= other;
		}
		bool operator != (const time_point& other) const {
			return *_time_point != other;
		}
		bool operator < (const DateTime& other) const {
			return *_time_point < *(other._time_point);
		}
		bool operator > (const DateTime& other) const {
			return *_time_point > * (other._time_point);
		}
		bool operator == (const DateTime& other) const {
			return *_time_point == *(other._time_point);
		}
		bool operator <= (const DateTime& other) const {
			return *_time_point <= *(other._time_point);
		}
		bool operator >= (const DateTime& other) const {
			return *_time_point >= *(other._time_point);
		}
		bool operator != (const DateTime& other) const {
			return *_time_point != *(other._time_point);
		}
		template <class _tp_Clock, class _tp_Duration=_tp_Clock::duration>
		operator std::chrono::time_point<_tp_Clock, _tp_Duration>() const {
			return std::chrono::time_point<_tp_Clock, _tp_Duration>(*_time_point);
		}
		operator time_point_seconds() const {
			return date::floor<seconds>(*_time_point);
		}
		friend std::ostream& operator << (std::ostream& out, const DateTime& dt) {
			out << dt.to_string();
		}
		void set_timezone(std::string new_tz) {
			std::tie(_tz_name, _tz) = get_zone(new_tz);
		}
		std::string get_timezone() const {
			return _tz_name;
		}
		std::string to_string() const {
			return date::format(format, date::floor<seconds>(GetZonedTime().get_local_time()));
		}
		std::string get_offset_from(const DateTime& other) const {
			std::string out {""};

			system_duration offset = *_time_point - *(other._time_point);

			zoned_time this_time = GetZonedTime();
			zoned_time other_time = other.GetZonedTime();

			offset += this_time.get_info().offset - other_time.get_info().offset;

			if (offset.count() == 0) {
				return out;
			}

			if (offset.count() < 0) {
				out += "-";
				offset = -offset;
			} else {
				out += "+";
			}

			days daysoff = date::floor<days>(offset);
			offset -= daysoff;
			if (daysoff.count() > 0) {
				out += std::to_string(daysoff.count()) + " days";
				if (offset.count() > 0) {
					out += ", ";
				} else {
					return out;
				}
			}

			hours hoursoff = date::floor<hours>(offset);
			offset -= hoursoff;
			minutes minutesoff = date::floor<minutes>(offset);
			offset -= minutesoff;
			seconds secondsoff = date::floor<seconds>(offset);

			if (hoursoff.count() > 0) {
				if (hoursoff.count() < 10) {
					out += "0";
				}
				out += std::to_string(hoursoff.count());
			} else {
				out += "00";
			}
			out += ":";
			if (minutesoff.count() > 0) {
				if (minutesoff.count() < 10) {
					out += "0";
				}
				out += std::to_string(minutesoff.count());
			} else {
				out += "00";
			}
			out += ":";
			if (secondsoff.count() > 0) {
				if (secondsoff.count() < 10) {
					out += "0";
				}
				out += std::to_string(secondsoff.count());
			} else {
				out += "00";
			}

			return out;
		}
		template <class _out_Duration>
		_out_Duration get_difference(const time_point& other) const {
			return date::floor<_out_Duration>(*_time_point - other);
		}
		template <class _out_Duration>
		_out_Duration get_difference(const DateTime& other) const {
			return date::floor<_out_Duration>(*this - other);
		}
		template <class _out_Duration>
		_out_Duration get_difference(const sys_days& other) const {
			return date::floor<_out_Duration>(GetDays() - other);
		}
		template <class _out_Duration>
		_out_Duration get_difference(const date_type& other) const {
			return date::floor<_out_Duration>(GetDays() - (sys_days)other);
		}
	};
}

#pragma comment(lib, "DateTime.lib")
