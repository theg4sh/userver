#include <clients/http/response.hpp>
#include <clients/http/response_future.hpp>

namespace clients::http {

std::ostream& operator<<(std::ostream& os, Status s) {
  switch (s) {
    case OK:
      return os << "200 OK";
    case Created:
      return os << "201 Created";
    case NoContent:
      return os << "204 NoContent";
    case BadRequest:
      return os << "400 BadRequest";
    case NotFound:
      return os << "404 Not Found";
    case Conflict:
      return os << "409 Conflict";
    case InternalServerError:
      return os << "500 Internal Server Error";
    case BadGateway:
      return os << "502 Bad Gateway";
    case ServiceUnavailable:
      return os << "503 Service Unavailable";
    case GatewayTimeout:
      return os << "504 Gateway Timeout";
    case InsufficientStorage:
      return os << "507 Insufficient Storage";
    case BandwithLimitExceeded:
      return os << "509 Bandwith Limit Exceeded";
    case WebServerIsDown:
      return os << "520 Web Server Is Down";
    case ConnectionTimedOut:
      return os << "522 Connection Timed Out";
    case OriginIsUnreachable:
      return os << "523 Origin Is Unreachable";
    case TimeoutOccured:
      return os << "524 A Timeout Occured";
    case SslHandshakeFailed:
      return os << "525 SSL Handshake Failed";
    case InvalidSslCertificate:
      return os << "526 Invalid SSL Certificate";
  }

  return os << static_cast<std::underlying_type_t<Status>>(s);
}

Status Response::status_code() const { return status_code_; }

void Response::RaiseForStatus(long code) {
  if (400 <= code && code < 500)
    throw HttpClientException(code);
  else if (500 <= code && code < 600)
    throw HttpServerException(code);
}

void Response::raise_for_status() const { RaiseForStatus(status_code()); }

curl::LocalStats Response::GetStats() const { return stats_; }

}  // namespace clients::http
