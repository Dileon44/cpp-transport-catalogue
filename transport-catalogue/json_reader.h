#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"

namespace catalogue {

	class JSONReader {
	public:
		JSONReader(TransportCatalogue& catalogue, 
				   RequestHandler& request_handler_,
				   renderer::MapRenderer& map_renderer);
		
		void ReadJSON(std::istream& input);
		void BuildDataBase();
		void GenerateAnswer();
		void PrintAnswer(std::ostream& output);

	private:
		TransportCatalogue& catalogue_;
		RequestHandler& request_handler_;
		renderer::MapRenderer& map_renderer_;
		renderer::RenderSettings render_settings_;

		json::Document base_requests_;
		json::Document stat_requests_;
		json::Document answers_;

		renderer::RenderSettings ReadJsonRenderSettings(const json::Node& settings) const;
		svg::Color ReadUnderlayerColor(const std::map<std::string, json::Node>& s) const;
		std::vector<svg::Color> ReadColorPalette(const std::map<std::string, json::Node>& s) const;

		void AddNameAndCoordinatesOfStop(const json::Node& node);
		void AddDistanceBetweenStops(const json::Node& stop_from);
		void AddJsonBus(const json::Node& node);

		std::map<std::string, json::Node> GenerateAnswerAboutRoute(
			const json::Node& request) const;
		std::map<std::string, json::Node> GenerateAnswerAboutStop(
			const json::Node& request) const;
		std::map<std::string, json::Node> GenerateAnswerAboutMap(int id) const;

		std::set< const Bus*, CompareBuses > GetBuses() const;
	};

} // namespace catalogue
