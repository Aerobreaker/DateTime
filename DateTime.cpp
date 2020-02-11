#include "DateTime.h"

#include <unordered_map>
#include <unordered_set>
#include <sstream>
#ifdef arith_parse_strings
#include <exprtk.hpp>
#endif

using namespace date;
using namespace std;

namespace datetime {
	
	namespace {
		//Namespace-private stuff goes here
		unordered_map<string, pair<string, const time_zone*>> _cached_zones;

		using std::filesystem::exists;

		string to_upper(string inps) {
			string out = string(inps);
			for (char& chr : out) {
				chr = std::toupper(chr);
			}
			return out;
		}

		string to_lower(string inp) {
			string out = inp;
			for (char& it : out) {
				it = tolower(it);
			}
			return out;
		}

		vector<string> split_string(string inp, string delims) {
			vector<string> outp {""};
			unordered_set<char> delimiters;
			for (const char& it : delims) {
				delimiters.insert(it);
			}
			for (const char& it : inp) {
				if (delimiters.contains(it)) {
					outp.push_back("");
				} else {
					outp.back().push_back(it);
				}
			}
			if (outp.back() == "") outp.pop_back();
			return outp;
		}

		vector<string> split_string(string inp, char delim) {
			vector<string> outp {""};
			for (const char& it : inp) {
				if (it == delim) {
					outp.push_back("");
				} else {
					outp.back().push_back(it);
				}
			}
			if (outp.back() == "") outp.pop_back();
			return outp;
		}

		long long to_number(string inp) {
#ifdef arith_parse_strings
			using namespace exprtk;
			expression<double> expr;
			parser<double>pars;
			if (!pars.compile(inp, expr)) return 0;
			return expr.value();
#else
			stringstream instrm(inp);
			long long outp;
			instrm >> outp;
			return outp;
#endif
		}

	}

	pair<string, const time_zone*> translate_zone(string search);

	pair<string, const time_zone*> get_zone(string search) {
		string newsearch = to_upper(search);
		if (!_cached_zones.contains(newsearch)) {
			_cached_zones[newsearch] = translate_zone(newsearch);
			if (_cached_zones[newsearch].second == nullptr) {
				_cached_zones[newsearch] = {search, locate_zone(search)};
			}
		}
		return _cached_zones[newsearch];
	}

	void clear_cache() {
		for (pair<const string, pair<string, const time_zone*>>& it : _cached_zones) {
			delete it.second.second;
		}
		_cached_zones.clear();
	}

	void set_install_dir(string new_dir) {
		std::filesystem::path new_path {new_dir};
		if (!exists(new_path.parent_path())) {
			std::filesystem::create_directories(new_path.parent_path());
		}
		date::set_install(new_dir);
	}

	sys_days parse_date(string instr, deque<string> fmts) {
		for (size_t i = 0; i < instr.length(); i++) {
			if (i == 0 || instr[i-1] == ' ' || (instr[i-1] >= '0' && instr[i-1] <= '9')) {
				instr[i] = toupper(instr[i]);
			} else {
				instr[i] = std::tolower(instr[i]);
			}
		}
		std::istringstream instrm(instr);
		sys_days outp(days(0));
		while (!fmts.empty()) {
			instrm.clear();
			instrm.str(instr);
			instrm >> parse(fmts.front(), outp);
			if (!instrm.fail()) {
				return outp;
			}
			fmts.pop_front();
		}
		return outp;
	}

	seconds parse_time(string instr) {
		/*
		This one's a bit longer than smart_date_parse, but since I'm only handling 2 formats and they're relatively
		easy to parse, I wanted to just build a quick state machine to handle it rather than figuring out formats and
		dealing with figuring out how date::parse is expecting AM/PM to come through when it's passed a %p flag
		*/
		enum reading {
			hrs,
			mins,
			secs,
			AMPM,
			finalize
		};
		enum ampm {
			am,
			pm,
			none
		};
		reading reading_state = hrs;
		ampm ampm_state = none;
		int hr = 0;
		int min = 0;
		int sec = 0;
		stringstream workingstr;
		for (const char& it : instr) {
			switch (reading_state) {
			case hrs:
				switch (it) {
				case ' ':
					workingstr >> hr;
					workingstr.clear();
					reading_state = AMPM;
					break;
				case ':':
					workingstr >> hr;
					workingstr.clear();
					reading_state = mins;
					break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					workingstr << it;
				case '-':
				case '.':
					break;
				default:
					workingstr >> hr;
					workingstr.clear();
					reading_state = finalize;
				}
				break;
			case mins:
				switch (it) {
				case ' ':
					workingstr >> min;
					workingstr.clear();
					reading_state = AMPM;
					break;
				case ':':
					workingstr >> min;
					workingstr.clear();
					reading_state = secs;
					break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					workingstr << it;
				case '-':
				case '.':
					break;
				default:
					workingstr >> min;
					workingstr.clear();
					reading_state = finalize;
				}
				break;
			case secs:
				switch (it) {
				case ' ':
					workingstr >> sec;
					workingstr.clear();
					reading_state = AMPM;
					break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					workingstr << it;
				case '-':
				case '.':
					break;
				default:
					workingstr >> sec;
					workingstr.clear();
					reading_state = finalize;
				}
				break;
			case AMPM:
				switch (it) {
				case 'a':
				case 'A':
					ampm_state = am;
					break;
				case 'p':
				case 'P':
					ampm_state = pm;
				}
				reading_state = finalize;
			}
			if (reading_state == finalize) {break;}
		}
		switch (reading_state) {
		case hrs:
			workingstr >> hr;
			break;
		case mins:
			workingstr >> min;
			break;
		case secs:
			workingstr >> sec;
		}
		//No need to handle negatives, as '-' characters are ignored in the state machine
		switch (ampm_state) {
		case am:
			hr %= 12;
			break;
		case pm:
			hr %= 12;
			hr += 12;
			break;
		case none:
			hr %= 24;
		}
		min %= 60;
		sec %= 60;
		return hours(hr) + minutes(min) + seconds(sec);
	}

	sys_days smart_parse_date(string inp, bool prefer_month) {
		inp = to_upper(inp);
		string delim = ",";
		int special_delims[3] = {0, 0, 0};
		vector<string> tokens {""};
		for (const char& it : inp) {
			if (it == ' ' || it == '\\') {
				delim += " \\";
				break;
			}
			if (it == '/') special_delims[0]++;
			if (it == '-') special_delims[1]++;
			if (it == '.') special_delims[2]++;
		}
		if (delim == "") {
			if ((special_delims[0] == 2) + (special_delims[1] == 2) + (special_delims[2] == 2) == 1) {
				if (special_delims[0] == 2) {
					delim += "/";
				}
				if (special_delims[1] == 2) {
					delim += "-";
				}
				if (special_delims[2] == 2) {
					delim += ".";
				}
			} else {
				if (special_delims[0] > 1) {
					delim += "/";
				} else if (special_delims[1] > 1) {
					delim += "-";
				} else if (special_delims[2] > 1) {
					delim += ".";
				}
			}
		}
		if (delim == ",") {
			constexpr sys_days null_date = sys_days(days(0));
			sys_days outp = null_date;
			if (inp.length() == 8) {
				outp = parse_date(inp, {"%d%m%y", "%m%d%y", "%y%m%d"});
			}
			if (outp == null_date) {
				bool reading_num;
				switch (inp[0]) {
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
				case '-':
				case '+':
				case '/':
				case '*':
					reading_num = true;
					break;
				default:
					reading_num = false;
				}
				for (const char& it : inp) {
					switch (it) {
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
					case '-':
					case '+':
					case '/':
					case '*':
						switch (reading_num) {
						case true:
							tokens.back().push_back(it);
							break;
						case false:
							tokens.push_back(string(1, it));
							reading_num = true;
						}
						break;
					case 'A':
					case 'B':
					case 'C':
					case 'D':
					case 'E':
					case 'F':
					case 'G':
					case 'H':
					case 'I':
					case 'J':
					case 'K':
					case 'L':
					case 'M':
					case 'N':
					case 'O':
					case 'P':
					case 'Q':
					case 'R':
					case 'S':
					case 'T':
					case 'U':
					case 'V':
					case 'W':
					case 'X':
					case 'Y':
					case 'Z':
						switch (reading_num) {
						case true:
							tokens.push_back(string(1, it));
							reading_num = false;
							break;
						case false:
							tokens.back().push_back(it);
						}
					}
				}
			} else {
				return outp;
			}
			//return outp != null_date ? outp : sys_days(days(to_number(inp)));
		} else {
			tokens = split_string(inp, delim);
		}
		deque<long long> unused;
		long long year = 0;
		long long month = 0;
		long long day = 0;
		date_type today {date::floor<days>(system_clock::now())};
		for (const string& token : tokens) {
			if (token == "JANUARY" || token == "JAN") {
				if (month != 0) {
					day = month;
				}
				month = 1;
			} else if (token == "FEBRUARY" || token == "FEB") {
				if (month != 0) {
					day = month;
				}
				month = 2;
			} else if (token == "MARCH" || token == "MAR") {
				if (month != 0) {
					day = month;
				}
				month = 3;
			} else if (token == "APRIL" || token == "APR") {
				if (month != 0) {
					day = month;
				}
				month = 4;
			} else if (token == "MAY") {
				if (month != 0) {
					day = month;
				}
				month = 5;
			} else if (token == "JUNE" || token == "JUN") {
				if (month != 0) {
					day = month;
				}
				month = 6;
			} else if (token == "JULY" || token == "JUL") {
				if (month != 0) {
					day = month;
				}
				month = 7;
			} else if (token == "AUGUST" || token == "AUG") {
				if (month != 0) {
					day = month;
				}
				month = 8;
			} else if (token == "SEPTEMBER" || token == "SEP") {
				if (month != 0) {
					day = month;
				}
				month = 9;
			} else if (token == "OCTOBER" || token == "OCT") {
				if (month != 0) {
					day = month;
				}
				month = 10;
			} else if (token == "NOVEMBER" || token == "NOV") {
				if (month != 0) {
					day = month;
				}
				month = 11;
			} else if (token == "DECEMBER" || token == "DEC") {
				if (month != 0) {
					day = month;
				}
				month = 12;
			} else {
				long long val = to_number(token);
				if (month == 0 && val > 0 && val < 13 && (day == 0) == prefer_month) {
					month = val;
				} else if (day == 0 && val > 0 && val < 32) {
					if (month == 0) {
						day = val;
					} else if (month == 2 && val < 30) {
						day = val;
					} else if (month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12) {
						day = val;
					} else if (val < 31) {
						day = val;
					}
				} else if (year == 0) {
					if (token.length() > 2 || (int)(val / 100)) {
						year = val;
					} else {
						//If they enter a 1 or 2 character token less than 100 (not that I can figure out how to get someting > 100 since we're not capturing factorial)
						year = val + (int)((int)today.year() / 100) * 100;
					}
				} else if (val != 0) {
					unused.push_back(val);
				}
			}
		}
		if (day > 31 && year < 32) {
			tie(day, year) = {year, day};
		}
		if (month == 0) {
			if (!unused.empty()) {
				month = unused.front();
				unused.pop_front();
			} else {
				month = (unsigned)today.month();
			}
		}
		if (day == 0) {
			if (!unused.empty()) {
				day = unused.front();
				unused.pop_front();
			} else {
				day = (unsigned)today.day();
			}
		}
		if (year == 0) {
			if (!unused.empty()) {
				year = unused.front();
				unused.pop_front();
			} else {
				year = (int)today.year();
			}
		}
		return sys_days(date::day(day) / date::month(month) / date::year(year));
	}

	seconds smart_parse_time(string inp) {
		enum class reading {
			hrs,
			mins,
			secs,
			AMPM,
			finalize
		};
		reading reading_state = reading::hrs;
		bool pm = false;
		long long hr = 0;
		long long min = 0;
		long long sec = 0;
		string workingstr;
		for (const char& it : inp) {
			switch (reading_state) {
			case reading::hrs:
				switch (it) {
				case ' ':
					hr = to_number(workingstr);
					workingstr = "";
					reading_state = reading::AMPM;
					break;
				case ':':
					hr = to_number(workingstr);
					workingstr = "";
					reading_state = reading::mins;
					break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
#ifndef  arith_parse_strings
					workingstr.push_back(it);
#endif // ! arith_parse_strings
				case '-':
				case '+':
				case '*':
				case '/':
#ifdef arith_parse_strings
					workingstr.push_back(it);
#endif // arith_parse_strings
					break;
				default:
					hr = to_number(workingstr);
					workingstr = "";
					reading_state = reading::finalize;
				}
				break;
			case reading::mins:
				switch (it) {
				case ' ':
					min = to_number(workingstr);
					workingstr = "";
					reading_state = reading::AMPM;
					break;
				case ':':
					min = to_number(workingstr);
					workingstr = "";
					reading_state = reading::secs;
					break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
#ifndef  arith_parse_strings
					workingstr.push_back(it);
#endif // ! arith_parse_strings
				case '-':
				case '+':
				case '*':
				case '/':
#ifdef arith_parse_strings
					workingstr.push_back(it);
#endif // arith_parse_strings
					break;
				default:
					min = to_number(workingstr);
					workingstr = "";
					reading_state = reading::finalize;
				}
				break;
			case reading::secs:
				switch (it) {
				case ' ':
					sec = to_number(workingstr);
					workingstr = "";
					reading_state = reading::AMPM;
					break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
#ifndef  arith_parse_strings
					workingstr.push_back(it);
#endif // ! arith_parse_strings
				case '-':
				case '+':
				case '*':
				case '/':
#ifdef arith_parse_strings
					workingstr.push_back(it);
#endif // arith_parse_strings
					break;
				default:
					sec = to_number(workingstr);
					workingstr = "";
					reading_state = reading::finalize;
				}
				break;
			case reading::AMPM:
				if (it == 'p' || it == 'P') {
					pm = true;
				}
				reading_state = reading::finalize;
			}
			if (reading_state == reading::finalize) { break; }
		}
		switch (reading_state) {
		case reading::hrs:
			hr = to_number(workingstr);
			break;
		case reading::mins:
			min = to_number(workingstr);
			break;
		case reading::secs:
			sec = to_number(workingstr);
		}
		if (pm) hr += 12;
		return hours(hr) + minutes(min) + seconds(sec);
	}

	hour::hour(bool h24) : _value(0), _h24(h24) {}

	constexpr hour::hour(unsigned h, bool h24) : _value(h % 24), _h24(h24) {}

	constexpr hour::hour(int h) : _value(h % 24), _h24(true) {}

	constexpr hour& hour::operator ++ () {
		_value = (_value + 1) % 24;
		return *this;
	}

	constexpr hour hour::operator ++ (int) {
		hour tmp = hour(_value, _h24);
		_value = (_value + 1) % 24;
		return tmp;
	}

	constexpr hour hour::operator + (const hour& h) {
		return hour(_value + h._value, _h24);
	}

	constexpr hour hour::operator + (const hours& h) {
		return hour(_value + h.count(), _h24);
	}

	constexpr hour& hour::operator -- () {
		if (_value > 0) {
			_value--;
		} else {
			_value = 23;
		}
		return *this;
	}

	constexpr hour hour::operator -- (int) {
		hour tmp = hour(_value, _h24);
		if (_value > 0) {
			_value--;
		} else {
			_value = 23;
		}
		return tmp;
	}

	constexpr hour hour::operator - (const hours& h) {
		return hour(_value - h.count(), _h24);
	}

	constexpr hours hour::operator - (const hour& h) {
		return (hours)(_value - (h._value));
	}

	constexpr hour& hour::operator += (const hours& h) {
		_value = (_value + h.count()) % 24;
		return *this;
	}

	constexpr hour& hour::operator -= (const hours& h) {
		_value = (_value - h.count()) % 24;
		return *this;
	}

	constexpr bool hour::operator == (const hour& h) {
		return _value == h._value;
	}

	constexpr bool hour::operator != (const hour& h) {
		return _value != h._value;
	}

	constexpr bool hour::operator < (const hour& h) {
		return _value < h._value;
	}

	constexpr bool hour::operator <= (const hour& h) {
		return _value <= h._value;
	}

	constexpr bool hour::operator > (const hour& h) {
		return _value > h._value;
	}

	constexpr bool hour::operator >= (const hour& h) {
		return _value >= h._value;
	}

	constexpr hour::operator int() const {
		if (_h24 || (_value < 12)) {
			return (int)_value;
		}
		return (int)(_value - 12);
	}

	constexpr hour::operator unsigned() const {
		if (_h24 || (_value < 12)) {
			return _value;
		}
		return _value - 12;
	}

	bool hour::is_24h() {
		return _h24;
	}

	bool hour::toggle_24h() {
		bool tmp = _h24;
		_h24 = !_h24;
		return tmp;
	}

	void hour::make_24h(bool new_24h) {
		_h24 = new_24h;
	}

	ostream& operator << (ostream& out, const hour& h) {
		if (h._h24) {
			out << (int)h._value;
		} else {
			if (h._value < 12) {
				out << (int)h._value << " AM";
			} else {
				out << (int)h._value - 12 << " PM";
			}
		}
		return out;
	}

	minute::minute() : _value(0) {}

	constexpr minute::minute(unsigned m) : _value(m % 60) {}

	constexpr minute& minute::operator ++ () {
		_value = (_value + 1) % 60;
		return *this;
	}

	constexpr minute minute::operator ++ (int) {
		minute tmp = minute(_value);
		_value = (_value + 1) % 60;
		return tmp;
	}

	constexpr minute minute::operator + (const minute& m) {
		return minute(_value + m._value);
	}

	constexpr minute minute::operator + (const minutes& m) {
		return minute(_value + m.count());
	}

	constexpr minute& minute::operator -- () {
		if (_value > 0) {
			_value--;
		} else {
			_value = 59;
		}
		return *this;
	}

	constexpr minute minute::operator -- (int) {
		minute tmp = minute(_value);
		if (_value > 0) {
			_value--;
		} else {
			_value = 59;
		}
		return tmp;
	}

	constexpr minute minute::operator - (const minutes& m) {
		return minute(_value - m.count());
	}

	constexpr minutes minute::operator - (const minute& m) {
		return (minutes)(_value - (m._value));
	}

	constexpr minute& minute::operator += (const minutes& m) {
		_value = (_value + m.count()) % 60;
		return *this;
	}

	constexpr minute& minute::operator -= (const minutes& m) {
		_value = (_value - m.count()) % 60;
		return *this;
	}

	constexpr bool minute::operator == (const minute& m) {
		return _value == m._value;
	}

	constexpr bool minute::operator != (const minute& m) {
		return _value != m._value;
	}

	constexpr bool minute::operator < (const minute& m) {
		return _value < m._value;
	}

	constexpr bool minute::operator <= (const minute& m) {
		return _value <= m._value;
	}

	constexpr bool minute::operator > (const minute& m) {
		return _value > m._value;
	}

	constexpr bool minute::operator >= (const minute& m) {
		return _value >= m._value;
	}

	constexpr minute::operator int() const {
		return (int)_value;
	}

	constexpr minute::operator unsigned() const {
		return _value;
	}

	ostream& operator << (ostream& out, const minute& m) {
		out << (int)m._value;
		return out;
	}

	second::second() : _value(0) {}

	constexpr second::second(unsigned s) : _value(s % 60) {}

	constexpr second& second::operator ++ () {
		_value = (_value + 1) % 60;
		return *this;
	}

	constexpr second second::operator ++ (int) {
		second tmp = second(_value);
		_value = (_value + 1) % 60;
		return tmp;
	}

	constexpr second second::operator + (const second& s) {
		return second(_value + s._value);
	}

	constexpr second second::operator + (const seconds& s) {
		return second(_value + s.count());
	}

	constexpr second& second::operator -- () {
		if (_value > 0) {
			_value--;
		} else {
			_value = 59;
		}
		return *this;
	}

	constexpr second second::operator -- (int) {
		second tmp = second(_value);
		if (_value > 0) {
			_value--;
		} else {
			_value = 59;
		}
		return tmp;
	}

	constexpr second second::operator - (const seconds& s) {
		return second(_value - s.count());
	}

	constexpr seconds second::operator - (const second& s) {
		return (seconds)(_value - (s._value));
	}

	constexpr second& second::operator += (const seconds& s) {
		_value = (_value + s.count()) % 60;
		return *this;
	}

	constexpr second& second::operator -= (const seconds& s) {
		_value = (_value - s.count()) % 60;
		return *this;
	}

	constexpr bool second::operator == (const second& s) {
		return _value == s._value;
	}

	constexpr bool second::operator != (const second& s) {
		return _value != s._value;
	}

	constexpr bool second::operator < (const second& s) {
		return _value < s._value;
	}

	constexpr bool second::operator <= (const second& s) {
		return _value <= s._value;
	}

	constexpr bool second::operator > (const second& s) {
		return _value > s._value;
	}

	constexpr bool second::operator >= (const second& s) {
		return _value >= s._value;
	}

	constexpr second::operator int() const {
		return (int)_value;
	}

	constexpr second::operator unsigned() const {
		return _value;
	}

	ostream& operator << (ostream& out, const second& s) {
		out << (int)s._value;
		return out;
	}

	pair<string, const time_zone*> translate_zone(string search) {
		if (search == "AFRICA/ABIDJAN") { return {"Africa/Abidjan", locate_zone("Africa/Abidjan")}; }
		if (search == "AFRICA/ACCRA") { return {"Africa/Accra", locate_zone("Africa/Accra")}; }
		if (search == "AFRICA/ADDIS_ABABA") { return {"Africa/Addis_Ababa", locate_zone("Africa/Addis_Ababa")}; }
		if (search == "AFRICA/ALGIERS") { return {"Africa/Algiers", locate_zone("Africa/Algiers")}; }
		if (search == "AFRICA/ASMARA") { return {"Africa/Asmara", locate_zone("Africa/Asmara")}; }
		if (search == "AFRICA/ASMERA") { return {"Africa/Asmera", locate_zone("Africa/Asmera")}; }
		if (search == "AFRICA/BAMAKO") { return {"Africa/Bamako", locate_zone("Africa/Bamako")}; }
		if (search == "AFRICA/BANGUI") { return {"Africa/Bangui", locate_zone("Africa/Bangui")}; }
		if (search == "AFRICA/BANJUL") { return {"Africa/Banjul", locate_zone("Africa/Banjul")}; }
		if (search == "AFRICA/BISSAU") { return {"Africa/Bissau", locate_zone("Africa/Bissau")}; }
		if (search == "AFRICA/BLANTYRE") { return {"Africa/Blantyre", locate_zone("Africa/Blantyre")}; }
		if (search == "AFRICA/BRAZZAVILLE") { return {"Africa/Brazzaville", locate_zone("Africa/Brazzaville")}; }
		if (search == "AFRICA/BUJUMBURA") { return {"Africa/Bujumbura", locate_zone("Africa/Bujumbura")}; }
		if (search == "AFRICA/CAIRO") { return {"Africa/Cairo", locate_zone("Africa/Cairo")}; }
		if (search == "AFRICA/CASABLANCA") { return {"Africa/Casablanca", locate_zone("Africa/Casablanca")}; }
		if (search == "AFRICA/CEUTA") { return {"Africa/Ceuta", locate_zone("Africa/Ceuta")}; }
		if (search == "AFRICA/CONAKRY") { return {"Africa/Conakry", locate_zone("Africa/Conakry")}; }
		if (search == "AFRICA/DAKAR") { return {"Africa/Dakar", locate_zone("Africa/Dakar")}; }
		if (search == "AFRICA/DAR_ES_SALAAM") { return {"Africa/Dar_es_Salaam", locate_zone("Africa/Dar_es_Salaam")}; }
		if (search == "AFRICA/DJIBOUTI") { return {"Africa/Djibouti", locate_zone("Africa/Djibouti")}; }
		if (search == "AFRICA/DOUALA") { return {"Africa/Douala", locate_zone("Africa/Douala")}; }
		if (search == "AFRICA/EL_AAIUN") { return {"Africa/El_Aaiun", locate_zone("Africa/El_Aaiun")}; }
		if (search == "AFRICA/FREETOWN") { return {"Africa/Freetown", locate_zone("Africa/Freetown")}; }
		if (search == "AFRICA/GABORONE") { return {"Africa/Gaborone", locate_zone("Africa/Gaborone")}; }
		if (search == "AFRICA/HARARE") { return {"Africa/Harare", locate_zone("Africa/Harare")}; }
		if (search == "AFRICA/JOHANNESBURG") { return {"Africa/Johannesburg", locate_zone("Africa/Johannesburg")}; }
		if (search == "AFRICA/JUBA") { return {"Africa/Juba", locate_zone("Africa/Juba")}; }
		if (search == "AFRICA/KAMPALA") { return {"Africa/Kampala", locate_zone("Africa/Kampala")}; }
		if (search == "AFRICA/KHARTOUM") { return {"Africa/Khartoum", locate_zone("Africa/Khartoum")}; }
		if (search == "AFRICA/KIGALI") { return {"Africa/Kigali", locate_zone("Africa/Kigali")}; }
		if (search == "AFRICA/KINSHASA") { return {"Africa/Kinshasa", locate_zone("Africa/Kinshasa")}; }
		if (search == "AFRICA/LAGOS") { return {"Africa/Lagos", locate_zone("Africa/Lagos")}; }
		if (search == "AFRICA/LIBREVILLE") { return {"Africa/Libreville", locate_zone("Africa/Libreville")}; }
		if (search == "AFRICA/LOME") { return {"Africa/Lome", locate_zone("Africa/Lome")}; }
		if (search == "AFRICA/LUANDA") { return {"Africa/Luanda", locate_zone("Africa/Luanda")}; }
		if (search == "AFRICA/LUBUMBASHI") { return {"Africa/Lubumbashi", locate_zone("Africa/Lubumbashi")}; }
		if (search == "AFRICA/LUSAKA") { return {"Africa/Lusaka", locate_zone("Africa/Lusaka")}; }
		if (search == "AFRICA/MALABO") { return {"Africa/Malabo", locate_zone("Africa/Malabo")}; }
		if (search == "AFRICA/MAPUTO") { return {"Africa/Maputo", locate_zone("Africa/Maputo")}; }
		if (search == "AFRICA/MASERU") { return {"Africa/Maseru", locate_zone("Africa/Maseru")}; }
		if (search == "AFRICA/MBABANE") { return {"Africa/Mbabane", locate_zone("Africa/Mbabane")}; }
		if (search == "AFRICA/MOGADISHU") { return {"Africa/Mogadishu", locate_zone("Africa/Mogadishu")}; }
		if (search == "AFRICA/MONROVIA") { return {"Africa/Monrovia", locate_zone("Africa/Monrovia")}; }
		if (search == "AFRICA/NAIROBI") { return {"Africa/Nairobi", locate_zone("Africa/Nairobi")}; }
		if (search == "AFRICA/NDJAMENA") { return {"Africa/Ndjamena", locate_zone("Africa/Ndjamena")}; }
		if (search == "AFRICA/NIAMEY") { return {"Africa/Niamey", locate_zone("Africa/Niamey")}; }
		if (search == "AFRICA/NOUAKCHOTT") { return {"Africa/Nouakchott", locate_zone("Africa/Nouakchott")}; }
		if (search == "AFRICA/OUAGADOUGOU") { return {"Africa/Ouagadougou", locate_zone("Africa/Ouagadougou")}; }
		if (search == "AFRICA/PORTO-NOVO") { return {"Africa/Porto-Novo", locate_zone("Africa/Porto-Novo")}; }
		if (search == "AFRICA/SAO_TOME") { return {"Africa/Sao_Tome", locate_zone("Africa/Sao_Tome")}; }
		if (search == "AFRICA/TIMBUKTU") { return {"Africa/Timbuktu", locate_zone("Africa/Timbuktu")}; }
		if (search == "AFRICA/TRIPOLI") { return {"Africa/Tripoli", locate_zone("Africa/Tripoli")}; }
		if (search == "AFRICA/TUNIS") { return {"Africa/Tunis", locate_zone("Africa/Tunis")}; }
		if (search == "AFRICA/WINDHOEK") { return {"Africa/Windhoek", locate_zone("Africa/Windhoek")}; }
		if (search == "AMERICA/ADAK") { return {"America/Adak", locate_zone("America/Adak")}; }
		if (search == "AMERICA/ANCHORAGE") { return {"America/Anchorage", locate_zone("America/Anchorage")}; }
		if (search == "AMERICA/ANGUILLA") { return {"America/Anguilla", locate_zone("America/Anguilla")}; }
		if (search == "AMERICA/ANTIGUA") { return {"America/Antigua", locate_zone("America/Antigua")}; }
		if (search == "AMERICA/ARAGUAINA") { return {"America/Araguaina", locate_zone("America/Araguaina")}; }
		if (search == "AMERICA/ARGENTINA/BUENOS_AIRES") { return {"America/Argentina/Buenos_Aires", locate_zone("America/Argentina/Buenos_Aires")}; }
		if (search == "AMERICA/ARGENTINA/CATAMARCA") { return {"America/Argentina/Catamarca", locate_zone("America/Argentina/Catamarca")}; }
		if (search == "AMERICA/ARGENTINA/COMODRIVADAVIA") { return {"America/Argentina/ComodRivadavia", locate_zone("America/Argentina/ComodRivadavia")}; }
		if (search == "AMERICA/ARGENTINA/CORDOBA") { return {"America/Argentina/Cordoba", locate_zone("America/Argentina/Cordoba")}; }
		if (search == "AMERICA/ARGENTINA/JUJUY") { return {"America/Argentina/Jujuy", locate_zone("America/Argentina/Jujuy")}; }
		if (search == "AMERICA/ARGENTINA/LA_RIOJA") { return {"America/Argentina/La_Rioja", locate_zone("America/Argentina/La_Rioja")}; }
		if (search == "AMERICA/ARGENTINA/MENDOZA") { return {"America/Argentina/Mendoza", locate_zone("America/Argentina/Mendoza")}; }
		if (search == "AMERICA/ARGENTINA/RIO_GALLEGOS") { return {"America/Argentina/Rio_Gallegos", locate_zone("America/Argentina/Rio_Gallegos")}; }
		if (search == "AMERICA/ARGENTINA/SALTA") { return {"America/Argentina/Salta", locate_zone("America/Argentina/Salta")}; }
		if (search == "AMERICA/ARGENTINA/SAN_JUAN") { return {"America/Argentina/San_Juan", locate_zone("America/Argentina/San_Juan")}; }
		if (search == "AMERICA/ARGENTINA/SAN_LUIS") { return {"America/Argentina/San_Luis", locate_zone("America/Argentina/San_Luis")}; }
		if (search == "AMERICA/ARGENTINA/TUCUMAN") { return {"America/Argentina/Tucuman", locate_zone("America/Argentina/Tucuman")}; }
		if (search == "AMERICA/ARGENTINA/USHUAIA") { return {"America/Argentina/Ushuaia", locate_zone("America/Argentina/Ushuaia")}; }
		if (search == "AMERICA/ARUBA") { return {"America/Aruba", locate_zone("America/Aruba")}; }
		if (search == "AMERICA/ASUNCION") { return {"America/Asuncion", locate_zone("America/Asuncion")}; }
		if (search == "AMERICA/ATIKOKAN") { return {"America/Atikokan", locate_zone("America/Atikokan")}; }
		if (search == "AMERICA/ATKA") { return {"America/Atka", locate_zone("America/Atka")}; }
		if (search == "AMERICA/BAHIA") { return {"America/Bahia", locate_zone("America/Bahia")}; }
		if (search == "AMERICA/BAHIA_BANDERAS") { return {"America/Bahia_Banderas", locate_zone("America/Bahia_Banderas")}; }
		if (search == "AMERICA/BARBADOS") { return {"America/Barbados", locate_zone("America/Barbados")}; }
		if (search == "AMERICA/BELEM") { return {"America/Belem", locate_zone("America/Belem")}; }
		if (search == "AMERICA/BELIZE") { return {"America/Belize", locate_zone("America/Belize")}; }
		if (search == "AMERICA/BLANC-SABLON") { return {"America/Blanc-Sablon", locate_zone("America/Blanc-Sablon")}; }
		if (search == "AMERICA/BOA_VISTA") { return {"America/Boa_Vista", locate_zone("America/Boa_Vista")}; }
		if (search == "AMERICA/BOGOTA") { return {"America/Bogota", locate_zone("America/Bogota")}; }
		if (search == "AMERICA/BOISE") { return {"America/Boise", locate_zone("America/Boise")}; }
		if (search == "AMERICA/BUENOS_AIRES") { return {"America/Buenos_Aires", locate_zone("America/Buenos_Aires")}; }
		if (search == "AMERICA/CAMBRIDGE_BAY") { return {"America/Cambridge_Bay", locate_zone("America/Cambridge_Bay")}; }
		if (search == "AMERICA/CAMPO_GRANDE") { return {"America/Campo_Grande", locate_zone("America/Campo_Grande")}; }
		if (search == "AMERICA/CANCUN") { return {"America/Cancun", locate_zone("America/Cancun")}; }
		if (search == "AMERICA/CARACAS") { return {"America/Caracas", locate_zone("America/Caracas")}; }
		if (search == "AMERICA/CATAMARCA") { return {"America/Catamarca", locate_zone("America/Catamarca")}; }
		if (search == "AMERICA/CAYENNE") { return {"America/Cayenne", locate_zone("America/Cayenne")}; }
		if (search == "AMERICA/CAYMAN") { return {"America/Cayman", locate_zone("America/Cayman")}; }
		if (search == "AMERICA/CHICAGO") { return {"America/Chicago", locate_zone("America/Chicago")}; }
		if (search == "AMERICA/CHIHUAHUA") { return {"America/Chihuahua", locate_zone("America/Chihuahua")}; }
		if (search == "AMERICA/CORAL_HARBOUR") { return {"America/Coral_Harbour", locate_zone("America/Coral_Harbour")}; }
		if (search == "AMERICA/CORDOBA") { return {"America/Cordoba", locate_zone("America/Cordoba")}; }
		if (search == "AMERICA/COSTA_RICA") { return {"America/Costa_Rica", locate_zone("America/Costa_Rica")}; }
		if (search == "AMERICA/CRESTON") { return {"America/Creston", locate_zone("America/Creston")}; }
		if (search == "AMERICA/CUIABA") { return {"America/Cuiaba", locate_zone("America/Cuiaba")}; }
		if (search == "AMERICA/CURACAO") { return {"America/Curacao", locate_zone("America/Curacao")}; }
		if (search == "AMERICA/DANMARKSHAVN") { return {"America/Danmarkshavn", locate_zone("America/Danmarkshavn")}; }
		if (search == "AMERICA/DAWSON") { return {"America/Dawson", locate_zone("America/Dawson")}; }
		if (search == "AMERICA/DAWSON_CREEK") { return {"America/Dawson_Creek", locate_zone("America/Dawson_Creek")}; }
		if (search == "AMERICA/DENVER") { return {"America/Denver", locate_zone("America/Denver")}; }
		if (search == "AMERICA/DETROIT") { return {"America/Detroit", locate_zone("America/Detroit")}; }
		if (search == "AMERICA/DOMINICA") { return {"America/Dominica", locate_zone("America/Dominica")}; }
		if (search == "AMERICA/EDMONTON") { return {"America/Edmonton", locate_zone("America/Edmonton")}; }
		if (search == "AMERICA/EIRUNEPE") { return {"America/Eirunepe", locate_zone("America/Eirunepe")}; }
		if (search == "AMERICA/EL_SALVADOR") { return {"America/El_Salvador", locate_zone("America/El_Salvador")}; }
		if (search == "AMERICA/ENSENADA") { return {"America/Ensenada", locate_zone("America/Ensenada")}; }
		if (search == "AMERICA/FORT_NELSON") { return {"America/Fort_Nelson", locate_zone("America/Fort_Nelson")}; }
		if (search == "AMERICA/FORT_WAYNE") { return {"America/Fort_Wayne", locate_zone("America/Fort_Wayne")}; }
		if (search == "AMERICA/FORTALEZA") { return {"America/Fortaleza", locate_zone("America/Fortaleza")}; }
		if (search == "AMERICA/GLACE_BAY") { return {"America/Glace_Bay", locate_zone("America/Glace_Bay")}; }
		if (search == "AMERICA/GODTHAB") { return {"America/Godthab", locate_zone("America/Godthab")}; }
		if (search == "AMERICA/GOOSE_BAY") { return {"America/Goose_Bay", locate_zone("America/Goose_Bay")}; }
		if (search == "AMERICA/GRAND_TURK") { return {"America/Grand_Turk", locate_zone("America/Grand_Turk")}; }
		if (search == "AMERICA/GRENADA") { return {"America/Grenada", locate_zone("America/Grenada")}; }
		if (search == "AMERICA/GUADELOUPE") { return {"America/Guadeloupe", locate_zone("America/Guadeloupe")}; }
		if (search == "AMERICA/GUATEMALA") { return {"America/Guatemala", locate_zone("America/Guatemala")}; }
		if (search == "AMERICA/GUAYAQUIL") { return {"America/Guayaquil", locate_zone("America/Guayaquil")}; }
		if (search == "AMERICA/GUYANA") { return {"America/Guyana", locate_zone("America/Guyana")}; }
		if (search == "AMERICA/HALIFAX") { return {"America/Halifax", locate_zone("America/Halifax")}; }
		if (search == "AMERICA/HAVANA") { return {"America/Havana", locate_zone("America/Havana")}; }
		if (search == "AMERICA/HERMOSILLO") { return {"America/Hermosillo", locate_zone("America/Hermosillo")}; }
		if (search == "AMERICA/INDIANA/INDIANAPOLIS") { return {"America/Indiana/Indianapolis", locate_zone("America/Indiana/Indianapolis")}; }
		if (search == "AMERICA/INDIANA/KNOX") { return {"America/Indiana/Knox", locate_zone("America/Indiana/Knox")}; }
		if (search == "AMERICA/INDIANA/MARENGO") { return {"America/Indiana/Marengo", locate_zone("America/Indiana/Marengo")}; }
		if (search == "AMERICA/INDIANA/PETERSBURG") { return {"America/Indiana/Petersburg", locate_zone("America/Indiana/Petersburg")}; }
		if (search == "AMERICA/INDIANA/TELL_CITY") { return {"America/Indiana/Tell_City", locate_zone("America/Indiana/Tell_City")}; }
		if (search == "AMERICA/INDIANA/VEVAY") { return {"America/Indiana/Vevay", locate_zone("America/Indiana/Vevay")}; }
		if (search == "AMERICA/INDIANA/VINCENNES") { return {"America/Indiana/Vincennes", locate_zone("America/Indiana/Vincennes")}; }
		if (search == "AMERICA/INDIANA/WINAMAC") { return {"America/Indiana/Winamac", locate_zone("America/Indiana/Winamac")}; }
		if (search == "AMERICA/INDIANAPOLIS") { return {"America/Indianapolis", locate_zone("America/Indianapolis")}; }
		if (search == "AMERICA/INUVIK") { return {"America/Inuvik", locate_zone("America/Inuvik")}; }
		if (search == "AMERICA/IQALUIT") { return {"America/Iqaluit", locate_zone("America/Iqaluit")}; }
		if (search == "AMERICA/JAMAICA") { return {"America/Jamaica", locate_zone("America/Jamaica")}; }
		if (search == "AMERICA/JUJUY") { return {"America/Jujuy", locate_zone("America/Jujuy")}; }
		if (search == "AMERICA/JUNEAU") { return {"America/Juneau", locate_zone("America/Juneau")}; }
		if (search == "AMERICA/KENTUCKY/LOUISVILLE") { return {"America/Kentucky/Louisville", locate_zone("America/Kentucky/Louisville")}; }
		if (search == "AMERICA/KENTUCKY/MONTICELLO") { return {"America/Kentucky/Monticello", locate_zone("America/Kentucky/Monticello")}; }
		if (search == "AMERICA/KNOX_IN") { return {"America/Knox_IN", locate_zone("America/Knox_IN")}; }
		if (search == "AMERICA/KRALENDIJK") { return {"America/Kralendijk", locate_zone("America/Kralendijk")}; }
		if (search == "AMERICA/LA_PAZ") { return {"America/La_Paz", locate_zone("America/La_Paz")}; }
		if (search == "AMERICA/LIMA") { return {"America/Lima", locate_zone("America/Lima")}; }
		if (search == "AMERICA/LOS_ANGELES") { return {"America/Los_Angeles", locate_zone("America/Los_Angeles")}; }
		if (search == "AMERICA/LOUISVILLE") { return {"America/Louisville", locate_zone("America/Louisville")}; }
		if (search == "AMERICA/LOWER_PRINCES") { return {"America/Lower_Princes", locate_zone("America/Lower_Princes")}; }
		if (search == "AMERICA/MACEIO") { return {"America/Maceio", locate_zone("America/Maceio")}; }
		if (search == "AMERICA/MANAGUA") { return {"America/Managua", locate_zone("America/Managua")}; }
		if (search == "AMERICA/MANAUS") { return {"America/Manaus", locate_zone("America/Manaus")}; }
		if (search == "AMERICA/MARIGOT") { return {"America/Marigot", locate_zone("America/Marigot")}; }
		if (search == "AMERICA/MARTINIQUE") { return {"America/Martinique", locate_zone("America/Martinique")}; }
		if (search == "AMERICA/MATAMOROS") { return {"America/Matamoros", locate_zone("America/Matamoros")}; }
		if (search == "AMERICA/MAZATLAN") { return {"America/Mazatlan", locate_zone("America/Mazatlan")}; }
		if (search == "AMERICA/MENDOZA") { return {"America/Mendoza", locate_zone("America/Mendoza")}; }
		if (search == "AMERICA/MENOMINEE") { return {"America/Menominee", locate_zone("America/Menominee")}; }
		if (search == "AMERICA/MERIDA") { return {"America/Merida", locate_zone("America/Merida")}; }
		if (search == "AMERICA/METLAKATLA") { return {"America/Metlakatla", locate_zone("America/Metlakatla")}; }
		if (search == "AMERICA/MEXICO_CITY") { return {"America/Mexico_City", locate_zone("America/Mexico_City")}; }
		if (search == "AMERICA/MIQUELON") { return {"America/Miquelon", locate_zone("America/Miquelon")}; }
		if (search == "AMERICA/MONCTON") { return {"America/Moncton", locate_zone("America/Moncton")}; }
		if (search == "AMERICA/MONTERREY") { return {"America/Monterrey", locate_zone("America/Monterrey")}; }
		if (search == "AMERICA/MONTEVIDEO") { return {"America/Montevideo", locate_zone("America/Montevideo")}; }
		if (search == "AMERICA/MONTREAL") { return {"America/Montreal", locate_zone("America/Montreal")}; }
		if (search == "AMERICA/MONTSERRAT") { return {"America/Montserrat", locate_zone("America/Montserrat")}; }
		if (search == "AMERICA/NASSAU") { return {"America/Nassau", locate_zone("America/Nassau")}; }
		if (search == "AMERICA/NEW_YORK") { return {"America/New_York", locate_zone("America/New_York")}; }
		if (search == "AMERICA/NIPIGON") { return {"America/Nipigon", locate_zone("America/Nipigon")}; }
		if (search == "AMERICA/NOME") { return {"America/Nome", locate_zone("America/Nome")}; }
		if (search == "AMERICA/NORONHA") { return {"America/Noronha", locate_zone("America/Noronha")}; }
		if (search == "AMERICA/NORTH_DAKOTA/BEULAH") { return {"America/North_Dakota/Beulah", locate_zone("America/North_Dakota/Beulah")}; }
		if (search == "AMERICA/NORTH_DAKOTA/CENTER") { return {"America/North_Dakota/Center", locate_zone("America/North_Dakota/Center")}; }
		if (search == "AMERICA/NORTH_DAKOTA/NEW_SALEM") { return {"America/North_Dakota/New_Salem", locate_zone("America/North_Dakota/New_Salem")}; }
		if (search == "AMERICA/OJINAGA") { return {"America/Ojinaga", locate_zone("America/Ojinaga")}; }
		if (search == "AMERICA/PANAMA") { return {"America/Panama", locate_zone("America/Panama")}; }
		if (search == "AMERICA/PANGNIRTUNG") { return {"America/Pangnirtung", locate_zone("America/Pangnirtung")}; }
		if (search == "AMERICA/PARAMARIBO") { return {"America/Paramaribo", locate_zone("America/Paramaribo")}; }
		if (search == "AMERICA/PHOENIX") { return {"America/Phoenix", locate_zone("America/Phoenix")}; }
		if (search == "AMERICA/PORT-AU-PRINCE") { return {"America/Port-au-Prince", locate_zone("America/Port-au-Prince")}; }
		if (search == "AMERICA/PORT_OF_SPAIN") { return {"America/Port_of_Spain", locate_zone("America/Port_of_Spain")}; }
		if (search == "AMERICA/PORTO_ACRE") { return {"America/Porto_Acre", locate_zone("America/Porto_Acre")}; }
		if (search == "AMERICA/PORTO_VELHO") { return {"America/Porto_Velho", locate_zone("America/Porto_Velho")}; }
		if (search == "AMERICA/PUERTO_RICO") { return {"America/Puerto_Rico", locate_zone("America/Puerto_Rico")}; }
		if (search == "AMERICA/PUNTA_ARENAS") { return {"America/Punta_Arenas", locate_zone("America/Punta_Arenas")}; }
		if (search == "AMERICA/RAINY_RIVER") { return {"America/Rainy_River", locate_zone("America/Rainy_River")}; }
		if (search == "AMERICA/RANKIN_INLET") { return {"America/Rankin_Inlet", locate_zone("America/Rankin_Inlet")}; }
		if (search == "AMERICA/RECIFE") { return {"America/Recife", locate_zone("America/Recife")}; }
		if (search == "AMERICA/REGINA") { return {"America/Regina", locate_zone("America/Regina")}; }
		if (search == "AMERICA/RESOLUTE") { return {"America/Resolute", locate_zone("America/Resolute")}; }
		if (search == "AMERICA/RIO_BRANCO") { return {"America/Rio_Branco", locate_zone("America/Rio_Branco")}; }
		if (search == "AMERICA/ROSARIO") { return {"America/Rosario", locate_zone("America/Rosario")}; }
		if (search == "AMERICA/SANTA_ISABEL") { return {"America/Santa_Isabel", locate_zone("America/Santa_Isabel")}; }
		if (search == "AMERICA/SANTAREM") { return {"America/Santarem", locate_zone("America/Santarem")}; }
		if (search == "AMERICA/SANTIAGO") { return {"America/Santiago", locate_zone("America/Santiago")}; }
		if (search == "AMERICA/SANTO_DOMINGO") { return {"America/Santo_Domingo", locate_zone("America/Santo_Domingo")}; }
		if (search == "AMERICA/SAO_PAULO") { return {"America/Sao_Paulo", locate_zone("America/Sao_Paulo")}; }
		if (search == "AMERICA/SCORESBYSUND") { return {"America/Scoresbysund", locate_zone("America/Scoresbysund")}; }
		if (search == "AMERICA/SHIPROCK") { return {"America/Shiprock", locate_zone("America/Shiprock")}; }
		if (search == "AMERICA/SITKA") { return {"America/Sitka", locate_zone("America/Sitka")}; }
		if (search == "AMERICA/ST_BARTHELEMY") { return {"America/St_Barthelemy", locate_zone("America/St_Barthelemy")}; }
		if (search == "AMERICA/ST_JOHNS") { return {"America/St_Johns", locate_zone("America/St_Johns")}; }
		if (search == "AMERICA/ST_KITTS") { return {"America/St_Kitts", locate_zone("America/St_Kitts")}; }
		if (search == "AMERICA/ST_LUCIA") { return {"America/St_Lucia", locate_zone("America/St_Lucia")}; }
		if (search == "AMERICA/ST_THOMAS") { return {"America/St_Thomas", locate_zone("America/St_Thomas")}; }
		if (search == "AMERICA/ST_VINCENT") { return {"America/St_Vincent", locate_zone("America/St_Vincent")}; }
		if (search == "AMERICA/SWIFT_CURRENT") { return {"America/Swift_Current", locate_zone("America/Swift_Current")}; }
		if (search == "AMERICA/TEGUCIGALPA") { return {"America/Tegucigalpa", locate_zone("America/Tegucigalpa")}; }
		if (search == "AMERICA/THULE") { return {"America/Thule", locate_zone("America/Thule")}; }
		if (search == "AMERICA/THUNDER_BAY") { return {"America/Thunder_Bay", locate_zone("America/Thunder_Bay")}; }
		if (search == "AMERICA/TIJUANA") { return {"America/Tijuana", locate_zone("America/Tijuana")}; }
		if (search == "AMERICA/TORONTO") { return {"America/Toronto", locate_zone("America/Toronto")}; }
		if (search == "AMERICA/TORTOLA") { return {"America/Tortola", locate_zone("America/Tortola")}; }
		if (search == "AMERICA/VANCOUVER") { return {"America/Vancouver", locate_zone("America/Vancouver")}; }
		if (search == "AMERICA/VIRGIN") { return {"America/Virgin", locate_zone("America/Virgin")}; }
		if (search == "AMERICA/WHITEHORSE") { return {"America/Whitehorse", locate_zone("America/Whitehorse")}; }
		if (search == "AMERICA/WINNIPEG") { return {"America/Winnipeg", locate_zone("America/Winnipeg")}; }
		if (search == "AMERICA/YAKUTAT") { return {"America/Yakutat", locate_zone("America/Yakutat")}; }
		if (search == "AMERICA/YELLOWKNIFE") { return {"America/Yellowknife", locate_zone("America/Yellowknife")}; }
		if (search == "ANTARCTICA/CASEY") { return {"Antarctica/Casey", locate_zone("Antarctica/Casey")}; }
		if (search == "ANTARCTICA/DAVIS") { return {"Antarctica/Davis", locate_zone("Antarctica/Davis")}; }
		if (search == "ANTARCTICA/DUMONTDURVILLE") { return {"Antarctica/DumontDUrville", locate_zone("Antarctica/DumontDUrville")}; }
		if (search == "ANTARCTICA/MACQUARIE") { return {"Antarctica/Macquarie", locate_zone("Antarctica/Macquarie")}; }
		if (search == "ANTARCTICA/MAWSON") { return {"Antarctica/Mawson", locate_zone("Antarctica/Mawson")}; }
		if (search == "ANTARCTICA/MCMURDO") { return {"Antarctica/McMurdo", locate_zone("Antarctica/McMurdo")}; }
		if (search == "ANTARCTICA/PALMER") { return {"Antarctica/Palmer", locate_zone("Antarctica/Palmer")}; }
		if (search == "ANTARCTICA/ROTHERA") { return {"Antarctica/Rothera", locate_zone("Antarctica/Rothera")}; }
		if (search == "ANTARCTICA/SOUTH_POLE") { return {"Antarctica/South_Pole", locate_zone("Antarctica/South_Pole")}; }
		if (search == "ANTARCTICA/SYOWA") { return {"Antarctica/Syowa", locate_zone("Antarctica/Syowa")}; }
		if (search == "ANTARCTICA/TROLL") { return {"Antarctica/Troll", locate_zone("Antarctica/Troll")}; }
		if (search == "ANTARCTICA/VOSTOK") { return {"Antarctica/Vostok", locate_zone("Antarctica/Vostok")}; }
		if (search == "ARCTIC/LONGYEARBYEN") { return {"Arctic/Longyearbyen", locate_zone("Arctic/Longyearbyen")}; }
		if (search == "ASIA/ADEN") { return {"Asia/Aden", locate_zone("Asia/Aden")}; }
		if (search == "ASIA/ALMATY") { return {"Asia/Almaty", locate_zone("Asia/Almaty")}; }
		if (search == "ASIA/AMMAN") { return {"Asia/Amman", locate_zone("Asia/Amman")}; }
		if (search == "ASIA/ANADYR") { return {"Asia/Anadyr", locate_zone("Asia/Anadyr")}; }
		if (search == "ASIA/AQTAU") { return {"Asia/Aqtau", locate_zone("Asia/Aqtau")}; }
		if (search == "ASIA/AQTOBE") { return {"Asia/Aqtobe", locate_zone("Asia/Aqtobe")}; }
		if (search == "ASIA/ASHGABAT") { return {"Asia/Ashgabat", locate_zone("Asia/Ashgabat")}; }
		if (search == "ASIA/ASHKHABAD") { return {"Asia/Ashkhabad", locate_zone("Asia/Ashkhabad")}; }
		if (search == "ASIA/ATYRAU") { return {"Asia/Atyrau", locate_zone("Asia/Atyrau")}; }
		if (search == "ASIA/BAGHDAD") { return {"Asia/Baghdad", locate_zone("Asia/Baghdad")}; }
		if (search == "ASIA/BAHRAIN") { return {"Asia/Bahrain", locate_zone("Asia/Bahrain")}; }
		if (search == "ASIA/BAKU") { return {"Asia/Baku", locate_zone("Asia/Baku")}; }
		if (search == "ASIA/BANGKOK") { return {"Asia/Bangkok", locate_zone("Asia/Bangkok")}; }
		if (search == "ASIA/BARNAUL") { return {"Asia/Barnaul", locate_zone("Asia/Barnaul")}; }
		if (search == "ASIA/BEIRUT") { return {"Asia/Beirut", locate_zone("Asia/Beirut")}; }
		if (search == "ASIA/BISHKEK") { return {"Asia/Bishkek", locate_zone("Asia/Bishkek")}; }
		if (search == "ASIA/BRUNEI") { return {"Asia/Brunei", locate_zone("Asia/Brunei")}; }
		if (search == "ASIA/CALCUTTA") { return {"Asia/Calcutta", locate_zone("Asia/Calcutta")}; }
		if (search == "ASIA/CHITA") { return {"Asia/Chita", locate_zone("Asia/Chita")}; }
		if (search == "ASIA/CHOIBALSAN") { return {"Asia/Choibalsan", locate_zone("Asia/Choibalsan")}; }
		if (search == "ASIA/CHONGQING") { return {"Asia/Chongqing", locate_zone("Asia/Chongqing")}; }
		if (search == "ASIA/CHUNGKING") { return {"Asia/Chungking", locate_zone("Asia/Chungking")}; }
		if (search == "ASIA/COLOMBO") { return {"Asia/Colombo", locate_zone("Asia/Colombo")}; }
		if (search == "ASIA/DACCA") { return {"Asia/Dacca", locate_zone("Asia/Dacca")}; }
		if (search == "ASIA/DAMASCUS") { return {"Asia/Damascus", locate_zone("Asia/Damascus")}; }
		if (search == "ASIA/DHAKA") { return {"Asia/Dhaka", locate_zone("Asia/Dhaka")}; }
		if (search == "ASIA/DILI") { return {"Asia/Dili", locate_zone("Asia/Dili")}; }
		if (search == "ASIA/DUBAI") { return {"Asia/Dubai", locate_zone("Asia/Dubai")}; }
		if (search == "ASIA/DUSHANBE") { return {"Asia/Dushanbe", locate_zone("Asia/Dushanbe")}; }
		if (search == "ASIA/FAMAGUSTA") { return {"Asia/Famagusta", locate_zone("Asia/Famagusta")}; }
		if (search == "ASIA/GAZA") { return {"Asia/Gaza", locate_zone("Asia/Gaza")}; }
		if (search == "ASIA/HARBIN") { return {"Asia/Harbin", locate_zone("Asia/Harbin")}; }
		if (search == "ASIA/HEBRON") { return {"Asia/Hebron", locate_zone("Asia/Hebron")}; }
		if (search == "ASIA/HO_CHI_MINH") { return {"Asia/Ho_Chi_Minh", locate_zone("Asia/Ho_Chi_Minh")}; }
		if (search == "ASIA/HONG_KONG") { return {"Asia/Hong_Kong", locate_zone("Asia/Hong_Kong")}; }
		if (search == "ASIA/HOVD") { return {"Asia/Hovd", locate_zone("Asia/Hovd")}; }
		if (search == "ASIA/IRKUTSK") { return {"Asia/Irkutsk", locate_zone("Asia/Irkutsk")}; }
		if (search == "ASIA/ISTANBUL") { return {"Asia/Istanbul", locate_zone("Asia/Istanbul")}; }
		if (search == "ASIA/JAKARTA") { return {"Asia/Jakarta", locate_zone("Asia/Jakarta")}; }
		if (search == "ASIA/JAYAPURA") { return {"Asia/Jayapura", locate_zone("Asia/Jayapura")}; }
		if (search == "ASIA/JERUSALEM") { return {"Asia/Jerusalem", locate_zone("Asia/Jerusalem")}; }
		if (search == "ASIA/KABUL") { return {"Asia/Kabul", locate_zone("Asia/Kabul")}; }
		if (search == "ASIA/KAMCHATKA") { return {"Asia/Kamchatka", locate_zone("Asia/Kamchatka")}; }
		if (search == "ASIA/KARACHI") { return {"Asia/Karachi", locate_zone("Asia/Karachi")}; }
		if (search == "ASIA/KASHGAR") { return {"Asia/Kashgar", locate_zone("Asia/Kashgar")}; }
		if (search == "ASIA/KATHMANDU") { return {"Asia/Kathmandu", locate_zone("Asia/Kathmandu")}; }
		if (search == "ASIA/KATMANDU") { return {"Asia/Katmandu", locate_zone("Asia/Katmandu")}; }
		if (search == "ASIA/KHANDYGA") { return {"Asia/Khandyga", locate_zone("Asia/Khandyga")}; }
		if (search == "ASIA/KOLKATA") { return {"Asia/Kolkata", locate_zone("Asia/Kolkata")}; }
		if (search == "ASIA/KRASNOYARSK") { return {"Asia/Krasnoyarsk", locate_zone("Asia/Krasnoyarsk")}; }
		if (search == "ASIA/KUALA_LUMPUR") { return {"Asia/Kuala_Lumpur", locate_zone("Asia/Kuala_Lumpur")}; }
		if (search == "ASIA/KUCHING") { return {"Asia/Kuching", locate_zone("Asia/Kuching")}; }
		if (search == "ASIA/KUWAIT") { return {"Asia/Kuwait", locate_zone("Asia/Kuwait")}; }
		if (search == "ASIA/MACAO") { return {"Asia/Macao", locate_zone("Asia/Macao")}; }
		if (search == "ASIA/MACAU") { return {"Asia/Macau", locate_zone("Asia/Macau")}; }
		if (search == "ASIA/MAGADAN") { return {"Asia/Magadan", locate_zone("Asia/Magadan")}; }
		if (search == "ASIA/MAKASSAR") { return {"Asia/Makassar", locate_zone("Asia/Makassar")}; }
		if (search == "ASIA/MANILA") { return {"Asia/Manila", locate_zone("Asia/Manila")}; }
		if (search == "ASIA/MUSCAT") { return {"Asia/Muscat", locate_zone("Asia/Muscat")}; }
		if (search == "ASIA/NICOSIA") { return {"Asia/Nicosia", locate_zone("Asia/Nicosia")}; }
		if (search == "ASIA/NOVOKUZNETSK") { return {"Asia/Novokuznetsk", locate_zone("Asia/Novokuznetsk")}; }
		if (search == "ASIA/NOVOSIBIRSK") { return {"Asia/Novosibirsk", locate_zone("Asia/Novosibirsk")}; }
		if (search == "ASIA/OMSK") { return {"Asia/Omsk", locate_zone("Asia/Omsk")}; }
		if (search == "ASIA/ORAL") { return {"Asia/Oral", locate_zone("Asia/Oral")}; }
		if (search == "ASIA/PHNOM_PENH") { return {"Asia/Phnom_Penh", locate_zone("Asia/Phnom_Penh")}; }
		if (search == "ASIA/PONTIANAK") { return {"Asia/Pontianak", locate_zone("Asia/Pontianak")}; }
		if (search == "ASIA/PYONGYANG") { return {"Asia/Pyongyang", locate_zone("Asia/Pyongyang")}; }
		if (search == "ASIA/QATAR") { return {"Asia/Qatar", locate_zone("Asia/Qatar")}; }
		if (search == "ASIA/QYZYLORDA") { return {"Asia/Qyzylorda", locate_zone("Asia/Qyzylorda")}; }
		if (search == "ASIA/RANGOON") { return {"Asia/Rangoon", locate_zone("Asia/Rangoon")}; }
		if (search == "ASIA/RIYADH") { return {"Asia/Riyadh", locate_zone("Asia/Riyadh")}; }
		if (search == "ASIA/SAIGON") { return {"Asia/Saigon", locate_zone("Asia/Saigon")}; }
		if (search == "ASIA/SAKHALIN") { return {"Asia/Sakhalin", locate_zone("Asia/Sakhalin")}; }
		if (search == "ASIA/SAMARKAND") { return {"Asia/Samarkand", locate_zone("Asia/Samarkand")}; }
		if (search == "ASIA/SEOUL") { return {"Asia/Seoul", locate_zone("Asia/Seoul")}; }
		if (search == "ASIA/SHANGHAI") { return {"Asia/Shanghai", locate_zone("Asia/Shanghai")}; }
		if (search == "ASIA/SINGAPORE") { return {"Asia/Singapore", locate_zone("Asia/Singapore")}; }
		if (search == "ASIA/SREDNEKOLYMSK") { return {"Asia/Srednekolymsk", locate_zone("Asia/Srednekolymsk")}; }
		if (search == "ASIA/TAIPEI") { return {"Asia/Taipei", locate_zone("Asia/Taipei")}; }
		if (search == "ASIA/TASHKENT") { return {"Asia/Tashkent", locate_zone("Asia/Tashkent")}; }
		if (search == "ASIA/TBILISI") { return {"Asia/Tbilisi", locate_zone("Asia/Tbilisi")}; }
		if (search == "ASIA/TEHRAN") { return {"Asia/Tehran", locate_zone("Asia/Tehran")}; }
		if (search == "ASIA/TEL_AVIV") { return {"Asia/Tel_Aviv", locate_zone("Asia/Tel_Aviv")}; }
		if (search == "ASIA/THIMBU") { return {"Asia/Thimbu", locate_zone("Asia/Thimbu")}; }
		if (search == "ASIA/THIMPHU") { return {"Asia/Thimphu", locate_zone("Asia/Thimphu")}; }
		if (search == "ASIA/TOKYO") { return {"Asia/Tokyo", locate_zone("Asia/Tokyo")}; }
		if (search == "ASIA/TOMSK") { return {"Asia/Tomsk", locate_zone("Asia/Tomsk")}; }
		if (search == "ASIA/UJUNG_PANDANG") { return {"Asia/Ujung_Pandang", locate_zone("Asia/Ujung_Pandang")}; }
		if (search == "ASIA/ULAANBAATAR") { return {"Asia/Ulaanbaatar", locate_zone("Asia/Ulaanbaatar")}; }
		if (search == "ASIA/ULAN_BATOR") { return {"Asia/Ulan_Bator", locate_zone("Asia/Ulan_Bator")}; }
		if (search == "ASIA/URUMQI") { return {"Asia/Urumqi", locate_zone("Asia/Urumqi")}; }
		if (search == "ASIA/UST-NERA") { return {"Asia/Ust-Nera", locate_zone("Asia/Ust-Nera")}; }
		if (search == "ASIA/VIENTIANE") { return {"Asia/Vientiane", locate_zone("Asia/Vientiane")}; }
		if (search == "ASIA/VLADIVOSTOK") { return {"Asia/Vladivostok", locate_zone("Asia/Vladivostok")}; }
		if (search == "ASIA/YAKUTSK") { return {"Asia/Yakutsk", locate_zone("Asia/Yakutsk")}; }
		if (search == "ASIA/YANGON") { return {"Asia/Yangon", locate_zone("Asia/Yangon")}; }
		if (search == "ASIA/YEKATERINBURG") { return {"Asia/Yekaterinburg", locate_zone("Asia/Yekaterinburg")}; }
		if (search == "ASIA/YEREVAN") { return {"Asia/Yerevan", locate_zone("Asia/Yerevan")}; }
		if (search == "ATLANTIC/AZORES") { return {"Atlantic/Azores", locate_zone("Atlantic/Azores")}; }
		if (search == "ATLANTIC/BERMUDA") { return {"Atlantic/Bermuda", locate_zone("Atlantic/Bermuda")}; }
		if (search == "ATLANTIC/CANARY") { return {"Atlantic/Canary", locate_zone("Atlantic/Canary")}; }
		if (search == "ATLANTIC/CAPE_VERDE") { return {"Atlantic/Cape_Verde", locate_zone("Atlantic/Cape_Verde")}; }
		if (search == "ATLANTIC/FAEROE") { return {"Atlantic/Faeroe", locate_zone("Atlantic/Faeroe")}; }
		if (search == "ATLANTIC/FAROE") { return {"Atlantic/Faroe", locate_zone("Atlantic/Faroe")}; }
		if (search == "ATLANTIC/JAN_MAYEN") { return {"Atlantic/Jan_Mayen", locate_zone("Atlantic/Jan_Mayen")}; }
		if (search == "ATLANTIC/MADEIRA") { return {"Atlantic/Madeira", locate_zone("Atlantic/Madeira")}; }
		if (search == "ATLANTIC/REYKJAVIK") { return {"Atlantic/Reykjavik", locate_zone("Atlantic/Reykjavik")}; }
		if (search == "ATLANTIC/SOUTH_GEORGIA") { return {"Atlantic/South_Georgia", locate_zone("Atlantic/South_Georgia")}; }
		if (search == "ATLANTIC/ST_HELENA") { return {"Atlantic/St_Helena", locate_zone("Atlantic/St_Helena")}; }
		if (search == "ATLANTIC/STANLEY") { return {"Atlantic/Stanley", locate_zone("Atlantic/Stanley")}; }
		if (search == "AUSTRALIA/ACT") { return {"Australia/ACT", locate_zone("Australia/ACT")}; }
		if (search == "AUSTRALIA/ADELAIDE") { return {"Australia/Adelaide", locate_zone("Australia/Adelaide")}; }
		if (search == "AUSTRALIA/BRISBANE") { return {"Australia/Brisbane", locate_zone("Australia/Brisbane")}; }
		if (search == "AUSTRALIA/BROKEN_HILL") { return {"Australia/Broken_Hill", locate_zone("Australia/Broken_Hill")}; }
		if (search == "AUSTRALIA/CANBERRA") { return {"Australia/Canberra", locate_zone("Australia/Canberra")}; }
		if (search == "AUSTRALIA/CURRIE") { return {"Australia/Currie", locate_zone("Australia/Currie")}; }
		if (search == "AUSTRALIA/DARWIN") { return {"Australia/Darwin", locate_zone("Australia/Darwin")}; }
		if (search == "AUSTRALIA/EUCLA") { return {"Australia/Eucla", locate_zone("Australia/Eucla")}; }
		if (search == "AUSTRALIA/HOBART") { return {"Australia/Hobart", locate_zone("Australia/Hobart")}; }
		if (search == "AUSTRALIA/LHI") { return {"Australia/LHI", locate_zone("Australia/LHI")}; }
		if (search == "AUSTRALIA/LINDEMAN") { return {"Australia/Lindeman", locate_zone("Australia/Lindeman")}; }
		if (search == "AUSTRALIA/LORD_HOWE") { return {"Australia/Lord_Howe", locate_zone("Australia/Lord_Howe")}; }
		if (search == "AUSTRALIA/MELBOURNE") { return {"Australia/Melbourne", locate_zone("Australia/Melbourne")}; }
		if (search == "AUSTRALIA/NSW") { return {"Australia/NSW", locate_zone("Australia/NSW")}; }
		if (search == "AUSTRALIA/NORTH") { return {"Australia/North", locate_zone("Australia/North")}; }
		if (search == "AUSTRALIA/PERTH") { return {"Australia/Perth", locate_zone("Australia/Perth")}; }
		if (search == "AUSTRALIA/QUEENSLAND") { return {"Australia/Queensland", locate_zone("Australia/Queensland")}; }
		if (search == "AUSTRALIA/SOUTH") { return {"Australia/South", locate_zone("Australia/South")}; }
		if (search == "AUSTRALIA/SYDNEY") { return {"Australia/Sydney", locate_zone("Australia/Sydney")}; }
		if (search == "AUSTRALIA/TASMANIA") { return {"Australia/Tasmania", locate_zone("Australia/Tasmania")}; }
		if (search == "AUSTRALIA/VICTORIA") { return {"Australia/Victoria", locate_zone("Australia/Victoria")}; }
		if (search == "AUSTRALIA/WEST") { return {"Australia/West", locate_zone("Australia/West")}; }
		if (search == "AUSTRALIA/YANCOWINNA") { return {"Australia/Yancowinna", locate_zone("Australia/Yancowinna")}; }
		if (search == "BRAZIL/ACRE") { return {"Brazil/Acre", locate_zone("Brazil/Acre")}; }
		if (search == "BRAZIL/DENORONHA") { return {"Brazil/DeNoronha", locate_zone("Brazil/DeNoronha")}; }
		if (search == "BRAZIL/EAST") { return {"Brazil/East", locate_zone("Brazil/East")}; }
		if (search == "BRAZIL/WEST") { return {"Brazil/West", locate_zone("Brazil/West")}; }
		if (search == "CET") { return {"CET", locate_zone("CET")}; }
		if (search == "CST6CDT") { return {"CST6CDT", locate_zone("CST6CDT")}; }
		if (search == "CANADA/ATLANTIC") { return {"Canada/Atlantic", locate_zone("Canada/Atlantic")}; }
		if (search == "CANADA/CENTRAL") { return {"Canada/Central", locate_zone("Canada/Central")}; }
		if (search == "CANADA/EASTERN") { return {"Canada/Eastern", locate_zone("Canada/Eastern")}; }
		if (search == "CANADA/MOUNTAIN") { return {"Canada/Mountain", locate_zone("Canada/Mountain")}; }
		if (search == "CANADA/NEWFOUNDLAND") { return {"Canada/Newfoundland", locate_zone("Canada/Newfoundland")}; }
		if (search == "CANADA/PACIFIC") { return {"Canada/Pacific", locate_zone("Canada/Pacific")}; }
		if (search == "CANADA/SASKATCHEWAN") { return {"Canada/Saskatchewan", locate_zone("Canada/Saskatchewan")}; }
		if (search == "CANADA/YUKON") { return {"Canada/Yukon", locate_zone("Canada/Yukon")}; }
		if (search == "CHILE/CONTINENTAL") { return {"Chile/Continental", locate_zone("Chile/Continental")}; }
		if (search == "CHILE/EASTERISLAND") { return {"Chile/EasterIsland", locate_zone("Chile/EasterIsland")}; }
		if (search == "CUBA") { return {"Cuba", locate_zone("Cuba")}; }
		if (search == "EET") { return {"EET", locate_zone("EET")}; }
		if (search == "EST") { return {"EST", locate_zone("EST")}; }
		if (search == "EST5EDT") { return {"EST5EDT", locate_zone("EST5EDT")}; }
		if (search == "EGYPT") { return {"Egypt", locate_zone("Egypt")}; }
		if (search == "EIRE") { return {"Eire", locate_zone("Eire")}; }
		if (search == "ETC/GMT") { return {"Etc/GMT", locate_zone("Etc/GMT")}; }
		if (search == "ETC/GMT+0") { return {"Etc/GMT+0", locate_zone("Etc/GMT+0")}; }
		if (search == "ETC/GMT+1") { return {"Etc/GMT+1", locate_zone("Etc/GMT+1")}; }
		if (search == "ETC/GMT+10") { return {"Etc/GMT+10", locate_zone("Etc/GMT+10")}; }
		if (search == "ETC/GMT+11") { return {"Etc/GMT+11", locate_zone("Etc/GMT+11")}; }
		if (search == "ETC/GMT+12") { return {"Etc/GMT+12", locate_zone("Etc/GMT+12")}; }
		if (search == "ETC/GMT+2") { return {"Etc/GMT+2", locate_zone("Etc/GMT+2")}; }
		if (search == "ETC/GMT+3") { return {"Etc/GMT+3", locate_zone("Etc/GMT+3")}; }
		if (search == "ETC/GMT+4") { return {"Etc/GMT+4", locate_zone("Etc/GMT+4")}; }
		if (search == "ETC/GMT+5") { return {"Etc/GMT+5", locate_zone("Etc/GMT+5")}; }
		if (search == "ETC/GMT+6") { return {"Etc/GMT+6", locate_zone("Etc/GMT+6")}; }
		if (search == "ETC/GMT+7") { return {"Etc/GMT+7", locate_zone("Etc/GMT+7")}; }
		if (search == "ETC/GMT+8") { return {"Etc/GMT+8", locate_zone("Etc/GMT+8")}; }
		if (search == "ETC/GMT+9") { return {"Etc/GMT+9", locate_zone("Etc/GMT+9")}; }
		if (search == "ETC/GMT-0") { return {"Etc/GMT-0", locate_zone("Etc/GMT-0")}; }
		if (search == "ETC/GMT-1") { return {"Etc/GMT-1", locate_zone("Etc/GMT-1")}; }
		if (search == "ETC/GMT-10") { return {"Etc/GMT-10", locate_zone("Etc/GMT-10")}; }
		if (search == "ETC/GMT-11") { return {"Etc/GMT-11", locate_zone("Etc/GMT-11")}; }
		if (search == "ETC/GMT-12") { return {"Etc/GMT-12", locate_zone("Etc/GMT-12")}; }
		if (search == "ETC/GMT-13") { return {"Etc/GMT-13", locate_zone("Etc/GMT-13")}; }
		if (search == "ETC/GMT-14") { return {"Etc/GMT-14", locate_zone("Etc/GMT-14")}; }
		if (search == "ETC/GMT-2") { return {"Etc/GMT-2", locate_zone("Etc/GMT-2")}; }
		if (search == "ETC/GMT-3") { return {"Etc/GMT-3", locate_zone("Etc/GMT-3")}; }
		if (search == "ETC/GMT-4") { return {"Etc/GMT-4", locate_zone("Etc/GMT-4")}; }
		if (search == "ETC/GMT-5") { return {"Etc/GMT-5", locate_zone("Etc/GMT-5")}; }
		if (search == "ETC/GMT-6") { return {"Etc/GMT-6", locate_zone("Etc/GMT-6")}; }
		if (search == "ETC/GMT-7") { return {"Etc/GMT-7", locate_zone("Etc/GMT-7")}; }
		if (search == "ETC/GMT-8") { return {"Etc/GMT-8", locate_zone("Etc/GMT-8")}; }
		if (search == "ETC/GMT-9") { return {"Etc/GMT-9", locate_zone("Etc/GMT-9")}; }
		if (search == "ETC/GMT0") { return {"Etc/GMT0", locate_zone("Etc/GMT0")}; }
		if (search == "ETC/GREENWICH") { return {"Etc/Greenwich", locate_zone("Etc/Greenwich")}; }
		if (search == "ETC/UCT") { return {"Etc/UCT", locate_zone("Etc/UCT")}; }
		if (search == "ETC/UTC") { return {"Etc/UTC", locate_zone("Etc/UTC")}; }
		if (search == "ETC/UNIVERSAL") { return {"Etc/Universal", locate_zone("Etc/Universal")}; }
		if (search == "ETC/ZULU") { return {"Etc/Zulu", locate_zone("Etc/Zulu")}; }
		if (search == "EUROPE/AMSTERDAM") { return {"Europe/Amsterdam", locate_zone("Europe/Amsterdam")}; }
		if (search == "EUROPE/ANDORRA") { return {"Europe/Andorra", locate_zone("Europe/Andorra")}; }
		if (search == "EUROPE/ASTRAKHAN") { return {"Europe/Astrakhan", locate_zone("Europe/Astrakhan")}; }
		if (search == "EUROPE/ATHENS") { return {"Europe/Athens", locate_zone("Europe/Athens")}; }
		if (search == "EUROPE/BELFAST") { return {"Europe/Belfast", locate_zone("Europe/Belfast")}; }
		if (search == "EUROPE/BELGRADE") { return {"Europe/Belgrade", locate_zone("Europe/Belgrade")}; }
		if (search == "EUROPE/BERLIN") { return {"Europe/Berlin", locate_zone("Europe/Berlin")}; }
		if (search == "EUROPE/BRATISLAVA") { return {"Europe/Bratislava", locate_zone("Europe/Bratislava")}; }
		if (search == "EUROPE/BRUSSELS") { return {"Europe/Brussels", locate_zone("Europe/Brussels")}; }
		if (search == "EUROPE/BUCHAREST") { return {"Europe/Bucharest", locate_zone("Europe/Bucharest")}; }
		if (search == "EUROPE/BUDAPEST") { return {"Europe/Budapest", locate_zone("Europe/Budapest")}; }
		if (search == "EUROPE/BUSINGEN") { return {"Europe/Busingen", locate_zone("Europe/Busingen")}; }
		if (search == "EUROPE/CHISINAU") { return {"Europe/Chisinau", locate_zone("Europe/Chisinau")}; }
		if (search == "EUROPE/COPENHAGEN") { return {"Europe/Copenhagen", locate_zone("Europe/Copenhagen")}; }
		if (search == "EUROPE/DUBLIN") { return {"Europe/Dublin", locate_zone("Europe/Dublin")}; }
		if (search == "EUROPE/GIBRALTAR") { return {"Europe/Gibraltar", locate_zone("Europe/Gibraltar")}; }
		if (search == "EUROPE/GUERNSEY") { return {"Europe/Guernsey", locate_zone("Europe/Guernsey")}; }
		if (search == "EUROPE/HELSINKI") { return {"Europe/Helsinki", locate_zone("Europe/Helsinki")}; }
		if (search == "EUROPE/ISLE_OF_MAN") { return {"Europe/Isle_of_Man", locate_zone("Europe/Isle_of_Man")}; }
		if (search == "EUROPE/ISTANBUL") { return {"Europe/Istanbul", locate_zone("Europe/Istanbul")}; }
		if (search == "EUROPE/JERSEY") { return {"Europe/Jersey", locate_zone("Europe/Jersey")}; }
		if (search == "EUROPE/KALININGRAD") { return {"Europe/Kaliningrad", locate_zone("Europe/Kaliningrad")}; }
		if (search == "EUROPE/KIEV") { return {"Europe/Kiev", locate_zone("Europe/Kiev")}; }
		if (search == "EUROPE/KIROV") { return {"Europe/Kirov", locate_zone("Europe/Kirov")}; }
		if (search == "EUROPE/LISBON") { return {"Europe/Lisbon", locate_zone("Europe/Lisbon")}; }
		if (search == "EUROPE/LJUBLJANA") { return {"Europe/Ljubljana", locate_zone("Europe/Ljubljana")}; }
		if (search == "EUROPE/LONDON") { return {"Europe/London", locate_zone("Europe/London")}; }
		if (search == "EUROPE/LUXEMBOURG") { return {"Europe/Luxembourg", locate_zone("Europe/Luxembourg")}; }
		if (search == "EUROPE/MADRID") { return {"Europe/Madrid", locate_zone("Europe/Madrid")}; }
		if (search == "EUROPE/MALTA") { return {"Europe/Malta", locate_zone("Europe/Malta")}; }
		if (search == "EUROPE/MARIEHAMN") { return {"Europe/Mariehamn", locate_zone("Europe/Mariehamn")}; }
		if (search == "EUROPE/MINSK") { return {"Europe/Minsk", locate_zone("Europe/Minsk")}; }
		if (search == "EUROPE/MONACO") { return {"Europe/Monaco", locate_zone("Europe/Monaco")}; }
		if (search == "EUROPE/MOSCOW") { return {"Europe/Moscow", locate_zone("Europe/Moscow")}; }
		if (search == "EUROPE/NICOSIA") { return {"Europe/Nicosia", locate_zone("Europe/Nicosia")}; }
		if (search == "EUROPE/OSLO") { return {"Europe/Oslo", locate_zone("Europe/Oslo")}; }
		if (search == "EUROPE/PARIS") { return {"Europe/Paris", locate_zone("Europe/Paris")}; }
		if (search == "EUROPE/PODGORICA") { return {"Europe/Podgorica", locate_zone("Europe/Podgorica")}; }
		if (search == "EUROPE/PRAGUE") { return {"Europe/Prague", locate_zone("Europe/Prague")}; }
		if (search == "EUROPE/RIGA") { return {"Europe/Riga", locate_zone("Europe/Riga")}; }
		if (search == "EUROPE/ROME") { return {"Europe/Rome", locate_zone("Europe/Rome")}; }
		if (search == "EUROPE/SAMARA") { return {"Europe/Samara", locate_zone("Europe/Samara")}; }
		if (search == "EUROPE/SAN_MARINO") { return {"Europe/San_Marino", locate_zone("Europe/San_Marino")}; }
		if (search == "EUROPE/SARAJEVO") { return {"Europe/Sarajevo", locate_zone("Europe/Sarajevo")}; }
		if (search == "EUROPE/SARATOV") { return {"Europe/Saratov", locate_zone("Europe/Saratov")}; }
		if (search == "EUROPE/SIMFEROPOL") { return {"Europe/Simferopol", locate_zone("Europe/Simferopol")}; }
		if (search == "EUROPE/SKOPJE") { return {"Europe/Skopje", locate_zone("Europe/Skopje")}; }
		if (search == "EUROPE/SOFIA") { return {"Europe/Sofia", locate_zone("Europe/Sofia")}; }
		if (search == "EUROPE/STOCKHOLM") { return {"Europe/Stockholm", locate_zone("Europe/Stockholm")}; }
		if (search == "EUROPE/TALLINN") { return {"Europe/Tallinn", locate_zone("Europe/Tallinn")}; }
		if (search == "EUROPE/TIRANE") { return {"Europe/Tirane", locate_zone("Europe/Tirane")}; }
		if (search == "EUROPE/TIRASPOL") { return {"Europe/Tiraspol", locate_zone("Europe/Tiraspol")}; }
		if (search == "EUROPE/ULYANOVSK") { return {"Europe/Ulyanovsk", locate_zone("Europe/Ulyanovsk")}; }
		if (search == "EUROPE/UZHGOROD") { return {"Europe/Uzhgorod", locate_zone("Europe/Uzhgorod")}; }
		if (search == "EUROPE/VADUZ") { return {"Europe/Vaduz", locate_zone("Europe/Vaduz")}; }
		if (search == "EUROPE/VATICAN") { return {"Europe/Vatican", locate_zone("Europe/Vatican")}; }
		if (search == "EUROPE/VIENNA") { return {"Europe/Vienna", locate_zone("Europe/Vienna")}; }
		if (search == "EUROPE/VILNIUS") { return {"Europe/Vilnius", locate_zone("Europe/Vilnius")}; }
		if (search == "EUROPE/VOLGOGRAD") { return {"Europe/Volgograd", locate_zone("Europe/Volgograd")}; }
		if (search == "EUROPE/WARSAW") { return {"Europe/Warsaw", locate_zone("Europe/Warsaw")}; }
		if (search == "EUROPE/ZAGREB") { return {"Europe/Zagreb", locate_zone("Europe/Zagreb")}; }
		if (search == "EUROPE/ZAPOROZHYE") { return {"Europe/Zaporozhye", locate_zone("Europe/Zaporozhye")}; }
		if (search == "EUROPE/ZURICH") { return {"Europe/Zurich", locate_zone("Europe/Zurich")}; }
		if (search == "GB") { return {"GB", locate_zone("GB")}; }
		if (search == "GB-EIRE") { return {"GB-Eire", locate_zone("GB-Eire")}; }
		if (search == "GMT") { return {"GMT", locate_zone("GMT")}; }
		if (search == "GMT+0") { return {"GMT+0", locate_zone("GMT+0")}; }
		if (search == "GMT-0") { return {"GMT-0", locate_zone("GMT-0")}; }
		if (search == "GMT0") { return {"GMT0", locate_zone("GMT0")}; }
		if (search == "GREENWICH") { return {"Greenwich", locate_zone("Greenwich")}; }
		if (search == "HST") { return {"HST", locate_zone("HST")}; }
		if (search == "HONGKONG") { return {"Hongkong", locate_zone("Hongkong")}; }
		if (search == "ICELAND") { return {"Iceland", locate_zone("Iceland")}; }
		if (search == "INDIAN/ANTANANARIVO") { return {"Indian/Antananarivo", locate_zone("Indian/Antananarivo")}; }
		if (search == "INDIAN/CHAGOS") { return {"Indian/Chagos", locate_zone("Indian/Chagos")}; }
		if (search == "INDIAN/CHRISTMAS") { return {"Indian/Christmas", locate_zone("Indian/Christmas")}; }
		if (search == "INDIAN/COCOS") { return {"Indian/Cocos", locate_zone("Indian/Cocos")}; }
		if (search == "INDIAN/COMORO") { return {"Indian/Comoro", locate_zone("Indian/Comoro")}; }
		if (search == "INDIAN/KERGUELEN") { return {"Indian/Kerguelen", locate_zone("Indian/Kerguelen")}; }
		if (search == "INDIAN/MAHE") { return {"Indian/Mahe", locate_zone("Indian/Mahe")}; }
		if (search == "INDIAN/MALDIVES") { return {"Indian/Maldives", locate_zone("Indian/Maldives")}; }
		if (search == "INDIAN/MAURITIUS") { return {"Indian/Mauritius", locate_zone("Indian/Mauritius")}; }
		if (search == "INDIAN/MAYOTTE") { return {"Indian/Mayotte", locate_zone("Indian/Mayotte")}; }
		if (search == "INDIAN/REUNION") { return {"Indian/Reunion", locate_zone("Indian/Reunion")}; }
		if (search == "IRAN") { return {"Iran", locate_zone("Iran")}; }
		if (search == "ISRAEL") { return {"Israel", locate_zone("Israel")}; }
		if (search == "JAMAICA") { return {"Jamaica", locate_zone("Jamaica")}; }
		if (search == "JAPAN") { return {"Japan", locate_zone("Japan")}; }
		if (search == "KWAJALEIN") { return {"Kwajalein", locate_zone("Kwajalein")}; }
		if (search == "LIBYA") { return {"Libya", locate_zone("Libya")}; }
		if (search == "MET") { return {"MET", locate_zone("MET")}; }
		if (search == "MST") { return {"MST", locate_zone("MST")}; }
		if (search == "MST7MDT") { return {"MST7MDT", locate_zone("MST7MDT")}; }
		if (search == "MEXICO/BAJANORTE") { return {"Mexico/BajaNorte", locate_zone("Mexico/BajaNorte")}; }
		if (search == "MEXICO/BAJASUR") { return {"Mexico/BajaSur", locate_zone("Mexico/BajaSur")}; }
		if (search == "MEXICO/GENERAL") { return {"Mexico/General", locate_zone("Mexico/General")}; }
		if (search == "NZ") { return {"NZ", locate_zone("NZ")}; }
		if (search == "NZ-CHAT") { return {"NZ-CHAT", locate_zone("NZ-CHAT")}; }
		if (search == "NAVAJO") { return {"Navajo", locate_zone("Navajo")}; }
		if (search == "PRC") { return {"PRC", locate_zone("PRC")}; }
		if (search == "PST8PDT") { return {"PST8PDT", locate_zone("PST8PDT")}; }
		if (search == "PACIFIC/APIA") { return {"Pacific/Apia", locate_zone("Pacific/Apia")}; }
		if (search == "PACIFIC/AUCKLAND") { return {"Pacific/Auckland", locate_zone("Pacific/Auckland")}; }
		if (search == "PACIFIC/BOUGAINVILLE") { return {"Pacific/Bougainville", locate_zone("Pacific/Bougainville")}; }
		if (search == "PACIFIC/CHATHAM") { return {"Pacific/Chatham", locate_zone("Pacific/Chatham")}; }
		if (search == "PACIFIC/CHUUK") { return {"Pacific/Chuuk", locate_zone("Pacific/Chuuk")}; }
		if (search == "PACIFIC/EASTER") { return {"Pacific/Easter", locate_zone("Pacific/Easter")}; }
		if (search == "PACIFIC/EFATE") { return {"Pacific/Efate", locate_zone("Pacific/Efate")}; }
		if (search == "PACIFIC/ENDERBURY") { return {"Pacific/Enderbury", locate_zone("Pacific/Enderbury")}; }
		if (search == "PACIFIC/FAKAOFO") { return {"Pacific/Fakaofo", locate_zone("Pacific/Fakaofo")}; }
		if (search == "PACIFIC/FIJI") { return {"Pacific/Fiji", locate_zone("Pacific/Fiji")}; }
		if (search == "PACIFIC/FUNAFUTI") { return {"Pacific/Funafuti", locate_zone("Pacific/Funafuti")}; }
		if (search == "PACIFIC/GALAPAGOS") { return {"Pacific/Galapagos", locate_zone("Pacific/Galapagos")}; }
		if (search == "PACIFIC/GAMBIER") { return {"Pacific/Gambier", locate_zone("Pacific/Gambier")}; }
		if (search == "PACIFIC/GUADALCANAL") { return {"Pacific/Guadalcanal", locate_zone("Pacific/Guadalcanal")}; }
		if (search == "PACIFIC/GUAM") { return {"Pacific/Guam", locate_zone("Pacific/Guam")}; }
		if (search == "PACIFIC/HONOLULU") { return {"Pacific/Honolulu", locate_zone("Pacific/Honolulu")}; }
		if (search == "PACIFIC/JOHNSTON") { return {"Pacific/Johnston", locate_zone("Pacific/Johnston")}; }
		if (search == "PACIFIC/KIRITIMATI") { return {"Pacific/Kiritimati", locate_zone("Pacific/Kiritimati")}; }
		if (search == "PACIFIC/KOSRAE") { return {"Pacific/Kosrae", locate_zone("Pacific/Kosrae")}; }
		if (search == "PACIFIC/KWAJALEIN") { return {"Pacific/Kwajalein", locate_zone("Pacific/Kwajalein")}; }
		if (search == "PACIFIC/MAJURO") { return {"Pacific/Majuro", locate_zone("Pacific/Majuro")}; }
		if (search == "PACIFIC/MARQUESAS") { return {"Pacific/Marquesas", locate_zone("Pacific/Marquesas")}; }
		if (search == "PACIFIC/MIDWAY") { return {"Pacific/Midway", locate_zone("Pacific/Midway")}; }
		if (search == "PACIFIC/NAURU") { return {"Pacific/Nauru", locate_zone("Pacific/Nauru")}; }
		if (search == "PACIFIC/NIUE") { return {"Pacific/Niue", locate_zone("Pacific/Niue")}; }
		if (search == "PACIFIC/NORFOLK") { return {"Pacific/Norfolk", locate_zone("Pacific/Norfolk")}; }
		if (search == "PACIFIC/NOUMEA") { return {"Pacific/Noumea", locate_zone("Pacific/Noumea")}; }
		if (search == "PACIFIC/PAGO_PAGO") { return {"Pacific/Pago_Pago", locate_zone("Pacific/Pago_Pago")}; }
		if (search == "PACIFIC/PALAU") { return {"Pacific/Palau", locate_zone("Pacific/Palau")}; }
		if (search == "PACIFIC/PITCAIRN") { return {"Pacific/Pitcairn", locate_zone("Pacific/Pitcairn")}; }
		if (search == "PACIFIC/POHNPEI") { return {"Pacific/Pohnpei", locate_zone("Pacific/Pohnpei")}; }
		if (search == "PACIFIC/PONAPE") { return {"Pacific/Ponape", locate_zone("Pacific/Ponape")}; }
		if (search == "PACIFIC/PORT_MORESBY") { return {"Pacific/Port_Moresby", locate_zone("Pacific/Port_Moresby")}; }
		if (search == "PACIFIC/RAROTONGA") { return {"Pacific/Rarotonga", locate_zone("Pacific/Rarotonga")}; }
		if (search == "PACIFIC/SAIPAN") { return {"Pacific/Saipan", locate_zone("Pacific/Saipan")}; }
		if (search == "PACIFIC/SAMOA") { return {"Pacific/Samoa", locate_zone("Pacific/Samoa")}; }
		if (search == "PACIFIC/TAHITI") { return {"Pacific/Tahiti", locate_zone("Pacific/Tahiti")}; }
		if (search == "PACIFIC/TARAWA") { return {"Pacific/Tarawa", locate_zone("Pacific/Tarawa")}; }
		if (search == "PACIFIC/TONGATAPU") { return {"Pacific/Tongatapu", locate_zone("Pacific/Tongatapu")}; }
		if (search == "PACIFIC/TRUK") { return {"Pacific/Truk", locate_zone("Pacific/Truk")}; }
		if (search == "PACIFIC/WAKE") { return {"Pacific/Wake", locate_zone("Pacific/Wake")}; }
		if (search == "PACIFIC/WALLIS") { return {"Pacific/Wallis", locate_zone("Pacific/Wallis")}; }
		if (search == "PACIFIC/YAP") { return {"Pacific/Yap", locate_zone("Pacific/Yap")}; }
		if (search == "POLAND") { return {"Poland", locate_zone("Poland")}; }
		if (search == "PORTUGAL") { return {"Portugal", locate_zone("Portugal")}; }
		if (search == "ROC") { return {"ROC", locate_zone("ROC")}; }
		if (search == "ROK") { return {"ROK", locate_zone("ROK")}; }
		if (search == "SINGAPORE") { return {"Singapore", locate_zone("Singapore")}; }
		if (search == "TURKEY") { return {"Turkey", locate_zone("Turkey")}; }
		if (search == "UCT") { return {"UCT", locate_zone("UCT")}; }
		if (search == "US/ALASKA") { return {"US/Alaska", locate_zone("US/Alaska")}; }
		if (search == "US/ALEUTIAN") { return {"US/Aleutian", locate_zone("US/Aleutian")}; }
		if (search == "US/ARIZONA") { return {"US/Arizona", locate_zone("US/Arizona")}; }
		if (search == "US/CENTRAL") { return {"US/Central", locate_zone("US/Central")}; }
		if (search == "US/EAST-INDIANA") { return {"US/East-Indiana", locate_zone("US/East-Indiana")}; }
		if (search == "US/EASTERN") { return {"US/Eastern", locate_zone("US/Eastern")}; }
		if (search == "US/HAWAII") { return {"US/Hawaii", locate_zone("US/Hawaii")}; }
		if (search == "US/INDIANA-STARKE") { return {"US/Indiana-Starke", locate_zone("US/Indiana-Starke")}; }
		if (search == "US/MICHIGAN") { return {"US/Michigan", locate_zone("US/Michigan")}; }
		if (search == "US/MOUNTAIN") { return {"US/Mountain", locate_zone("US/Mountain")}; }
		if (search == "US/PACIFIC") { return {"US/Pacific", locate_zone("US/Pacific")}; }
		if (search == "US/SAMOA") { return {"US/Samoa", locate_zone("US/Samoa")}; }
		if (search == "UTC") { return {"UTC", locate_zone("UTC")}; }
		if (search == "UNIVERSAL") { return {"Universal", locate_zone("Universal")}; }
		if (search == "W-SU") { return {"W-SU", locate_zone("W-SU")}; }
		if (search == "WET") { return {"WET", locate_zone("WET")}; }
		if (search == "ZULU") { return {"Zulu", locate_zone("Zulu")}; }
		if (search == "PST") { return {"PST", locate_zone("US/Pacific")}; }
		if (search == "PDT") { return {"PDT", locate_zone("US/Pacific")}; }
		if (search == "EDT") { return {"EDT", locate_zone("US/Eastern")}; }
		if (search == "CST") { return {"CST", locate_zone("US/Central")}; }
		if (search == "CDT") { return {"CDT", locate_zone("US/Central")}; }
		if (search == "MDT") { return {"MDT", locate_zone("US/Mountain")}; }
		return {search, nullptr};
	}
}