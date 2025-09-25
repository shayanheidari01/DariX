package native

import (
	"darix/object"
	"strconv"
	"strings"
)

func init() {
	Register("base", map[string]*object.Builtin{
		"base_to_binary":    {Fn: baseToBinary},
		"base_to_octal":     {Fn: baseToOctal},
		"base_to_hex":       {Fn: baseToHex},
		"base_from_binary":  {Fn: baseFromBinary},
		"base_from_octal":   {Fn: baseFromOctal},
		"base_from_hex":     {Fn: baseFromHex},
		"base_convert":      {Fn: baseConvert},
		"base_to_base":      {Fn: baseToBase},
		"base_validate":     {Fn: baseValidate},
	})
}

// baseToBinary converts integer to binary string
func baseToBinary(args ...object.Object) object.Object {
	if !ModuleAllowed("base") {
		return object.NewError("base_to_binary: access to native module base denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("base_to_binary: expected 1 argument, got %d", len(args))
	}
	
	num, ok := args[0].(*object.Integer)
	if !ok {
		return object.NewError("base_to_binary: argument must be integer, got %s", args[0].Type())
	}
	
	binary := strconv.FormatInt(num.Value, 2)
	return object.NewString(binary)
}

// baseToOctal converts integer to octal string
func baseToOctal(args ...object.Object) object.Object {
	if !ModuleAllowed("base") {
		return object.NewError("base_to_octal: access to native module base denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("base_to_octal: expected 1 argument, got %d", len(args))
	}
	
	num, ok := args[0].(*object.Integer)
	if !ok {
		return object.NewError("base_to_octal: argument must be integer, got %s", args[0].Type())
	}
	
	octal := strconv.FormatInt(num.Value, 8)
	return object.NewString(octal)
}

// baseToHex converts integer to hexadecimal string
func baseToHex(args ...object.Object) object.Object {
	if !ModuleAllowed("base") {
		return object.NewError("base_to_hex: access to native module base denied by policy")
	}
	if len(args) < 1 || len(args) > 2 {
		return object.NewError("base_to_hex: expected 1-2 arguments, got %d", len(args))
	}
	
	num, ok := args[0].(*object.Integer)
	if !ok {
		return object.NewError("base_to_hex: first argument must be integer, got %s", args[0].Type())
	}
	
	uppercase := false
	if len(args) == 2 {
		if uppercaseObj, ok := args[1].(*object.Boolean); ok {
			uppercase = uppercaseObj.Value
		} else {
			return object.NewError("base_to_hex: second argument must be boolean")
		}
	}
	
	hex := strconv.FormatInt(num.Value, 16)
	if uppercase {
		hex = strings.ToUpper(hex)
	}
	
	return object.NewString(hex)
}

// baseFromBinary converts binary string to integer
func baseFromBinary(args ...object.Object) object.Object {
	if !ModuleAllowed("base") {
		return object.NewError("base_from_binary: access to native module base denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("base_from_binary: expected 1 argument, got %d", len(args))
	}
	
	binaryStr, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("base_from_binary: argument must be string, got %s", args[0].Type())
	}
	
	num, err := strconv.ParseInt(binaryStr.Value, 2, 64)
	if err != nil {
		return object.NewError("base_from_binary: %s", err.Error())
	}
	
	return object.NewInteger(num)
}

// baseFromOctal converts octal string to integer
func baseFromOctal(args ...object.Object) object.Object {
	if !ModuleAllowed("base") {
		return object.NewError("base_from_octal: access to native module base denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("base_from_octal: expected 1 argument, got %d", len(args))
	}
	
	octalStr, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("base_from_octal: argument must be string, got %s", args[0].Type())
	}
	
	num, err := strconv.ParseInt(octalStr.Value, 8, 64)
	if err != nil {
		return object.NewError("base_from_octal: %s", err.Error())
	}
	
	return object.NewInteger(num)
}

// baseFromHex converts hexadecimal string to integer
func baseFromHex(args ...object.Object) object.Object {
	if !ModuleAllowed("base") {
		return object.NewError("base_from_hex: access to native module base denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("base_from_hex: expected 1 argument, got %d", len(args))
	}
	
	hexStr, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("base_from_hex: argument must be string, got %s", args[0].Type())
	}
	
	// Remove common hex prefixes
	cleanHex := strings.TrimPrefix(strings.ToLower(hexStr.Value), "0x")
	cleanHex = strings.TrimPrefix(cleanHex, "#")
	
	num, err := strconv.ParseInt(cleanHex, 16, 64)
	if err != nil {
		return object.NewError("base_from_hex: %s", err.Error())
	}
	
	return object.NewInteger(num)
}

// baseConvert converts number from one base to another
func baseConvert(args ...object.Object) object.Object {
	if !ModuleAllowed("base") {
		return object.NewError("base_convert: access to native module base denied by policy")
	}
	if len(args) != 3 {
		return object.NewError("base_convert: expected 3 arguments, got %d", len(args))
	}
	
	numStr, ok1 := args[0].(*object.String)
	fromBase, ok2 := args[1].(*object.Integer)
	toBase, ok3 := args[2].(*object.Integer)
	if !ok1 || !ok2 || !ok3 {
		return object.NewError("base_convert: arguments must be (string, integer, integer)")
	}
	
	if fromBase.Value < 2 || fromBase.Value > 36 {
		return object.NewError("base_convert: from_base must be between 2 and 36")
	}
	
	if toBase.Value < 2 || toBase.Value > 36 {
		return object.NewError("base_convert: to_base must be between 2 and 36")
	}
	
	// Parse number in source base
	num, err := strconv.ParseInt(numStr.Value, int(fromBase.Value), 64)
	if err != nil {
		return object.NewError("base_convert: %s", err.Error())
	}
	
	// Convert to target base
	result := strconv.FormatInt(num, int(toBase.Value))
	return object.NewString(result)
}

// baseToBase converts between arbitrary bases with custom digits
func baseToBase(args ...object.Object) object.Object {
	if !ModuleAllowed("base") {
		return object.NewError("base_to_base: access to native module base denied by policy")
	}
	if len(args) != 4 {
		return object.NewError("base_to_base: expected 4 arguments, got %d", len(args))
	}
	
	numStr, ok1 := args[0].(*object.String)
	fromBase, ok2 := args[1].(*object.Integer)
	toBase, ok3 := args[2].(*object.Integer)
	digits, ok4 := args[3].(*object.String)
	if !ok1 || !ok2 || !ok3 || !ok4 {
		return object.NewError("base_to_base: arguments must be (string, integer, integer, string)")
	}
	
	if fromBase.Value < 2 || fromBase.Value > int64(len(digits.Value)) {
		return object.NewError("base_to_base: from_base must be between 2 and %d", len(digits.Value))
	}
	
	if toBase.Value < 2 || toBase.Value > int64(len(digits.Value)) {
		return object.NewError("base_to_base: to_base must be between 2 and %d", len(digits.Value))
	}
	
	// Convert from source base to decimal
	var decimal int64 = 0
	var power int64 = 1
	
	for i := len(numStr.Value) - 1; i >= 0; i-- {
		char := numStr.Value[i]
		digitIndex := strings.IndexByte(digits.Value, char)
		if digitIndex == -1 || digitIndex >= int(fromBase.Value) {
			return object.NewError("base_to_base: invalid digit '%c' for base %d", char, fromBase.Value)
		}
		decimal += int64(digitIndex) * power
		power *= fromBase.Value
	}
	
	// Convert from decimal to target base
	if decimal == 0 {
		return object.NewString(string(digits.Value[0]))
	}
	
	var result strings.Builder
	for decimal > 0 {
		remainder := decimal % toBase.Value
		result.WriteByte(digits.Value[remainder])
		decimal /= toBase.Value
	}
	
	// Reverse the result
	resultStr := result.String()
	runes := []rune(resultStr)
	for i, j := 0, len(runes)-1; i < j; i, j = i+1, j-1 {
		runes[i], runes[j] = runes[j], runes[i]
	}
	
	return object.NewString(string(runes))
}

// baseValidate validates if string is valid in given base
func baseValidate(args ...object.Object) object.Object {
	if !ModuleAllowed("base") {
		return object.NewError("base_validate: access to native module base denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("base_validate: expected 2 arguments, got %d", len(args))
	}
	
	numStr, ok1 := args[0].(*object.String)
	base, ok2 := args[1].(*object.Integer)
	if !ok1 || !ok2 {
		return object.NewError("base_validate: arguments must be (string, integer)")
	}
	
	if base.Value < 2 || base.Value > 36 {
		return object.NewError("base_validate: base must be between 2 and 36")
	}
	
	_, err := strconv.ParseInt(numStr.Value, int(base.Value), 64)
	return object.NewBoolean(err == nil)
}
