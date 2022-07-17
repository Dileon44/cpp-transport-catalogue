#include "request_handler.h"
#include "json_reader.h"
#include "map_renderer.h"

int main() {

	catalogue::TransportCatalogue catalogue;
	catalogue::renderer::MapRenderer map_renderer;
	catalogue::RequestHandler request_handler(catalogue);
	catalogue::JSONReader json_reader(catalogue, request_handler, map_renderer);

	json_reader.ReadJSON(std::cin);
	json_reader.BuildDataBase();
	json_reader.GenerateAnswer();
	json_reader.PrintAnswer(std::cout);
	
	return 0;
}
