#include "request_handler.h"
#include "json_reader.h"
#include "transport_catalogue.h"

#include "log_duration.h"
#include <fstream>

int main() {
	LOG_DURATION("Time: ");

	catalogue::TransportCatalogue catalogue;
	catalogue::renderer::MapRenderer map_renderer;
	catalogue::RequestHandler request_handler(catalogue, map_renderer);
	catalogue::JSONReader json_reader(catalogue, request_handler, map_renderer);

	std::ifstream input("s10_final_opentest_3.json");
	std::ofstream output("b.txt");
	{
		LOG_DURATION("ReadJSON: ");
		json_reader.ReadJSON(input);
	}
	{
		LOG_DURATION("BuildDataBase: ");
		json_reader.BuildDataBase();
	}
	{
		LOG_DURATION("GenerateAnswer: ");
		json_reader.GenerateAnswer();
	}
	{
		LOG_DURATION("PrintAnswer: ");
		json_reader.PrintAnswer(output);
	}

	return 0;
}
