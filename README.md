
# HTTP Server in C
 
![Language](https://img.shields.io/badge/language-C-blue)


A minimalist **HTTP server** written in C.  
Supports static file serving and dynamic directory listing.

---
## Project Structure

```
http-simple/
├── include/    – header files
├── src/        – source code
│   ├── main.c
│   ├── socket_setup.c
│   ├── http_handler.c
├── static/     – static assets (HTML, CSS, etc.)
├── playground/ – test and experiment code
├── Makefile    – build instructions
└── README.md   – project documentation
```

---

##  Features

-  **Static File Serving**
   - Serves HTML, CSS, JS, images, text, video files from the `static/` directory.
-  **Directory Listing**
   - Browsable directory views, including `..` to navigate up.

---
##  How it works

-   **Socket setup**: `socket()`, `bind()`, `listen()`, `accept()` create a TCP server on port specified.
    
-   **Request parsing**: Read client data with `recv()`, extract method and path.
    
-   **File serving**:
    
    -   Use `open()`, `fread()`, `send()` to return file contents.
        
    -   Detect type (file/dir) with `stat()`.
        
-   **Directory listing**:
    
    -   Use `opendir()`, `readdir()` to read contents.
        
    -   Generate HTML on the fly and `send()` line by line.
        
-   **Response headers**: Sent using `dprintf()` for status, `Content-Type`, and `Content-Length` (when known).

---

## Building

```bash
make
```

To clean build artifacts:

```bash
make clean
```

---

##  Usage



```bash
bin/http-simple
```
---


## References

Socket programming and networking concepts inspired by _Beej’s Guide to Network Programming_:  
https://beej.us/guide/bgnet/



---

## License

This project is licensed under the [MIT License](./LICENSE).

---

