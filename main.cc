#include <cstddef>
#include <cstdlib>
#include <cstdio>

#include <string>
#include <vector>
#include <future>
#include <tuple>
#include <utility>
#include <iterator>
#include <stdexcept>
#include <system_error>

#ifndef NDEBUG
#define BOOST_SPIRIT_X3_DEBUG
#endif

#include <boost/spirit/home/x3.hpp>

#include <cpprest/http_listener.h>
#include <cpprest/http_msg.h>
#include <cpprest/json.h>

#include <osrm/osrm.hpp>
#include <osrm/status.hpp>
#include <osrm/coordinate.hpp>
#include <osrm/engine_config.hpp>
#include <osrm/json_container.hpp>
#include <osrm/route_parameters.hpp>

#ifdef __unix__
#include <errno.h>
#include <signal.h>
#endif


// Fulfilled promise signals clean shutdown
static std::promise<void> signal_shutdown;


int main(int argc, char** argv) try {
  if (argc != 3) {
    std::fprintf(stderr, "Usage: %s uri osrmfile\n", argv[0]);
    return EXIT_FAILURE;
  }

#ifdef __unix__
  // Register CTRL+C handler on POSIX systems for clean shutdown
  struct ::sigaction action;
  ::sigemptyset(&action.sa_mask);
  action.sa_flags = SA_SIGINFO;
  action.sa_sigaction = [](int, ::siginfo_t*, void*) { signal_shutdown.set_value(); };

  if (::sigaction(SIGINT, &action, nullptr) == -1)
    throw std::system_error{errno, std::system_category()};
#endif

  // OSRM Setup
  osrm::EngineConfig config{argv[2]};
  config.use_shared_memory = false;

  osrm::OSRM osrm{config};

  // Server listens based on URI (Host, Port)
  web::uri uri{argv[1]};
  web::http::experimental::listener::http_listener listener{uri};

  // GET Request Handler
  listener.support(web::http::methods::GET, [&osrm](const auto& request) {
    const auto uri = request.relative_uri();
    const auto path = uri.path();
    const auto query = uri.query();

    std::fprintf(stderr, "%s\t%s\t%s\n", request.method().c_str(), path.c_str(), query.c_str());

    // Parse Coordinates with X3
    const auto parser = "/route/v1/car/" >> (boost::spirit::x3::float_ >> "," >> boost::spirit::x3::float_) % ";";
    std::vector<float> coordinates;

    if (!parse(begin(path), end(path), parser, coordinates) || coordinates.size() < 4) {
      request.reply(web::http::status_codes::BadRequest);
      return; // Early Exit
    }

    // Request Route
    osrm::RouteParameters params;
    params.steps = false;
    params.alternatives = false;

    for (std::size_t lonIdx{0}, latIdx{1}; latIdx < coordinates.size(); lonIdx += 2, latIdx += 2)
      params.coordinates.emplace_back(osrm::util::FloatLongitude{coordinates[lonIdx]},
                                      osrm::util::FloatLatitude{coordinates[latIdx]});

    osrm::json::Object result;

    const auto status = osrm.Route(params, result);

    if (status == osrm::Status::Error) {
      request.reply(web::http::status_codes::NotFound);
      return; // Early Exit
    }

    auto& routes = result.values["routes"].get<osrm::json::Array>();
    auto& route = routes.values.at(0).get<osrm::json::Object>();
    const auto distance = route.values["distance"].get<osrm::json::Number>().value;
    const auto duration = route.values["duration"].get<osrm::json::Number>().value;

    web::json::value response;

    response["distance"] = distance;
    response["duration"] = duration;

    request.reply(web::http::status_codes::OK, response);
  });


  // Start up server, handles concurrency internally
  listener.open().then([&uri] { std::fprintf(stderr, "Host %s\nPort %d\n\n", uri.host().c_str(), uri.port()); }).wait();

  // Main thread blocks on future until its associated promise is fulfilled from within the CTRL+C handler
  signal_shutdown.get_future().wait();

  // Only then we shutdown the server
  listener.close().then([] { std::fprintf(stderr, "\nBye!\n"); }).wait();

} catch (const std::exception& e) {
  std::fprintf(stderr, "Error: %s\n", e.what());
  return EXIT_FAILURE;
}
