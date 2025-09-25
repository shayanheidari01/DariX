package native

import (
	"darix/object"
	"strings"
	"time"
)

func init() {
	Register("time", map[string]*object.Builtin{
		"time_now":        {Fn: timeNow},
		"time_unix":       {Fn: timeUnix},
		"time_format":     {Fn: timeFormat},
		"time_parse":      {Fn: timeParse},
		"time_sleep":      {Fn: timeSleep},
		"time_year":       {Fn: timeYear},
		"time_month":      {Fn: timeMonth},
		"time_day":        {Fn: timeDay},
		"time_hour":       {Fn: timeHour},
		"time_minute":     {Fn: timeMinute},
		"time_second":     {Fn: timeSecond},
		"time_weekday":    {Fn: timeWeekday},
		"time_add_days":   {Fn: timeAddDays},
		"time_add_hours":  {Fn: timeAddHours},
		"time_diff":       {Fn: timeDiff},
	})
}

// timeNow returns current Unix timestamp
func timeNow(args ...object.Object) object.Object {
	if !ModuleAllowed("time") {
		return object.NewError("time_now: access to native module time denied by policy")
	}
	if len(args) != 0 {
		return object.NewError("time_now: expected 0 arguments, got %d", len(args))
	}
	
	return object.NewInteger(time.Now().Unix())
}

// timeUnix creates time from Unix timestamp
func timeUnix(args ...object.Object) object.Object {
	if !ModuleAllowed("time") {
		return object.NewError("time_unix: access to native module time denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("time_unix: expected 1 argument, got %d", len(args))
	}
	
	timestamp, ok := args[0].(*object.Integer)
	if !ok {
		return object.NewError("time_unix: argument must be integer, got %s", args[0].Type())
	}
	
	return object.NewInteger(timestamp.Value)
}

// timeFormat formats Unix timestamp to string
func timeFormat(args ...object.Object) object.Object {
	if !ModuleAllowed("time") {
		return object.NewError("time_format: access to native module time denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("time_format: expected 2 arguments, got %d", len(args))
	}
	
	timestamp, ok1 := args[0].(*object.Integer)
	format, ok2 := args[1].(*object.String)
	if !ok1 || !ok2 {
		return object.NewError("time_format: first argument must be integer, second must be string")
	}
	
	t := time.Unix(timestamp.Value, 0)
	
	// Convert common format patterns
	goFormat := convertTimeFormat(format.Value)
	formatted := t.Format(goFormat)
	
	return object.NewString(formatted)
}

// timeParse parses time string to Unix timestamp
func timeParse(args ...object.Object) object.Object {
	if !ModuleAllowed("time") {
		return object.NewError("time_parse: access to native module time denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("time_parse: expected 2 arguments, got %d", len(args))
	}
	
	format, ok1 := args[0].(*object.String)
	timeStr, ok2 := args[1].(*object.String)
	if !ok1 || !ok2 {
		return object.NewError("time_parse: arguments must be strings")
	}
	
	goFormat := convertTimeFormat(format.Value)
	t, err := time.Parse(goFormat, timeStr.Value)
	if err != nil {
		return object.NewError("time_parse: %s", err.Error())
	}
	
	return object.NewInteger(t.Unix())
}

// timeSleep pauses execution for specified seconds
func timeSleep(args ...object.Object) object.Object {
	if !ModuleAllowed("time") {
		return object.NewError("time_sleep: access to native module time denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("time_sleep: expected 1 argument, got %d", len(args))
	}
	
	var seconds float64
	switch v := args[0].(type) {
	case *object.Integer:
		seconds = float64(v.Value)
	case *object.Float:
		seconds = v.Value
	default:
		return object.NewError("time_sleep: argument must be number, got %s", args[0].Type())
	}
	
	if seconds < 0 {
		return object.NewError("time_sleep: sleep duration cannot be negative")
	}
	
	duration := time.Duration(seconds * float64(time.Second))
	time.Sleep(duration)
	
	return object.NULL
}

// timeYear extracts year from Unix timestamp
func timeYear(args ...object.Object) object.Object {
	if !ModuleAllowed("time") {
		return object.NewError("time_year: access to native module time denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("time_year: expected 1 argument, got %d", len(args))
	}
	
	timestamp, ok := args[0].(*object.Integer)
	if !ok {
		return object.NewError("time_year: argument must be integer, got %s", args[0].Type())
	}
	
	t := time.Unix(timestamp.Value, 0)
	return object.NewInteger(int64(t.Year()))
}

// timeMonth extracts month from Unix timestamp
func timeMonth(args ...object.Object) object.Object {
	if !ModuleAllowed("time") {
		return object.NewError("time_month: access to native module time denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("time_month: expected 1 argument, got %d", len(args))
	}
	
	timestamp, ok := args[0].(*object.Integer)
	if !ok {
		return object.NewError("time_month: argument must be integer, got %s", args[0].Type())
	}
	
	t := time.Unix(timestamp.Value, 0)
	return object.NewInteger(int64(t.Month()))
}

// timeDay extracts day from Unix timestamp
func timeDay(args ...object.Object) object.Object {
	if !ModuleAllowed("time") {
		return object.NewError("time_day: access to native module time denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("time_day: expected 1 argument, got %d", len(args))
	}
	
	timestamp, ok := args[0].(*object.Integer)
	if !ok {
		return object.NewError("time_day: argument must be integer, got %s", args[0].Type())
	}
	
	t := time.Unix(timestamp.Value, 0)
	return object.NewInteger(int64(t.Day()))
}

// timeHour extracts hour from Unix timestamp
func timeHour(args ...object.Object) object.Object {
	if !ModuleAllowed("time") {
		return object.NewError("time_hour: access to native module time denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("time_hour: expected 1 argument, got %d", len(args))
	}
	
	timestamp, ok := args[0].(*object.Integer)
	if !ok {
		return object.NewError("time_hour: argument must be integer, got %s", args[0].Type())
	}
	
	t := time.Unix(timestamp.Value, 0)
	return object.NewInteger(int64(t.Hour()))
}

// timeMinute extracts minute from Unix timestamp
func timeMinute(args ...object.Object) object.Object {
	if !ModuleAllowed("time") {
		return object.NewError("time_minute: access to native module time denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("time_minute: expected 1 argument, got %d", len(args))
	}
	
	timestamp, ok := args[0].(*object.Integer)
	if !ok {
		return object.NewError("time_minute: argument must be integer, got %s", args[0].Type())
	}
	
	t := time.Unix(timestamp.Value, 0)
	return object.NewInteger(int64(t.Minute()))
}

// timeSecond extracts second from Unix timestamp
func timeSecond(args ...object.Object) object.Object {
	if !ModuleAllowed("time") {
		return object.NewError("time_second: access to native module time denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("time_second: expected 1 argument, got %d", len(args))
	}
	
	timestamp, ok := args[0].(*object.Integer)
	if !ok {
		return object.NewError("time_second: argument must be integer, got %s", args[0].Type())
	}
	
	t := time.Unix(timestamp.Value, 0)
	return object.NewInteger(int64(t.Second()))
}

// timeWeekday gets weekday from Unix timestamp
func timeWeekday(args ...object.Object) object.Object {
	if !ModuleAllowed("time") {
		return object.NewError("time_weekday: access to native module time denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("time_weekday: expected 1 argument, got %d", len(args))
	}
	
	timestamp, ok := args[0].(*object.Integer)
	if !ok {
		return object.NewError("time_weekday: argument must be integer, got %s", args[0].Type())
	}
	
	t := time.Unix(timestamp.Value, 0)
	return object.NewInteger(int64(t.Weekday()))
}

// timeAddDays adds days to Unix timestamp
func timeAddDays(args ...object.Object) object.Object {
	if !ModuleAllowed("time") {
		return object.NewError("time_add_days: access to native module time denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("time_add_days: expected 2 arguments, got %d", len(args))
	}
	
	timestamp, ok1 := args[0].(*object.Integer)
	days, ok2 := args[1].(*object.Integer)
	if !ok1 || !ok2 {
		return object.NewError("time_add_days: arguments must be integers")
	}
	
	t := time.Unix(timestamp.Value, 0)
	newTime := t.AddDate(0, 0, int(days.Value))
	
	return object.NewInteger(newTime.Unix())
}

// timeAddHours adds hours to Unix timestamp
func timeAddHours(args ...object.Object) object.Object {
	if !ModuleAllowed("time") {
		return object.NewError("time_add_hours: access to native module time denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("time_add_hours: expected 2 arguments, got %d", len(args))
	}
	
	timestamp, ok1 := args[0].(*object.Integer)
	hours, ok2 := args[1].(*object.Integer)
	if !ok1 || !ok2 {
		return object.NewError("time_add_hours: arguments must be integers")
	}
	
	t := time.Unix(timestamp.Value, 0)
	duration := time.Duration(hours.Value) * time.Hour
	newTime := t.Add(duration)
	
	return object.NewInteger(newTime.Unix())
}

// timeDiff calculates difference between two timestamps in seconds
func timeDiff(args ...object.Object) object.Object {
	if !ModuleAllowed("time") {
		return object.NewError("time_diff: access to native module time denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("time_diff: expected 2 arguments, got %d", len(args))
	}
	
	time1, ok1 := args[0].(*object.Integer)
	time2, ok2 := args[1].(*object.Integer)
	if !ok1 || !ok2 {
		return object.NewError("time_diff: arguments must be integers")
	}
	
	diff := time1.Value - time2.Value
	return object.NewInteger(diff)
}

// convertTimeFormat converts common time format patterns to Go format
func convertTimeFormat(format string) string {
	// Convert common patterns to Go time format
	// Go uses reference time: Mon Jan 2 15:04:05 MST 2006
	replacements := map[string]string{
		"YYYY": "2006",
		"YY":   "06",
		"MM":   "01",
		"DD":   "02",
		"HH":   "15",
		"mm":   "04",
		"ss":   "05",
	}
	
	result := format
	for pattern, goPattern := range replacements {
		result = strings.Replace(result, pattern, goPattern, -1)
	}
	
	return result
}
