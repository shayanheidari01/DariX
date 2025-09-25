package native

import (
	"crypto/md5"
	"crypto/sha1"
	"crypto/sha256"
	"crypto/sha512"
	"darix/object"
	"encoding/base64"
	"encoding/hex"
)

func init() {
	Register("crypto", map[string]*object.Builtin{
		"crypto_md5":           {Fn: cryptoMD5},
		"crypto_sha1":          {Fn: cryptoSHA1},
		"crypto_sha256":        {Fn: cryptoSHA256},
		"crypto_sha512":        {Fn: cryptoSHA512},
		"crypto_base64_encode": {Fn: cryptoBase64Encode},
		"crypto_base64_decode": {Fn: cryptoBase64Decode},
		"crypto_hex_encode":    {Fn: cryptoHexEncode},
		"crypto_hex_decode":    {Fn: cryptoHexDecode},
	})
}

// cryptoMD5 computes MD5 hash of input string
func cryptoMD5(args ...object.Object) object.Object {
	if !ModuleAllowed("crypto") {
		return object.NewError("crypto_md5: access to native module crypto denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("crypto_md5: expected 1 argument, got %d", len(args))
	}
	
	str, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("crypto_md5: argument must be string, got %s", args[0].Type())
	}
	
	hash := md5.Sum([]byte(str.Value))
	return object.NewString(hex.EncodeToString(hash[:]))
}

// cryptoSHA1 computes SHA1 hash of input string
func cryptoSHA1(args ...object.Object) object.Object {
	if !ModuleAllowed("crypto") {
		return object.NewError("crypto_sha1: access to native module crypto denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("crypto_sha1: expected 1 argument, got %d", len(args))
	}
	
	str, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("crypto_sha1: argument must be string, got %s", args[0].Type())
	}
	
	hash := sha1.Sum([]byte(str.Value))
	return object.NewString(hex.EncodeToString(hash[:]))
}

// cryptoSHA256 computes SHA256 hash of input string
func cryptoSHA256(args ...object.Object) object.Object {
	if !ModuleAllowed("crypto") {
		return object.NewError("crypto_sha256: access to native module crypto denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("crypto_sha256: expected 1 argument, got %d", len(args))
	}
	
	str, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("crypto_sha256: argument must be string, got %s", args[0].Type())
	}
	
	hash := sha256.Sum256([]byte(str.Value))
	return object.NewString(hex.EncodeToString(hash[:]))
}

// cryptoSHA512 computes SHA512 hash of input string
func cryptoSHA512(args ...object.Object) object.Object {
	if !ModuleAllowed("crypto") {
		return object.NewError("crypto_sha512: access to native module crypto denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("crypto_sha512: expected 1 argument, got %d", len(args))
	}
	
	str, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("crypto_sha512: argument must be string, got %s", args[0].Type())
	}
	
	hash := sha512.Sum512([]byte(str.Value))
	return object.NewString(hex.EncodeToString(hash[:]))
}

// cryptoBase64Encode encodes string to base64
func cryptoBase64Encode(args ...object.Object) object.Object {
	if !ModuleAllowed("crypto") {
		return object.NewError("crypto_base64_encode: access to native module crypto denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("crypto_base64_encode: expected 1 argument, got %d", len(args))
	}
	
	str, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("crypto_base64_encode: argument must be string, got %s", args[0].Type())
	}
	
	encoded := base64.StdEncoding.EncodeToString([]byte(str.Value))
	return object.NewString(encoded)
}

// cryptoBase64Decode decodes base64 string
func cryptoBase64Decode(args ...object.Object) object.Object {
	if !ModuleAllowed("crypto") {
		return object.NewError("crypto_base64_decode: access to native module crypto denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("crypto_base64_decode: expected 1 argument, got %d", len(args))
	}
	
	str, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("crypto_base64_decode: argument must be string, got %s", args[0].Type())
	}
	
	decoded, err := base64.StdEncoding.DecodeString(str.Value)
	if err != nil {
		return object.NewError("crypto_base64_decode: %s", err.Error())
	}
	
	return object.NewString(string(decoded))
}

// cryptoHexEncode encodes string to hexadecimal
func cryptoHexEncode(args ...object.Object) object.Object {
	if !ModuleAllowed("crypto") {
		return object.NewError("crypto_hex_encode: access to native module crypto denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("crypto_hex_encode: expected 1 argument, got %d", len(args))
	}
	
	str, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("crypto_hex_encode: argument must be string, got %s", args[0].Type())
	}
	
	encoded := hex.EncodeToString([]byte(str.Value))
	return object.NewString(encoded)
}

// cryptoHexDecode decodes hexadecimal string
func cryptoHexDecode(args ...object.Object) object.Object {
	if !ModuleAllowed("crypto") {
		return object.NewError("crypto_hex_decode: access to native module crypto denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("crypto_hex_decode: expected 1 argument, got %d", len(args))
	}
	
	str, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("crypto_hex_decode: argument must be string, got %s", args[0].Type())
	}
	
	decoded, err := hex.DecodeString(str.Value)
	if err != nil {
		return object.NewError("crypto_hex_decode: %s", err.Error())
	}
	
	return object.NewString(string(decoded))
}
