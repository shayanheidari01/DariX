package vm

import (
	"darix/code"
	"darix/object"
	"sync"
	"unsafe"
)

const (
	JITThreshold = 100
	// JIT_THRESHOLD is kept for backward compatibility with existing code.
	JIT_THRESHOLD = JITThreshold
)

// HotPath describes a frequently executed bytecode region.
type HotPath struct {
	StartIP      int
	EndIP        int
	ExecuteCount int
	CompiledCode unsafe.Pointer
	Instructions code.Instructions
	compiled     bool
}

func (hp *HotPath) IsCompiled() bool { return hp.compiled }

func (hp *HotPath) markCompiled() { hp.compiled = true }

// JITCompiler tracks execution counts and manages hot-path compilation.
type JITCompiler struct {
	hotPaths   map[int]*HotPath
	execCounts map[int]int
	mutex      sync.RWMutex
	enabled    bool
	threshold  int
}

// NewJITCompiler returns an initialized compiler instance.
func NewJITCompiler() *JITCompiler {
	return &JITCompiler{
		hotPaths:   make(map[int]*HotPath),
		execCounts: make(map[int]int),
		enabled:    true,
		threshold:  JITThreshold,
	}
}

// SetEnabled toggles JIT compilation.
func (jit *JITCompiler) SetEnabled(enabled bool) {
	jit.mutex.Lock()
	defer jit.mutex.Unlock()
	jit.enabled = enabled
}

// RecordExecution updates counters for a bytecode instruction and returns
// true when a new hot path should be compiled.
func (jit *JITCompiler) RecordExecution(ip int) bool {
	if !jit.enabled {
		return false
	}

	jit.mutex.Lock()
	defer jit.mutex.Unlock()

	count := jit.execCounts[ip] + 1
	jit.execCounts[ip] = count

	if hp, ok := jit.hotPaths[ip]; ok {
		hp.ExecuteCount = count
	}

	if count < jit.threshold {
		return false
	}

	if _, exists := jit.hotPaths[ip]; !exists {
		jit.hotPaths[ip] = &HotPath{StartIP: ip, ExecuteCount: count}
		return true
	}

	return false
}

// ShouldJITCompile reports whether the instruction pointer qualifies for JIT.
func (jit *JITCompiler) ShouldJITCompile(ip int) (*HotPath, bool) {
	jit.mutex.RLock()
	defer jit.mutex.RUnlock()

	hp, ok := jit.hotPaths[ip]
	if !ok || hp.IsCompiled() {
		return nil, false
	}
	return hp, true
}

// CompileHotPath performs a minimal "compilation" step by slicing bytecode.
func (jit *JITCompiler) CompileHotPath(hotPath *HotPath, instructions code.Instructions) error {
	jit.mutex.Lock()
	defer jit.mutex.Unlock()

	if hotPath == nil {
		return nil
	}
	if count, ok := jit.execCounts[hotPath.StartIP]; ok {
		hotPath.ExecuteCount = count
	}

	endIP := jit.findHotPathEnd(hotPath.StartIP, instructions)
	if endIP < hotPath.StartIP {
		endIP = hotPath.StartIP
	}
	hotPath.EndIP = endIP
	if endIP > hotPath.StartIP && endIP <= len(instructions) {
		hotPath.Instructions = instructions[hotPath.StartIP:endIP]
	}

	hotPath.markCompiled()
	return nil
}

func (jit *JITCompiler) findHotPathEnd(startIP int, instructions code.Instructions) int {
	ip := startIP
	limit := startIP + 50
	for ip < len(instructions) {
		op := code.Opcode(instructions[ip])
		switch op {
		case code.OpJump, code.OpJumpNotTruthy:
			return ip + 3
		case code.OpPrint:
			return ip + 3
		}

		if def, ok := code.Lookup(op); ok {
			ip++
			for _, width := range def.OperandWidths {
				ip += width
			}
		} else {
			ip++
		}

		if ip >= limit {
			break
		}
	}
	return ip
}

// ExecuteCompiledPath executes the optimized bytecode for a hot path.
func (jit *JITCompiler) ExecuteCompiledPath(hotPath *HotPath, vm *VM) object.Object {
	if hotPath == nil || !hotPath.IsCompiled() || len(hotPath.Instructions) == 0 {
		return nil
	}
	return jit.executeOptimizedBytecode(hotPath, vm)
}

func (jit *JITCompiler) executeOptimizedBytecode(hotPath *HotPath, vm *VM) object.Object {
	savedIP := vm.ip
	for i := 0; i < len(hotPath.Instructions); i++ {
		op := code.Opcode(hotPath.Instructions[i])
		switch op {
		case code.OpConstant:
			if i+2 < len(hotPath.Instructions) {
				operand := uint16(hotPath.Instructions[i+1])<<8 | uint16(hotPath.Instructions[i+2])
				if err := vm.pushChecked(vm.constants[operand]); err != nil {
					vm.ip = savedIP
					return err
				}
				i += 2
			}
		case code.OpAdd, code.OpSub, code.OpMul:
			if err := vm.binaryOp(op); err != nil {
				vm.ip = savedIP
				return err
			}
		default:
			vm.ip = savedIP + hotPath.StartIP + i
			return nil
		}
	}
	vm.ip = savedIP + (hotPath.EndIP - hotPath.StartIP)
	return nil
}

// GetStats reports basic counters for diagnostics.
func (jit *JITCompiler) GetStats() map[string]interface{} {
	jit.mutex.RLock()
	defer jit.mutex.RUnlock()

	compiledPaths := 0
	for _, hp := range jit.hotPaths {
		if hp.IsCompiled() {
			compiledPaths++
		}
	}

	return map[string]interface{}{
		"enabled":        jit.enabled,
		"threshold":      jit.threshold,
		"hot_paths":      len(jit.hotPaths),
		"compiled_paths": compiledPaths,
	}
}

// Reset clears all tracked counters and hot paths.
func (jit *JITCompiler) Reset() {
	jit.mutex.Lock()
	defer jit.mutex.Unlock()

	jit.hotPaths = make(map[int]*HotPath)
	jit.execCounts = make(map[int]int)
}
