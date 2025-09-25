package native

import (
	"darix/object"
	"fmt"
	"net"
	"strconv"
	"time"
)

func init() {
	Register("socket", map[string]*object.Builtin{
		"socket_tcp_connect":    {Fn: socketTCPConnect},
		"socket_tcp_listen":     {Fn: socketTCPListen},
		"socket_tcp_accept":     {Fn: socketTCPAccept},
		"socket_tcp_send":       {Fn: socketTCPSend},
		"socket_tcp_receive":    {Fn: socketTCPReceive},
		"socket_tcp_close":      {Fn: socketTCPClose},
		"socket_udp_connect":    {Fn: socketUDPConnect},
		"socket_udp_listen":     {Fn: socketUDPListen},
		"socket_udp_send":       {Fn: socketUDPSend},
		"socket_udp_receive":    {Fn: socketUDPReceive},
		"socket_udp_close":      {Fn: socketUDPClose},
		"socket_set_timeout":    {Fn: socketSetTimeout},
		"socket_get_local_addr": {Fn: socketGetLocalAddr},
		"socket_get_remote_addr": {Fn: socketGetRemoteAddr},
	})
}

// Global socket storage
var (
	tcpConnections = make(map[string]net.Conn)
	tcpListeners   = make(map[string]net.Listener)
	udpConnections = make(map[string]*net.UDPConn)
	socketCounter  = 0
)

// generateSocketID generates a unique socket ID
func generateSocketID() string {
	socketCounter++
	return fmt.Sprintf("socket_%d", socketCounter)
}

// socketTCPConnect creates a TCP connection
func socketTCPConnect(args ...object.Object) object.Object {
	if !ModuleAllowed("socket") {
		return object.NewError("socket_tcp_connect: access to native module socket denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("socket_tcp_connect: expected 2 arguments, got %d", len(args))
	}
	
	host, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("socket_tcp_connect: first argument must be string, got %s", args[0].Type())
	}
	
	port, ok := args[1].(*object.Integer)
	if !ok {
		return object.NewError("socket_tcp_connect: second argument must be integer, got %s", args[1].Type())
	}
	
	address := net.JoinHostPort(host.Value, strconv.Itoa(int(port.Value)))
	conn, err := net.Dial("tcp", address)
	if err != nil {
		return object.NewError("socket_tcp_connect: failed to connect: %s", err.Error())
	}
	
	socketID := generateSocketID()
	tcpConnections[socketID] = conn
	
	result := make(map[object.Object]object.Object)
	result[object.NewString("socket_id")] = object.NewString(socketID)
	result[object.NewString("local_addr")] = object.NewString(conn.LocalAddr().String())
	result[object.NewString("remote_addr")] = object.NewString(conn.RemoteAddr().String())
	
	return object.NewMap(result)
}

// socketTCPListen creates a TCP listener
func socketTCPListen(args ...object.Object) object.Object {
	if !ModuleAllowed("socket") {
		return object.NewError("socket_tcp_listen: access to native module socket denied by policy")
	}
	if len(args) < 1 || len(args) > 2 {
		return object.NewError("socket_tcp_listen: expected 1-2 arguments, got %d", len(args))
	}
	
	port, ok := args[0].(*object.Integer)
	if !ok {
		return object.NewError("socket_tcp_listen: first argument must be integer, got %s", args[0].Type())
	}
	
	host := "0.0.0.0"
	if len(args) == 2 {
		if hostObj, ok := args[1].(*object.String); ok {
			host = hostObj.Value
		}
	}
	
	address := net.JoinHostPort(host, strconv.Itoa(int(port.Value)))
	listener, err := net.Listen("tcp", address)
	if err != nil {
		return object.NewError("socket_tcp_listen: failed to listen: %s", err.Error())
	}
	
	socketID := generateSocketID()
	tcpListeners[socketID] = listener
	
	result := make(map[object.Object]object.Object)
	result[object.NewString("socket_id")] = object.NewString(socketID)
	result[object.NewString("address")] = object.NewString(listener.Addr().String())
	
	return object.NewMap(result)
}

// socketTCPAccept accepts a TCP connection
func socketTCPAccept(args ...object.Object) object.Object {
	if !ModuleAllowed("socket") {
		return object.NewError("socket_tcp_accept: access to native module socket denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("socket_tcp_accept: expected 1 argument, got %d", len(args))
	}
	
	socketID, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("socket_tcp_accept: argument must be string, got %s", args[0].Type())
	}
	
	listener, exists := tcpListeners[socketID.Value]
	if !exists {
		return object.NewError("socket_tcp_accept: listener not found")
	}
	
	conn, err := listener.Accept()
	if err != nil {
		return object.NewError("socket_tcp_accept: failed to accept: %s", err.Error())
	}
	
	connID := generateSocketID()
	tcpConnections[connID] = conn
	
	result := make(map[object.Object]object.Object)
	result[object.NewString("socket_id")] = object.NewString(connID)
	result[object.NewString("local_addr")] = object.NewString(conn.LocalAddr().String())
	result[object.NewString("remote_addr")] = object.NewString(conn.RemoteAddr().String())
	
	return object.NewMap(result)
}

// socketTCPSend sends data over TCP connection
func socketTCPSend(args ...object.Object) object.Object {
	if !ModuleAllowed("socket") {
		return object.NewError("socket_tcp_send: access to native module socket denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("socket_tcp_send: expected 2 arguments, got %d", len(args))
	}
	
	socketID, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("socket_tcp_send: first argument must be string, got %s", args[0].Type())
	}
	
	data, ok := args[1].(*object.String)
	if !ok {
		return object.NewError("socket_tcp_send: second argument must be string, got %s", args[1].Type())
	}
	
	conn, exists := tcpConnections[socketID.Value]
	if !exists {
		return object.NewError("socket_tcp_send: connection not found")
	}
	
	n, err := conn.Write([]byte(data.Value))
	if err != nil {
		return object.NewError("socket_tcp_send: failed to send: %s", err.Error())
	}
	
	return object.NewInteger(int64(n))
}

// socketTCPReceive receives data from TCP connection
func socketTCPReceive(args ...object.Object) object.Object {
	if !ModuleAllowed("socket") {
		return object.NewError("socket_tcp_receive: access to native module socket denied by policy")
	}
	if len(args) < 1 || len(args) > 2 {
		return object.NewError("socket_tcp_receive: expected 1-2 arguments, got %d", len(args))
	}
	
	socketID, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("socket_tcp_receive: first argument must be string, got %s", args[0].Type())
	}
	
	bufferSize := 1024
	if len(args) == 2 {
		if sizeObj, ok := args[1].(*object.Integer); ok {
			bufferSize = int(sizeObj.Value)
		}
	}
	
	conn, exists := tcpConnections[socketID.Value]
	if !exists {
		return object.NewError("socket_tcp_receive: connection not found")
	}
	
	buffer := make([]byte, bufferSize)
	n, err := conn.Read(buffer)
	if err != nil {
		return object.NewError("socket_tcp_receive: failed to receive: %s", err.Error())
	}
	
	result := make(map[object.Object]object.Object)
	result[object.NewString("data")] = object.NewString(string(buffer[:n]))
	result[object.NewString("bytes_read")] = object.NewInteger(int64(n))
	
	return object.NewMap(result)
}

// socketTCPClose closes a TCP connection or listener
func socketTCPClose(args ...object.Object) object.Object {
	if !ModuleAllowed("socket") {
		return object.NewError("socket_tcp_close: access to native module socket denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("socket_tcp_close: expected 1 argument, got %d", len(args))
	}
	
	socketID, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("socket_tcp_close: argument must be string, got %s", args[0].Type())
	}
	
	// Try to close connection first
	if conn, exists := tcpConnections[socketID.Value]; exists {
		err := conn.Close()
		delete(tcpConnections, socketID.Value)
		if err != nil {
			return object.NewError("socket_tcp_close: failed to close connection: %s", err.Error())
		}
		return object.TRUE
	}
	
	// Try to close listener
	if listener, exists := tcpListeners[socketID.Value]; exists {
		err := listener.Close()
		delete(tcpListeners, socketID.Value)
		if err != nil {
			return object.NewError("socket_tcp_close: failed to close listener: %s", err.Error())
		}
		return object.TRUE
	}
	
	return object.NewError("socket_tcp_close: socket not found")
}

// socketUDPConnect creates a UDP connection
func socketUDPConnect(args ...object.Object) object.Object {
	if !ModuleAllowed("socket") {
		return object.NewError("socket_udp_connect: access to native module socket denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("socket_udp_connect: expected 2 arguments, got %d", len(args))
	}
	
	host, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("socket_udp_connect: first argument must be string, got %s", args[0].Type())
	}
	
	port, ok := args[1].(*object.Integer)
	if !ok {
		return object.NewError("socket_udp_connect: second argument must be integer, got %s", args[1].Type())
	}
	
	address := net.JoinHostPort(host.Value, strconv.Itoa(int(port.Value)))
	udpAddr, err := net.ResolveUDPAddr("udp", address)
	if err != nil {
		return object.NewError("socket_udp_connect: failed to resolve address: %s", err.Error())
	}
	
	conn, err := net.DialUDP("udp", nil, udpAddr)
	if err != nil {
		return object.NewError("socket_udp_connect: failed to connect: %s", err.Error())
	}
	
	socketID := generateSocketID()
	udpConnections[socketID] = conn
	
	result := make(map[object.Object]object.Object)
	result[object.NewString("socket_id")] = object.NewString(socketID)
	result[object.NewString("local_addr")] = object.NewString(conn.LocalAddr().String())
	result[object.NewString("remote_addr")] = object.NewString(conn.RemoteAddr().String())
	
	return object.NewMap(result)
}

// socketUDPListen creates a UDP listener
func socketUDPListen(args ...object.Object) object.Object {
	if !ModuleAllowed("socket") {
		return object.NewError("socket_udp_listen: access to native module socket denied by policy")
	}
	if len(args) < 1 || len(args) > 2 {
		return object.NewError("socket_udp_listen: expected 1-2 arguments, got %d", len(args))
	}
	
	port, ok := args[0].(*object.Integer)
	if !ok {
		return object.NewError("socket_udp_listen: first argument must be integer, got %s", args[0].Type())
	}
	
	host := "0.0.0.0"
	if len(args) == 2 {
		if hostObj, ok := args[1].(*object.String); ok {
			host = hostObj.Value
		}
	}
	
	address := net.JoinHostPort(host, strconv.Itoa(int(port.Value)))
	udpAddr, err := net.ResolveUDPAddr("udp", address)
	if err != nil {
		return object.NewError("socket_udp_listen: failed to resolve address: %s", err.Error())
	}
	
	conn, err := net.ListenUDP("udp", udpAddr)
	if err != nil {
		return object.NewError("socket_udp_listen: failed to listen: %s", err.Error())
	}
	
	socketID := generateSocketID()
	udpConnections[socketID] = conn
	
	result := make(map[object.Object]object.Object)
	result[object.NewString("socket_id")] = object.NewString(socketID)
	result[object.NewString("address")] = object.NewString(conn.LocalAddr().String())
	
	return object.NewMap(result)
}

// socketUDPSend sends data over UDP
func socketUDPSend(args ...object.Object) object.Object {
	if !ModuleAllowed("socket") {
		return object.NewError("socket_udp_send: access to native module socket denied by policy")
	}
	if len(args) < 2 || len(args) > 4 {
		return object.NewError("socket_udp_send: expected 2-4 arguments, got %d", len(args))
	}
	
	socketID, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("socket_udp_send: first argument must be string, got %s", args[0].Type())
	}
	
	data, ok := args[1].(*object.String)
	if !ok {
		return object.NewError("socket_udp_send: second argument must be string, got %s", args[1].Type())
	}
	
	conn, exists := udpConnections[socketID.Value]
	if !exists {
		return object.NewError("socket_udp_send: connection not found")
	}
	
	var n int
	var err error
	
	// If host and port are provided, send to specific address
	if len(args) >= 4 {
		host, ok := args[2].(*object.String)
		if !ok {
			return object.NewError("socket_udp_send: third argument must be string, got %s", args[2].Type())
		}
		
		port, ok := args[3].(*object.Integer)
		if !ok {
			return object.NewError("socket_udp_send: fourth argument must be integer, got %s", args[3].Type())
		}
		
		address := net.JoinHostPort(host.Value, strconv.Itoa(int(port.Value)))
		udpAddr, err := net.ResolveUDPAddr("udp", address)
		if err != nil {
			return object.NewError("socket_udp_send: failed to resolve address: %s", err.Error())
		}
		
		n, err = conn.WriteToUDP([]byte(data.Value), udpAddr)
	} else {
		// Send to connected address
		n, err = conn.Write([]byte(data.Value))
	}
	
	if err != nil {
		return object.NewError("socket_udp_send: failed to send: %s", err.Error())
	}
	
	return object.NewInteger(int64(n))
}

// socketUDPReceive receives data from UDP
func socketUDPReceive(args ...object.Object) object.Object {
	if !ModuleAllowed("socket") {
		return object.NewError("socket_udp_receive: access to native module socket denied by policy")
	}
	if len(args) < 1 || len(args) > 2 {
		return object.NewError("socket_udp_receive: expected 1-2 arguments, got %d", len(args))
	}
	
	socketID, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("socket_udp_receive: first argument must be string, got %s", args[0].Type())
	}
	
	bufferSize := 1024
	if len(args) == 2 {
		if sizeObj, ok := args[1].(*object.Integer); ok {
			bufferSize = int(sizeObj.Value)
		}
	}
	
	conn, exists := udpConnections[socketID.Value]
	if !exists {
		return object.NewError("socket_udp_receive: connection not found")
	}
	
	buffer := make([]byte, bufferSize)
	n, addr, err := conn.ReadFromUDP(buffer)
	if err != nil {
		return object.NewError("socket_udp_receive: failed to receive: %s", err.Error())
	}
	
	result := make(map[object.Object]object.Object)
	result[object.NewString("data")] = object.NewString(string(buffer[:n]))
	result[object.NewString("bytes_read")] = object.NewInteger(int64(n))
	if addr != nil {
		result[object.NewString("from_addr")] = object.NewString(addr.String())
	}
	
	return object.NewMap(result)
}

// socketUDPClose closes a UDP connection
func socketUDPClose(args ...object.Object) object.Object {
	if !ModuleAllowed("socket") {
		return object.NewError("socket_udp_close: access to native module socket denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("socket_udp_close: expected 1 argument, got %d", len(args))
	}
	
	socketID, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("socket_udp_close: argument must be string, got %s", args[0].Type())
	}
	
	conn, exists := udpConnections[socketID.Value]
	if !exists {
		return object.NewError("socket_udp_close: connection not found")
	}
	
	err := conn.Close()
	delete(udpConnections, socketID.Value)
	if err != nil {
		return object.NewError("socket_udp_close: failed to close: %s", err.Error())
	}
	
	return object.TRUE
}

// socketSetTimeout sets timeout for a connection
func socketSetTimeout(args ...object.Object) object.Object {
	if !ModuleAllowed("socket") {
		return object.NewError("socket_set_timeout: access to native module socket denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("socket_set_timeout: expected 2 arguments, got %d", len(args))
	}
	
	socketID, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("socket_set_timeout: first argument must be string, got %s", args[0].Type())
	}
	
	timeout, ok := args[1].(*object.Integer)
	if !ok {
		return object.NewError("socket_set_timeout: second argument must be integer, got %s", args[1].Type())
	}
	
	duration := time.Duration(timeout.Value) * time.Second
	
	// Try TCP connection first
	if conn, exists := tcpConnections[socketID.Value]; exists {
		if tcpConn, ok := conn.(*net.TCPConn); ok {
			err := tcpConn.SetDeadline(time.Now().Add(duration))
			if err != nil {
				return object.NewError("socket_set_timeout: failed to set timeout: %s", err.Error())
			}
			return object.TRUE
		}
	}
	
	// Try UDP connection
	if conn, exists := udpConnections[socketID.Value]; exists {
		err := conn.SetDeadline(time.Now().Add(duration))
		if err != nil {
			return object.NewError("socket_set_timeout: failed to set timeout: %s", err.Error())
		}
		return object.TRUE
	}
	
	return object.NewError("socket_set_timeout: connection not found")
}

// socketGetLocalAddr gets local address of a connection
func socketGetLocalAddr(args ...object.Object) object.Object {
	if !ModuleAllowed("socket") {
		return object.NewError("socket_get_local_addr: access to native module socket denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("socket_get_local_addr: expected 1 argument, got %d", len(args))
	}
	
	socketID, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("socket_get_local_addr: argument must be string, got %s", args[0].Type())
	}
	
	// Try TCP connection
	if conn, exists := tcpConnections[socketID.Value]; exists {
		return object.NewString(conn.LocalAddr().String())
	}
	
	// Try UDP connection
	if conn, exists := udpConnections[socketID.Value]; exists {
		return object.NewString(conn.LocalAddr().String())
	}
	
	// Try TCP listener
	if listener, exists := tcpListeners[socketID.Value]; exists {
		return object.NewString(listener.Addr().String())
	}
	
	return object.NewError("socket_get_local_addr: socket not found")
}

// socketGetRemoteAddr gets remote address of a connection
func socketGetRemoteAddr(args ...object.Object) object.Object {
	if !ModuleAllowed("socket") {
		return object.NewError("socket_get_remote_addr: access to native module socket denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("socket_get_remote_addr: expected 1 argument, got %d", len(args))
	}
	
	socketID, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("socket_get_remote_addr: argument must be string, got %s", args[0].Type())
	}
	
	// Try TCP connection
	if conn, exists := tcpConnections[socketID.Value]; exists {
		return object.NewString(conn.RemoteAddr().String())
	}
	
	// Try UDP connection
	if conn, exists := udpConnections[socketID.Value]; exists {
		if conn.RemoteAddr() != nil {
			return object.NewString(conn.RemoteAddr().String())
		}
		return object.NewString("") // UDP listener doesn't have remote addr
	}
	
	return object.NewError("socket_get_remote_addr: socket not found")
}
