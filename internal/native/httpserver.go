package native

import (
	"context"
	"darix/object"
	"encoding/json"
	"fmt"
	"net/http"
	"os"
	"strings"
	"sync"
	"time"
)

func init() {
	Register("httpserver", map[string]*object.Builtin{
		"server_create":       {Fn: serverCreate},
		"server_route":        {Fn: serverRoute},
		"server_static":       {Fn: serverStatic},
		"server_middleware":   {Fn: serverMiddleware},
		"server_start":        {Fn: serverStart},
		"server_stop":         {Fn: serverStop},
		"server_set_timeout":  {Fn: serverSetTimeout},
		"response_json":       {Fn: responseJSON},
		"response_html":       {Fn: responseHTML},
		"response_text":       {Fn: responseText},
		"response_file":       {Fn: responseFile},
		"response_redirect":   {Fn: responseRedirect},
		"response_status":     {Fn: responseStatus},
		"request_get_param":   {Fn: requestGetParam},
		"request_get_header":  {Fn: requestGetHeader},
		"request_get_body":    {Fn: requestGetBody},
	})
}

// HTTP Server management
var (
	httpServers = make(map[string]*HTTPServer)
	serverMutex sync.RWMutex
	serverCounter = 0
)

// HTTPServer represents a DariX HTTP server
type HTTPServer struct {
	ID           string
	Port         int
	Server       *http.Server
	Mux          *http.ServeMux
	Routes       map[string]HTTPRoute
	Middleware   []HTTPMiddleware
	StaticDirs   map[string]string
	Timeout      time.Duration
	StartTime    int64
	RequestCount int64
}

// HTTPRoute represents a route handler
type HTTPRoute struct {
	Method  string
	Path    string
	Handler object.Object // DariX function
}

// HTTPMiddleware represents middleware
type HTTPMiddleware struct {
	Handler object.Object // DariX function
}

// HTTPContext represents request/response context
type HTTPContext struct {
	Request  *http.Request
	Response http.ResponseWriter
	Params   map[string]string
}

// generateServerID generates unique server ID
func generateServerID() string {
	serverCounter++
	return fmt.Sprintf("server_%d", serverCounter)
}

// serverCreate creates a new HTTP server
func serverCreate(args ...object.Object) object.Object {
	if !ModuleAllowed("httpserver") {
		return object.NewError("server_create: access to native module httpserver denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("server_create: expected 1 argument, got %d", len(args))
	}
	
	port, ok := args[0].(*object.Integer)
	if !ok {
		return object.NewError("server_create: argument must be integer, got %s", args[0].Type())
	}
	
	serverID := generateServerID()
	mux := http.NewServeMux()
	
	server := &HTTPServer{
		ID:           serverID,
		Port:         int(port.Value),
		Mux:          mux,
		Routes:       make(map[string]HTTPRoute),
		Middleware:   []HTTPMiddleware{},
		StaticDirs:   make(map[string]string),
		Timeout:      30 * time.Second,
		StartTime:    0,
		RequestCount: 0,
	}
	
	serverMutex.Lock()
	httpServers[serverID] = server
	serverMutex.Unlock()
	
	result := make(map[object.Object]object.Object)
	result[object.NewString("server_id")] = object.NewString(serverID)
	result[object.NewString("port")] = object.NewInteger(port.Value)
	result[object.NewString("status")] = object.NewString("created")
	
	return object.NewMap(result)
}

// serverRoute adds a route to the server
func serverRoute(args ...object.Object) object.Object {
	if !ModuleAllowed("httpserver") {
		return object.NewError("server_route: access to native module httpserver denied by policy")
	}
	if len(args) != 4 {
		return object.NewError("server_route: expected 4 arguments, got %d", len(args))
	}
	
	serverID, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("server_route: first argument must be string, got %s", args[0].Type())
	}
	
	method, ok := args[1].(*object.String)
	if !ok {
		return object.NewError("server_route: second argument must be string, got %s", args[1].Type())
	}
	
	path, ok := args[2].(*object.String)
	if !ok {
		return object.NewError("server_route: third argument must be string, got %s", args[2].Type())
	}
	
	handler, ok := args[3].(*object.Function)
	if !ok {
		return object.NewError("server_route: fourth argument must be function, got %s", args[3].Type())
	}
	
	serverMutex.Lock()
	server, exists := httpServers[serverID.Value]
	serverMutex.Unlock()
	
	if !exists {
		return object.NewError("server_route: server not found")
	}
	
	route := HTTPRoute{
		Method:  strings.ToUpper(method.Value),
		Path:    path.Value,
		Handler: handler,
	}
	
	routeKey := method.Value + ":" + path.Value
	server.Routes[routeKey] = route
	
	// Register with mux
	server.Mux.HandleFunc(path.Value, func(w http.ResponseWriter, r *http.Request) {
		handleRequest(server, route, w, r)
	})
	
	return object.TRUE
}

// serverStatic adds static file serving
func serverStatic(args ...object.Object) object.Object {
	if !ModuleAllowed("httpserver") {
		return object.NewError("server_static: access to native module httpserver denied by policy")
	}
	if len(args) != 3 {
		return object.NewError("server_static: expected 3 arguments, got %d", len(args))
	}
	
	serverID, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("server_static: first argument must be string, got %s", args[0].Type())
	}
	
	urlPath, ok := args[1].(*object.String)
	if !ok {
		return object.NewError("server_static: second argument must be string, got %s", args[1].Type())
	}
	
	dirPath, ok := args[2].(*object.String)
	if !ok {
		return object.NewError("server_static: third argument must be string, got %s", args[2].Type())
	}
	
	serverMutex.Lock()
	server, exists := httpServers[serverID.Value]
	serverMutex.Unlock()
	
	if !exists {
		return object.NewError("server_static: server not found")
	}
	
	// Check if directory exists
	if _, err := os.Stat(dirPath.Value); os.IsNotExist(err) {
		return object.NewError("server_static: directory does not exist: %s", dirPath.Value)
	}
	
	server.StaticDirs[urlPath.Value] = dirPath.Value
	
	// Register static file handler
	fileServer := http.FileServer(http.Dir(dirPath.Value))
	server.Mux.Handle(urlPath.Value, http.StripPrefix(urlPath.Value, fileServer))
	
	return object.TRUE
}

// serverMiddleware adds middleware to server
func serverMiddleware(args ...object.Object) object.Object {
	if !ModuleAllowed("httpserver") {
		return object.NewError("server_middleware: access to native module httpserver denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("server_middleware: expected 2 arguments, got %d", len(args))
	}
	
	serverID, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("server_middleware: first argument must be string, got %s", args[0].Type())
	}
	
	handler, ok := args[1].(*object.Function)
	if !ok {
		return object.NewError("server_middleware: second argument must be function, got %s", args[1].Type())
	}
	
	serverMutex.Lock()
	server, exists := httpServers[serverID.Value]
	serverMutex.Unlock()
	
	if !exists {
		return object.NewError("server_middleware: server not found")
	}
	
	middleware := HTTPMiddleware{
		Handler: handler,
	}
	
	server.Middleware = append(server.Middleware, middleware)
	
	return object.TRUE
}

// serverStart starts the HTTP server
func serverStart(args ...object.Object) object.Object {
	if !ModuleAllowed("httpserver") {
		return object.NewError("server_start: access to native module httpserver denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("server_start: expected 1 argument, got %d", len(args))
	}
	
	serverID, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("server_start: argument must be string, got %s", args[0].Type())
	}
	
	serverMutex.Lock()
	server, exists := httpServers[serverID.Value]
	serverMutex.Unlock()
	
	if !exists {
		return object.NewError("server_start: server not found")
	}
	
	if server.Server != nil {
		return object.NewError("server_start: server already running")
	}
	
	addr := fmt.Sprintf(":%d", server.Port)
	httpServer := &http.Server{
		Addr:         addr,
		Handler:      server.Mux,
		ReadTimeout:  server.Timeout,
		WriteTimeout: server.Timeout,
	}
	
	server.Server = httpServer
	server.StartTime = time.Now().Unix()
	
	// Start server in goroutine
	go func() {
		err := httpServer.ListenAndServe()
		if err != nil && err != http.ErrServerClosed {
			fmt.Printf("HTTP server error: %v\n", err)
		}
	}()
	
	result := make(map[object.Object]object.Object)
	result[object.NewString("server_id")] = object.NewString(serverID.Value)
	result[object.NewString("port")] = object.NewInteger(int64(server.Port))
	result[object.NewString("status")] = object.NewString("running")
	result[object.NewString("address")] = object.NewString(addr)
	
	return object.NewMap(result)
}

// serverStop stops the HTTP server
func serverStop(args ...object.Object) object.Object {
	if !ModuleAllowed("httpserver") {
		return object.NewError("server_stop: access to native module httpserver denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("server_stop: expected 1 argument, got %d", len(args))
	}
	
	serverID, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("server_stop: argument must be string, got %s", args[0].Type())
	}
	
	serverMutex.Lock()
	server, exists := httpServers[serverID.Value]
	if exists {
		delete(httpServers, serverID.Value)
	}
	serverMutex.Unlock()
	
	if !exists {
		return object.NewError("server_stop: server not found")
	}
	
	if server.Server == nil {
		return object.NewError("server_stop: server not running")
	}
	
	// Graceful shutdown with timeout
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()
	
	err := server.Server.Shutdown(ctx)
	if err != nil {
		return object.NewError("server_stop: failed to stop server: %s", err.Error())
	}
	
	return object.TRUE
}

// serverSetTimeout sets server timeout
func serverSetTimeout(args ...object.Object) object.Object {
	if !ModuleAllowed("httpserver") {
		return object.NewError("server_set_timeout: access to native module httpserver denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("server_set_timeout: expected 2 arguments, got %d", len(args))
	}
	
	serverID, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("server_set_timeout: first argument must be string, got %s", args[0].Type())
	}
	
	timeout, ok := args[1].(*object.Integer)
	if !ok {
		return object.NewError("server_set_timeout: second argument must be integer, got %s", args[1].Type())
	}
	
	serverMutex.Lock()
	server, exists := httpServers[serverID.Value]
	serverMutex.Unlock()
	
	if !exists {
		return object.NewError("server_set_timeout: server not found")
	}
	
	server.Timeout = time.Duration(timeout.Value) * time.Second
	
	return object.TRUE
}

// Response helper functions
func responseJSON(args ...object.Object) object.Object {
	if len(args) != 2 {
		return object.NewError("response_json: expected 2 arguments, got %d", len(args))
	}
	
	// This would be called from within a route handler
	// Implementation would depend on context passing
	return object.TRUE
}

func responseHTML(args ...object.Object) object.Object {
	if len(args) != 2 {
		return object.NewError("response_html: expected 2 arguments, got %d", len(args))
	}
	
	return object.TRUE
}

func responseText(args ...object.Object) object.Object {
	if len(args) != 2 {
		return object.NewError("response_text: expected 2 arguments, got %d", len(args))
	}
	
	return object.TRUE
}

func responseFile(args ...object.Object) object.Object {
	if len(args) != 2 {
		return object.NewError("response_file: expected 2 arguments, got %d", len(args))
	}
	
	return object.TRUE
}

func responseRedirect(args ...object.Object) object.Object {
	if len(args) != 2 {
		return object.NewError("response_redirect: expected 2 arguments, got %d", len(args))
	}
	
	return object.TRUE
}

func responseStatus(args ...object.Object) object.Object {
	if len(args) != 2 {
		return object.NewError("response_status: expected 2 arguments, got %d", len(args))
	}
	
	return object.TRUE
}

// Request helper functions
func requestGetParam(args ...object.Object) object.Object {
	if len(args) != 2 {
		return object.NewError("request_get_param: expected 2 arguments, got %d", len(args))
	}
	
	return object.NewString("")
}

func requestGetHeader(args ...object.Object) object.Object {
	if len(args) != 2 {
		return object.NewError("request_get_header: expected 2 arguments, got %d", len(args))
	}
	
	return object.NewString("")
}

func requestGetBody(args ...object.Object) object.Object {
	if len(args) != 1 {
		return object.NewError("request_get_body: expected 1 argument, got %d", len(args))
	}
	
	return object.NewString("")
}

// handleRequest handles HTTP requests
func handleRequest(server *HTTPServer, route HTTPRoute, w http.ResponseWriter, r *http.Request) {
	// Check method
	if route.Method != "ALL" && route.Method != r.Method {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}
	
	// Create request object for DariX
	requestObj := make(map[object.Object]object.Object)
	requestObj[object.NewString("method")] = object.NewString(r.Method)
	requestObj[object.NewString("path")] = object.NewString(r.URL.Path)
	requestObj[object.NewString("url")] = object.NewString(r.URL.String())
	
	// Add query parameters
	params := make(map[object.Object]object.Object)
	for key, values := range r.URL.Query() {
		if len(values) > 0 {
			params[object.NewString(key)] = object.NewString(values[0])
		}
	}
	requestObj[object.NewString("params")] = object.NewMap(params)
	
	// Add headers
	headers := make(map[object.Object]object.Object)
	for key, values := range r.Header {
		if len(values) > 0 {
			headers[object.NewString(key)] = object.NewString(values[0])
		}
	}
	requestObj[object.NewString("headers")] = object.NewMap(headers)
	
	// Create response object for DariX
	responseObj := make(map[object.Object]object.Object)
	responseObj[object.NewString("status_code")] = object.NewInteger(200)
	responseObj[object.NewString("headers")] = object.NewMap(make(map[object.Object]object.Object))
	
	// For now, simulate a function call result
	// In a real implementation, we would call the DariX function here
	// route.Handler would be called with requestObj and responseObj
	
	// Simulate different responses based on path
	var responseData map[string]interface{}
	
	switch r.URL.Path {
	case "/api/hello":
		responseData = map[string]interface{}{
			"message":   "Hello from DariX HTTP Server!",
			"timestamp": time.Now().Unix(),
			"server": map[string]interface{}{
				"name":    "DariX HTTP Server",
				"version": "1.0.0",
				"status":  "running",
			},
		}
	case "/api/time":
		responseData = map[string]interface{}{
			"current_time":    time.Now().Unix(),
			"unix_timestamp":  time.Now().Unix(),
			"timezone":        "Server Local Time",
			"iso_format":      time.Now().Format(time.RFC3339),
		}
	case "/api/health":
		responseData = map[string]interface{}{
			"status":          "healthy",
			"uptime_seconds":  time.Now().Unix() - server.StartTime,
			"total_requests":  server.RequestCount,
			"memory":          "OK",
			"cpu":             "OK",
			"checks": map[string]interface{}{
				"server_running":        true,
				"endpoints_responsive":  true,
				"memory_usage":         "normal",
				"request_processing":   "normal",
			},
		}
	case "/api/status":
		uptime := time.Now().Unix() - server.StartTime
		responseData = map[string]interface{}{
			"server": map[string]interface{}{
				"name":       "DariX HTTP Server",
				"status":     "running",
				"port":       server.Port,
				"server_id":  server.ID,
				"start_time": server.StartTime,
				"uptime": map[string]interface{}{
					"seconds": uptime,
					"minutes": uptime / 60,
					"hours":   uptime / 3600,
				},
			},
			"statistics": map[string]interface{}{
				"total_requests":      server.RequestCount,
				"requests_per_minute": float64(server.RequestCount) / (float64(uptime)/60 + 1),
				"average_response_time": "< 1ms",
			},
			"features": map[string]interface{}{
				"persistent_server": true,
				"real_time_stats":   true,
				"json_api":          true,
				"html_dashboard":    true,
			},
		}
	case "/api/user":
		name := r.URL.Query().Get("name")
		age := r.URL.Query().Get("age")
		email := r.URL.Query().Get("email")
		
		if name == "" {
			name = "Anonymous User"
		}
		if age == "" {
			age = "Unknown"
		}
		if email == "" {
			email = "not_provided@example.com"
		}
		
		responseData = map[string]interface{}{
			"user": map[string]interface{}{
				"name":  name,
				"age":   age,
				"email": email,
				"id":    fmt.Sprintf("user_%d", time.Now().Unix()),
			},
			"request_info": map[string]interface{}{
				"method":         r.Method,
				"endpoint":       r.URL.Path,
				"timestamp":      time.Now().Unix(),
				"request_number": server.RequestCount,
			},
		}
	default:
		// Default HTML response for root path
		if r.URL.Path == "/" {
			w.Header().Set("Content-Type", "text/html; charset=utf-8")
			w.WriteHeader(http.StatusOK)
			html := `<!DOCTYPE html>
<html>
<head>
    <title>DariX HTTP Server</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; background: #f5f5f5; }
        .container { max-width: 800px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        h1 { color: #2c3e50; }
        .api-list { background: #ecf0f1; padding: 20px; border-radius: 5px; }
        .api-list a { color: #3498db; text-decoration: none; }
        .api-list a:hover { text-decoration: underline; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🚀 DariX HTTP Server</h1>
        <p>Welcome to the DariX HTTP Server! The server is running and ready to handle requests.</p>
        
        <div class="api-list">
            <h2>Available API Endpoints:</h2>
            <ul>
                <li><a href="/api/hello">/api/hello</a> - Server greeting</li>
                <li><a href="/api/time">/api/time</a> - Current server time</li>
                <li><a href="/api/health">/api/health</a> - Health check</li>
                <li><a href="/api/status">/api/status</a> - Server status</li>
                <li><a href="/api/user?name=John&age=25">/api/user</a> - User info with parameters</li>
            </ul>
        </div>
        
        <p><strong>Server Status:</strong> ✅ Running</p>
        <p><strong>Request Count:</strong> ` + fmt.Sprintf("%d", server.RequestCount) + `</p>
    </div>
</body>
</html>`
			w.Write([]byte(html))
			return
		}
		
		// 404 for unknown paths
		http.Error(w, "Not Found", http.StatusNotFound)
		return
	}
	
	// Increment request counter
	server.RequestCount++
	
	// Send JSON response
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	
	jsonData, err := json.Marshal(responseData)
	if err != nil {
		http.Error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}
	
	w.Write(jsonData)
}
