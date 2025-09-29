# Ngin42 - a simple HTTP Webserver written in C++98

### [125/125]

## Overview

Ngin42 is a C++98 implementation of a non-blocking HTTP/1.1 server built around the Linux `epoll` API. The project targets feature parity with a subset of nginx: it supports multiple virtual hosts, per-location configuration, CGI execution, file uploads, and connection-level timeouts while remaining compatible with the constraints of the 42 school curriculum.

## Table of Contents

- [Feature Summary](#feature-summary)
- [Architecture](#architecture)
  - [Event Loop and Connection Model](#event-loop-and-connection-model)
  - [Request Lifecycle](#request-lifecycle)
  - [CGI Subsystem](#cgi-subsystem)
  - [Cookie Management](#cookie-management)
- [Build and Run](#build-and-run)
- [Configuration Reference](#configuration-reference)
- [HTTP Capabilities](#http-capabilities)
- [Project Layout](#project-layout)
- [Development Workflow](#development-workflow)
- [Testing Resources](#testing-resources)
- [Extending the Server](#extending-the-server)
- [Team Members](#team-members)
- [License](#license)

## Feature Summary

- Event-driven loop with edge-safe connection management
- Virtual host resolution based on `Host` headers and listen directives
- Static file serving with autoindex support and MIME type detection
- HTTP methods: `GET`, `POST`, and `DELETE` with configurable allowlists
- Chunked upload handling and disk persistence under configurable directories
- Built-in CGI gateway for `.py`, `.php`, and `.sh` scripts with interpreter mapping
- Custom and fallback error pages for the full HTTP status range
- Session cookie management with auto-generated identifiers
- Per-server and per-location body size limits, redirects, and error overrides

## Architecture

### Event Loop and Connection Model

- `main.cpp` seeds configuration, instantiates listening sockets via `Socket::initSockets`, and drives the `Controller` inside an `epoll` loop.
- `Controller` owns every active file descriptor in `_connections`, tagged by `con_type`:
  - `CON_SERVER`: listening sockets accepting inbound clients.
  - `CON_CLIENT`: live client connections with read/write buffers and activity timestamps.
  - `CON_CGI`: pipes for CGI child processes (stdin/stdout pairs).
- Each loop iteration dispatches `EPOLLIN`, `EPOLLOUT`, and `EPOLLHUP` events, performing reads, writes, or cleanup as needed.
- Idle sockets are reclaimed by `Controller::checkTimeouts`, which enforces a 15-second inactivity window for clients and a 30-second limit for CGI execution.

### Request Lifecycle

1. A readable client socket is drained into a connection-specific `readBuffer`.
2. `checkBody` verifies whether the HTTP message is complete; partial bodies remain buffered until the next event.
3. The `Request` constructor parses the request line, headers, and body, normalizing header casing and extracting metadata into `requestInfo`.
4. `Request::checkServer` selects the appropriate `Server` from the listen map and the best matching `Location` using longest-prefix logic.
5. `Controller::handleRequest` routes to:
   - `Response::handleGet` for static content, autoindex, or directory redirects.
   - `Response::handlePost` for multipart or raw uploads saved to the configured `upload_dir`.
   - `Response::handleDelete` for file removal when permitted.
   - `Controller::handleCGI` when the URI or configuration marks the resource as CGI-enabled.
6. `Response` composes headers (including `Set-Cookie` values collected from the `Cookie` subsystem) and sends the final payload back to the client.

### CGI Subsystem

- `Controller::handleCGI` validates the script path, locates the interpreter from `Location::cgi_extension`, and creates bidirectional pipes via `initPipes`.
- The parent process forks, with the child re-binding stdout/stdin to pipes and executing the interpreter using `execve`.
- Environment variables are assembled in `Controller::generateCGIEnv`, mirroring CGI/1.1 expectations (`REQUEST_METHOD`, `CONTENT_TYPE`, `SERVER_NAME`, etc.).
- The controller tracks CGI PIDs in `_cgiConnections` to enforce timeouts and map pipe events back to the initiating client connection.

### Cookie Management

- `Cookie` keeps a vector of `Client` entries, each bearing a generated eight-character session identifier.
- New visitors receive a `sessionId` cookie with a one-hour lifetime; returning clients are recognized by parsing the incoming `Cookie` header.
- `Controller::handleRequest` appends any `Set-Cookie` headers before preparing the response buffer.

## Build and Run

### Prerequisites

- Linux environment with `g++` supporting the C++98 standard
- Make utility

### Build Targets

```bash
make               # Build the webserv binary
make clean         # Remove object files
make fclean        # Remove objects and the binary
make re            # Rebuild from scratch
make c             # Generate compile_commands.json via compiledb (requires compiledb)
```

### Running the Server

```bash
make run default/config.conf   # Launch using the bundled configuration
```

To run under Valgrind:

```bash
make valgrind default/config.conf
```

Logs are printed to standard output; use your shell to redirect them if persistent logging is required.

## Configuration Reference

Configuration files must end with `.conf` and follow an nginx-like syntax. The parser validates every directive and emits descriptive exceptions for invalid values.

### Server-Level Directives

- `server_name`: list of hostnames matched against the `Host` header.
- `listen`: either a port or `ip:port` pair; multiple entries create distinct sockets.
- `root`: filesystem directory used when no location override is present.
- `index`: ordered list of default files inspected for directory requests.
- `methods`: bitmask of allowed methods (default `GET`).
- `autoindex`: `on` or `off`; controls directory listings when no index matches.
- `client_max_body_size`: limit in bytes applied to requests for this server.
- `upload_dir`: fallback directory for file uploads.
- `error_page <code> <path>`: per-status override; valid for codes 300–599.

### Location-Level Directives

- Inherit all server-level defaults unless explicitly overridden.
- `root`: directory fused with the request URI to locate resources or scripts.
- `methods`: restricts the allowed operations for the path prefix.
- `index`: ordered index files searched before autoindex.
- `autoindex`: enables directory listings when no index file resolves.
- `upload_dir`: destination for POST bodies tied to this prefix.
- `client_max_body_size`: per-location override of the request body limit.
- `cgi_extension <ext> <binary>`: maps `.py`, `.php`, or `.sh` to an interpreter.
- `return <code> <target>`: issues redirects; codes 301–308 require a location.
- `error_page`: override error documents scoped to this location.

Refer to `default/config.conf` for a comprehensive example featuring static content, uploads, CGI routes, and redirects.

## HTTP Capabilities

- **Static Files**: Served from the active `root`; directory requests append a trailing slash and redirect if needed.
- **Autoindex**: `Response::generateAutoindex` lists directory contents when `autoindex on`.
- **Uploads**: `Response::handlePost` saves bodies to `upload_dir`, returning status `201 Created` on success.
- **Deletion**: `Response::handleDelete` removes files and returns `204 No Content` when allowed.
- **Redirects**: `return` directives populate `serverInfo::to_client` and trigger `Response::handleRedirect`.
- **CGI**: Scripts run under the configured interpreter with environment variables mirroring nginx behavior.
- **Cookies**: Session identifiers propagate through `Set-Cookie` headers and are analyzed on subsequent requests.
- **Error Handling**: Missing files, permission issues, or system errors map to HTTP status via `Response::getErrnoHttpStatus`; custom error pages are served when available, otherwise a generated HTML fallback is returned.

## Project Layout

```
include/    # Public headers for core classes (Controller, Request, Response, etc.)
src/        # C++ source files implementing the server
www/        # Static assets, CGI scripts, uploads, and error pages
default/    # Sample configuration file(s)
obj/        # Build artifacts generated by make
```

Key entry points:

- `src/main.cpp`: program initialization and epoll loop.
- `src/Controller.cpp`: connection orchestration, routing, and CGI lifecycle.
- `src/Request.cpp`: HTTP parsing, host/location selection, and validation.
- `src/Response.cpp`: payload generation, autoindex, and error handling.
- `src/parser.cpp`: configuration parser with directive validation.
- `src/Cookie.cpp`: session cookie management utilities.

## Development Workflow

- Use `make` for standard compilation; rebuild individual translation units by editing their corresponding files under `src/`.
- `make c` integrates with tools that rely on `compile_commands.json` for code navigation or static analysis.
- Run `make valgrind default/config.conf` to check for leaks or invalid memory access; the build includes debugging symbols (`-g`) by default.
- When debugging request handling, `Request::printInfoRequest` and strategic logging within the `Controller` are viable inspection points.

## Testing Resources

The `www/` directory contains assets that exercise major features:

- `/` serves `www/index.html`, a dashboard linking to feature demos.
- `/cgi-bin/` includes Python, PHP, and shell scripts highlighting CGI environment variables and request bodies.
- `/upload/` is configured for uploads and deletion tests under `www/uploads/`.
- `/files/` exposes a directory tree to demonstrate autoindex listings.
- `/protected/` produces `403 Forbidden`, while `/redirect` triggers a location redirect.

Example manual tests:

```bash
curl -i http://127.0.0.1:8080/
curl -i http://127.0.0.1:8080/cgi-bin/test.py
curl -i -X POST --data-binary @README.md http://127.0.0.1:8080/upload/
curl -i -X DELETE http://127.0.0.1:8080/upload/nomefile.json
```

## Extending the Server

- Add new virtual hosts or routes by editing a `.conf` file and restarting the binary. The parser validates the directives at startup, so syntax errors fail fast with explicit messages.
- Additional HTTP methods require extending the `Methods` enum in `parser.hpp`, updating the parser, and implementing handler logic in `Controller::handleRequest` and `Response`.
- To support more CGI extensions, enhance `parser.cpp` (look for the `cgi_extension` validation) and provide the corresponding interpreter mapping.

## Team Members

<table>
  <tr>
    <td align="center">
      <a href="https://github.com/cole43623">
        <img src="https://github.com/cole43623.png" width="100" alt="cole43623" />
        <br />
        cole43623
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/Livi-maker">
        <img src="https://github.com/Livi-maker.png" width="100" alt="Livi-maker" />
        <br />
        Livi-maker
      </a>
    </td>
  </tr>
</table>

## License

This project is part of the 42 curriculum and follows its academic policies.
