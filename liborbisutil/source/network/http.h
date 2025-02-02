#pragma once

#include <string>
#include <unordered_map>

#include <libhttp.h>
#include "../system/logger.h"
#include "../system/resolve.h"

#define LIBORBISUTIL_HTTP_CTX_DEFAULT -1

namespace liborbisutil
{
	namespace http
	{
		struct request
		{
			std::string url;
			std::string path;
			std::string data;
			SceHttpMethods method;
			std::unordered_map<std::string, std::string> headers;

			request() : url(""), path(""), data("") {}
			// get
			request(const std::string& uri, const std::string& path, SceHttpMethods method, const std::unordered_map<std::string, std::string>& headers = {})
				: url(uri), path(path), method(method), headers(headers) {
			}

			// post
			request(const std::string& uri, const std::string& path, const std::string& data, SceHttpMethods method, const std::unordered_map<std::string, std::string>& headers = {})
				: url(uri), path(path), data(data), method(method), headers(headers) {
			}
		};

		struct response
		{
			bool success;
			int req_id;
			int status;
			SceHttpMethods method;
			std::string data;

			std::unordered_map<std::string, std::string> response_headers;

			response() : success(false), req_id(-1), status(-1), method(SCE_HTTP_METHOD_GET) {}
			response(bool success, int req_id, int status, SceHttpMethods method, const std::string& data, const std::unordered_map<std::string, std::string>& response_headers = {})
				: success(success), req_id(req_id), status(status), method(method), data(data), response_headers(response_headers) {
			}
		};

		struct connection
		{
			int id;
			std::string url;
			std::string user_agent;

			std::vector<request> requests;
			std::vector<response> responses;

			connection() : id(-1) {}
		};

		namespace internal
		{
			inline int is_running = false;
			inline bool using_override = false;
			inline int template_id = LIBORBISUTIL_HTTP_CTX_DEFAULT;
			inline int memory_id = LIBORBISUTIL_HTTP_CTX_DEFAULT;
			inline int http_ctx = LIBORBISUTIL_HTTP_CTX_DEFAULT;
			inline int ssl_ctx = LIBORBISUTIL_HTTP_CTX_DEFAULT;

			inline std::unordered_map<std::string, connection> connections;

			bool load_modules();

			response get(connection& conn, request req);
			response post(connection& conn, request req);
			response put(connection& conn, request req);
			response del(connection& conn, request req);
			response download(connection& conn, request req, const std::string& path);
		}

		response get(const std::string& uri, const std::unordered_map<std::string, std::string>& headers = {});
		response post(const std::string& uri, const std::string& data, const std::unordered_map<std::string, std::string>& headers = {});
		response put(const std::string& uri, const std::string& data, const std::unordered_map<std::string, std::string>& headers = {});
		response del(const std::string& uri, const std::unordered_map<std::string, std::string>& headers = {});
		response download(const std::string& uri, const std::string& path, const std::unordered_map<std::string, std::string>& headers = {});

		connection find_connection(const std::string& url);
		connection create_connection(const std::string& url, const std::string& user_agent = "liborbisutil");
		void destroy_connection(connection conn);
		void destroy_connections();

		bool initialize(int http_context_override = LIBORBISUTIL_HTTP_CTX_DEFAULT);
		void finalize();
	}
}