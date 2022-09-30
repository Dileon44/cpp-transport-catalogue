#include "json_reader.h"
#include "json_builder.h"

#include <stdexcept>
#include <vector>
#include <map>
#include <algorithm> 
#include <utility>
#include <sstream>

#include "log_duration.h"

namespace catalogue {

	JSONReader::JSONReader(TransportCatalogue& catalogue,
							RequestHandler& request_handler,
							renderer::MapRenderer& map_renderer)
		: catalogue_(catalogue)
		, request_handler_(request_handler)
		, map_renderer_(map_renderer) {
	}

	void JSONReader::ReadJSON(std::istream& input) {
		json::Document input_doc = std::move(json::Load(input));

		if (input_doc.GetRoot().IsDict()
			&& input_doc.GetRoot().AsDict().count("base_requests")
			&& input_doc.GetRoot().AsDict().count("stat_requests")
			&& input_doc.GetRoot().AsDict().count("render_settings")) {

			base_requests_ = json::Document{
				input_doc.GetRoot().AsDict().at("base_requests")
			};
			stat_requests_ = json::Document{
				input_doc.GetRoot().AsDict().at("stat_requests")
			};
			render_settings_ = ReadJsonRenderSettings(
				input_doc.GetRoot().AsDict().at("render_settings").AsDict()
			);
		}
		else {
			throw std::logic_error("incorrect input data");
		}
	}

	renderer::RenderSettings JSONReader::ReadJsonRenderSettings(
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

	void JSONReader::BuildDataBase() {
		for (const json::Node& stop_json : base_requests_.GetRoot().AsArray()) {
			if (stop_json.IsDict() && stop_json.AsDict().at("type") == "Stop") {
				AddNameAndCoordinatesOfStop(stop_json);
			}
		}
		for (const json::Node& stop_from : base_requests_.GetRoot().AsArray()) {
			if (stop_from.IsDict() && stop_from.AsDict().at("type") == "Stop") {
				AddDistanceBetweenStops(stop_from);
			}
		}
		for (const json::Node& bus_json : base_requests_.GetRoot().AsArray()) {
			if (bus_json.AsDict().at("type") == "Bus") {
				AddJsonBus(bus_json);
			}
		}
	}

	void JSONReader::AddNameAndCoordinatesOfStop(const json::Node& node) {
		catalogue_.AddStop(
			{
			  node.AsDict().at("name").AsString()
			, node.AsDict().at("latitude").AsDouble()
			, node.AsDict().at("longitude").AsDouble()
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

	void JSONReader::GenerateAnswer() {
		std::vector<json::Node> answers;

		for (const json::Node& request : stat_requests_.GetRoot().AsArray()) {
			if (request.AsDict().at("type") == "Bus") {
				answers.push_back(std::move(GenerateAnswerAboutRoute(request)));
			}
			else if (request.AsDict().at("type") == "Stop") {
				answers.push_back(std::move(GenerateAnswerAboutStop(request)));
			}
			else {
				answers.push_back(std::move(GenerateAnswerAboutMap(request.AsDict().at("id").AsInt())));
			}
		}
		answers_ = std::move(json::Document{ answers });
	}

	json::Node JSONReader::GenerateAnswerAboutRoute(const json::Node& request) {
		std::string_view bus_name = request.AsDict().at("name").AsString();
		json::Builder answer;
		answer.StartDict().Key("request_id").Value(request.AsDict().at("id").AsInt());

		if (catalogue_.FindBus(bus_name)) {
			answer.Key("curvature").Value(request_handler_.GetBusStat(bus_name)->curvature)
				.Key("route_length").Value(request_handler_.GetBusStat(bus_name)->length_route)
				.Key("stop_count").Value(static_cast<int>(request_handler_.GetBusStat(bus_name)->number_stops))
				.Key("unique_stop_count").Value(static_cast<int>(request_handler_.GetBusStat(bus_name)->unique_stops))
				.EndDict();
		}
		else {
			answer.Key("error_message").Value("not found").EndDict();
		}

		return answer.Build();
	}

	json::Node JSONReader::GenerateAnswerAboutStop(const json::Node& request) {
		std::string_view stop_name = request.AsDict().at("name").AsString();
		json::Builder answer;
		answer.StartDict().Key("request_id").Value(request.AsDict().at("id").AsInt());
		if (catalogue_.FindStop(stop_name)) {
			answer.Key("buses").StartArray();

			std::vector<json::Node> buses;
			for (const auto& bus : request_handler_.GetBusesByStop(stop_name)) {
				buses.push_back(bus->name);
				//answers.Value(bus->name);
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

	json::Node JSONReader::GenerateAnswerAboutMap(int id) {
		std::ostringstream output;

		std::set< const Bus*, CompareBuses > buses = std::move(GetBuses());
		map_renderer_.RenderMap(render_settings_, buses).Render(output);

		json::Builder answer;
		answer.StartDict()
			.Key("request_id").Value(id)
			.Key("map").Value(output.str())
		.EndDict();

		return answer.Build();
	}

	std::set< const Bus*, CompareBuses > JSONReader::GetBuses() const {
		std::set< const Bus*, CompareBuses > buses;
		for (const json::Node& bus_json : base_requests_.GetRoot().AsArray()) {
			if (bus_json.AsDict().at("type") == "Bus") {
				buses.insert(
					catalogue_.FindBus(bus_json.AsDict().at("name").AsString())
				);
			}
		}
		return buses;
	}

	void JSONReader::PrintAnswer(std::ostream& output) {
		json::Print(answers_, output);
	}

} // namespace catalogue
