#include "DateTime.h"

#include <unordered_map>

namespace datetime {
	
	namespace {
		//Namespace-private stuff goes here
		using date::locate_zone;
		using std::string;
		using std::pair;
		using std::ostream;
		using std::filesystem::exists;

		std::unordered_map<string, pair<string, const time_zone*>> _cached_zones;
		string to_upper(string inps) {
			string out = string(inps);
			for (auto& chr : out) {
				chr = std::toupper(chr);
			}
			return out;
		};
	}

	pair<string, const time_zone*> get_zone(string search) {
		if (_cached_zones.empty()) {
			build_zones_map();
		}
		string newsearch = to_upper(search);
		if (!_cached_zones.contains(newsearch)) {
			_cached_zones[newsearch] = {search, locate_zone(search)};
		}
		return _cached_zones[newsearch];
	}

	void set_install_dir(string new_dir) {
		std::filesystem::path new_path {new_dir};
		if (!exists(new_path.parent_path())) {
			std::filesystem::create_directories(new_path.parent_path());
		}
		date::set_install(new_dir);
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
		}
		else {
			_value = 23;
		}
		return *this;
	}

	constexpr hour hour::operator -- (int) {
		hour tmp = hour(_value, _h24);
		if (_value > 0) {
			_value--;
		}
		else {
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
		}
		else {
			_value = 59;
		}
		return *this;
	}

	constexpr minute minute::operator -- (int) {
		minute tmp = minute(_value);
		if (_value > 0) {
			_value--;
		}
		else {
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
		}
		else {
			_value = 59;
		}
		return *this;
	}

	constexpr second second::operator -- (int) {
		second tmp = second(_value);
		if (_value > 0) {
			_value--;
		}
		else {
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

	void build_zones_map() {
		_cached_zones["AFRICA/ABIDJAN"] = {"Africa/Abidjan", locate_zone("Africa/Abidjan")};
		_cached_zones["AFRICA/ACCRA"] = {"Africa/Accra", locate_zone("Africa/Accra")};
		_cached_zones["AFRICA/ADDIS_ABABA"] = {"Africa/Addis_Ababa", locate_zone("Africa/Addis_Ababa")};
		_cached_zones["AFRICA/ALGIERS"] = {"Africa/Algiers", locate_zone("Africa/Algiers")};
		_cached_zones["AFRICA/ASMARA"] = {"Africa/Asmara", locate_zone("Africa/Asmara")};
		_cached_zones["AFRICA/ASMERA"] = {"Africa/Asmera", locate_zone("Africa/Asmera")};
		_cached_zones["AFRICA/BAMAKO"] = {"Africa/Bamako", locate_zone("Africa/Bamako")};
		_cached_zones["AFRICA/BANGUI"] = {"Africa/Bangui", locate_zone("Africa/Bangui")};
		_cached_zones["AFRICA/BANJUL"] = {"Africa/Banjul", locate_zone("Africa/Banjul")};
		_cached_zones["AFRICA/BISSAU"] = {"Africa/Bissau", locate_zone("Africa/Bissau")};
		_cached_zones["AFRICA/BLANTYRE"] = {"Africa/Blantyre", locate_zone("Africa/Blantyre")};
		_cached_zones["AFRICA/BRAZZAVILLE"] = {"Africa/Brazzaville", locate_zone("Africa/Brazzaville")};
		_cached_zones["AFRICA/BUJUMBURA"] = {"Africa/Bujumbura", locate_zone("Africa/Bujumbura")};
		_cached_zones["AFRICA/CAIRO"] = {"Africa/Cairo", locate_zone("Africa/Cairo")};
		_cached_zones["AFRICA/CASABLANCA"] = {"Africa/Casablanca", locate_zone("Africa/Casablanca")};
		_cached_zones["AFRICA/CEUTA"] = {"Africa/Ceuta", locate_zone("Africa/Ceuta")};
		_cached_zones["AFRICA/CONAKRY"] = {"Africa/Conakry", locate_zone("Africa/Conakry")};
		_cached_zones["AFRICA/DAKAR"] = {"Africa/Dakar", locate_zone("Africa/Dakar")};
		_cached_zones["AFRICA/DAR_ES_SALAAM"] = {"Africa/Dar_es_Salaam", locate_zone("Africa/Dar_es_Salaam")};
		_cached_zones["AFRICA/DJIBOUTI"] = {"Africa/Djibouti", locate_zone("Africa/Djibouti")};
		_cached_zones["AFRICA/DOUALA"] = {"Africa/Douala", locate_zone("Africa/Douala")};
		_cached_zones["AFRICA/EL_AAIUN"] = {"Africa/El_Aaiun", locate_zone("Africa/El_Aaiun")};
		_cached_zones["AFRICA/FREETOWN"] = {"Africa/Freetown", locate_zone("Africa/Freetown")};
		_cached_zones["AFRICA/GABORONE"] = {"Africa/Gaborone", locate_zone("Africa/Gaborone")};
		_cached_zones["AFRICA/HARARE"] = {"Africa/Harare", locate_zone("Africa/Harare")};
		_cached_zones["AFRICA/JOHANNESBURG"] = {"Africa/Johannesburg", locate_zone("Africa/Johannesburg")};
		_cached_zones["AFRICA/JUBA"] = {"Africa/Juba", locate_zone("Africa/Juba")};
		_cached_zones["AFRICA/KAMPALA"] = {"Africa/Kampala", locate_zone("Africa/Kampala")};
		_cached_zones["AFRICA/KHARTOUM"] = {"Africa/Khartoum", locate_zone("Africa/Khartoum")};
		_cached_zones["AFRICA/KIGALI"] = {"Africa/Kigali", locate_zone("Africa/Kigali")};
		_cached_zones["AFRICA/KINSHASA"] = {"Africa/Kinshasa", locate_zone("Africa/Kinshasa")};
		_cached_zones["AFRICA/LAGOS"] = {"Africa/Lagos", locate_zone("Africa/Lagos")};
		_cached_zones["AFRICA/LIBREVILLE"] = {"Africa/Libreville", locate_zone("Africa/Libreville")};
		_cached_zones["AFRICA/LOME"] = {"Africa/Lome", locate_zone("Africa/Lome")};
		_cached_zones["AFRICA/LUANDA"] = {"Africa/Luanda", locate_zone("Africa/Luanda")};
		_cached_zones["AFRICA/LUBUMBASHI"] = {"Africa/Lubumbashi", locate_zone("Africa/Lubumbashi")};
		_cached_zones["AFRICA/LUSAKA"] = {"Africa/Lusaka", locate_zone("Africa/Lusaka")};
		_cached_zones["AFRICA/MALABO"] = {"Africa/Malabo", locate_zone("Africa/Malabo")};
		_cached_zones["AFRICA/MAPUTO"] = {"Africa/Maputo", locate_zone("Africa/Maputo")};
		_cached_zones["AFRICA/MASERU"] = {"Africa/Maseru", locate_zone("Africa/Maseru")};
		_cached_zones["AFRICA/MBABANE"] = {"Africa/Mbabane", locate_zone("Africa/Mbabane")};
		_cached_zones["AFRICA/MOGADISHU"] = {"Africa/Mogadishu", locate_zone("Africa/Mogadishu")};
		_cached_zones["AFRICA/MONROVIA"] = {"Africa/Monrovia", locate_zone("Africa/Monrovia")};
		_cached_zones["AFRICA/NAIROBI"] = {"Africa/Nairobi", locate_zone("Africa/Nairobi")};
		_cached_zones["AFRICA/NDJAMENA"] = {"Africa/Ndjamena", locate_zone("Africa/Ndjamena")};
		_cached_zones["AFRICA/NIAMEY"] = {"Africa/Niamey", locate_zone("Africa/Niamey")};
		_cached_zones["AFRICA/NOUAKCHOTT"] = {"Africa/Nouakchott", locate_zone("Africa/Nouakchott")};
		_cached_zones["AFRICA/OUAGADOUGOU"] = {"Africa/Ouagadougou", locate_zone("Africa/Ouagadougou")};
		_cached_zones["AFRICA/PORTO-NOVO"] = {"Africa/Porto-Novo", locate_zone("Africa/Porto-Novo")};
		_cached_zones["AFRICA/SAO_TOME"] = {"Africa/Sao_Tome", locate_zone("Africa/Sao_Tome")};
		_cached_zones["AFRICA/TIMBUKTU"] = {"Africa/Timbuktu", locate_zone("Africa/Timbuktu")};
		_cached_zones["AFRICA/TRIPOLI"] = {"Africa/Tripoli", locate_zone("Africa/Tripoli")};
		_cached_zones["AFRICA/TUNIS"] = {"Africa/Tunis", locate_zone("Africa/Tunis")};
		_cached_zones["AFRICA/WINDHOEK"] = {"Africa/Windhoek", locate_zone("Africa/Windhoek")};
		_cached_zones["AMERICA/ADAK"] = {"America/Adak", locate_zone("America/Adak")};
		_cached_zones["AMERICA/ANCHORAGE"] = {"America/Anchorage", locate_zone("America/Anchorage")};
		_cached_zones["AMERICA/ANGUILLA"] = {"America/Anguilla", locate_zone("America/Anguilla")};
		_cached_zones["AMERICA/ANTIGUA"] = {"America/Antigua", locate_zone("America/Antigua")};
		_cached_zones["AMERICA/ARAGUAINA"] = {"America/Araguaina", locate_zone("America/Araguaina")};
		_cached_zones["AMERICA/ARGENTINA/BUENOS_AIRES"] = {"America/Argentina/Buenos_Aires", locate_zone("America/Argentina/Buenos_Aires")};
		_cached_zones["AMERICA/ARGENTINA/CATAMARCA"] = {"America/Argentina/Catamarca", locate_zone("America/Argentina/Catamarca")};
		_cached_zones["AMERICA/ARGENTINA/COMODRIVADAVIA"] = {"America/Argentina/ComodRivadavia", locate_zone("America/Argentina/ComodRivadavia")};
		_cached_zones["AMERICA/ARGENTINA/CORDOBA"] = {"America/Argentina/Cordoba", locate_zone("America/Argentina/Cordoba")};
		_cached_zones["AMERICA/ARGENTINA/JUJUY"] = {"America/Argentina/Jujuy", locate_zone("America/Argentina/Jujuy")};
		_cached_zones["AMERICA/ARGENTINA/LA_RIOJA"] = {"America/Argentina/La_Rioja", locate_zone("America/Argentina/La_Rioja")};
		_cached_zones["AMERICA/ARGENTINA/MENDOZA"] = {"America/Argentina/Mendoza", locate_zone("America/Argentina/Mendoza")};
		_cached_zones["AMERICA/ARGENTINA/RIO_GALLEGOS"] = {"America/Argentina/Rio_Gallegos", locate_zone("America/Argentina/Rio_Gallegos")};
		_cached_zones["AMERICA/ARGENTINA/SALTA"] = {"America/Argentina/Salta", locate_zone("America/Argentina/Salta")};
		_cached_zones["AMERICA/ARGENTINA/SAN_JUAN"] = {"America/Argentina/San_Juan", locate_zone("America/Argentina/San_Juan")};
		_cached_zones["AMERICA/ARGENTINA/SAN_LUIS"] = {"America/Argentina/San_Luis", locate_zone("America/Argentina/San_Luis")};
		_cached_zones["AMERICA/ARGENTINA/TUCUMAN"] = {"America/Argentina/Tucuman", locate_zone("America/Argentina/Tucuman")};
		_cached_zones["AMERICA/ARGENTINA/USHUAIA"] = {"America/Argentina/Ushuaia", locate_zone("America/Argentina/Ushuaia")};
		_cached_zones["AMERICA/ARUBA"] = {"America/Aruba", locate_zone("America/Aruba")};
		_cached_zones["AMERICA/ASUNCION"] = {"America/Asuncion", locate_zone("America/Asuncion")};
		_cached_zones["AMERICA/ATIKOKAN"] = {"America/Atikokan", locate_zone("America/Atikokan")};
		_cached_zones["AMERICA/ATKA"] = {"America/Atka", locate_zone("America/Atka")};
		_cached_zones["AMERICA/BAHIA"] = {"America/Bahia", locate_zone("America/Bahia")};
		_cached_zones["AMERICA/BAHIA_BANDERAS"] = {"America/Bahia_Banderas", locate_zone("America/Bahia_Banderas")};
		_cached_zones["AMERICA/BARBADOS"] = {"America/Barbados", locate_zone("America/Barbados")};
		_cached_zones["AMERICA/BELEM"] = {"America/Belem", locate_zone("America/Belem")};
		_cached_zones["AMERICA/BELIZE"] = {"America/Belize", locate_zone("America/Belize")};
		_cached_zones["AMERICA/BLANC-SABLON"] = {"America/Blanc-Sablon", locate_zone("America/Blanc-Sablon")};
		_cached_zones["AMERICA/BOA_VISTA"] = {"America/Boa_Vista", locate_zone("America/Boa_Vista")};
		_cached_zones["AMERICA/BOGOTA"] = {"America/Bogota", locate_zone("America/Bogota")};
		_cached_zones["AMERICA/BOISE"] = {"America/Boise", locate_zone("America/Boise")};
		_cached_zones["AMERICA/BUENOS_AIRES"] = {"America/Buenos_Aires", locate_zone("America/Buenos_Aires")};
		_cached_zones["AMERICA/CAMBRIDGE_BAY"] = {"America/Cambridge_Bay", locate_zone("America/Cambridge_Bay")};
		_cached_zones["AMERICA/CAMPO_GRANDE"] = {"America/Campo_Grande", locate_zone("America/Campo_Grande")};
		_cached_zones["AMERICA/CANCUN"] = {"America/Cancun", locate_zone("America/Cancun")};
		_cached_zones["AMERICA/CARACAS"] = {"America/Caracas", locate_zone("America/Caracas")};
		_cached_zones["AMERICA/CATAMARCA"] = {"America/Catamarca", locate_zone("America/Catamarca")};
		_cached_zones["AMERICA/CAYENNE"] = {"America/Cayenne", locate_zone("America/Cayenne")};
		_cached_zones["AMERICA/CAYMAN"] = {"America/Cayman", locate_zone("America/Cayman")};
		_cached_zones["AMERICA/CHICAGO"] = {"America/Chicago", locate_zone("America/Chicago")};
		_cached_zones["AMERICA/CHIHUAHUA"] = {"America/Chihuahua", locate_zone("America/Chihuahua")};
		_cached_zones["AMERICA/CORAL_HARBOUR"] = {"America/Coral_Harbour", locate_zone("America/Coral_Harbour")};
		_cached_zones["AMERICA/CORDOBA"] = {"America/Cordoba", locate_zone("America/Cordoba")};
		_cached_zones["AMERICA/COSTA_RICA"] = {"America/Costa_Rica", locate_zone("America/Costa_Rica")};
		_cached_zones["AMERICA/CRESTON"] = {"America/Creston", locate_zone("America/Creston")};
		_cached_zones["AMERICA/CUIABA"] = {"America/Cuiaba", locate_zone("America/Cuiaba")};
		_cached_zones["AMERICA/CURACAO"] = {"America/Curacao", locate_zone("America/Curacao")};
		_cached_zones["AMERICA/DANMARKSHAVN"] = {"America/Danmarkshavn", locate_zone("America/Danmarkshavn")};
		_cached_zones["AMERICA/DAWSON"] = {"America/Dawson", locate_zone("America/Dawson")};
		_cached_zones["AMERICA/DAWSON_CREEK"] = {"America/Dawson_Creek", locate_zone("America/Dawson_Creek")};
		_cached_zones["AMERICA/DENVER"] = {"America/Denver", locate_zone("America/Denver")};
		_cached_zones["AMERICA/DETROIT"] = {"America/Detroit", locate_zone("America/Detroit")};
		_cached_zones["AMERICA/DOMINICA"] = {"America/Dominica", locate_zone("America/Dominica")};
		_cached_zones["AMERICA/EDMONTON"] = {"America/Edmonton", locate_zone("America/Edmonton")};
		_cached_zones["AMERICA/EIRUNEPE"] = {"America/Eirunepe", locate_zone("America/Eirunepe")};
		_cached_zones["AMERICA/EL_SALVADOR"] = {"America/El_Salvador", locate_zone("America/El_Salvador")};
		_cached_zones["AMERICA/ENSENADA"] = {"America/Ensenada", locate_zone("America/Ensenada")};
		_cached_zones["AMERICA/FORT_NELSON"] = {"America/Fort_Nelson", locate_zone("America/Fort_Nelson")};
		_cached_zones["AMERICA/FORT_WAYNE"] = {"America/Fort_Wayne", locate_zone("America/Fort_Wayne")};
		_cached_zones["AMERICA/FORTALEZA"] = {"America/Fortaleza", locate_zone("America/Fortaleza")};
		_cached_zones["AMERICA/GLACE_BAY"] = {"America/Glace_Bay", locate_zone("America/Glace_Bay")};
		_cached_zones["AMERICA/GODTHAB"] = {"America/Godthab", locate_zone("America/Godthab")};
		_cached_zones["AMERICA/GOOSE_BAY"] = {"America/Goose_Bay", locate_zone("America/Goose_Bay")};
		_cached_zones["AMERICA/GRAND_TURK"] = {"America/Grand_Turk", locate_zone("America/Grand_Turk")};
		_cached_zones["AMERICA/GRENADA"] = {"America/Grenada", locate_zone("America/Grenada")};
		_cached_zones["AMERICA/GUADELOUPE"] = {"America/Guadeloupe", locate_zone("America/Guadeloupe")};
		_cached_zones["AMERICA/GUATEMALA"] = {"America/Guatemala", locate_zone("America/Guatemala")};
		_cached_zones["AMERICA/GUAYAQUIL"] = {"America/Guayaquil", locate_zone("America/Guayaquil")};
		_cached_zones["AMERICA/GUYANA"] = {"America/Guyana", locate_zone("America/Guyana")};
		_cached_zones["AMERICA/HALIFAX"] = {"America/Halifax", locate_zone("America/Halifax")};
		_cached_zones["AMERICA/HAVANA"] = {"America/Havana", locate_zone("America/Havana")};
		_cached_zones["AMERICA/HERMOSILLO"] = {"America/Hermosillo", locate_zone("America/Hermosillo")};
		_cached_zones["AMERICA/INDIANA/INDIANAPOLIS"] = {"America/Indiana/Indianapolis", locate_zone("America/Indiana/Indianapolis")};
		_cached_zones["AMERICA/INDIANA/KNOX"] = {"America/Indiana/Knox", locate_zone("America/Indiana/Knox")};
		_cached_zones["AMERICA/INDIANA/MARENGO"] = {"America/Indiana/Marengo", locate_zone("America/Indiana/Marengo")};
		_cached_zones["AMERICA/INDIANA/PETERSBURG"] = {"America/Indiana/Petersburg", locate_zone("America/Indiana/Petersburg")};
		_cached_zones["AMERICA/INDIANA/TELL_CITY"] = {"America/Indiana/Tell_City", locate_zone("America/Indiana/Tell_City")};
		_cached_zones["AMERICA/INDIANA/VEVAY"] = {"America/Indiana/Vevay", locate_zone("America/Indiana/Vevay")};
		_cached_zones["AMERICA/INDIANA/VINCENNES"] = {"America/Indiana/Vincennes", locate_zone("America/Indiana/Vincennes")};
		_cached_zones["AMERICA/INDIANA/WINAMAC"] = {"America/Indiana/Winamac", locate_zone("America/Indiana/Winamac")};
		_cached_zones["AMERICA/INDIANAPOLIS"] = {"America/Indianapolis", locate_zone("America/Indianapolis")};
		_cached_zones["AMERICA/INUVIK"] = {"America/Inuvik", locate_zone("America/Inuvik")};
		_cached_zones["AMERICA/IQALUIT"] = {"America/Iqaluit", locate_zone("America/Iqaluit")};
		_cached_zones["AMERICA/JAMAICA"] = {"America/Jamaica", locate_zone("America/Jamaica")};
		_cached_zones["AMERICA/JUJUY"] = {"America/Jujuy", locate_zone("America/Jujuy")};
		_cached_zones["AMERICA/JUNEAU"] = {"America/Juneau", locate_zone("America/Juneau")};
		_cached_zones["AMERICA/KENTUCKY/LOUISVILLE"] = {"America/Kentucky/Louisville", locate_zone("America/Kentucky/Louisville")};
		_cached_zones["AMERICA/KENTUCKY/MONTICELLO"] = {"America/Kentucky/Monticello", locate_zone("America/Kentucky/Monticello")};
		_cached_zones["AMERICA/KNOX_IN"] = {"America/Knox_IN", locate_zone("America/Knox_IN")};
		_cached_zones["AMERICA/KRALENDIJK"] = {"America/Kralendijk", locate_zone("America/Kralendijk")};
		_cached_zones["AMERICA/LA_PAZ"] = {"America/La_Paz", locate_zone("America/La_Paz")};
		_cached_zones["AMERICA/LIMA"] = {"America/Lima", locate_zone("America/Lima")};
		_cached_zones["AMERICA/LOS_ANGELES"] = {"America/Los_Angeles", locate_zone("America/Los_Angeles")};
		_cached_zones["AMERICA/LOUISVILLE"] = {"America/Louisville", locate_zone("America/Louisville")};
		_cached_zones["AMERICA/LOWER_PRINCES"] = {"America/Lower_Princes", locate_zone("America/Lower_Princes")};
		_cached_zones["AMERICA/MACEIO"] = {"America/Maceio", locate_zone("America/Maceio")};
		_cached_zones["AMERICA/MANAGUA"] = {"America/Managua", locate_zone("America/Managua")};
		_cached_zones["AMERICA/MANAUS"] = {"America/Manaus", locate_zone("America/Manaus")};
		_cached_zones["AMERICA/MARIGOT"] = {"America/Marigot", locate_zone("America/Marigot")};
		_cached_zones["AMERICA/MARTINIQUE"] = {"America/Martinique", locate_zone("America/Martinique")};
		_cached_zones["AMERICA/MATAMOROS"] = {"America/Matamoros", locate_zone("America/Matamoros")};
		_cached_zones["AMERICA/MAZATLAN"] = {"America/Mazatlan", locate_zone("America/Mazatlan")};
		_cached_zones["AMERICA/MENDOZA"] = {"America/Mendoza", locate_zone("America/Mendoza")};
		_cached_zones["AMERICA/MENOMINEE"] = {"America/Menominee", locate_zone("America/Menominee")};
		_cached_zones["AMERICA/MERIDA"] = {"America/Merida", locate_zone("America/Merida")};
		_cached_zones["AMERICA/METLAKATLA"] = {"America/Metlakatla", locate_zone("America/Metlakatla")};
		_cached_zones["AMERICA/MEXICO_CITY"] = {"America/Mexico_City", locate_zone("America/Mexico_City")};
		_cached_zones["AMERICA/MIQUELON"] = {"America/Miquelon", locate_zone("America/Miquelon")};
		_cached_zones["AMERICA/MONCTON"] = {"America/Moncton", locate_zone("America/Moncton")};
		_cached_zones["AMERICA/MONTERREY"] = {"America/Monterrey", locate_zone("America/Monterrey")};
		_cached_zones["AMERICA/MONTEVIDEO"] = {"America/Montevideo", locate_zone("America/Montevideo")};
		_cached_zones["AMERICA/MONTREAL"] = {"America/Montreal", locate_zone("America/Montreal")};
		_cached_zones["AMERICA/MONTSERRAT"] = {"America/Montserrat", locate_zone("America/Montserrat")};
		_cached_zones["AMERICA/NASSAU"] = {"America/Nassau", locate_zone("America/Nassau")};
		_cached_zones["AMERICA/NEW_YORK"] = {"America/New_York", locate_zone("America/New_York")};
		_cached_zones["AMERICA/NIPIGON"] = {"America/Nipigon", locate_zone("America/Nipigon")};
		_cached_zones["AMERICA/NOME"] = {"America/Nome", locate_zone("America/Nome")};
		_cached_zones["AMERICA/NORONHA"] = {"America/Noronha", locate_zone("America/Noronha")};
		_cached_zones["AMERICA/NORTH_DAKOTA/BEULAH"] = {"America/North_Dakota/Beulah", locate_zone("America/North_Dakota/Beulah")};
		_cached_zones["AMERICA/NORTH_DAKOTA/CENTER"] = {"America/North_Dakota/Center", locate_zone("America/North_Dakota/Center")};
		_cached_zones["AMERICA/NORTH_DAKOTA/NEW_SALEM"] = {"America/North_Dakota/New_Salem", locate_zone("America/North_Dakota/New_Salem")};
		_cached_zones["AMERICA/OJINAGA"] = {"America/Ojinaga", locate_zone("America/Ojinaga")};
		_cached_zones["AMERICA/PANAMA"] = {"America/Panama", locate_zone("America/Panama")};
		_cached_zones["AMERICA/PANGNIRTUNG"] = {"America/Pangnirtung", locate_zone("America/Pangnirtung")};
		_cached_zones["AMERICA/PARAMARIBO"] = {"America/Paramaribo", locate_zone("America/Paramaribo")};
		_cached_zones["AMERICA/PHOENIX"] = {"America/Phoenix", locate_zone("America/Phoenix")};
		_cached_zones["AMERICA/PORT-AU-PRINCE"] = {"America/Port-au-Prince", locate_zone("America/Port-au-Prince")};
		_cached_zones["AMERICA/PORT_OF_SPAIN"] = {"America/Port_of_Spain", locate_zone("America/Port_of_Spain")};
		_cached_zones["AMERICA/PORTO_ACRE"] = {"America/Porto_Acre", locate_zone("America/Porto_Acre")};
		_cached_zones["AMERICA/PORTO_VELHO"] = {"America/Porto_Velho", locate_zone("America/Porto_Velho")};
		_cached_zones["AMERICA/PUERTO_RICO"] = {"America/Puerto_Rico", locate_zone("America/Puerto_Rico")};
		_cached_zones["AMERICA/PUNTA_ARENAS"] = {"America/Punta_Arenas", locate_zone("America/Punta_Arenas")};
		_cached_zones["AMERICA/RAINY_RIVER"] = {"America/Rainy_River", locate_zone("America/Rainy_River")};
		_cached_zones["AMERICA/RANKIN_INLET"] = {"America/Rankin_Inlet", locate_zone("America/Rankin_Inlet")};
		_cached_zones["AMERICA/RECIFE"] = {"America/Recife", locate_zone("America/Recife")};
		_cached_zones["AMERICA/REGINA"] = {"America/Regina", locate_zone("America/Regina")};
		_cached_zones["AMERICA/RESOLUTE"] = {"America/Resolute", locate_zone("America/Resolute")};
		_cached_zones["AMERICA/RIO_BRANCO"] = {"America/Rio_Branco", locate_zone("America/Rio_Branco")};
		_cached_zones["AMERICA/ROSARIO"] = {"America/Rosario", locate_zone("America/Rosario")};
		_cached_zones["AMERICA/SANTA_ISABEL"] = {"America/Santa_Isabel", locate_zone("America/Santa_Isabel")};
		_cached_zones["AMERICA/SANTAREM"] = {"America/Santarem", locate_zone("America/Santarem")};
		_cached_zones["AMERICA/SANTIAGO"] = {"America/Santiago", locate_zone("America/Santiago")};
		_cached_zones["AMERICA/SANTO_DOMINGO"] = {"America/Santo_Domingo", locate_zone("America/Santo_Domingo")};
		_cached_zones["AMERICA/SAO_PAULO"] = {"America/Sao_Paulo", locate_zone("America/Sao_Paulo")};
		_cached_zones["AMERICA/SCORESBYSUND"] = {"America/Scoresbysund", locate_zone("America/Scoresbysund")};
		_cached_zones["AMERICA/SHIPROCK"] = {"America/Shiprock", locate_zone("America/Shiprock")};
		_cached_zones["AMERICA/SITKA"] = {"America/Sitka", locate_zone("America/Sitka")};
		_cached_zones["AMERICA/ST_BARTHELEMY"] = {"America/St_Barthelemy", locate_zone("America/St_Barthelemy")};
		_cached_zones["AMERICA/ST_JOHNS"] = {"America/St_Johns", locate_zone("America/St_Johns")};
		_cached_zones["AMERICA/ST_KITTS"] = {"America/St_Kitts", locate_zone("America/St_Kitts")};
		_cached_zones["AMERICA/ST_LUCIA"] = {"America/St_Lucia", locate_zone("America/St_Lucia")};
		_cached_zones["AMERICA/ST_THOMAS"] = {"America/St_Thomas", locate_zone("America/St_Thomas")};
		_cached_zones["AMERICA/ST_VINCENT"] = {"America/St_Vincent", locate_zone("America/St_Vincent")};
		_cached_zones["AMERICA/SWIFT_CURRENT"] = {"America/Swift_Current", locate_zone("America/Swift_Current")};
		_cached_zones["AMERICA/TEGUCIGALPA"] = {"America/Tegucigalpa", locate_zone("America/Tegucigalpa")};
		_cached_zones["AMERICA/THULE"] = {"America/Thule", locate_zone("America/Thule")};
		_cached_zones["AMERICA/THUNDER_BAY"] = {"America/Thunder_Bay", locate_zone("America/Thunder_Bay")};
		_cached_zones["AMERICA/TIJUANA"] = {"America/Tijuana", locate_zone("America/Tijuana")};
		_cached_zones["AMERICA/TORONTO"] = {"America/Toronto", locate_zone("America/Toronto")};
		_cached_zones["AMERICA/TORTOLA"] = {"America/Tortola", locate_zone("America/Tortola")};
		_cached_zones["AMERICA/VANCOUVER"] = {"America/Vancouver", locate_zone("America/Vancouver")};
		_cached_zones["AMERICA/VIRGIN"] = {"America/Virgin", locate_zone("America/Virgin")};
		_cached_zones["AMERICA/WHITEHORSE"] = {"America/Whitehorse", locate_zone("America/Whitehorse")};
		_cached_zones["AMERICA/WINNIPEG"] = {"America/Winnipeg", locate_zone("America/Winnipeg")};
		_cached_zones["AMERICA/YAKUTAT"] = {"America/Yakutat", locate_zone("America/Yakutat")};
		_cached_zones["AMERICA/YELLOWKNIFE"] = {"America/Yellowknife", locate_zone("America/Yellowknife")};
		_cached_zones["ANTARCTICA/CASEY"] = {"Antarctica/Casey", locate_zone("Antarctica/Casey")};
		_cached_zones["ANTARCTICA/DAVIS"] = {"Antarctica/Davis", locate_zone("Antarctica/Davis")};
		_cached_zones["ANTARCTICA/DUMONTDURVILLE"] = {"Antarctica/DumontDUrville", locate_zone("Antarctica/DumontDUrville")};
		_cached_zones["ANTARCTICA/MACQUARIE"] = {"Antarctica/Macquarie", locate_zone("Antarctica/Macquarie")};
		_cached_zones["ANTARCTICA/MAWSON"] = {"Antarctica/Mawson", locate_zone("Antarctica/Mawson")};
		_cached_zones["ANTARCTICA/MCMURDO"] = {"Antarctica/McMurdo", locate_zone("Antarctica/McMurdo")};
		_cached_zones["ANTARCTICA/PALMER"] = {"Antarctica/Palmer", locate_zone("Antarctica/Palmer")};
		_cached_zones["ANTARCTICA/ROTHERA"] = {"Antarctica/Rothera", locate_zone("Antarctica/Rothera")};
		_cached_zones["ANTARCTICA/SOUTH_POLE"] = {"Antarctica/South_Pole", locate_zone("Antarctica/South_Pole")};
		_cached_zones["ANTARCTICA/SYOWA"] = {"Antarctica/Syowa", locate_zone("Antarctica/Syowa")};
		_cached_zones["ANTARCTICA/TROLL"] = {"Antarctica/Troll", locate_zone("Antarctica/Troll")};
		_cached_zones["ANTARCTICA/VOSTOK"] = {"Antarctica/Vostok", locate_zone("Antarctica/Vostok")};
		_cached_zones["ARCTIC/LONGYEARBYEN"] = {"Arctic/Longyearbyen", locate_zone("Arctic/Longyearbyen")};
		_cached_zones["ASIA/ADEN"] = {"Asia/Aden", locate_zone("Asia/Aden")};
		_cached_zones["ASIA/ALMATY"] = {"Asia/Almaty", locate_zone("Asia/Almaty")};
		_cached_zones["ASIA/AMMAN"] = {"Asia/Amman", locate_zone("Asia/Amman")};
		_cached_zones["ASIA/ANADYR"] = {"Asia/Anadyr", locate_zone("Asia/Anadyr")};
		_cached_zones["ASIA/AQTAU"] = {"Asia/Aqtau", locate_zone("Asia/Aqtau")};
		_cached_zones["ASIA/AQTOBE"] = {"Asia/Aqtobe", locate_zone("Asia/Aqtobe")};
		_cached_zones["ASIA/ASHGABAT"] = {"Asia/Ashgabat", locate_zone("Asia/Ashgabat")};
		_cached_zones["ASIA/ASHKHABAD"] = {"Asia/Ashkhabad", locate_zone("Asia/Ashkhabad")};
		_cached_zones["ASIA/ATYRAU"] = {"Asia/Atyrau", locate_zone("Asia/Atyrau")};
		_cached_zones["ASIA/BAGHDAD"] = {"Asia/Baghdad", locate_zone("Asia/Baghdad")};
		_cached_zones["ASIA/BAHRAIN"] = {"Asia/Bahrain", locate_zone("Asia/Bahrain")};
		_cached_zones["ASIA/BAKU"] = {"Asia/Baku", locate_zone("Asia/Baku")};
		_cached_zones["ASIA/BANGKOK"] = {"Asia/Bangkok", locate_zone("Asia/Bangkok")};
		_cached_zones["ASIA/BARNAUL"] = {"Asia/Barnaul", locate_zone("Asia/Barnaul")};
		_cached_zones["ASIA/BEIRUT"] = {"Asia/Beirut", locate_zone("Asia/Beirut")};
		_cached_zones["ASIA/BISHKEK"] = {"Asia/Bishkek", locate_zone("Asia/Bishkek")};
		_cached_zones["ASIA/BRUNEI"] = {"Asia/Brunei", locate_zone("Asia/Brunei")};
		_cached_zones["ASIA/CALCUTTA"] = {"Asia/Calcutta", locate_zone("Asia/Calcutta")};
		_cached_zones["ASIA/CHITA"] = {"Asia/Chita", locate_zone("Asia/Chita")};
		_cached_zones["ASIA/CHOIBALSAN"] = {"Asia/Choibalsan", locate_zone("Asia/Choibalsan")};
		_cached_zones["ASIA/CHONGQING"] = {"Asia/Chongqing", locate_zone("Asia/Chongqing")};
		_cached_zones["ASIA/CHUNGKING"] = {"Asia/Chungking", locate_zone("Asia/Chungking")};
		_cached_zones["ASIA/COLOMBO"] = {"Asia/Colombo", locate_zone("Asia/Colombo")};
		_cached_zones["ASIA/DACCA"] = {"Asia/Dacca", locate_zone("Asia/Dacca")};
		_cached_zones["ASIA/DAMASCUS"] = {"Asia/Damascus", locate_zone("Asia/Damascus")};
		_cached_zones["ASIA/DHAKA"] = {"Asia/Dhaka", locate_zone("Asia/Dhaka")};
		_cached_zones["ASIA/DILI"] = {"Asia/Dili", locate_zone("Asia/Dili")};
		_cached_zones["ASIA/DUBAI"] = {"Asia/Dubai", locate_zone("Asia/Dubai")};
		_cached_zones["ASIA/DUSHANBE"] = {"Asia/Dushanbe", locate_zone("Asia/Dushanbe")};
		_cached_zones["ASIA/FAMAGUSTA"] = {"Asia/Famagusta", locate_zone("Asia/Famagusta")};
		_cached_zones["ASIA/GAZA"] = {"Asia/Gaza", locate_zone("Asia/Gaza")};
		_cached_zones["ASIA/HARBIN"] = {"Asia/Harbin", locate_zone("Asia/Harbin")};
		_cached_zones["ASIA/HEBRON"] = {"Asia/Hebron", locate_zone("Asia/Hebron")};
		_cached_zones["ASIA/HO_CHI_MINH"] = {"Asia/Ho_Chi_Minh", locate_zone("Asia/Ho_Chi_Minh")};
		_cached_zones["ASIA/HONG_KONG"] = {"Asia/Hong_Kong", locate_zone("Asia/Hong_Kong")};
		_cached_zones["ASIA/HOVD"] = {"Asia/Hovd", locate_zone("Asia/Hovd")};
		_cached_zones["ASIA/IRKUTSK"] = {"Asia/Irkutsk", locate_zone("Asia/Irkutsk")};
		_cached_zones["ASIA/ISTANBUL"] = {"Asia/Istanbul", locate_zone("Asia/Istanbul")};
		_cached_zones["ASIA/JAKARTA"] = {"Asia/Jakarta", locate_zone("Asia/Jakarta")};
		_cached_zones["ASIA/JAYAPURA"] = {"Asia/Jayapura", locate_zone("Asia/Jayapura")};
		_cached_zones["ASIA/JERUSALEM"] = {"Asia/Jerusalem", locate_zone("Asia/Jerusalem")};
		_cached_zones["ASIA/KABUL"] = {"Asia/Kabul", locate_zone("Asia/Kabul")};
		_cached_zones["ASIA/KAMCHATKA"] = {"Asia/Kamchatka", locate_zone("Asia/Kamchatka")};
		_cached_zones["ASIA/KARACHI"] = {"Asia/Karachi", locate_zone("Asia/Karachi")};
		_cached_zones["ASIA/KASHGAR"] = {"Asia/Kashgar", locate_zone("Asia/Kashgar")};
		_cached_zones["ASIA/KATHMANDU"] = {"Asia/Kathmandu", locate_zone("Asia/Kathmandu")};
		_cached_zones["ASIA/KATMANDU"] = {"Asia/Katmandu", locate_zone("Asia/Katmandu")};
		_cached_zones["ASIA/KHANDYGA"] = {"Asia/Khandyga", locate_zone("Asia/Khandyga")};
		_cached_zones["ASIA/KOLKATA"] = {"Asia/Kolkata", locate_zone("Asia/Kolkata")};
		_cached_zones["ASIA/KRASNOYARSK"] = {"Asia/Krasnoyarsk", locate_zone("Asia/Krasnoyarsk")};
		_cached_zones["ASIA/KUALA_LUMPUR"] = {"Asia/Kuala_Lumpur", locate_zone("Asia/Kuala_Lumpur")};
		_cached_zones["ASIA/KUCHING"] = {"Asia/Kuching", locate_zone("Asia/Kuching")};
		_cached_zones["ASIA/KUWAIT"] = {"Asia/Kuwait", locate_zone("Asia/Kuwait")};
		_cached_zones["ASIA/MACAO"] = {"Asia/Macao", locate_zone("Asia/Macao")};
		_cached_zones["ASIA/MACAU"] = {"Asia/Macau", locate_zone("Asia/Macau")};
		_cached_zones["ASIA/MAGADAN"] = {"Asia/Magadan", locate_zone("Asia/Magadan")};
		_cached_zones["ASIA/MAKASSAR"] = {"Asia/Makassar", locate_zone("Asia/Makassar")};
		_cached_zones["ASIA/MANILA"] = {"Asia/Manila", locate_zone("Asia/Manila")};
		_cached_zones["ASIA/MUSCAT"] = {"Asia/Muscat", locate_zone("Asia/Muscat")};
		_cached_zones["ASIA/NICOSIA"] = {"Asia/Nicosia", locate_zone("Asia/Nicosia")};
		_cached_zones["ASIA/NOVOKUZNETSK"] = {"Asia/Novokuznetsk", locate_zone("Asia/Novokuznetsk")};
		_cached_zones["ASIA/NOVOSIBIRSK"] = {"Asia/Novosibirsk", locate_zone("Asia/Novosibirsk")};
		_cached_zones["ASIA/OMSK"] = {"Asia/Omsk", locate_zone("Asia/Omsk")};
		_cached_zones["ASIA/ORAL"] = {"Asia/Oral", locate_zone("Asia/Oral")};
		_cached_zones["ASIA/PHNOM_PENH"] = {"Asia/Phnom_Penh", locate_zone("Asia/Phnom_Penh")};
		_cached_zones["ASIA/PONTIANAK"] = {"Asia/Pontianak", locate_zone("Asia/Pontianak")};
		_cached_zones["ASIA/PYONGYANG"] = {"Asia/Pyongyang", locate_zone("Asia/Pyongyang")};
		_cached_zones["ASIA/QATAR"] = {"Asia/Qatar", locate_zone("Asia/Qatar")};
		_cached_zones["ASIA/QYZYLORDA"] = {"Asia/Qyzylorda", locate_zone("Asia/Qyzylorda")};
		_cached_zones["ASIA/RANGOON"] = {"Asia/Rangoon", locate_zone("Asia/Rangoon")};
		_cached_zones["ASIA/RIYADH"] = {"Asia/Riyadh", locate_zone("Asia/Riyadh")};
		_cached_zones["ASIA/SAIGON"] = {"Asia/Saigon", locate_zone("Asia/Saigon")};
		_cached_zones["ASIA/SAKHALIN"] = {"Asia/Sakhalin", locate_zone("Asia/Sakhalin")};
		_cached_zones["ASIA/SAMARKAND"] = {"Asia/Samarkand", locate_zone("Asia/Samarkand")};
		_cached_zones["ASIA/SEOUL"] = {"Asia/Seoul", locate_zone("Asia/Seoul")};
		_cached_zones["ASIA/SHANGHAI"] = {"Asia/Shanghai", locate_zone("Asia/Shanghai")};
		_cached_zones["ASIA/SINGAPORE"] = {"Asia/Singapore", locate_zone("Asia/Singapore")};
		_cached_zones["ASIA/SREDNEKOLYMSK"] = {"Asia/Srednekolymsk", locate_zone("Asia/Srednekolymsk")};
		_cached_zones["ASIA/TAIPEI"] = {"Asia/Taipei", locate_zone("Asia/Taipei")};
		_cached_zones["ASIA/TASHKENT"] = {"Asia/Tashkent", locate_zone("Asia/Tashkent")};
		_cached_zones["ASIA/TBILISI"] = {"Asia/Tbilisi", locate_zone("Asia/Tbilisi")};
		_cached_zones["ASIA/TEHRAN"] = {"Asia/Tehran", locate_zone("Asia/Tehran")};
		_cached_zones["ASIA/TEL_AVIV"] = {"Asia/Tel_Aviv", locate_zone("Asia/Tel_Aviv")};
		_cached_zones["ASIA/THIMBU"] = {"Asia/Thimbu", locate_zone("Asia/Thimbu")};
		_cached_zones["ASIA/THIMPHU"] = {"Asia/Thimphu", locate_zone("Asia/Thimphu")};
		_cached_zones["ASIA/TOKYO"] = {"Asia/Tokyo", locate_zone("Asia/Tokyo")};
		_cached_zones["ASIA/TOMSK"] = {"Asia/Tomsk", locate_zone("Asia/Tomsk")};
		_cached_zones["ASIA/UJUNG_PANDANG"] = {"Asia/Ujung_Pandang", locate_zone("Asia/Ujung_Pandang")};
		_cached_zones["ASIA/ULAANBAATAR"] = {"Asia/Ulaanbaatar", locate_zone("Asia/Ulaanbaatar")};
		_cached_zones["ASIA/ULAN_BATOR"] = {"Asia/Ulan_Bator", locate_zone("Asia/Ulan_Bator")};
		_cached_zones["ASIA/URUMQI"] = {"Asia/Urumqi", locate_zone("Asia/Urumqi")};
		_cached_zones["ASIA/UST-NERA"] = {"Asia/Ust-Nera", locate_zone("Asia/Ust-Nera")};
		_cached_zones["ASIA/VIENTIANE"] = {"Asia/Vientiane", locate_zone("Asia/Vientiane")};
		_cached_zones["ASIA/VLADIVOSTOK"] = {"Asia/Vladivostok", locate_zone("Asia/Vladivostok")};
		_cached_zones["ASIA/YAKUTSK"] = {"Asia/Yakutsk", locate_zone("Asia/Yakutsk")};
		_cached_zones["ASIA/YANGON"] = {"Asia/Yangon", locate_zone("Asia/Yangon")};
		_cached_zones["ASIA/YEKATERINBURG"] = {"Asia/Yekaterinburg", locate_zone("Asia/Yekaterinburg")};
		_cached_zones["ASIA/YEREVAN"] = {"Asia/Yerevan", locate_zone("Asia/Yerevan")};
		_cached_zones["ATLANTIC/AZORES"] = {"Atlantic/Azores", locate_zone("Atlantic/Azores")};
		_cached_zones["ATLANTIC/BERMUDA"] = {"Atlantic/Bermuda", locate_zone("Atlantic/Bermuda")};
		_cached_zones["ATLANTIC/CANARY"] = {"Atlantic/Canary", locate_zone("Atlantic/Canary")};
		_cached_zones["ATLANTIC/CAPE_VERDE"] = {"Atlantic/Cape_Verde", locate_zone("Atlantic/Cape_Verde")};
		_cached_zones["ATLANTIC/FAEROE"] = {"Atlantic/Faeroe", locate_zone("Atlantic/Faeroe")};
		_cached_zones["ATLANTIC/FAROE"] = {"Atlantic/Faroe", locate_zone("Atlantic/Faroe")};
		_cached_zones["ATLANTIC/JAN_MAYEN"] = {"Atlantic/Jan_Mayen", locate_zone("Atlantic/Jan_Mayen")};
		_cached_zones["ATLANTIC/MADEIRA"] = {"Atlantic/Madeira", locate_zone("Atlantic/Madeira")};
		_cached_zones["ATLANTIC/REYKJAVIK"] = {"Atlantic/Reykjavik", locate_zone("Atlantic/Reykjavik")};
		_cached_zones["ATLANTIC/SOUTH_GEORGIA"] = {"Atlantic/South_Georgia", locate_zone("Atlantic/South_Georgia")};
		_cached_zones["ATLANTIC/ST_HELENA"] = {"Atlantic/St_Helena", locate_zone("Atlantic/St_Helena")};
		_cached_zones["ATLANTIC/STANLEY"] = {"Atlantic/Stanley", locate_zone("Atlantic/Stanley")};
		_cached_zones["AUSTRALIA/ACT"] = {"Australia/ACT", locate_zone("Australia/ACT")};
		_cached_zones["AUSTRALIA/ADELAIDE"] = {"Australia/Adelaide", locate_zone("Australia/Adelaide")};
		_cached_zones["AUSTRALIA/BRISBANE"] = {"Australia/Brisbane", locate_zone("Australia/Brisbane")};
		_cached_zones["AUSTRALIA/BROKEN_HILL"] = {"Australia/Broken_Hill", locate_zone("Australia/Broken_Hill")};
		_cached_zones["AUSTRALIA/CANBERRA"] = {"Australia/Canberra", locate_zone("Australia/Canberra")};
		_cached_zones["AUSTRALIA/CURRIE"] = {"Australia/Currie", locate_zone("Australia/Currie")};
		_cached_zones["AUSTRALIA/DARWIN"] = {"Australia/Darwin", locate_zone("Australia/Darwin")};
		_cached_zones["AUSTRALIA/EUCLA"] = {"Australia/Eucla", locate_zone("Australia/Eucla")};
		_cached_zones["AUSTRALIA/HOBART"] = {"Australia/Hobart", locate_zone("Australia/Hobart")};
		_cached_zones["AUSTRALIA/LHI"] = {"Australia/LHI", locate_zone("Australia/LHI")};
		_cached_zones["AUSTRALIA/LINDEMAN"] = {"Australia/Lindeman", locate_zone("Australia/Lindeman")};
		_cached_zones["AUSTRALIA/LORD_HOWE"] = {"Australia/Lord_Howe", locate_zone("Australia/Lord_Howe")};
		_cached_zones["AUSTRALIA/MELBOURNE"] = {"Australia/Melbourne", locate_zone("Australia/Melbourne")};
		_cached_zones["AUSTRALIA/NSW"] = {"Australia/NSW", locate_zone("Australia/NSW")};
		_cached_zones["AUSTRALIA/NORTH"] = {"Australia/North", locate_zone("Australia/North")};
		_cached_zones["AUSTRALIA/PERTH"] = {"Australia/Perth", locate_zone("Australia/Perth")};
		_cached_zones["AUSTRALIA/QUEENSLAND"] = {"Australia/Queensland", locate_zone("Australia/Queensland")};
		_cached_zones["AUSTRALIA/SOUTH"] = {"Australia/South", locate_zone("Australia/South")};
		_cached_zones["AUSTRALIA/SYDNEY"] = {"Australia/Sydney", locate_zone("Australia/Sydney")};
		_cached_zones["AUSTRALIA/TASMANIA"] = {"Australia/Tasmania", locate_zone("Australia/Tasmania")};
		_cached_zones["AUSTRALIA/VICTORIA"] = {"Australia/Victoria", locate_zone("Australia/Victoria")};
		_cached_zones["AUSTRALIA/WEST"] = {"Australia/West", locate_zone("Australia/West")};
		_cached_zones["AUSTRALIA/YANCOWINNA"] = {"Australia/Yancowinna", locate_zone("Australia/Yancowinna")};
		_cached_zones["BRAZIL/ACRE"] = {"Brazil/Acre", locate_zone("Brazil/Acre")};
		_cached_zones["BRAZIL/DENORONHA"] = {"Brazil/DeNoronha", locate_zone("Brazil/DeNoronha")};
		_cached_zones["BRAZIL/EAST"] = {"Brazil/East", locate_zone("Brazil/East")};
		_cached_zones["BRAZIL/WEST"] = {"Brazil/West", locate_zone("Brazil/West")};
		_cached_zones["CET"] = {"CET", locate_zone("CET")};
		_cached_zones["CST6CDT"] = {"CST6CDT", locate_zone("CST6CDT")};
		_cached_zones["CANADA/ATLANTIC"] = {"Canada/Atlantic", locate_zone("Canada/Atlantic")};
		_cached_zones["CANADA/CENTRAL"] = {"Canada/Central", locate_zone("Canada/Central")};
		_cached_zones["CANADA/EASTERN"] = {"Canada/Eastern", locate_zone("Canada/Eastern")};
		_cached_zones["CANADA/MOUNTAIN"] = {"Canada/Mountain", locate_zone("Canada/Mountain")};
		_cached_zones["CANADA/NEWFOUNDLAND"] = {"Canada/Newfoundland", locate_zone("Canada/Newfoundland")};
		_cached_zones["CANADA/PACIFIC"] = {"Canada/Pacific", locate_zone("Canada/Pacific")};
		_cached_zones["CANADA/SASKATCHEWAN"] = {"Canada/Saskatchewan", locate_zone("Canada/Saskatchewan")};
		_cached_zones["CANADA/YUKON"] = {"Canada/Yukon", locate_zone("Canada/Yukon")};
		_cached_zones["CHILE/CONTINENTAL"] = {"Chile/Continental", locate_zone("Chile/Continental")};
		_cached_zones["CHILE/EASTERISLAND"] = {"Chile/EasterIsland", locate_zone("Chile/EasterIsland")};
		_cached_zones["CUBA"] = {"Cuba", locate_zone("Cuba")};
		_cached_zones["EET"] = {"EET", locate_zone("EET")};
		_cached_zones["EST"] = {"EST", locate_zone("EST")};
		_cached_zones["EST5EDT"] = {"EST5EDT", locate_zone("EST5EDT")};
		_cached_zones["EGYPT"] = {"Egypt", locate_zone("Egypt")};
		_cached_zones["EIRE"] = {"Eire", locate_zone("Eire")};
		_cached_zones["ETC/GMT"] = {"Etc/GMT", locate_zone("Etc/GMT")};
		_cached_zones["ETC/GMT+0"] = {"Etc/GMT+0", locate_zone("Etc/GMT+0")};
		_cached_zones["ETC/GMT+1"] = {"Etc/GMT+1", locate_zone("Etc/GMT+1")};
		_cached_zones["ETC/GMT+10"] = {"Etc/GMT+10", locate_zone("Etc/GMT+10")};
		_cached_zones["ETC/GMT+11"] = {"Etc/GMT+11", locate_zone("Etc/GMT+11")};
		_cached_zones["ETC/GMT+12"] = {"Etc/GMT+12", locate_zone("Etc/GMT+12")};
		_cached_zones["ETC/GMT+2"] = {"Etc/GMT+2", locate_zone("Etc/GMT+2")};
		_cached_zones["ETC/GMT+3"] = {"Etc/GMT+3", locate_zone("Etc/GMT+3")};
		_cached_zones["ETC/GMT+4"] = {"Etc/GMT+4", locate_zone("Etc/GMT+4")};
		_cached_zones["ETC/GMT+5"] = {"Etc/GMT+5", locate_zone("Etc/GMT+5")};
		_cached_zones["ETC/GMT+6"] = {"Etc/GMT+6", locate_zone("Etc/GMT+6")};
		_cached_zones["ETC/GMT+7"] = {"Etc/GMT+7", locate_zone("Etc/GMT+7")};
		_cached_zones["ETC/GMT+8"] = {"Etc/GMT+8", locate_zone("Etc/GMT+8")};
		_cached_zones["ETC/GMT+9"] = {"Etc/GMT+9", locate_zone("Etc/GMT+9")};
		_cached_zones["ETC/GMT-0"] = {"Etc/GMT-0", locate_zone("Etc/GMT-0")};
		_cached_zones["ETC/GMT-1"] = {"Etc/GMT-1", locate_zone("Etc/GMT-1")};
		_cached_zones["ETC/GMT-10"] = {"Etc/GMT-10", locate_zone("Etc/GMT-10")};
		_cached_zones["ETC/GMT-11"] = {"Etc/GMT-11", locate_zone("Etc/GMT-11")};
		_cached_zones["ETC/GMT-12"] = {"Etc/GMT-12", locate_zone("Etc/GMT-12")};
		_cached_zones["ETC/GMT-13"] = {"Etc/GMT-13", locate_zone("Etc/GMT-13")};
		_cached_zones["ETC/GMT-14"] = {"Etc/GMT-14", locate_zone("Etc/GMT-14")};
		_cached_zones["ETC/GMT-2"] = {"Etc/GMT-2", locate_zone("Etc/GMT-2")};
		_cached_zones["ETC/GMT-3"] = {"Etc/GMT-3", locate_zone("Etc/GMT-3")};
		_cached_zones["ETC/GMT-4"] = {"Etc/GMT-4", locate_zone("Etc/GMT-4")};
		_cached_zones["ETC/GMT-5"] = {"Etc/GMT-5", locate_zone("Etc/GMT-5")};
		_cached_zones["ETC/GMT-6"] = {"Etc/GMT-6", locate_zone("Etc/GMT-6")};
		_cached_zones["ETC/GMT-7"] = {"Etc/GMT-7", locate_zone("Etc/GMT-7")};
		_cached_zones["ETC/GMT-8"] = {"Etc/GMT-8", locate_zone("Etc/GMT-8")};
		_cached_zones["ETC/GMT-9"] = {"Etc/GMT-9", locate_zone("Etc/GMT-9")};
		_cached_zones["ETC/GMT0"] = {"Etc/GMT0", locate_zone("Etc/GMT0")};
		_cached_zones["ETC/GREENWICH"] = {"Etc/Greenwich", locate_zone("Etc/Greenwich")};
		_cached_zones["ETC/UCT"] = {"Etc/UCT", locate_zone("Etc/UCT")};
		_cached_zones["ETC/UTC"] = {"Etc/UTC", locate_zone("Etc/UTC")};
		_cached_zones["ETC/UNIVERSAL"] = {"Etc/Universal", locate_zone("Etc/Universal")};
		_cached_zones["ETC/ZULU"] = {"Etc/Zulu", locate_zone("Etc/Zulu")};
		_cached_zones["EUROPE/AMSTERDAM"] = {"Europe/Amsterdam", locate_zone("Europe/Amsterdam")};
		_cached_zones["EUROPE/ANDORRA"] = {"Europe/Andorra", locate_zone("Europe/Andorra")};
		_cached_zones["EUROPE/ASTRAKHAN"] = {"Europe/Astrakhan", locate_zone("Europe/Astrakhan")};
		_cached_zones["EUROPE/ATHENS"] = {"Europe/Athens", locate_zone("Europe/Athens")};
		_cached_zones["EUROPE/BELFAST"] = {"Europe/Belfast", locate_zone("Europe/Belfast")};
		_cached_zones["EUROPE/BELGRADE"] = {"Europe/Belgrade", locate_zone("Europe/Belgrade")};
		_cached_zones["EUROPE/BERLIN"] = {"Europe/Berlin", locate_zone("Europe/Berlin")};
		_cached_zones["EUROPE/BRATISLAVA"] = {"Europe/Bratislava", locate_zone("Europe/Bratislava")};
		_cached_zones["EUROPE/BRUSSELS"] = {"Europe/Brussels", locate_zone("Europe/Brussels")};
		_cached_zones["EUROPE/BUCHAREST"] = {"Europe/Bucharest", locate_zone("Europe/Bucharest")};
		_cached_zones["EUROPE/BUDAPEST"] = {"Europe/Budapest", locate_zone("Europe/Budapest")};
		_cached_zones["EUROPE/BUSINGEN"] = {"Europe/Busingen", locate_zone("Europe/Busingen")};
		_cached_zones["EUROPE/CHISINAU"] = {"Europe/Chisinau", locate_zone("Europe/Chisinau")};
		_cached_zones["EUROPE/COPENHAGEN"] = {"Europe/Copenhagen", locate_zone("Europe/Copenhagen")};
		_cached_zones["EUROPE/DUBLIN"] = {"Europe/Dublin", locate_zone("Europe/Dublin")};
		_cached_zones["EUROPE/GIBRALTAR"] = {"Europe/Gibraltar", locate_zone("Europe/Gibraltar")};
		_cached_zones["EUROPE/GUERNSEY"] = {"Europe/Guernsey", locate_zone("Europe/Guernsey")};
		_cached_zones["EUROPE/HELSINKI"] = {"Europe/Helsinki", locate_zone("Europe/Helsinki")};
		_cached_zones["EUROPE/ISLE_OF_MAN"] = {"Europe/Isle_of_Man", locate_zone("Europe/Isle_of_Man")};
		_cached_zones["EUROPE/ISTANBUL"] = {"Europe/Istanbul", locate_zone("Europe/Istanbul")};
		_cached_zones["EUROPE/JERSEY"] = {"Europe/Jersey", locate_zone("Europe/Jersey")};
		_cached_zones["EUROPE/KALININGRAD"] = {"Europe/Kaliningrad", locate_zone("Europe/Kaliningrad")};
		_cached_zones["EUROPE/KIEV"] = {"Europe/Kiev", locate_zone("Europe/Kiev")};
		_cached_zones["EUROPE/KIROV"] = {"Europe/Kirov", locate_zone("Europe/Kirov")};
		_cached_zones["EUROPE/LISBON"] = {"Europe/Lisbon", locate_zone("Europe/Lisbon")};
		_cached_zones["EUROPE/LJUBLJANA"] = {"Europe/Ljubljana", locate_zone("Europe/Ljubljana")};
		_cached_zones["EUROPE/LONDON"] = {"Europe/London", locate_zone("Europe/London")};
		_cached_zones["EUROPE/LUXEMBOURG"] = {"Europe/Luxembourg", locate_zone("Europe/Luxembourg")};
		_cached_zones["EUROPE/MADRID"] = {"Europe/Madrid", locate_zone("Europe/Madrid")};
		_cached_zones["EUROPE/MALTA"] = {"Europe/Malta", locate_zone("Europe/Malta")};
		_cached_zones["EUROPE/MARIEHAMN"] = {"Europe/Mariehamn", locate_zone("Europe/Mariehamn")};
		_cached_zones["EUROPE/MINSK"] = {"Europe/Minsk", locate_zone("Europe/Minsk")};
		_cached_zones["EUROPE/MONACO"] = {"Europe/Monaco", locate_zone("Europe/Monaco")};
		_cached_zones["EUROPE/MOSCOW"] = {"Europe/Moscow", locate_zone("Europe/Moscow")};
		_cached_zones["EUROPE/NICOSIA"] = {"Europe/Nicosia", locate_zone("Europe/Nicosia")};
		_cached_zones["EUROPE/OSLO"] = {"Europe/Oslo", locate_zone("Europe/Oslo")};
		_cached_zones["EUROPE/PARIS"] = {"Europe/Paris", locate_zone("Europe/Paris")};
		_cached_zones["EUROPE/PODGORICA"] = {"Europe/Podgorica", locate_zone("Europe/Podgorica")};
		_cached_zones["EUROPE/PRAGUE"] = {"Europe/Prague", locate_zone("Europe/Prague")};
		_cached_zones["EUROPE/RIGA"] = {"Europe/Riga", locate_zone("Europe/Riga")};
		_cached_zones["EUROPE/ROME"] = {"Europe/Rome", locate_zone("Europe/Rome")};
		_cached_zones["EUROPE/SAMARA"] = {"Europe/Samara", locate_zone("Europe/Samara")};
		_cached_zones["EUROPE/SAN_MARINO"] = {"Europe/San_Marino", locate_zone("Europe/San_Marino")};
		_cached_zones["EUROPE/SARAJEVO"] = {"Europe/Sarajevo", locate_zone("Europe/Sarajevo")};
		_cached_zones["EUROPE/SARATOV"] = {"Europe/Saratov", locate_zone("Europe/Saratov")};
		_cached_zones["EUROPE/SIMFEROPOL"] = {"Europe/Simferopol", locate_zone("Europe/Simferopol")};
		_cached_zones["EUROPE/SKOPJE"] = {"Europe/Skopje", locate_zone("Europe/Skopje")};
		_cached_zones["EUROPE/SOFIA"] = {"Europe/Sofia", locate_zone("Europe/Sofia")};
		_cached_zones["EUROPE/STOCKHOLM"] = {"Europe/Stockholm", locate_zone("Europe/Stockholm")};
		_cached_zones["EUROPE/TALLINN"] = {"Europe/Tallinn", locate_zone("Europe/Tallinn")};
		_cached_zones["EUROPE/TIRANE"] = {"Europe/Tirane", locate_zone("Europe/Tirane")};
		_cached_zones["EUROPE/TIRASPOL"] = {"Europe/Tiraspol", locate_zone("Europe/Tiraspol")};
		_cached_zones["EUROPE/ULYANOVSK"] = {"Europe/Ulyanovsk", locate_zone("Europe/Ulyanovsk")};
		_cached_zones["EUROPE/UZHGOROD"] = {"Europe/Uzhgorod", locate_zone("Europe/Uzhgorod")};
		_cached_zones["EUROPE/VADUZ"] = {"Europe/Vaduz", locate_zone("Europe/Vaduz")};
		_cached_zones["EUROPE/VATICAN"] = {"Europe/Vatican", locate_zone("Europe/Vatican")};
		_cached_zones["EUROPE/VIENNA"] = {"Europe/Vienna", locate_zone("Europe/Vienna")};
		_cached_zones["EUROPE/VILNIUS"] = {"Europe/Vilnius", locate_zone("Europe/Vilnius")};
		_cached_zones["EUROPE/VOLGOGRAD"] = {"Europe/Volgograd", locate_zone("Europe/Volgograd")};
		_cached_zones["EUROPE/WARSAW"] = {"Europe/Warsaw", locate_zone("Europe/Warsaw")};
		_cached_zones["EUROPE/ZAGREB"] = {"Europe/Zagreb", locate_zone("Europe/Zagreb")};
		_cached_zones["EUROPE/ZAPOROZHYE"] = {"Europe/Zaporozhye", locate_zone("Europe/Zaporozhye")};
		_cached_zones["EUROPE/ZURICH"] = {"Europe/Zurich", locate_zone("Europe/Zurich")};
		_cached_zones["GB"] = {"GB", locate_zone("GB")};
		_cached_zones["GB-EIRE"] = {"GB-Eire", locate_zone("GB-Eire")};
		_cached_zones["GMT"] = {"GMT", locate_zone("GMT")};
		_cached_zones["GMT+0"] = {"GMT+0", locate_zone("GMT+0")};
		_cached_zones["GMT-0"] = {"GMT-0", locate_zone("GMT-0")};
		_cached_zones["GMT0"] = {"GMT0", locate_zone("GMT0")};
		_cached_zones["GREENWICH"] = {"Greenwich", locate_zone("Greenwich")};
		_cached_zones["HST"] = {"HST", locate_zone("HST")};
		_cached_zones["HONGKONG"] = {"Hongkong", locate_zone("Hongkong")};
		_cached_zones["ICELAND"] = {"Iceland", locate_zone("Iceland")};
		_cached_zones["INDIAN/ANTANANARIVO"] = {"Indian/Antananarivo", locate_zone("Indian/Antananarivo")};
		_cached_zones["INDIAN/CHAGOS"] = {"Indian/Chagos", locate_zone("Indian/Chagos")};
		_cached_zones["INDIAN/CHRISTMAS"] = {"Indian/Christmas", locate_zone("Indian/Christmas")};
		_cached_zones["INDIAN/COCOS"] = {"Indian/Cocos", locate_zone("Indian/Cocos")};
		_cached_zones["INDIAN/COMORO"] = {"Indian/Comoro", locate_zone("Indian/Comoro")};
		_cached_zones["INDIAN/KERGUELEN"] = {"Indian/Kerguelen", locate_zone("Indian/Kerguelen")};
		_cached_zones["INDIAN/MAHE"] = {"Indian/Mahe", locate_zone("Indian/Mahe")};
		_cached_zones["INDIAN/MALDIVES"] = {"Indian/Maldives", locate_zone("Indian/Maldives")};
		_cached_zones["INDIAN/MAURITIUS"] = {"Indian/Mauritius", locate_zone("Indian/Mauritius")};
		_cached_zones["INDIAN/MAYOTTE"] = {"Indian/Mayotte", locate_zone("Indian/Mayotte")};
		_cached_zones["INDIAN/REUNION"] = {"Indian/Reunion", locate_zone("Indian/Reunion")};
		_cached_zones["IRAN"] = {"Iran", locate_zone("Iran")};
		_cached_zones["ISRAEL"] = {"Israel", locate_zone("Israel")};
		_cached_zones["JAMAICA"] = {"Jamaica", locate_zone("Jamaica")};
		_cached_zones["JAPAN"] = {"Japan", locate_zone("Japan")};
		_cached_zones["KWAJALEIN"] = {"Kwajalein", locate_zone("Kwajalein")};
		_cached_zones["LIBYA"] = {"Libya", locate_zone("Libya")};
		_cached_zones["MET"] = {"MET", locate_zone("MET")};
		_cached_zones["MST"] = {"MST", locate_zone("MST")};
		_cached_zones["MST7MDT"] = {"MST7MDT", locate_zone("MST7MDT")};
		_cached_zones["MEXICO/BAJANORTE"] = {"Mexico/BajaNorte", locate_zone("Mexico/BajaNorte")};
		_cached_zones["MEXICO/BAJASUR"] = {"Mexico/BajaSur", locate_zone("Mexico/BajaSur")};
		_cached_zones["MEXICO/GENERAL"] = {"Mexico/General", locate_zone("Mexico/General")};
		_cached_zones["NZ"] = {"NZ", locate_zone("NZ")};
		_cached_zones["NZ-CHAT"] = {"NZ-CHAT", locate_zone("NZ-CHAT")};
		_cached_zones["NAVAJO"] = {"Navajo", locate_zone("Navajo")};
		_cached_zones["PRC"] = {"PRC", locate_zone("PRC")};
		_cached_zones["PST8PDT"] = {"PST8PDT", locate_zone("PST8PDT")};
		_cached_zones["PACIFIC/APIA"] = {"Pacific/Apia", locate_zone("Pacific/Apia")};
		_cached_zones["PACIFIC/AUCKLAND"] = {"Pacific/Auckland", locate_zone("Pacific/Auckland")};
		_cached_zones["PACIFIC/BOUGAINVILLE"] = {"Pacific/Bougainville", locate_zone("Pacific/Bougainville")};
		_cached_zones["PACIFIC/CHATHAM"] = {"Pacific/Chatham", locate_zone("Pacific/Chatham")};
		_cached_zones["PACIFIC/CHUUK"] = {"Pacific/Chuuk", locate_zone("Pacific/Chuuk")};
		_cached_zones["PACIFIC/EASTER"] = {"Pacific/Easter", locate_zone("Pacific/Easter")};
		_cached_zones["PACIFIC/EFATE"] = {"Pacific/Efate", locate_zone("Pacific/Efate")};
		_cached_zones["PACIFIC/ENDERBURY"] = {"Pacific/Enderbury", locate_zone("Pacific/Enderbury")};
		_cached_zones["PACIFIC/FAKAOFO"] = {"Pacific/Fakaofo", locate_zone("Pacific/Fakaofo")};
		_cached_zones["PACIFIC/FIJI"] = {"Pacific/Fiji", locate_zone("Pacific/Fiji")};
		_cached_zones["PACIFIC/FUNAFUTI"] = {"Pacific/Funafuti", locate_zone("Pacific/Funafuti")};
		_cached_zones["PACIFIC/GALAPAGOS"] = {"Pacific/Galapagos", locate_zone("Pacific/Galapagos")};
		_cached_zones["PACIFIC/GAMBIER"] = {"Pacific/Gambier", locate_zone("Pacific/Gambier")};
		_cached_zones["PACIFIC/GUADALCANAL"] = {"Pacific/Guadalcanal", locate_zone("Pacific/Guadalcanal")};
		_cached_zones["PACIFIC/GUAM"] = {"Pacific/Guam", locate_zone("Pacific/Guam")};
		_cached_zones["PACIFIC/HONOLULU"] = {"Pacific/Honolulu", locate_zone("Pacific/Honolulu")};
		_cached_zones["PACIFIC/JOHNSTON"] = {"Pacific/Johnston", locate_zone("Pacific/Johnston")};
		_cached_zones["PACIFIC/KIRITIMATI"] = {"Pacific/Kiritimati", locate_zone("Pacific/Kiritimati")};
		_cached_zones["PACIFIC/KOSRAE"] = {"Pacific/Kosrae", locate_zone("Pacific/Kosrae")};
		_cached_zones["PACIFIC/KWAJALEIN"] = {"Pacific/Kwajalein", locate_zone("Pacific/Kwajalein")};
		_cached_zones["PACIFIC/MAJURO"] = {"Pacific/Majuro", locate_zone("Pacific/Majuro")};
		_cached_zones["PACIFIC/MARQUESAS"] = {"Pacific/Marquesas", locate_zone("Pacific/Marquesas")};
		_cached_zones["PACIFIC/MIDWAY"] = {"Pacific/Midway", locate_zone("Pacific/Midway")};
		_cached_zones["PACIFIC/NAURU"] = {"Pacific/Nauru", locate_zone("Pacific/Nauru")};
		_cached_zones["PACIFIC/NIUE"] = {"Pacific/Niue", locate_zone("Pacific/Niue")};
		_cached_zones["PACIFIC/NORFOLK"] = {"Pacific/Norfolk", locate_zone("Pacific/Norfolk")};
		_cached_zones["PACIFIC/NOUMEA"] = {"Pacific/Noumea", locate_zone("Pacific/Noumea")};
		_cached_zones["PACIFIC/PAGO_PAGO"] = {"Pacific/Pago_Pago", locate_zone("Pacific/Pago_Pago")};
		_cached_zones["PACIFIC/PALAU"] = {"Pacific/Palau", locate_zone("Pacific/Palau")};
		_cached_zones["PACIFIC/PITCAIRN"] = {"Pacific/Pitcairn", locate_zone("Pacific/Pitcairn")};
		_cached_zones["PACIFIC/POHNPEI"] = {"Pacific/Pohnpei", locate_zone("Pacific/Pohnpei")};
		_cached_zones["PACIFIC/PONAPE"] = {"Pacific/Ponape", locate_zone("Pacific/Ponape")};
		_cached_zones["PACIFIC/PORT_MORESBY"] = {"Pacific/Port_Moresby", locate_zone("Pacific/Port_Moresby")};
		_cached_zones["PACIFIC/RAROTONGA"] = {"Pacific/Rarotonga", locate_zone("Pacific/Rarotonga")};
		_cached_zones["PACIFIC/SAIPAN"] = {"Pacific/Saipan", locate_zone("Pacific/Saipan")};
		_cached_zones["PACIFIC/SAMOA"] = {"Pacific/Samoa", locate_zone("Pacific/Samoa")};
		_cached_zones["PACIFIC/TAHITI"] = {"Pacific/Tahiti", locate_zone("Pacific/Tahiti")};
		_cached_zones["PACIFIC/TARAWA"] = {"Pacific/Tarawa", locate_zone("Pacific/Tarawa")};
		_cached_zones["PACIFIC/TONGATAPU"] = {"Pacific/Tongatapu", locate_zone("Pacific/Tongatapu")};
		_cached_zones["PACIFIC/TRUK"] = {"Pacific/Truk", locate_zone("Pacific/Truk")};
		_cached_zones["PACIFIC/WAKE"] = {"Pacific/Wake", locate_zone("Pacific/Wake")};
		_cached_zones["PACIFIC/WALLIS"] = {"Pacific/Wallis", locate_zone("Pacific/Wallis")};
		_cached_zones["PACIFIC/YAP"] = {"Pacific/Yap", locate_zone("Pacific/Yap")};
		_cached_zones["POLAND"] = {"Poland", locate_zone("Poland")};
		_cached_zones["PORTUGAL"] = {"Portugal", locate_zone("Portugal")};
		_cached_zones["ROC"] = {"ROC", locate_zone("ROC")};
		_cached_zones["ROK"] = {"ROK", locate_zone("ROK")};
		_cached_zones["SINGAPORE"] = {"Singapore", locate_zone("Singapore")};
		_cached_zones["TURKEY"] = {"Turkey", locate_zone("Turkey")};
		_cached_zones["UCT"] = {"UCT", locate_zone("UCT")};
		_cached_zones["US/ALASKA"] = {"US/Alaska", locate_zone("US/Alaska")};
		_cached_zones["US/ALEUTIAN"] = {"US/Aleutian", locate_zone("US/Aleutian")};
		_cached_zones["US/ARIZONA"] = {"US/Arizona", locate_zone("US/Arizona")};
		_cached_zones["US/CENTRAL"] = {"US/Central", locate_zone("US/Central")};
		_cached_zones["US/EAST-INDIANA"] = {"US/East-Indiana", locate_zone("US/East-Indiana")};
		_cached_zones["US/EASTERN"] = {"US/Eastern", locate_zone("US/Eastern")};
		_cached_zones["US/HAWAII"] = {"US/Hawaii", locate_zone("US/Hawaii")};
		_cached_zones["US/INDIANA-STARKE"] = {"US/Indiana-Starke", locate_zone("US/Indiana-Starke")};
		_cached_zones["US/MICHIGAN"] = {"US/Michigan", locate_zone("US/Michigan")};
		_cached_zones["US/MOUNTAIN"] = {"US/Mountain", locate_zone("US/Mountain")};
		_cached_zones["US/PACIFIC"] = {"US/Pacific", locate_zone("US/Pacific")};
		_cached_zones["US/SAMOA"] = {"US/Samoa", locate_zone("US/Samoa")};
		_cached_zones["UTC"] = {"UTC", locate_zone("UTC")};
		_cached_zones["UNIVERSAL"] = {"Universal", locate_zone("Universal")};
		_cached_zones["W-SU"] = {"W-SU", locate_zone("W-SU")};
		_cached_zones["WET"] = {"WET", locate_zone("WET")};
		_cached_zones["ZULU"] = {"Zulu", locate_zone("Zulu")};
		_cached_zones["PST"] = {"PST", locate_zone("US/Pacific")};
		_cached_zones["PDT"] = {"PDT", locate_zone("US/Pacific")};
		_cached_zones["EDT"] = {"EDT", locate_zone("US/Eastern")};
		_cached_zones["CST"] = {"CST", locate_zone("US/Central")};
		_cached_zones["CDT"] = {"CDT", locate_zone("US/Central")};
		_cached_zones["MDT"] = {"MDT", locate_zone("US/Mountain")};
	}
}
