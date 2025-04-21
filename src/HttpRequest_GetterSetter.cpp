#include"../include/ServerConfig.hpp"

/********************/
/*		Helper		*/
/********************/


HttpMethod HttpRequest::stringToHttpMethod(const std::string& method)
{
	if(method == "GET") return HttpMethod::GET;
	if(method == "POST") return HttpMethod::POST;
	if(method == "DELETE") return HttpMethod::DELETE;
	return HttpMethod::UNKNOWN;
}



/*******************/
/* Getter & Setter */
/*******************/

const std::string& HttpRequest::getPath(void)
{
	return _requestLine._path;
}

void HttpRequest::setPath(const std::string& path)
{
	_requestLine._path = path;
}

void HttpRequest::setMethod(const std::string& method)
{
	_requestLine._method = stringToHttpMethod(method);
}

HttpMethod HttpRequest::getMethod(routeConfig &route)
{
	if (route.checkMethod(_requestLine._method))
		return (_requestLine._method );
	else
		return HttpMethod::FORBIDDEN;
}

void HttpRequest::setVersion(const std::string& version)
{
	_requestLine._version = version;
}

const std::string& HttpRequest::getHttpResponse(void)
{
	return _httpResponse;
}

