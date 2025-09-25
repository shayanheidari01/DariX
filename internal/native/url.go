package native

import (
	"darix/object"
	"net/url"
)

func init() {
	Register("url", map[string]*object.Builtin{
		"url_parse":         {Fn: urlParse},
		"url_build":         {Fn: urlBuild},
		"url_encode":        {Fn: urlEncode},
		"url_decode":        {Fn: urlDecode},
		"url_query_encode":  {Fn: urlQueryEncode},
		"url_query_decode":  {Fn: urlQueryDecode},
		"url_join":          {Fn: urlJoin},
		"url_resolve":       {Fn: urlResolve},
		"url_is_valid":      {Fn: urlIsValid},
	})
}

// urlParse parses URL string into components
func urlParse(args ...object.Object) object.Object {
	if !ModuleAllowed("url") {
		return object.NewError("url_parse: access to native module url denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("url_parse: expected 1 argument, got %d", len(args))
	}
	
	urlStr, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("url_parse: argument must be string, got %s", args[0].Type())
	}
	
	u, err := url.Parse(urlStr.Value)
	if err != nil {
		return object.NewError("url_parse: %s", err.Error())
	}
	
	// Create URL components map
	components := make(map[object.Object]object.Object)
	components[object.NewString("scheme")] = object.NewString(u.Scheme)
	components[object.NewString("host")] = object.NewString(u.Host)
	components[object.NewString("hostname")] = object.NewString(u.Hostname())
	components[object.NewString("port")] = object.NewString(u.Port())
	components[object.NewString("path")] = object.NewString(u.Path)
	components[object.NewString("query")] = object.NewString(u.RawQuery)
	components[object.NewString("fragment")] = object.NewString(u.Fragment)
	components[object.NewString("user")] = object.NewString(u.User.Username())
	
	if password, set := u.User.Password(); set {
		components[object.NewString("password")] = object.NewString(password)
	} else {
		components[object.NewString("password")] = object.NewString("")
	}
	
	return object.NewMap(components)
}

// urlBuild builds URL from components
func urlBuild(args ...object.Object) object.Object {
	if !ModuleAllowed("url") {
		return object.NewError("url_build: access to native module url denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("url_build: expected 1 argument, got %d", len(args))
	}
	
	components, ok := args[0].(*object.Map)
	if !ok {
		return object.NewError("url_build: argument must be map, got %s", args[0].Type())
	}
	
	u := &url.URL{}
	
	// Extract components from map
	for key, value := range components.Pairs {
		keyStr, ok := key.(*object.String)
		if !ok {
			continue
		}
		
		valueStr, ok := value.(*object.String)
		if !ok {
			continue
		}
		
		switch keyStr.Value {
		case "scheme":
			u.Scheme = valueStr.Value
		case "host":
			u.Host = valueStr.Value
		case "path":
			u.Path = valueStr.Value
		case "query":
			u.RawQuery = valueStr.Value
		case "fragment":
			u.Fragment = valueStr.Value
		case "user":
			if u.User == nil {
				u.User = url.User(valueStr.Value)
			}
		case "password":
			if u.User != nil {
				username := u.User.Username()
				u.User = url.UserPassword(username, valueStr.Value)
			}
		}
	}
	
	return object.NewString(u.String())
}

// urlEncode encodes string for URL
func urlEncode(args ...object.Object) object.Object {
	if !ModuleAllowed("url") {
		return object.NewError("url_encode: access to native module url denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("url_encode: expected 1 argument, got %d", len(args))
	}
	
	str, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("url_encode: argument must be string, got %s", args[0].Type())
	}
	
	encoded := url.PathEscape(str.Value)
	return object.NewString(encoded)
}

// urlDecode decodes URL-encoded string
func urlDecode(args ...object.Object) object.Object {
	if !ModuleAllowed("url") {
		return object.NewError("url_decode: access to native module url denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("url_decode: expected 1 argument, got %d", len(args))
	}
	
	str, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("url_decode: argument must be string, got %s", args[0].Type())
	}
	
	decoded, err := url.PathUnescape(str.Value)
	if err != nil {
		return object.NewError("url_decode: %s", err.Error())
	}
	
	return object.NewString(decoded)
}

// urlQueryEncode encodes map as query string
func urlQueryEncode(args ...object.Object) object.Object {
	if !ModuleAllowed("url") {
		return object.NewError("url_query_encode: access to native module url denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("url_query_encode: expected 1 argument, got %d", len(args))
	}
	
	params, ok := args[0].(*object.Map)
	if !ok {
		return object.NewError("url_query_encode: argument must be map, got %s", args[0].Type())
	}
	
	values := url.Values{}
	for key, value := range params.Pairs {
		keyStr, ok := key.(*object.String)
		if !ok {
			continue
		}
		
		valueStr, ok := value.(*object.String)
		if !ok {
			// Convert non-string values to strings
			valueStr = object.NewString(value.Inspect())
		}
		
		values.Add(keyStr.Value, valueStr.Value)
	}
	
	return object.NewString(values.Encode())
}

// urlQueryDecode decodes query string to map
func urlQueryDecode(args ...object.Object) object.Object {
	if !ModuleAllowed("url") {
		return object.NewError("url_query_decode: access to native module url denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("url_query_decode: expected 1 argument, got %d", len(args))
	}
	
	queryStr, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("url_query_decode: argument must be string, got %s", args[0].Type())
	}
	
	values, err := url.ParseQuery(queryStr.Value)
	if err != nil {
		return object.NewError("url_query_decode: %s", err.Error())
	}
	
	result := make(map[object.Object]object.Object)
	for key, valueList := range values {
		if len(valueList) == 1 {
			result[object.NewString(key)] = object.NewString(valueList[0])
		} else {
			// Multiple values - return as array
			elements := make([]object.Object, len(valueList))
			for i, v := range valueList {
				elements[i] = object.NewString(v)
			}
			result[object.NewString(key)] = object.NewArray(elements)
		}
	}
	
	return object.NewMap(result)
}

// urlJoin joins base URL with relative path
func urlJoin(args ...object.Object) object.Object {
	if !ModuleAllowed("url") {
		return object.NewError("url_join: access to native module url denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("url_join: expected 2 arguments, got %d", len(args))
	}
	
	baseStr, ok1 := args[0].(*object.String)
	relativeStr, ok2 := args[1].(*object.String)
	if !ok1 || !ok2 {
		return object.NewError("url_join: arguments must be strings")
	}
	
	base, err := url.Parse(baseStr.Value)
	if err != nil {
		return object.NewError("url_join: invalid base URL: %s", err.Error())
	}
	
	relative, err := url.Parse(relativeStr.Value)
	if err != nil {
		return object.NewError("url_join: invalid relative URL: %s", err.Error())
	}
	
	resolved := base.ResolveReference(relative)
	return object.NewString(resolved.String())
}

// urlResolve resolves relative URL against base URL
func urlResolve(args ...object.Object) object.Object {
	if !ModuleAllowed("url") {
		return object.NewError("url_resolve: access to native module url denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("url_resolve: expected 2 arguments, got %d", len(args))
	}
	
	baseStr, ok1 := args[0].(*object.String)
	relativeStr, ok2 := args[1].(*object.String)
	if !ok1 || !ok2 {
		return object.NewError("url_resolve: arguments must be strings")
	}
	
	base, err := url.Parse(baseStr.Value)
	if err != nil {
		return object.NewError("url_resolve: invalid base URL: %s", err.Error())
	}
	
	relative, err := url.Parse(relativeStr.Value)
	if err != nil {
		return object.NewError("url_resolve: invalid relative URL: %s", err.Error())
	}
	
	resolved := base.ResolveReference(relative)
	return object.NewString(resolved.String())
}

// urlIsValid checks if URL string is valid
func urlIsValid(args ...object.Object) object.Object {
	if !ModuleAllowed("url") {
		return object.NewError("url_is_valid: access to native module url denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("url_is_valid: expected 1 argument, got %d", len(args))
	}
	
	urlStr, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("url_is_valid: argument must be string, got %s", args[0].Type())
	}
	
	_, err := url.Parse(urlStr.Value)
	return object.NewBoolean(err == nil)
}
