# Mock Server

This is a configurable mock server in Python that responds to specific endpoints with predefined responses or files. It can operate over HTTP or HTTPS based on provided arguments.

## Features

- **JWT Authentication**: The `/security/user/authenticate` and `/api/v1/authentication` endpoints generate and return a JWT token with a configurable expiration time.
- **File Serving**: The `/api/v1/files` endpoint serves files from a specified directory.
- **Custom Endpoints**: Includes endpoints like `/api/v1/events/commands`, each returning mock JSON responses from local files.

## Prerequisites

- **Python 3.6+**
- **Libraries**: Install the required libraries using the following:

  ```bash
  pip install pyjwt
  ```

## Usage
### Generate SSL Certificates for HTTPS
To run the server with HTTPS, you need SSL certificates. You can create a self-signed certificate using OpenSSL:

```bash
openssl req -newkey rsa:2048 -nodes -keyout key.pem -x509 -days 365 -out cert.pem
```

### Running the Server
You can configure and start the server by specifying the desired ports and SSL certificate paths through command-line arguments.

#### Command-Line Arguments
 - `--port1`: Port number for the first server instance (default: 55000)
 - `--port2`: Port number for the second server instance (default: 27000)
 - `--ssl_key`: Path to the SSL key file for HTTPS (default: key.pem)
 - `--ssl_cert`: Path to the SSL certificate file for HTTPS (default: cert.pem)
 - `--protocol`: Specify the protocol to use (http or https). Defaults to https.
 - `--outfile`: File path to save incoming request logs

#### Examples
Run the server over HTTPS (default):

```bash
python3 mock_server.py --port1 55000 --port2 27000
```

Run the server over HTTP:

```bash
python3 mock_server.py --port1 55000 --port2 27000 --protocol http
```

## Endpoints

### POST Endpoints
 - `/security/user/authenticate`: Returns a generated JWT token with a configurable expiration time.
 - `/agents`: Returns successful response.
 - `/api/v1/authentication`: Returns a generated JWT token with a configurable expiration time.
 - `/api/v1/events/stateful`: Returns a mock response from `responses/events_stateful.json`.
 - `/api/v1/events/stateless`: Returns a mock response from `responses/events_stateless.json`.

### GET Endpoints
 - `/api/v1/commands`: Returns a mock response from `responses/commands.json`.
 - `/api/v1/files?file_name=<file_name>`: Serves a specified file from the `group_files` directory.

## Directory Structure
To serve files and responses, organize your directory structure as follows:

```
mock_server/
    responses/
        commands.json
        events_stateful.json
        events_stateless.json
    group_files/
        file1.txt
        file2.conf
    cert.pem
    key.pem
    mock_server.py
```

## License
This project is open-source. Use it as a reference for your own mock server setup and customization.
