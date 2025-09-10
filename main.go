// main.go

package main

import (
	"bufio"
	"darix/interpreter"
	"darix/lexer"
	"darix/object"
	"darix/parser"
	"fmt"
	"os"
	"strings"
)

func main() {
	if len(os.Args) > 1 {
		runFile(os.Args[1])
	} else {
		startRepl()
	}
}

func runFile(filename string) {
	content, err := os.ReadFile(filename)
	if err != nil {
		fmt.Printf("Error reading file: %s\n", err)
		os.Exit(1)
	}

	l := lexer.New(string(content))
	p := parser.New(l)
	program := p.ParseProgram()

	if len(p.Errors()) != 0 {
		for _, msg := range p.Errors() {
			fmt.Printf("Parse error: %s\n", msg)
		}
		os.Exit(1)
	}

	inter := interpreter.New()
	result := inter.Interpret(program)
	if result != nil && result.Type() == object.ERROR_OBJ {
		fmt.Printf("Runtime error: %s\n", result.Inspect())
		os.Exit(1)
	}
}

func startRepl() {
	scanner := bufio.NewScanner(os.Stdin)
	inter := interpreter.New()
	fmt.Println("DariX Language REPL")
	fmt.Println("Type 'exit' to quit")
	var buffer strings.Builder

	var parenCount, braceCount, bracketCount int

	for {
		fmt.Print(">>> ")
		scanner.Scan()
		line := scanner.Text()
		trimmedLine := strings.TrimSpace(line)

		if trimmedLine == "exit" && buffer.Len() == 0 && parenCount == 0 && braceCount == 0 && bracketCount == 0 {
			break
		}

		// شمارش گروه‌بندی‌های باز و بسته در خط فعلی
		for _, ch := range line {
			switch ch {
			case '(':
				parenCount++
			case ')':
				parenCount--
			case '{':
				braceCount++
			case '}':
				braceCount--
			case '[':
				bracketCount++
			case ']':
				bracketCount--
			}
		}

		// جمع‌آوری خط در buffer
		buffer.WriteString(line)
		buffer.WriteString("\n") // اضافه کردن خط جدید برای حفظ ساختار

		// اگر هنوز گروه‌بندی‌های بازی وجود دارد، منتظر بقیه ورودی‌ها بمان
		if parenCount > 0 || braceCount > 0 || bracketCount > 0 {
			continue
		}

		// اکنون فرض می‌کنیم که یک دستور کامل جمع‌آوری شده است
		input := buffer.String()
		// ریست کردن buffer و شمارنده‌ها برای دستور بعدی
		buffer.Reset()
		parenCount = 0
		braceCount = 0
		bracketCount = 0

		// لکس و پارس کردن ورودی کامل
		l := lexer.New(input)
		p := parser.New(l)
		p.SetReplMode(true) // فعال کردن حالت REPL
		program := p.ParseProgram()

		// چک کردن خطاها
		if len(p.Errors()) != 0 {
			for _, msg := range p.Errors() {
				// حذف هشدارهای مربوط به پرانتز/براکت ناکامل
				if !strings.Contains(msg, "warning: missing closing") {
					fmt.Printf("Parse error: %s\n", msg)
				}
			}
			continue
		}

		// تفسیر و اجرای برنامه
		result := inter.Interpret(program)
		if result != nil {
			switch result.Type() {
			case object.ERROR_OBJ:
				fmt.Printf("Runtime error: %s\n", result.Inspect())
			case object.NULL_OBJ:
				// خالی
			default:
				fmt.Println(result.Inspect()) // ← این‌جا رشتهٔ برگردانده‌شده را چاپ می‌کند
			}
		}

		// اطمینان از پاک شدن بافر خروجی
		os.Stdout.Sync()
	}
}
