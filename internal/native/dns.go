package native

import (
	"context"
	"darix/object"
	"net"
	"strings"
	"time"
)

func init() {
	Register("dns", map[string]*object.Builtin{
		"dns_lookup":        {Fn: dnsLookup},
		"dns_reverse":       {Fn: dnsReverse},
		"dns_lookup_mx":     {Fn: dnsLookupMX},
		"dns_lookup_txt":    {Fn: dnsLookupTXT},
		"dns_lookup_cname":  {Fn: dnsLookupCNAME},
		"dns_lookup_ns":     {Fn: dnsLookupNS},
		"dns_is_valid_ip":   {Fn: dnsIsValidIP},
		"dns_is_valid_domain": {Fn: dnsIsValidDomain},
	})
}

// dnsLookup performs DNS lookup for a domain
func dnsLookup(args ...object.Object) object.Object {
	if !ModuleAllowed("dns") {
		return object.NewError("dns_lookup: access to native module dns denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("dns_lookup: expected 1 argument, got %d", len(args))
	}
	
	domain, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("dns_lookup: argument must be string, got %s", args[0].Type())
	}
	
	// Create context with timeout
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()
	
	// Perform DNS lookup
	ips, err := net.DefaultResolver.LookupIPAddr(ctx, domain.Value)
	if err != nil {
		return object.NewError("dns_lookup: failed to lookup domain: %s", err.Error())
	}
	
	// Convert IPs to array
	ipArray := make([]object.Object, len(ips))
	for i, ip := range ips {
		ipArray[i] = object.NewString(ip.IP.String())
	}
	
	result := make(map[object.Object]object.Object)
	result[object.NewString("domain")] = object.NewString(domain.Value)
	result[object.NewString("ips")] = object.NewArray(ipArray)
	result[object.NewString("count")] = object.NewInteger(int64(len(ips)))
	
	return object.NewMap(result)
}

// dnsReverse performs reverse DNS lookup
func dnsReverse(args ...object.Object) object.Object {
	if !ModuleAllowed("dns") {
		return object.NewError("dns_reverse: access to native module dns denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("dns_reverse: expected 1 argument, got %d", len(args))
	}
	
	ip, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("dns_reverse: argument must be string, got %s", args[0].Type())
	}
	
	// Validate IP address
	parsedIP := net.ParseIP(ip.Value)
	if parsedIP == nil {
		return object.NewError("dns_reverse: invalid IP address: %s", ip.Value)
	}
	
	// Create context with timeout
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()
	
	// Perform reverse DNS lookup
	names, err := net.DefaultResolver.LookupAddr(ctx, ip.Value)
	if err != nil {
		return object.NewError("dns_reverse: failed to reverse lookup IP: %s", err.Error())
	}
	
	// Convert names to array
	nameArray := make([]object.Object, len(names))
	for i, name := range names {
		// Remove trailing dot if present
		name = strings.TrimSuffix(name, ".")
		nameArray[i] = object.NewString(name)
	}
	
	result := make(map[object.Object]object.Object)
	result[object.NewString("ip")] = object.NewString(ip.Value)
	result[object.NewString("names")] = object.NewArray(nameArray)
	result[object.NewString("count")] = object.NewInteger(int64(len(names)))
	
	return object.NewMap(result)
}

// dnsLookupMX performs MX record lookup
func dnsLookupMX(args ...object.Object) object.Object {
	if !ModuleAllowed("dns") {
		return object.NewError("dns_lookup_mx: access to native module dns denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("dns_lookup_mx: expected 1 argument, got %d", len(args))
	}
	
	domain, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("dns_lookup_mx: argument must be string, got %s", args[0].Type())
	}
	
	// Create context with timeout
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()
	
	// Perform MX lookup
	mxRecords, err := net.DefaultResolver.LookupMX(ctx, domain.Value)
	if err != nil {
		return object.NewError("dns_lookup_mx: failed to lookup MX records: %s", err.Error())
	}
	
	// Convert MX records to array
	mxArray := make([]object.Object, len(mxRecords))
	for i, mx := range mxRecords {
		mxMap := make(map[object.Object]object.Object)
		mxMap[object.NewString("host")] = object.NewString(strings.TrimSuffix(mx.Host, "."))
		mxMap[object.NewString("priority")] = object.NewInteger(int64(mx.Pref))
		mxArray[i] = object.NewMap(mxMap)
	}
	
	result := make(map[object.Object]object.Object)
	result[object.NewString("domain")] = object.NewString(domain.Value)
	result[object.NewString("mx_records")] = object.NewArray(mxArray)
	result[object.NewString("count")] = object.NewInteger(int64(len(mxRecords)))
	
	return object.NewMap(result)
}

// dnsLookupTXT performs TXT record lookup
func dnsLookupTXT(args ...object.Object) object.Object {
	if !ModuleAllowed("dns") {
		return object.NewError("dns_lookup_txt: access to native module dns denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("dns_lookup_txt: expected 1 argument, got %d", len(args))
	}
	
	domain, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("dns_lookup_txt: argument must be string, got %s", args[0].Type())
	}
	
	// Create context with timeout
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()
	
	// Perform TXT lookup
	txtRecords, err := net.DefaultResolver.LookupTXT(ctx, domain.Value)
	if err != nil {
		return object.NewError("dns_lookup_txt: failed to lookup TXT records: %s", err.Error())
	}
	
	// Convert TXT records to array
	txtArray := make([]object.Object, len(txtRecords))
	for i, txt := range txtRecords {
		txtArray[i] = object.NewString(txt)
	}
	
	result := make(map[object.Object]object.Object)
	result[object.NewString("domain")] = object.NewString(domain.Value)
	result[object.NewString("txt_records")] = object.NewArray(txtArray)
	result[object.NewString("count")] = object.NewInteger(int64(len(txtRecords)))
	
	return object.NewMap(result)
}

// dnsLookupCNAME performs CNAME record lookup
func dnsLookupCNAME(args ...object.Object) object.Object {
	if !ModuleAllowed("dns") {
		return object.NewError("dns_lookup_cname: access to native module dns denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("dns_lookup_cname: expected 1 argument, got %d", len(args))
	}
	
	domain, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("dns_lookup_cname: argument must be string, got %s", args[0].Type())
	}
	
	// Create context with timeout
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()
	
	// Perform CNAME lookup
	cname, err := net.DefaultResolver.LookupCNAME(ctx, domain.Value)
	if err != nil {
		return object.NewError("dns_lookup_cname: failed to lookup CNAME record: %s", err.Error())
	}
	
	// Remove trailing dot if present
	cname = strings.TrimSuffix(cname, ".")
	
	result := make(map[object.Object]object.Object)
	result[object.NewString("domain")] = object.NewString(domain.Value)
	result[object.NewString("cname")] = object.NewString(cname)
	
	return object.NewMap(result)
}

// dnsLookupNS performs NS record lookup
func dnsLookupNS(args ...object.Object) object.Object {
	if !ModuleAllowed("dns") {
		return object.NewError("dns_lookup_ns: access to native module dns denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("dns_lookup_ns: expected 1 argument, got %d", len(args))
	}
	
	domain, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("dns_lookup_ns: argument must be string, got %s", args[0].Type())
	}
	
	// Create context with timeout
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()
	
	// Perform NS lookup
	nsRecords, err := net.DefaultResolver.LookupNS(ctx, domain.Value)
	if err != nil {
		return object.NewError("dns_lookup_ns: failed to lookup NS records: %s", err.Error())
	}
	
	// Convert NS records to array
	nsArray := make([]object.Object, len(nsRecords))
	for i, ns := range nsRecords {
		nsArray[i] = object.NewString(strings.TrimSuffix(ns.Host, "."))
	}
	
	result := make(map[object.Object]object.Object)
	result[object.NewString("domain")] = object.NewString(domain.Value)
	result[object.NewString("ns_records")] = object.NewArray(nsArray)
	result[object.NewString("count")] = object.NewInteger(int64(len(nsRecords)))
	
	return object.NewMap(result)
}

// dnsIsValidIP checks if string is a valid IP address
func dnsIsValidIP(args ...object.Object) object.Object {
	if !ModuleAllowed("dns") {
		return object.NewError("dns_is_valid_ip: access to native module dns denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("dns_is_valid_ip: expected 1 argument, got %d", len(args))
	}
	
	ip, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("dns_is_valid_ip: argument must be string, got %s", args[0].Type())
	}
	
	parsedIP := net.ParseIP(ip.Value)
	isValid := parsedIP != nil
	
	result := make(map[object.Object]object.Object)
	result[object.NewString("ip")] = object.NewString(ip.Value)
	result[object.NewString("valid")] = object.NewBoolean(isValid)
	
	if isValid {
		if parsedIP.To4() != nil {
			result[object.NewString("version")] = object.NewString("IPv4")
		} else {
			result[object.NewString("version")] = object.NewString("IPv6")
		}
	}
	
	return object.NewMap(result)
}

// dnsIsValidDomain checks if string is a valid domain name
func dnsIsValidDomain(args ...object.Object) object.Object {
	if !ModuleAllowed("dns") {
		return object.NewError("dns_is_valid_domain: access to native module dns denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("dns_is_valid_domain: expected 1 argument, got %d", len(args))
	}
	
	domain, ok := args[0].(*object.String)
	if !ok {
		return object.NewError("dns_is_valid_domain: argument must be string, got %s", args[0].Type())
	}
	
	// Basic domain validation
	domainStr := strings.TrimSpace(domain.Value)
	isValid := true
	
	// Check basic requirements
	if len(domainStr) == 0 || len(domainStr) > 253 {
		isValid = false
	} else if strings.HasPrefix(domainStr, ".") || strings.HasSuffix(domainStr, ".") {
		isValid = false
	} else if strings.Contains(domainStr, "..") {
		isValid = false
	} else {
		// Check each label
		labels := strings.Split(domainStr, ".")
		for _, label := range labels {
			if len(label) == 0 || len(label) > 63 {
				isValid = false
				break
			}
			if strings.HasPrefix(label, "-") || strings.HasSuffix(label, "-") {
				isValid = false
				break
			}
			// Check for valid characters (letters, digits, hyphens)
			for _, char := range label {
				if !((char >= 'a' && char <= 'z') || 
					 (char >= 'A' && char <= 'Z') || 
					 (char >= '0' && char <= '9') || 
					 char == '-') {
					isValid = false
					break
				}
			}
			if !isValid {
				break
			}
		}
	}
	
	result := make(map[object.Object]object.Object)
	result[object.NewString("domain")] = object.NewString(domain.Value)
	result[object.NewString("valid")] = object.NewBoolean(isValid)
	
	if isValid {
		labels := strings.Split(domainStr, ".")
		result[object.NewString("labels")] = object.NewInteger(int64(len(labels)))
		if len(labels) > 0 {
			result[object.NewString("tld")] = object.NewString(labels[len(labels)-1])
		}
	}
	
	return object.NewMap(result)
}
