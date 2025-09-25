package native

import (
	"crypto/rand"
	"darix/object"
	"fmt"
	"math/big"
	mathRand "math/rand"
	"time"
)

func init() {
	// Initialize random seed
	mathRand.Seed(time.Now().UnixNano())
	
	Register("random", map[string]*object.Builtin{
		"random_int":        {Fn: randomInt},
		"random_float":      {Fn: randomFloat},
		"random_range":      {Fn: randomRange},
		"random_choice":     {Fn: randomChoice},
		"random_shuffle":    {Fn: randomShuffle},
		"random_sample":     {Fn: randomSample},
		"random_bytes":      {Fn: randomBytes},
		"random_string":     {Fn: randomString},
		"random_uuid":       {Fn: randomUUID},
		"random_seed":       {Fn: randomSeed},
		"random_crypto_int": {Fn: randomCryptoInt},
	})
}

// randomInt generates random integer between 0 and max-1
func randomInt(args ...object.Object) object.Object {
	if !ModuleAllowed("random") {
		return object.NewError("random_int: access to native module random denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("random_int: expected 1 argument, got %d", len(args))
	}
	
	max, ok := args[0].(*object.Integer)
	if !ok {
		return object.NewError("random_int: argument must be integer, got %s", args[0].Type())
	}
	
	if max.Value <= 0 {
		return object.NewError("random_int: max must be positive")
	}
	
	result := mathRand.Int63n(max.Value)
	return object.NewInteger(result)
}

// randomFloat generates random float between 0.0 and 1.0
func randomFloat(args ...object.Object) object.Object {
	if !ModuleAllowed("random") {
		return object.NewError("random_float: access to native module random denied by policy")
	}
	if len(args) != 0 {
		return object.NewError("random_float: expected 0 arguments, got %d", len(args))
	}
	
	result := mathRand.Float64()
	return object.NewFloat(result)
}

// randomRange generates random integer in range [min, max]
func randomRange(args ...object.Object) object.Object {
	if !ModuleAllowed("random") {
		return object.NewError("random_range: access to native module random denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("random_range: expected 2 arguments, got %d", len(args))
	}
	
	min, ok1 := args[0].(*object.Integer)
	max, ok2 := args[1].(*object.Integer)
	if !ok1 || !ok2 {
		return object.NewError("random_range: arguments must be integers")
	}
	
	if min.Value >= max.Value {
		return object.NewError("random_range: min must be less than max")
	}
	
	result := mathRand.Int63n(max.Value-min.Value+1) + min.Value
	return object.NewInteger(result)
}

// randomChoice selects random element from array
func randomChoice(args ...object.Object) object.Object {
	if !ModuleAllowed("random") {
		return object.NewError("random_choice: access to native module random denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("random_choice: expected 1 argument, got %d", len(args))
	}
	
	arr, ok := args[0].(*object.Array)
	if !ok {
		return object.NewError("random_choice: argument must be array, got %s", args[0].Type())
	}
	
	if len(arr.Elements) == 0 {
		return object.NewError("random_choice: cannot choose from empty array")
	}
	
	index := mathRand.Intn(len(arr.Elements))
	return arr.Elements[index]
}

// randomShuffle shuffles array elements
func randomShuffle(args ...object.Object) object.Object {
	if !ModuleAllowed("random") {
		return object.NewError("random_shuffle: access to native module random denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("random_shuffle: expected 1 argument, got %d", len(args))
	}
	
	arr, ok := args[0].(*object.Array)
	if !ok {
		return object.NewError("random_shuffle: argument must be array, got %s", args[0].Type())
	}
	
	// Create a copy to avoid modifying original
	shuffled := make([]object.Object, len(arr.Elements))
	copy(shuffled, arr.Elements)
	
	// Fisher-Yates shuffle
	for i := len(shuffled) - 1; i > 0; i-- {
		j := mathRand.Intn(i + 1)
		shuffled[i], shuffled[j] = shuffled[j], shuffled[i]
	}
	
	return object.NewArray(shuffled)
}

// randomSample selects random sample from array
func randomSample(args ...object.Object) object.Object {
	if !ModuleAllowed("random") {
		return object.NewError("random_sample: access to native module random denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("random_sample: expected 2 arguments, got %d", len(args))
	}
	
	arr, ok1 := args[0].(*object.Array)
	size, ok2 := args[1].(*object.Integer)
	if !ok1 || !ok2 {
		return object.NewError("random_sample: first argument must be array, second must be integer")
	}
	
	if size.Value < 0 {
		return object.NewError("random_sample: sample size must be non-negative")
	}
	
	if size.Value > int64(len(arr.Elements)) {
		return object.NewError("random_sample: sample size cannot exceed array length")
	}
	
	// Create indices and shuffle them
	indices := make([]int, len(arr.Elements))
	for i := range indices {
		indices[i] = i
	}
	
	// Partial Fisher-Yates shuffle
	for i := 0; i < int(size.Value); i++ {
		j := mathRand.Intn(len(indices)-i) + i
		indices[i], indices[j] = indices[j], indices[i]
	}
	
	// Select elements
	sample := make([]object.Object, size.Value)
	for i := 0; i < int(size.Value); i++ {
		sample[i] = arr.Elements[indices[i]]
	}
	
	return object.NewArray(sample)
}

// randomBytes generates random bytes
func randomBytes(args ...object.Object) object.Object {
	if !ModuleAllowed("random") {
		return object.NewError("random_bytes: access to native module random denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("random_bytes: expected 1 argument, got %d", len(args))
	}
	
	length, ok := args[0].(*object.Integer)
	if !ok {
		return object.NewError("random_bytes: argument must be integer, got %s", args[0].Type())
	}
	
	if length.Value <= 0 {
		return object.NewError("random_bytes: length must be positive")
	}
	
	bytes := make([]byte, length.Value)
	_, err := rand.Read(bytes)
	if err != nil {
		return object.NewError("random_bytes: %s", err.Error())
	}
	
	// Convert bytes to array of integers
	elements := make([]object.Object, len(bytes))
	for i, b := range bytes {
		elements[i] = object.NewInteger(int64(b))
	}
	
	return object.NewArray(elements)
}

// randomString generates random string
func randomString(args ...object.Object) object.Object {
	if !ModuleAllowed("random") {
		return object.NewError("random_string: access to native module random denied by policy")
	}
	if len(args) < 1 || len(args) > 2 {
		return object.NewError("random_string: expected 1-2 arguments, got %d", len(args))
	}
	
	length, ok := args[0].(*object.Integer)
	if !ok {
		return object.NewError("random_string: first argument must be integer, got %s", args[0].Type())
	}
	
	if length.Value <= 0 {
		return object.NewError("random_string: length must be positive")
	}
	
	charset := "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
	if len(args) == 2 {
		if charsetObj, ok := args[1].(*object.String); ok {
			charset = charsetObj.Value
		} else {
			return object.NewError("random_string: second argument must be string")
		}
	}
	
	if len(charset) == 0 {
		return object.NewError("random_string: charset cannot be empty")
	}
	
	result := make([]byte, length.Value)
	for i := range result {
		result[i] = charset[mathRand.Intn(len(charset))]
	}
	
	return object.NewString(string(result))
}

// randomUUID generates UUID v4
func randomUUID(args ...object.Object) object.Object {
	if !ModuleAllowed("random") {
		return object.NewError("random_uuid: access to native module random denied by policy")
	}
	if len(args) != 0 {
		return object.NewError("random_uuid: expected 0 arguments, got %d", len(args))
	}
	
	bytes := make([]byte, 16)
	_, err := rand.Read(bytes)
	if err != nil {
		return object.NewError("random_uuid: %s", err.Error())
	}
	
	// Set version (4) and variant bits
	bytes[6] = (bytes[6] & 0x0f) | 0x40 // Version 4
	bytes[8] = (bytes[8] & 0x3f) | 0x80 // Variant 10
	
	uuid := fmt.Sprintf("%x-%x-%x-%x-%x",
		bytes[0:4], bytes[4:6], bytes[6:8], bytes[8:10], bytes[10:16])
	
	return object.NewString(uuid)
}

// randomSeed sets random seed
func randomSeed(args ...object.Object) object.Object {
	if !ModuleAllowed("random") {
		return object.NewError("random_seed: access to native module random denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("random_seed: expected 1 argument, got %d", len(args))
	}
	
	seed, ok := args[0].(*object.Integer)
	if !ok {
		return object.NewError("random_seed: argument must be integer, got %s", args[0].Type())
	}
	
	mathRand.Seed(seed.Value)
	return object.TRUE
}

// randomCryptoInt generates cryptographically secure random integer
func randomCryptoInt(args ...object.Object) object.Object {
	if !ModuleAllowed("random") {
		return object.NewError("random_crypto_int: access to native module random denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("random_crypto_int: expected 1 argument, got %d", len(args))
	}
	
	max, ok := args[0].(*object.Integer)
	if !ok {
		return object.NewError("random_crypto_int: argument must be integer, got %s", args[0].Type())
	}
	
	if max.Value <= 0 {
		return object.NewError("random_crypto_int: max must be positive")
	}
	
	n, err := rand.Int(rand.Reader, big.NewInt(max.Value))
	if err != nil {
		return object.NewError("random_crypto_int: %s", err.Error())
	}
	
	return object.NewInteger(n.Int64())
}
