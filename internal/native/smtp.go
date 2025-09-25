package native

import (
	"crypto/tls"
	"darix/object"
	"fmt"
	"net/smtp"
	"strings"
)

func init() {
	Register("smtp", map[string]*object.Builtin{
		"smtp_send_email":     {Fn: smtpSendEmail},
		"smtp_send_html":      {Fn: smtpSendHTML},
		"smtp_send_with_auth": {Fn: smtpSendWithAuth},
		"smtp_test_connection": {Fn: smtpTestConnection},
	})
}

// SMTPConfig represents SMTP server configuration
type SMTPConfig struct {
	Host     string
	Port     int
	Username string
	Password string
	UseTLS   bool
}

// smtpSendEmail sends a plain text email
func smtpSendEmail(args ...object.Object) object.Object {
	if !ModuleAllowed("smtp") {
		return object.NewError("smtp_send_email: access to native module smtp denied by policy")
	}
	if len(args) != 6 {
		return object.NewError("smtp_send_email: expected 6 arguments, got %d", len(args))
	}
	
	// Parse arguments
	host, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("smtp_send_email: first argument must be string, got %s", args[0].Type())
	}
	
	port, ok := args[1].(*object.Integer)
	if !ok {
		return object.NewError("smtp_send_email: second argument must be integer, got %s", args[1].Type())
	}
	
	from, ok := args[2].(*object.String)
	if !ok {
		return object.NewError("smtp_send_email: third argument must be string, got %s", args[2].Type())
	}
	
	to, ok := args[3].(*object.String)
	if !ok {
		return object.NewError("smtp_send_email: fourth argument must be string, got %s", args[3].Type())
	}
	
	subject, ok := args[4].(*object.String)
	if !ok {
		return object.NewError("smtp_send_email: fifth argument must be string, got %s", args[4].Type())
	}
	
	body, ok := args[5].(*object.String)
	if !ok {
		return object.NewError("smtp_send_email: sixth argument must be string, got %s", args[5].Type())
	}
	
	// Create email message
	message := fmt.Sprintf("From: %s\r\nTo: %s\r\nSubject: %s\r\n\r\n%s",
		from.Value, to.Value, subject.Value, body.Value)
	
	// Send email
	addr := fmt.Sprintf("%s:%d", host.Value, port.Value)
	err := smtp.SendMail(addr, nil, from.Value, []string{to.Value}, []byte(message))
	if err != nil {
		return object.NewError("smtp_send_email: failed to send email: %s", err.Error())
	}
	
	result := make(map[object.Object]object.Object)
	result[object.NewString("sent")] = object.TRUE
	result[object.NewString("from")] = object.NewString(from.Value)
	result[object.NewString("to")] = object.NewString(to.Value)
	result[object.NewString("subject")] = object.NewString(subject.Value)
	
	return object.NewMap(result)
}

// smtpSendHTML sends an HTML email
func smtpSendHTML(args ...object.Object) object.Object {
	if !ModuleAllowed("smtp") {
		return object.NewError("smtp_send_html: access to native module smtp denied by policy")
	}
	if len(args) != 6 {
		return object.NewError("smtp_send_html: expected 6 arguments, got %d", len(args))
	}
	
	// Parse arguments (same as smtpSendEmail)
	host, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("smtp_send_html: first argument must be string, got %s", args[0].Type())
	}
	
	port, ok := args[1].(*object.Integer)
	if !ok {
		return object.NewError("smtp_send_html: second argument must be integer, got %s", args[1].Type())
	}
	
	from, ok := args[2].(*object.String)
	if !ok {
		return object.NewError("smtp_send_html: third argument must be string, got %s", args[2].Type())
	}
	
	to, ok := args[3].(*object.String)
	if !ok {
		return object.NewError("smtp_send_html: fourth argument must be string, got %s", args[3].Type())
	}
	
	subject, ok := args[4].(*object.String)
	if !ok {
		return object.NewError("smtp_send_html: fifth argument must be string, got %s", args[4].Type())
	}
	
	htmlBody, ok := args[5].(*object.String)
	if !ok {
		return object.NewError("smtp_send_html: sixth argument must be string, got %s", args[5].Type())
	}
	
	// Create HTML email message
	message := fmt.Sprintf("From: %s\r\nTo: %s\r\nSubject: %s\r\nMIME-Version: 1.0\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n%s",
		from.Value, to.Value, subject.Value, htmlBody.Value)
	
	// Send email
	addr := fmt.Sprintf("%s:%d", host.Value, port.Value)
	err := smtp.SendMail(addr, nil, from.Value, []string{to.Value}, []byte(message))
	if err != nil {
		return object.NewError("smtp_send_html: failed to send email: %s", err.Error())
	}
	
	result := make(map[object.Object]object.Object)
	result[object.NewString("sent")] = object.TRUE
	result[object.NewString("from")] = object.NewString(from.Value)
	result[object.NewString("to")] = object.NewString(to.Value)
	result[object.NewString("subject")] = object.NewString(subject.Value)
	result[object.NewString("type")] = object.NewString("html")
	
	return object.NewMap(result)
}

// smtpSendWithAuth sends email with authentication
func smtpSendWithAuth(args ...object.Object) object.Object {
	if !ModuleAllowed("smtp") {
		return object.NewError("smtp_send_with_auth: access to native module smtp denied by policy")
	}
	if len(args) != 8 {
		return object.NewError("smtp_send_with_auth: expected 8 arguments, got %d", len(args))
	}
	
	// Parse arguments
	host, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("smtp_send_with_auth: first argument must be string, got %s", args[0].Type())
	}
	
	port, ok := args[1].(*object.Integer)
	if !ok {
		return object.NewError("smtp_send_with_auth: second argument must be integer, got %s", args[1].Type())
	}
	
	username, ok := args[2].(*object.String)
	if !ok {
		return object.NewError("smtp_send_with_auth: third argument must be string, got %s", args[2].Type())
	}
	
	password, ok := args[3].(*object.String)
	if !ok {
		return object.NewError("smtp_send_with_auth: fourth argument must be string, got %s", args[3].Type())
	}
	
	from, ok := args[4].(*object.String)
	if !ok {
		return object.NewError("smtp_send_with_auth: fifth argument must be string, got %s", args[4].Type())
	}
	
	to, ok := args[5].(*object.String)
	if !ok {
		return object.NewError("smtp_send_with_auth: sixth argument must be string, got %s", args[5].Type())
	}
	
	subject, ok := args[6].(*object.String)
	if !ok {
		return object.NewError("smtp_send_with_auth: seventh argument must be string, got %s", args[6].Type())
	}
	
	body, ok := args[7].(*object.String)
	if !ok {
		return object.NewError("smtp_send_with_auth: eighth argument must be string, got %s", args[7].Type())
	}
	
	// Create email message
	message := fmt.Sprintf("From: %s\r\nTo: %s\r\nSubject: %s\r\n\r\n%s",
		from.Value, to.Value, subject.Value, body.Value)
	
	// Set up authentication
	auth := smtp.PlainAuth("", username.Value, password.Value, host.Value)
	
	// Send email with authentication
	addr := fmt.Sprintf("%s:%d", host.Value, port.Value)
	
	// Parse recipients (support multiple recipients separated by comma)
	recipients := strings.Split(to.Value, ",")
	for i, recipient := range recipients {
		recipients[i] = strings.TrimSpace(recipient)
	}
	
	err := smtp.SendMail(addr, auth, from.Value, recipients, []byte(message))
	if err != nil {
		return object.NewError("smtp_send_with_auth: failed to send email: %s", err.Error())
	}
	
	result := make(map[object.Object]object.Object)
	result[object.NewString("sent")] = object.TRUE
	result[object.NewString("from")] = object.NewString(from.Value)
	result[object.NewString("to")] = object.NewString(to.Value)
	result[object.NewString("subject")] = object.NewString(subject.Value)
	result[object.NewString("recipients")] = object.NewInteger(int64(len(recipients)))
	result[object.NewString("authenticated")] = object.TRUE
	
	return object.NewMap(result)
}

// smtpTestConnection tests SMTP server connection
func smtpTestConnection(args ...object.Object) object.Object {
	if !ModuleAllowed("smtp") {
		return object.NewError("smtp_test_connection: access to native module smtp denied by policy")
	}
	if len(args) < 2 || len(args) > 5 {
		return object.NewError("smtp_test_connection: expected 2-5 arguments, got %d", len(args))
	}
	
	// Parse arguments
	host, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("smtp_test_connection: first argument must be string, got %s", args[0].Type())
	}
	
	port, ok := args[1].(*object.Integer)
	if !ok {
		return object.NewError("smtp_test_connection: second argument must be integer, got %s", args[1].Type())
	}
	
	var username, password string
	var useTLS bool
	
	if len(args) >= 3 {
		if usernameObj, ok := args[2].(*object.String); ok {
			username = usernameObj.Value
		}
	}
	
	if len(args) >= 4 {
		if passwordObj, ok := args[3].(*object.String); ok {
			password = passwordObj.Value
		}
	}
	
	if len(args) == 5 {
		if tlsObj, ok := args[4].(*object.Boolean); ok {
			useTLS = tlsObj.Value
		}
	}
	
	addr := fmt.Sprintf("%s:%d", host.Value, port.Value)
	
	// Test connection
	var client *smtp.Client
	var err error
	
	if useTLS {
		// Connect with TLS
		tlsConfig := &tls.Config{
			ServerName: host.Value,
		}
		conn, err := tls.Dial("tcp", addr, tlsConfig)
		if err != nil {
			return object.NewError("smtp_test_connection: TLS connection failed: %s", err.Error())
		}
		defer conn.Close()
		
		client, err = smtp.NewClient(conn, host.Value)
		if err != nil {
			return object.NewError("smtp_test_connection: SMTP client creation failed: %s", err.Error())
		}
	} else {
		// Connect without TLS
		client, err = smtp.Dial(addr)
		if err != nil {
			return object.NewError("smtp_test_connection: connection failed: %s", err.Error())
		}
	}
	defer client.Close()
	
	// Test authentication if credentials provided
	var authSuccess bool
	if username != "" && password != "" {
		auth := smtp.PlainAuth("", username, password, host.Value)
		err = client.Auth(auth)
		if err != nil {
			authSuccess = false
		} else {
			authSuccess = true
		}
	}
	
	result := make(map[object.Object]object.Object)
	result[object.NewString("connected")] = object.TRUE
	result[object.NewString("host")] = object.NewString(host.Value)
	result[object.NewString("port")] = object.NewInteger(port.Value)
	result[object.NewString("tls")] = object.NewBoolean(useTLS)
	
	if username != "" {
		result[object.NewString("auth_tested")] = object.TRUE
		result[object.NewString("auth_success")] = object.NewBoolean(authSuccess)
	} else {
		result[object.NewString("auth_tested")] = object.FALSE
	}
	
	return object.NewMap(result)
}
