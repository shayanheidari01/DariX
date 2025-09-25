package native

import (
	"darix/object"
	"encoding/json"
	"fmt"
)

func init() {
	Register("json", map[string]*object.Builtin{
		"json_parse":     {Fn: jsonParse},
		"json_stringify": {Fn: jsonStringify},
	})
}

// jsonParse converts JSON string to DariX object
func jsonParse(args ...object.Object) object.Object {
	if !ModuleAllowed("json") {
		return object.NewError("json_parse: access to native module json denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("json_parse: expected 1 argument, got %d", len(args))
	}
	
	str, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("json_parse: argument must be string, got %s", args[0].Type())
	}
	
	var data interface{}
	if err := json.Unmarshal([]byte(str.Value), &data); err != nil {
		return object.NewError("json_parse: %s", err.Error())
	}
	
	return convertFromJSON(data)
}

// jsonStringify converts DariX object to JSON string
func jsonStringify(args ...object.Object) object.Object {
	if !ModuleAllowed("json") {
		return object.NewError("json_stringify: access to native module json denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("json_stringify: expected 1 argument, got %d", len(args))
	}
	
	data := convertToJSON(args[0])
	if data == nil {
		return object.NewError("json_stringify: cannot convert %s to JSON", args[0].Type())
	}
	
	bytes, err := json.Marshal(data)
	if err != nil {
		return object.NewError("json_stringify: %s", err.Error())
	}
	
	return object.NewString(string(bytes))
}

// convertFromJSON converts Go interface{} to DariX object
func convertFromJSON(data interface{}) object.Object {
	switch v := data.(type) {
	case nil:
		return object.NULL
	case bool:
		return object.NewBoolean(v)
	case float64:
		return object.NewFloat(v)
	case string:
		return object.NewString(v)
	case []interface{}:
		elements := make([]object.Object, len(v))
		for i, elem := range v {
			elements[i] = convertFromJSON(elem)
		}
		return object.NewArray(elements)
	case map[string]interface{}:
		pairs := make(map[object.Object]object.Object)
		for key, val := range v {
			keyObj := object.NewString(key)
			valObj := convertFromJSON(val)
			pairs[keyObj] = valObj
		}
		return object.NewMap(pairs)
	default:
		return object.NewString(fmt.Sprintf("%v", v))
	}
}

// convertToJSON converts DariX object to Go interface{}
func convertToJSON(obj object.Object) interface{} {
	switch v := obj.(type) {
	case *object.Null:
		return nil
	case *object.Boolean:
		return v.Value
	case *object.Integer:
		return float64(v.Value)
	case *object.Float:
		return v.Value
	case *object.String:
		return v.Value
	case *object.Array:
		result := make([]interface{}, len(v.Elements))
		for i, elem := range v.Elements {
			result[i] = convertToJSON(elem)
		}
		return result
	case *object.Map:
		result := make(map[string]interface{})
		for key, val := range v.Pairs {
			if keyStr, ok := key.(*object.String); ok {
				result[keyStr.Value] = convertToJSON(val)
			} else {
				// Convert non-string keys to strings
				result[key.Inspect()] = convertToJSON(val)
			}
		}
		return result
	default:
		return nil
	}
}
