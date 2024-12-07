# Hearty Store Project

## Project Structure

### Server-side Files (`include/`)
- Contains server-related header files and implementations
- Handles server-side logic and data management

### Client-side Files (`src/`)
- Contains client-side implementations
- Exception: `hearty-store-server.cpp` which is a server-side component

## Running the Application

### Starting the Server
1. Execute the reset script:

```bash
./reset-server.sh
```

### Running Test Cases
1. Make sure the server is running
2. Run the test cases from the client side:

```bash
./testcases.sh
```

## Note
Ensure the server is properly initialized before running any client-side tests or operations.
