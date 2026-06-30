# DariX Native Modules Reference

All modules are imported with `import module_name` and accessed via `module.function()`.

---

## math — Mathematical Functions

```dax
import math
```

| Function | Signature | Description |
|----------|-----------|-------------|
| `sqrt` | `(x)` | Square root |
| `pow` | `(base, exp)` | Power |
| `exp` | `(x)` | e^x |
| `log` | `(x)` | Natural logarithm |
| `log10` | `(x)` | Base-10 logarithm |
| `log2` | `(x)` | Base-2 logarithm |
| `sin` | `(x)` | Sine (radians) |
| `cos` | `(x)` | Cosine (radians) |
| `tan` | `(x)` | Tangent (radians) |
| `asin` | `(x)` | Arc sine |
| `acos` | `(x)` | Arc cosine |
| `atan` | `(x)` | Arc tangent |
| `atan2` | `(y, x)` | Two-argument arc tangent |
| `sinh` | `(x)` | Hyperbolic sine |
| `cosh` | `(x)` | Hyperbolic cosine |
| `tanh` | `(x)` | Hyperbolic tangent |
| `ceil` | `(x)` | Round up |
| `floor` | `(x)` | Round down |
| `round` | `(x)` | Round to nearest |
| `trunc` | `(x)` | Truncate to integer |
| `max` | `(a, b, ...)` | Maximum value |
| `min` | `(a, b, ...)` | Minimum value |
| `pi` | `()` | Pi constant |
| `e` | `()` | Euler's number |
| `abs` | `(x)` | Absolute value |
| `mod` | `(x, y)` | Floating-point modulo |
| `random` | `()` | Random float [0, 1) |

---

## string — String Manipulation

```dax
import string
```

| Function | Signature | Description |
|----------|-----------|-------------|
| `upper` | `(s)` | Uppercase |
| `lower` | `(s)` | Lowercase |
| `trim` | `(s)` | Trim whitespace |
| `trim_left` | `(s, chars)` | Trim left characters |
| `trim_right` | `(s, chars)` | Trim right characters |
| `split` | `(s, sep)` | Split by separator |
| `join` | `(arr, sep)` | Join array with separator |
| `replace` | `(s, old, new)` | Replace all occurrences |
| `contains` | `(s, sub)` | Check if substring exists |
| `starts` | `(s, prefix)` | Check prefix |
| `ends` | `(s, suffix)` | Check suffix |
| `index` | `(s, sub)` | Find first index (-1 if not found) |
| `last_index` | `(s, sub)` | Find last index |
| `repeat` | `(s, n)` | Repeat string n times |
| `reverse` | `(s)` | Reverse string |
| `is_alpha` | `(s)` | Check if all characters are letters |
| `is_digit` | `(s)` | Check if all characters are digits |
| `is_space` | `(s)` | Check if all characters are whitespace |
| `pad_left` | `(s, width, char?)` | Left-pad to width |
| `pad_right` | `(s, width, char?)` | Right-pad to width |
| `slice` | `(s, start, end?)` | Substring |
| `count` | `(s, sub)` | Count occurrences |
| `char_at` | `(s, index)` | Character at index |
| `to_title` | `(s)` | Title Case |
| `chars` | `(s)` | Array of characters |
| `words` | `(s)` | Split by whitespace |
| `lines` | `(s)` | Split by newline |
| `truncate` | `(s, max, suffix?)` | Truncate with suffix |
| `center` | `(s, width, char?)` | Center-align |
| `replace_first` | `(s, old, new)` | Replace first occurrence |
| `is_empty` | `(s)` | Check if empty |
| `starts_with` | `(s, prefix)` | Alias for starts |
| `ends_with` | `(s, suffix)` | Alias for ends |
| `to_int` | `(s)` | Convert to integer |
| `to_float` | `(s)` | Convert to float |
| `is_number` | `(s)` | Check if numeric |

---

## array — Array Operations

```dax
import array
```

| Function | Signature | Description |
|----------|-----------|-------------|
| `filter` | `(arr, fn)` | Filter elements |
| `map` | `(arr, fn)` | Transform elements |
| `reduce` | `(arr, fn, init)` | Reduce to single value |
| `find` | `(arr, fn)` | Find first match |
| `find_all` | `(arr, fn)` | Find all matches |
| `unique` | `(arr)` | Remove duplicates |
| `flatten` | `(arr)` | Flatten one level |
| `chunk` | `(arr, size)` | Split into chunks |
| `zip` | `(a, b)` | Pair elements |
| `unzip` | `(arr)` | Unpair elements |
| `group_by` | `(arr, fn)` | Group by key |
| `sort_by` | `(arr, fn)` | Sort by key |
| `partition` | `(arr, fn)` | Split by predicate |
| `diff` | `(a, b)` | Elements in a not in b |
| `intersect` | `(a, b)` | Elements in both |
| `union` | `(a, b)` | All unique elements |
| `each` | `(arr, fn)` | Iterate with side effects |
| `all` | `(arr, fn)` | All match predicate |
| `any` | `(arr, fn)` | Any match predicate |
| `contains_value` | `(arr, val)` | Check if value exists |
| `index_of` | `(arr, val)` | Find index of value |
| `first` | `(arr)` | First element |
| `last` | `(arr)` | Last element |
| `take` | `(arr, n)` | First n elements |
| `drop` | `(arr, n)` | Skip first n elements |
| `min_by` | `(arr, fn)` | Element with minimum key |
| `max_by` | `(arr, fn)` | Element with maximum key |
| `enumerate` | `(arr)` | Array of [index, value] pairs |

---

## map — Map Operations

```dax
import map
```

| Function | Signature | Description |
|----------|-----------|-------------|
| `keys` | `(m)` | Array of keys |
| `values` | `(m)` | Array of values |
| `items` | `(m)` | Array of [key, value] pairs |
| `has_key` | `(m, key)` | Check if key exists |
| `has_value` | `(m, val)` | Check if value exists |
| `get` | `(m, key, default?)` | Get value with default |
| `put` | `(m, key, val)` | Add/update entry |
| `remove` | `(m, key)` | Remove entry |
| `merge` | `(a, b)` | Merge two maps |
| `size` | `(m)` | Number of entries |
| `is_empty` | `(m)` | Check if empty |
| `clear` | `(m)` | Remove all entries |
| `map_keys` | `(m, fn)` | Transform keys |
| `map_values` | `(m, fn)` | Transform values |
| `filter` | `(m, fn)` | Filter entries |
| `find_key` | `(m, fn)` | Find first matching key |
| `from_pairs` | `(arr)` | Create map from pairs array |
| `to_pairs` | `(m)` | Convert to pairs array |
| `invert` | `(m)` | Swap keys and values |
| `equals` | `(a, b)` | Structural equality |
| `keys_array` | `(m)` | Sorted array of keys |

---

## set — Set Operations

```dax
import set
```

| Function | Signature | Description |
|----------|-----------|-------------|
| `from_array` | `(arr)` | Create set from array |
| `to_array` | `(s)` | Convert to array |
| `size` | `(s)` | Number of elements |
| `is_empty` | `(s)` | Check if empty |
| `contains` | `(s, elem)` | Check membership |
| `add` | `(s, elem)` | Add element |
| `remove` | `(s, elem)` | Remove element |
| `union` | `(a, b)` | All elements from both |
| `intersection` | `(a, b)` | Common elements |
| `difference` | `(a, b)` | Elements in a not in b |
| `symmetric_difference` | `(a, b)` | Elements in either but not both |
| `is_subset` | `(a, b)` | Check subset |
| `is_superset` | `(a, b)` | Check superset |
| `is_disjoint` | `(a, b)` | Check no common elements |
| `power` | `(s)` | Power set |
| `cartesian` | `(a, b)` | Cartesian product |
| `fold` | `(s, fn, init)` | Reduce |
| `map_set` | `(s, fn)` | Transform elements |
| `filter_set` | `(s, fn)` | Filter elements |
| `min` | `(s)` | Minimum element |
| `max` | `(s)` | Maximum element |
| `sorted` | `(s)` | Sorted array |
| `equals` | `(a, b)` | Set equality |

---

## queue — FIFO Queue

```dax
import queue
```

| Function | Signature | Description |
|----------|-----------|-------------|
| `new` | `()` | Create empty queue |
| `from_array` | `(arr)` | Create from array |
| `to_array` | `(q)` | Convert to array |
| `enqueue` | `(q, elem)` | Add to back |
| `dequeue` | `(q)` | Remove from front → [front, rest] |
| `peek` | `(q)` | View front element |
| `peek_back` | `(q)` | View back element |
| `size` | `(q)` | Number of elements |
| `is_empty` | `(q)` | Check if empty |
| `contains` | `(q, elem)` | Check membership |
| `clear` | `(q)` | Empty the queue |
| `reverse` | `(q)` | Reverse order |
| `enqueue_front` | `(q, elem)` | Add to front |
| `dequeue_back` | `(q)` | Remove from back |
| `take` | `(q, n)` | First n elements |
| `drop` | `(q, n)` | Skip first n elements |
| `merge` | `(a, b)` | Concatenate queues |
| `filter` | `(q, fn)` | Filter elements |
| `map_queue` | `(q, fn)` | Transform elements |
| `rotate` | `(q, n)` | Rotate left by n |
| `flatten` | `(q)` | Flatten one level |
| `unique` | `(q)` | Remove duplicates |
| `index_of` | `(q, elem)` | Find index |
| `slice` | `(q, start, end?)` | Sub-queue |

---

## stack — LIFO Stack

```dax
import stack
```

| Function | Signature | Description |
|----------|-----------|-------------|
| `new` | `()` | Create empty stack |
| `push` | `(s, elem)` | Push element |
| `pop` | `(s)` | Pop top → [top, rest] |
| `peek` | `(s)` | View top element |
| `peek_bottom` | `(s)` | View bottom element |
| `size` | `(s)` | Number of elements |
| `is_empty` | `(s)` | Check if empty |
| `contains` | `(s, elem)` | Check membership |
| `push_many` | `(s, arr)` | Push all elements |
| `pop_n` | `(s, n)` | Pop n elements → [popped, rest] |
| `peek_n` | `(s, n)` | View top n elements |
| `merge` | `(a, b)` | Combine stacks |
| `filter` | `(s, fn)` | Filter elements |
| `map_stack` | `(s, fn)` | Transform elements |
| `reverse` | `(s)` | Reverse order |
| `unique` | `(s)` | Remove duplicates |
| `flatten` | `(s)` | Flatten one level |
| `min` | `(s)` | Minimum element |
| `max` | `(s)` | Maximum element |
| `index_of` | `(s, elem)` | Find index |
| `clear` | `(s)` | Empty the stack |

---

## linkedlist — Linked List Operations

```dax
import linkedlist
```

| Function | Signature | Description |
|----------|-----------|-------------|
| `new` | `()` | Create empty list |
| `from_array` | `(arr)` | Create from array |
| `to_array` | `(ll)` | Convert to array |
| `head` | `(ll)` | First element |
| `tail` | `(ll)` | All but first |
| `last` | `(ll)` | Last element |
| `init` | `(ll)` | All but last |
| `cons` | `(ll, elem)` | Prepend element |
| `append` | `(ll, elem)` | Append element |
| `concat` | `(a, b)` | Concatenate lists |
| `length` | `(ll)` | Number of elements |
| `nth` | `(ll, index)` | Element at index |
| `insert_at` | `(ll, index, elem)` | Insert at position |
| `remove_at` | `(ll, index)` | Remove at position |
| `remove_first` | `(ll, elem)` | Remove first occurrence |
| `remove_all` | `(ll, elem)` | Remove all occurrences |
| `contains` | `(ll, elem)` | Check membership |
| `index_of` | `(ll, elem)` | Find index |
| `reverse` | `(ll)` | Reverse order |
| `sort` | `(ll)` | Sort elements |
| `unique` | `(ll)` | Remove duplicates |
| `take` | `(ll, n)` | First n elements |
| `drop` | `(ll, n)` | Skip first n elements |
| `slice` | `(ll, start, end?)` | Sub-list |
| `zip` | `(a, b)` | Pair elements |
| `flatten` | `(ll)` | Flatten one level |
| `enumerate` | `(ll)` | Array of [index, value] |
| `min` | `(ll)` | Minimum element |
| `max` | `(ll)` | Maximum element |
| `fold` | `(ll, fn, init)` | Reduce |
| `map_list` | `(ll, fn)` | Transform elements |
| `filter_list` | `(ll, fn)` | Filter elements |
| `partition` | `(ll, fn)` | Split by predicate |
| `group_by` | `(ll, fn)` | Group by key |
| `equals` | `(a, b)` | Structural equality |
| `to_string` | `(ll)` | String representation |
| `clear` | `(ll)` | Empty the list |

---

## tree — Tree Operations

```dax
import tree
```

| Function | Signature | Description |
|----------|-----------|-------------|
| `node` | `(value, children?)` | Create tree node |
| `leaf` | `(value)` | Create leaf node |
| `value` | `(node)` | Get node value |
| `children` | `(node)` | Get children array |
| `is_leaf` | `(node)` | Check if leaf |
| `set_value` | `(node, value)` | New node with updated value |
| `add_child` | `(node, child)` | Add child |
| `add_children` | `(node, arr)` | Add multiple children |
| `size` | `(node)` | Total nodes in subtree |
| `depth` | `(node)` | Maximum depth |
| `height` | `(node)` | Height (edges to deepest leaf) |
| `depth_of` | `(node, value)` | Depth of node with value |
| `find` | `(node, value)` | Find node with value |
| `find_all` | `(node, fn)` | Find all matching nodes |
| `preorder` | `(node)` | Preorder traversal values |
| `inorder` | `(node)` | Inorder traversal values |
| `postorder` | `(node)` | Postorder traversal values |
| `levelorder` | `(node)` | Level-order (by level) |
| `map_tree` | `(node, fn)` | Transform all values |
| `filter_tree` | `(node, fn)` | Prune non-matching nodes |
| `clone` | `(node)` | Deep copy |
| `equals` | `(a, b)` | Structural equality |
| `to_string` | `(node)` | String representation |

---

## graph — Graph Operations

```dax
import graph
```

| Function | Signature | Description |
|----------|-----------|-------------|
| `new` | `()` | Create empty graph |
| `from_edges` | `(edges)` | Create from [[from,to], ...] |
| `add_vertex` | `(g, v)` | Add vertex |
| `add_edge` | `(g, from, to)` | Add directed edge |
| `add_undirected_edge` | `(g, a, b)` | Add bidirectional edge |
| `vertices` | `(g)` | Array of all vertices |
| `edges` | `(g)` | Array of [from, to] pairs |
| `neighbors` | `(g, v)` | Adjacent vertices |
| `degree` | `(g, v)` | Out-degree |
| `in_degree` | `(g, v)` | In-degree |
| `has_vertex` | `(g, v)` | Check vertex exists |
| `has_edge` | `(g, from, to)` | Check edge exists |
| `remove_vertex` | `(g, v)` | Remove vertex and edges |
| `remove_edge` | `(g, from, to)` | Remove edge |
| `vertex_count` | `(g)` | Number of vertices |
| `edge_count` | `(g)` | Number of edges |
| `is_empty` | `(g)` | Check if empty |
| `bfs` | `(g, start)` | Breadth-first traversal |
| `dfs` | `(g, start)` | Depth-first traversal |
| `shortest_path` | `(g, from, to)` | BFS shortest path |
| `has_path` | `(g, from, to)` | Check reachability |
| `is_connected` | `(g)` | Check if all reachable |
| `topological_sort` | `(g)` | Topological order (DAG) |
| `transpose` | `(g)` | Reverse all edges |
| `subgraph` | `(g, verts)` | Induced subgraph |
| `clone` | `(g)` | Deep copy |
| `equals` | `(a, b)` | Structural equality |
| `to_string` | `(g)` | String representation |

---

## json — JSON Serialization

```dax
import json
```

| Function | Signature | Description |
|----------|-----------|-------------|
| `parse` | `(str)` | Parse JSON string to objects |
| `stringify` | `(obj, indent?)` | Convert to JSON string |
| `is_valid` | `(str)` | Check if valid JSON |

---

## fs — File System

```dax
import fs
```

| Function | Signature | Description |
|----------|-----------|-------------|
| `read` | `(path)` | Read file to string |
| `write` | `(path, content)` | Write string to file |
| `append` | `(path, content)` | Append to file |
| `exists` | `(path)` | Check if exists |
| `is_file` | `(path)` | Check if regular file |
| `is_dir` | `(path)` | Check if directory |
| `mkdir` | `(path)` | Create directories |
| `rmdir` | `(path)` | Remove directory tree |
| `remove` | `(path)` | Remove file |
| `rename` | `(old, new)` | Rename/move |
| `copy` | `(src, dst)` | Copy file |
| `size` | `(path)` | File size in bytes |
| `list_dir` | `(path)` | Array of filenames |
| `list_dir_full` | `(path)` | Array of {name, is_dir, size} |
| `cwd` | `()` | Current working directory |
| `chdir` | `(path)` | Change working directory |
| `join` | `(parts...)` | Join path components |
| `parent` | `(path)` | Parent directory |
| `filename` | `(path)` | Filename from path |
| `extension` | `(path)` | File extension |
| `stem` | `(path)` | Filename without extension |
| `absolute` | `(path)` | Absolute path |
| `temp_dir` | `()` | System temp directory |
| `env` | `(name)` | Get environment variable |

---

## net — Networking

```dax
import net
```

| Function | Signature | Description |
|----------|-----------|-------------|
| `tcp_connect` | `(host, port)` | TCP connect → fd |
| `tcp_send` | `(fd, data)` | Send data |
| `tcp_recv` | `(fd, bufsize)` | Receive data |
| `tcp_close` | `(fd)` | Close connection |
| `udp_send` | `(host, port, data)` | UDP send |
| `http_get` | `(url)` | HTTP GET → {status, body} |
| `http_post` | `(url, body, type?)` | HTTP POST → {status, body} |
| `resolve` | `(host)` | DNS resolve → [ips] |

---

## crypto — Cryptographic Operations

```dax
import crypto
```

| Function | Signature | Description |
|----------|-----------|-------------|
| `md5` | `(data)` | MD5 hash |
| `sha256` | `(data)` | SHA-256 hash |
| `sha512` | `(data)` | SHA-512 hash |
| `hmac_sha256` | `(key, data)` | HMAC-SHA256 |
| `base64_encode` | `(data)` | Base64 encode |
| `base64_decode` | `(data)` | Base64 decode |
| `hex_encode` | `(data)` | Hex encode |
| `hex_decode` | `(data)` | Hex decode |
| `random_bytes` | `(n)` | Random bytes |
| `random_hex` | `(n)` | Random hex string |
| `uuid` | `()` | UUID v4 |
| `crc32` | `(data)` | CRC32 checksum |
| `pbkdf2` | `(pass, salt, iters)` | Key derivation |
| `hash` | `(data)` | FNV-1a hash |

---

## datetime — Date and Time

```dax
import datetime
```

| Function | Signature | Description |
|----------|-----------|-------------|
| `now` | `()` | Current timestamp |
| `now_ms` | `()` | Current timestamp (ms) |
| `format` | `(ts, fmt)` | Format timestamp |
| `year` | `(ts)` | Year |
| `month` | `(ts)` | Month (1-12) |
| `day` | `(ts)` | Day (1-31) |
| `hour` | `(ts)` | Hour (0-23) |
| `minute` | `(ts)` | Minute (0-59) |
| `second` | `(ts)` | Second (0-59) |
| `weekday` | `(ts)` | Day of week (0-6) |
| `day_of_year` | `(ts)` | Day of year (0-365) |
| `is_leap_year` | `(year)` | Check leap year |
| `days_in_month` | `(year, month)` | Days in month |
| `add_days` | `(ts, n)` | Add n days |
| `add_hours` | `(ts, n)` | Add n hours |
| `add_minutes` | `(ts, n)` | Add n minutes |
| `add_seconds` | `(ts, n)` | Add n seconds |
| `diff` | `(a, b)` | Difference in seconds |
| `diff_days` | `(a, b)` | Difference in days |
| `to_string` | `(ts)` | "YYYY-MM-DD HH:MM:SS" |
| `to_date_string` | `(ts)` | "YYYY-MM-DD" |
| `to_time_string` | `(ts)` | "HH:MM:SS" |
| `to_iso` | `(ts)` | ISO 8601 |
| `make` | `(y, m, d, h?, min?, s?)` | Create timestamp |
| `parse` | `(fmt, str)` | Parse string |
| `is_before` | `(a, b)` | a < b |
| `is_after` | `(a, b)` | a > b |
| `is_same_day` | `(a, b)` | Same calendar day |
| `timezone_offset` | `()` | UTC offset in seconds |
| `clock` | `()` | High-res time (ms) |

---

## random — Random Number Generation

```dax
import random
```

| Function | Signature | Description |
|----------|-----------|-------------|
| `seed` | `(n)` | Set seed |
| `int` | `(max?)` | Random integer |
| `int_range` | `(min, max)` | Integer in [min, max) |
| `float` | `()` | Random float [0, 1) |
| `float_range` | `(min, max)` | Float in [min, max) |
| `choice` | `(arr)` | Random element |
| `choices` | `(arr, n)` | n random with replacement |
| `sample` | `(arr, n)` | n random without replacement |
| `shuffle` | `(arr)` | Shuffled copy |
| `coin` | `()` | Random boolean |
| `weighted_choice` | `(opts, weights)` | Weighted random |
| `booleans` | `(n, prob?)` | Array of random booleans |
| `ints` | `(n, min, max)` | Array of random integers |
| `normal` | `(mean, stddev)` | Normal distribution |
| `exponential` | `(lambda)` | Exponential distribution |

---

## regex — Regular Expressions

```dax
import regex
```

| Function | Signature | Description |
|----------|-----------|-------------|
| `match` | `(pattern, str)` | First match or null |
| `matches` | `(pattern, str)` | All matches |
| `groups` | `(pattern, str)` | Capture groups |
| `named_groups` | `(pattern, str)` | Named groups |
| `test` | `(pattern, str)` | Check match |
| `replace` | `(pattern, str, rep)` | Replace first |
| `replace_all` | `(pattern, str, rep)` | Replace all |
| `replace_with_fn` | `(pattern, str, fn)` | Replace with function |
| `split` | `(pattern, str)` | Split by pattern |
| `find` | `(pattern, str)` | Find with positions |
| `count` | `(pattern, str)` | Count matches |
| `escape` | `(str)` | Escape regex metacharacters |
| `is_valid` | `(pattern)` | Check if valid regex |

---

## io — Input/Output

```dax
import io
```

| Function | Signature | Description |
|----------|-----------|-------------|
| `print` | `(args...)` | Print with newline |
| `println` | `(args...)` | Alias for print |
| `print_no_newline` | `(args...)` | Print without newline |
| `format` | `(tmpl, args...)` | Format string with {0}, {1}, etc. |
| `sprint` | `(tmpl, args...)` | Alias for format |
| `read_line` | `()` | Read line from stdin |
| `read_all` | `()` | Read all of stdin |
| `read` | `(prompt?)` | Read with optional prompt |
| `read_int` | `(prompt?)` | Read integer |
| `read_float` | `(prompt?)` | Read float |
| `read_bool` | `(prompt?)` | Read boolean |
| `read_until` | `(prompt?, delim)` | Read until delimiter |
| `confirm` | `(prompt?, default?)` | Yes/no confirmation |
| `choose` | `(opts, prompt?)` | Multiple choice menu |
| `progress` | `(current, total, width?)` | Progress bar |
| `spinner` | `()` | Spinning character |
| `table` | `(headers, rows)` | Formatted table |
| `json_table` | `(data)` | Table from objects |
| `clear_screen` | `()` | Clear terminal |
| `beep` | `()` | Terminal beep |

---

## os — Operating System

```dax
import os
```

| Function | Signature | Description |
|----------|-----------|-------------|
| `getenv` | `(name)` | Get environment variable |
| `setenv` | `(name, val)` | Set environment variable |
| `unsetenv` | `(name)` | Remove environment variable |
| `platform` | `()` | "windows"/"linux"/"darwin" |
| `arch` | `()` | CPU architecture |
| `hostname` | `()` | Computer name |
| `user` | `()` | Current username |
| `home` | `()` | Home directory |
| `getpid` | `()` | Process ID |
| `cpu_count` | `()` | Number of CPU cores |
| `memory_info` | `()` | {total, free, used, usage_percent} |
| `uname` | `()` | System information |
| `clock` | `()` | High-res time (ms) |
| `exec` | `(cmd)` | Run command → {exit_code, stdout} |
| `exit` | `(code?)` | Exit process |
| `sleep` | `(seconds)` | Sleep |

---

## encoding — Encoding/Decoding

```dax
import encoding
```

| Function | Signature | Description |
|----------|-----------|-------------|
| `base64_encode` | `(data)` | Base64 encode |
| `base64_decode` | `(data)` | Base64 decode |
| `base32_encode` | `(data)` | Base32 encode |
| `base32_decode` | `(data)` | Base32 decode |
| `hex_encode` | `(data)` | Hex encode (lowercase) |
| `hex_encode_upper` | `(data)` | Hex encode (uppercase) |
| `hex_decode` | `(data)` | Hex decode |
| `url_encode` | `(data)` | URL encode |
| `url_decode` | `(data)` | URL decode |
| `html_encode` | `(data)` | HTML entity encode |
| `html_decode` | `(data)` | HTML entity decode |
| `binary_encode` | `(data)` | Binary string encode |
| `binary_decode` | `(data)` | Binary string decode |
| `octal_encode` | `(data)` | Octal encode |
| `octal_decode` | `(data)` | Octal decode |
| `caesar_encode` | `(data, shift)` | Caesar cipher encrypt |
| `caesar_decode` | `(data, shift)` | Caesar cipher decrypt |
| `rot13` | `(data)` | ROT13 transform |
| `xor_encode` | `(data, key)` | XOR cipher (symmetric) |
