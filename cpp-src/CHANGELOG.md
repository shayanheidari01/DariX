# Changelog — DariX C++ Implementation

## [1.0.0] — 2026

### Core Language
- Full lexer, parser (Pratt), and AST for all DariX constructs
- Tree-walking interpreter with 35+ built-in functions
- Bytecode compiler with constant folding and peephole optimization
- Stack-based VM with 30 opcodes
- Bytecode/interpreter auto-selection with fallback
- Dynamic typing, closures, lambdas, decorators
- Classes with `__init__`, methods, bound methods
- Exception handling (try/catch/finally, throw/raise)
- Keywords: `import`, `del`, `assert`, `pass`, `global`, `nonlocal`, `with`, `yield`, `in`, `is`, `and`, `or`, `not`, `lambda`
- Short-circuit evaluation for `&&`, `||`, `and`, `or`
- For-loop desugaring with `continue`/`break` support

### Native Modules (21 modules, ~600 functions)
- **math** (27): sqrt, pow, trig, hyperbolic, rounding, constants, random
- **string** (37): case, trim, split, join, replace, pad, slice, count, char_at, to_title, chars, words, lines, truncate, center, is_alpha/digit/space/number, to_int/float
- **array** (28): filter, map, reduce, sort_by, group_by, partition, flatten, chunk, zip, unzip, unique, diff, intersect, union, all, any, min_by, max_by, enumerate, contains_value, index_of, first/last, take/drop, fold, each
- **map** (22): keys, values, items, has_key/value, get, put, remove, merge, size, is_empty, clear, map_keys/values, filter, find_key, from/to_pairs, invert, equals, keys_array
- **set** (27): from_array, add, remove, union, intersection, difference, symmetric_difference, is_subset/superset/disjoint, power, cartesian, fold, map_set, filter_set, min, max, equals, sorted
- **queue** (25): enqueue, dequeue, peek, peek_back, enqueue_front, dequeue_back, take, drop, merge, filter, map_queue, rotate, flatten, unique, index_of, slice, clear
- **stack** (28): push, pop, peek, peek_bottom, push_many, pop_n, peek_n, merge, filter, map_stack, reverse, unique, flatten, min, max, index_of, clear
- **linkedlist** (38): head, tail, last, init, cons, append, concat, nth, insert_at, remove_at, remove_first/all, contains, index_of, reverse, sort, unique, take/drop/slice, zip, flatten, enumerate, min/max, fold, map/filter/partition/group_by, equals, to_string, clear
- **tree** (22): node, leaf, value, children, is_leaf, set_value, add_child/children, size, depth, height, depth_of, find, find_all, preorder/inorder/postorder/levelorder, map_tree, filter_tree, clone, equals, to_string
- **graph** (22): from_edges, add/remove vertex/edge, vertices, edges, neighbors, degree, in_degree, has_vertex/edge, bfs, dfs, shortest_path, has_path, is_connected, topological_sort, transpose, subgraph, clone, equals, to_string
- **json** (3): parse, stringify (with indent), is_valid
- **fs** (22): read, write, append, exists, is_file/dir, mkdir, rmdir, remove, rename, copy, size, list_dir, list_dir_full, cwd, chdir, join, parent, filename, extension, stem, absolute, temp_dir, env
- **net** (9): tcp_connect/send/recv/close, udp_send, http_get/post, resolve
- **crypto** (14): md5, sha256, sha512, hmac_sha256, base64_encode/decode, hex_encode/decode, random_bytes/hex, uuid, crc32, pbkdf2, hash
- **datetime** (30): now, now_ms, format, year/month/day/hour/minute/second, weekday, day_of_year, is_leap_year, days_in_month, add_days/hours/minutes/seconds, diff/days, to_string/date/time/iso, make, parse, is_before/after/same_day, timezone_offset, clock, sleep
- **random** (16): seed, int, int_range, float, float_range, choice, choices, sample, shuffle, coin, weighted_choice, booleans, ints, normal, exponential
- **regex** (13): match, matches, groups, named_groups, test, replace, replace_all, replace_with_fn, split, find, count, escape, is_valid
- **io** (20): print, println, print_no_newline, format, sprint, read_line, read_all, read, read_int/float/bool, read_until, confirm, choose, progress, spinner, table, json_table, clear_screen, beep
- **os** (15): getenv, setenv, unsetenv, platform, arch, hostname, getpid, exit, exec, sleep, cpu_count, uname, clock, memory_info, user, home
- **encoding** (17): base64_encode/decode, base32_encode/decode, hex_encode/decode/upper, url_encode/decode, html_encode/decode, binary_encode/decode, octal_encode/decode, caesar_encode/decode, rot13, xor_encode

### Import Syntax
```dax
import math
math.sqrt(16)    // 4
```

### Bug Fixes
- For-loop `continue` now correctly executes post-increment
- `IfExpression` as statement pushes NULL for proper stack balance
- Map/Array equality comparisons (`==`, `!=`) now work correctly
- Variable reassignment in inner scopes updates the correct scope
- NameError exceptions are catchable via try/catch
- VM falls back to interpreter for unsupported features
- `print` no longer causes stack underflow in bytecode

### Test Coverage
- 133 language feature tests
- 460+ module function tests across 20 modules
- 593 total test cases, all passing
