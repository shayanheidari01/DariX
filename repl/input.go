// repl/input.go - Enhanced input handling for DariX REPL

package repl

import (
	"bufio"
	"fmt"
	"io"
	"strings"
)

// InputHandler handles enhanced input with history and completion
type InputHandler struct {
	reader  *bufio.Reader
	writer  io.Writer
	history []string
	histIdx int
}

// NewInputHandler creates a new input handler
func NewInputHandler(reader io.Reader, writer io.Writer) *InputHandler {
	return &InputHandler{
		reader:  bufio.NewReader(reader),
		writer:  writer,
		history: make([]string, 0),
		histIdx: -1,
	}
}

// ReadLine reads a line with basic editing capabilities
func (ih *InputHandler) ReadLine(prompt string) (string, error) {
	fmt.Fprint(ih.writer, prompt)
	
	line, err := ih.reader.ReadString('\n')
	if err != nil {
		return "", err
	}
	
	// Remove trailing newline
	line = strings.TrimRight(line, "\r\n")
	
	return line, nil
}

// AddToHistory adds a line to the command history
func (ih *InputHandler) AddToHistory(line string) {
	line = strings.TrimSpace(line)
	if line == "" {
		return
	}
	
	// Avoid duplicate consecutive entries
	if len(ih.history) > 0 && ih.history[len(ih.history)-1] == line {
		return
	}
	
	ih.history = append(ih.history, line)
	
	// Limit history size to 1000 entries
	if len(ih.history) > 1000 {
		ih.history = ih.history[1:]
	}
	
	ih.histIdx = len(ih.history)
}

// GetHistory returns the command history
func (ih *InputHandler) GetHistory() []string {
	return ih.history
}

// ClearHistory clears the command history
func (ih *InputHandler) ClearHistory() {
	ih.history = make([]string, 0)
	ih.histIdx = -1
}

// Enhanced input handling for platforms that support it
// This is a basic implementation - for full readline functionality,
// you would typically use a library like github.com/chzyer/readline

// ReadLineWithFeatures attempts to provide enhanced input features
func (ih *InputHandler) ReadLineWithFeatures(prompt string, repl *REPL) (string, error) {
	// For now, fall back to basic readline
	// In a full implementation, this would handle:
	// - Arrow key navigation through history
	// - Tab completion
	// - Line editing (Ctrl+A, Ctrl+E, etc.)
	// - Syntax highlighting
	
	return ih.ReadLine(prompt)
}

// ShowCompletions displays completion suggestions
func (ih *InputHandler) ShowCompletions(completions *CompletionResult) {
	if len(completions.Suggestions) == 0 {
		return
	}
	
	fmt.Fprintln(ih.writer)
	
	if len(completions.Suggestions) == 1 {
		fmt.Fprintf(ih.writer, "Completion: %s\n", completions.Suggestions[0])
	} else {
		fmt.Fprintf(ih.writer, "Completions (%d):\n", len(completions.Suggestions))
		
		// Display in columns
		maxCols := 4
		cols := len(completions.Suggestions)
		if cols > maxCols {
			cols = maxCols
		}
		
		for i, suggestion := range completions.Suggestions {
			fmt.Fprintf(ih.writer, "  %-20s", suggestion)
			if (i+1)%cols == 0 || i == len(completions.Suggestions)-1 {
				fmt.Fprintln(ih.writer)
			}
		}
	}
}

// PrintHistory displays the command history
func (ih *InputHandler) PrintHistory(writer io.Writer, maxEntries int) {
	if len(ih.history) == 0 {
		fmt.Fprintln(writer, "No history available")
		return
	}

	fmt.Fprintln(writer, "Command History:")
	
	start := 0
	if maxEntries > 0 && len(ih.history) > maxEntries {
		start = len(ih.history) - maxEntries
		fmt.Fprintf(writer, "   ... (showing last %d)\n", maxEntries)
	}

	for i := start; i < len(ih.history); i++ {
		fmt.Fprintf(writer, "  %3d: %s\n", i+1, ih.history[i])
	}
}

// GetPreviousCommand gets the previous command from history
func (ih *InputHandler) GetPreviousCommand() string {
	if len(ih.history) == 0 {
		return ""
	}
	
	if ih.histIdx > 0 {
		ih.histIdx--
	}
	
	if ih.histIdx >= 0 && ih.histIdx < len(ih.history) {
		return ih.history[ih.histIdx]
	}
	
	return ""
}

// GetNextCommand gets the next command from history
func (ih *InputHandler) GetNextCommand() string {
	if len(ih.history) == 0 {
		return ""
	}
	
	if ih.histIdx < len(ih.history)-1 {
		ih.histIdx++
		return ih.history[ih.histIdx]
	}
	
	ih.histIdx = len(ih.history)
	return ""
}

// ResetHistoryIndex resets the history navigation index
func (ih *InputHandler) ResetHistoryIndex() {
	ih.histIdx = len(ih.history)
}

// IsAtEndOfHistory returns true if we're at the end of history
func (ih *InputHandler) IsAtEndOfHistory() bool {
	return ih.histIdx >= len(ih.history)
}

// IsAtStartOfHistory returns true if we're at the start of history
func (ih *InputHandler) IsAtStartOfHistory() bool {
	return ih.histIdx <= 0
}
