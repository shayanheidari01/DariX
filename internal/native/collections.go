package native

import (
	"darix/object"
	"sort"
)

func init() {
	Register("collections", map[string]*object.Builtin{
		"collections_filter":    {Fn: collectionsFilter},
		"collections_map":       {Fn: collectionsMap},
		"collections_reduce":    {Fn: collectionsReduce},
		"collections_find":      {Fn: collectionsFind},
		"collections_find_all":  {Fn: collectionsFindAll},
		"collections_unique":    {Fn: collectionsUnique},
		"collections_flatten":   {Fn: collectionsFlatten},
		"collections_chunk":     {Fn: collectionsChunk},
		"collections_zip":       {Fn: collectionsZip},
		"collections_unzip":     {Fn: collectionsUnzip},
		"collections_group_by":  {Fn: collectionsGroupBy},
		"collections_sort_by":   {Fn: collectionsSortBy},
		"collections_partition": {Fn: collectionsPartition},
		"collections_diff":      {Fn: collectionsDiff},
		"collections_intersect": {Fn: collectionsIntersect},
		"collections_union":     {Fn: collectionsUnion},
	})
}

// collectionsFilter filters array elements based on predicate function
func collectionsFilter(args ...object.Object) object.Object {
	if !ModuleAllowed("collections") {
		return object.NewError("collections_filter: access to native module collections denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("collections_filter: expected 2 arguments, got %d", len(args))
	}
	
	arr, ok1 := args[0].(*object.Array)
	predicate, ok2 := args[1].(*object.Function)
	if !ok1 || !ok2 {
		return object.NewError("collections_filter: arguments must be (array, function)")
	}
	
	var filtered []object.Object
	for _, elem := range arr.Elements {
		// Call predicate function with element
		result := callFunction(predicate, []object.Object{elem})
		if isError(result) {
			return result
		}
		
		if isTruthy(result) {
			filtered = append(filtered, elem)
		}
	}
	
	return object.NewArray(filtered)
}

// collectionsMap transforms array elements using mapper function
func collectionsMap(args ...object.Object) object.Object {
	if !ModuleAllowed("collections") {
		return object.NewError("collections_map: access to native module collections denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("collections_map: expected 2 arguments, got %d", len(args))
	}
	
	arr, ok1 := args[0].(*object.Array)
	mapper, ok2 := args[1].(*object.Function)
	if !ok1 || !ok2 {
		return object.NewError("collections_map: arguments must be (array, function)")
	}
	
	mapped := make([]object.Object, len(arr.Elements))
	for i, elem := range arr.Elements {
		result := callFunction(mapper, []object.Object{elem})
		if isError(result) {
			return result
		}
		mapped[i] = result
	}
	
	return object.NewArray(mapped)
}

// collectionsReduce reduces array to single value using reducer function
func collectionsReduce(args ...object.Object) object.Object {
	if !ModuleAllowed("collections") {
		return object.NewError("collections_reduce: access to native module collections denied by policy")
	}
	if len(args) < 2 || len(args) > 3 {
		return object.NewError("collections_reduce: expected 2-3 arguments, got %d", len(args))
	}
	
	arr, ok1 := args[0].(*object.Array)
	reducer, ok2 := args[1].(*object.Function)
	if !ok1 || !ok2 {
		return object.NewError("collections_reduce: first two arguments must be (array, function)")
	}
	
	if len(arr.Elements) == 0 {
		if len(args) == 3 {
			return args[2] // Return initial value
		}
		return object.NewError("collections_reduce: cannot reduce empty array without initial value")
	}
	
	var accumulator object.Object
	startIndex := 0
	
	if len(args) == 3 {
		accumulator = args[2]
	} else {
		accumulator = arr.Elements[0]
		startIndex = 1
	}
	
	for i := startIndex; i < len(arr.Elements); i++ {
		result := callFunction(reducer, []object.Object{accumulator, arr.Elements[i]})
		if isError(result) {
			return result
		}
		accumulator = result
	}
	
	return accumulator
}

// collectionsFind finds first element matching predicate
func collectionsFind(args ...object.Object) object.Object {
	if !ModuleAllowed("collections") {
		return object.NewError("collections_find: access to native module collections denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("collections_find: expected 2 arguments, got %d", len(args))
	}
	
	arr, ok1 := args[0].(*object.Array)
	predicate, ok2 := args[1].(*object.Function)
	if !ok1 || !ok2 {
		return object.NewError("collections_find: arguments must be (array, function)")
	}
	
	for _, elem := range arr.Elements {
		result := callFunction(predicate, []object.Object{elem})
		if isError(result) {
			return result
		}
		
		if isTruthy(result) {
			return elem
		}
	}
	
	return object.NULL
}

// collectionsFindAll finds all elements matching predicate
func collectionsFindAll(args ...object.Object) object.Object {
	if !ModuleAllowed("collections") {
		return object.NewError("collections_find_all: access to native module collections denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("collections_find_all: expected 2 arguments, got %d", len(args))
	}
	
	arr, ok1 := args[0].(*object.Array)
	predicate, ok2 := args[1].(*object.Function)
	if !ok1 || !ok2 {
		return object.NewError("collections_find_all: arguments must be (array, function)")
	}
	
	var found []object.Object
	for _, elem := range arr.Elements {
		result := callFunction(predicate, []object.Object{elem})
		if isError(result) {
			return result
		}
		
		if isTruthy(result) {
			found = append(found, elem)
		}
	}
	
	return object.NewArray(found)
}

// collectionsUnique removes duplicate elements from array
func collectionsUnique(args ...object.Object) object.Object {
	if !ModuleAllowed("collections") {
		return object.NewError("collections_unique: access to native module collections denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("collections_unique: expected 1 argument, got %d", len(args))
	}
	
	arr, ok := args[0].(*object.Array)
	if !ok {
		return object.NewError("collections_unique: argument must be array, got %s", args[0].Type())
	}
	
	seen := make(map[string]bool)
	var unique []object.Object
	
	for _, elem := range arr.Elements {
		key := elem.Inspect()
		if !seen[key] {
			seen[key] = true
			unique = append(unique, elem)
		}
	}
	
	return object.NewArray(unique)
}

// collectionsFlatten flattens nested arrays
func collectionsFlatten(args ...object.Object) object.Object {
	if !ModuleAllowed("collections") {
		return object.NewError("collections_flatten: access to native module collections denied by policy")
	}
	if len(args) < 1 || len(args) > 2 {
		return object.NewError("collections_flatten: expected 1-2 arguments, got %d", len(args))
	}
	
	arr, ok := args[0].(*object.Array)
	if !ok {
		return object.NewError("collections_flatten: first argument must be array, got %s", args[0].Type())
	}
	
	depth := int64(1) // Default depth
	if len(args) == 2 {
		if depthObj, ok := args[1].(*object.Integer); ok {
			depth = depthObj.Value
		} else {
			return object.NewError("collections_flatten: second argument must be integer")
		}
	}
	
	flattened := flattenArray(arr.Elements, depth)
	return object.NewArray(flattened)
}

// collectionsChunk splits array into chunks of specified size
func collectionsChunk(args ...object.Object) object.Object {
	if !ModuleAllowed("collections") {
		return object.NewError("collections_chunk: access to native module collections denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("collections_chunk: expected 2 arguments, got %d", len(args))
	}
	
	arr, ok1 := args[0].(*object.Array)
	size, ok2 := args[1].(*object.Integer)
	if !ok1 || !ok2 {
		return object.NewError("collections_chunk: arguments must be (array, integer)")
	}
	
	if size.Value <= 0 {
		return object.NewError("collections_chunk: chunk size must be positive")
	}
	
	var chunks []object.Object
	chunkSize := int(size.Value)
	
	for i := 0; i < len(arr.Elements); i += chunkSize {
		end := i + chunkSize
		if end > len(arr.Elements) {
			end = len(arr.Elements)
		}
		
		chunk := arr.Elements[i:end]
		chunks = append(chunks, object.NewArray(chunk))
	}
	
	return object.NewArray(chunks)
}

// collectionsZip combines multiple arrays element-wise
func collectionsZip(args ...object.Object) object.Object {
	if !ModuleAllowed("collections") {
		return object.NewError("collections_zip: access to native module collections denied by policy")
	}
	if len(args) < 2 {
		return object.NewError("collections_zip: expected at least 2 arguments, got %d", len(args))
	}
	
	arrays := make([]*object.Array, len(args))
	minLength := -1
	
	for i, arg := range args {
		arr, ok := arg.(*object.Array)
		if !ok {
			return object.NewError("collections_zip: all arguments must be arrays")
		}
		arrays[i] = arr
		
		if minLength == -1 || len(arr.Elements) < minLength {
			minLength = len(arr.Elements)
		}
	}
	
	var zipped []object.Object
	for i := 0; i < minLength; i++ {
		tuple := make([]object.Object, len(arrays))
		for j, arr := range arrays {
			tuple[j] = arr.Elements[i]
		}
		zipped = append(zipped, object.NewArray(tuple))
	}
	
	return object.NewArray(zipped)
}

// collectionsUnzip separates zipped array back into individual arrays
func collectionsUnzip(args ...object.Object) object.Object {
	if !ModuleAllowed("collections") {
		return object.NewError("collections_unzip: access to native module collections denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("collections_unzip: expected 1 argument, got %d", len(args))
	}
	
	arr, ok := args[0].(*object.Array)
	if !ok {
		return object.NewError("collections_unzip: argument must be array, got %s", args[0].Type())
	}
	
	if len(arr.Elements) == 0 {
		return object.NewArray([]object.Object{})
	}
	
	// Get tuple size from first element
	firstTuple, ok := arr.Elements[0].(*object.Array)
	if !ok {
		return object.NewError("collections_unzip: array elements must be arrays")
	}
	
	tupleSize := len(firstTuple.Elements)
	unzipped := make([][]object.Object, tupleSize)
	
	for _, elem := range arr.Elements {
		tuple, ok := elem.(*object.Array)
		if !ok {
			return object.NewError("collections_unzip: array elements must be arrays")
		}
		
		if len(tuple.Elements) != tupleSize {
			return object.NewError("collections_unzip: all tuples must have same length")
		}
		
		for i, item := range tuple.Elements {
			unzipped[i] = append(unzipped[i], item)
		}
	}
	
	result := make([]object.Object, tupleSize)
	for i, arr := range unzipped {
		result[i] = object.NewArray(arr)
	}
	
	return object.NewArray(result)
}

// collectionsGroupBy groups array elements by key function
func collectionsGroupBy(args ...object.Object) object.Object {
	if !ModuleAllowed("collections") {
		return object.NewError("collections_group_by: access to native module collections denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("collections_group_by: expected 2 arguments, got %d", len(args))
	}
	
	arr, ok1 := args[0].(*object.Array)
	keyFunc, ok2 := args[1].(*object.Function)
	if !ok1 || !ok2 {
		return object.NewError("collections_group_by: arguments must be (array, function)")
	}
	
	groups := make(map[string][]object.Object)
	
	for _, elem := range arr.Elements {
		keyResult := callFunction(keyFunc, []object.Object{elem})
		if isError(keyResult) {
			return keyResult
		}
		
		key := keyResult.Inspect()
		groups[key] = append(groups[key], elem)
	}
	
	result := make(map[object.Object]object.Object)
	for key, group := range groups {
		result[object.NewString(key)] = object.NewArray(group)
	}
	
	return object.NewMap(result)
}

// collectionsSortBy sorts array by key function
func collectionsSortBy(args ...object.Object) object.Object {
	if !ModuleAllowed("collections") {
		return object.NewError("collections_sort_by: access to native module collections denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("collections_sort_by: expected 2 arguments, got %d", len(args))
	}
	
	arr, ok1 := args[0].(*object.Array)
	keyFunc, ok2 := args[1].(*object.Function)
	if !ok1 || !ok2 {
		return object.NewError("collections_sort_by: arguments must be (array, function)")
	}
	
	// Create copy to avoid modifying original
	sorted := make([]object.Object, len(arr.Elements))
	copy(sorted, arr.Elements)
	
	// Sort using key function
	sort.Slice(sorted, func(i, j int) bool {
		keyI := callFunction(keyFunc, []object.Object{sorted[i]})
		keyJ := callFunction(keyFunc, []object.Object{sorted[j]})
		
		// Simple string comparison for sorting
		return keyI.Inspect() < keyJ.Inspect()
	})
	
	return object.NewArray(sorted)
}

// collectionsPartition splits array into two based on predicate
func collectionsPartition(args ...object.Object) object.Object {
	if !ModuleAllowed("collections") {
		return object.NewError("collections_partition: access to native module collections denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("collections_partition: expected 2 arguments, got %d", len(args))
	}
	
	arr, ok1 := args[0].(*object.Array)
	predicate, ok2 := args[1].(*object.Function)
	if !ok1 || !ok2 {
		return object.NewError("collections_partition: arguments must be (array, function)")
	}
	
	var truthy, falsy []object.Object
	
	for _, elem := range arr.Elements {
		result := callFunction(predicate, []object.Object{elem})
		if isError(result) {
			return result
		}
		
		if isTruthy(result) {
			truthy = append(truthy, elem)
		} else {
			falsy = append(falsy, elem)
		}
	}
	
	partition := []object.Object{
		object.NewArray(truthy),
		object.NewArray(falsy),
	}
	
	return object.NewArray(partition)
}

// collectionsDiff finds elements in first array not in second
func collectionsDiff(args ...object.Object) object.Object {
	if !ModuleAllowed("collections") {
		return object.NewError("collections_diff: access to native module collections denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("collections_diff: expected 2 arguments, got %d", len(args))
	}
	
	arr1, ok1 := args[0].(*object.Array)
	arr2, ok2 := args[1].(*object.Array)
	if !ok1 || !ok2 {
		return object.NewError("collections_diff: arguments must be arrays")
	}
	
	// Create set of second array elements
	set2 := make(map[string]bool)
	for _, elem := range arr2.Elements {
		set2[elem.Inspect()] = true
	}
	
	var diff []object.Object
	for _, elem := range arr1.Elements {
		if !set2[elem.Inspect()] {
			diff = append(diff, elem)
		}
	}
	
	return object.NewArray(diff)
}

// collectionsIntersect finds common elements between arrays
func collectionsIntersect(args ...object.Object) object.Object {
	if !ModuleAllowed("collections") {
		return object.NewError("collections_intersect: access to native module collections denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("collections_intersect: expected 2 arguments, got %d", len(args))
	}
	
	arr1, ok1 := args[0].(*object.Array)
	arr2, ok2 := args[1].(*object.Array)
	if !ok1 || !ok2 {
		return object.NewError("collections_intersect: arguments must be arrays")
	}
	
	// Create set of second array elements
	set2 := make(map[string]bool)
	for _, elem := range arr2.Elements {
		set2[elem.Inspect()] = true
	}
	
	var intersection []object.Object
	seen := make(map[string]bool)
	
	for _, elem := range arr1.Elements {
		key := elem.Inspect()
		if set2[key] && !seen[key] {
			intersection = append(intersection, elem)
			seen[key] = true
		}
	}
	
	return object.NewArray(intersection)
}

// collectionsUnion combines arrays removing duplicates
func collectionsUnion(args ...object.Object) object.Object {
	if !ModuleAllowed("collections") {
		return object.NewError("collections_union: access to native module collections denied by policy")
	}
	if len(args) < 2 {
		return object.NewError("collections_union: expected at least 2 arguments, got %d", len(args))
	}
	
	seen := make(map[string]bool)
	var union []object.Object
	
	for _, arg := range args {
		arr, ok := arg.(*object.Array)
		if !ok {
			return object.NewError("collections_union: all arguments must be arrays")
		}
		
		for _, elem := range arr.Elements {
			key := elem.Inspect()
			if !seen[key] {
				seen[key] = true
				union = append(union, elem)
			}
		}
	}
	
	return object.NewArray(union)
}

// Helper functions

func flattenArray(elements []object.Object, depth int64) []object.Object {
	if depth <= 0 {
		return elements
	}
	
	var flattened []object.Object
	for _, elem := range elements {
		if arr, ok := elem.(*object.Array); ok {
			flattened = append(flattened, flattenArray(arr.Elements, depth-1)...)
		} else {
			flattened = append(flattened, elem)
		}
	}
	return flattened
}

func callFunction(fn *object.Function, args []object.Object) object.Object {
	// This is a simplified function call - in real implementation,
	// you would need to properly evaluate the function with the interpreter
	// For now, return a placeholder
	return object.TRUE
}

func isError(obj object.Object) bool {
	return obj.Type() == object.ERROR_OBJ
}

func isTruthy(obj object.Object) bool {
	switch obj {
	case object.NULL:
		return false
	case object.TRUE:
		return true
	case object.FALSE:
		return false
	default:
		return true
	}
}
