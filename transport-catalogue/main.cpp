#include "json.h"
#include "json_reader.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "transport_router.h"

int main() {
	catalogue::TransportCatalogue catalogue;
	catalogue::renderer::MapRenderer map_renderer;
	catalogue::RequestHandler request_handler(catalogue, map_renderer);
	catalogue::JSONReader json_reader(catalogue, request_handler, map_renderer);

	catalogue::Data data = std::move(json_reader.ReadJSON(std::cin));

	json_reader.BuildDataBase(data);
	catalogue.BuildGraph();
	catalogue::TransportRouter transport_router(catalogue.GetGraph());

	json::Document answers = std::move(json_reader.GenerateAnswer(transport_router, data.stat_requests));
	json::Print(answers, std::cout);
	
	return 0;
}
