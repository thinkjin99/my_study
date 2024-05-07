from http.server import ThreadingHTTPServer, CGIHTTPRequestHandler


def run(server_class=ThreadingHTTPServer, handler_class=CGIHTTPRequestHandler):
    server_address = ("", 8000)
    handler_class.cgi_directories = ["/cgi-bin"]
    httpd = server_class(server_address, handler_class)
    httpd.serve_forever()


if __name__ == "__main__":
    run()
