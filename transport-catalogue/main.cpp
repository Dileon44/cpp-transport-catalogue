//#include <cassert>
//#include <iostream>
//#include <istream>
//#include <string>
//#include <string_view>
//#include <sstream>
//#include <vector>
//#include <string>
//#include <deque>
//#include <list>
//#include <iomanip>
//#include <algorithm>
//#include <unordered_set>
//#include <random>

//#include <deque>
//#include <string>
//#include <unordered_map>
//#include <vector>
//#include <string_view>
//#include <set>
//#include <functional>

//#include "transport_catalogue.h"
//#include "input_reader.h"
//#include "stat_reader.h"
#include "request_handler.h"
#include "json_reader.h"
#include "map_renderer.h"

//#include "json.h" // not need
//#include "svg.h"

//#include <iostream>
//#include <algorithm>
//#include <set>
//#include <map>

//struct Man {
//	std::string name = "Dima";
//	std::string famely = "Leonov";
//};
//
//struct Compare {
//public:
//	bool operator()(const Man& l, const Man& r) const {
//		return std::lexicographical_compare(l.name.begin(), l.name.end(),
//			r.name.begin(), r.name.end(),
//			[&](const char l, const char r) {
//				return l < r;
//			}
//		);
//		;
//	}
//};

//bool Compare(const Man& l, const Man& r) {
//	return std::lexicographical_compare(l.name.begin(), l.name.end(),
//										r.name.begin(), r.name.end(),
//				[&](const char l, const char r) {
//					return l < r;
//				}
//			);
//	;
//}

//struct PrinterVector {
//	std::ostream& out;
//
//	void operator()(int value) const {
//		out << value << std::endl;
//	}
//	void operator()(double value) const {
//		out << value << std::endl;
//	}
//	void operator()(std::string str) const {
//		out << str << std::endl;
//	}
//};
//
//enum class TypeRoute {
//	CIRCLE,
//	DIRECT
//};
//
//struct Stop {
//	std::string name;
//	double latitude;
//	double longitude;
//};
//
//struct Bus {
//	TypeRoute type_route;
//	std::string name;
//	std::deque<std::string> stops;
//};
//
//struct CompareBuses {
//public:
//	bool operator()(const Bus* l, const Bus* r) const {
//		return std::lexicographical_compare(l->name.begin(), l->name.end(),
//			r->name.begin(), r->name.end());;
//	}
//};
// 
//#include <string>
//struct G {
//	double d;
//	std::vector<int> v{1, 2, 3};
//	std::vector<std::string> s{"a", "b"};
//	//std::vector<svg::Color> color_palette = { "red" };
//};

int main() {

	catalogue::TransportCatalogue catalogue;
	catalogue::renderer::MapRenderer map_renderer;
	catalogue::RequestHandler request_handler(catalogue);
	catalogue::JSONReader json_reader(catalogue, request_handler, map_renderer);

	json_reader.ReadJSON(std::cin);
	json_reader.BuildDataBase();
	json_reader.GenerateAnswer();
	json_reader.PrintAnswer(std::cout);
	//map_renderer.RenderMap().Render(std::cout);

	//G g;
	//std::cout << " " << g.v[0] << std::endl;
	//std::cout << " " << g.v[1] << std::endl;
	//std::cout << " " << g.s[0] << std::endl;
	//std::cout << " " << g.s[1] << std::endl;

	//std::cout << "New:" << std::endl;

	//std::vector<int> v1{ 10, 20, 30 };
	//std::vector<std::string> s1{ "z", "x" };
	//g.v = v1;
	//g.s = s1;
	//std::cout << " " << g.v[0] << std::endl;
	//std::cout << " " << g.v[1] << std::endl;
	//std::cout << " " << g.s[0] << std::endl;
	//std::cout << " " << g.s[1] << std::endl;

	//using rgb = std::variant<int, double, std::string>;

	//std::vector<rgb> v;

	//v.push_back("Dima");
	//v.push_back(5);
	//v.push_back(5.0);

	//for (const auto& f : v) {
	//	std::visit(PrinterVector{std::cout}, f);
	//}



	//std::set< const Bus*, CompareBuses > buses;

	//Bus bus1{ TypeRoute::CIRCLE, "0sdf", {"A", "B", "C"}};
	//Bus bus2{ TypeRoute::CIRCLE, "14", {"A", "V", "Z"}};
	//Bus bus3{ TypeRoute::CIRCLE, "114", {"A", "V", "Z"}};
	//const Bus* ptr1 = &bus1;
	//const Bus* ptr2 = &bus2;
	//const Bus* ptr3 = &bus3;
	//buses.insert(ptr2);
	//buses.insert(ptr3);
	//buses.insert(ptr1);

	//for (const auto& f : buses) {
	//	std::cout << " " << f->name << std::endl;
	//}

	//std::map< const Bus*, std::vector<int>, CompareBuses > buses_map;

	//for (const auto& f : buses) {
	//	buses_map.insert({ f, { 1, 2, 3 } });
	//}

	//for (const auto& f : buses_map) {
	//	std::cout << " " << f.first->name << ": " << f.second[0] << std::endl;
	//}

	//Man man1;
	//Man man2{"A", "zsdf"};
	//Man man3{"B", "zsdf"};
	//Man man4{"Z", "zsdf"};


	//std::set < Man, Compare > mans{ man1, man2, man3, man4 };

	//for (const auto& man : mans) {
	//	std::cout << " " << man.name << std::endl;
	//}

	//std::vector<std::string> v{"a", "ac", "z", "ab", "asdf"};
	//std::sort(v.begin(), v.end());

	//for (const auto& i : v) {
	//	std::cout << " " << i << std::endl;
	//}

	////std::string str1 = "a";
	////std::string str2 = "z";
	////std::cout << 
	////	std::lexicographical_compare(str1.begin(), str1.end(),
	////		str2.begin(), str2.end(),
	////		[&](const char l, const char r) {
	////			return l < r;
	////		}
	////	)
	////<< std::endl;
	//std::cout << "Sort Node:" << std::endl;
	//std::vector<json::Node> n{ "a", "ac", "z", "ab", "asdf" };
	//std::sort(n.begin(), n.end(), [&](const json::Node& l, const json::Node& r) {

	//	return std::lexicographical_compare(l.AsString().begin(), l.AsString().end(),
	//										r.AsString().begin(), r.AsString().end(),
	//		[&](const char l, const char r) {
	//			return l < r;
	//		}
	//		);
	//	}
	//);
	//for (const auto& i : n) {
	//	std::cout << " " << i.AsString() << std::endl;
	//}

	//std::vector<std::string> v{ "a", "b", "c"};
	//std::vector<std::string> c{ v };

	//std::move(std::next(c.rbegin()), c.rend(), std::back_inserter(v));

	//for (const auto& i : v) {
	//	std::cout << " " << i << std::endl;
	//}


	//// out first version application
	//// read input json-document
	//std::pair<json::Document, json::Document> database_and_requests = 
	//	json_reader.ReadJSON(std::cin);
	//// build database of catalogue
	//json_reader.BuildDataBase(database_and_requests.first);
	//// formation answers
	//json::Document answers = json_reader.GenerateAnswer(database_and_requests.second);
	//// print
	//json_reader.PrintAnswer(answers, std::cout);

	return 0;
}