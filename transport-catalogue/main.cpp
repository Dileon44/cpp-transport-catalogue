#include "json.h"
#include "json_reader.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "transport_router.h"

int main() {
	catalogue::TransportCatalogue catalogue;
	catalogue::renderer::MapRenderer map_renderer;
	catalogue::JSONReader json_reader(catalogue, map_renderer);

	catalogue::Data data = std::move(json_reader.ReadJSON(std::cin));

	catalogue::TransportRouter transport_router(catalogue, data.routing_settings);
	catalogue::RequestHandler request_handler(catalogue, map_renderer, transport_router);
	
	json_reader.BuildDataBase(data);
	transport_router.BuildGraphAndRouter();

	json::Document answers = std::move(json_reader.GenerateAnswer(transport_router, data.stat_requests));
	json::Print(answers, std::cout);
	
	return 0;
}
