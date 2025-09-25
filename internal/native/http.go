package native

import (
	"bytes"
	"darix/object"
	"io"
	"mime/multipart"
	"net/http"
	"net/http/cookiejar"
	"net/url"
	"os"
	"path/filepath"
	"strings"
	"time"
)

func init() {
	Register("http", map[string]*object.Builtin{
		"http_get":         {Fn: httpGet},
		"http_post":        {Fn: httpPost},
		"http_put":         {Fn: httpPut},
		"http_delete":      {Fn: httpDelete},
		"http_patch":       {Fn: httpPatch},
		"http_head":        {Fn: httpHead},
		"http_options":     {Fn: httpOptions},
		"http_request":     {Fn: httpRequest},
		"http_download":    {Fn: httpDownload},
		"http_upload":      {Fn: httpUpload},
		"http_set_timeout": {Fn: httpSetTimeout},
		"http_get_cookies": {Fn: httpGetCookies},
		"http_set_cookies": {Fn: httpSetCookies},
	})
}

// httpGet performs HTTP GET request
func httpGet(args ...object.Object) object.Object {
	if !ModuleAllowed("http") {
		return object.NewError("http_get: access to native module http denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("http_get: expected 1 argument, got %d", len(args))
	}
	
	url, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("http_get: argument must be string, got %s", args[0].Type())
	}
	
	return performHTTPRequest("GET", url.Value, nil, nil)
}

// httpPost performs HTTP POST request
func httpPost(args ...object.Object) object.Object {
	if !ModuleAllowed("http") {
		return object.NewError("http_post: access to native module http denied by policy")
	}
	if len(args) < 2 || len(args) > 3 {
		return object.NewError("http_post: expected 2-3 arguments, got %d", len(args))
	}
	
	url, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("http_post: first argument must be string, got %s", args[0].Type())
	}
	
	body, ok := args[1].(*object.String)
	if !ok {
		return object.NewError("http_post: second argument must be string, got %s", args[1].Type())
	}
	
	var headers map[string]string
	if len(args) == 3 {
		if headerMap, ok := args[2].(*object.Map); ok {
			headers = make(map[string]string)
			for k, v := range headerMap.Pairs {
				if keyStr, ok := k.(*object.String); ok {
					if valStr, ok := v.(*object.String); ok {
						headers[keyStr.Value] = valStr.Value
					}
				}
			}
		}
	}
	
	return performHTTPRequest("POST", url.Value, []byte(body.Value), headers)
}

// httpPut performs HTTP PUT request
func httpPut(args ...object.Object) object.Object {
	if !ModuleAllowed("http") {
		return object.NewError("http_put: access to native module http denied by policy")
	}
	if len(args) < 2 || len(args) > 3 {
		return object.NewError("http_put: expected 2-3 arguments, got %d", len(args))
	}
	
	url, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("http_put: first argument must be string, got %s", args[0].Type())
	}
	
	body, ok := args[1].(*object.String)
	if !ok {
		return object.NewError("http_put: second argument must be string, got %s", args[1].Type())
	}
	
	var headers map[string]string
	if len(args) == 3 {
		if headerMap, ok := args[2].(*object.Map); ok {
			headers = make(map[string]string)
			for k, v := range headerMap.Pairs {
				if keyStr, ok := k.(*object.String); ok {
					if valStr, ok := v.(*object.String); ok {
						headers[keyStr.Value] = valStr.Value
					}
				}
			}
		}
	}
	
	return performHTTPRequest("PUT", url.Value, []byte(body.Value), headers)
}

// httpDelete performs HTTP DELETE request
func httpDelete(args ...object.Object) object.Object {
	if !ModuleAllowed("http") {
		return object.NewError("http_delete: access to native module http denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("http_delete: expected 1 argument, got %d", len(args))
	}
	
	url, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("http_delete: argument must be string, got %s", args[0].Type())
	}
	
	return performHTTPRequest("DELETE", url.Value, nil, nil)
}

// performHTTPRequest is a helper function to perform HTTP requests
func performHTTPRequest(method, url string, body []byte, headers map[string]string) object.Object {
	client := &http.Client{
		Timeout: 30 * time.Second,
	}
	
	var bodyReader io.Reader
	if body != nil {
		bodyReader = bytes.NewReader(body)
	}
	
	req, err := http.NewRequest(method, url, bodyReader)
	if err != nil {
		return object.NewError("http request creation failed: %s", err.Error())
	}
	
	// Set headers
	if headers != nil {
		for key, value := range headers {
			req.Header.Set(key, value)
		}
	}
	
	// Set default Content-Type for POST/PUT if not specified
	if (method == "POST" || method == "PUT") && body != nil {
		if req.Header.Get("Content-Type") == "" {
			req.Header.Set("Content-Type", "application/json")
		}
	}
	
	resp, err := client.Do(req)
	if err != nil {
		return object.NewError("http request failed: %s", err.Error())
	}
	defer resp.Body.Close()
	
	responseBody, err := io.ReadAll(resp.Body)
	if err != nil {
		return object.NewError("failed to read response body: %s", err.Error())
	}
	
	// Create response object
	responseHeaders := make(map[object.Object]object.Object)
	for key, values := range resp.Header {
		if len(values) > 0 {
			responseHeaders[object.NewString(key)] = object.NewString(values[0])
		}
	}
	
	response := make(map[object.Object]object.Object)
	response[object.NewString("status")] = object.NewInteger(int64(resp.StatusCode))
	response[object.NewString("body")] = object.NewString(string(responseBody))
	response[object.NewString("headers")] = object.NewMap(responseHeaders)
	
	return object.NewMap(response)
}

// Global HTTP client configuration
var (
	httpTimeout    = 30 * time.Second
	httpCookieJar  *cookiejar.Jar
)

func init() {
	// Initialize cookie jar
	jar, _ := cookiejar.New(nil)
	httpCookieJar = jar
}

// httpPatch performs HTTP PATCH request
func httpPatch(args ...object.Object) object.Object {
	if !ModuleAllowed("http") {
		return object.NewError("http_patch: access to native module http denied by policy")
	}
	if len(args) < 2 || len(args) > 3 {
		return object.NewError("http_patch: expected 2-3 arguments, got %d", len(args))
	}
	
	url, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("http_patch: first argument must be string, got %s", args[0].Type())
	}
	
	data, ok := args[1].(*object.String)
	if !ok {
		return object.NewError("http_patch: second argument must be string, got %s", args[1].Type())
	}
	
	var headers map[string]string
	if len(args) == 3 {
		if headersObj, ok := args[2].(*object.Map); ok {
			headers = make(map[string]string)
			for key, value := range headersObj.Pairs {
				if keyStr, ok := key.(*object.String); ok {
					if valueStr, ok := value.(*object.String); ok {
						headers[keyStr.Value] = valueStr.Value
					}
				}
			}
		}
	}
	
	return performHTTPRequest("PATCH", url.Value, []byte(data.Value), headers)
}

// httpHead performs HTTP HEAD request
func httpHead(args ...object.Object) object.Object {
	if !ModuleAllowed("http") {
		return object.NewError("http_head: access to native module http denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("http_head: expected 1 argument, got %d", len(args))
	}
	
	url, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("http_head: argument must be string, got %s", args[0].Type())
	}
	
	return performHTTPRequest("HEAD", url.Value, nil, nil)
}

// httpOptions performs HTTP OPTIONS request
func httpOptions(args ...object.Object) object.Object {
	if !ModuleAllowed("http") {
		return object.NewError("http_options: access to native module http denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("http_options: expected 1 argument, got %d", len(args))
	}
	
	url, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("http_options: argument must be string, got %s", args[0].Type())
	}
	
	return performHTTPRequest("OPTIONS", url.Value, nil, nil)
}

// httpRequest performs custom HTTP request with full control
func httpRequest(args ...object.Object) object.Object {
	if !ModuleAllowed("http") {
		return object.NewError("http_request: access to native module http denied by policy")
	}
	if len(args) < 2 || len(args) > 4 {
		return object.NewError("http_request: expected 2-4 arguments, got %d", len(args))
	}
	
	method, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("http_request: first argument must be string, got %s", args[0].Type())
	}
	
	url, ok := args[1].(*object.String)
	if !ok {
		return object.NewError("http_request: second argument must be string, got %s", args[1].Type())
	}
	
	var body []byte
	if len(args) >= 3 && args[2] != object.NULL {
		if bodyStr, ok := args[2].(*object.String); ok {
			body = []byte(bodyStr.Value)
		}
	}
	
	var headers map[string]string
	if len(args) == 4 {
		if headersObj, ok := args[3].(*object.Map); ok {
			headers = make(map[string]string)
			for key, value := range headersObj.Pairs {
				if keyStr, ok := key.(*object.String); ok {
					if valueStr, ok := value.(*object.String); ok {
						headers[keyStr.Value] = valueStr.Value
					}
				}
			}
		}
	}
	
	return performHTTPRequest(strings.ToUpper(method.Value), url.Value, body, headers)
}

// httpDownload downloads a file from URL
func httpDownload(args ...object.Object) object.Object {
	if !ModuleAllowed("http") {
		return object.NewError("http_download: access to native module http denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("http_download: expected 2 arguments, got %d", len(args))
	}
	
	url, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("http_download: first argument must be string, got %s", args[0].Type())
	}
	
	filename, ok := args[1].(*object.String)
	if !ok {
		return object.NewError("http_download: second argument must be string, got %s", args[1].Type())
	}
	
	client := &http.Client{
		Timeout:   httpTimeout,
		Jar:       httpCookieJar,
	}
	
	resp, err := client.Get(url.Value)
	if err != nil {
		return object.NewError("http_download: failed to download: %s", err.Error())
	}
	defer resp.Body.Close()
	
	if resp.StatusCode != http.StatusOK {
		return object.NewError("http_download: server returned status %d", resp.StatusCode)
	}
	
	// Create directory if it doesn't exist
	dir := filepath.Dir(filename.Value)
	if err := os.MkdirAll(dir, 0755); err != nil {
		return object.NewError("http_download: failed to create directory: %s", err.Error())
	}
	
	// Create the file
	file, err := os.Create(filename.Value)
	if err != nil {
		return object.NewError("http_download: failed to create file: %s", err.Error())
	}
	defer file.Close()
	
	// Copy the response body to file
	size, err := io.Copy(file, resp.Body)
	if err != nil {
		return object.NewError("http_download: failed to write file: %s", err.Error())
	}
	
	result := make(map[object.Object]object.Object)
	result[object.NewString("filename")] = object.NewString(filename.Value)
	result[object.NewString("size")] = object.NewInteger(size)
	result[object.NewString("status")] = object.NewInteger(int64(resp.StatusCode))
	
	return object.NewMap(result)
}

// httpUpload uploads a file via HTTP POST with multipart form
func httpUpload(args ...object.Object) object.Object {
	if !ModuleAllowed("http") {
		return object.NewError("http_upload: access to native module http denied by policy")
	}
	if len(args) < 3 || len(args) > 4 {
		return object.NewError("http_upload: expected 3-4 arguments, got %d", len(args))
	}
	
	url, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("http_upload: first argument must be string, got %s", args[0].Type())
	}
	
	fieldName, ok := args[1].(*object.String)
	if !ok {
		return object.NewError("http_upload: second argument must be string, got %s", args[1].Type())
	}
	
	filename, ok := args[2].(*object.String)
	if !ok {
		return object.NewError("http_upload: third argument must be string, got %s", args[2].Type())
	}
	
	// Open the file
	file, err := os.Open(filename.Value)
	if err != nil {
		return object.NewError("http_upload: failed to open file: %s", err.Error())
	}
	defer file.Close()
	
	// Create multipart form
	var body bytes.Buffer
	writer := multipart.NewWriter(&body)
	
	// Add file field
	part, err := writer.CreateFormFile(fieldName.Value, filepath.Base(filename.Value))
	if err != nil {
		return object.NewError("http_upload: failed to create form file: %s", err.Error())
	}
	
	_, err = io.Copy(part, file)
	if err != nil {
		return object.NewError("http_upload: failed to copy file: %s", err.Error())
	}
	
	// Add additional form fields if provided
	if len(args) == 4 {
		if fieldsObj, ok := args[3].(*object.Map); ok {
			for key, value := range fieldsObj.Pairs {
				if keyStr, ok := key.(*object.String); ok {
					if valueStr, ok := value.(*object.String); ok {
						writer.WriteField(keyStr.Value, valueStr.Value)
					}
				}
			}
		}
	}
	
	err = writer.Close()
	if err != nil {
		return object.NewError("http_upload: failed to close writer: %s", err.Error())
	}
	
	// Create request
	client := &http.Client{
		Timeout: httpTimeout,
		Jar:     httpCookieJar,
	}
	
	req, err := http.NewRequest("POST", url.Value, &body)
	if err != nil {
		return object.NewError("http_upload: failed to create request: %s", err.Error())
	}
	
	req.Header.Set("Content-Type", writer.FormDataContentType())
	
	resp, err := client.Do(req)
	if err != nil {
		return object.NewError("http_upload: request failed: %s", err.Error())
	}
	defer resp.Body.Close()
	
	responseBody, err := io.ReadAll(resp.Body)
	if err != nil {
		return object.NewError("http_upload: failed to read response: %s", err.Error())
	}
	
	result := make(map[object.Object]object.Object)
	result[object.NewString("status")] = object.NewInteger(int64(resp.StatusCode))
	result[object.NewString("body")] = object.NewString(string(responseBody))
	
	return object.NewMap(result)
}

// httpSetTimeout sets global HTTP timeout
func httpSetTimeout(args ...object.Object) object.Object {
	if !ModuleAllowed("http") {
		return object.NewError("http_set_timeout: access to native module http denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("http_set_timeout: expected 1 argument, got %d", len(args))
	}
	
	timeout, ok := args[0].(*object.Integer)
	if !ok {
		return object.NewError("http_set_timeout: argument must be integer, got %s", args[0].Type())
	}
	
	if timeout.Value <= 0 {
		return object.NewError("http_set_timeout: timeout must be positive")
	}
	
	httpTimeout = time.Duration(timeout.Value) * time.Second
	return object.TRUE
}

// httpGetCookies gets cookies for a domain
func httpGetCookies(args ...object.Object) object.Object {
	if !ModuleAllowed("http") {
		return object.NewError("http_get_cookies: access to native module http denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("http_get_cookies: expected 1 argument, got %d", len(args))
	}
	
	urlStr, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("http_get_cookies: argument must be string, got %s", args[0].Type())
	}
	
	// Parse URL
	parsedURL, err := url.Parse(urlStr.Value)
	if err != nil {
		return object.NewError("http_get_cookies: invalid URL: %s", err.Error())
	}
	
	cookies := httpCookieJar.Cookies(parsedURL)
	cookieArray := make([]object.Object, len(cookies))
	
	for i, cookie := range cookies {
		cookieMap := make(map[object.Object]object.Object)
		cookieMap[object.NewString("name")] = object.NewString(cookie.Name)
		cookieMap[object.NewString("value")] = object.NewString(cookie.Value)
		cookieMap[object.NewString("domain")] = object.NewString(cookie.Domain)
		cookieMap[object.NewString("path")] = object.NewString(cookie.Path)
		cookieMap[object.NewString("secure")] = object.NewBoolean(cookie.Secure)
		cookieMap[object.NewString("httponly")] = object.NewBoolean(cookie.HttpOnly)
		
		cookieArray[i] = object.NewMap(cookieMap)
	}
	
	return object.NewArray(cookieArray)
}

// httpSetCookies sets cookies for a domain
func httpSetCookies(args ...object.Object) object.Object {
	if !ModuleAllowed("http") {
		return object.NewError("http_set_cookies: access to native module http denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("http_set_cookies: expected 2 arguments, got %d", len(args))
	}
	
	urlStr, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("http_set_cookies: first argument must be string, got %s", args[0].Type())
	}
	
	cookiesArray, ok := args[1].(*object.Array)
	if !ok {
		return object.NewError("http_set_cookies: second argument must be array, got %s", args[1].Type())
	}
	
	// Parse URL
	parsedURL, err := url.Parse(urlStr.Value)
	if err != nil {
		return object.NewError("http_set_cookies: invalid URL: %s", err.Error())
	}
	
	var cookies []*http.Cookie
	
	for _, cookieObj := range cookiesArray.Elements {
		cookieMap, ok := cookieObj.(*object.Map)
		if !ok {
			continue
		}
		
		cookie := &http.Cookie{}
		
		for key, value := range cookieMap.Pairs {
			keyStr, ok := key.(*object.String)
			if !ok {
				continue
			}
			
			switch keyStr.Value {
			case "name":
				if valueStr, ok := value.(*object.String); ok {
					cookie.Name = valueStr.Value
				}
			case "value":
				if valueStr, ok := value.(*object.String); ok {
					cookie.Value = valueStr.Value
				}
			case "domain":
				if valueStr, ok := value.(*object.String); ok {
					cookie.Domain = valueStr.Value
				}
			case "path":
				if valueStr, ok := value.(*object.String); ok {
					cookie.Path = valueStr.Value
				}
			case "secure":
				if valueBool, ok := value.(*object.Boolean); ok {
					cookie.Secure = valueBool.Value
				}
			case "httponly":
				if valueBool, ok := value.(*object.Boolean); ok {
					cookie.HttpOnly = valueBool.Value
				}
			}
		}
		
		if cookie.Name != "" {
			cookies = append(cookies, cookie)
		}
	}
	
	httpCookieJar.SetCookies(parsedURL, cookies)
	return object.TRUE
}
