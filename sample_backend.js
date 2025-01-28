const http = require('http');

const server = http.createServer((req, res) => {
  if (req.url === '/backend') {
    res.writeHead(200, { 'Content-Type': 'text/html' });

    res.end(`
      <!DOCTYPE html>
      <html lang="en">
      <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Backend Server</title>
      </head>
      <body>
        <h1>Hello from the Backend!</h1>
        <p>This is a sample response from the /backend endpoint.</p>
      </body>
      </html>
    `);
  } else {
    res.writeHead(404, { 'Content-Type': 'text/plain' });
    res.end('404 Not Found');
  }
});

const PORT = 3000;
server.listen(PORT, () => {
  console.log(`Backend server is running on http://localhost:${PORT}`);
});
