package vm

import (
	"darix/code"
	"darix/object"
	"sync"
	"unsafe"
)

// JIT compilation threshold - how many times a code path must be executed before JIT compilation
const JIT_THRESHOLD = 100

// HotPath represents a frequently executed code segment
type HotPath struct {
	StartIP     int                    // Starting instruction pointer
	EndIP       int                    // Ending instruction pointer
	ExecuteCount int                   // Number of times this path has been executed
	CompiledCode unsafe.Pointer        // Pointer to compiled native code
	IsCompiled   bool                  // Whether this path has been JIT compiled
	Instructions code.Instructions     // Original bytecode instructions
}

// JITCompiler handles Just-In-Time compilation of hot paths
type JITCompiler struct {
	hotPaths    map[int]*HotPath      // Map of instruction pointer to hot path
	execCounts  map[int]int           // Execution count for each instruction
	mutex       sync.RWMutex          // Thread safety
	enabled     bool                  // Whether JIT compilation is enabled
}

// NewJITCompiler creates a new JIT compiler
func NewJITCompiler() *JITCompiler {
	return &JITCompiler{
		hotPaths:   make(map[int]*HotPath),
		execCounts: make(map[int]int),
		enabled:    true,
	}
}

// Enable/Disable JIT compilation
func (jit *JITCompiler) SetEnabled(enabled bool) {
	jit.mutex.Lock()
	defer jit.mutex.Unlock()
	jit.enabled = enabled
}

// RecordExecution records the execution of an instruction and checks for hot paths
func (jit *JITCompiler) RecordExecution(ip int) bool {
	if !jit.enabled {
		return false
	}
	
	jit.mutex.Lock()
	defer jit.mutex.Unlock()
	
	jit.execCounts[ip]++
	
	// Check if this instruction has become hot
	if jit.execCounts[ip] >= JIT_THRESHOLD {
		// Check if we already have a hot path starting here
		if _, exists := jit.hotPaths[ip]; !exists {
			// Create a new hot path
			jit.hotPaths[ip] = &HotPath{
				StartIP:      ip,
				ExecuteCount: jit.execCounts[ip],
				IsCompiled:   false,
			}
			return true // Signal that this is a new hot path
		}
	}
	
	return false
}

// ShouldJITCompile checks if a code path should be JIT compiled
func (jit *JITCompiler) ShouldJITCompile(ip int) (*HotPath, bool) {
	jit.mutex.RLock()
	defer jit.mutex.RUnlock()
	
	if hotPath, exists := jit.hotPaths[ip]; exists && !hotPath.IsCompiled {
		return hotPath, true
	}
	
	return nil, false
}

// CompileHotPath compiles a hot path to native code (simplified implementation)
func (jit *JITCompiler) CompileHotPath(hotPath *HotPath, instructions code.Instructions) error {
	jit.mutex.Lock()
	defer jit.mutex.Unlock()
	
	// For now, we'll implement a simple "compilation" that creates optimized bytecode
	// In a real JIT, this would generate actual machine code
	
	// Analyze the instruction sequence to determine the end of the hot path
	endIP := jit.findHotPathEnd(hotPath.StartIP, instructions)
	hotPath.EndIP = endIP
	
	// Extract the instructions for this hot path
	if endIP > hotPath.StartIP && endIP < len(instructions) {
		hotPath.Instructions = instructions[hotPath.StartIP:endIP]
	}
	
	// Mark as compiled
	hotPath.IsCompiled = true
	
	return nil
}

// findHotPathEnd finds the end of a hot path (simplified heuristic)
func (jit *JITCompiler) findHotPathEnd(startIP int, instructions code.Instructions) int {
	// Simple heuristic: find the next jump instruction or end of a loop
	ip := startIP
	for ip < len(instructions) {
		op := code.Opcode(instructions[ip])
		
		switch op {
		case code.OpJump, code.OpJumpNotTruthy:
			// End of hot path at jump
			return ip + 3 // Include the jump instruction
		case code.OpPrint:
			// Function calls might be good stopping points
			return ip + 3
		}
		
		// Move to next instruction
		if def, ok := code.Lookup(op); ok {
			ip += 1 // opcode
			for _, width := range def.OperandWidths {
				ip += width
			}
		} else {
			ip++
		}
		
		// Limit hot path length
		if ip-startIP > 50 {
			break
		}
	}
	
	return ip
}

// ExecuteCompiledPath executes a JIT-compiled hot path
func (jit *JITCompiler) ExecuteCompiledPath(hotPath *HotPath, vm *VM) object.Object {
	// For this simplified implementation, we'll execute optimized bytecode
	// In a real JIT, this would call the compiled native code
	
	if !hotPath.IsCompiled || len(hotPath.Instructions) == 0 {
		return nil
	}
	
	// Execute the hot path with optimizations
	return jit.executeOptimizedBytecode(hotPath, vm)
}

// executeOptimizedBytecode executes bytecode with JIT optimizations
func (jit *JITCompiler) executeOptimizedBytecode(hotPath *HotPath, vm *VM) object.Object {
	// Save current VM state
	savedIP := vm.ip
	
	// Execute the hot path instructions with optimizations
	for i := 0; i < len(hotPath.Instructions); {
		op := code.Opcode(hotPath.Instructions[i])
		
		switch op {
		case code.OpConstant:
			if i+2 < len(hotPath.Instructions) {
				operand := uint16(hotPath.Instructions[i+1])<<8 | uint16(hotPath.Instructions[i+2])
				vm.stack[vm.sp] = vm.constants[operand]
				vm.sp++
				i += 3
			} else {
				i++
			}
		case code.OpAdd:
			if vm.sp >= 2 {
				right := vm.stack[vm.sp-1]
				left := vm.stack[vm.sp-2]
				vm.sp -= 2
				
				// Fast path for integers
				if l, ok := left.(*object.Integer); ok {
					if r, ok := right.(*object.Integer); ok {
						vm.stack[vm.sp] = object.AddIntegers(l, r)
						vm.sp++
						i++
						continue
					}
				}
				
				// Fallback to regular execution
				result := vm.execBinary(code.OpAdd, left, right)
				if isError(result) {
					vm.ip = savedIP
					return result
				}
				vm.stack[vm.sp] = result
				vm.sp++
			}
			i++
		case code.OpSub:
			if vm.sp >= 2 {
				right := vm.stack[vm.sp-1]
				left := vm.stack[vm.sp-2]
				vm.sp -= 2
				
				// Fast path for integers
				if l, ok := left.(*object.Integer); ok {
					if r, ok := right.(*object.Integer); ok {
						vm.stack[vm.sp] = object.SubIntegers(l, r)
						vm.sp++
						i++
						continue
					}
				}
				
				result := vm.execBinary(code.OpSub, left, right)
				if isError(result) {
					vm.ip = savedIP
					return result
				}
				vm.stack[vm.sp] = result
				vm.sp++
			}
			i++
		case code.OpMul:
			if vm.sp >= 2 {
				right := vm.stack[vm.sp-1]
				left := vm.stack[vm.sp-2]
				vm.sp -= 2
				
				// Fast path for integers
				if l, ok := left.(*object.Integer); ok {
					if r, ok := right.(*object.Integer); ok {
						vm.stack[vm.sp] = object.MulIntegers(l, r)
						vm.sp++
						i++
						continue
					}
				}
				
				result := vm.execBinary(code.OpMul, left, right)
				if isError(result) {
					vm.ip = savedIP
					return result
				}
				vm.stack[vm.sp] = result
				vm.sp++
			}
			i++
		default:
			// For other instructions, fall back to regular VM execution
			vm.ip = savedIP + hotPath.StartIP + i
			return nil // Signal to continue with regular execution
		}
	}
	
	// Update VM instruction pointer
	vm.ip = savedIP + (hotPath.EndIP - hotPath.StartIP)
	return nil
}

// GetStats returns JIT compilation statistics
func (jit *JITCompiler) GetStats() map[string]interface{} {
	jit.mutex.RLock()
	defer jit.mutex.RUnlock()
	
	compiledPaths := 0
	totalHotPaths := len(jit.hotPaths)
	
	for _, hotPath := range jit.hotPaths {
		if hotPath.IsCompiled {
			compiledPaths++
		}
	}
	
	return map[string]interface{}{
		"enabled":        jit.enabled,
		"hot_paths":      totalHotPaths,
		"compiled_paths": compiledPaths,
		"threshold":      JIT_THRESHOLD,
	}
}

// Reset clears all JIT compilation data
func (jit *JITCompiler) Reset() {
	jit.mutex.Lock()
	defer jit.mutex.Unlock()
	
	jit.hotPaths = make(map[int]*HotPath)
	jit.execCounts = make(map[int]int)
}
