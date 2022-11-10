#include "json_reader.h"
#include "json_builder.h"
#include "graph.h"
#include "router.h"
#include "domain.h"
#include "transport_router.h"

#include <stdexcept>
#include <vector>
#include <map>
#include <algorithm> 
#include <utility>
#include <sstream>
#include <unordered_set>

namespace catalogue {

	JSONReader::JSONReader(
		TransportCatalogue& catalogue,
		renderer::MapRenderer& map_renderer)

		: catalogue_(catalogue)
		, map_renderer_(map_renderer) {
	}

	bool CheckResenceSettings(const json::Node& input_node) {

		return input_node.AsDict().count("base_requests")
			&& input_node.AsDict().count("stat_requests")
			&& input_node.AsDict().count("render_settings")
			&& input_node.AsDict().count("routing_settings");
	}

	Data JSONReader::ReadJSON(std::istream& input) const {
		json::Document input_doc = std::move(json::Load(input));
		const json::Node& input_node = input_doc.GetRoot();

		Data db_documents;

		if (input_node.IsDict() && CheckResenceSettings(input_node)) {
			db_documents.base_requests = std::move(json::Document{
				input_node.AsDict().at("base_requests")
				});
			db_documents.stat_requests = std::move(json::Document{
				input_node.AsDict().at("stat_requests")
				});
			db_documents.render_settings = std::move(CreateRenderSettings(
				input_node.AsDict().at("render_settings").AsDict()
			));
			db_documents.routing_settings = std::move(CreateRoutingSettings(
				input_node.AsDict().at("routing_settings").AsDict()
			));
		}
		else {
			throw std::logic_error("incorrect input data");
		}

		return db_documents;
	}

	renderer::RenderSettings JSONReader::CreateRenderSettings(
		const json::Node& settings_json) const {
		if (settings_json.AsDict().empty()) {
			return renderer::RenderSettings{};
		}
		std::map<std::string, json::Node> s = settings_json.AsDict();
		renderer::RenderSettings settings;

		settings.width = s.at("width").AsDouble();
		settings.height = s.at("height").AsDouble();
		settings.padding = s.at("padding").AsDouble();
		settings.line_width = s.at("line_width").AsDouble();
		settings.stop_radius = s.at("stop_radius").AsDouble();
		settings.bus_label_font_size = s.at("bus_label_font_size").AsInt();
		settings.bus_label_offset[0] = s.at("bus_label_offset").AsArray()[0].AsDouble();
		settings.bus_label_offset[1] = s.at("bus_label_offset").AsArray()[1].AsDouble();
		settings.stop_label_font_size = s.at("stop_label_font_size").AsInt();
		settings.stop_label_offset[0] = s.at("stop_label_offset").AsArray()[0].AsDouble();
		settings.stop_label_offset[1] = s.at("stop_label_offset").AsArray()[1].AsDouble();
		settings.underlayer_color = ReadUnderlayerColor(s);
		settings.underlayer_width = s.at("underlayer_width").AsDouble();
		settings.color_palette = ReadColorPalette(s);

		return settings;
	}

	svg::Color JSONReader::ReadUnderlayerColor(const std::map<std::string, json::Node>& s) const {
		svg::Color underlayer_color;

		if (s.at("underlayer_color").IsString()) {
			underlayer_color = s.at("underlayer_color").AsString();
		}
		if (s.at("underlayer_color").IsArray()) {
			if (s.at("underlayer_color").AsArray().size() == 3) {
				underlayer_color = svg::Rgb{
					uint8_t(s.at("underlayer_color").AsArray()[0].AsInt()),
					uint8_t(s.at("underlayer_color").AsArray()[1].AsInt()),
					uint8_t(s.at("underlayer_color").AsArray()[2].AsInt())
				};
			}
			else {
				underlayer_color = svg::Rgba{
					uint8_t(s.at("underlayer_color").AsArray()[0].AsInt()),
					uint8_t(s.at("underlayer_color").AsArray()[1].AsInt()),
					uint8_t(s.at("underlayer_color").AsArray()[2].AsInt()),
					s.at("underlayer_color").AsArray()[3].AsDouble()
				};
			}
		}
		return underlayer_color;
	}

	std::vector<svg::Color> JSONReader::ReadColorPalette(const std::map<std::string, json::Node>& s) const {
		std::vector<svg::Color> color_palette;
		if (s.at("color_palette").AsArray().empty()) {
			color_palette.push_back("none");
		}
		else {
			for (const auto& color : s.at("color_palette").AsArray()) {
				if (color.IsString()) {
					color_palette.push_back(color.AsString());
				}
				else if (color.IsArray() && color.AsArray().size() == 3) {
					uint8_t r, g, b;
					r = color.AsArray()[0].AsInt();
					g = color.AsArray()[1].AsInt();
					b = color.AsArray()[2].AsInt();
					color_palette.push_back(svg::Rgb{ r, g, b });
				}
				else {
					uint8_t r, g, b;
					double o;
					r = color.AsArray()[0].AsInt();
					g = color.AsArray()[1].AsInt();
					b = color.AsArray()[2].AsInt();
					o = color.AsArray()[3].AsDouble();
					color_palette.push_back(svg::Rgba{ r, g, b, o });
				}
			}
		}
		return color_palette;
	}

	RoutingSettings JSONReader::CreateRoutingSettings(const json::Node& route_settings_node) const {
		double bus_velocity = double(route_settings_node.AsDict().at("bus_velocity").AsInt()) * 1000 / 60;
		double bus_wait_time = double(route_settings_node.AsDict().at("bus_wait_time").AsInt());

		return RoutingSettings{ bus_velocity, bus_wait_time };
	}

	void JSONReader::BuildDataBase(const Data& data) {
		render_settings_ = data.render_settings;

		for (const json::Node& json_doc : data.base_requests.GetRoot().AsArray()) {
			if (json_doc.IsDict() && json_doc.AsDict().at("type") == "Stop") {
				AddNameAndCoordinatesOfStop(json_doc);
			}
		}

		for (const json::Node& stop_from : data.base_requests.GetRoot().AsArray()) {
			if (stop_from.IsDict() && stop_from.AsDict().at("type") == "Stop") {
				AddDistanceBetweenStops(stop_from);
			}
		}

		for (const json::Node& bus_json : data.base_requests.GetRoot().AsArray()) {
			if (bus_json.AsDict().at("type") == "Bus") {
				AddJsonBus(bus_json);
			}
		}
	}



	void JSONReader::AddNameAndCoordinatesOfStop(const json::Node& node) {
		catalogue_.AddStop(
			{
			  node.AsDict().at("name").AsString(),
			  node.AsDict().at("latitude").AsDouble(),
			  node.AsDict().at("longitude").AsDouble()
			}
		);
	}

	void JSONReader::AddDistanceBetweenStops(const json::Node& stop_from) {
		for (const auto& stop_to : stop_from.AsDict().at("road_distances").AsDict()) {
			catalogue_.SetDistance(
				stop_from.AsDict().at("name").AsString(),
				stop_to.first,
				stop_to.second.AsInt()
			);
		}
	}

	void JSONReader::AddJsonBus(const json::Node& node) {
		Bus bus;
		bus.name = node.AsDict().at("name").AsString();
		bus.type_route = node.AsDict().at("is_roundtrip").AsBool()
			? TypeRoute::CIRCLE
			: TypeRoute::DIRECT;

		for (const json::Node& stop : node.AsDict().at("stops").AsArray()) {
			bus.stops.push_back(catalogue_.FindStop(stop.AsString()));
		}

		if (bus.type_route == TypeRoute::DIRECT) {
			std::deque<const Stop*> copy = bus.stops;
			std::move(std::next(copy.rbegin()), copy.rend(), std::back_inserter(bus.stops));
		}

		catalogue_.AddBus(bus);
	}

	json::Document JSONReader::GenerateAnswer(const TransportRouter& transport_router,
		const json::Document& stat_requests) const {
		std::vector<json::Node> answers;

		for (const json::Node& request : stat_requests.GetRoot().AsArray()) {
			if (request.AsDict().at("type") == "Bus") {
				answers.push_back(std::move(GenerateAnswerBus(request)));
			}
			else if (request.AsDict().at("type") == "Stop") {
				answers.push_back(std::move(GenerateAnswerStop(request)));
			}
			else if (request.AsDict().at("type") == "Route") {
				answers.push_back(std::move(GenerateAnswerRoute(transport_router, request)));
			}
			else {
				answers.push_back(std::move(GenerateAnswerMap(request.AsDict().at("id").AsInt())));
			}
		}
		return json::Document{ answers };
	}

	json::Node JSONReader::GenerateAnswerBus(const json::Node& request) const {
		std::string_view bus_name = request.AsDict().at("name").AsString();
		json::Builder answer;
		answer.StartDict().Key("request_id").Value(request.AsDict().at("id").AsInt());
		
		const BusInfo bus_info = std::move(catalogue_.GetBusInfo(bus_name));
		
		if (catalogue_.FindBus(bus_name)) {
			answer.Key("curvature").Value(bus_info.curvature)
				.Key("route_length").Value(bus_info.length_route)
				.Key("stop_count").Value(static_cast<int>(bus_info.number_stops))
				.Key("unique_stop_count").Value(static_cast<int>(bus_info.unique_stops))
			.EndDict();
		}
		else {
			answer.Key("error_message").Value("not found").EndDict();
		}

		return answer.Build();
	}

	json::Node JSONReader::GenerateAnswerStop(const json::Node& request) const {
		std::string_view stop_name = request.AsDict().at("name").AsString();
		json::Builder answer;
		answer.StartDict().Key("request_id").Value(request.AsDict().at("id").AsInt());
		if (catalogue_.FindStop(stop_name)) {
			answer.Key("buses").StartArray();

			std::vector<json::Node> buses;
			for (const auto& bus : catalogue_.GetStopInfo(stop_name).buses) {
				buses.push_back(bus->name);
			}
			std::sort(buses.begin(), buses.end(), [&](const json::Node& l, const json::Node& r) {
				return std::lexicographical_compare(l.AsString().begin(), l.AsString().end(),
					r.AsString().begin(), r.AsString().end());
				}
			);
			// Add sorted array
			for (const auto& bus : buses) {
				answer.Value(bus.AsString());
			}
			answer.EndArray();
		}
		else {
			answer.Key("error_message").Value("not found");
		}
		answer.EndDict();

		return answer.Build();
	}

	json::Node JSONReader::GenerateAnswerRoute(const TransportRouter& router,
		const json::Node& request) const {
		
		auto route_info = router.BuildRoute(request.AsDict().at("from").AsString(),
											request.AsDict().at("to").AsString());
		if (!route_info) {
			return json::Builder{}.StartDict().
				Key("request_id").Value(request.AsDict().at("id").AsInt()).
				Key("error_message").Value("not found")
			.EndDict().Build();
		}

		json::Array items;
		for (const auto& edge_id : route_info->edges) {
			items.emplace_back(ConvertEdgeInfo(router, edge_id));
		}

		return json::Builder{}.StartDict()
			.Key("request_id").Value(request.AsDict().at("id").AsInt())
			.Key("total_time").Value(route_info->weight)
			.Key("items").Value(items)
		.EndDict().Build();
	}

	json::Node JSONReader::ConvertEdgeInfo(const TransportRouter& router, const EdgeId edge_id) const {

		if (std::holds_alternative<EdgeBusInfo>(router.GetEdgeInfo(edge_id))) {
			const EdgeBusInfo edge_info = std::get<EdgeBusInfo>(router.GetEdgeInfo(edge_id));

			return json::Builder{}.StartDict()
				.Key("type").Value("Bus")
				.Key("bus").Value(std::string(edge_info.bus->name))
				.Key("span_count").Value(static_cast<int>(edge_info.span_count))
				.Key("time").Value(edge_info.weight)
			.EndDict().Build();
		}

		const EdgeWaitInfo edge_info = std::get<EdgeWaitInfo>(router.GetEdgeInfo(edge_id));

		return json::Builder{}.StartDict()
			.Key("type").Value("Wait")
			.Key("stop_name").Value(std::string(edge_info.stop->name))
			.Key("time").Value(edge_info.weight)
		.EndDict().Build();
	}

	json::Node JSONReader::GenerateAnswerMap(const int id) const {
		std::ostringstream output;

		std::set< const Bus*, CompareBuses > buses;
		for (const Bus* bus_ptr : catalogue_.GetAllBuses()) {
			buses.insert(bus_ptr);
		}

		map_renderer_.RenderMap(render_settings_, buses).Render(output);

		json::Builder answer;
		answer.StartDict()
			.Key("request_id").Value(id)
			.Key("map").Value(output.str())
			.EndDict();

		return answer.Build();
	}

} // namespace catalogue
