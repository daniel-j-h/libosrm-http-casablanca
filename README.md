### libosrm-http-casablanca

Experimental alternative Casablanca HTTP server integration for libosrm (read: osrm-routed replacement).

Warning: proof of concept; I do not recommend using it for anything serious.


## Building the Server

    ./deps.sh
    make

or using Nix for dependencies

    nix-shell --run './deps && make'


## Starting the Server

Run `osrm-extract` and `osrm-contract` on your `.pbf` extract, then start up the server:

    $ ./build/Release/osrm-casablanca http://localhost:5000 berlin-latest.osrm

And run queries against it:

    $ http 'http://localhost:5000/route/v1/car/13.438640,52.519930;13.415852,52.513191'

    HTTP/1.1 200 OK
    Content-Length: 61
    Content-Type: application/json
    
    {
        "distance": 2670.8568731104956, 
        "duration": 84.4
    }

Now you have a high-performance concurrent routing HTTP server, with keep alive and all that by design :D


## License

Copyright © 2016 Daniel J. Hofmann

Distributed under the MIT License (MIT).
