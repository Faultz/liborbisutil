#include "http.h"

#include "../system/filesystem.h"

#define DEBUG_HTTP

namespace liborbisutil
{
	namespace http
	{
		namespace internal
		{
			bool load_modules()
			{
				auto res = resolve::load_module("libSceHttp.sprx");
				if (!res)
				{
					LOG_ERROR("Failed to load libhttp\n");
					return false;
				}

				res = resolve::load_module("libSceSsl2.sprx");
				if (!res)
				{
					LOG_ERROR("Failed to load libssl\n");
					return false;
				}

				return true;
			}

			bool parse_response_header(const char* const header, char** const nameStr, char** const valueStr)
			{
				const char* start;
				const char* end;
				uint32_t nameLen;
				uint32_t valueLen;
				bool success = false;

				if (header == NULL || nameStr == NULL || valueStr == NULL)
					return -1;

				if ((start = strchr(header, ':')) == NULL)
					return false;

				end = header;
				end += strlen(header);

				nameLen = start - header;
				*nameStr = (char*)malloc(nameLen + 1);

				if (*nameStr != NULL)
				{
					strncpy((char*)*nameStr, header, nameLen);
					(*nameStr)[nameLen] = '\0';
					start++;
					valueLen = end - start;
					*valueStr = (char*)malloc(valueLen + 1);

					if (*valueStr != NULL)
					{
						strncpy((char*)*valueStr, start, valueLen);
						(*valueStr)[valueLen] = '\0';
						success = true;
					}
					else
					{
						delete* nameStr;
					}
				}

				return success;
			}
			std::unordered_map<std::string, std::string> parse_response_headers(int request_id)
			{
				size_t bytes;
				char* headers;
				const char* header;
				std::unordered_map<std::string, std::string> responseHeaders;

				if (int res = sceHttpGetAllResponseHeaders(request_id, &headers, &bytes) < 0)
				{
					LOG_ERROR("Failed to fetch all response headers: 0x%X", res);

					return responseHeaders;
				}

				char* name;
				char* value;
				char* stringToParse = headers;
				while ((header = strtok(stringToParse, "\r\n")) != nullptr)
				{
					stringToParse = NULL;
					name = NULL;
					value = NULL;
					if (parse_response_header(header, &name, &value))
					{
						responseHeaders[name] = value;
					}
				}

				return responseHeaders;
			}

			response get(connection& conn, request req)
			{
				auto request_id = sceHttpCreateRequestWithURL(conn.id, req.method, req.url.c_str(), 0);
				if (request_id < 0)
				{
					LOG_ERROR("Failed to create request: 0x%08X\n", request_id);
					return response();
				}

#ifdef DEBUG_HTTP
				LOG_DEBUG("[%i] Created request: %s\n", conn.id, req.url.data());
#endif

				for (auto& header : req.headers)
				{
					auto res = sceHttpAddRequestHeader(request_id, header.first.c_str(), header.second.c_str(), 0);
					if (res < 0)
					{
						LOG_ERROR("Failed to add header: 0x%08X\n", res);

						sceHttpDeleteRequest(request_id);
						return response();
					}

#ifdef DEBUG_HTTP
					LOG_DEBUG("[%i] Added header: %s: %s\n", conn.id, header.first.data(), header.second.data());
#endif
				}

				auto res = sceHttpSendRequest(request_id, nullptr, 0);
				if (res < 0)
				{
					LOG_ERROR("Failed to send request: 0x%08X\n", res);
					sceHttpDeleteRequest(request_id);
					return response();
				}

#ifdef DEBUG_HTTP
				LOG_DEBUG("[%i] Sent request\n", conn.id);
#endif

				std::string data;
				char buffer[4096];
				size_t bytes_read;
				while ((bytes_read = sceHttpReadData(request_id, buffer, sizeof(buffer))) > 0)
				{
					data.append(buffer, bytes_read);
				}

				int status_code = 0;
				res = sceHttpGetStatusCode(request_id, &status_code);
				if (res < 0)
				{
					LOG_ERROR("Failed to get status code: 0x%08X\n", res);
					sceHttpDeleteRequest(request_id);
					return response();
				}

#ifdef DEBUG_HTTP
				LOG_DEBUG("[%i] Got status code: %i\n", conn.id, status_code);
#endif

				auto headers = parse_response_headers(request_id);

				conn.responses.push_back(response(true, request_id, status_code, req.method, data, headers));
				return conn.responses.back();
			}
			response post(connection& conn, request req)
			{

				auto request_id = sceHttpCreateRequestWithURL(conn.id, req.method, req.url.data(), 0);
				if (request_id < 0)
				{
					LOG_ERROR("Failed to create request: 0x%08X\n", request_id);
					return response();
				}

#ifdef DEBUG_HTTP
				LOG_DEBUG("[%i] Created request: %s\n", conn.id, req.url.data());
#endif

				for (auto& header : req.headers)
				{
					auto res = sceHttpAddRequestHeader(request_id, header.first.c_str(), header.second.c_str(), 0);
					if (res < 0)
					{
						LOG_ERROR("Failed to add header: 0x%08X\n", res);

						sceHttpDeleteRequest(request_id);
						return response();
					}

#ifdef DEBUG_HTTP
					LOG_DEBUG("[%i] Added header: %s: %s\n", conn.id, header.first.data(), header.second.data());
#endif
				}

				sceHttpSetRequestContentLength(request_id, req.data.size());
				auto res = sceHttpSendRequest(request_id, req.data.c_str(), req.data.size());
				if (res < 0)
				{
					LOG_ERROR("Failed to send request: 0x%08X\n", res);
					sceHttpDeleteRequest(request_id);
					return response();
				}

#ifdef DEBUG_HTTP
				LOG_DEBUG("[%i] Sent request\n", conn.id);
#endif

				std::string data;
				char buffer[4096];
				size_t bytes_read;
				while ((bytes_read = sceHttpReadData(request_id, buffer, sizeof(buffer))) > 0)
				{
					data.append(buffer, bytes_read);
				}

				int status_code = 0;
				res = sceHttpGetStatusCode(request_id, &status_code);
				if (res < 0)
				{
					LOG_ERROR("Failed to get status code: 0x%08X\n", res);
					sceHttpDeleteRequest(request_id);
					return response();
				}

#ifdef DEBUG_HTTP
				LOG_DEBUG("[%i] Got status code: %i\n", conn.id, status_code);
#endif

				auto headers = parse_response_headers(request_id);

				conn.responses.push_back(response(true, request_id, status_code, req.method, data, headers));
				return conn.responses.back();
			}
			response put(connection& conn, request req)
			{
				auto request_id = sceHttpCreateRequestWithURL(conn.id, req.method, req.url.c_str(), 0);
				if (request_id < 0)
				{
					LOG_ERROR("Failed to create request: 0x%08X\n", request_id);
					return response();
				}

#ifdef DEBUG_HTTP
				LOG_DEBUG("[%i] Created request: %s\n", conn.id, req.url.data());
#endif

				for (auto& header : req.headers)
				{
					auto res = sceHttpAddRequestHeader(request_id, header.first.c_str(), header.second.c_str(), 0);
					if (res < 0)
					{
						LOG_ERROR("Failed to add header: 0x%08X\n", res);
						sceHttpDeleteRequest(request_id);
						return response();
					}
				}
				sceHttpSetRequestContentLength(request_id, req.data.size());
				auto res = sceHttpSendRequest(request_id, req.data.c_str(), req.data.size());
				if (res < 0)
				{
					LOG_ERROR("Failed to send request: 0x%08X\n", res);
					sceHttpDeleteRequest(request_id);
					return response();
				}

#ifdef DEBUG_HTTP
				LOG_DEBUG("[%i] Sent request\n", conn.id);
#endif

				std::string data;
				char buffer[4096];
				size_t bytes_read;
				while ((bytes_read = sceHttpReadData(request_id, buffer, sizeof(buffer))) > 0)
				{
					data.append(buffer, bytes_read);
				}
				int status_code = 0;
				res = sceHttpGetStatusCode(request_id, &status_code);
				if (res < 0)
				{
					LOG_ERROR("Failed to get status code: 0x%08X\n", res);
					sceHttpDeleteRequest(request_id);
					return response();
				}

#ifdef DEBUG_HTTP
				LOG_DEBUG("[%i] Got status code: %i\n", conn.id, status_code);
#endif

				auto headers = parse_response_headers(request_id);
				conn.responses.push_back(response(true, request_id, status_code, SCE_HTTP_METHOD_PUT, data, headers));
				return conn.responses.back();
			}
			response del(connection& conn, request req)
			{
				auto request_id = sceHttpCreateRequestWithURL(conn.id, req.method, req.url.c_str(), 0);
				if (request_id < 0)
				{
					LOG_ERROR("Failed to create request: 0x%08X\n", request_id);
					return response();
				}

#ifdef DEBUG_HTTP
				LOG_DEBUG("[%i] Created request: %s\n", conn.id, req.url.data());
#endif

				auto res = sceHttpSendRequest(request_id, nullptr, 0);
				if (res < 0)
				{
					LOG_ERROR("Failed to send request: 0x%08X\n", res);
					sceHttpDeleteRequest(request_id);
					return response();
				}

#ifdef DEBUG_HTTP
				LOG_DEBUG("[%i] Sent request\n", conn.id);
#endif

				std::string data;
				char buffer[4096];
				size_t bytes_read;
				while ((bytes_read = sceHttpReadData(request_id, buffer, sizeof(buffer))) > 0)
				{
					data.append(buffer, bytes_read);
				}
				int status_code = 0;
				res = sceHttpGetStatusCode(request_id, &status_code);
				if (res < 0)
				{
					LOG_ERROR("Failed to get status code: 0x%08X\n", res);
					sceHttpDeleteRequest(request_id);
					return response();
				}

#ifdef DEBUG_HTTP
				LOG_DEBUG("[%i] Got status code: %i\n", conn.id, status_code);
#endif

				auto headers = parse_response_headers(request_id);
				conn.responses.push_back(response(true, request_id, status_code, SCE_HTTP_METHOD_DELETE, data, headers));
				return conn.responses.back();
			}
			response download(connection& conn, request req, const std::string& path)
			{
				auto request_id = sceHttpCreateRequestWithURL(conn.id, req.method, req.url.data(), 0);
				if (request_id < 0)
				{
					LOG_ERROR("Failed to create request: 0x%08X\n", request_id);
					return response();
				}

#ifdef DEBUG_HTTP
				LOG_DEBUG("[%i] Created request: %s\n", conn.id, req.url.data());
#endif

				for (auto& header : req.headers)
				{
					auto res = sceHttpAddRequestHeader(request_id, header.first.c_str(), header.second.c_str(), 0);
					if (res < 0)
					{
						LOG_ERROR("Failed to add header: 0x%08X\n", res);
						sceHttpDeleteRequest(request_id);
						return response();
					}
				}

				auto res = sceHttpSendRequest(request_id, nullptr, 0);
				if (res < 0)
				{
					LOG_ERROR("Failed to send request: 0x%08X\n", res);
					sceHttpDeleteRequest(request_id);
					return response();
				}

#ifdef DEBUG_HTTP
				LOG_DEBUG("[%i] Sent request\n", conn.id);
#endif

				int result;
				uintptr_t content_length;
				res = sceHttpGetResponseContentLength(request_id, &result, &content_length);
				if (res < 0)
				{
					LOG_ERROR("Failed to get response content length: 0x%08X\n", res);
					sceHttpDeleteRequest(request_id);
					return response();
				}

				file_system out(path, "w");
				if (!out.is_open())
				{
					LOG_ERROR("Failed to open file: %s\n", path.data());
					sceHttpDeleteRequest(request_id);
					return response();
				}

				char buffer[4096];
				size_t bytes_read;
				size_t total_bytes_read = 0;
				while(content_length > 0)
				{
					bytes_read = sceHttpReadData(request_id, buffer, sizeof(buffer));
					if (bytes_read < 0)
					{
						LOG_ERROR("Failed to read data: 0x%08X\n", bytes_read);
						sceHttpDeleteRequest(request_id);
						return response();
					}
					if (bytes_read == 0)
						break;

					total_bytes_read += bytes_read;
					out.write(buffer, bytes_read);
				}

				out.close();

				int status_code = 0;
				res = sceHttpGetStatusCode(request_id, &status_code);
				if (res < 0)
				{
					LOG_ERROR("Failed to get status code: 0x%08X\n", res);
					sceHttpDeleteRequest(request_id);
					return response();
				}

#ifdef DEBUG_HTTP
				LOG_DEBUG("[%i] Got status code: %i\n", conn.id, status_code);
#endif

				auto headers = parse_response_headers(request_id);
				conn.responses.push_back(response(true, request_id, status_code, req.method, "", headers));
				return conn.responses.back();
			}
		}

		response get(const std::string& uri, const std::unordered_map<std::string, std::string>& headers)
		{
			if(!internal::is_running)
			{
				LOG_ERROR("Http is not initialized\n");
				return response();
			}
			request req = request(uri, "", SCE_HTTP_METHOD_GET, headers);

			auto conn = find_connection(uri);
			if (conn.id == -1)
			{
				conn = create_connection(uri, "Mozilla/5.0 (Playerstation 4; 42.0)");

				auto connection_id = sceHttpCreateConnectionWithURL(internal::template_id, uri.c_str(), 0);
				if (connection_id < 0)
				{
					LOG_ERROR("Failed to create connection: 0x%08X\n", connection_id);
					return response();
				}

				conn.id = connection_id;
				conn.requests.push_back(req);

				internal::connections[uri] = conn;
			}

			return internal::get(internal::connections[uri], req);
		}
		response post(const std::string& uri, const std::string& data, const std::unordered_map<std::string, std::string>& headers)
		{
			if (!internal::is_running)
			{
				LOG_ERROR("Http is not initialized\n");
				return response();
			}
			request req = request(uri, data, SCE_HTTP_METHOD_POST, headers);

			auto conn = find_connection(uri);
			if (conn.id == -1)
			{
				conn = create_connection(uri, "Mozilla/5.0 (Playerstation 4; 42.0)");

				auto connection_id = sceHttpCreateConnectionWithURL(internal::template_id, uri.c_str(), 0);
				if (connection_id < 0)
				{
					LOG_ERROR("Failed to create connection: 0x%08X\n", connection_id);
					return response();
				}

				conn.id = connection_id;
				conn.requests.push_back(req);

				internal::connections[uri] = conn;
			}

			return internal::post(internal::connections[uri], req);
		}
		response put(const std::string& uri, const std::string& data, const std::unordered_map<std::string, std::string>& headers)
		{
			if (!internal::is_running)
			{
				LOG_ERROR("Http is not initialized\n");
				return response();
			}

			request req = request(uri, data, SCE_HTTP_METHOD_PUT, headers);
			auto conn = find_connection(uri);
			if (conn.id == -1)
			{
				conn = create_connection(uri, "Mozilla/5.0 (Playerstation 4; 42.0)");
				auto connection_id = sceHttpCreateConnectionWithURL(internal::template_id, uri.c_str(), 0);
				if (connection_id < 0)
				{
					LOG_ERROR("Failed to create connection: 0x%08X\n", connection_id);
					return response();
				}

				conn.id = connection_id;
				conn.requests.push_back(req);
				internal::connections[uri] = conn;
			}

			return internal::put(internal::connections[uri], req);
		}
		response del(const std::string& uri, const std::unordered_map<std::string, std::string>& headers)
		{
			if (!internal::is_running)
			{
				LOG_ERROR("Http is not initialized\n");
				return response();
			}
			request req = request(uri, "", SCE_HTTP_METHOD_DELETE, headers);
			auto conn = find_connection(uri);
			if (conn.id == -1)
			{
				conn = create_connection(uri, "Mozilla/5.0 (Playerstation 4; 42.0)");
				auto connection_id = sceHttpCreateConnectionWithURL(internal::template_id, uri.c_str(), 0);
				if (connection_id < 0)
				{
					LOG_ERROR("Failed to create connection: 0x%08X\n", connection_id);
					return response();
				}
				conn.id = connection_id;
				conn.requests.push_back(req);
				internal::connections[uri] = conn;
			}
			return internal::del(internal::connections[uri], req);
		}
		response download(const std::string& uri, const std::string& path, const std::unordered_map<std::string, std::string>& headers)
		{
			if (!internal::is_running)
			{
				LOG_ERROR("Http is not initialized\n");
				return response();
			}

			request req = request(uri, path, "", SCE_HTTP_METHOD_GET, headers);

			auto conn = find_connection(uri);
			if (conn.id == -1)
			{
				conn = create_connection(uri, "Mozilla/5.0 (Playerstation 4; 42.0)");
				auto connection_id = sceHttpCreateConnectionWithURL(internal::template_id, uri.c_str(), 0);
				if (connection_id < 0)
				{
					LOG_ERROR("Failed to create connection: 0x%08X\n", connection_id);
					return response();
				}
				conn.id = connection_id;
				conn.requests.push_back(req);
				internal::connections[uri] = conn;
			}
			return internal::download(internal::connections[uri], req, path);
		}

		connection find_connection(const std::string& url)
		{
			auto it = internal::connections.find(url);
			if (it != internal::connections.end())
			{
				return it->second;
			}
			return connection();
		}
		connection create_connection(const std::string& url, const std::string& user_agent)
		{
			connection conn;
			conn.id = -1;
			conn.url = url;
			conn.user_agent = user_agent;
			return conn;
		}

		void destroy_connection(connection conn)
		{
			if (conn.id != -1)
			{
				for (auto& res : conn.responses)
				{
					sceHttpDeleteRequest(res.req_id);

					LOG_INFO("Deleting Request: %d\n", res.req_id);
				}

				LOG_INFO("Deleting Connection: %d\n", conn.id);
				sceHttpDeleteConnection(conn.id);
				conn.id = -1;
			}
		}
		void destroy_connections()
		{
			for (auto& connection : internal::connections)
			{
				destroy_connection(connection.second);
			}
		}

		bool initialize(int http_context_override)
		{
			if(!internal::load_modules())
			{
				LOG_ERROR("Failed to load modules\n");
				return false;
			}

			auto res = sceNetInit();
			if (res < 0)
			{
				LOG_ERROR("Failed to initialize net: 0x%08X\n", res);
				return false;
			}

			if (http_context_override != LIBORBISUTIL_HTTP_CTX_DEFAULT)
			{
				internal::http_ctx = http_context_override;

				auto res = sceHttpCreateTemplate(http_context_override, "Mozilla/5.0 (Playstation 4; 42.0)", SCE_HTTP_VERSION_1_1, true);
				if (res < 0)
				{
					LOG_ERROR("Failed to create http template: 0x%08X\n", res);
					return false;
				}

				internal::template_id = res;
				internal::using_override = true;
				internal::is_running = true;

				LOG_WARNING("Initialized http with context override: %i\n", http_context_override);

				return true;
			}
			else
			{
				res = sceNetPoolCreate("liborbisutil http pool", 4_KB, 0);
				if (res < 0)
				{
					LOG_ERROR("Failed to create net pool: 0x%08X\n", res);
					return false;
				}

				internal::memory_id = res;

				LOG_DEBUG("Created net pool: %i\n", internal::memory_id);

				res = sceSslInit(128_KB);
				if (res < 0)
				{
					LOG_ERROR("Failed to initialize ssl: 0x%08X\n", res);
					finalize();
					return false;
				}

				internal::ssl_ctx = res;

				LOG_DEBUG("Initialized ssl: %i\n", res);

				res = sceHttpInit(internal::memory_id, internal::ssl_ctx, 64_KB);
				if (res < 0)
				{
					LOG_ERROR("Failed to initialize http: 0x%08X\n", res);
					finalize();
					return false;
				}

				internal::http_ctx = res;

				LOG_DEBUG("Initialized http: %i\n", res);

				res = sceHttpCreateTemplate(internal::http_ctx, "Mozilla/5.0 (Playerstation 4; 42.0)", SCE_HTTP_VERSION_1_1, true);
				if (res < 0)
				{
					LOG_ERROR("Failed to create http template: 0x%08X\n", res);
					finalize();
					return false;
				}

				internal::template_id = res;
				internal::is_running = true;

				LOG_SUCCESS("Initialized http\n");

				return true;
			}

			return false;
		}
		void finalize()
		{
			if (!internal::is_running)
				return;

			destroy_connections();

			if (internal::using_override)
			{
				auto res = sceHttpDeleteTemplate(internal::template_id);
				if (res < 0)
				{
					LOG_ERROR("Failed to delete http template: 0x%08X\n", res);
				}
			}
			else
			{
				if (internal::template_id != LIBORBISUTIL_HTTP_CTX_DEFAULT)
				{
					auto res = sceHttpDeleteTemplate(internal::template_id);
					if (res < 0)
					{
						LOG_ERROR("Failed to delete http template: 0x%08X\n", res);
					}
				}

				if (internal::http_ctx != LIBORBISUTIL_HTTP_CTX_DEFAULT)
				{
					auto res = sceHttpTerm(internal::http_ctx);
					if (res < 0)
					{
						LOG_ERROR("Failed to terminate http: 0x%08X\n", res);
					}
				}

				if (internal::ssl_ctx != LIBORBISUTIL_HTTP_CTX_DEFAULT)
				{
					auto res = sceSslTerm(internal::ssl_ctx);
					if (res < 0)
					{
						LOG_ERROR("Failed to terminate ssl: 0x%08X\n", res);
					}
				}

				if (internal::memory_id != LIBORBISUTIL_HTTP_CTX_DEFAULT)
				{
					auto res = sceNetPoolDestroy(internal::memory_id);
					if (res < 0)
					{
						LOG_ERROR("Failed to destroy net pool: 0x%08X\n", res);
					}
				}
			}

			internal::is_running = false;
		}
	}
}