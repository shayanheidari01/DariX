package native

import (
	"darix/object"
	"fmt"
	"net/http"
	"net/url"
	"sync"
	"time"

	"github.com/gorilla/websocket"
)

func init() {
	Register("websocket", map[string]*object.Builtin{
		"ws_connect":      {Fn: wsConnect},
		"ws_send":         {Fn: wsSend},
		"ws_receive":      {Fn: wsReceive},
		"ws_close":        {Fn: wsClose},
		"ws_ping":         {Fn: wsPing},
		"ws_is_connected": {Fn: wsIsConnected},
		"ws_set_timeout":  {Fn: wsSetTimeout},
	})
}

// WebSocket connection storage
var (
	wsConnections = make(map[string]*websocket.Conn)
	wsCounter     = 0
	wsMutex       sync.RWMutex
)

// generateWSID generates a unique WebSocket ID
func generateWSID() string {
	wsCounter++
	return fmt.Sprintf("ws_%d", wsCounter)
}

// wsConnect connects to a WebSocket server
func wsConnect(args ...object.Object) object.Object {
	if !ModuleAllowed("websocket") {
		return object.NewError("ws_connect: access to native module websocket denied by policy")
	}
	if len(args) < 1 || len(args) > 2 {
		return object.NewError("ws_connect: expected 1-2 arguments, got %d", len(args))
	}
	
	urlStr, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("ws_connect: first argument must be string, got %s", args[0].Type())
	}
	
	// Parse URL
	u, err := url.Parse(urlStr.Value)
	if err != nil {
		return object.NewError("ws_connect: invalid URL: %s", err.Error())
	}
	
	// Set up headers
	headers := http.Header{}
	if len(args) == 2 {
		if headersObj, ok := args[1].(*object.Map); ok {
			for key, value := range headersObj.Pairs {
				if keyStr, ok := key.(*object.String); ok {
					if valueStr, ok := value.(*object.String); ok {
						headers.Set(keyStr.Value, valueStr.Value)
					}
				}
			}
		}
	}
	
	// Create dialer with timeout
	dialer := websocket.Dialer{
		HandshakeTimeout: 10 * time.Second,
	}
	
	// Connect to WebSocket
	conn, _, err := dialer.Dial(u.String(), headers)
	if err != nil {
		return object.NewError("ws_connect: failed to connect: %s", err.Error())
	}
	
	// Store connection
	wsID := generateWSID()
	wsMutex.Lock()
	wsConnections[wsID] = conn
	wsMutex.Unlock()
	
	result := make(map[object.Object]object.Object)
	result[object.NewString("ws_id")] = object.NewString(wsID)
	result[object.NewString("url")] = object.NewString(u.String())
	result[object.NewString("connected")] = object.TRUE
	
	return object.NewMap(result)
}

// wsSend sends a message to WebSocket
func wsSend(args ...object.Object) object.Object {
	if !ModuleAllowed("websocket") {
		return object.NewError("ws_send: access to native module websocket denied by policy")
	}
	if len(args) < 2 || len(args) > 3 {
		return object.NewError("ws_send: expected 2-3 arguments, got %d", len(args))
	}
	
	wsID, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("ws_send: first argument must be string, got %s", args[0].Type())
	}
	
	message, ok := args[1].(*object.String)
	if !ok {
		return object.NewError("ws_send: second argument must be string, got %s", args[1].Type())
	}
	
	messageType := websocket.TextMessage
	if len(args) == 3 {
		if typeObj, ok := args[2].(*object.String); ok {
			switch typeObj.Value {
			case "text":
				messageType = websocket.TextMessage
			case "binary":
				messageType = websocket.BinaryMessage
			case "ping":
				messageType = websocket.PingMessage
			case "pong":
				messageType = websocket.PongMessage
			default:
				return object.NewError("ws_send: invalid message type: %s", typeObj.Value)
			}
		}
	}
	
	wsMutex.RLock()
	conn, exists := wsConnections[wsID.Value]
	wsMutex.RUnlock()
	
	if !exists {
		return object.NewError("ws_send: connection not found")
	}
	
	err := conn.WriteMessage(messageType, []byte(message.Value))
	if err != nil {
		return object.NewError("ws_send: failed to send message: %s", err.Error())
	}
	
	return object.TRUE
}

// wsReceive receives a message from WebSocket
func wsReceive(args ...object.Object) object.Object {
	if !ModuleAllowed("websocket") {
		return object.NewError("ws_receive: access to native module websocket denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("ws_receive: expected 1 argument, got %d", len(args))
	}
	
	wsID, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("ws_receive: argument must be string, got %s", args[0].Type())
	}
	
	wsMutex.RLock()
	conn, exists := wsConnections[wsID.Value]
	wsMutex.RUnlock()
	
	if !exists {
		return object.NewError("ws_receive: connection not found")
	}
	
	messageType, message, err := conn.ReadMessage()
	if err != nil {
		return object.NewError("ws_receive: failed to receive message: %s", err.Error())
	}
	
	var msgTypeStr string
	switch messageType {
	case websocket.TextMessage:
		msgTypeStr = "text"
	case websocket.BinaryMessage:
		msgTypeStr = "binary"
	case websocket.PingMessage:
		msgTypeStr = "ping"
	case websocket.PongMessage:
		msgTypeStr = "pong"
	case websocket.CloseMessage:
		msgTypeStr = "close"
	default:
		msgTypeStr = "unknown"
	}
	
	result := make(map[object.Object]object.Object)
	result[object.NewString("message")] = object.NewString(string(message))
	result[object.NewString("type")] = object.NewString(msgTypeStr)
	result[object.NewString("length")] = object.NewInteger(int64(len(message)))
	
	return object.NewMap(result)
}

// wsClose closes a WebSocket connection
func wsClose(args ...object.Object) object.Object {
	if !ModuleAllowed("websocket") {
		return object.NewError("ws_close: access to native module websocket denied by policy")
	}
	if len(args) < 1 || len(args) > 2 {
		return object.NewError("ws_close: expected 1-2 arguments, got %d", len(args))
	}
	
	wsID, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("ws_close: first argument must be string, got %s", args[0].Type())
	}
	
	closeCode := websocket.CloseNormalClosure
	closeMessage := ""
	
	if len(args) == 2 {
		if msgObj, ok := args[1].(*object.String); ok {
			closeMessage = msgObj.Value
		}
	}
	
	wsMutex.Lock()
	conn, exists := wsConnections[wsID.Value]
	if exists {
		delete(wsConnections, wsID.Value)
	}
	wsMutex.Unlock()
	
	if !exists {
		return object.NewError("ws_close: connection not found")
	}
	
	// Send close message
	err := conn.WriteMessage(websocket.CloseMessage, 
		websocket.FormatCloseMessage(closeCode, closeMessage))
	if err != nil {
		// Ignore error, connection might already be closed
	}
	
	// Close the connection
	err = conn.Close()
	if err != nil {
		return object.NewError("ws_close: failed to close connection: %s", err.Error())
	}
	
	return object.TRUE
}

// wsPing sends a ping message
func wsPing(args ...object.Object) object.Object {
	if !ModuleAllowed("websocket") {
		return object.NewError("ws_ping: access to native module websocket denied by policy")
	}
	if len(args) < 1 || len(args) > 2 {
		return object.NewError("ws_ping: expected 1-2 arguments, got %d", len(args))
	}
	
	wsID, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("ws_ping: first argument must be string, got %s", args[0].Type())
	}
	
	data := []byte{}
	if len(args) == 2 {
		if dataObj, ok := args[1].(*object.String); ok {
			data = []byte(dataObj.Value)
		}
	}
	
	wsMutex.RLock()
	conn, exists := wsConnections[wsID.Value]
	wsMutex.RUnlock()
	
	if !exists {
		return object.NewError("ws_ping: connection not found")
	}
	
	err := conn.WriteMessage(websocket.PingMessage, data)
	if err != nil {
		return object.NewError("ws_ping: failed to send ping: %s", err.Error())
	}
	
	return object.TRUE
}

// wsIsConnected checks if WebSocket is connected
func wsIsConnected(args ...object.Object) object.Object {
	if !ModuleAllowed("websocket") {
		return object.NewError("ws_is_connected: access to native module websocket denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("ws_is_connected: expected 1 argument, got %d", len(args))
	}
	
	wsID, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("ws_is_connected: argument must be string, got %s", args[0].Type())
	}
	
	wsMutex.RLock()
	_, exists := wsConnections[wsID.Value]
	wsMutex.RUnlock()
	
	return object.NewBoolean(exists)
}

// wsSetTimeout sets read/write timeout for WebSocket
func wsSetTimeout(args ...object.Object) object.Object {
	if !ModuleAllowed("websocket") {
		return object.NewError("ws_set_timeout: access to native module websocket denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("ws_set_timeout: expected 2 arguments, got %d", len(args))
	}
	
	wsID, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("ws_set_timeout: first argument must be string, got %s", args[0].Type())
	}
	
	timeout, ok := args[1].(*object.Integer)
	if !ok {
		return object.NewError("ws_set_timeout: second argument must be integer, got %s", args[1].Type())
	}
	
	wsMutex.RLock()
	conn, exists := wsConnections[wsID.Value]
	wsMutex.RUnlock()
	
	if !exists {
		return object.NewError("ws_set_timeout: connection not found")
	}
	
	duration := time.Duration(timeout.Value) * time.Second
	
	// Set read deadline
	err := conn.SetReadDeadline(time.Now().Add(duration))
	if err != nil {
		return object.NewError("ws_set_timeout: failed to set read timeout: %s", err.Error())
	}
	
	// Set write deadline
	err = conn.SetWriteDeadline(time.Now().Add(duration))
	if err != nil {
		return object.NewError("ws_set_timeout: failed to set write timeout: %s", err.Error())
	}
	
	return object.TRUE
}
