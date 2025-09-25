# DariX Native Libraries Documentation

DariX provides a comprehensive set of native libraries implemented in Go for high-performance operations. These libraries are accessible through the `import "go:module"` syntax and provide capabilities ranging from mathematical operations to network requests.

## 🔧 Security & Capabilities

All native libraries are protected by DariX's capability policy system:

```bash
# Allow all native modules
darix run --allow=* script.dax

# Allow specific modules only
darix run --allow=math,json,string script.dax

# Deny specific modules
darix run --deny=os,http script.dax

# Filesystem restrictions
darix run --fs-root=/safe/path --fs-ro script.dax
```

## 📚 Available Libraries

### 1. Math Library (`go:math`)

Advanced mathematical functions with full floating-point precision.

```dax
import "go:math";

// Basic functions
var result = math_sqrt(16);        // 4.0
var power = math_pow(2, 3);        // 8.0
var absolute = math_abs(-5);       // 5.0

// Trigonometric functions
var sine = math_sin(math_pi() / 2);     // 1.0
var cosine = math_cos(0);               // 1.0
var tangent = math_tan(math_pi() / 4);  // 1.0

// Inverse trigonometric
var arcsine = math_asin(1);        // π/2
var arccosine = math_acos(0);      // π/2
var arctangent = math_atan(1);     // π/4
var atan2 = math_atan2(1, 1);     // π/4

// Hyperbolic functions
var sinh = math_sinh(0);           // 0.0
var cosh = math_cosh(0);           // 1.0
var tanh = math_tanh(0);           // 0.0

// Logarithmic functions
var natural_log = math_log(math_e());    // 1.0
var log10 = math_log10(100);             // 2.0
var log2 = math_log2(8);                 // 3.0
var exponential = math_exp(1);           // e

// Rounding functions
var ceiling = math_ceil(3.2);      // 4.0
var floor = math_floor(3.8);       // 3.0
var rounded = math_round(3.6);     // 4.0
var truncated = math_trunc(3.9);   // 3.0

// Comparison functions
var maximum = math_max(1, 5, 3, 8);     // 8.0
var minimum = math_min(1, 5, 3, 8);     // 1.0

// Utility functions
var modulus = math_mod(7, 3);      // 1.0
var random = math_random();        // Random float [0.0, 1.0)

// Mathematical constants
var pi = math_pi();                // 3.141592653589793
var e = math_e();                  // 2.718281828459045
```

### 2. JSON Library (`go:json`)

High-performance JSON parsing and serialization.

```dax
import "go:json";

// JSON Serialization
var data = {
    "name": "DariX",
    "version": 1.0,
    "features": ["classes", "functions", "native"],
    "active": true,
    "config": null
};

var json_string = json_stringify(data);
print(json_string);
// Output: {"name":"DariX","version":1,"features":["classes","functions","native"],"active":true,"config":null}

// JSON Parsing
var json_text = '{"user": "admin", "id": 123, "permissions": ["read", "write"]}';
var parsed_data = json_parse(json_text);

print(parsed_data["user"]);        // "admin"
print(parsed_data["id"]);          // 123
print(len(parsed_data["permissions"])); // 2
```

### 3. HTTP Library (`go:http`)

Complete HTTP client for web requests with automatic JSON handling.

```dax
import "go:http";
import "go:json";

// GET Request
var response = http_get("https://api.example.com/users");
print("Status:", response["status"]);
print("Body:", response["body"]);

// POST Request with JSON
var post_data = json_stringify({"name": "John", "email": "john@example.com"});
var headers = {"Content-Type": "application/json"};
var post_response = http_post("https://api.example.com/users", post_data, headers);

// PUT Request
var put_data = json_stringify({"name": "Jane", "email": "jane@example.com"});
var put_response = http_put("https://api.example.com/users/1", put_data, headers);

// DELETE Request
var delete_response = http_delete("https://api.example.com/users/1");

// Response structure
// {
//   "status": 200,
//   "body": "response content",
//   "headers": {"Content-Type": "application/json", ...}
// }
```

### 4. String Library (`go:string`)

Enhanced string manipulation with Unicode support.

```dax
import "go:string";

var text = "  Hello, DariX World!  ";

// Case conversion
var upper = str_upper(text);       // "  HELLO, DARIX WORLD!  "
var lower = str_lower(text);       // "  hello, darix world!  "

// Trimming
var trimmed = str_trim(text);      // "Hello, DariX World!"
var left_trimmed = str_trim_left(text, " H");   // "ello, DariX World!  "
var right_trimmed = str_trim_right(text, "! ");  // "  Hello, DariX World"

// Splitting and joining
var words = str_split("apple,banana,cherry", ",");  // ["apple", "banana", "cherry"]
var joined = str_join(words, " | ");                // "apple | banana | cherry"

// Search and replace
var replaced = str_replace("Hello World", "World", "DariX");  // "Hello DariX"
var contains = str_contains("Hello World", "World");          // true
var starts = str_starts("Hello World", "Hello");             // true
var ends = str_ends("Hello World", "World");                 // true

// Indexing
var index = str_index("Hello World", "World");      // 6
var last_index = str_last_index("Hello World World", "World"); // 12

// Utility functions
var repeated = str_repeat("Ha", 3);                  // "HaHaHa"
var reversed = str_reverse("Hello");                 // "olleH"

// Character validation
var is_alpha = str_is_alpha("Hello");               // true
var is_digit = str_is_digit("12345");               // true
var is_space = str_is_space("   ");                 // true
```

### 5. Time Library (`go:time`)

Comprehensive date and time operations with Unix timestamp support.

```dax
import "go:time";

// Current time
var now = time_now();              // Current Unix timestamp
print("Current timestamp:", now);

// Time components
var year = time_year(now);         // 2024
var month = time_month(now);       // 1-12
var day = time_day(now);           // 1-31
var hour = time_hour(now);         // 0-23
var minute = time_minute(now);     // 0-59
var second = time_second(now);     // 0-59
var weekday = time_weekday(now);   // 0=Sunday, 1=Monday, ..., 6=Saturday

// Time arithmetic
var tomorrow = time_add_days(now, 1);
var next_week = time_add_days(now, 7);
var next_hour = time_add_hours(now, 1);

// Time difference (in seconds)
var diff = time_diff(tomorrow, now);  // 86400 (24 hours * 60 minutes * 60 seconds)

// Time formatting
var formatted = time_format(now, "YYYY-MM-DD HH:mm:ss");
print("Formatted time:", formatted);

// Time parsing
var timestamp = time_parse("YYYY-MM-DD", "2024-01-15");

// Sleep function
time_sleep(1);                     // Sleep for 1 second
time_sleep(0.5);                   // Sleep for 500 milliseconds
```

### 6. Crypto Library (`go:crypto`)

Cryptographic functions for hashing and encoding.

```dax
import "go:crypto";

var message = "Hello, DariX!";

// Hash functions
var md5_hash = crypto_md5(message);        // 32-character hex string
var sha1_hash = crypto_sha1(message);      // 40-character hex string
var sha256_hash = crypto_sha256(message);  // 64-character hex string
var sha512_hash = crypto_sha512(message);  // 128-character hex string

print("MD5:", md5_hash);
print("SHA256:", sha256_hash);

// Base64 encoding/decoding
var encoded = crypto_base64_encode(message);
var decoded = crypto_base64_decode(encoded);
print("Original:", message);
print("Encoded:", encoded);
print("Decoded:", decoded);

// Hexadecimal encoding/decoding
var hex_encoded = crypto_hex_encode(message);
var hex_decoded = crypto_hex_decode(hex_encoded);
print("Hex encoded:", hex_encoded);
print("Hex decoded:", hex_decoded);
```

### 7. OS Library (`go:os`)

Operating system interactions with security controls.

```dax
import "go:os";

// Environment variables
var path = os_getenv("PATH");
os_setenv("MY_VAR", "my_value");
var my_var = os_getenv("MY_VAR");
os_unsetenv("MY_VAR");

// Directory operations
var current_dir = os_getcwd();
print("Current directory:", current_dir);

os_mkdir("test_directory");
os_chdir("test_directory");
os_chdir("..");
os_rmdir("test_directory");

// File operations
os_rename("old_name.txt", "new_name.txt");
os_remove("unwanted_file.txt");

// System information
var platform = os_platform();     // "windows", "linux", "darwin"
var arch = os_arch();             // "amd64", "arm64", "386"
var hostname = os_hostname();
var pid = os_getpid();

print("Platform:", platform);
print("Architecture:", arch);
print("Hostname:", hostname);
print("Process ID:", pid);

// Command execution
var output = os_exec("echo Hello from system");
print("Command output:", output);

// Program termination
// os_exit(0);  // Exit with code 0
```

### 8. Regex Library (`go:regex`)

Full regular expression support with Go's regex engine.

```dax
import "go:regex";

var text = "The year 2024 has 365 days, and 2025 will have 365 days too.";

// Pattern matching
var has_numbers = regex_match("\\d+", text);  // true
print("Contains numbers:", has_numbers);

// Pattern validation
var valid_pattern = regex_test("[a-z]+");     // true
var invalid_pattern = regex_test("[a-z");     // false (unclosed bracket)

// Find operations
var first_number = regex_find("\\d+", text);           // "2024"
var all_numbers = regex_find_all("\\d+", text);        // ["2024", "365", "2025", "365"]
var limited_numbers = regex_find_all("\\d+", text, 2); // ["2024", "365"]

print("First number:", first_number);
print("All numbers:", all_numbers);

// Replace operations
var censored = regex_replace("\\d+", text, "XXXX");
print("Censored:", censored);
// Output: "The year XXXX has XXXX days, and XXXX will have XXXX days too."

// Split operations
var sentences = regex_split("[.!?]", "Hello world! How are you? Fine.");
print("Sentences:", sentences);  // ["Hello world", " How are you", " Fine", ""]

// Advanced patterns
var email_pattern = "[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}";
var email_text = "Contact us at info@example.com or support@test.org";
var emails = regex_find_all(email_pattern, email_text);
print("Found emails:", emails);  // ["info@example.com", "support@test.org"]
```

## 🛡️ Security Features

### Capability-Based Access Control

```bash
# Restrictive mode - only allow specific modules
darix run --allow=math,string script.dax

# Permissive mode with specific denials
darix run --allow=* --deny=os,http script.dax
```

### Filesystem Sandboxing

```bash
# Restrict filesystem access to specific directory
darix run --fs-root=/safe/directory script.dax

# Make filesystem read-only
darix run --fs-ro script.dax
```

### Global Injection Control

```bash
# Disable global injection (require explicit imports)
darix run --inject=false script.dax
```

## 🚀 Performance Features

- **Zero-Copy Operations**: String and binary operations avoid unnecessary copying
- **Native Go Performance**: All libraries implemented in optimized Go code
- **Memory Pooling**: Automatic object pooling for frequently used types
- **Concurrent Safe**: All libraries are thread-safe for concurrent access

## 📝 Error Handling

All native libraries provide comprehensive error handling:

```dax
import "go:math";

try {
    var result = math_sqrt(-1);  // Will throw error for negative input
} catch (e) {
    print("Math error:", e);
}

try {
    var invalid = json_parse("invalid json");
} catch (e) {
    print("JSON parsing error:", e);
}
```

## 🔗 Integration Examples

### Web API Client

```dax
import "go:http";
import "go:json";
import "go:crypto";

func fetch_user_data(user_id) {
    var url = "https://api.example.com/users/" + str(user_id);
    var response = http_get(url);
    
    if (response["status"] == 200) {
        var user = json_parse(response["body"]);
        return user;
    } else {
        throw "Failed to fetch user: " + str(response["status"]);
    }
}

func create_user(name, email) {
    var user_data = {
        "name": name,
        "email": email,
        "id": crypto_sha256(email + str(time_now()))
    };
    
    var json_data = json_stringify(user_data);
    var headers = {"Content-Type": "application/json"};
    
    var response = http_post("https://api.example.com/users", json_data, headers);
    return json_parse(response["body"]);
}
```

### Log Processing

```dax
import "go:regex";
import "go:string";
import "go:time";

func parse_log_entry(line) {
    var pattern = "(\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}) \\[(\\w+)\\] (.+)";
    var matches = regex_find_all(pattern, line);
    
    if (len(matches) >= 3) {
        return {
            "timestamp": matches[0],
            "level": str_upper(matches[1]),
            "message": str_trim(matches[2])
        };
    }
    return null;
}

func filter_logs_by_level(logs, level) {
    var filtered = [];
    for (var i = 0; i < len(logs); i = i + 1) {
        var entry = parse_log_entry(logs[i]);
        if (entry && entry["level"] == str_upper(level)) {
            filtered = filtered + [entry];
        }
    }
    return filtered;
}
```

## 📊 Performance Benchmarks

Native libraries provide significant performance improvements over pure DariX implementations:

- **Math operations**: 10-50x faster than interpreted math
- **JSON processing**: 5-20x faster than manual parsing
- **String operations**: 3-15x faster with Unicode support
- **Regex matching**: Native Go regex engine performance
- **HTTP requests**: Full HTTP/1.1 and HTTP/2 support

### 9. Path Library (`go:path`)

Cross-platform file path operations with proper separator handling.

```dax
import "go:path";

// Path joining and splitting
var full_path = path_join("home", "user", "documents", "file.txt");
var components = path_split("/home/user/file.txt");  // ["/home/user/", "file.txt"]

// Path components
var directory = path_dir("/home/user/file.txt");     // "/home/user"
var filename = path_base("/home/user/file.txt");     // "file.txt"
var extension = path_ext("document.pdf");            // ".pdf"

// Path utilities
var cleaned = path_clean("./home/../home/user/./file.txt");
var absolute = path_abs("relative/path");
var relative = path_rel("/home/user", "/home/user/documents/file.txt");

// Path validation and matching
var is_absolute = path_is_abs("/home/user");         // true
var matches = path_match("*.txt", "file.txt");       // true
var normalized = path_normalize("home\\user\\file.txt"); // Cross-platform normalization
```

### 10. Random Library (`go:random`)

Advanced random number generation with cryptographic options.

```dax
import "go:random";

// Basic random numbers
var rand_int = random_int(100);                      // 0-99
var rand_float = random_float();                     // 0.0-1.0
var rand_range = random_range(10, 20);               // 10-20

// Array operations
var choices = ["apple", "banana", "cherry"];
var choice = random_choice(choices);                 // Random element
var shuffled = random_shuffle([1, 2, 3, 4, 5]);     // Shuffled array
var sample = random_sample([1, 2, 3, 4, 5], 3);     // Random sample of 3

// String and byte generation
var rand_string = random_string(10);                 // 10-char random string
var custom_string = random_string(8, "ABCDEF123");   // Custom charset
var rand_bytes = random_bytes(16);                   // 16 random bytes
var uuid = random_uuid();                            // UUID v4

// Cryptographically secure
var crypto_int = random_crypto_int(1000);            // Secure random integer

// Reproducible randomness
random_seed(12345);                                  // Set seed for reproducible results
```

### 11. URL Library (`go:url`)

Complete URL parsing, building, and manipulation.

```dax
import "go:url";

// URL parsing
var url_string = "https://user:pass@example.com:8080/path?query=value#fragment";
var parsed = url_parse(url_string);
print(parsed["scheme"]);    // "https"
print(parsed["host"]);      // "example.com:8080"
print(parsed["path"]);      // "/path"
print(parsed["query"]);     // "query=value"

// URL building
var components = {
    "scheme": "https",
    "host": "api.example.com",
    "path": "/v1/users",
    "query": "limit=10"
};
var built_url = url_build(components);

// URL encoding/decoding
var encoded = url_encode("hello world!");            // "hello%20world%21"
var decoded = url_decode(encoded);                   // "hello world!"

// Query string operations
var params = {"name": "John", "age": "30", "city": "NYC"};
var query_string = url_query_encode(params);         // "name=John&age=30&city=NYC"
var decoded_params = url_query_decode(query_string);

// URL utilities
var joined = url_join("https://example.com/api", "users/123");
var resolved = url_resolve("https://example.com/api/", "../auth/login");
var is_valid = url_is_valid("https://example.com");  // true
```

### 12. Base Library (`go:base`)

Number base conversion utilities for binary, octal, hexadecimal, and custom bases.

```dax
import "go:base";

// Standard base conversions
var binary = base_to_binary(42);                     // "101010"
var octal = base_to_octal(64);                       // "100"
var hex = base_to_hex(255);                          // "ff"
var hex_upper = base_to_hex(255, true);              // "FF"

// Reverse conversions
var from_binary = base_from_binary("101010");        // 42
var from_octal = base_from_octal("100");             // 64
var from_hex = base_from_hex("FF");                  // 255
var from_hex_prefix = base_from_hex("0xFF");         // 255 (handles prefixes)

// Arbitrary base conversion
var result = base_convert("1010", 2, 10);            // "10" (binary to decimal)
var base36 = base_convert("255", 10, 36);            // "73" (decimal to base36)

// Custom base conversion with custom digits
var custom = base_to_base("123", 10, 2, "0123456789ABCDEF");

// Base validation
var valid = base_validate("1010", 2);                // true
var invalid = base_validate("1012", 2);              // false (invalid binary)
```

### 13. Collections Library (`go:collections`)

Functional programming utilities for advanced data structure operations.

```dax
import "go:collections";

// Functional operations (Note: requires function support)
var numbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];

// Filter, map, reduce operations
func is_even(x) { return x % 2 == 0; }
func square(x) { return x * x; }
func add(a, b) { return a + b; }

var evens = collections_filter(numbers, is_even);     // [2, 4, 6, 8, 10]
var squares = collections_map(numbers, square);       // [1, 4, 9, 16, 25, ...]
var sum = collections_reduce(numbers, add, 0);        // 55

// Find operations
var first_even = collections_find(numbers, is_even); // 2
var all_evens = collections_find_all(numbers, is_even);

// Array utilities
var unique = collections_unique([1, 2, 2, 3, 3, 4]); // [1, 2, 3, 4]
var flattened = collections_flatten([[1, 2], [3, 4], [5]]); // [1, 2, 3, 4, 5]
var chunks = collections_chunk([1, 2, 3, 4, 5], 2);  // [[1, 2], [3, 4], [5]]

// Array combinations
var arr1 = [1, 2, 3];
var arr2 = ["a", "b", "c"];
var zipped = collections_zip(arr1, arr2);             // [[1, "a"], [2, "b"], [3, "c"]]
var unzipped = collections_unzip(zipped);             // [[1, 2, 3], ["a", "b", "c"]]

// Set operations
var diff = collections_diff([1, 2, 3, 4], [3, 4, 5, 6]);      // [1, 2]
var intersection = collections_intersect([1, 2, 3], [2, 3, 4]); // [2, 3]
var union = collections_union([1, 2, 3], [3, 4, 5]);          // [1, 2, 3, 4, 5]

// Advanced operations
func get_length(x) { return len(x); }
var by_length = collections_sort_by(["cat", "elephant", "dog"], get_length);
var grouped = collections_group_by(["apple", "apricot", "banana"], get_first_char);
var partitioned = collections_partition(numbers, is_even); // [[2, 4, 6, 8, 10], [1, 3, 5, 7, 9]]
```

### 14. Enhanced HTTP Library (`go:http`)

Complete HTTP client with advanced features for web development.

```dax
import "go:http";

// Enhanced HTTP methods
var response = http_patch("https://api.example.com/users/1", 
    "{\"name\": \"Updated\"}", {"Content-Type": "application/json"});

var head = http_head("https://example.com");
var options = http_options("https://api.example.com");

// Custom HTTP request
var custom = http_request("PUT", "https://api.example.com/data", 
    "{\"data\": \"value\"}", {"Authorization": "Bearer token"});

// File operations
var download = http_download("https://example.com/file.zip", "./downloads/file.zip");
var upload = http_upload("https://upload.example.com", "file", "./document.pdf", 
    {"description": "Important document"});

// Configuration
http_set_timeout(30); // 30 seconds timeout

// Cookie management
var cookies = http_get_cookies("https://example.com");
http_set_cookies("https://example.com", [
    {"name": "session", "value": "abc123", "secure": true}
]);
```

### 15. Socket Library (`go:socket`)

Low-level TCP and UDP socket programming for network applications.

```dax
import "go:socket";

// TCP Server
var server = socket_tcp_listen(8080);
var server_id = server["socket_id"];

// Accept connections
var client = socket_tcp_accept(server_id);
var client_id = client["socket_id"];

// Send and receive data
socket_tcp_send(client_id, "Hello from server!");
var response = socket_tcp_receive(client_id, 1024);
print("Received:", response["data"]);

// TCP Client
var connection = socket_tcp_connect("localhost", 8080);
var conn_id = connection["socket_id"];

socket_tcp_send(conn_id, "Hello from client!");
var reply = socket_tcp_receive(conn_id);

// UDP Communication
var udp_server = socket_udp_listen(9090);
var udp_id = udp_server["socket_id"];

var message = socket_udp_receive(udp_id, 1024);
print("UDP message from:", message["from_addr"]);

// Send UDP response
socket_udp_send(udp_id, "UDP response", "127.0.0.1", 9091);

// Utilities
socket_set_timeout(conn_id, 10); // 10 second timeout
var local_addr = socket_get_local_addr(conn_id);
var remote_addr = socket_get_remote_addr(conn_id);

// Cleanup
socket_tcp_close(client_id);
socket_tcp_close(server_id);
socket_udp_close(udp_id);
```

### 16. WebSocket Library (`go:websocket`)

Real-time bidirectional communication with WebSocket support.

```dax
import "go:websocket";

// Connect to WebSocket server
var ws = ws_connect("wss://echo.websocket.org", 
    {"Origin": "https://example.com"});
var ws_id = ws["ws_id"];

// Send different message types
ws_send(ws_id, "Hello WebSocket!", "text");
ws_send(ws_id, "Binary data", "binary");

// Receive messages
var message = ws_receive(ws_id);
print("Received:", message["message"]);
print("Type:", message["type"]);

// Ping/Pong for keepalive
ws_ping(ws_id, "ping data");

// Check connection status
if (ws_is_connected(ws_id)) {
    print("WebSocket is connected");
}

// Set timeout
ws_set_timeout(ws_id, 30);

// Close connection
ws_close(ws_id, "Goodbye");
```

### 17. DNS Library (`go:dns`)

Domain Name System operations and validation.

```dax
import "go:dns";

// Basic DNS lookup
var result = dns_lookup("google.com");
print("IPs:", result["ips"]);

// Reverse DNS lookup
var reverse = dns_reverse("8.8.8.8");
print("Hostnames:", reverse["names"]);

// MX records for email
var mx = dns_lookup_mx("gmail.com");
for (var record in mx["mx_records"]) {
    print("Mail server:", record["host"], "Priority:", record["priority"]);
}

// TXT records
var txt = dns_lookup_txt("google.com");
print("TXT records:", txt["txt_records"]);

// CNAME records
var cname = dns_lookup_cname("www.github.com");
print("CNAME:", cname["cname"]);

// Name servers
var ns = dns_lookup_ns("google.com");
print("Name servers:", ns["ns_records"]);

// Validation
var ip_check = dns_is_valid_ip("192.168.1.1");
if (ip_check["valid"]) {
    print("Valid", ip_check["version"], "address");
}

var domain_check = dns_is_valid_domain("example.com");
if (domain_check["valid"]) {
    print("Valid domain with", domain_check["labels"], "labels");
    print("TLD:", domain_check["tld"]);
}
```

### 18. SMTP Library (`go:smtp`)

Email sending capabilities with authentication support.

```dax
import "go:smtp";

// Simple email (no authentication)
var result = smtp_send_email("smtp.example.com", 25, 
    "sender@example.com", "recipient@example.com", 
    "Test Subject", "This is a test email.");

// HTML email
var html_result = smtp_send_html("smtp.gmail.com", 587,
    "sender@gmail.com", "recipient@example.com",
    "HTML Email", "<h1>Hello</h1><p>This is <b>HTML</b> email!</p>");

// Email with authentication
var auth_result = smtp_send_with_auth("smtp.gmail.com", 587,
    "username@gmail.com", "app_password",
    "sender@gmail.com", "recipient@example.com,another@example.com",
    "Authenticated Email", "This email was sent with authentication.");

// Test SMTP connection
var test = smtp_test_connection("smtp.gmail.com", 587, 
    "username@gmail.com", "password", true); // TLS enabled

if (test["connected"]) {
    print("SMTP connection successful");
    if (test["auth_tested"] && test["auth_success"]) {
        print("Authentication successful");
    }
}
```

### 19. HTTP Server Library (`go:httpserver`)

Complete HTTP server implementation for building web applications and APIs.

```dax
import "go:httpserver";

// Create HTTP server
var server = server_create(8080);
var server_id = server["server_id"];

// Define route handlers
func home_handler(request, response) {
    return response_html(response, "<h1>Welcome to DariX Web Server!</h1>");
}

func api_handler(request, response) {
    var data = {"message": "Hello from API", "timestamp": time_now()};
    return response_json(response, data);
}

func user_handler(request, response) {
    var user_id = request_get_param(request, "id");
    var user_data = {"id": user_id, "name": "User " + user_id};
    return response_json(response, user_data);
}

// Add routes
server_route(server_id, "GET", "/", home_handler);
server_route(server_id, "GET", "/api/hello", api_handler);
server_route(server_id, "GET", "/api/user", user_handler);
server_route(server_id, "POST", "/api/data", api_handler);

// Serve static files
server_static(server_id, "/static/", "./public/");
server_static(server_id, "/assets/", "./assets/");

// Add middleware
func logging_middleware(request, response, next) {
    var method = request["method"];
    var path = request["path"];
    print("Request: " + method + " " + path);
    return next();
}

func auth_middleware(request, response, next) {
    var auth_header = request_get_header(request, "Authorization");
    if (auth_header == "") {
        return response_status(response, 401);
    }
    return next();
}

server_middleware(server_id, logging_middleware);
server_middleware(server_id, auth_middleware);

// Configure server
server_set_timeout(server_id, 30); // 30 seconds timeout

// Start server
var result = server_start(server_id);
print("🚀 Server running on:", result["address"]);
print("📡 Server ID:", result["server_id"]);

// Server will run until stopped
// server_stop(server_id);
```

#### Response Functions:
```dax
// JSON response
func json_handler(request, response) {
    var data = {"status": "success", "data": [1, 2, 3]};
    return response_json(response, data);
}

// HTML response
func html_handler(request, response) {
    var html = "<html><body><h1>DariX Server</h1></body></html>";
    return response_html(response, html);
}

// Text response
func text_handler(request, response) {
    return response_text(response, "Plain text response");
}

// File response
func file_handler(request, response) {
    return response_file(response, "./files/document.pdf");
}

// Redirect response
func redirect_handler(request, response) {
    return response_redirect(response, "https://example.com");
}

// Custom status
func not_found_handler(request, response) {
    response_status(response, 404);
    return response_text(response, "Page not found");
}
```

#### Request Processing:
```dax
func process_request(request, response) {
    // Get query parameters
    var name = request_get_param(request, "name");
    var age = request_get_param(request, "age");
    
    // Get headers
    var user_agent = request_get_header(request, "User-Agent");
    var content_type = request_get_header(request, "Content-Type");
    
    // Get request body (for POST/PUT requests)
    var body = request_get_body(request);
    
    // Process and respond
    var response_data = {
        "name": name,
        "age": age,
        "user_agent": user_agent,
        "body_length": len(body)
    };
    
    return response_json(response, response_data);
}
```

The native library system makes DariX suitable for production applications requiring high performance and comprehensive functionality.
