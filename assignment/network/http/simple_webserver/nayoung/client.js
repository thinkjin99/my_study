const http = require("http");

const request = () => {
  const startTime = Date.now();

  const options = {
    hostname: "localhost",
    port: 8080,
    path: "/article1.html",
    method: "GET",
  };

  const req = http.request(options, (res) => {
    let data = "";

    res.on("data", (chunk) => {
      data += chunk;
    });

    res.on("end", () => {
      const endTime = Date.now();
      const elapsedTime = endTime - startTime;
      console.log(`Request completed in ${elapsedTime} milliseconds`);
    });
  });

  req.on("error", (err) => {
    console.error(`Request error: ${err}`);
  });

  req.end();
};

const parallelRequests = (num) => {
  for (let i = 0; i < num; i++) {
    request();
  }
};

const numRequests = 100;
parallelRequests(numRequests);
