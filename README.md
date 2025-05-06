# This is a Http webserver Project  🖥️🕸️🖥️ written in C++

### Objective: Build a Http Webserver that:
- is Http 1.1 compliant
- serves static websites
- uploads files
- executes CGI requests
- uses I/O multiplexing
- most critically _never_ chrashes or blocks
- reads a configuration file and behaves accordingly
- handles GET, POST, DELETE methods
- has default error pages
- is single threaded, only forks for CGI

### We tackled this as a 3 person team

## Usage:
#### This server is running on MacOS and Linux, windows not tested. There is a Docker container provided to run it independently and be able to change the Hostnames.
- clone the repo:
  ```
  git clone git@github.com:strohhelm/webserv_42.git Webserv && cd Webserv
  ```
- build the image and run the container,  the folder will be mounted to it as a volume
  ```
  docker compose -f ./Docker/docker-compose.yaml up
  ```
- enter the docker container
  ```
  docker exec -it ubu zsh
  ```
- build the executable
  ```
  make
  ```
- run the server with default cinfig file, the one we used to demonstrate the project runs 2 servers.
  ```
  ./webserv
  ```
- to run in debug mode use "-d" flag

- to use a specific config file specify the path to it in the arguments
 ```
  ./webserv config/evaluation/multipleLocation.conf
```

- example with current default config:
  ![Docker](/img/example1.png)

## Configuration ⚙️
  The Configutation is heavily inspired by the config syntax of nginx. It does have the following options:
#### main context:
  | directive        |      configuration     |
| ------------- |:-------------:| 
| `worker_connections 1024;`| number of simultaneous connections, capped at 1024, because single threaded|
| `keepalive_timeout  10;` | number of seconds without I/O operation before the server closes the connetction|
|  `http {}`                 | configurations for servers|

#### http context:

  | directive        |configuration|
| ------------- |:-------------:| 
|`listen 8080;`| Port the server should listen on|
|`server_name localhost example.com;`| the hostnames for this server (will get appended if more than once in config file)|
|`root /var/www/html;`| the root directory for the website files to serve |
|`error_page 400 401 403 ./www/html/40x.html;'  | maps certain error status codes to custom .html files|
|`index index.html index.php;` | default files to look for if directory is requested. If not set, these will always be set by default.|
|`client_max_body_size 3gb;` | maximum allowed upload size for a single request. Default is 1 mb if not set.|
|`location {}` | configuration for endpoints|

#### lcoation context:

  | directive        |configuration|
| ------------- |:-------------:| 
|`methods GET POST DELETE;` | Methods to allow in this location. if not set, get is enabled by default.|
|`root ./location1;` | root directory for this locaiton. Overwrites server root Dir if set. does not include location name in path |
|`return 301 https://github.com/strohhelm;` | redirects with the status code given to a different url |
|`autoindex on;` | turns on default directory listing if url is a directory|
|`index test.html;` | sets default files to serve when url is a directory |
|`upload_path ./uploads` | sets a path independently to root directory where file can be uploaded in a POST request. Also by that allows DELETE in that directory |
|`cgi .php /usr/bin/php;`| allows cgi to be executed. Maps fileend to executable. |


