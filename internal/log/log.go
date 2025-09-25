package log

import (
	"fmt"
	"io"
	"os"
	"sync"
	"time"
)

// Level represents structured log levels packaged in increasing severity order.
type Level int

const (
	LevelDebug Level = iota
	LevelInfo
	LevelWarn
	LevelError
)

// Fields is a lightweight map used to attach structured metadata to log events.
type Fields map[string]any

// Event captures a single log entry.
type Event struct {
	Timestamp time.Time
	Level     Level
	Message   string
	Fields    Fields
}

// Handler processes log events. Implementations may write to stdout/stderr,
// forward to files, or export to a diagnostics system.
type Handler interface {
	Handle(Event) error
}

// Logger provides structured logging with leveled output and shared fields.
type Logger struct {
	mu      sync.RWMutex
	level   Level
	fields  Fields
	handler Handler
}

// New creates a Logger with the provided minimum level and handler. Fields may
// be nil; a copy is stored internally.
func New(level Level, handler Handler, fields Fields) *Logger {
	l := &Logger{level: level, handler: handler}
	if fields != nil {
		l.fields = clone(fields)
	} else {
		l.fields = Fields{}
	}
	return l
}

// clone performs a shallow copy of fields to keep Logger state immutable.
func clone(fields Fields) Fields {
	out := make(Fields, len(fields))
	for k, v := range fields {
		out[k] = v
	}
	return out
}

// WithFields returns a child logger enriched with additional fields.
func (l *Logger) WithFields(fields Fields) *Logger {
	child := &Logger{
		level:   l.Level(),
		handler: l.Handler(),
		fields:  clone(l.Fields()),
	}
	for k, v := range fields {
		child.fields[k] = v
	}
	return child
}

// Level returns the current minimum level.
func (l *Logger) Level() Level {
	l.mu.RLock()
	defer l.mu.RUnlock()
	return l.level
}

// Fields returns a copy of the logger's base fields.
func (l *Logger) Fields() Fields {
	l.mu.RLock()
	defer l.mu.RUnlock()
	return clone(l.fields)
}

// Handler returns the installed handler.
func (l *Logger) Handler() Handler {
	l.mu.RLock()
	defer l.mu.RUnlock()
	return l.handler
}

// SetLevel updates the minimum log level.
func (l *Logger) SetLevel(level Level) {
	l.mu.Lock()
	l.level = level
	l.mu.Unlock()
}

// SetHandler updates the handler used to process events.
func (l *Logger) SetHandler(handler Handler) {
	l.mu.Lock()
	l.handler = handler
	l.mu.Unlock()
}

// Emit logs an event with the given level, message, and fields.
func (l *Logger) Emit(level Level, msg string, fields Fields) {
	if level < l.Level() {
		return
	}
	event := Event{
		Timestamp: time.Now(),
		Level:     level,
		Message:   msg,
		Fields:    clone(l.Fields()),
	}
	for k, v := range fields {
		event.Fields[k] = v
	}
	if handler := l.Handler(); handler != nil {
		_ = handler.Handle(event)
	}
}

// Convenience methods ------------------------------------------------------

func (l *Logger) Debug(msg string, fields Fields) { l.Emit(LevelDebug, msg, fields) }
func (l *Logger) Info(msg string, fields Fields)  { l.Emit(LevelInfo, msg, fields) }
func (l *Logger) Warn(msg string, fields Fields)  { l.Emit(LevelWarn, msg, fields) }
func (l *Logger) Error(msg string, fields Fields) { l.Emit(LevelError, msg, fields) }

// Global logger ------------------------------------------------------------

var (
	globalMu sync.RWMutex
	global   = New(LevelInfo, &WriterHandler{Writer: os.Stderr}, nil)
)

// SetGlobal swaps the global logger instance used by convenience helpers.
func SetGlobal(logger *Logger) {
	globalMu.Lock()
	global = logger
	globalMu.Unlock()
}

func Global() *Logger {
	globalMu.RLock()
	defer globalMu.RUnlock()
	return global
}

func Debug(msg string, fields Fields) { Global().Debug(msg, fields) }
func Info(msg string, fields Fields)  { Global().Info(msg, fields) }
func Warn(msg string, fields Fields)  { Global().Warn(msg, fields) }
func Error(msg string, fields Fields) { Global().Error(msg, fields) }

// WriterHandler is a simple handler that writes events to an io.Writer.
type WriterHandler struct {
	Writer io.Writer
}

func (h *WriterHandler) Handle(event Event) error {
	if h.Writer == nil {
		return nil
	}
	_, err := fmt.Fprintf(h.Writer, "%s [%s] %s %v\n",
		event.Timestamp.Format(time.RFC3339),
		levelString(event.Level),
		event.Message,
		event.Fields,
	)
	return err
}

func levelString(level Level) string {
	switch level {
	case LevelDebug:
		return "DEBUG"
	case LevelInfo:
		return "INFO"
	case LevelWarn:
		return "WARN"
	case LevelError:
		return "ERROR"
	default:
		return "UNKNOWN"
	}
}
