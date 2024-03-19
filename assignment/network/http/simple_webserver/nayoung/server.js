const http = require("http");
const fs = require("fs");
const url = require("url");

const HTTP_RESPONSE_TEMPLATE = (status_code, content_type, content) => {
  return `HTTP/1.1 ${status_code}\nContent-Type: ${content_type}\n${content}`;
};

const handleRequest = (request, response) => {
  const parsedUrl = url.parse(request.url);

  // random timeout
  const timeout = Math.floor(Math.random() * 1000);
  setTimeout(() => {
    console.log(`Request for ${parsedUrl.pathname} received`);
  }, timeout);

  if (request.method === "GET") {
    let filePath = "./" + parsedUrl.pathname;

    // "/" 경로인 경우에는 index.html로 설정
    if (filePath === "./") {
      filePath = "./index.html";
    }

    fs.readFile(filePath, (err, data) => {
      if (err) {
        const errMessage = HTTP_RESPONSE_TEMPLATE(
          500,
          "text/plain",
          "Server Error"
        );
        response.writeHead(500);
        response.end(errMessage);
        console.log(`Error: ${err}`);
        return;
      }

      const responseMessage = HTTP_RESPONSE_TEMPLATE(200, "text/html", data);
      response.writeHead(200);
      response.end(responseMessage);
      console.log(`File served: ${filePath}`);
    });
  } else {
    const errMessage = HTTP_RESPONSE_TEMPLATE(
      405,
      "text/plain",
      "Method Not Allowed"
    );
    response.writeHead(405);
    response.end(errMessage);
    console.log(`Error: Method Not Allowed - ${request.method}`);
  }
};

const server = http.createServer(handleRequest);

server.listen(8080, () => {
  console.log("Server is running on port 8080");
});
