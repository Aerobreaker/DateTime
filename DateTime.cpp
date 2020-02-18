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
			return outp;
		}

		long long to_number(string inp) {
			if (inp == "") {
				return 0;
			}
#ifdef arith_parse_strings
			using namespace exprtk;
			expression<double> expr;
			parser<double>pars;
			if (!pars.compile(inp, expr)) return 0;
			return expr.value();
#else
			stringstream instrm(inp);
			long long outp = 0;
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
		//I initially put this in to try to stop some memory leaks I was having in another program
		//But this section causes errors
		/*for (pair<const string, pair<string, const time_zone*>>& it : _cached_zones) {
			delete it.second.second;
		}*/
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
			if (i == 0 || instr[i - 1] == ' ' || (instr[i - 1] >= '0' && instr[i - 1] <= '9')) {
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
			if (reading_state == finalize) { break; }
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

	constexpr int hour::operator + (const int& h) {
		return _value + h;
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

	constexpr int hour::operator - (const int& h) {
		return _value - h;
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

	constexpr int minute::operator + (const int& m) {
		return _value + m;
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

	constexpr int minute::operator - (const int& m) {
		return _value - m;
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

	ostream& operator << (ostream& out, const minute& m) {
		out << (int)m._value;
		return out;
	}

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
		return second((int)(_value + s.count()));
	}

	constexpr int second::operator + (const int& s) {
		return _value + s;
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
		return second((int)(_value - s.count()));
	}

	constexpr seconds second::operator - (const second& s) {
		return (seconds)(_value - (s._value));
	}

	constexpr int second::operator - (const int& s) {
		return _value - s;
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

	ostream& operator << (ostream& out, const second& s) {
		out << (int)s._value;
		return out;
	}

	pair<string, const time_zone*> translate_zone(string search) {
		if (search == "AFRICA/ADDIS_ABABA") return {"Africa/Addis_Ababa", locate_zone("Africa/Nairobi")};
		if (search == "AFRICA/ASMARA") return {"Africa/Asmara", locate_zone("Africa/Nairobi")};
		if (search == "AFRICA/ASMERA") return {"Africa/Asmera", locate_zone("Africa/Nairobi")};
		if (search == "AFRICA/BAMAKO") return {"Africa/Bamako", locate_zone("Africa/Abidjan")};
		if (search == "AFRICA/BANGUI") return {"Africa/Bangui", locate_zone("Africa/Lagos")};
		if (search == "AFRICA/BANJUL") return {"Africa/Banjul", locate_zone("Africa/Abidjan")};
		if (search == "AFRICA/BLANTYRE") return {"Africa/Blantyre", locate_zone("Africa/Maputo")};
		if (search == "AFRICA/BRAZZAVILLE") return {"Africa/Brazzaville", locate_zone("Africa/Lagos")};
		if (search == "AFRICA/BUJUMBURA") return {"Africa/Bujumbura", locate_zone("Africa/Maputo")};
		if (search == "AFRICA/CONAKRY") return {"Africa/Conakry", locate_zone("Africa/Abidjan")};
		if (search == "AFRICA/DAKAR") return {"Africa/Dakar", locate_zone("Africa/Abidjan")};
		if (search == "AFRICA/DAR_ES_SALAAM") return {"Africa/Dar_es_Salaam", locate_zone("Africa/Nairobi")};
		if (search == "AFRICA/DJIBOUTI") return {"Africa/Djibouti", locate_zone("Africa/Nairobi")};
		if (search == "AFRICA/DOUALA") return {"Africa/Douala", locate_zone("Africa/Lagos")};
		if (search == "AFRICA/FREETOWN") return {"Africa/Freetown", locate_zone("Africa/Abidjan")};
		if (search == "AFRICA/GABORONE") return {"Africa/Gaborone", locate_zone("Africa/Maputo")};
		if (search == "AFRICA/HARARE") return {"Africa/Harare", locate_zone("Africa/Maputo")};
		if (search == "AFRICA/KAMPALA") return {"Africa/Kampala", locate_zone("Africa/Nairobi")};
		if (search == "AFRICA/KIGALI") return {"Africa/Kigali", locate_zone("Africa/Maputo")};
		if (search == "AFRICA/KINSHASA") return {"Africa/Kinshasa", locate_zone("Africa/Lagos")};
		if (search == "AFRICA/LIBREVILLE") return {"Africa/Libreville", locate_zone("Africa/Lagos")};
		if (search == "AFRICA/LOME") return {"Africa/Lome", locate_zone("Africa/Abidjan")};
		if (search == "AFRICA/LUANDA") return {"Africa/Luanda", locate_zone("Africa/Lagos")};
		if (search == "AFRICA/LUBUMBASHI") return {"Africa/Lubumbashi", locate_zone("Africa/Maputo")};
		if (search == "AFRICA/LUSAKA") return {"Africa/Lusaka", locate_zone("Africa/Maputo")};
		if (search == "AFRICA/MALABO") return {"Africa/Malabo", locate_zone("Africa/Lagos")};
		if (search == "AFRICA/MASERU") return {"Africa/Maseru", locate_zone("Africa/Johannesburg")};
		if (search == "AFRICA/MBABANE") return {"Africa/Mbabane", locate_zone("Africa/Johannesburg")};
		if (search == "AFRICA/MOGADISHU") return {"Africa/Mogadishu", locate_zone("Africa/Nairobi")};
		if (search == "AFRICA/NIAMEY") return {"Africa/Niamey", locate_zone("Africa/Lagos")};
		if (search == "AFRICA/NOUAKCHOTT") return {"Africa/Nouakchott", locate_zone("Africa/Abidjan")};
		if (search == "AFRICA/OUAGADOUGOU") return {"Africa/Ouagadougou", locate_zone("Africa/Abidjan")};
		if (search == "AFRICA/PORTO-NOVO") return {"Africa/Porto-Novo", locate_zone("Africa/Lagos")};
		if (search == "AFRICA/TIMBUKTU") return {"Africa/Timbuktu", locate_zone("Africa/Abidjan")};
		if (search == "AMERICA/ANGUILLA") return {"America/Anguilla", locate_zone("America/Port_of_Spain")};
		if (search == "AMERICA/ANTIGUA") return {"America/Antigua", locate_zone("America/Port_of_Spain")};
		if (search == "AMERICA/ARGENTINA/COMODRIVADAVIA") return {"America/Argentina/ComodRivadavia", locate_zone("America/Argentina/Catamarca")};
		if (search == "AMERICA/ARUBA") return {"America/Aruba", locate_zone("America/Curacao")};
		if (search == "AMERICA/ATKA") return {"America/Atka", locate_zone("America/Adak")};
		if (search == "AMERICA/BUENOS_AIRES") return {"America/Buenos_Aires", locate_zone("America/Argentina/Buenos_Aires")};
		if (search == "AMERICA/CATAMARCA") return {"America/Catamarca", locate_zone("America/Argentina/Catamarca")};
		if (search == "AMERICA/CAYMAN") return {"America/Cayman", locate_zone("America/Panama")};
		if (search == "AMERICA/CORAL_HARBOUR") return {"America/Coral_Harbour", locate_zone("America/Atikokan")};
		if (search == "AMERICA/CORDOBA") return {"America/Cordoba", locate_zone("America/Argentina/Cordoba")};
		if (search == "AMERICA/DOMINICA") return {"America/Dominica", locate_zone("America/Port_of_Spain")};
		if (search == "AMERICA/ENSENADA") return {"America/Ensenada", locate_zone("America/Tijuana")};
		if (search == "AMERICA/FORT_WAYNE") return {"America/Fort_Wayne", locate_zone("America/Indiana/Indianapolis")};
		if (search == "AMERICA/GRENADA") return {"America/Grenada", locate_zone("America/Port_of_Spain")};
		if (search == "AMERICA/GUADELOUPE") return {"America/Guadeloupe", locate_zone("America/Port_of_Spain")};
		if (search == "AMERICA/INDIANAPOLIS") return {"America/Indianapolis", locate_zone("America/Indiana/Indianapolis")};
		if (search == "AMERICA/JUJUY") return {"America/Jujuy", locate_zone("America/Argentina/Jujuy")};
		if (search == "AMERICA/KNOX_IN") return {"America/Knox_IN", locate_zone("America/Indiana/Knox")};
		if (search == "AMERICA/KRALENDIJK") return {"America/Kralendijk", locate_zone("America/Curacao")};
		if (search == "AMERICA/LOUISVILLE") return {"America/Louisville", locate_zone("America/Kentucky/Louisville")};
		if (search == "AMERICA/LOWER_PRINCES") return {"America/Lower_Princes", locate_zone("America/Curacao")};
		if (search == "AMERICA/MARIGOT") return {"America/Marigot", locate_zone("America/Port_of_Spain")};
		if (search == "AMERICA/MENDOZA") return {"America/Mendoza", locate_zone("America/Argentina/Mendoza")};
		if (search == "AMERICA/MONTREAL") return {"America/Montreal", locate_zone("America/Toronto")};
		if (search == "AMERICA/MONTSERRAT") return {"America/Montserrat", locate_zone("America/Port_of_Spain")};
		if (search == "AMERICA/PORTO_ACRE") return {"America/Porto_Acre", locate_zone("America/Rio_Branco")};
		if (search == "AMERICA/ROSARIO") return {"America/Rosario", locate_zone("America/Argentina/Cordoba")};
		if (search == "AMERICA/SANTA_ISABEL") return {"America/Santa_Isabel", locate_zone("America/Tijuana")};
		if (search == "AMERICA/SHIPROCK") return {"America/Shiprock", locate_zone("America/Denver")};
		if (search == "AMERICA/ST_BARTHELEMY") return {"America/St_Barthelemy", locate_zone("America/Port_of_Spain")};
		if (search == "AMERICA/ST_KITTS") return {"America/St_Kitts", locate_zone("America/Port_of_Spain")};
		if (search == "AMERICA/ST_LUCIA") return {"America/St_Lucia", locate_zone("America/Port_of_Spain")};
		if (search == "AMERICA/ST_THOMAS") return {"America/St_Thomas", locate_zone("America/Port_of_Spain")};
		if (search == "AMERICA/ST_VINCENT") return {"America/St_Vincent", locate_zone("America/Port_of_Spain")};
		if (search == "AMERICA/TORTOLA") return {"America/Tortola", locate_zone("America/Port_of_Spain")};
		if (search == "AMERICA/VIRGIN") return {"America/Virgin", locate_zone("America/Port_of_Spain")};
		if (search == "ANTARCTICA/MCMURDO") return {"Antarctica/McMurdo", locate_zone("Pacific/Auckland")};
		if (search == "ANTARCTICA/SOUTH_POLE") return {"Antarctica/South_Pole", locate_zone("Pacific/Auckland")};
		if (search == "ARCTIC/LONGYEARBYEN") return {"Arctic/Longyearbyen", locate_zone("Europe/Oslo")};
		if (search == "ASIA/ADEN") return {"Asia/Aden", locate_zone("Asia/Riyadh")};
		if (search == "ASIA/ASHKHABAD") return {"Asia/Ashkhabad", locate_zone("Asia/Ashgabat")};
		if (search == "ASIA/BAHRAIN") return {"Asia/Bahrain", locate_zone("Asia/Qatar")};
		if (search == "ASIA/CALCUTTA") return {"Asia/Calcutta", locate_zone("Asia/Kolkata")};
		if (search == "ASIA/CHONGQING") return {"Asia/Chongqing", locate_zone("Asia/Shanghai")};
		if (search == "ASIA/CHUNGKING") return {"Asia/Chungking", locate_zone("Asia/Shanghai")};
		if (search == "ASIA/DACCA") return {"Asia/Dacca", locate_zone("Asia/Dhaka")};
		if (search == "ASIA/HARBIN") return {"Asia/Harbin", locate_zone("Asia/Shanghai")};
		if (search == "ASIA/ISTANBUL") return {"Asia/Istanbul", locate_zone("Europe/Istanbul")};
		if (search == "ASIA/KASHGAR") return {"Asia/Kashgar", locate_zone("Asia/Urumqi")};
		if (search == "ASIA/KATMANDU") return {"Asia/Katmandu", locate_zone("Asia/Kathmandu")};
		if (search == "ASIA/KUWAIT") return {"Asia/Kuwait", locate_zone("Asia/Riyadh")};
		if (search == "ASIA/MACAO") return {"Asia/Macao", locate_zone("Asia/Macau")};
		if (search == "ASIA/MUSCAT") return {"Asia/Muscat", locate_zone("Asia/Dubai")};
		if (search == "ASIA/PHNOM_PENH") return {"Asia/Phnom_Penh", locate_zone("Asia/Bangkok")};
		if (search == "ASIA/RANGOON") return {"Asia/Rangoon", locate_zone("Asia/Yangon")};
		if (search == "ASIA/SAIGON") return {"Asia/Saigon", locate_zone("Asia/Ho_Chi_Minh")};
		if (search == "ASIA/TEL_AVIV") return {"Asia/Tel_Aviv", locate_zone("Asia/Jerusalem")};
		if (search == "ASIA/THIMBU") return {"Asia/Thimbu", locate_zone("Asia/Thimphu")};
		if (search == "ASIA/UJUNG_PANDANG") return {"Asia/Ujung_Pandang", locate_zone("Asia/Makassar")};
		if (search == "ASIA/ULAN_BATOR") return {"Asia/Ulan_Bator", locate_zone("Asia/Ulaanbaatar")};
		if (search == "ASIA/VIENTIANE") return {"Asia/Vientiane", locate_zone("Asia/Bangkok")};
		if (search == "ATLANTIC/FAEROE") return {"Atlantic/Faeroe", locate_zone("Atlantic/Faroe")};
		if (search == "ATLANTIC/JAN_MAYEN") return {"Atlantic/Jan_Mayen", locate_zone("Europe/Oslo")};
		if (search == "ATLANTIC/ST_HELENA") return {"Atlantic/St_Helena", locate_zone("Africa/Abidjan")};
		if (search == "AUSTRALIA/ACT") return {"Australia/ACT", locate_zone("Australia/Sydney")};
		if (search == "AUSTRALIA/CANBERRA") return {"Australia/Canberra", locate_zone("Australia/Sydney")};
		if (search == "AUSTRALIA/LHI") return {"Australia/LHI", locate_zone("Australia/Lord_Howe")};
		if (search == "AUSTRALIA/NSW") return {"Australia/NSW", locate_zone("Australia/Sydney")};
		if (search == "AUSTRALIA/NORTH") return {"Australia/North", locate_zone("Australia/Darwin")};
		if (search == "AUSTRALIA/QUEENSLAND") return {"Australia/Queensland", locate_zone("Australia/Brisbane")};
		if (search == "AUSTRALIA/SOUTH") return {"Australia/South", locate_zone("Australia/Adelaide")};
		if (search == "AUSTRALIA/TASMANIA") return {"Australia/Tasmania", locate_zone("Australia/Hobart")};
		if (search == "AUSTRALIA/VICTORIA") return {"Australia/Victoria", locate_zone("Australia/Melbourne")};
		if (search == "AUSTRALIA/WEST") return {"Australia/West", locate_zone("Australia/Perth")};
		if (search == "AUSTRALIA/YANCOWINNA") return {"Australia/Yancowinna", locate_zone("Australia/Broken_Hill")};
		if (search == "BRAZIL/ACRE") return {"Brazil/Acre", locate_zone("America/Rio_Branco")};
		if (search == "BRAZIL/DENORONHA") return {"Brazil/DeNoronha", locate_zone("America/Noronha")};
		if (search == "BRAZIL/EAST") return {"Brazil/East", locate_zone("America/Sao_Paulo")};
		if (search == "BRAZIL/WEST") return {"Brazil/West", locate_zone("America/Manaus")};
		if (search == "CANADA/ATLANTIC") return {"Canada/Atlantic", locate_zone("America/Halifax")};
		if (search == "CANADA/CENTRAL") return {"Canada/Central", locate_zone("America/Winnipeg")};
		if (search == "CANADA/EASTERN") return {"Canada/Eastern", locate_zone("America/Toronto")};
		if (search == "CANADA/MOUNTAIN") return {"Canada/Mountain", locate_zone("America/Edmonton")};
		if (search == "CANADA/NEWFOUNDLAND") return {"Canada/Newfoundland", locate_zone("America/St_Johns")};
		if (search == "CANADA/PACIFIC") return {"Canada/Pacific", locate_zone("America/Vancouver")};
		if (search == "CANADA/SASKATCHEWAN") return {"Canada/Saskatchewan", locate_zone("America/Regina")};
		if (search == "CANADA/YUKON") return {"Canada/Yukon", locate_zone("America/Whitehorse")};
		if (search == "CHILE/CONTINENTAL") return {"Chile/Continental", locate_zone("America/Santiago")};
		if (search == "CHILE/EASTERISLAND") return {"Chile/EasterIsland", locate_zone("Pacific/Easter")};
		if (search == "CUBA") return {"Cuba", locate_zone("America/Havana")};
		if (search == "EGYPT") return {"Egypt", locate_zone("Africa/Cairo")};
		if (search == "EIRE") return {"Eire", locate_zone("Europe/Dublin")};
		if (search == "ETC/GMT+0") return {"Etc/GMT+0", locate_zone("Etc/GMT")};
		if (search == "ETC/GMT-0") return {"Etc/GMT-0", locate_zone("Etc/GMT")};
		if (search == "ETC/GMT0") return {"Etc/GMT0", locate_zone("Etc/GMT")};
		if (search == "ETC/GREENWICH") return {"Etc/Greenwich", locate_zone("Etc/GMT")};
		if (search == "ETC/UCT") return {"Etc/UCT", locate_zone("Etc/UTC")};
		if (search == "ETC/UNIVERSAL") return {"Etc/Universal", locate_zone("Etc/UTC")};
		if (search == "ETC/ZULU") return {"Etc/Zulu", locate_zone("Etc/UTC")};
		if (search == "EUROPE/BELFAST") return {"Europe/Belfast", locate_zone("Europe/London")};
		if (search == "EUROPE/BRATISLAVA") return {"Europe/Bratislava", locate_zone("Europe/Prague")};
		if (search == "EUROPE/BUSINGEN") return {"Europe/Busingen", locate_zone("Europe/Zurich")};
		if (search == "EUROPE/GUERNSEY") return {"Europe/Guernsey", locate_zone("Europe/London")};
		if (search == "EUROPE/ISLE_OF_MAN") return {"Europe/Isle_of_Man", locate_zone("Europe/London")};
		if (search == "EUROPE/JERSEY") return {"Europe/Jersey", locate_zone("Europe/London")};
		if (search == "EUROPE/LJUBLJANA") return {"Europe/Ljubljana", locate_zone("Europe/Belgrade")};
		if (search == "EUROPE/MARIEHAMN") return {"Europe/Mariehamn", locate_zone("Europe/Helsinki")};
		if (search == "EUROPE/NICOSIA") return {"Europe/Nicosia", locate_zone("Asia/Nicosia")};
		if (search == "EUROPE/PODGORICA") return {"Europe/Podgorica", locate_zone("Europe/Belgrade")};
		if (search == "EUROPE/SAN_MARINO") return {"Europe/San_Marino", locate_zone("Europe/Rome")};
		if (search == "EUROPE/SARAJEVO") return {"Europe/Sarajevo", locate_zone("Europe/Belgrade")};
		if (search == "EUROPE/SKOPJE") return {"Europe/Skopje", locate_zone("Europe/Belgrade")};
		if (search == "EUROPE/TIRASPOL") return {"Europe/Tiraspol", locate_zone("Europe/Chisinau")};
		if (search == "EUROPE/VADUZ") return {"Europe/Vaduz", locate_zone("Europe/Zurich")};
		if (search == "EUROPE/VATICAN") return {"Europe/Vatican", locate_zone("Europe/Rome")};
		if (search == "EUROPE/ZAGREB") return {"Europe/Zagreb", locate_zone("Europe/Belgrade")};
		if (search == "GB") return {"GB", locate_zone("Europe/London")};
		if (search == "GB-EIRE") return {"GB-Eire", locate_zone("Europe/London")};
		if (search == "GMT") return {"GMT", locate_zone("Etc/GMT")};
		if (search == "GMT+0") return {"GMT+0", locate_zone("Etc/GMT")};
		if (search == "GMT-0") return {"GMT-0", locate_zone("Etc/GMT")};
		if (search == "GMT0") return {"GMT0", locate_zone("Etc/GMT")};
		if (search == "GREENWICH") return {"Greenwich", locate_zone("Etc/GMT")};
		if (search == "HONGKONG") return {"Hongkong", locate_zone("Asia/Hong_Kong")};
		if (search == "ICELAND") return {"Iceland", locate_zone("Atlantic/Reykjavik")};
		if (search == "INDIAN/ANTANANARIVO") return {"Indian/Antananarivo", locate_zone("Africa/Nairobi")};
		if (search == "INDIAN/COMORO") return {"Indian/Comoro", locate_zone("Africa/Nairobi")};
		if (search == "INDIAN/MAYOTTE") return {"Indian/Mayotte", locate_zone("Africa/Nairobi")};
		if (search == "IRAN") return {"Iran", locate_zone("Asia/Tehran")};
		if (search == "ISRAEL") return {"Israel", locate_zone("Asia/Jerusalem")};
		if (search == "JAMAICA") return {"Jamaica", locate_zone("America/Jamaica")};
		if (search == "JAPAN") return {"Japan", locate_zone("Asia/Tokyo")};
		if (search == "KWAJALEIN") return {"Kwajalein", locate_zone("Pacific/Kwajalein")};
		if (search == "LIBYA") return {"Libya", locate_zone("Africa/Tripoli")};
		if (search == "MEXICO/BAJANORTE") return {"Mexico/BajaNorte", locate_zone("America/Tijuana")};
		if (search == "MEXICO/BAJASUR") return {"Mexico/BajaSur", locate_zone("America/Mazatlan")};
		if (search == "MEXICO/GENERAL") return {"Mexico/General", locate_zone("America/Mexico_City")};
		if (search == "NZ") return {"NZ", locate_zone("Pacific/Auckland")};
		if (search == "NZ-CHAT") return {"NZ-CHAT", locate_zone("Pacific/Chatham")};
		if (search == "NAVAJO") return {"Navajo", locate_zone("America/Denver")};
		if (search == "PRC") return {"PRC", locate_zone("Asia/Shanghai")};
		if (search == "PACIFIC/JOHNSTON") return {"Pacific/Johnston", locate_zone("Pacific/Honolulu")};
		if (search == "PACIFIC/MIDWAY") return {"Pacific/Midway", locate_zone("Pacific/Pago_Pago")};
		if (search == "PACIFIC/PONAPE") return {"Pacific/Ponape", locate_zone("Pacific/Pohnpei")};
		if (search == "PACIFIC/SAIPAN") return {"Pacific/Saipan", locate_zone("Pacific/Guam")};
		if (search == "PACIFIC/SAMOA") return {"Pacific/Samoa", locate_zone("Pacific/Pago_Pago")};
		if (search == "PACIFIC/TRUK") return {"Pacific/Truk", locate_zone("Pacific/Chuuk")};
		if (search == "PACIFIC/YAP") return {"Pacific/Yap", locate_zone("Pacific/Chuuk")};
		if (search == "POLAND") return {"Poland", locate_zone("Europe/Warsaw")};
		if (search == "PORTUGAL") return {"Portugal", locate_zone("Europe/Lisbon")};
		if (search == "ROC") return {"ROC", locate_zone("Asia/Taipei")};
		if (search == "ROK") return {"ROK", locate_zone("Asia/Seoul")};
		if (search == "SINGAPORE") return {"Singapore", locate_zone("Asia/Singapore")};
		if (search == "TURKEY") return {"Turkey", locate_zone("Europe/Istanbul")};
		if (search == "UCT") return {"UCT", locate_zone("Etc/UTC")};
		if (search == "US/ALASKA") return {"US/Alaska", locate_zone("America/Anchorage")};
		if (search == "US/ALEUTIAN") return {"US/Aleutian", locate_zone("America/Adak")};
		if (search == "US/ARIZONA") return {"US/Arizona", locate_zone("America/Phoenix")};
		if (search == "US/CENTRAL") return {"US/Central", locate_zone("America/Chicago")};
		if (search == "US/EAST-INDIANA") return {"US/East-Indiana", locate_zone("America/Indiana/Indianapolis")};
		if (search == "US/EASTERN") return {"US/Eastern", locate_zone("America/New_York")};
		if (search == "US/HAWAII") return {"US/Hawaii", locate_zone("Pacific/Honolulu")};
		if (search == "US/INDIANA-STARKE") return {"US/Indiana-Starke", locate_zone("America/Indiana/Knox")};
		if (search == "US/MICHIGAN") return {"US/Michigan", locate_zone("America/Detroit")};
		if (search == "US/MOUNTAIN") return {"US/Mountain", locate_zone("America/Denver")};
		if (search == "US/PACIFIC") return {"US/Pacific", locate_zone("America/Los_Angeles")};
		if (search == "US/SAMOA") return {"US/Samoa", locate_zone("Pacific/Pago_Pago")};
		if (search == "UTC") return {"UTC", locate_zone("Etc/UTC")};
		if (search == "UNIVERSAL") return {"Universal", locate_zone("Etc/UTC")};
		if (search == "W-SU") return {"W-SU", locate_zone("Europe/Moscow")};
		if (search == "ZULU") return {"Zulu", locate_zone("Etc/UTC")};
		if (search == "PST") return {"PST", locate_zone("America/Los_Angeles")};
		if (search == "PDT") return {"PDT", locate_zone("America/Los_Angeles")};
		if (search == "EDT") return {"EDT", locate_zone("America/New_York")};
		if (search == "CST") return {"CST", locate_zone("America/Chicago")};
		if (search == "CDT") return {"CDT", locate_zone("America/Chicago")};
		if (search == "MDT") return {"MDT", locate_zone("America/Denver")};
		return {search, nullptr};
	}
}