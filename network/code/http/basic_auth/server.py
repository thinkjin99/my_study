from http.server import BaseHTTPRequestHandler, HTTPServer
import base64

# 유효한 사용자 이름과 비밀번호
USERNAME = "user"
PASSWORD = "pass"


class RequestHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        # 인증 헤더 확인
        auth_header = self.headers.get("Authorization")

        if auth_header is None or not self.check_auth(auth_header):
            self.send_auth_request()
        else:
            self.send_response(200)
            self.send_header("Content-type", "text/html")
            self.end_headers()
            self.wfile.write(b"Authentication successful!")

    def check_auth(self, auth_header):
        auth_type, encoded_credentials = auth_header.split()
        if auth_type != "Basic":
            return False

        # Base64로 인코딩된 인증 정보 디코딩
        decoded_credentials = base64.b64decode(encoded_credentials).decode("utf-8")
        username, password = decoded_credentials.split(":")

        return username == USERNAME and password == PASSWORD

    def send_auth_request(self):
        self.send_response(401)
        self.send_header("WWW-Authenticate", 'Basic realm="Login Required"')
        self.send_header("Content-type", "text/html")
        self.end_headers()
        self.wfile.write(b"Authentication required.")


def run(server_class=HTTPServer, handler_class=RequestHandler, port=8080):
    server_address = ("", port)
    httpd = server_class(server_address, handler_class)
    print(f"Starting httpd server on port {port}")
    httpd.serve_forever()


if __name__ == "__main__":
    run()
